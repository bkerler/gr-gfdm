/* -*- c++ -*- */
/*
 * Copyright 2018 - 2022 Johannes Demel.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#ifndef INCLUDED_GFDM_TRANSMITTER_KERNEL_H
#define INCLUDED_GFDM_TRANSMITTER_KERNEL_H

#include "gfdm_kernel_utils.h"
#include <gfdm/add_cyclic_prefix_cc.h>
#include <gfdm/api.h>
#include <gfdm/modulator_kernel_cc.h>
#include <gfdm/resource_mapper_kernel_cc.h>
#include <unordered_map>
#include <volk/volk_alloc.hh>
#include <memory>

namespace gr {
namespace gfdm {

/*!
 * \brief GFDM Transmitter kernel
 *
 * Perform all necessary operations to obtain a transmittable frame.
 *
 * This block encapsulates resource mapping, GFDM modulation, cyclic prefixing, and
 * preamble insertion.
 *
 * See
 * - gr::gfdm::resource_mapper_kernel_cc
 * - gr::gfdm::modulator_kernel_cc
 * - gr::gfdm::add_cyclic_prefix_cc
 *
 * \param timeslots Number of timeslots in a GFDM frame
 * \param subcarriers Number of subcarriers in a GFDM frame
 * \param active_subcarriers Number of occupied subcarriers smaller or equal to \p
 * subcarriers
 * \param cp_len Cyclic prefix length in samples
 * \param cs_len Cyclic suffix length in samples. Typically equal to \p rampl_len
 * \param ramp_len Number of samples used for block pinching.
 * \param subcarrier_map Indices of occupied subcarriers
 * \param per_timeslot Fill vector timeslot-wise or subcarrier-wise
 * \param overlap Steers modulation complexity and accuracy. 2 is usually sufficient.
 * \param frequency_taps Subcarrier filter taps in frequency domain
 * \param window_taps Taps used for block pinching. Typically RC taps cf. WiFi.
 * \param cyclic_shift Number of samples cyclic block shift for CDD
 * \param preambles Preamble vectors with cyclic shifts.
 */
class GFDM_API transmitter_kernel
{
public:
    typedef gr::gfdm::gfdm_kernel_utils::gfdm_complex gfdm_complex;

    transmitter_kernel(int timeslots,
                       int subcarriers,
                       int active_subcarriers,
                       int cp_len,
                       int cs_len,
                       int ramp_len,
                       std::vector<int> subcarrier_map,
                       bool per_timeslot,
                       int overlap,
                       std::vector<gfdm_complex> frequency_taps,
                       std::vector<gfdm_complex> window_taps,
                       std::vector<int> cyclic_shifts,
                       std::vector<std::vector<gfdm_complex>> preambles);
    ~transmitter_kernel();

    int input_vector_size() { return d_mapper->input_vector_size(); }
    int output_vector_size() { return d_prefixer->frame_size() + d_preamble_size; }
    void
    generic_work(gfdm_complex* p_out, const gfdm_complex* p_in, const int ninput_size);
    void modulate(gfdm_complex* out, const gfdm_complex* in, const int ninput_size);
    void add_frame(gfdm_complex* out, const gfdm_complex* in, const int cyclic_shift);
    const std::vector<int>& cyclic_shifts() const { return d_cyclic_shifts; };

    void set_pilots(const std::vector<std::tuple<unsigned, unsigned, gr_complex>> pilots)
    {
        d_mapper->set_pilots(pilots);
    }

    std::vector<std::tuple<unsigned, unsigned, gr_complex>> pilots() const
    {
        return d_mapper->pilots();
    }

private:
    std::unique_ptr<resource_mapper_kernel_cc> d_mapper;
    std::unique_ptr<modulator_kernel_cc> d_modulator;
    std::unique_ptr<add_cyclic_prefix_cc> d_prefixer;

    std::vector<int> d_cyclic_shifts;

    volk::vector<gfdm_complex> d_mapped;
    volk::vector<gfdm_complex> d_frame;
    const unsigned d_preamble_size;
    std::unordered_map<int, volk::vector<gfdm_complex>> d_preambles;
    void insert_preamble(gfdm_complex* out, const int cyclic_shift);
};

} // namespace gfdm
} // namespace gr

#endif /* INCLUDED_GFDM_TRANSMITTER_KERNEL_H */
