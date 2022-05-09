/* -*- c++ -*- */
/*
 * Copyright 2017 Johannes Demel.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gfdm/preamble_channel_estimator_cc.h>
#include <volk/volk.h>
#include <cmath>
#include <cstring>
#include <iostream>

namespace gr {
namespace gfdm {

preamble_channel_estimator_cc::preamble_channel_estimator_cc(
    int timeslots,
    int subcarriers,
    int active_subcarriers,
    bool is_dc_free,
    int which_estimator,
    std::vector<gfdm_complex> preamble)
    : d_timeslots(timeslots),
      d_fft_len(subcarriers),
      d_active_subcarriers(active_subcarriers),
      d_is_dc_free(is_dc_free),
      d_which_estimator(which_estimator),
      d_n_gaussian_taps(9)
{
    d_preamble_fft_in.resize(subcarriers);
    d_preamble_fft_out.resize(subcarriers);
    d_preamble_fft_plan = initialize_fft(
        d_preamble_fft_out.data(), d_preamble_fft_in.data(), subcarriers, true);

    d_snr_fft_in.resize(2 * subcarriers);
    d_snr_fft_out.resize(2 * subcarriers);
    d_snr_fft_plan =
        initialize_fft(d_snr_fft_out.data(), d_snr_fft_in.data(), 2 * subcarriers, true);

    d_inv_freq_preamble0.resize(subcarriers);
    d_inv_freq_preamble1.resize(subcarriers);
    initialize_inv_freq_preamble(d_inv_freq_preamble0.data(), &preamble[0]);
    initialize_inv_freq_preamble(d_inv_freq_preamble1.data(), &preamble[subcarriers]);

    d_intermediate_channel_estimate.resize(subcarriers);

    d_gaussian_taps.resize(d_n_gaussian_taps);
    initialize_gaussian_filter(d_gaussian_taps.data(), 1.0f, d_n_gaussian_taps);

    int intermediate_filter_len =
        active_subcarriers + d_n_gaussian_taps + (is_dc_free ? 1 : 0);
    d_filter_intermediate.resize(intermediate_filter_len);
    d_preamble_estimate.resize(subcarriers);
    d_filtered_estimate.resize(active_subcarriers + (is_dc_free ? 1 : 0));

    d_one_reference.resize(timeslots * subcarriers);
    for (int i = 0; i < frame_len(); ++i) {
        d_one_reference[i] = gfdm_complex(1.0f, 0.0f);
    }
}

preamble_channel_estimator_cc::~preamble_channel_estimator_cc()
{
    fftwf_destroy_plan(d_preamble_fft_plan);
    fftwf_destroy_plan(d_snr_fft_plan);
}

void preamble_channel_estimator_cc::initialize_gaussian_filter(float* taps,
                                                               const float sigma_sq,
                                                               const int n_taps)
{
    float s = 0.0f;
    for (int i = 0; i < n_taps; ++i) {
        float val = std::pow(float(i - (n_taps / 2)), 2.0f) / sigma_sq;
        taps[i] = std::exp(-0.5f * val);
        s += taps[i];
    }

    for (int i = 0; i < n_taps; ++i) {
        taps[i] = taps[i] / s;
    }
}

std::vector<float> preamble_channel_estimator_cc::preamble_filter_taps()
{
    std::vector<float> t = std::vector<float>(d_n_gaussian_taps);
    for (int i = 0; i < d_n_gaussian_taps; ++i) {
        t[i] = d_gaussian_taps[i];
    }
    return t;
}

void preamble_channel_estimator_cc::initialize_inv_freq_preamble(
    gfdm_complex* p_out, const gfdm_complex* p_preamble_part)
{
    memcpy(d_preamble_fft_in.data(), p_preamble_part, sizeof(gfdm_complex) * d_fft_len);
    fftwf_execute(d_preamble_fft_plan);
    for (int i = 0; i < d_fft_len; ++i) {
        p_out[i] = gfdm_complex(0.5, 0.0) / d_preamble_fft_out[i];
    }
}

void preamble_channel_estimator_cc::estimate_preamble_channel(
    gfdm_complex* fd_preamble_channel, const gfdm_complex* rx_preamble)
{
    estimate_fftlen_preamble_channel(
        d_intermediate_channel_estimate.data(), rx_preamble, d_inv_freq_preamble0.data());
    estimate_fftlen_preamble_channel(
        fd_preamble_channel, rx_preamble + d_fft_len, d_inv_freq_preamble1.data());
    volk_32f_x2_add_32f((float*)fd_preamble_channel,
                        (float*)fd_preamble_channel,
                        (float*)d_intermediate_channel_estimate.data(),
                        2 * d_fft_len);
}

void preamble_channel_estimator_cc::estimate_fftlen_preamble_channel(
    gfdm_complex* p_out,
    const gfdm_complex* rx_samples,
    const gfdm_complex* fd_ref_samples)
{
    memcpy(d_preamble_fft_in.data(), rx_samples, sizeof(gfdm_complex) * d_fft_len);
    fftwf_execute(d_preamble_fft_plan);
    volk_32fc_x2_multiply_32fc(
        p_out, d_preamble_fft_out.data(), fd_ref_samples, d_fft_len);
}

void preamble_channel_estimator_cc::filter_preamble_estimate(gfdm_complex* filtered,
                                                             const gfdm_complex* estimate)
{
    const auto left_fill_element = estimate[d_fft_len - d_active_subcarriers / 2];
    std::fill(d_filter_intermediate.begin(),
              d_filter_intermediate.begin() + d_n_gaussian_taps / 2,
              left_fill_element);

    for (int i = 0; i < d_active_subcarriers / 2; ++i) {
        d_filter_intermediate[i + d_n_gaussian_taps / 2] =
            estimate[i + d_fft_len - d_active_subcarriers / 2];
    }

    int offset = 0;
    if (d_is_dc_free) {
        d_filter_intermediate[d_n_gaussian_taps / 2 + d_active_subcarriers / 2] =
            (estimate[d_fft_len - 1] + estimate[1]) / gfdm_complex(2.0f, 0.0f);
        offset = 1;
    }

    for (int i = 0; i < d_active_subcarriers / 2; ++i) {
        d_filter_intermediate[i + offset + d_n_gaussian_taps / 2 +
                              d_active_subcarriers / 2] = estimate[offset + i];
    }

    for (int i = d_active_subcarriers / 2;
         i < d_active_subcarriers / 2 + d_n_gaussian_taps / 2;
         ++i) {
        d_filter_intermediate[i + offset + d_n_gaussian_taps / 2 +
                              d_active_subcarriers / 2] =
            estimate[offset + d_active_subcarriers / 2 - 1];
    }

    int n_taps = d_active_subcarriers + offset;
    for (int i = 0; i < n_taps; ++i) {
        volk_32fc_32f_dot_prod_32fc(filtered + i,
                                    d_filter_intermediate.data() + i,
                                    d_gaussian_taps.data(),
                                    d_n_gaussian_taps);
    }
}

float preamble_channel_estimator_cc::estimate_snr(std::vector<float>& cnrs,
                                                  const gfdm_complex* rx_preamble)
{
    cnrs.resize(d_active_subcarriers);
    memcpy(d_snr_fft_in.data(), rx_preamble, sizeof(gfdm_complex) * 2 * d_fft_len);
    fftwf_execute(d_snr_fft_plan);
    float symbol_energy = 0.0f;
    float noise_energy = 0.0f;

    const unsigned active_half = d_active_subcarriers / 2;
    const unsigned offset = d_is_dc_free ? 1 : 0;
    for (unsigned i = 0; i < active_half; ++i) {
        const unsigned pos = 2 * (i + offset);
        const auto se = std::norm(d_snr_fft_out[pos]);
        const auto ne = std::norm(d_snr_fft_out[pos + 1]);

        cnrs[i] = se;

        // std::cout << pos << "\t" << se << ",\t" << ne << std::endl;
        symbol_energy += se;
        noise_energy += ne;
    }

    const unsigned unused_half = (d_fft_len - d_active_subcarriers) / 2;
    const unsigned low_offset = unused_half + d_fft_len / 2;
    for (unsigned i = 0; i < active_half; ++i) {
        const unsigned pos = 2 * (i + low_offset);
        const auto se = std::norm(d_snr_fft_out[pos]);
        const auto ne = std::norm(d_snr_fft_out[pos + 1]);
        // std::cout << pos << "\t" << se << ",\t" << ne << std::endl;

        cnrs[active_half + i] = se;
        symbol_energy += se;
        noise_energy += ne;
    }

    const float snr_lin = (symbol_energy - noise_energy) / noise_energy;
    const float snr_db = 10.0f * std::log10(snr_lin);
    const float scale = snr_lin / (symbol_energy / cnrs.size());
    volk_32f_s32f_multiply_32f(cnrs.data(), cnrs.data(), scale, cnrs.size());
    // std::cout << "SNR " << snr_lin
    //           << "lin,\t" << snr_db
    //           << "dB\n";
    // std::cout << "SNR: " << symbol_energy
    //           << ", noise: " << noise_energy
    //           << ", estimate: " << snr_lin <<
    //           " dB: " << snr_db << std::endl;
    return snr_lin;
}


void preamble_channel_estimator_cc::interpolate_frame(gfdm_complex* frame_estimate,
                                                      const gfdm_complex* estimate)
{
    const int n_estimated_taps = d_active_subcarriers + (d_is_dc_free ? 1 : 0);
    const int center = d_fft_len * d_timeslots / 2;
    const int dead_subcarriers = d_fft_len - d_active_subcarriers;
    const gfdm_complex step_size = gfdm_complex(1.0f / float(d_timeslots), 0.0f);

    for (int i = center; i < center + d_timeslots * dead_subcarriers / 2; ++i) {
        frame_estimate[i] = estimate[0];
    }

    for (int i = d_timeslots * d_active_subcarriers / 2 - d_timeslots; i < center; ++i) {
        frame_estimate[i] = estimate[n_estimated_taps - 1];
    }

    for (int i = 0; i < n_estimated_taps / 2; ++i) {
        gfdm_complex inc = (estimate[i + 1] - estimate[i]) * step_size;
        gfdm_complex factor = estimate[i];
        for (int j = 0; j < d_timeslots; ++j) {
            const int pos =
                center + d_timeslots * dead_subcarriers / 2 + i * d_timeslots + j;
            frame_estimate[pos] = factor;
            factor += inc;
        }
    }

    for (int i = n_estimated_taps / 2; i < n_estimated_taps - 1; ++i) {
        int offset = (i - n_estimated_taps / 2) * d_timeslots;
        gfdm_complex inc = (estimate[i + 1] - estimate[i]) * step_size;
        gfdm_complex factor = estimate[i];
        for (int j = 0; j < d_timeslots; ++j) {
            const int pos = offset + j;
            frame_estimate[pos] = factor;
            factor += inc;
        }
    }
}

void preamble_channel_estimator_cc::prepare_for_zf(gfdm_complex* transformed_frame,
                                                   const gfdm_complex* frame_estimate)
{
    volk_32fc_x2_divide_32fc(
        transformed_frame, d_one_reference.data(), frame_estimate, frame_len());
    volk_32fc_conjugate_32fc(transformed_frame, transformed_frame, frame_len());
}


void preamble_channel_estimator_cc::estimate_frame(gfdm_complex* frame_estimate,
                                                   const gfdm_complex* rx_preamble)
{
    estimate_preamble_channel(d_preamble_estimate.data(), rx_preamble);
    filter_preamble_estimate(d_filtered_estimate.data(), d_preamble_estimate.data());
    interpolate_frame(frame_estimate, d_filtered_estimate.data());

    // switch(d_which_estimator){
    //   case 1: prepare_for_zf(frame_estimate, frame_estimate); break;
    // }
}

} /* namespace gfdm */
} /* namespace gr */
