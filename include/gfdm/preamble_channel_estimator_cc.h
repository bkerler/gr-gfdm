/* -*- c++ -*- */
/*
 * Copyright 2017 - 2022 Johannes Demel.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
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
 * Estimate current channel realization based on a received preamble.
 * The output is a channel estimate that one may use directly
 * to equalize a GFDM frame in the frequency domain.
 *
 * \param timeslots Number of timeslots in a GFDM frame
 * \param fft_len Number of subcarriers in a GFDM frame. occupied an unoccupied.
 * \param active_subcarriers Number of occupied/used subcarriers.
 * \param is_dc_free true if the DC carrier is unoccupied.
 * \param which_estimator Deprecated. Expected to be 0.
 * \param preamble Known transmitted preamble.
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
