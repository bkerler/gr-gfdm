/* -*- c++ -*- */
/*
 * Copyright 2019 Johannes Demel.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "short_burst_shaper_impl.h"
#include <gnuradio/io_signature.h>
#include <fmt/core.h>
#include <volk/volk.h>
#include <chrono>
#include <cstring>
#include <exception>

namespace gr {
namespace gfdm {

short_burst_shaper::sptr short_burst_shaper::make(int pre_padding,
                                                  int post_padding,
                                                  gr_complex scale,
                                                  const unsigned nports,
                                                  const std::string& length_tag_name,
                                                  bool use_timed_commands,
                                                  double timing_advance,
                                                  double cycle_interval)
{
    return gnuradio::make_block_sptr<short_burst_shaper_impl>(pre_padding,
                                                              post_padding,
                                                              scale,
                                                              nports,
                                                              length_tag_name,
                                                              use_timed_commands,
                                                              timing_advance,
                                                              cycle_interval);
}

/*
 * The private constructor
 */
short_burst_shaper_impl::short_burst_shaper_impl(int pre_padding,
                                                 int post_padding,
                                                 gr_complex scale,
                                                 const unsigned nports,
                                                 const std::string& length_tag_name,
                                                 bool use_timed_commands,
                                                 double timing_advance,
                                                 double cycle_interval,
                                                 bool enable_dsp_latency_reporting)
    : gr::tagged_stream_block("short_burst_shaper",
                              gr::io_signature::make(nports, nports, sizeof(gr_complex)),
                              gr::io_signature::make(nports, nports, sizeof(gr_complex)),
                              length_tag_name),
      d_pre_padding(pre_padding),
      d_post_padding(post_padding),
      d_scale(scale),
      d_use_timed_commands(use_timed_commands),
      d_enable_dsp_latency_reporting(enable_dsp_latency_reporting),
      d_timing_advance(timing_advance),
      d_timing_advance_ticks(double2ticks(timing_advance)),
      d_cycle_interval(cycle_interval),
      d_cycle_interval_ticks(double2ticks(cycle_interval)),
      d_src_id_key(pmt::mp(symbol_name()))
{
    if (d_pre_padding < 0) {
        throw std::invalid_argument(
            fmt::format("Pre-padding length MUST be >= 0!, but is: {}", d_pre_padding));
    }
    if (d_post_padding < 0) {
        throw std::invalid_argument(
            fmt::format("Post-padding length MUST be >= 0!, but is: {}", d_post_padding));
    }

    GR_LOG_DEBUG(d_logger,
                 fmt::format("cycle interval: {}, ticks: {}",
                             d_cycle_interval,
                             d_cycle_interval_ticks));

    enable_update_rate(true);

    message_port_register_out(MSG_OUT_PORT);

    message_port_register_in(MSG_IN_PORT);
    set_msg_handler(MSG_IN_PORT, [this](pmt::pmt_t msg) { this->handle_msg(msg); });
}

/*
 * Our virtual destructor.
 */
short_burst_shaper_impl::~short_burst_shaper_impl() {}

int short_burst_shaper_impl::calculate_output_stream_length(
    const gr_vector_int& ninput_items)
{
    int noutput_items = ninput_items[0] + d_pre_padding + d_post_padding;
    return noutput_items;
}

void short_burst_shaper_impl::send_rx_gain_command(const uint64_t full_secs,
                                                   const double frac_secs,
                                                   const double gain)
{
    auto msg = pmt::dict_add(
        pmt::make_dict(),
        d_command_time_key,
        pmt::cons(pmt::from_uint64(full_secs), pmt::from_double(frac_secs)));
    msg = pmt::dict_add(msg, d_command_gain_key, pmt::from_double(gain));

    message_port_pub(MSG_OUT_PORT, msg);
}

void short_burst_shaper_impl::send_rx_gain_commands(const int packet_len)
{
    double frac_secs = d_frac_secs - 1.0e-4;
    uint64_t full_secs = d_full_secs;
    if (frac_secs < 0.0) {
        full_secs -= 1;
        frac_secs += 1.0;
    }
    send_rx_gain_command(full_secs, frac_secs, 0.0);

    frac_secs = d_frac_secs + 1.0e-4 + packet_len / d_samp_rate;
    full_secs = d_full_secs;
    if (frac_secs >= 1.0) {
        full_secs += 1;
        frac_secs -= 1.0;
    }
    send_rx_gain_command(full_secs, frac_secs, 65.0);
}

