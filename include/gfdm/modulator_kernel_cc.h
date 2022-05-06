/* -*- c++ -*- */
/*
 * Copyright 2016, 2019 - 2022 Johannes Demel.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#ifndef INCLUDED_GFDM_MODULATOR_KERNEL_CC_H
#define INCLUDED_GFDM_MODULATOR_KERNEL_CC_H

#include "gfdm_kernel_utils.h"
#include <fftw3.h>
#include <volk/volk_alloc.hh>
#include <complex>
#include <stdexcept>
#include <vector>

namespace gr {
namespace gfdm {

/*!
 * \brief GFDM modulator kernel
 * \ingroup gfdm
 *
 * This kernel holds the GFDM modulator. It takes a vector with occupied an unoccupied
 * subcarriers. The result is one GFDM frame. The \p frequency_taps should decay rapidly
 * outside the subcarrier bandwidth. In that case, \p overlap 2 yields very accurate
 * results and holds complexity at bay.
 *
 * \param n_timeslots Number of timeslots in a GFDM frame
 * \param n_subcarriers Number of subcarriers in a GFDM frame
 * \param overlap Steers modulation complexity and accuracy. 2 is usually sufficient.
 * \param frequency_taps Subcarrier filter taps in frequency domain
 */
class __attribute__((visibility("default"))) modulator_kernel_cc
    : public gfdm_kernel_utils
{
public:
    modulator_kernel_cc(int n_timeslots,
                        int n_subcarriers,
                        int overlap,
                        std::vector<gfdm_complex> frequency_taps);
    ~modulator_kernel_cc();
    void generic_work(gfdm_complex* p_out, const gfdm_complex* p_in);
    int block_size() { return d_n_subcarriers * d_n_timeslots; };
    std::vector<gfdm_complex> filter_taps();

private:
    int d_n_timeslots;
    int d_n_subcarriers;
    int d_ifft_len;
    int d_overlap;
    volk::vector<gfdm_complex> d_filter_taps;

    void initialize_taps_vector(gfdm_complex* filter_taps,
                                std::vector<gfdm_complex> frequency_taps,
                                const int n_timeslots);

    volk::vector<gfdm_complex> d_sub_fft_in;
    volk::vector<gfdm_complex> d_sub_fft_out;
    fftwf_plan d_sub_fft_plan;
    volk::vector<gfdm_complex> d_filtered;
    volk::vector<gfdm_complex> d_ifft_in;
    volk::vector<gfdm_complex> d_ifft_out;
    fftwf_plan d_ifft_plan;

    // DEBUG function
    const void print_vector(const gfdm_complex* v, const int size);
    static bool complex_compare(gfdm_complex i, gfdm_complex j)
    {
        return std::abs(i) < std::abs(j);
    }
};

} // namespace gfdm
} // namespace gr

#endif /* INCLUDED_GFDM_MODULATOR_KERNEL_CC_H */
