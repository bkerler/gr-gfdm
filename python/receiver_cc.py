#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2021 Johannes Demel.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#


from gnuradio import gr
from gnuradio import digital
import gfdm


class receiver_cc(gr.hier_block2):
    """
    docstring for block receiver_cc
    """

    def __init__(self,
                 timeslots=15,
                 subcarriers=64,
                 active_subcarriers=60,
                 overlap=2,
                 subcarrier_map=list(range(1, 61)),
                 cp_len=16,
                 cs_len=8,
                 ramp_len=8,
                 frequency_domain_taps=list(range(30)),
                 map_resources_per_timeslot=True,
                 preamble=[1.0 + 1.0j] * 128,
                 channel_estimator_id=1,
                 gfdm_constellation=digital.constellation_qpsk().base(),
                 ic_iterations=2,
                 activate_phase_compensation=True,
                 activate_cfo_compensation=True,
                 sync_tag_key="frame_start"):
        gr.hier_block2.__init__(self,
                                "receiver_cc",
                                gr.io_signature(1, 1, gr.sizeof_gr_complex),
                                gr.io_signature(2, 2, gr.sizeof_gr_complex))

        # These are setable on runtime.
        self.activate_cfo_compensation = activate_cfo_compensation
        self.activate_phase_compensation = activate_phase_compensation
        self.ic_iterations = ic_iterations

        block_len = timeslots * subcarriers
        full_block_len = block_len + cp_len + cs_len
        preamble_len = len(preamble)
        full_preamble_len = cp_len + preamble_len + cs_len
        # frame_len includes preamble, payload and CP+CS for both
        frame_len = full_preamble_len + full_block_len

        sc_map_is_dc_free = (subcarrier_map[0] == 0)

        self.extract_burst = gfdm.extract_burst_cc(
            frame_len, cp_len, sync_tag_key, activate_cfo_compensation
        )

        self.remove_prefix_data = gfdm.remove_prefix_cc(
            frame_len, block_len, full_preamble_len + cp_len, sync_tag_key
        )

        self.remove_prefix_preamble = gfdm.remove_prefix_cc(frame_len, preamble_len,
                                                            cp_len, sync_tag_key)

        self.channel_estimator = gfdm.channel_estimator_cc(
            timeslots,
            subcarriers,
            active_subcarriers,
            sc_map_is_dc_free,
            channel_estimator_id,
            preamble,
        )

        self.advanced_receiver = gfdm.advanced_receiver_sb_cc(
            timeslots,
            subcarriers,
            overlap,
            ic_iterations,
            frequency_domain_taps,
            gfdm_constellation,
            subcarrier_map,
            activate_phase_compensation,
        )

        self.resource_demapper = gfdm.resource_demapper_cc(
            timeslots,
            subcarriers,
            active_subcarriers,
            subcarrier_map,
            map_resources_per_timeslot,
        )

        self.connect(
            (self, 0),
            (self.extract_burst, 0),
            (self.remove_prefix_data, 0),
            (self.advanced_receiver, 0),
            (self.resource_demapper, 0),
            (self, 0)
        )
        self.connect(
            (self.extract_burst, 0),
            (self.remove_prefix_preamble, 0),
            (self.channel_estimator, 0),
            (self.advanced_receiver, 1),
        )

        self.connect(
            (self.channel_estimator, 0), (self, 1)
        )

    def get_activate_cfo_compensation(self):
        return self.activate_cfo_compensation

    def set_activate_cfo_compensation(self, activate_cfo_compensation):
        self.activate_cfo_compensation = activate_cfo_compensation
        self.extract_burst.activate_cfo_compensation(activate_cfo_compensation)

    def get_ic_iterations(self):
        return self.ic_iterations

    def set_ic_iterations(self, ic_iterations):
        self.ic_iterations = ic_iterations
        self.advanced_receiver.set_ic(self.ic_iterations)

    def get_activate_phase_compensation(self):
        return self.activate_phase_compensation

    def set_activate_phase_compensation(self, activate_phase_compensation):
        self.activate_phase_compensation = activate_phase_compensation
        self.advanced_receiver.set_phase_compensation(
            activate_phase_compensation)
