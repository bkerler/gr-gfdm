/* -*- c++ -*- */
/*
 * Copyright 2017 - 2022 Johannes Demel.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#ifndef INCLUDED_GFDM_CHANNEL_ESTIMATOR_CC_H
#define INCLUDED_GFDM_CHANNEL_ESTIMATOR_CC_H

#include <gnuradio/block.h>
#include <gfdm/api.h>

namespace gr {
namespace gfdm {

/*!
 * \brief Preamble-based channel estimator
 * \ingroup gfdm
 *
 * Estimate current channel realization based on a received preamble.
 * The output is a channel estimate that one may use directly
 * to equalize a GFDM frame in the frequency domain.
 *
 * See gr::gfdm::preamble_channel_estimator_cc for details.
 *
 * \param timeslots Number of timeslots in a GFDM frame
 * \param fft_len Number of subcarriers in a GFDM frame. occupied an unoccupied.
 * \param active_subcarriers Number of occupied/used subcarriers.
 * \param is_dc_free true if the DC carrier is unoccupied.
 * \param which_estimator Select estimator by integer.
 * \param preamble Known transmitted preamble.
 * \param snr_tag_key key for sample tag that carries SNR info.
 * \param cnr_tag_key key for sample tag that carries CNR values.
 */
class GFDM_API channel_estimator_cc : virtual public gr::block
{
public:
    typedef std::shared_ptr<channel_estimator_cc> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of gfdm::channel_estimator_cc.
     *
     * To avoid accidental use of raw pointers, gfdm::channel_estimator_cc's
     * constructor is in a private implementation
     * class. gfdm::channel_estimator_cc::make is the public interface for
     * creating new instances.
     */
    static sptr make(int timeslots,
                     int fft_len,
                     int active_subcarriers,
                     bool is_dc_free,
                     int which_estimator,
                     std::vector<gr_complex> preamble,
                     const std::string& snr_tag_key = "snr_lin",
                     const std::string& cnr_tag_key = "cnr");
};

} // namespace gfdm
} // namespace gr

#endif /* INCLUDED_GFDM_CHANNEL_ESTIMATOR_CC_H */
