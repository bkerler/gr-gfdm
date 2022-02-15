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
/* BINDTOOL_HEADER_FILE(extract_burst_cc.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(8c18a0416d00ccac96c54f013cd16636)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gfdm/extract_burst_cc.h>
// pydoc.h is automatically generated in the build directory
#include <extract_burst_cc_pydoc.h>

void bind_extract_burst_cc(py::module& m)
{

    using extract_burst_cc = ::gr::gfdm::extract_burst_cc;


    py::class_<extract_burst_cc,
               gr::block,
               gr::basic_block,
               std::shared_ptr<extract_burst_cc>>(
        m, "extract_burst_cc", D(extract_burst_cc))

        .def(py::init(&extract_burst_cc::make),
             py::arg("burst_len"),
             py::arg("tag_backoff"),
             py::arg("burst_start_tag"),
             py::arg("activate_cfo_correction") = false,
             py::arg("forward_burst_start_tag") = "",
             D(extract_burst_cc, make))


        .def("activate_cfo_compensation",
             &extract_burst_cc::activate_cfo_compensation,
             py::arg("activate_cfo_compensation"),
             D(extract_burst_cc, activate_cfo_compensation))


        .def("cfo_compensation",
             &extract_burst_cc::cfo_compensation,
             D(extract_burst_cc, cfo_compensation))


        .def("set_fixed_phase_increment",
             &extract_burst_cc::set_fixed_phase_increment,
             py::arg("phase_increment"),
             py::arg("activate"),
             D(extract_burst_cc, set_fixed_phase_increment))

        ;
}
