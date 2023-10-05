#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2022 Johannes Demel.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

from gnuradio import gr, digital
from gfdm import receiver_cc
import numpy as np


class multi_port_receiver_cc(gr.hier_block2):
    """
    docstring for block multi_port_receiver_cc
    """

    def __init__(
        self,
        num_antenna_ports,
        timeslots=15,
        subcarriers=64,
        active_subcarriers=60,
        overlap=2,
        subcarrier_map=list(range(1, 61)),
        cp_len=16,
        cs_len=8,
        frequency_domain_taps=list(range(30)),
        map_resources_per_timeslot=True,
        preamble=[1.0 + 1.0j] * 128,
        channel_estimator_id=1,
        symbol_constellation=digital.constellation_qpsk().base(),
        ic_iterations=2,
        activate_phase_compensation=True,
        activate_cfo_compensation=True,
        sync_tag_key="frame_start",
    ):
        gr.hier_block2.__init__(
            self,
            "multi_port_receiver_cc",
            # Input signature
            gr.io_signature(num_antenna_ports, num_antenna_ports, gr.sizeof_gr_complex),
            gr.io_signature(
                3 * num_antenna_ports, 3 * num_antenna_ports, gr.sizeof_gr_complex
            ),
        )  # Output signature
        self.logger = gr.logger(f"gr_log.{self.symbol_name()}")
        self.logger.debug(
            f"{activate_cfo_compensation=}, {activate_phase_compensation=}"
        )

        self.receivers = [
            receiver_cc(
                timeslots,
                subcarriers,
                active_subcarriers,
                overlap,
                subcarrier_map,
                cp_len,
                cs_len,
                cs_len,
                frequency_domain_taps,
                map_resources_per_timeslot,
                preamble,
                channel_estimator_id,
                symbol_constellation,
                ic_iterations,
                activate_phase_compensation,
                activate_cfo_compensation,
                sync_tag_key,
                port,
            )
            for port in range(num_antenna_ports)
        ]

        # pilotsc = np.concatenate((np.arange(0, 29, 4) + 1, np.arange(63, 34, -4)))
        # pilotsc = np.sort(pilotsc)
        # pilots = []
        # # pilots.append((1, 0, 1-1j ));
        # pilots.append((5, 0, -1+1j ));
        # pilots.append((9, 0, 1-1j ));
        # pilots.append((13, 0, 1+1j ));
        # pilots.append((17, 0, 1-1j ));
        # pilots.append((21, 0, -1-1j ));
        # pilots.append((25, 0, 1+1j ));
        # pilots.append((29, 0, -1+1j ));

        # pilots.append((35, 0, -1-1j ));
        # pilots.append((39, 0, 1-1j ));
        # pilots.append((43, 0, 1-1j ));
        # pilots.append((47, 0, -1-1j ));
        # pilots.append((51, 0, 1-1j ));
        # pilots.append((55, 0, -1+1j ));
        # pilots.append((59, 0, 1-1j ));
        # pilots.append((63, 0, 1+1j ));

        # pilots = [(p[0] - 1, p[1], p[2] / np.sqrt(2.)) for p in pilots]

        # for r in self.receivers:
        #     r.activate_pilot_estimation(True)
            # r.set_pilots(pilots)

        # Define blocks and connect them
        for port in range(num_antenna_ports):
            # The demodulated complex symbols output
            self.connect((self, port), (self.receivers[port], 0), (self, port))

            # The channel estimates
            self.connect((self.receivers[port], 1), (self, num_antenna_ports + port))

            # The extracted bursts
            self.connect(
                (self.receivers[port], 2), (self, 2 * num_antenna_ports + port)
            )

    def get_activate_cfo_compensation(self):
        vals = np.array([r.get_activate_cfo_compensation() for r in self.receivers])
        if np.all(vals == True):
            return True
        elif np.all(vals == False):
            return False
        else:
            return vals

    def set_activate_cfo_compensation(self, activate_cfo_compensation):
        self.logger.debug(f"set_activate_cfo_compensation({activate_cfo_compensation})")
        for r in self.receivers:
            r.set_activate_cfo_compensation(activate_cfo_compensation)

    def set_fixed_phase_increment(self, phase_increment, activate):
        for r in self.receivers:
            r.set_fixed_phase_increment(phase_increment, activate)

    def get_ic_iterations(self):
        vals = np.array([r.get_ic_iterations() for r in self.receivers])
        uvals = np.unique(vals)
        if uvals.size == 1:
            return uvals[0]
        else:
            return uvals

    def set_ic_iterations(self, ic_iterations):
        for r in self.receivers:
            r.set_ic_iterations(ic_iterations)

    def get_activate_phase_compensation(self):
        vals = np.array([r.get_activate_phase_compensation() for r in self.receivers])
        if np.all(vals == True):
            return True
        elif np.all(vals == False):
            return False
        else:
            return vals

    def set_activate_phase_compensation(self, activate_phase_compensation):
        for r in self.receivers:
            r.set_activate_phase_compensation(activate_phase_compensation)

    def activate_pilot_estimation(self, activate):
        res = [r.activate_pilot_estimation(activate) for r in self.receivers]
        ures = np.unique(res)
        assert ures.size == 1
        assert ures[0] == activate
        return ures[0]

    def set_pilots(self, pilots):
        for r in self.receivers:
            r.set_pilots(pilots)

    def pilots(self):
        all_pilots = [r.pilots() for r in self.receivers]
        for p in all_pilots[1:]:
            assert len(all_pilots[0]) == len(p)
        return all_pilots[0]
