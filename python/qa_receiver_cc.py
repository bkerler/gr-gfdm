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
from pygfdm.cyclic_prefix import add_cyclic_starfix
import numpy as np
import pmt


class qa_receiver_cc(gr_unittest.TestCase):
    def setUp(self):
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_001_instance(self):
        instance = receiver_cc(
            ic_iterations=2,
            activate_phase_compensation=True,
            activate_cfo_compensation=True,
        )

        self.assertEqual(instance.get_ic_iterations(), 2)
        instance.set_ic_iterations(5)
        self.assertEqual(instance.get_ic_iterations(), 5)

        self.assertTrue(instance.get_activate_cfo_compensation())
        instance.set_activate_cfo_compensation(False)
        self.assertFalse(instance.get_activate_cfo_compensation())

        self.assertTrue(instance.get_activate_phase_compensation())
        instance.set_activate_phase_compensation(False)
        self.assertFalse(instance.get_activate_phase_compensation())

    def test_002_basic_frames(self):
        print("test 002!")
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
        taps = get_frequency_domain_filter("rrc", 0.2, timeslots, subcarriers, overlap)
        tag_key = "frame_start"
        preamble, x_preamble = mapped_preamble(
            seed,
            "rrc",
            0.2,
            active_subcarriers,
            subcarriers,
            subcarrier_map,
            2,
            cp_len,
            cp_len // 2,
            use_zadoff_chu=True,
        )

        data = np.array([], dtype=np.complex)
        ref = np.array([], dtype=np.complex)
        frames = np.array([], dtype=np.complex)
        tags = []
        for _ in range(n_frames):
            symbols = get_random_qpsk(timeslots * active_subcarriers)
            ref = np.concatenate((ref, symbols))
            d_block = modulate_mapped_gfdm_block(
                symbols, timeslots, subcarriers, active_subcarriers, 2, 0.2
            )

            frame = add_cyclic_starfix(d_block, cp_len, cs_len)
            frame = np.concatenate((preamble, frame))
            frames = np.concatenate((frames, frame))

            tag = gr.tag_t()
            tag.key = pmt.string_to_symbol(tag_key)
            tag.offset = data.size + cp_len
            tag.srcid = pmt.string_to_symbol("qa")
            tag.value = pmt.from_long(timeslots * subcarriers)
            tag.value = pmt.make_dict()
            tags.append(tag)
            data = np.concatenate((data, frame))
        data = np.concatenate((data, np.zeros(2 * cp_len, dtype=data.dtype)))

        src = blocks.vector_source_c(data, False, 1, tags)
        instance = receiver_cc(
            timeslots,
            subcarriers,
            active_subcarriers,
            overlap,
            subcarrier_map,
            cp_len,
            cs_len,
            ramp_len,
            taps,
            True,
            x_preamble,
            ic_iterations=2,
            activate_phase_compensation=True,
            activate_cfo_compensation=True,
            sync_tag_key=tag_key,
        )

        snk = blocks.vector_sink_c()
        estimate_snk = blocks.vector_sink_c()
        frame_snk = blocks.vector_sink_c()
        self.tb.connect(src, instance, snk)
        self.tb.connect((instance, 1), estimate_snk)
        self.tb.connect((instance, 2), frame_snk)
        self.tb.run()

        # # check data
        frametags = frame_snk.tags()
        self.assertEqual(len(frametags), len(tags))
        for ft, t in zip(frametags, tags):
            self.assertEqual(ft.offset + cp_len, t.offset)
        rxframes = np.array(frame_snk.data())
        self.assertComplexTuplesAlmostEqual(rxframes, frames)

        estimates = np.array(estimate_snk.data())
        self.assertEqual(estimates.size, n_frames * timeslots * subcarriers)
        self.assertComplexTuplesAlmostEqual(estimates, np.ones_like(estimates), 5)

        rxtags = snk.tags()
        print(len(rxtags))
        for t in rxtags:
            print(t.offset, t.srcid, t.value)

        res = np.array(snk.data())
        self.assertComplexTuplesAlmostEqual(res, ref, 0)


if __name__ == "__main__":
    gr_unittest.run(qa_receiver_cc)
