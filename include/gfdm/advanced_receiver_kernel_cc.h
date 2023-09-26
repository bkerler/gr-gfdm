/* -*- c++ -*- */
/*
 * Copyright 2017 - 2022 Johannes Demel.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#ifndef INCLUDED_GFDM_ADVANCED_RECEIVER_KERNEL_CC_H
#define INCLUDED_GFDM_ADVANCED_RECEIVER_KERNEL_CC_H

#include <gnuradio/digital/constellation.h>
#include <gfdm/api.h>
#include <gfdm/receiver_kernel_cc.h>
#include <volk/volk_alloc.hh>

namespace gr {
namespace gfdm {

/*!
 * \brief Advanced GFDM demodulator kernel
 * \ingroup gfdm
 *
 * The advanced GFDM demodulator kernel holds all functions and buffers that are required
 * to demodulate with Interference Cancellation (IC). Computational complexity is mostly
 * steered with \p overlap, \p ic_iter, and \p do_phase_compensation. This kernel uses the
 * gr::gfdm::receiver_kernel_cc internally.
 *
 * \param timeslots Number of timeslots in a GFDM frame
 * \param subcarriers Number of subcarriers in a GFDM frame. occupied an unoccupied.
 * \param overlap Overlap factor. 2 is sufficient in most cases.
 * \param frequency_taps Filter taps in the frequency domain
 * \param subcarrier_map vector with indices of used subcarriers
 * \param ic_iter Number of IC iterations. 2 iterations is sufficient.
 * \param constellation GR constellation object. Usually PSK/QAM
 * \param do_phase_compensation Estimate and equalize residual phase error in first IC
 * iteration.
 *
 */
class GFDM_API advanced_receiver_kernel_cc
{
public:
    advanced_receiver_kernel_cc(int timeslots,
                                int subcarriers,
                                int overlap,
                                std::vector<gr_complex> frequency_taps,
                                std::vector<int> subcarrier_map,
                                int ic_iter,
                                gr::digital::constellation_sptr constellation,
                                int do_phase_compensation);
    ~advanced_receiver_kernel_cc() = default;

    void generic_work(gr_complex* p_out, const gr_complex* p_in);
    void generic_work_equalize(gr_complex* out,
                               const gr_complex* in,
                               const gr_complex* f_eq_in);
    void set_ic(int ic_iter) { d_ic_iter = ic_iter; }
    int get_ic(void) { return d_ic_iter; }
    int block_size() { return d_kernel->block_size(); }
    void set_phase_compensation(int do_phase_compensation)
    {
        d_do_phase_compensation = do_phase_compensation;
    }
    int get_phase_compensation() { return d_do_phase_compensation; }

    bool activate_pilot_estimation(bool activate)
    {
        d_active_pilot_estimation = activate;
        return d_active_pilot_estimation;
    }

    void set_pilots(const std::vector<std::tuple<unsigned, unsigned, gr_complex>> pilots)
    {
        for (const auto& pilot : pilots) {
            auto [sidx, tidx, ref] = pilot;
            fmt::print("{}\tsidx={}, tidx={}, ref={}\n", pilot, sidx, tidx, ref);
        }
        d_pilot_reference = pilots;
    }

    std::vector<std::tuple<unsigned, unsigned, gr_complex>> pilots() const
    {
        return d_pilot_reference;
    }

private:
    bool d_active_pilot_estimation = false;
    std::vector<std::tuple<unsigned, unsigned, gr_complex>> d_pilot_reference;
    std::unique_ptr<receiver_kernel_cc> d_kernel;
    std::vector<int> d_subcarrier_map;
    int d_ic_iter;
    gr::digital::constellation_sptr d_constellation;
    int d_do_phase_compensation;

    void map_symbols_to_constellation_points(gr_complex* p_out, const gr_complex* p_in);
    void perform_ic_iterations(gr_complex* p_out, gr_complex* p_freq_block);
    float calculate_phase_offset(const gr_complex* detected_symbols_buffer,
                                 const gr_complex* demod_symbols_buffer);

    gr_complex estimate_pilot_distortion(const gr_complex* p_in);

    volk::vector<gr_complex> d_freq_block;
    volk::vector<gr_complex> d_ic_time_buffer;
    volk::vector<gr_complex> d_ic_freq_buffer;
};

} // namespace gfdm
} // namespace gr

#endif /* INCLUDED_GFDM_ADVANCED_RECEIVER_KERNEL_CC_H */
