#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2021 Johannes Demel.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

from gnuradio import gr, gr_unittest
from gnuradio import blocks
from receiver_cc import receiver_cc
from pygfdm.preamble import mapped_preamble
from pygfdm.mapping import get_subcarrier_map
from pygfdm.filters import get_frequency_domain_filter
from pygfdm.utils import get_random_qpsk
from pygfdm.gfdm_modulation import modulate_mapped_gfdm_block
from pygfdm.cyclic_prefix import pinch_cp_add_block
import numpy as np
import pmt


class qa_receiver_cc(gr_unittest.TestCase):

    def setUp(self):
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_instance(self):
        instance = receiver_cc(
            ic_iterations=2, activate_phase_compensation=True, activate_cfo_compensation=True)

        self.assertEqual(instance.get_ic_iterations(), 2)
        instance.set_ic_iterations(5)
        self.assertEqual(instance.get_ic_iterations(), 5)

        self.assertTrue(instance.get_activate_cfo_compensation())
        instance.set_activate_cfo_compensation(False)
        self.assertFalse(instance.get_activate_cfo_compensation())

        self.assertTrue(instance.get_activate_phase_compensation())
        instance.set_activate_phase_compensation(False)
        self.assertFalse(instance.get_activate_phase_compensation())

    def test_001_descriptive_test_name(self):
        n_frames = 2
        timeslots = 5
        subcarriers = 64
        active_subcarriers = 52
        cp_len = subcarriers // 2
        cs_len = cp_len // 2
        ramp_len = cs_len
        overlap = 2
        seed = 0
        subcarrier_map = get_subcarrier_map(subcarriers, active_subcarriers)
        taps = get_frequency_domain_filter(
            'rrc', 0.2, timeslots, subcarriers, overlap)
        tag_key = "frame_start"
        preamble, x_preamble = mapped_preamble(
            seed, 'rrc', 0.2, active_subcarriers, subcarriers, subcarrier_map, 2, cp_len, cp_len // 2, use_zadoff_chu=True)
        offset = len(preamble) + cp_len
        block_len = timeslots * subcarriers

        data = np.array([], dtype=np.complex)
        ref = np.array([], dtype=np.complex)
        tags = []
        for i in range(n_frames):
            d_block = modulate_mapped_gfdm_block(get_random_qpsk(
                timeslots * active_subcarriers), timeslots, subcarriers, active_subcarriers, 2, 0.2)
            frame = pinch_cp_add_block(
                d_block, timeslots, subcarriers, cp_len, cp_len // 2)
            frame = np.concatenate((preamble, frame))
            r = frame[offset:offset + block_len]
            ref = np.concatenate((ref, r))
            tag = gr.tag_t()
            tag.key = pmt.string_to_symbol(tag_key)
            tag.offset = len(data)
            tag.srcid = pmt.string_to_symbol('qa')
            tag.value = pmt.from_long(block_len)
            tag.value = pmt.make_dict()
            tags.append(tag)
            data = np.concatenate((data, frame))

        src = blocks.vector_source_c(data, False, 1, tags)
        instance = receiver_cc(
            timeslots, subcarriers, active_subcarriers, overlap, subcarrier_map, cp_len, cs_len, ramp_len, taps,
            True, x_preamble,
            ic_iterations=2, activate_phase_compensation=True, activate_cfo_compensation=True,
            sync_tag_key=tag_key)

        snk = blocks.vector_sink_c()
        self.tb.connect(src, instance, snk)
        self.tb.run()

        # # check data
        res = np.array(snk.data())
        tags = snk.tags()
        for t in tags:
            print(t)
        print(res)
        import matplotlib.pyplot as plt
        plt.scatter(res.real, res.imag)
        plt.show()
        # self.assertComplexTuplesAlmostEqual(res, ref, 5)


if __name__ == '__main__':
    gr_unittest.run(qa_receiver_cc)
