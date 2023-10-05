/* -*- c++ -*- */
/*
 * Copyright 2018 - 2022 Johannes Demel.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#ifndef INCLUDED_GFDM_TRANSMITTER_CC_H
#define INCLUDED_GFDM_TRANSMITTER_CC_H

#include <gnuradio/block.h>
#include <gfdm/api.h>

namespace gr {
namespace gfdm {

/*!
 * \brief GFDM transmitter chain in one GR block
 * \ingroup gfdm
 *
 * See gr::gfdm::transmitter_kernel
 *
 * This blocks performs resource mapping, modulation, cyclic prefixing, and preamble
 * insertion. Besides GFDM modulation these operations are lightweight and thus combining
 * them in one block seems beneficial from a latency point of view.
 *
 * \param timeslots Number of timeslots in a GFDM frame
 * \param subcarriers Number of subcarriers in a GFDM frame
 * \param active_subcarriers Number of occupied subcarriers smaller or equal to \p
 * subcarriers
 * \param cp_len Cyclic prefix length in samples
 * \param cs_len Cyclic suffix length in samples. Typically equal to \p rampl_len
 * \param ramp_len Number of samples used for block pinching.
 * \param subcarrier_map Indices of occupied subcarriers
 * \param per_timeslot Fill vector timeslot-wise or subcarrier-wise
 * \param overlap Steers modulation complexity and accuracy. 2 is usually sufficient.
 * \param frequency_taps Subcarrier filter taps in frequency domain
 * \param window_taps Taps used for block pinching. Typically RC taps cf. WiFi.
 * \param cyclic_shift Number of samples cyclic block shift for CDD
 * \param preambles Preamble vectors with cyclic shifts.
 * \param tsb_tag_key Length tag key. If non-empty update GR length tag key.
 */
class GFDM_API transmitter_cc : virtual public gr::block
{
public:
    typedef std::shared_ptr<transmitter_cc> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of gfdm::transmitter_cc.
     *
     * To avoid accidental use of raw pointers, gfdm::transmitter_cc's
     * constructor is in a private implementation
     * class. gfdm::transmitter_cc::make is the public interface for
     * creating new instances.
     */
    static sptr make(int timeslots,
                     int subcarriers,
                     int active_subcarriers,
                     int cp_len,
                     int cs_len,
                     int ramp_len,
                     std::vector<int> subcarrier_map,
                     bool per_timeslot,
                     int overlap,
                     std::vector<gr_complex> frequency_taps,
                     std::vector<gr_complex> window_taps,
                     std::vector<int> cyclic_shifts,
                     std::vector<std::vector<gr_complex>> preambles,
                     const std::string& tsb_tag_key = "");

    virtual void
    set_pilots(const std::vector<std::tuple<unsigned, unsigned, gr_complex>> pilots) = 0;

    virtual std::vector<std::tuple<unsigned, unsigned, gr_complex>> pilots() const = 0;
};

} // namespace gfdm
} // namespace gr

#endif /* INCLUDED_GFDM_TRANSMITTER_CC_H */
