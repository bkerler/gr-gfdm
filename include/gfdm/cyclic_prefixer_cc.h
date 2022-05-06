/* -*- c++ -*- */
/*
 * Copyright 2016 Andrej Rode.
 * Copyright 2016, 2019, 2020, 2022 Johannes Demel.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#ifndef INCLUDED_GFDM_CYCLIC_PREFIXER_CC_H
#define INCLUDED_GFDM_CYCLIC_PREFIXER_CC_H

#include <gnuradio/block.h>
#include <gfdm/api.h>

namespace gr {
namespace gfdm {

/*!
 * \brief Add Cyclic Prefix, Suffix, and block pinching
 * \ingroup gfdm
 *
 * For parameters see gr::gfdm::add_cyclic_prefix_cc
 *
 */
class GFDM_API cyclic_prefixer_cc : virtual public gr::block
{
public:
    typedef std::shared_ptr<cyclic_prefixer_cc> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of gfdm::cyclic_prefixer_cc.
     *
     * To avoid accidental use of raw pointers, gfdm::cyclic_prefixer_cc's
     * constructor is in a private implementation
     * class. gfdm::cyclic_prefixer_cc::make is the public interface for
     * creating new instances.
     */
    static sptr make(int block_len,
                     int cp_len,
                     int cs_len,
                     int ramp_len,
                     std::vector<gr_complex> window_taps);
};

} // namespace gfdm
} // namespace gr

#endif /* INCLUDED_GFDM_CYCLIC_PREFIXER_CC_H */
