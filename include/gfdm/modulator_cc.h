/* -*- c++ -*- */
/*
 * Copyright 2016 Andrej Rode.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#ifndef INCLUDED_GFDM_MODULATOR_CC_H
#define INCLUDED_GFDM_MODULATOR_CC_H

#include <gnuradio/tagged_stream_block.h>
#include <gfdm/api.h>

namespace gr {
namespace gfdm {

/*!
 * \brief Obsolete GFDM modulator use gr::gfdm::simple_modulator_cc
 * \ingroup gfdm
 */
class GFDM_API modulator_cc : virtual public gr::tagged_stream_block
{
public:
    typedef std::shared_ptr<modulator_cc> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of gfdm::modulator_cc.
     *
     * To avoid accidental use of raw pointers, gfdm::modulator_cc's
     * constructor is in a private implementation
     * class. gfdm::modulator_cc::make is the public interface for
     * creating new instances.
     */
    static sptr make(int nsubcarrier,
                     int ntimeslots,
                     double filter_alpha,
                     int fft_len,
                     int sync_fft_len,
                     const std::string& len_tag_key = "frame_len");
};

} // namespace gfdm
} // namespace gr

#endif /* INCLUDED_GFDM_MODULATOR_CC_H */
