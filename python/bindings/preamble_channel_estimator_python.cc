/*
 * Copyright 2020 Johannes Demel
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
/* BINDTOOL_HEADER_FILE(preamble_channel_estimator_cc.h)                           */
/* BINDTOOL_HEADER_FILE_HASH(198f054f648075440a55edafcc17cb65)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <cstdint>

#include <gfdm/preamble_channel_estimator_cc.h>


namespace py = pybind11;


void bind_preamble_channel_estimator(py::module& m)
{
    using namespace gr::gfdm;
    py::class_<preamble_channel_estimator_cc>(m, "Preamble_channel_estimator")

        .def(py::init<int,
                      int,
                      int,
                      bool,
                      int,
                      std::vector<preamble_channel_estimator_cc::gfdm_complex>>(),
             py::arg("timeslots"),
             py::arg("subcarriers"),
             py::arg("active_subcarriers"),
             py::arg("is_dc_free"),
             py::arg("which_estimator"),
             py::arg("preamble"))
        .def("timeslots", &preamble_channel_estimator_cc::timeslots)
        .def("subcarriers", &preamble_channel_estimator_cc::fft_len)
        .def("active_subcarriers", &preamble_channel_estimator_cc::active_subcarriers)
        .def("frame_len", &preamble_channel_estimator_cc::frame_len)
        .def("is_dc_free", &preamble_channel_estimator_cc::is_dc_free)
        .def("estimate_frame",
             [](preamble_channel_estimator_cc& self,
                const py::array_t<preamble_channel_estimator_cc::gfdm_complex,
                                  py::array::c_style | py::array::forcecast> array) {
                 py::buffer_info inb = array.request();
                 if (inb.ndim != 1) {
                     throw std::runtime_error("Only ONE-dimensional vectors allowed!");
                 }
                 if (inb.size != 2 * self.fft_len()) {
                     throw std::runtime_error("Input vector size(" +
                                              std::to_string(inb.size) +
                                              ") MUST be equal to 2 * subcarriers(" +
                                              std::to_string(2 * self.fft_len()) + ")!");
                 }
                 auto result = py::array_t<preamble_channel_estimator_cc::gfdm_complex>(
                     self.frame_len());
                 py::buffer_info resb = result.request();

                 self.estimate_frame(
                     (preamble_channel_estimator_cc::gfdm_complex*)resb.ptr,
                     (preamble_channel_estimator_cc::gfdm_complex*)inb.ptr);
                 return result;
             })
        .def("estimate_snr",
             [](preamble_channel_estimator_cc& self,
                const py::array_t<preamble_channel_estimator_cc::gfdm_complex,
                                  py::array::c_style | py::array::forcecast> array) {
                 py::buffer_info inb = array.request();
                 if (inb.ndim != 1) {
                     throw std::runtime_error("Only ONE-dimensional vectors allowed!");
                 }
                 if (inb.size != 2 * self.fft_len()) {
                     throw std::runtime_error("Input vector size(" +
                                              std::to_string(inb.size) +
                                              ") MUST be equal to 2 * subcarriers(" +
                                              std::to_string(2 * self.fft_len()) + ")!");
                 }
                 auto result = py::array_t<preamble_channel_estimator_cc::gfdm_complex>(
                     self.frame_len());
                 py::buffer_info resb = result.request();

                 std::vector<float> cnrs(self.active_subcarriers());
                 const float snr_lin = self.estimate_snr(
                     cnrs, (preamble_channel_estimator_cc::gfdm_complex*)inb.ptr);

                 return snr_lin;
             });
}
