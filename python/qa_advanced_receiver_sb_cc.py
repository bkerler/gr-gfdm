#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2016, 2019 Andrej Rode, Johannes Demel.
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
# import sys
# print(sys.path)

from gnuradio import gr, gr_unittest
from gnuradio import blocks, digital
import gfdm_python as gfdm
from pygfdm import filters, utils
from pygfdm.gfdm_receiver import gfdm_demodulate_block
from pygfdm.mapping import get_subcarrier_map, map_to_waveform_resources, map_to_waveform_resource_grid
from pygfdm.utils import get_random_qpsk, calculate_signal_energy
import numpy as np


def struct(data):
    return type("Struct", (object,), data)()


class qa_advanced_receiver_sb_cc(gr_unittest.TestCase):
    def setUp(self):
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_001_simple_receiver(self):
        # make sure advanced receiver works like simple receiver
        # in case no IC iterations are applied!
        reps = 5
        alpha = 0.5
        M = 127
        K = 16
        L = 2
        taps = filters.get_frequency_domain_filter("rrc", alpha, M, K, L)
        data = np.array([], dtype=complex)
        ref = np.array([], dtype=complex)
        for _11 in range(reps):
            d = utils.get_random_qpsk(M * K)
            ref = np.append(ref, gfdm_demodulate_block(d, taps, K, M, L))
            data = np.append(data, d)
        # print data
        # print ref
        # print "MAXIMUM ref value: ", np.max(abs(ref))

        src = blocks.vector_source_c(data)
        est_data = np.ones(len(data), dtype=complex)
        est_src = blocks.vector_source_c(est_data)
        gfdm_constellation = digital.constellation_qpsk().base()
        mod = gfdm.advanced_receiver_sb_cc(
            M, K, L, 0, taps, gfdm_constellation, np.arange(K), 0
        )
        dst = blocks.vector_sink_c()

        self.tb.connect(src, (mod, 0), dst)
        self.tb.connect(est_src, (mod, 1))
        # set up fg
        self.tb.run()
        # check data
        res = np.array(dst.data())

        self.assertComplexTuplesAlmostEqual(ref, res, 4)

    def test_002_t(self):
        n_frames = 1
        gfdm_var = struct(
            {
                "subcarriers": 64,
                "timeslots": 9,
                "alpha": 0.5,
                "overlap": 2,
            }
        )
        gfdm_constellation = digital.constellation_qpsk().base()
        self.f_taps = f_taps = filters.get_frequency_domain_filter(
            "rrc", 1.0, gfdm_var.timeslots, gfdm_var.subcarriers, gfdm_var.overlap
        )
        source_bits = np.random.randint(
            0,
            len(gfdm_constellation.points()),
            n_frames * gfdm_var.timeslots * gfdm_var.subcarriers,
        ).astype(np.uint8)
        self.random_bits = blocks.vector_source_b(source_bits, False)
        self.bits_to_symbols = digital.chunks_to_symbols_bc(
            (gfdm_constellation.points()), 1
        )
        self.mod = gfdm.simple_modulator_cc(
            gfdm_var.timeslots, gfdm_var.subcarriers, gfdm_var.overlap, f_taps
        )
        self.demod = gfdm.advanced_receiver_sb_cc(
            gfdm_var.timeslots,
            gfdm_var.subcarriers,
            gfdm_var.overlap,
            64,
            f_taps,
            gfdm_constellation,
            np.arange(gfdm_var.subcarriers),
            0,
        )
        self.tx_symbols = blocks.vector_sink_c()
        self.rx_symbols = blocks.vector_sink_c()
        self.tb.connect((self.random_bits, 0), (self.bits_to_symbols, 0))
        self.tb.connect((self.bits_to_symbols, 0), (self.tx_symbols, 0))
        self.tb.connect((self.bits_to_symbols, 0), (self.mod, 0))
        self.tb.connect((self.mod, 0), (self.demod, 0))
        self.tb.connect((self.demod, 0), (self.rx_symbols, 0))
        self.tb.run()
        ref = np.array(self.tx_symbols.data())
        res = np.array(self.rx_symbols.data())
        # more or less make sure all symbols have their correct sign.
        self.assertComplexTuplesAlmostEqual(ref, res, 2)

    def test_003_setIC(self):
        ic = 2
        timeslots = 9
        subcarriers = 32
        active_subcarriers = 20
        overlap = 2
        f_taps = filters.get_frequency_domain_filter(
            "rrc", 0.5, timeslots, subcarriers, overlap
        )
        gfdm_constellation = digital.constellation_qpsk().base()
        subcarrier_map = get_subcarrier_map(subcarriers, active_subcarriers)
        demod = gfdm.advanced_receiver_sb_cc(
            timeslots,
            subcarriers,
            overlap,
            64,
            f_taps,
            gfdm_constellation,
            subcarrier_map,
            0,
        )
        demod.set_ic(ic)
        self.assertEqual(ic, demod.get_ic())

    def test_004_active_subcarriers(self):
        n_frames = 1
        timeslots = 9
        subcarriers = 32
        active_subcarriers = 20
        overlap = 2
        ic_iterations = 64
        f_taps = filters.get_frequency_domain_filter(
            "rrc", 0.5, timeslots, subcarriers, overlap
        )
        gfdm_constellation = digital.constellation_qpsk().base()
        print(gfdm_constellation.points())
        subcarrier_map = get_subcarrier_map(subcarriers, active_subcarriers)

        data = get_random_qpsk(n_frames * timeslots * active_subcarriers)
        data *= 2.0
        src = blocks.vector_source_c(data)
        mapper = gfdm.resource_mapper_cc(
            timeslots, subcarriers, active_subcarriers, subcarrier_map, True
        )
        mod = gfdm.simple_modulator_cc(timeslots, subcarriers, overlap, f_taps)
        demod = gfdm.advanced_receiver_sb_cc(
            timeslots,
            subcarriers,
            overlap,
            ic_iterations,
            f_taps,
            gfdm_constellation,
            subcarrier_map,
            0,
        )
        demod.set_ic(64)
        demapper = gfdm.resource_demapper_cc(
            timeslots, subcarriers, active_subcarriers, subcarrier_map, True
        )
        snk = blocks.vector_sink_c()
        self.tb.connect(src, mapper, mod, demod, demapper, snk)
        self.tb.run()

        res = np.array(snk.data())
        print(data[0:10])
        print(res[0:10])
        self.assertComplexTuplesAlmostEqual(data, res, 1)
        # self.assertComplexTuplesAlmostEqual(data, res, 2)


    def test_005_pilots(self):
        np.set_printoptions(linewidth=250, precision=3)
        print(f"Test receiver demodulator with pilots")
        n_frames = 1
        timeslots = 15
        subcarriers = 64
        active_subcarriers = 60
        overlap = 2
        ic_iterations = 2
        f_taps = filters.get_frequency_domain_filter(
            "rrc", 0.2, timeslots, subcarriers, overlap
        )
        gfdm_constellation = digital.constellation_calcdist(
            [-0.707 - 0.707j, -0.707 + 0.707j, 0.707 + 0.707j, 0.707 - 0.707j],
            [0, 1, 3, 2],
            4,
            1,
            digital.constellation.AMPLITUDE_NORMALIZATION,
        ).base()
        # print(gfdm_constellation.points())
        subcarrier_map = get_subcarrier_map(subcarriers, active_subcarriers, True)
        num_pilots = 16
        pilot_spacing = 1 + subcarrier_map.size // num_pilots

        upper_subcarrier_map = subcarrier_map[0:subcarrier_map.size // 2]
        lower_subcarrier_map = subcarrier_map[subcarrier_map.size // 2:]
        pilot_subcarriers = np.concatenate((upper_subcarrier_map[::-1][::pilot_spacing][::-1], lower_subcarrier_map[::pilot_spacing]))
        pilot_subcarriers = np.sort(pilot_subcarriers)

        import pathlib
        data = np.load(pathlib.Path(__file__).parent / "symbols.npy")
        # frame = np.load(pathlib.Path(__file__).parent / "frame.npy")

        matdata = map_to_waveform_resource_grid(np.copy(data), active_subcarriers, subcarriers, subcarrier_map, True)

        print(matdata)
        print(matdata.shape)

        scs = np.concatenate((np.arange(0, 29, 4) + 1, np.arange(63, 34, -4)))
        scs = np.sort(scs)
        refpilots = []
        for s in scs:
            p = matdata[s, 0]
            print(f"{s=} t=0\t{p.real:+.3f} {p.imag:+.3f}")
            refpilots.append((s, 0, p))

        # data = get_random_qpsk(n_frames * timeslots * active_subcarriers)
        # data *= 2.0
        src = blocks.vector_source_c(data)
        rot = 2. * np.pi * 0.02
        eqdata = np.array([1.+0.j,] * timeslots * subcarriers) * np.exp(1.j * rot)
        estimate = blocks.vector_source_c(eqdata)
        mapper = gfdm.resource_mapper_cc(
            timeslots, subcarriers, active_subcarriers, subcarrier_map, True
        )
        mod = gfdm.simple_modulator_cc(timeslots, subcarriers, overlap, f_taps)
        demod = gfdm.advanced_receiver_sb_cc(
            timeslots,
            subcarriers,
            overlap,
            ic_iterations,
            f_taps,
            gfdm_constellation,
            subcarrier_map,
            0,
        )
        demod.set_ic(64)
        demod.set_pilots(refpilots)
        print(f"{demod.activate_pilot_estimation(True)=}")
        demapper = gfdm.resource_demapper_cc(
            timeslots, subcarriers, active_subcarriers, subcarrier_map, True
        )
        snk = blocks.vector_sink_c()
        self.tb.connect(src, mapper, mod, demod, demapper, snk)
        self.tb.connect(estimate, (demod, 1))
        self.tb.run()

        res = np.array(snk.data())
        print(f"{rot=}")
        print(data[0:10])
        print(res[0:10])
        used_pilots = demod.pilots()
        print(used_pilots)
        for ref, used in zip(refpilots, used_pilots):
            self.assertEqual(ref[0], used[0])
            self.assertEqual(ref[1], used[1])
            self.assertComplexAlmostEqual(ref[2], used[2])
        self.assertComplexTuplesAlmostEqual(data, res, 1)
        # self.assertComplexTuplesAlmostEqual(data, res, 2)

    def test_006_pilot_processing_chain(self):
        np.set_printoptions(linewidth=250, precision=3)
        print(f"Test pilot chain")
        n_frames = 1
        timeslots = 15
        subcarriers = 64
        active_subcarriers = 60
        overlap = 2
        ic_iterations = 2
        f_taps = filters.get_frequency_domain_filter(
            "rrc", 0.2, timeslots, subcarriers, overlap
        )
        gfdm_constellation = digital.constellation_calcdist(
            [-0.707 - 0.707j, -0.707 + 0.707j, 0.707 + 0.707j, 0.707 - 0.707j],
            [0, 1, 3, 2],
            4,
            1,
            digital.constellation.AMPLITUDE_NORMALIZATION,
        ).base()
        subcarrier_map = get_subcarrier_map(subcarriers, active_subcarriers, True)
        num_pilots = 16
        pilot_spacing = 1 + subcarrier_map.size // num_pilots

        pilot_values = np.repeat((1+1j) / np.sqrt(2), num_pilots)
        upper_subcarrier_map = subcarrier_map[0:subcarrier_map.size // 2]
        lower_subcarrier_map = subcarrier_map[subcarrier_map.size // 2:]
        pilot_subcarriers = np.concatenate((upper_subcarrier_map[::-1][::pilot_spacing][::-1], lower_subcarrier_map[::pilot_spacing]))
        pilot_subcarriers = np.sort(pilot_subcarriers)

        pilots = []
        for sidx, value in zip(pilot_subcarriers, pilot_values):
            pilots.append((sidx, 0, value))

        data_frame_len = active_subcarriers * timeslots - len(pilots)
        data = get_random_qpsk(data_frame_len * n_frames)
        src = blocks.vector_source_c(data)
        rot = 2. * np.pi * 0.02
        eqdata = np.array([1.+0.j,] * timeslots * subcarriers) * np.exp(1.j * rot)
        estimate = blocks.vector_source_c(eqdata)
        mapper = gfdm.resource_mapper_cc(
            timeslots, subcarriers, active_subcarriers, subcarrier_map, True
        )
        mapper.set_pilots(pilots)
        mod = gfdm.simple_modulator_cc(timeslots, subcarriers, overlap, f_taps)
        demod = gfdm.advanced_receiver_sb_cc(
            timeslots,
            subcarriers,
            overlap,
            ic_iterations,
            f_taps,
            gfdm_constellation,
            subcarrier_map,
            0,
        )
        demod.set_ic(64)
        demod.set_pilots(pilots)
        demod.activate_pilot_estimation(True)
        demapper = gfdm.resource_demapper_cc(
            timeslots, subcarriers, active_subcarriers, subcarrier_map, True
        )
        demapper.set_pilots(pilots)
        # print(f"{mapper.pilots()=}")
        # print(f"{demapper.pilots()=}")
        # print(f"{demod.pilots()=}")

        snk = blocks.vector_sink_c()
        demodsnk = blocks.vector_sink_c()

        self.tb.connect(src, mapper, mod, demod, demapper, snk)
        self.tb.connect(estimate, (demod, 1))
        self.tb.connect(demod, demodsnk)
        self.tb.run()

        res = np.array(snk.data())
        frame = np.array(demodsnk.data())
        print(f"{rot=}")

        used_pilots = demod.pilots()

        for ref, used in zip(pilots, used_pilots):
            self.assertEqual(ref[0], used[0])
            self.assertEqual(ref[1], used[1])
            self.assertComplexAlmostEqual(ref[2], used[2])

        framemat = np.reshape(frame, (-1, timeslots))
        rxpilots = np.array([framemat[p[0], p[1]] for p in pilots])
        # print(framemat)
        # print(rxpilots)
        self.assertComplexTuplesAlmostEqual(rxpilots, [p[2] for p in pilots], 1)
        self.assertComplexTuplesAlmostEqual(data, res, 1)

if __name__ == "__main__":
    gr_unittest.run(qa_advanced_receiver_sb_cc)
