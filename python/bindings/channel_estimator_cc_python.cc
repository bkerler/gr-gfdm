/*
 * Copyright 2020 Free Software Foundation, Inc.
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
/* BINDTOOL_HEADER_FILE(channel_estimator_cc.h)                                    */
/* BINDTOOL_HEADER_FILE_HASH(3091b6dd0ff759736a7a41019d6417f0)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gfdm/channel_estimator_cc.h>
// pydoc.h is automatically generated in the build directory
#include <channel_estimator_cc_pydoc.h>

void bind_channel_estimator_cc(py::module& m)
{

    using channel_estimator_cc = ::gr::gfdm::channel_estimator_cc;


    py::class_<channel_estimator_cc,
               gr::block,
               gr::basic_block,
               std::shared_ptr<channel_estimator_cc>>(
        m, "channel_estimator_cc", D(channel_estimator_cc))

        .def(py::init(&channel_estimator_cc::make),
             py::arg("timeslots"),
             py::arg("fft_len"),
             py::arg("active_subcarriers"),
             py::arg("is_dc_free"),
             py::arg("which_estimator"),
             py::arg("preamble"),
             py::arg("snr_tag_key") = "snr_lin",
             py::arg("cnr_tag_key") = "cnr",
             D(channel_estimator_cc, make))


        ;
}
