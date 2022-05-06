/* -*- c++ -*- */
/*
 * Copyright 2016 Andrej Rode.
 * Copyright 2016, 2019, 2020, 2022 Johannes Demel.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#ifndef INCLUDED_GFDM_REMOVE_PREFIX_CC_H
#define INCLUDED_GFDM_REMOVE_PREFIX_CC_H

#include <gnuradio/block.h>
#include <gfdm/api.h>

namespace gr {
namespace gfdm {

/*!
 * \brief Extract \p block_len samples from \p frame_len samples at \p offset
 * \ingroup gfdm
 *
 * \param frame_len Number of samples to extract sub part from
 * \param block_len Number of samples to extract
 * \param offset Offset to start at in \p frame_len
 * \param gfdm_sync_tag_key Tag key that indicates start of a frame with \p frame_len
 * samples.
 */
class GFDM_API remove_prefix_cc : virtual public gr::block
{
public:
    typedef std::shared_ptr<remove_prefix_cc> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of gfdm::remove_prefix_cc.
     *
     * To avoid accidental use of raw pointers, gfdm::remove_prefix_cc'
     * constructor is in a private implementation
     * class. gfdm::remove_prefix_cc::make is the public interface for
     * creating new instances.
     */
    static sptr
    make(int frame_len, int block_len, int offset, const std::string& gfdm_sync_tag_key);
};

} // namespace gfdm
} // namespace gr

#endif /* INCLUDED_GFDM_REMOVE_PREFIX_CC_H */
