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


#include <fmt/core.h>
#include <fmt/ranges.h>
#include <gfdm/resource_mapper_kernel_cc.h>
#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace gr {
namespace gfdm {

resource_mapper_kernel_cc::resource_mapper_kernel_cc(int timeslots,
                                                     int subcarriers,
                                                     int active_subcarriers,
                                                     std::vector<int> subcarrier_map,
                                                     bool per_timeslot,
                                                     bool is_mapper)
    : d_timeslots(timeslots),
      d_subcarriers(subcarriers),
      d_active_subcarriers(active_subcarriers),
      d_frame_size(timeslots * subcarriers),
      d_per_timeslot(per_timeslot),
      d_is_mapper(is_mapper)
{
    if (active_subcarriers > subcarriers) {
        throw std::invalid_argument(fmt::format(
            "active_subcarriers({})) MUST be smaller or equal to subcarriers({})!",
            active_subcarriers,
            subcarriers));
    }
    if (int(subcarrier_map.size()) != active_subcarriers) {
        throw std::invalid_argument(
            fmt::format("number of subcarrier_map entries({}) MUST be equal to "
                        "active_subcarriers({})!",
                        subcarrier_map.size(),
                        active_subcarriers));
    }
    std::sort(subcarrier_map.begin(), subcarrier_map.end());
    if (!(adjacent_find(subcarrier_map.begin(), subcarrier_map.end()) ==
          subcarrier_map.end())) {
        throw std::invalid_argument("All entries in subcarrier_map MUST be unique!");
    }
    if (*std::min_element(subcarrier_map.begin(), subcarrier_map.end()) < 0) {
        throw std::invalid_argument(
            "All subcarrier indices MUST be greater or equal to ZERO!");
    }
    if (*std::max_element(subcarrier_map.begin(), subcarrier_map.end()) > subcarriers) {
        throw std::invalid_argument(
            "All subcarrier indices MUST be smaller or equal to subcarriers!");
    }
    d_subcarrier_map = subcarrier_map;
    prepare_index_vectors();
}

void resource_mapper_kernel_cc::set_pilots_checked(
    const std::vector<std::tuple<unsigned, unsigned, gfdm_complex>> pilots)
{

    for (const auto& pilot : pilots) {
        auto [sidx, tidx, value] = pilot;
        // fmt::print(
        //     "s={:3}\tt={:3}\t({:.2}{:+.2})\n", sidx, tidx, value.real(), value.imag());
        if (std::find(d_subcarrier_map.begin(), d_subcarrier_map.end(), sidx) ==
            d_subcarrier_map.end()) {
            throw std::invalid_argument(fmt::format(
                "Invalid subcarrier index ({}) not in active_subcarriers: {}!",
                sidx,
                d_subcarrier_map));
        }

        if (tidx < 0 or tidx >= d_timeslots) {
            throw std::invalid_argument(fmt::format(
                "Invalid timeslot index ({}) for range [0, {})!", tidx, d_timeslots));
        }
    }
    d_pilot_reference = pilots;
    prepare_index_vectors();
}

void resource_mapper_kernel_cc::prepare_index_vectors()
{
    // First for per_timeslot
    d_per_timeslot_indices.clear();
    for (int tidx = 0; tidx < d_timeslots; ++tidx) {
        for (const auto sidx : d_subcarrier_map) {
            d_per_timeslot_indices.push_back(d_timeslots * sidx + tidx);
        }
    }

    for (const auto& pilot : d_pilot_reference) {
        auto [sidx, tidx, value] = pilot;
        auto pilotidx = d_timeslots * sidx + tidx;
        auto it = std::remove(
            d_per_timeslot_indices.begin(), d_per_timeslot_indices.end(), pilotidx);
        d_per_timeslot_indices.erase(it);
    }

    // Second for per_subcarrier
    d_per_subcarrier_indices.clear();
    for (const auto sidx : d_subcarrier_map) {
        for (int tidx = 0; tidx < d_timeslots; ++tidx) {
            d_per_subcarrier_indices.push_back(d_timeslots * sidx + tidx);
        }
    }

    for (const auto& pilot : d_pilot_reference) {
        auto [sidx, tidx, value] = pilot;
        auto pilotidx = d_timeslots * sidx + tidx;
        auto it = std::remove(
            d_per_subcarrier_indices.begin(), d_per_subcarrier_indices.end(), pilotidx);
        d_per_subcarrier_indices.erase(it);
    }

    // fmt::print("per_timeslot   items per frame: {}\n", d_per_timeslot_indices.size());
    // fmt::print("per_subcarrier items per frame: {}\n",
    // d_per_subcarrier_indices.size());

    if (d_per_timeslot_indices.size() != d_per_subcarrier_indices.size()) {
        throw std::invalid_argument(fmt::format(
            "per_timeslot_indices.size({}) != per_subcarriers_indices.size({})",
            d_per_timeslot_indices.size(),
            d_per_subcarrier_indices.size()));
    }
}

void resource_mapper_kernel_cc::map_to_resources(gfdm_complex* p_out,
                                                 const gfdm_complex* p_in,
                                                 const size_t ninput_size)
{
    // fmt::print("ninput_size={}\tblock_size={}\n", ninput_size, block_size());
    if (ninput_size > block_size()) {
        throw std::invalid_argument(fmt::format(
            "input vector size({}) MUST not exceed active_subcarriers * timeslots({})!",
            ninput_size,
            block_size()));
    }
    std::fill(p_out, p_out + frame_size(), gfdm_complex(0.0f, 0.0f));
    // memset(p_out, 0x0, sizeof(gfdm_complex) * frame_size());
    if (d_per_timeslot) {
        map_per_timeslot(p_out, p_in, ninput_size);
    } else {
        map_per_subcarrier(p_out, p_in, ninput_size);
    }
}

void resource_mapper_kernel_cc::demap_from_resources(gfdm_complex* p_out,
                                                     const gfdm_complex* p_in,
                                                     const size_t noutput_size)
{
    if (noutput_size > block_size()) {
        throw std::invalid_argument(fmt::format(
            "output vector size({}) MUST not exceed active_subcarriers * timeslots({})!",
            noutput_size,
            block_size()));
    }
    memset(p_out, 0x0, sizeof(gfdm_complex) * noutput_size);
    if (d_per_timeslot) {
        demap_per_timeslot(p_out, p_in, noutput_size);
    } else {
        demap_per_subcarrier(p_out, p_in, noutput_size);
    }
}

void resource_mapper_kernel_cc::map_per_timeslot(gfdm_complex* p_out,
                                                 const gfdm_complex* p_in,
                                                 const int ninput_size)
{
    int sym_ctr = 0;
    for (const auto idx : d_per_timeslot_indices) {
        p_out[idx] = sym_ctr < ninput_size ? *p_in++ : gfdm_complex(0.0, 0.0);
        sym_ctr++;
    }

    for (const auto& pilot : d_pilot_reference) {
        auto [sidx, tidx, value] = pilot;
        auto pilotidx = d_timeslots * sidx + tidx;
        p_out[pilotidx] = value;
    }
}

void resource_mapper_kernel_cc::map_per_subcarrier(gfdm_complex* p_out,
                                                   const gfdm_complex* p_in,
                                                   const int ninput_size)
{
    int sym_ctr = 0;
    for (const auto idx : d_per_subcarrier_indices) {
        p_out[idx] = sym_ctr < ninput_size ? *p_in++ : gfdm_complex(0.0, 0.0);
        sym_ctr++;
    }
    for (const auto& pilot : d_pilot_reference) {
        auto [sidx, tidx, value] = pilot;
        auto pilotidx = d_timeslots * sidx + tidx;
        p_out[pilotidx] = value;
    }
}

void resource_mapper_kernel_cc::demap_per_timeslot(gfdm_complex* p_out,
                                                   const gfdm_complex* p_in,
                                                   const int noutput_size)
{
    // C++20: https://en.cppreference.com/w/cpp/container/span/subspan
    int sym_ctr = 0;
    for (const auto idx : d_per_timeslot_indices) {
        *p_out++ = p_in[idx]; // sym_ctr < ninput_size ? *p_in++ : gfdm_complex(0.0, 0.0);
        sym_ctr++;
        if (sym_ctr > noutput_size) {
            break;
        }
    }
}

void resource_mapper_kernel_cc::demap_per_subcarrier(gfdm_complex* p_out,
                                                     const gfdm_complex* p_in,
                                                     const int noutput_size)
{
    // C++20: https://en.cppreference.com/w/cpp/container/span/subspan
    int sym_ctr = 0;
    for (const auto idx : d_per_subcarrier_indices) {
        *p_out++ = p_in[idx];
        sym_ctr++;
        if (sym_ctr > noutput_size) {
            return;
        }
    }
}

} /* namespace gfdm */
} /* namespace gr */
