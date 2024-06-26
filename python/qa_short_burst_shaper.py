#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright 2019 Johannes Demel.
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
import pmt
import numpy as np


class qa_short_burst_shaper(gr_unittest.TestCase):
    def setUp(self):
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_001_t(self):
        tkey = "packet_len"
        tsrc = "testsrc"
        ptkey = pmt.intern(tkey)
        ptsrc = pmt.intern(tsrc)
        pre_padding = 17
        post_padding = 23
        tag = gr.tag_utils.python_to_tag((0, ptkey, pmt.from_long(128), ptsrc))

        data = np.arange(128) + 1
        data = data.astype(complex)

        ref = np.concatenate(
            (
                np.zeros(pre_padding, dtype=data.dtype),
                data,
                np.zeros(post_padding, dtype=data.dtype),
            )
        )

        src = blocks.vector_source_c(data, tags=(tag,))
        uut = gfdm.short_burst_shaper(pre_padding, post_padding, 1.0 + 0.0j, 1, tkey)
        snk = blocks.vector_sink_c()
        # set up fg
        self.tb.connect(src, uut, snk)
        self.tb.run()
        # check data
        res = np.array(snk.data())

        self.assertComplexTuplesAlmostEqual(ref, res)

    def test_002_t(self):
        tkey = "packet_len"
        tsrc = "testsrc"
        ptkey = pmt.intern(tkey)
        ptsrc = pmt.intern(tsrc)
        pre_padding = 17
        post_padding = 23
        n_bursts = 3
        tags = []
        burst_len = 128
        offset = 0
        data = np.array((), dtype=complex)
        ref = np.array((), dtype=complex)
        for i in range(n_bursts):
            tag = gr.tag_utils.python_to_tag(
                (offset, ptkey, pmt.from_long(burst_len * (i + 1)), ptsrc)
            )
            tags.append(tag)
            offset += burst_len * (i + 1)

            d = np.arange(burst_len * (i + 1)) + 1
            d = d.astype(data.dtype)
            data = np.append(data, d)
            r = np.concatenate(
                (
                    np.zeros(pre_padding, dtype=d.dtype),
                    d,
                    np.zeros(post_padding, dtype=d.dtype),
                )
            )
            ref = np.append(ref, r)
        # ref = np.concatenate((np.zeros(pre_padding, dtype=data.dtype),
        #                       data, np.zeros(post_padding, dtype=data.dtype)))

        src = blocks.vector_source_c(data, tags=tags)
        uut = gfdm.short_burst_shaper(pre_padding, post_padding, 1.0 + 0.0j, 1, tkey)
        snk = blocks.vector_sink_c()
        # set up fg
        self.tb.connect(src, uut, snk)
        self.tb.run()
        # check data
        res = np.array(snk.data())
        # print(res)

        self.assertComplexTuplesAlmostEqual(ref, res)

    def test_003_nports(self):
        tkey = "packet_len"
        tsrc = "testsrc"
        ptkey = pmt.intern(tkey)
        ptsrc = pmt.intern(tsrc)
        pre_padding = 17
        post_padding = 23
        n_bursts = 3
        nports = 3
        tags = []
        burst_len = 128
        offset = 0
        data = np.array((), dtype=complex)
        ref = np.array((), dtype=complex)
        for i in range(n_bursts):
            tag = gr.tag_utils.python_to_tag(
                (offset, ptkey, pmt.from_long(burst_len * (i + 1)), ptsrc)
            )
            tags.append(tag)
            offset += burst_len * (i + 1)

            d = np.arange(burst_len * (i + 1)) + 1
            d = d.astype(data.dtype)
            data = np.append(data, d)
            r = np.concatenate(
                (
                    np.zeros(pre_padding, dtype=d.dtype),
                    d,
                    np.zeros(post_padding, dtype=d.dtype),
                )
            )
            ref = np.append(ref, r)

        # set up fg
        srcs = [blocks.vector_source_c(data, tags=tags) for _ in range(nports)]
        uut = gfdm.short_burst_shaper(
            pre_padding, post_padding, 1.0 + 0.0j, nports, tkey
        )
        snks = [blocks.vector_sink_c() for _ in range(nports)]

        for i in range(nports):
            self.tb.connect(srcs[i], (uut, i))
            self.tb.connect((uut, i), snks[i])

        self.tb.run()

        # check data
        for i, snk in enumerate(snks):
            res = np.array(snk.data())
            self.assertComplexTuplesAlmostEqual(ref, res)
            tags = snk.tags()
            offset = 0
            for j, t in enumerate(tags):
                print(f"port={i}, offset={t.offset}, value={pmt.to_python(t.value)}")
                self.assertEqual(t.offset, offset)
                offset += pmt.to_python(t.value)
            # print(tags)


if __name__ == "__main__":
    gr_unittest.run(qa_short_burst_shaper)
