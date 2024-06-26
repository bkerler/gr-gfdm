/* -*- c++ -*- */
/*
 * Copyright 2016 Johannes Demel.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <fmt/core.h>
#include <gfdm/modulator_kernel_cc.h>
#include <string.h>
#include <volk/volk.h>
#include <iostream>


namespace gr {
namespace gfdm {

modulator_kernel_cc::modulator_kernel_cc(int timeslots,
                                         int subcarriers,
                                         int overlap,
                                         std::vector<gfdm_complex> frequency_taps)
    : d_n_timeslots(timeslots),
      d_n_subcarriers(subcarriers),
      d_ifft_len(timeslots * subcarriers),
      d_overlap(overlap)
{
    if (int(frequency_taps.size()) != timeslots * overlap) {
        auto err_str = fmt::format("number of frequency taps({}) MUST be equal to "
                                   "timeslots({}) * overlap({}) = {}!",
                                   frequency_taps.size(),
                                   timeslots,
                                   overlap,
                                   timeslots * overlap);
        throw std::invalid_argument(err_str.c_str());
    }

    d_filter_taps.resize(timeslots * overlap);
    initialize_taps_vector(d_filter_taps.data(), frequency_taps, timeslots);

    // first create input and output buffers for a new FFTW plan.
    d_sub_fft_in.resize(timeslots);
    d_sub_fft_out.resize(timeslots);
    d_sub_fft_plan =
        initialize_fft(d_sub_fft_out.data(), d_sub_fft_in.data(), timeslots, true);

    d_filtered.resize(timeslots);

    d_ifft_in.resize(d_ifft_len);
    d_ifft_out.resize(d_ifft_len);
    d_ifft_plan = initialize_fft(
        d_ifft_out.data(), d_ifft_in.data(), timeslots * subcarriers, false);
}

modulator_kernel_cc::~modulator_kernel_cc()
{
    fftwf_destroy_plan(d_sub_fft_plan);
    fftwf_destroy_plan(d_ifft_plan);
}

void modulator_kernel_cc::initialize_taps_vector(gfdm_complex* filter_taps,
                                                 std::vector<gfdm_complex> frequency_taps,
                                                 const int n_timeslots)
{
    gfdm_complex res = gfdm_complex(0.0, 0.0);
    volk_32fc_x2_conjugate_dot_prod_32fc(
        &res, &frequency_taps[0], &frequency_taps[0], frequency_taps.size());
    // std::cout << "BEFORE energy of taps: " << std::abs(res) << std::endl;

    const gfdm_complex scaling_factor =
        gfdm_complex(1. / std::sqrt(std::abs(res) / n_timeslots), 0.0f);
    volk_32fc_s32fc_multiply_32fc(d_filter_taps.data(),
                                  frequency_taps.data(),
                                  scaling_factor,
                                  frequency_taps.size());

    volk_32fc_x2_conjugate_dot_prod_32fc(
        &res, d_filter_taps.data(), d_filter_taps.data(), frequency_taps.size());
    // std::cout << "AFTER  energy of taps: " << std::abs(res) << std::endl;
}

std::vector<modulator_kernel_cc::gfdm_complex> modulator_kernel_cc::filter_taps()
{
    return std::vector<gfdm_complex>(d_filter_taps.begin(), d_filter_taps.end());
}


void modulator_kernel_cc::generic_work(gfdm_complex* p_out, const gfdm_complex* p_in)
{
    // assume data symbols are stacked subcarrier-wise in 'in'.
    const int part_len = std::min(d_n_timeslots * d_overlap / 2, d_n_timeslots);

    // make sure we don't sum up old results.
    memset(d_ifft_in.data(), 0x00, sizeof(gfdm_complex) * d_ifft_len);

    // perform modulation for each subcarrier separately
    for (int k = 0; k < d_n_subcarriers; ++k) {
        // get items into subcarrier FFT
        memcpy(d_sub_fft_in.data(), p_in, sizeof(gfdm_complex) * d_n_timeslots);
        fftwf_execute(d_sub_fft_plan);

        // handle each part separately. The length of a part should always be
        // d_n_timeslots.
        // FIXME: Assumption and algorithm will probably fail for d_overlap = 1 (Should
        // never be used though).
        for (int i = 0; i < d_overlap; ++i) {
            // calculate positions for next part to handle
            int src_part_pos = ((i + d_overlap / 2) % d_overlap) * d_n_timeslots;
            int target_part_pos =
                ((k + i + d_n_subcarriers - (d_overlap / 2)) % d_n_subcarriers) *
                d_n_timeslots;
            // perform filtering operation!
            volk_32fc_x2_multiply_32fc(d_filtered.data(),
                                       d_sub_fft_out.data(),
                                       d_filter_taps.data() + src_part_pos,
                                       d_n_timeslots);
            // add generated part at correct position.
            volk_32f_x2_add_32f((float*)(d_ifft_in.data() + target_part_pos),
                                (float*)(d_ifft_in.data() + target_part_pos),
                                (float*)d_filtered.data(),
                                2 * part_len);
        }
        p_in += d_n_timeslots;
    }

    // Back to time domain!
    fftwf_execute(d_ifft_plan);
    //      memcpy(p_out, d_ifft_out, sizeof(gfdm_complex) * d_ifft_len);
    volk_32fc_s32fc_multiply_32fc(
        p_out, d_ifft_out.data(), gfdm_complex(1.0 / d_ifft_len, 0), d_ifft_len);
}

const void modulator_kernel_cc::print_vector(const gfdm_complex* v, const int size)
{
    for (int i = 0; i < size; ++i) {
        std::cout << v[i] << ", ";
        if (i % 8 == 0 && i > 0) {
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
}

} /* namespace gfdm */
} /* namespace gr */
