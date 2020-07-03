/* -*- c++ -*- */
/*
 * Copyright 2017 Johannes Demel.
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

#include "extract_burst_cc_impl.h"
#include <gnuradio/io_signature.h>
#include <volk/volk.h>
#include <cmath>

namespace gr {
namespace gfdm {

extract_burst_cc::sptr extract_burst_cc::make(int burst_len,
                                              int tag_backoff,
                                              std::string burst_start_tag,
                                              bool activate_cfo_correction)
{
    return gnuradio::get_initial_sptr(new extract_burst_cc_impl(
        burst_len, tag_backoff, burst_start_tag, activate_cfo_correction));
}

/*
 * The private constructor
 */
extract_burst_cc_impl::extract_burst_cc_impl(int burst_len,
                                             int tag_backoff,
                                             std::string burst_start_tag,
                                             bool activate_cfo_correction,
                                             const unsigned num_inputs)
    : gr::block("extract_burst_cc",
                gr::io_signature::make(num_inputs, num_inputs, sizeof(gr_complex)),
                gr::io_signature::make(num_inputs, num_inputs, sizeof(gr_complex))),
      d_burst_len(burst_len),
      d_tag_backoff(tag_backoff),
      d_burst_start_tag(pmt::mp(burst_start_tag)),
      d_src_id(pmt::mp(name())),
      d_activate_cfo_correction(activate_cfo_correction),
      d_num_ports(num_inputs)
{
    set_output_multiple(burst_len);
    set_tag_propagation_policy(TPP_DONT);
}

/*
 * Our virtual destructor.
 */
extract_burst_cc_impl::~extract_burst_cc_impl() {}

void extract_burst_cc_impl::forecast(int noutput_items,
                                     gr_vector_int& ninput_items_required)
{
    for (auto& nreq : ninput_items_required) {
        nreq = noutput_items;
    }
    // ninput_items_required[0] = noutput_items;
}

float extract_burst_cc_impl::get_scale_factor(const pmt::pmt_t& info) const
{
    float scale_factor = 1.0f;
    if (pmt::is_dict(info)) {
        scale_factor = pmt::to_double(
            pmt::dict_ref(info, d_scale_factor_key, pmt::from_double(1.0)));
    }
    return scale_factor;
}

gr_complex extract_burst_cc_impl::get_phase_rotation(const pmt::pmt_t& info) const
{
    const auto phase_rotation = pmt::to_complex(pmt::dict_ref(
        info, d_phase_rotation_key, pmt::from_complex(gr_complex(1.0f, 0.0f))));
    const auto scale = 1.0 / std::abs(phase_rotation);
    return gr_complex(scale * phase_rotation.real(),
                      -1.0f * scale * phase_rotation.imag());
}

void extract_burst_cc_impl::normalize_power_level(gr_complex* p_out,
                                                  const gr_complex* p_in,
                                                  const float norm_factor,
                                                  const int ninput_size)
{
    volk_32f_s32f_multiply_32f(
        (float*)p_out, (const float*)p_in, norm_factor, 2 * ninput_size);
}

void extract_burst_cc_impl::activate_cfo_compensation(bool activate_cfo_compensation)
{
    std::cout << "activate_cfo_compensation="
              << (activate_cfo_compensation ? "True" : "False") << std::endl;
    d_activate_cfo_correction = activate_cfo_compensation;
}

void extract_burst_cc_impl::compensate_cfo(gr_complex* p_out,
                                           const gr_complex* p_in,
                                           const gr_complex phase_increment,
                                           const int ninput_size)
{
    gr_complex initial_phase = gr_complex(1.0f, 0.0f);
    volk_32fc_s32fc_x2_rotator_32fc(
        p_out, p_in, phase_increment, &initial_phase, ninput_size);
}

void extract_burst_cc_impl::get_tags_all_streams(
    std::vector<std::vector<gr::tag_t>>& stream_input_tags,
    const unsigned available_items)
{
    for (unsigned i = 0; i < d_num_ports; ++i) {
        std::vector<gr::tag_t>& tags = stream_input_tags.at(i);
        get_tags_in_window(tags, i, 0, available_items, d_burst_start_tag);
        if (tags.size() > 0) {
            std::sort(tags.begin(), tags.begin(), tag_t::offset_compare);
        }
        // stream_input_tags.push_back(tags);
    }
}

void extract_burst_cc_impl::produce_frame_items(gr_complex* out,
                                                const gr_complex* in,
                                                const pmt::pmt_t& tag_value,
                                                const int actual_start)
{
    const float scale_factor = get_scale_factor(tag_value);
    const int num_prepend_zeros = (actual_start < 0) ? std::abs(actual_start) : 0;
    memset(out, 0, sizeof(gr_complex) * num_prepend_zeros);
    normalize_power_level(out + num_prepend_zeros,
                          in + actual_start - num_prepend_zeros,
                          scale_factor,
                          d_burst_len - num_prepend_zeros);

    if (d_activate_cfo_correction) {
        compensate_cfo(out, out, get_phase_rotation(tag_value), d_burst_len);
    }
}

