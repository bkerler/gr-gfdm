/*
 * Copyright 2022 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

/***********************************************************************************/
/* This file is automatically generated using bindtool and can be manually edited  */
/* The following lines can be configured to regenerate this file during cmake      */
/* If manual edits are made, the following tags should be modified accordingly.    */
/* BINDTOOL_GEN_AUTOMATIC(0)                                                       */
/* BINDTOOL_USE_PYGCCXML(0)                                                        */
/* BINDTOOL_HEADER_FILE(short_burst_shaper.h)                                      */
/* BINDTOOL_HEADER_FILE_HASH(87b2cc82729151f831353a1de4b8a6ab)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gfdm/short_burst_shaper.h>
// pydoc.h is automatically generated in the build directory
#include <short_burst_shaper_pydoc.h>

void bind_short_burst_shaper(py::module& m)
{

    using short_burst_shaper = ::gr::gfdm::short_burst_shaper;


    py::class_<short_burst_shaper,
               gr::tagged_stream_block,
               gr::block,
               gr::basic_block,
               std::shared_ptr<short_burst_shaper>>(
        m, "short_burst_shaper", D(short_burst_shaper))

        .def(py::init(&short_burst_shaper::make),
             py::arg("pre_padding"),
             py::arg("post_padding"),
             py::arg("scale"),
             py::arg("nports") = 1,
             py::arg("length_tag_name") = "packet_len",
             py::arg("use_timed_commands") = false,
             py::arg("timing_advance") = 0.001,
             py::arg("cycle_interval") = 2.5000000000000001E-4,
             py::arg("enable_dsp_latency_reporting") = false,
             D(short_burst_shaper, make))


        .def("scale", &short_burst_shaper::scale, D(short_burst_shaper, scale))


        .def("set_scale",
             &short_burst_shaper::set_scale,
             py::arg("scale"),
             D(short_burst_shaper, set_scale))


        .def("timing_advance",
             &short_burst_shaper::timing_advance,
             D(short_burst_shaper, timing_advance))


        .def("set_timing_advance",
             &short_burst_shaper::set_timing_advance,
             py::arg("timing_advance"),
             D(short_burst_shaper, set_timing_advance))


        .def("cycle_interval",
             &short_burst_shaper::cycle_interval,
             D(short_burst_shaper, cycle_interval))


        .def("set_cycle_interval",
             &short_burst_shaper::set_cycle_interval,
             py::arg("cycle_interval"),
             D(short_burst_shaper, set_cycle_interval))

        ;
}