void short_burst_shaper_impl::handle_msg(pmt::pmt_t time_msg)
{
    if (pmt::is_dict(time_msg) &&
        (pmt::equal(pmt::dict_ref(time_msg, pmt::mp("src"), pmt::from_long(-1)),
                    pmt::from_long(0)))) {
        d_rx_time =
            pmt::to_long(pmt::dict_ref(time_msg, d_rx_time_key, pmt::from_long(0)));

    } else if (d_use_timed_commands && pmt::is_tuple(time_msg)) {
        d_full_secs = pmt::to_uint64(pmt::tuple_ref(time_msg, 0));
        d_frac_secs = pmt::to_double(pmt::tuple_ref(time_msg, 1));
        d_time_ticks = timespec2ticks(d_full_secs, d_frac_secs);
        d_tag_offset = pmt::to_uint64(pmt::tuple_ref(time_msg, 2));
        d_samp_rate = pmt::to_double(pmt::tuple_ref(time_msg, 3));
        d_slot_counter = pmt::to_uint64(pmt::tuple_ref(time_msg, 4));

        d_has_new_time_tag = true;
    }
}

int short_burst_shaper_impl::work(int noutput_items,
                                  gr_vector_int& ninput_items,
                                  gr_vector_const_void_star& input_items,
                                  gr_vector_void_star& output_items)
{
    const gr_complex* in = (const gr_complex*)input_items[0];
    gr_complex* out = (gr_complex*)output_items[0];

    // const auto nexpected_input_items = ninput_items[0];
    for (unsigned port = 0; port < input_items.size(); ++port) {
        const gr_complex* in = (const gr_complex*)input_items[port];
        gr_complex* out = (gr_complex*)output_items[port];

        std::fill(out, out + d_pre_padding, gr_complex(0.0f, 0.0f));

        volk_32fc_s32fc_multiply_32fc(
            out + d_pre_padding, in, d_scale, ninput_items[port]);

        std::fill(out + d_pre_padding + ninput_items[port],
                  out + d_pre_padding + ninput_items[port] + d_post_padding,
                  gr_complex(0.0f, 0.0f));
    }

    if (d_use_timed_commands) {

        // uint64_t fts = pc_clock_ticks();
        // uint64_t ticks = fts;
        uint64_t fts = d_time_ticks;

        fts -= fts % d_cycle_interval_ticks;
        fts += d_cycle_interval_ticks;
        while (fts <= d_last_tx_ns) {
            fts += d_cycle_interval_ticks;
        }
        fts += d_rx_time % d_cycle_interval_ticks;
        d_last_tx_ns = fts;

        fts += d_timing_advance_ticks;

        uint64_t full_secs = ticks2fullsecs(fts);
        double frac_secs = ticks2fracsecs(fts);

        if (d_enable_dsp_latency_reporting) {
            uint64_t now = pc_clock_ticks();
            std::vector<gr::tag_t> tags;
            get_tags_in_range(tags,
                              0,
                              nitems_read(0),
                              nitems_read(0) + ninput_items[0],
                              d_dsp_time_key);
            for (const auto& tag : tags) {
                uint64_t time = pmt::to_uint64(tag.value);
                uint64_t dsp_latency_ticks = now - time;
                const float dsp_latency_ms = 1.0e-6 * dsp_latency_ticks;

                GR_LOG_DEBUG(d_logger,
                             fmt::format("TX DSP latency:\tticks={}\t{:.4f}ms",
                                         dsp_latency_ticks,
                                         dsp_latency_ms));
            }
        }

        for (unsigned port = 0; port < input_items.size(); ++port) {
            add_item_tag(
                port,
                nitems_written(port),
                d_tx_time_key,
                pmt::make_tuple(pmt::from_uint64(full_secs), pmt::from_double(frac_secs)),
                d_src_id_key);
        }

        d_has_new_time_tag = false;
        d_last_full_secs = full_secs;
        d_last_frac_secs = frac_secs;
    }

    // Tell runtime system how many output items we produced.
    return ninput_items[0] + d_pre_padding + d_post_padding;
}

} /* namespace gfdm */
} /* namespace gr */
