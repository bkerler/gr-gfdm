#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2022 Johannes Demel.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

from gnuradio import gr, gr_unittest

# from gnuradio import blocks
from multi_port_receiver_cc import multi_port_receiver_cc


class qa_multi_port_receiver_cc(gr_unittest.TestCase):
    def setUp(self):
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_instance(self):
        instance = multi_port_receiver_cc(
            3,
            activate_phase_compensation=True,
            activate_cfo_compensation=True,
            ic_iterations=5,
        )
        self.assertEqual(instance.get_activate_cfo_compensation(), True)
        self.assertEqual(instance.get_activate_phase_compensation(), True)
        self.assertEqual(instance.get_ic_iterations(), 5)

        instance.set_activate_cfo_compensation(False)
        self.assertEqual(instance.get_activate_cfo_compensation(), False)
        instance.set_activate_cfo_compensation(True)
        self.assertEqual(instance.get_activate_cfo_compensation(), True)

        instance.set_activate_phase_compensation(False)
        self.assertEqual(instance.get_activate_phase_compensation(), False)
        instance.set_activate_phase_compensation(True)
        self.assertEqual(instance.get_activate_phase_compensation(), True)

        instance.set_ic_iterations(3)
        self.assertEqual(instance.get_ic_iterations(), 3)

    # def test_001_descriptive_test_name(self):
    #     # set up fg
    #     self.tb.run()
    #     # check data


if __name__ == "__main__":
    gr_unittest.run(qa_multi_port_receiver_cc)
