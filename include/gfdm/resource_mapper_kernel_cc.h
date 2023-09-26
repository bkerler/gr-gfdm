/* -*- c++ -*- */
/*
 * Copyright 2016 Johannes Demel.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#ifndef INCLUDED_GFDM_RESOURCE_MAPPER_KERNEL_CC_H
#define INCLUDED_GFDM_RESOURCE_MAPPER_KERNEL_CC_H

#include <complex>
#include <vector>

namespace gr {
namespace gfdm {

/*!
 * \brief Kernel to map and demap complex samples to/from a GFDM frame.
 * \ingroup gfdm
 *
 * Map complex information symbols to GFDM resource grid.
 * Input is a vector with all complex information symbols for one GFDM frame.
 * Result is a vector which is fed to gr::gfdm::modulator_kernel_cc
 * In demapper mode, the gr::gfdm::receiver_kernel_cc emits the expected vector.
 *
 * \param timeslots Number of timeslots in a GFDM frame
 * \param subcarriers Number of subcarriers in a GFDM frame
 * \param active_subcarriers Number of occupied subcarriers. Smaller/equal \p subcarriers
 * \param subcarrier_map Indices of occupied subcarriers \param per_timeslot
 * Fill vector timeslot-wise or subcarrier-wise \param is_mapper switch between mapping
 * and demapping operation
 */
class __attribute__((visibility("default"))) resource_mapper_kernel_cc
{
public:
    typedef std::complex<float> gfdm_complex;

    resource_mapper_kernel_cc(int timeslots,
                              int subcarriers,
                              int active_subcarriers,
                              std::vector<int> subcarrier_map,
                              bool per_timeslot = true,
                              bool is_mapper = true);
    ~resource_mapper_kernel_cc() = default;
    size_t frame_size() { return d_frame_size; }
    size_t block_size() { return d_per_timeslot_indices.size(); }
    size_t input_vector_size() { return d_is_mapper ? block_size() : frame_size(); };
    size_t output_vector_size() { return d_is_mapper ? frame_size() : block_size(); };
    void map_to_resources(gfdm_complex* p_out,
                          const gfdm_complex* p_in,
                          const size_t ninput_size);
    void demap_from_resources(gfdm_complex* p_out,
                              const gfdm_complex* p_in,
                              const size_t noutput_size);

    void
    set_pilots(const std::vector<std::tuple<unsigned, unsigned, gfdm_complex>> pilots)
    {
        set_pilots_checked(pilots);
    }

    std::vector<std::tuple<unsigned, unsigned, gfdm_complex>> pilots() const
    {
        return d_pilot_reference;
    }

private:
    std::vector<std::tuple<unsigned, unsigned, gfdm_complex>> d_pilot_reference = {};
    std::vector<unsigned> d_per_timeslot_indices = {};
    std::vector<unsigned> d_per_subcarrier_indices = {};
    const size_t d_timeslots;
    const size_t d_subcarriers;
    const size_t d_active_subcarriers;
    const size_t d_frame_size;
    std::vector<int> d_subcarrier_map;
    const bool d_per_timeslot;
    const bool d_is_mapper;

    void prepare_index_vectors();
    void set_pilots_checked(
        const std::vector<std::tuple<unsigned, unsigned, gfdm_complex>> pilots);

    void map_per_timeslot(gfdm_complex* p_out,
                          const gfdm_complex* p_in,
                          const int ninput_size);
    void map_per_subcarrier(gfdm_complex* p_out,
                            const gfdm_complex* p_in,
                            const int ninput_size);

    void demap_per_timeslot(gfdm_complex* p_out,
                            const gfdm_complex* p_in,
                            const int noutput_size);
    void demap_per_subcarrier(gfdm_complex* p_out,
                              const gfdm_complex* p_in,
                              const int noutput_size);
};

} // namespace gfdm
} // namespace gr

#endif /* INCLUDED_GFDM_RESOURCE_MAPPER_KERNEL_CC_H */
