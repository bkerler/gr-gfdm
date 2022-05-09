/* -*- c++ -*- */
/*
 * Copyright 2017 Johannes Demel.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "channel_estimator_cc_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
namespace gfdm {

channel_estimator_cc::sptr channel_estimator_cc::make(int timeslots,
                                                      int subcarriers,
                                                      int active_subcarriers,
                                                      bool is_dc_free,
                                                      int which_estimator,
                                                      std::vector<gr_complex> preamble,
                                                      const std::string& snr_tag_key,
                                                      const std::string& cnr_tag_key)
{
    return gnuradio::make_block_sptr<channel_estimator_cc_impl>(timeslots,
                                                                subcarriers,
                                                                active_subcarriers,
                                                                is_dc_free,
                                                                which_estimator,
                                                                preamble,
                                                                snr_tag_key,
                                                                cnr_tag_key);
}

/*
 * The private constructor
 */
channel_estimator_cc_impl::channel_estimator_cc_impl(int timeslots,
                                                     int subcarriers,
                                                     int active_subcarriers,
                                                     bool is_dc_free,
                                                     int which_estimator,
                                                     std::vector<gr_complex> preamble,
                                                     const std::string& snr_tag_key,
                                                     const std::string& cnr_tag_key)
    : gr::block("channel_estimator_cc",
                gr::io_signature::make(1, 1, sizeof(gr_complex)),
                gr::io_signature::make(1, 1, sizeof(gr_complex))),
      d_snr_tag_key(pmt::intern(snr_tag_key)),
      d_cnr_vector_tag_key(pmt::intern(cnr_tag_key))
{
    d_estimator_kernel =
        std::make_unique<preamble_channel_estimator_cc>(timeslots,
                                                        subcarriers,
                                                        active_subcarriers,
                                                        is_dc_free,
                                                        which_estimator,
                                                        preamble);

    // set block properties!
    set_relative_rate(timeslots / 2.0);
    set_fixed_rate(true);
    set_output_multiple(subcarriers * timeslots);
}

/*
 * Our virtual destructor.
 */
channel_estimator_cc_impl::~channel_estimator_cc_impl() {}

void channel_estimator_cc_impl::forecast(int noutput_items,
                                         gr_vector_int& ninput_items_required)
{
    for (int i = 0; i < ninput_items_required.size(); ++i) {
        ninput_items_required[i] = fixed_rate_noutput_to_ninput(noutput_items);
    }
}

int channel_estimator_cc_impl::fixed_rate_ninput_to_noutput(int ninput)
{
    return ninput * d_estimator_kernel->timeslots() / 2;
}

int channel_estimator_cc_impl::fixed_rate_noutput_to_ninput(int noutput)
{
    return 2 * noutput / d_estimator_kernel->timeslots();
}

int channel_estimator_cc_impl::general_work(int noutput_items,
                                            gr_vector_int& ninput_items,
                                            gr_vector_const_void_star& input_items,
                                            gr_vector_void_star& output_items)
{
    const gr_complex* in = (const gr_complex*)input_items[0];
    gr_complex* out = (gr_complex*)output_items[0];

    const int invec_len = 2 * d_estimator_kernel->fft_len();
    const int frame_len = d_estimator_kernel->frame_len();
    const int n_frames = noutput_items / frame_len;

    for (int i = 0; i < n_frames; ++i) {
        // fmt::print("channel_estimator in_len={}\tframe_len={}\n{}\n",
        //            invec_len,
        //            frame_len,
        //            std::vector<gr_complex>(in, in + invec_len));
        d_estimator_kernel->estimate_frame(out, in);

        std::vector<float> cnrs(d_estimator_kernel->active_subcarriers());
        const float snr_lin = d_estimator_kernel->estimate_snr(cnrs, in);
        add_item_tag(0,
                     nitems_written(0) + i * frame_len,
                     d_snr_tag_key,
                     pmt::from_float(snr_lin));
        add_item_tag(0,
                     nitems_written(0) + i * frame_len,
                     d_cnr_vector_tag_key,
                     pmt::init_f32vector(cnrs.size(), cnrs));
        in += invec_len;
        out += frame_len;
    }

    consume_each(n_frames * invec_len);
    return n_frames * frame_len;
}

} /* namespace gfdm */
} /* namespace gr */
