/* -*- c++ -*- */
/*
 * Copyright 2017, 2019 - 2022 Johannes Demel.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#ifndef INCLUDED_GFDM_GFDM_KERNEL_UTILS_H
#define INCLUDED_GFDM_GFDM_KERNEL_UTILS_H

#include <fftw3.h>
#include <complex>
#include <stdexcept>
#include <vector>

namespace gr {
namespace gfdm {

/*!
 * \brief Useful functions outside the public API
 *
 */
class gfdm_kernel_utils
{
public:
    typedef std::complex<float> gfdm_complex;

    gfdm_kernel_utils();
    ~gfdm_kernel_utils();

    fftwf_plan initialize_fft(gfdm_complex* out_buf,
                              gfdm_complex* in_buf,
                              const int fft_size,
                              bool forward);
    float calculate_signal_energy(const gfdm_complex* p_in, const int ninput_size);

private:
};

} // namespace gfdm
} // namespace gr

#endif /* INCLUDED_GFDM_GFDM_KERNEL_UTILS_H */