uint64_t extract_burst_cc_impl::get_uint64_from_dict(const pmt::pmt_t& dict,
                                                     const pmt::pmt_t& key) const
{
    return pmt::to_uint64(pmt::dict_ref(dict, key, pmt::from_uint64(0)));
}

uint64_t extract_burst_cc_impl::get_stream_tag_offset(const std::vector<gr::tag_t>& tags,
                                                      const size_t position) const
{
    return tags.size() > position ? tags[position].offset
                                  : std::numeric_limits<uint64_t>::max();
}


int extract_burst_cc_impl::general_work(int noutput_items,
                                        gr_vector_int& ninput_items,
                                        gr_vector_const_void_star& input_items,
                                        gr_vector_void_star& output_items)
{
    const int n_out_bursts = noutput_items / d_burst_len;
    const int avail_items = *std::min_element(ninput_items.begin(), ninput_items.end());
    std::vector<std::vector<gr::tag_t>> stream_tags(d_num_ports);
    get_tags_all_streams(stream_tags, avail_items);

    bool tags_found = false;
    for (const auto& s : stream_tags) {
        tags_found = not s.empty();
    }
    if (not tags_found) {
        consume_each(avail_items);
        return 0;
    }

    int consumed_items = avail_items;
    int produced_items = 0;

    std::vector<size_t> positions(d_num_ports, 0);
    std::vector<uint64_t> offsets(d_num_ports, 0);
    while (produced_items < noutput_items - d_burst_len) {
        for (size_t i = 0; i < d_num_ports; ++i) {
            offsets[i] = stream_tags[i].size() > positions[i]
                             ? stream_tags[i][positions[i]].offset
                             : std::numeric_limits<uint64_t>::max();
        }
        const auto [mino, maxo] = std::minmax_element(offsets.begin(), offsets.end());

        if (*maxo - *mino < d_max_tag_offset_difference &&
            avail_items - (*maxo - nitems_read(0)) >= d_burst_len) {
            for (size_t i = 0; i < d_num_ports; ++i) {
                const gr_complex* in = (const gr_complex*)input_items[i];
                gr_complex* out = (gr_complex*)output_items[i];
                const auto& tag = stream_tags[i][positions[i]];
                const int burst_start = tag.offset - nitems_read(0);
                const int actual_start = burst_start - d_tag_backoff;
                produce_frame_items(out + produced_items, in, tag.value, actual_start);
                auto value = pmt::dict_add(tag.value,
                                           pmt::intern("burst_idx"),
                                           pmt::from_uint64(d_frame_counter));
                add_item_tag(i,
                             nitems_written(i) + produced_items,
                             d_burst_start_tag,
                             value,
                             d_src_id);
                positions[i]++;
            }
            d_frame_counter++;
            produced_items += d_burst_len;
            consumed_items = *maxo - nitems_read(0) + d_burst_len;
        }
        else{
            consumed_items = *mino - nitems_read(0);
            break;
        }


        bool more_tags = false;
        for (size_t i = 0; i < d_num_ports; ++i) {
            more_tags = stream_tags[i].size() < positions[i];
        }
        if (not more_tags) {
            break;
        }
    }
    consume_each(consumed_items);
    return produced_items;

    // const gr_complex* in = (const gr_complex*)input_items[0];
    // gr_complex* out = (gr_complex*)output_items[0];

    // // int consumed_items = avail_items;
    // // int produced_items = 0;

    // std::vector<tag_t> tags;
    // get_tags_in_window(tags, 0, 0, avail_items, d_burst_start_tag);

    // const int n_max_bursts = std::min(int(tags.size()), n_out_bursts);

    // for (int i = 0; i < n_max_bursts; ++i) {
    //     const auto& tag = tags[i];
    //     const int burst_start = tag.offset - nitems_read(0);
    //     const int actual_start = burst_start - d_tag_backoff;

    //     const pmt::pmt_t& info = tag.value;
    //     const uint64_t xcorr_idx = get_uint64_from_dict(tag.value, pmt::mp("xcorr_idx"));
    //     const uint64_t xcorr_offset =
    //         get_uint64_from_dict(tag.value, pmt::mp("xcorr_offset"));

    //     if (avail_items - burst_start >= d_burst_len) {
    //         produce_frame_items(out, in, tag.value, actual_start);

    //         auto value = pmt::dict_add(
    //             info, pmt::intern("burst_idx"), pmt::from_uint64(d_frame_counter));
    //         add_item_tag(0,
    //                      nitems_written(0) + produced_items,
    //                      d_burst_start_tag,
    //                      value,
    //                      pmt::string_to_symbol(name()));

    //         d_last_xcorr_offset = xcorr_offset;
    //         d_last_xcorr_idx = xcorr_idx;

    //         d_frame_counter++;
    //         produced_items += d_burst_len;
    //         consumed_items = burst_start + d_burst_len;
    //         out += d_burst_len;
    //     } else {

    //         d_expected_xcorr_idx = xcorr_idx;

    //         consumed_items = std::max(0, actual_start);
    //         break;
    //     }
    // }

    // consume_each(consumed_items);
    // return produced_items;
}

} /* namespace gfdm */
} /* namespace gr */
