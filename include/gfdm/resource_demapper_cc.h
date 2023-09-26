/* -*- c++ -*- */
/*
 * Copyright 2016, 2019, 2020 Johannes Demel.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#ifndef INCLUDED_GFDM_RESOURCE_DEMAPPER_CC_H
#define INCLUDED_GFDM_RESOURCE_DEMAPPER_CC_H

#include <gnuradio/block.h>
#include <gfdm/api.h>

namespace gr {
namespace gfdm {

/*!
 * \brief Demap info symbols from GFDM frame.
 * \ingroup gfdm
 *
 * This block extract the actual used symbols from a GFDM frame.
 *
 * See: gr::gfdm::resource_mapper_kernel_cc
 *
 * \param timeslots Number of timeslots in a GFDM frame
 * \param subcarriers Number of subcarriers in a GFDM frame
 * \param active_subcarriers Number of occupied subcarriers. Smaller/equal \p subcarriers
 * \param subcarrier_map Indices of occupied subcarriers \param per_timeslot
 * Fill vector timeslot-wise or subcarrier-wise
 */
class GFDM_API resource_demapper_cc : virtual public gr::block
{
public:
    typedef std::shared_ptr<resource_demapper_cc> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of gfdm::resource_demapper_cc.
     *
     * To avoid accidental use of raw pointers, gfdm::resource_demapper_cc's
     * constructor is in a private implementation
     * class. gfdm::resource_demapper_cc::make is the public interface for
     * creating new instances.
     */
    static sptr make(int timeslots,
                     int subcarriers,
                     int active_subcarriers,
                     std::vector<int> subcarrier_map,
                     bool per_timeslot);

    virtual void
    set_pilots(const std::vector<std::tuple<unsigned, unsigned, gr_complex>> pilots) = 0;

    virtual std::vector<std::tuple<unsigned, unsigned, gr_complex>> pilots() const = 0;
};

} // namespace gfdm
} // namespace gr

#endif /* INCLUDED_GFDM_RESOURCE_DEMAPPER_CC_H */
