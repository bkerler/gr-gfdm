/* -*- c++ -*- */
/*
 * Copyright 2017 Johannes Demel.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_GFDM_CHANNEL_ESTIMATOR_CC_IMPL_H
#define INCLUDED_GFDM_CHANNEL_ESTIMATOR_CC_IMPL_H

#include <gfdm/channel_estimator_cc.h>
#include <gfdm/preamble_channel_estimator_cc.h>

namespace gr {
namespace gfdm {

class channel_estimator_cc_impl : public channel_estimator_cc
{
private:
    std::unique_ptr<preamble_channel_estimator_cc> d_estimator_kernel;

    const pmt::pmt_t d_snr_tag_key;
    const pmt::pmt_t d_cnr_vector_tag_key;

public:
    channel_estimator_cc_impl(int timeslots,
                              int subcarriers,
                              int active_subcarriers,
                              bool is_dc_free,
                              int which_estimator,
                              std::vector<gr_complex> preamble,
                              const std::string& snr_tag_key = "snr_lin",
                              const std::string& cnr_tag_key = "cnr");
    ~channel_estimator_cc_impl();

    // Where all the action really happens
    void forecast(int noutput_items, gr_vector_int& ninput_items_required);
    int fixed_rate_ninput_to_noutput(int ninput);
    int fixed_rate_noutput_to_ninput(int noutput);

    int general_work(int noutput_items,
                     gr_vector_int& ninput_items,
                     gr_vector_const_void_star& input_items,
                     gr_vector_void_star& output_items);
};

} // namespace gfdm
} // namespace gr

#endif /* INCLUDED_GFDM_CHANNEL_ESTIMATOR_CC_IMPL_H */
