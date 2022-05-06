/* -*- c++ -*- */
/*
 * Copyright 2017 - 2022 Johannes Demel.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#ifndef INCLUDED_GFDM_EXTRACT_BURST_CC_H
#define INCLUDED_GFDM_EXTRACT_BURST_CC_H

#include <gnuradio/block.h>
#include <gfdm/api.h>

namespace gr {
namespace gfdm {

/*!
 * \brief Extract burst at tag position
 * \ingroup gfdm
 *
 * Extract a burst, or block, or frame with a fixed length.
 * A tag indicates the start of a burst and CFO compensation may be applied if this burst start tag carries such information.
 * The \p burst_start_tag info may be forwarded with a different key.
 *
 * \param burst_len Number of samples to extract for each tag
 * \param tag_backoff Number of samples before the tag to extract
 * \param burst_start_tag Key of the tag indicating a burst start
 * \param activate_cfo_correction Activate CFO compensation based on tag info
 * \param forward_burst_start_tag Tag key for info forwarded from \p burst_start_tag
 */
class GFDM_API extract_burst_cc : virtual public gr::block
{
public:
    typedef std::shared_ptr<extract_burst_cc> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of gfdm::extract_burst_cc.
     *
     * To avoid accidental use of raw pointers, gfdm::extract_burst_cc's
     * constructor is in a private implementation
     * class. gfdm::extract_burst_cc::make is the public interface for
     * creating new instances.
     */
    static sptr make(int burst_len,
                     int tag_backoff,
                     std::string burst_start_tag,
                     bool activate_cfo_correction = false,
                     std::string forward_burst_start_tag = "");

    virtual void activate_cfo_compensation(bool activate_cfo_compensation) = 0;
    virtual bool cfo_compensation() const = 0;
    virtual void set_fixed_phase_increment(double phase_increment, bool activate) = 0;
};

} // namespace gfdm
} // namespace gr

#endif /* INCLUDED_GFDM_EXTRACT_BURST_CC_H */
