/* -*- c++ -*- */
/*
 * Copyright 2016 Andrej Rode.
 * Copyright 2019, 2020, 2022 Johannes Demel.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#ifndef INCLUDED_GFDM_SIMPLE_RECEIVER_CC_H
#define INCLUDED_GFDM_SIMPLE_RECEIVER_CC_H

#include <gnuradio/sync_block.h>
#include <gfdm/api.h>

namespace gr {
namespace gfdm {

/*!
 * \brief Simple GFDM demodulator GR block
 * \ingroup gfdm
 *
 * See gr::gfdm::receiver_kernel_cc
 *
 * \param timeslots Number of timeslots in a GFDM frame
 * \param subcarriers Number of subcarriers in a GFDM frame
 * \param overlap Steers modulation complexity and accuracy. 2 is usually sufficient.
 * \param frequency_taps Subcarrier filter taps in frequency domain
 *
 */
class GFDM_API simple_receiver_cc : virtual public gr::sync_block
{
public:
    typedef std::shared_ptr<simple_receiver_cc> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of gfdm::simple_receiver_cc.
     *
     * To avoid accidental use of raw pointers, gfdm::simple_receiver_cc's
     * constructor is in a private implementation
     * class. gfdm::simple_receiver_cc::make is the public interface for
     * creating new instances.
     */
    static sptr make(int timeslots,
                     int subcarriers,
                     int overlap,
                     std::vector<gr_complex> frequency_taps);
};

} // namespace gfdm
} // namespace gr

#endif /* INCLUDED_GFDM_SIMPLE_RECEIVER_CC_H */
