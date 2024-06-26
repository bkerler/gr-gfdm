#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2016 Johannes Demel.
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
from pygfdm.cyclic_prefix import (
    add_cyclic_prefix,
    pinch_block,
    get_raised_cosine_ramp,
    get_window_len,
    add_cyclic_starfix,
)


class qa_cyclic_prefixer_cc(gr_unittest.TestCase):
    def setUp(self):
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_001_init(self):
        # check if prefixer is properly ctor'ed / dtor'ed
        prefixer = gfdm.cyclic_prefixer_cc(16 * 8, 4, 0, 4, np.arange(4 * 2))
        prefixer = gfdm.cyclic_prefixer_cc(16 * 8, 4, 0, 4, np.arange(16 * 8 + 4))
        try:
            prefixer = gfdm.cyclic_prefixer_cc(16 * 8, 4, 0, 4, np.arange(16 * 8))
            raise ValueError("invalid parameter set, but passed anyway!")
        except:
            # expected behavior!
            pass

    def test_002_simple_cp(self):
        print("simple_cp test")
        block_len = 48
        cp_len = 8
        data = np.arange(block_len, dtype=complex) + 1
        ref = add_cyclic_prefix(data, cp_len)

        prefixer = gfdm.cyclic_prefixer_cc(
            block_len, cp_len, 0, 0, np.ones(block_len + cp_len)
        )
        src = blocks.vector_source_c(data)
        dst = blocks.vector_sink_c()
        self.tb.connect(src, prefixer, dst)
        self.tb.run()
        res = np.array(dst.data())
        self.assertComplexTuplesAlmostEqual(res, ref)

    def test_003_block_pinching(self):
        n_reps = 1
        n_subcarriers = 8
        n_timeslots = 8
        block_len = n_subcarriers * n_timeslots
        cp_len = 8
        ramp_len = 4
        cs_len = ramp_len * 2
        window_len = get_window_len(cp_len, n_timeslots, n_subcarriers, cs_len)
        window_taps = get_raised_cosine_ramp(ramp_len, window_len)
        data = np.arange(block_len, dtype=complex) + 1
        ref = add_cyclic_starfix(data, cp_len, cs_len)
        ref = pinch_block(ref, window_taps)
        data = np.tile(data, n_reps)
        ref = np.tile(ref, n_reps)
        print("input is: ", len(data), " -> ", len(ref))
        # short_window = np.concatenate((window_taps[0:ramp_len], window_taps[-ramp_len:]))
        prefixer = gfdm.cyclic_prefixer_cc(
            block_len, cp_len, cs_len, ramp_len, window_taps
        )
        src = blocks.vector_source_c(data)
        dst = blocks.vector_sink_c()
        self.tb.connect(src, prefixer, dst)
        self.tb.run()

        res = np.array(dst.data())
        print(ref[-10:])
        print(res[-10:])

        self.assertComplexTuplesAlmostEqual(res, ref, 4)


if __name__ == "__main__":
    gr_unittest.run(qa_cyclic_prefixer_cc)
