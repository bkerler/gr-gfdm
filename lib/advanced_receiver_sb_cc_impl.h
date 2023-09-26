/* -*- c++ -*- */
/*
 * Copyright 2016 Andrej Rode, Johannes Demel.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_GFDM_ADVANCED_RECEIVER_SB_CC_IMPL_H
#define INCLUDED_GFDM_ADVANCED_RECEIVER_SB_CC_IMPL_H

#include <fmt/core.h>
#include <fmt/ranges.h>
#include <gfdm/advanced_receiver_kernel_cc.h>
#include <gfdm/advanced_receiver_sb_cc.h>
#include <memory>

namespace gr {
namespace gfdm {

class advanced_receiver_sb_cc_impl : public advanced_receiver_sb_cc
{
private:
    std::unique_ptr<advanced_receiver_kernel_cc> d_adv_kernel;

public:
    advanced_receiver_sb_cc_impl(int timeslots,
                                 int subcarriers,
                                 int overlap,
                                 int ic_iter,
                                 std::vector<gr_complex> frequency_taps,
                                 gr::digital::constellation_sptr constellation,
                                 std::vector<int> subcarrier_map,
                                 int do_phase_compensation);

    ~advanced_receiver_sb_cc_impl();

    void set_phase_compensation(int do_phase_compensation)
    {
        d_adv_kernel->set_phase_compensation(do_phase_compensation);
    }
    int get_phase_compensation() { return d_adv_kernel->get_phase_compensation(); }

    bool activate_pilot_estimation(bool activate)
    {
        return d_adv_kernel->activate_pilot_estimation(activate);
    }

    void set_pilots(const std::vector<std::tuple<unsigned, unsigned, gr_complex>> pilots)
    {
        d_adv_kernel->set_pilots(pilots);
    }

    std::vector<std::tuple<unsigned, unsigned, gr_complex>> pilots() const
    {
        return d_adv_kernel->pilots();
    }

    void set_ic(int ic_iter) { d_adv_kernel->set_ic(ic_iter); }

    int get_ic(void) { return d_adv_kernel->get_ic(); }

    // Where all the action really happens
    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items);
};

} // namespace gfdm
} // namespace gr

#endif /* INCLUDED_GFDM_ADVANCED_RECEIVER_SB_CC_IMPL_H */
