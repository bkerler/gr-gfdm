/* -*- c++ -*- */
/*
 * Copyright 2016, 2019, 2020, 2022 Johannes Demel.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#ifndef INCLUDED_GFDM_RESOURCE_MAPPER_CC_H
#define INCLUDED_GFDM_RESOURCE_MAPPER_CC_H

#include <gnuradio/block.h>
#include <gfdm/api.h>

namespace gr {
namespace gfdm {

/*!
 * \brief Map info symbols to GFDM frame.
 * \ingroup gfdm
 *
 * This block prepares the input vector for the GFDM modulator.
 *
 * See: gr::gfdm::resource_mapper_kernel_cc
 *
 * \param timeslots Number of timeslots in a GFDM frame
 * \param subcarriers Number of subcarriers in a GFDM frame
 * \param active_subcarriers Number of occupied subcarriers. Smaller/equal \p subcarriers
 * \param subcarrier_map Indices of occupied subcarriers \param per_timeslot
 * Fill vector timeslot-wise or subcarrier-wise
 */
class GFDM_API resource_mapper_cc : virtual public gr::block
{
public:
    typedef std::shared_ptr<resource_mapper_cc> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of gfdm::resource_mapper_cc.
     *
     * To avoid accidental use of raw pointers, gfdm::resource_mapper_cc's
     * constructor is in a private implementation
     * class. gfdm::resource_mapper_cc::make is the public interface for
     * creating new instances.
     */
    static sptr make(int timeslots,
                     int subcarriers,
                     int active_subcarriers,
                     std::vector<int> subcarrier_map,
                     bool per_timeslot);
};

} // namespace gfdm
} // namespace gr

#endif /* INCLUDED_GFDM_RESOURCE_MAPPER_CC_H */
