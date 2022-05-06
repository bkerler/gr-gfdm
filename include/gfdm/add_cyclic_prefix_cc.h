/* -*- c++ -*- */
/*
 * Copyright 2016, 2019, 2020, 2021, 2022 Johannes Demel.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#ifndef INCLUDED_GFDM_ADD_CYCLIC_PREFIX_CC_H
#define INCLUDED_GFDM_ADD_CYCLIC_PREFIX_CC_H

#include <volk/volk_alloc.hh>
#include <complex>
#include <stdexcept>
#include <vector>

namespace gr {
namespace gfdm {

/*!
 * \brief Kernel adds cyclic prefix to GFDM frame and applies block pinching window.
 * \ingroup gfdm
 *
 * This block prepends \p cp_len samples from the back of a block to the front
 * and appends \p cs_len samples from the front to the back of a block.
 * The first and last \p ramp_len samples are multiplied with the \p ramp_len
 * first and last samples in \p window_taps. This measure helps to reduce OOB emissions.
 *
 * The \p cyclic_shift parameter helps with Cyclic Delay Diversity (CDD).
 *
 * \param block_len Number of modulated symbols
 * \param cp_len Cyclic prefix length in samples
 * \param cs_len Cyclic suffix length in samples. Typically equal to \p rampl_len
 * \param ramp_len Number of samples used for block pinching.
 * \param window_taps Taps used for block pinching. Typically RC taps cf. WiFi.
 * \param cyclic_shift Number of samples cyclic block shift for CDD
 *
 */
class __attribute__((visibility("default"))) add_cyclic_prefix_cc
{
public:
    typedef std::complex<float> gfdm_complex;

    add_cyclic_prefix_cc(int block_len,
                         int cp_len,
                         int cs_len,
                         int ramp_len,
                         std::vector<gfdm_complex> window_taps,
                         int cyclic_shift = 0);
    ~add_cyclic_prefix_cc();
    void generic_work(gfdm_complex* p_out, const gfdm_complex* p_in);
    void add_cyclic_prefix(gfdm_complex* p_out,
                           const gfdm_complex* p_in,
                           const int cyclic_prefix);
    void remove_cyclic_prefix(gfdm_complex* p_out, const gfdm_complex* p_in);
    int block_size() { return d_block_len; };
    int frame_size() { return block_size() + d_cp_len + d_cs_len; };
    int cyclic_shift() const { return d_cyclic_shift; };

private:
    const int d_block_len;
    const int d_cp_len;
    const int d_cs_len;
    const int d_ramp_len;
    const int d_cyclic_shift;

    void add_cyclic_extension(gfdm_complex* out,
                              const gfdm_complex* in,
                              const int cyclic_shift);

    volk::vector<gfdm_complex> d_front_ramp;
    volk::vector<gfdm_complex> d_back_ramp;
    void apply_ramp(gfdm_complex* out, const gfdm_complex* in);
};

} // namespace gfdm
} // namespace gr

#endif /* INCLUDED_GFDM_ADD_CYCLIC_PREFIX_CC_H */
