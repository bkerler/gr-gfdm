/* -*- c++ -*- */
/*
 * Copyright 2016, 2019, 2020, 2022 Johannes Demel.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_GFDM_SIMPLE_MODULATOR_CC_IMPL_H
#define INCLUDED_GFDM_SIMPLE_MODULATOR_CC_IMPL_H

#include <gfdm/modulator_kernel_cc.h>
#include <gfdm/simple_modulator_cc.h>

#include <memory>

namespace gr {
namespace gfdm {

class simple_modulator_cc_impl : public simple_modulator_cc
{
private:
    std::unique_ptr<modulator_kernel_cc> d_kernel;

public:
    simple_modulator_cc_impl(int timeslots,
                             int subcarriers,
                             int overlap,
                             std::vector<gr_complex> frequency_taps);
    ~simple_modulator_cc_impl();

    // Where all the action really happens
    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items);
};

} // namespace gfdm
} // namespace gr

#endif /* INCLUDED_GFDM_SIMPLE_MODULATOR_CC_IMPL_H */
