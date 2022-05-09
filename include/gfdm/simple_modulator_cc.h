/* -*- c++ -*- */
/*
 * Copyright 2016, 2019, 2020, 2022 Johannes Demel.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#ifndef INCLUDED_GFDM_SIMPLE_MODULATOR_CC_H
#define INCLUDED_GFDM_SIMPLE_MODULATOR_CC_H

#include <gnuradio/sync_block.h>
#include <gfdm/api.h>

namespace gr {
namespace gfdm {

/*!
 * \brief GFDM modulator block
 * \ingroup gfdm
 *
 * This block takes in a vector emitted from gr::gfdm::resource_mapper_cc
 * and performs modulation. Computational complexity is steered via \p overlap.
 *
 * See gr::gfdm::modulator_kernel_cc
 *
 * \param timeslots Number of timeslots in a GFDM frame
 * \param subcarriers Number of subcarriers in a GFDM frame
 * \param overlap Steers modulation complexity and accuracy. 2 is usually sufficient.
 * \param frequency_taps Subcarrier filter taps in frequency domain
 */
class GFDM_API simple_modulator_cc : virtual public gr::sync_block
{
public:
    typedef std::shared_ptr<simple_modulator_cc> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of gfdm::simple_modulator_cc.
     *
     * To avoid accidental use of raw pointers, gfdm::simple_modulator_cc's
     * constructor is in a private implementation
     * class. gfdm::simple_modulator_cc::make is the public interface for
     * creating new instances.
     */
    static sptr make(int timeslots,
                     int subcarriers,
                     int overlap,
                     std::vector<gr_complex> frequency_taps);
};

} // namespace gfdm
} // namespace gr

#endif /* INCLUDED_GFDM_SIMPLE_MODULATOR_CC_H */
