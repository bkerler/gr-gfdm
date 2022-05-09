/* -*- c++ -*- */
/*
 * Copyright 2016, 2019 - 2022 Johannes Demel.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "simple_modulator_cc_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
namespace gfdm {

simple_modulator_cc::sptr simple_modulator_cc::make(
    int timeslots, int subcarriers, int overlap, std::vector<gr_complex> frequency_taps)
{
    return gnuradio::make_block_sptr<simple_modulator_cc_impl>(
        timeslots, subcarriers, overlap, frequency_taps);
}

/*
 * The private constructor
 */
simple_modulator_cc_impl::simple_modulator_cc_impl(int timeslots,
                                                   int subcarriers,
                                                   int overlap,
                                                   std::vector<gr_complex> frequency_taps)
    : gr::sync_block("simple_modulator_cc",
                     gr::io_signature::make(1, 1, sizeof(gr_complex)),
                     gr::io_signature::make(1, 1, sizeof(gr_complex))),
      d_kernel(std::make_unique<modulator_kernel_cc>(
          timeslots, subcarriers, overlap, frequency_taps))
{
    set_output_multiple(d_kernel->block_size());
}

/*
 * Our virtual destructor.
 */
simple_modulator_cc_impl::~simple_modulator_cc_impl() {}

int simple_modulator_cc_impl::work(int noutput_items,
                                   gr_vector_const_void_star& input_items,
                                   gr_vector_void_star& output_items)
{
    const gr_complex* in = (const gr_complex*)input_items[0];
    gr_complex* out = (gr_complex*)output_items[0];

    const int n_blocks = noutput_items / d_kernel->block_size();
    for (int i = 0; i < n_blocks; ++i) {
        d_kernel->generic_work(out, in);
        in += d_kernel->block_size();
        out += d_kernel->block_size();
    }

    // Tell runtime system how many output items we produced.
    return noutput_items;
}

} /* namespace gfdm */
} /* namespace gr */
