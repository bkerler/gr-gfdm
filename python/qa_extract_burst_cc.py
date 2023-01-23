#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2017 Johannes Demel.
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#

from gnuradio import gr, gr_unittest
from gnuradio import blocks
import gfdm_python as gfdm
import numpy as np
import pmt


def convert_frequency_to_phase_increment(freq, samp_rate):
    return 2.0 * np.pi * freq / samp_rate


def convert_phase_increment_to_frequency(phase_increment, samp_rate):
    return phase_increment * samp_rate / (2.0 * np.pi)


class qa_extract_burst_cc(gr_unittest.TestCase):
    def setUp(self):
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_001_t(self):
        n_frames = 500
        burst_len = 383
        gap_len = 53
        tag_key = "energy_start"

        data = np.arange(burst_len)
        ref = np.array([], dtype=complex)
        tags = []
        for i in range(n_frames):
            frame = np.ones(burst_len) * (i + 1)
            ref = np.concatenate((ref, frame))
            tag = gr.tag_t()
            tag.key = pmt.string_to_symbol(tag_key)
            tag.offset = burst_len + i * (burst_len + gap_len)
            tag.srcid = pmt.string_to_symbol("qa")
            tag.value = pmt.make_dict()
            tags.append(tag)
            data = np.concatenate((data, frame, np.zeros(gap_len)))
        # print(np.reshape(data, (-1, burst_len)))
        # print('data len', len(data), 'ref len', len(ref))

        src = blocks.vector_source_c(data, False, 1, tags)
        burster = gfdm.extract_burst_cc(burst_len, 0, tag_key)
        burster.set_fixed_phase_increment(300.0, True)
        burster.set_fixed_phase_increment(300.0, False)
        snk = blocks.vector_sink_c()
        self.tb.connect(src, burster, snk)
        self.tb.run()

        res = np.array(snk.data())
        rx_tags = snk.tags()
        self.assertEqual(len(rx_tags), n_frames)
        for i, t in enumerate(rx_tags):
            self.assertEqual(pmt.symbol_to_string(t.key), tag_key)
            self.assertTrue(pmt.is_true(t.value))
            self.assertEqual(pmt.symbol_to_string(t.srcid), burster.name())
            self.assertEqual(t.offset, i * burst_len)

        # check data
        self.assertComplexTuplesAlmostEqual(ref, res)

    def test_002_cfo_compensation(self):
        samp_rate = 30.72e6
        freq_offset = 1000.0 * 10
        n_frames = 2
        burst_len = 383
        gap_len = 53
        tag_key = "energy_start"

        data = np.arange(burst_len)
        ref = np.array([], dtype=complex)
        tags = []
        for i in range(n_frames):
            frame = np.ones(burst_len) * (i + 1)
            frame = frame.astype(complex)
            ref = np.concatenate((ref, frame))

            phase = convert_frequency_to_phase_increment(freq_offset, samp_rate)
            sine = np.exp(1.0j * phase * np.arange(frame.size))
            frame *= sine

            tag = gr.tag_t()
            tag.key = pmt.string_to_symbol(tag_key)
            tag.offset = burst_len + i * (burst_len + gap_len)
            tag.srcid = pmt.string_to_symbol("qa")
            tagvalue = {"sc_rot": complex(np.cos(phase), np.sin(phase))}
            tag.value = pmt.to_pmt(tagvalue)
            tags.append(tag)
            data = np.concatenate((data, frame, np.zeros(gap_len)))
        # print(np.reshape(data, (-1, burst_len)))
        # print('data len', len(data), 'ref len', len(ref))

        src = blocks.vector_source_c(data, False, 1, tags)
        burster = gfdm.extract_burst_cc(burst_len, 0, tag_key, True)
        snk = blocks.vector_sink_c()
        self.tb.connect(src, burster, snk)
        self.tb.run()

        res = np.array(snk.data())
        rx_tags = snk.tags()
        self.assertEqual(len(rx_tags), n_frames)
        for i, t in enumerate(rx_tags):
            self.assertEqual(pmt.symbol_to_string(t.key), tag_key)
            self.assertTrue(pmt.is_true(t.value))
            self.assertEqual(pmt.symbol_to_string(t.srcid), burster.name())
            self.assertEqual(t.offset, i * burst_len)

        # check data
        print(ref[0:10])
        print(res[0:10])
        reference_phase = convert_frequency_to_phase_increment(freq_offset, samp_rate)
        for i, left, right in zip(range(ref.size), ref, res):
            phase = np.angle(right)
            leftampl = np.abs(left)
            rightampl = np.abs(right)
            absdiff = np.abs(right - left)

            print(
                f"{i:4} {left:.7} == {right:.7}\t |{leftampl:.7}|\t|{rightampl:.7}|\tampl=={np.abs(leftampl - rightampl) < 1.0e-7}\t{absdiff=:.7e} != {absdiff < 1.0e-4}\t{phase:.7}"
            )
            self.assertAlmostEqual(leftampl, rightampl, 4)
            self.assertLess(phase, 1.0e-6)
        print(f"{reference_phase=}")
        self.assertComplexTuplesAlmostEqual(ref, res, 4)


if __name__ == "__main__":
    gr_unittest.run(qa_extract_burst_cc)
