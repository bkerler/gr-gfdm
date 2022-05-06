/* -*- c++ -*- */
/*
 * Copyright 2016 Andrej Rode
 * Copyright 2016, 2017, 2019 - 2022 Johannes Demel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_GFDM_RECEIVER_KERNEL_CC_H
#define INCLUDED_GFDM_RECEIVER_KERNEL_CC_H

#include "gfdm_kernel_utils.h"
#include <fftw3.h>
#include <volk/volk_alloc.hh>
#include <complex>
#include <stdexcept>
#include <vector>

namespace gr {
namespace gfdm {

/*!
 * \brief GFDM demodulator kernel
 *  This class initializes and performs all operations necessary to demodulate a GFDM
 * block.
 *
 * \details
 * The GFDM receiver kernel class provides all necessary operations to blocks which
 * instantiate it. Further functions and methods not depending on GNU Radio should be
 * implemented here. This receiver implementation is based on [Gas+13]. It is recommended
 * to use \p overlap = 2 to make use of sparse frequency domain processing.
 *
 * [Gas+13] I.S. Gaspar et al. "Low Complexity GFDM Receiver Based on Sparse Frequency
 * Domain Processing"
 *
 * See gr::gfdm::modulator_kernel_cc
 *
 * \param n_timeslots Number of timeslots in a GFDM frame
 * \param n_subcarriers Number of subcarriers in a GFDM frame
 * \param overlap Steers modulation complexity and accuracy. 2 is usually sufficient.
 * \param frequency_taps Subcarrier filter taps in frequency domain
 *
 */
class __attribute__((visibility("default"))) receiver_kernel_cc : public gfdm_kernel_utils
{
public:
    receiver_kernel_cc(int n_timeslots,
                       int n_subcarriers,
                       int overlap,
                       std::vector<gfdm_complex> frequency_taps);
    ~receiver_kernel_cc();

    void generic_work(gfdm_complex* out, const gfdm_complex* in);
    void generic_work_equalize(gfdm_complex* out,
                               const gfdm_complex* in,
                               const gfdm_complex* f_eq_in);
    void fft_filter_downsample(gfdm_complex* p_out, const gfdm_complex* p_in);
    void fft_equalize_filter_downsample(gfdm_complex* p_out,
                                        const gfdm_complex* p_in,
                                        const gfdm_complex* f_eq_in);
    void transform_subcarriers_to_td(gfdm_complex* p_out, const gfdm_complex* p_in);
    void cancel_sc_interference(gfdm_complex* p_out,
                                const gfdm_complex* p_td_in,
                                const gfdm_complex* p_fd_in);

    void filter_superposition(std::vector<std::vector<gfdm_complex>>& out,
                              const gfdm_complex* in);
    void demodulate_subcarrier(std::vector<std::vector<gfdm_complex>>& out,
                               std::vector<std::vector<gfdm_complex>>& sc_fdomain);
    void serialize_output(gfdm_complex out[],
                          std::vector<std::vector<gfdm_complex>>& sc_symbols);
    void vectorize_2d(std::vector<std::vector<gfdm_complex>>& out_vector,
                      const gfdm_complex* p_in);
    void remove_sc_interference(std::vector<std::vector<gfdm_complex>>& sc_symbols,
                                std::vector<std::vector<gfdm_complex>>& sc_fdomain);
    int block_size() const { return d_block_len; }
    std::vector<gfdm_complex> filter_taps() const;
    std::vector<gfdm_complex> ic_filter_taps() const;
    int timeslots() const { return d_n_timeslots; };
    int subcarriers() const { return d_n_subcarriers; };
    int overlap() const { return d_overlap; };

private:
    int d_n_subcarriers;
    int d_n_timeslots;
    int d_block_len;
    int d_overlap;
    volk::vector<gfdm_complex> d_filter_taps;
    volk::vector<gfdm_complex> d_ic_filter_taps;

    void initialize_taps_vector(gfdm_complex* filter_taps,
                                std::vector<gfdm_complex> frequency_taps,
                                const int n_timeslots);

    fftwf_plan d_in_fft_plan;
    volk::vector<gfdm_complex> d_in_fft_in;
    volk::vector<gfdm_complex> d_in_fft_out;

    volk::vector<gfdm_complex> d_equalized;

    fftwf_plan d_sc_ifft_plan;
    volk::vector<gfdm_complex> d_sc_ifft_in;
    volk::vector<gfdm_complex> d_sc_ifft_out;

    fftwf_plan d_sc_fft_plan;
    volk::vector<gfdm_complex> d_sc_fft_in;
    volk::vector<gfdm_complex> d_sc_fft_out;

    volk::vector<gfdm_complex> d_sc_postfilter;
    volk::vector<gfdm_complex> d_sc_filtered;

    void filter_subcarriers_and_downsample_fd(gfdm_complex* p_out,
                                              const gfdm_complex* p_in);
};
} /* namespace gfdm */
} /* namespace gr */

#endif /* INCLUDED_GFDM_RECEIVER_KERNEL_CC_H */
