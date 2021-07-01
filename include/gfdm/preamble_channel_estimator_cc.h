/* -*- c++ -*- */
/*
 * Copyright 2017 Johannes Demel.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */


#ifndef INCLUDED_GFDM_PREAMBLE_CHANNEL_ESTIMATOR_CC_H
#define INCLUDED_GFDM_PREAMBLE_CHANNEL_ESTIMATOR_CC_H


#include <fftw3.h>
#include <volk/volk_alloc.hh>
#include <complex>
#include <stdexcept>
#include <vector>

#include "gfdm_kernel_utils.h"

namespace gr {
namespace gfdm {

/*!
 * \brief Estimate frequency domain channel based on preamble
 *
 * Block expects a received preamble to produce a channel estimate
 * for a frame as specified.
 *
 */
class __attribute__((visibility("default"))) preamble_channel_estimator_cc
    : public gfdm_kernel_utils
{
public:
    preamble_channel_estimator_cc(int timeslots,
                                  int fft_len,
                                  int active_subcarriers,
                                  bool is_dc_free,
                                  int which_estimator,
                                  std::vector<gfdm_complex> preamble);
    ~preamble_channel_estimator_cc();

    void estimate_preamble_channel(gfdm_complex* fd_preamble_channel,
                                   const gfdm_complex* rx_preamble);
    int fft_len() { return d_fft_len; };
    int timeslots() { return d_timeslots; };
    int frame_len() { return d_timeslots * d_fft_len; };
    int active_subcarriers() { return d_active_subcarriers; };
    bool is_dc_free() { return d_is_dc_free; };
    std::vector<float> preamble_filter_taps();

    void filter_preamble_estimate(gfdm_complex* filtered, const gfdm_complex* estimate);

    void interpolate_frame(gfdm_complex* frame_estimate, const gfdm_complex* estimate);

    void estimate_frame(gfdm_complex* frame_estimate, const gfdm_complex* rx_preamble);

    void prepare_for_zf(gfdm_complex* transformed_frame,
                        const gfdm_complex* frame_estimate);

    float estimate_snr(std::vector<float>& cnrs, const gfdm_complex* rx_preamble);

private:
    int d_timeslots;
    int d_fft_len;
    int d_active_subcarriers;
    bool d_is_dc_free;
    int d_which_estimator;

    volk::vector<gfdm_complex> d_preamble_fft_in;
    volk::vector<gfdm_complex> d_preamble_fft_out;
    fftwf_plan d_preamble_fft_plan;

    volk::vector<gfdm_complex> d_snr_fft_in;
    volk::vector<gfdm_complex> d_snr_fft_out;
    fftwf_plan d_snr_fft_plan;

    volk::vector<gfdm_complex> d_inv_freq_preamble0;
    volk::vector<gfdm_complex> d_inv_freq_preamble1;
    volk::vector<gfdm_complex> d_intermediate_channel_estimate;
    void initialize_inv_freq_preamble(gfdm_complex* p_out,
                                      const gfdm_complex* p_preamble_part);
    void estimate_fftlen_preamble_channel(gfdm_complex* p_out,
                                          const gfdm_complex* rx_samples,
                                          const gfdm_complex* fd_ref_samples);

    int d_n_gaussian_taps;
    volk::vector<float> d_gaussian_taps;
    void initialize_gaussian_filter(float* taps, const float sigma_sq, const int n_taps);

    volk::vector<gfdm_complex> d_filter_intermediate;
    volk::vector<gfdm_complex> d_preamble_estimate;
    volk::vector<gfdm_complex> d_filtered_estimate;
    volk::vector<gfdm_complex> d_one_reference;
};

} // namespace gfdm
} // namespace gr

#endif /* INCLUDED_GFDM_PREAMBLE_CHANNEL_ESTIMATOR_CC_H */
