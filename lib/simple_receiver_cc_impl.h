/* -*- c++ -*- */
/*
 * Copyright 2016 Andrej Rode.
 * Copyright 2019, 2020, 2022 Johannes Demel.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_GFDM_SIMPLE_RECEIVER_CC_IMPL_H
#define INCLUDED_GFDM_SIMPLE_RECEIVER_CC_IMPL_H

#include <gfdm/receiver_kernel_cc.h>
#include <gfdm/simple_receiver_cc.h>

namespace gr {
namespace gfdm {

class simple_receiver_cc_impl : public simple_receiver_cc
{
private:
    std::unique_ptr<receiver_kernel_cc> d_kernel;

public:
    simple_receiver_cc_impl(int timeslots,
                            int subcarriers,
                            int overlap,
                            std::vector<gr_complex> frequency_taps);
    ~simple_receiver_cc_impl();

    // Where all the action really happens
    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items);
};

} // namespace gfdm
} // namespace gr

#endif /* INCLUDED_GFDM_SIMPLE_RECEIVER_CC_IMPL_H */
