/* -*- c++ -*- */
/*
 * Copyright 2016 Johannes Demel.
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

#include "resource_demapper_cc_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
namespace gfdm {

resource_demapper_cc::sptr resource_demapper_cc::make(int timeslots,
                                                      int subcarriers,
                                                      int active_subcarriers,
                                                      std::vector<int> subcarrier_map,
                                                      bool per_timeslot)
{
    return gnuradio::make_block_sptr<resource_demapper_cc_impl>(
        timeslots, subcarriers, active_subcarriers, subcarrier_map, per_timeslot);
}

/*
 * The private constructor
 */
resource_demapper_cc_impl::resource_demapper_cc_impl(int timeslots,
                                                     int subcarriers,
                                                     int active_subcarriers,
                                                     std::vector<int> subcarrier_map,
                                                     bool per_timeslot)
    : gr::block("resource_demapper_cc",
                gr::io_signature::make(1, 1, sizeof(gr_complex)),
                gr::io_signature::make(1, 1, sizeof(gr_complex))),
      d_kernel(std::make_unique<resource_mapper_kernel_cc>(timeslots,
                                                           subcarriers,
                                                           active_subcarriers,
                                                           subcarrier_map,
                                                           per_timeslot,
                                                           false))
{
    set_relative_rate(1.0 * d_kernel->output_vector_size() /
                      d_kernel->input_vector_size());
    set_fixed_rate(true);
    set_output_multiple(d_kernel->output_vector_size());
}

/*
 * Our virtual destructor.
 */
resource_demapper_cc_impl::~resource_demapper_cc_impl() {}

void resource_demapper_cc_impl::forecast(int noutput_items,
                                         gr_vector_int& ninput_items_required)
{
    ninput_items_required[0] = fixed_rate_noutput_to_ninput(noutput_items);
}

int resource_demapper_cc_impl::fixed_rate_ninput_to_noutput(int ninput)
{
    return (ninput / d_kernel->input_vector_size()) * d_kernel->output_vector_size();
}

int resource_demapper_cc_impl::fixed_rate_noutput_to_ninput(int noutput)
{
    return (noutput / d_kernel->output_vector_size()) * d_kernel->input_vector_size();
}

int resource_demapper_cc_impl::general_work(int noutput_items,
                                            gr_vector_int& ninput_items,
                                            gr_vector_const_void_star& input_items,
                                            gr_vector_void_star& output_items)
{
    const gr_complex* in = (const gr_complex*)input_items[0];
    gr_complex* out = (gr_complex*)output_items[0];

    int n_frames = std::min(noutput_items / d_kernel->output_vector_size(),
                            ninput_items[0] / d_kernel->input_vector_size());
    for (int i = 0; i < n_frames; ++i) {
        d_kernel->demap_from_resources(out, in, d_kernel->output_vector_size());
        out += d_kernel->output_vector_size();
        in += d_kernel->input_vector_size();
    }

    // std::vector<tag_t> tags;
    // const uint64_t nsamps = n_frames * d_kernel->input_vector_size();
    // get_tags_in_window(tags, 0, 0, nsamps);
    // std::string tagstr;
    // for (const auto& tag : tags) {
    //     auto keystr = pmt::write_string(tag.key);
    //     if (keystr.find("cnr") != std::string::npos) {
    //         continue;
    //     }
    //     auto meta = fmt::format("id={}, key={:16}", identifier(), keystr);
    //     if (keystr.find("snr_lin") != std::string::npos) {
    //         auto snr_lin = pmt::to_double(tag.value);
    //         auto snr_db = 10.0 * std::log10(snr_lin);
    //         meta += fmt::format("value={:8.2f}\t\t\t{:5.2f}dB\n", snr_lin, snr_db);
    //     } else if (keystr.find("frame_start") != std::string::npos) {
    //         auto rx_timestamp = pmt::to_uint64(
    //             pmt::dict_ref(tag.value, pmt::mp("rx_time"), pmt::from_uint64(0)));
    //         auto time = std::chrono::nanoseconds(rx_timestamp);
    //         auto hours = std::chrono::duration_cast<std::chrono::hours>(time);
    //         auto minutes = std::chrono::duration_cast<std::chrono::minutes>(time -
    //         hours); auto seconds =
    //             std::chrono::duration_cast<std::chrono::seconds>(time - hours -
    //             minutes);
    //         auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
    //             time - hours - minutes - seconds);
    //         auto micros = std::chrono::duration_cast<std::chrono::microseconds>(
    //             time - hours - minutes - seconds - millis);
    //         auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
    //             time - hours - minutes - seconds - millis - micros);
    //         // auto timestr = fmt::format("{:02}:{:02}:{:02}.{:03},{:03},{:03}",
    //         //                            hours.count() % 24,
    //         //                            minutes.count(),
    //         //                            seconds.count(),
    //         //                            millis.count(),
    //         //                            micros.count(),
    //         //                            nanos.count());
    //         auto timestr = fmt::format("{:02}.{:03},{:03},{:03}",
    //                                    seconds.count(),
    //                                    millis.count(),
    //                                    micros.count(),
    //                                    nanos.count());
    //         auto sc_offset = pmt::to_uint64(pmt::dict_ref(tag.value,
    //         pmt::mp("sc_offset"), pmt::from_uint64(0))); auto xc_offset =
    //         pmt::to_uint64(pmt::dict_ref(tag.value, pmt::mp("xcorr_offset"),
    //         pmt::from_uint64(0))); auto diff_offset = int64_t(sc_offset) -
    //         int64_t(xc_offset); auto sc_rot = pmt::dict_ref(tag.value,
    //         pmt::mp("sc_rot"), pmt::from_complex(gr_complex(1.0, 0.0))); auto phase =
    //         std::arg(pmt::to_complex(sc_rot)); meta += fmt::format(
    //             "value={:22}{:.6f}rad {}\tsc={}\txc={}, delta={}\n",
    //             pmt::write_string(sc_rot), phase, timestr, sc_offset, xc_offset,
    //             diff_offset);
    //     } else if (pmt::is_dict(tag.value)) {
    //         auto keys = pmt::dict_keys(tag.value);
    //         for (unsigned i = 0; i < pmt::length(keys); i++) {
    //             auto dk = pmt::nth(i, keys);
    //             auto dv = pmt::dict_ref(tag.value, dk, pmt::get_PMT_F());
    //             meta += fmt::format(
    //                 "\nkey={:<18}value={}", pmt::write_string(dk),
    //                 pmt::write_string(dv));
    //         }
    //         meta += "\n";
    //     } else {
    //         meta += fmt::format(", value={}\n", pmt::write_string(tag.value));
    //     }
    //     tagstr += meta;
    //     // fmt::print(meta);
    // }
    // if (tagstr.size() > 0) {
    //     fmt::print(tagstr);
    // }


    consume_each(n_frames * d_kernel->input_vector_size());

    // Tell runtime system how many output items we produced.
    return n_frames * d_kernel->output_vector_size();
}

} /* namespace gfdm */
} /* namespace gr */
