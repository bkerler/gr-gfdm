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
from pygfdm.mapping import map_to_waveform_resources
from pygfdm.utils import get_random_qpsk


class qa_resource_mapper_cc(gr_unittest.TestCase):
    def setUp(self):
        self.n_frames = 3
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_001_per_timeslot(self):
        # set up fg
        n_frames = self.n_frames
        active_subcarriers = 110
        subcarriers = 128
        timeslots = 205
        smap = np.arange(active_subcarriers) + (subcarriers - active_subcarriers) // 2

        data = get_random_qpsk(active_subcarriers * timeslots)
        ref = map_to_waveform_resources(
            data, active_subcarriers, subcarriers, smap, True
        )
        for i in range(n_frames - 1):
            d = get_random_qpsk(active_subcarriers * timeslots)
            data = np.concatenate((data, d))
            ref = np.concatenate(
                (
                    ref,
                    map_to_waveform_resources(
                        d, active_subcarriers, subcarriers, smap, True
                    ),
                )
            )

        src = blocks.vector_source_c(data)
        mapper = gfdm.resource_mapper_cc(
            timeslots, subcarriers, active_subcarriers, smap, True
        )
        snk = blocks.vector_sink_c()
        self.tb.connect(src, mapper, snk)
        self.tb.run()
        # check data
        res = snk.data()
        self.assertComplexTuplesAlmostEqual(ref, res)

    def test_002_per_subcarrier(self):
        # set up fg
        n_frames = self.n_frames
        active_subcarriers = 110
        subcarriers = 128
        timeslots = 205
        smap = np.arange(active_subcarriers) + (subcarriers - active_subcarriers) // 2

        data = get_random_qpsk(active_subcarriers * timeslots)
        ref = map_to_waveform_resources(
            data, active_subcarriers, subcarriers, smap, False
        )
        for i in range(n_frames - 1):
            d = get_random_qpsk(active_subcarriers * timeslots)
            data = np.concatenate((data, d))
            ref = np.concatenate(
                (
                    ref,
                    map_to_waveform_resources(
                        d, active_subcarriers, subcarriers, smap, False
                    ),
                )
            )

        src = blocks.vector_source_c(data)
        mapper = gfdm.resource_mapper_cc(
            timeslots, subcarriers, active_subcarriers, smap, False
        )
        snk = blocks.vector_sink_c()
        self.tb.connect(src, mapper, snk)
        self.tb.run()
        # check data
        res = snk.data()
        self.assertComplexTuplesAlmostEqual(ref, res)

    def test_003_pilots(self):
        active_subcarriers = 110
        subcarriers = 128
        timeslots = 205
        smap = np.arange(active_subcarriers) + (subcarriers - active_subcarriers) // 2
        mapper = gfdm.resource_mapper_cc(
            timeslots, subcarriers, active_subcarriers, smap, True
        )

        pilots = [(0, 1, 1 + 1j)]
        self.assertRaises(ValueError, mapper.set_pilots, pilots)

        pilots = [(64, 225, 1 + 1j)]
        self.assertRaises(ValueError, mapper.set_pilots, pilots)

        pilots = [(64, 100, 1 + 1j)]
        mapper.set_pilots(pilots)
        value = mapper.pilots()[0]
        self.assertEqual(value[0], 64)
        self.assertEqual(value[1], 100)
        self.assertComplexAlmostEqual(value[2], 1 + 1j)

    def test_004_pilots_per_timeslot(self):
        # set up fg
        n_frames = self.n_frames
        active_subcarriers = 26
        subcarriers = 32
        timeslots = 5
        smap = np.arange(active_subcarriers) + (subcarriers - active_subcarriers) // 2
        pilots = [(6, 0, 99 + 0j), (6, 2, 99 + 0j), (12, 0, 99 + 0j)]

        frame_len = active_subcarriers * timeslots - len(pilots)

        data = get_random_qpsk(frame_len)
        ref = map_to_waveform_resources(
            data, active_subcarriers, subcarriers, smap, True
        )

        for i in range(n_frames - 1):
            d = get_random_qpsk(frame_len)
            data = np.concatenate((data, d))
            for p in pilots:
                d = np.insert(d, p[0] * timeslots + p[1], p[2])
            r = map_to_waveform_resources(
                d, active_subcarriers, subcarriers, smap, True
            )
            ref = np.concatenate((ref, r))
        print(f"{len(data)=}\t{frame_len*n_frames=}")
        print(f"{len(data)=}\t{frame_len=}\t{n_frames=}\t{len(pilots)=}")
        src = blocks.vector_source_c(data)
        mapper = gfdm.resource_mapper_cc(
            timeslots, subcarriers, active_subcarriers, smap, True
        )
        mapper.set_pilots(pilots)
        snk = blocks.vector_sink_c()
        self.tb.connect(src, mapper, snk)
        self.tb.run()
        # check data
        res = snk.data()
        for p in pilots:
            pos = p[0] * timeslots + p[1]
            self.assertComplexAlmostEqual(res[pos], p[2])


if __name__ == "__main__":
    gr_unittest.run(qa_resource_mapper_cc)
