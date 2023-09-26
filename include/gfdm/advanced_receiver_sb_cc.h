/* -*- c++ -*- */
/*
 * Copyright 2016, 2017 Andrej Rode.
 * Copyright 2016, 2018, 2019, 2020, 2022 Johannes Demel.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#ifndef INCLUDED_GFDM_ADVANCED_RECEIVER_SB_CC_H
#define INCLUDED_GFDM_ADVANCED_RECEIVER_SB_CC_H

#include <gnuradio/digital/api.h>
#include <gnuradio/digital/constellation.h>
#include <gnuradio/sync_block.h>
#include <gfdm/api.h>

namespace gr {
namespace gfdm {

/*!
 * \brief Advanced GFDM demodulator GR block
 * \ingroup gfdm
 *
 * This block integrates the gr::gfdm::advanced_receiver_kernel_cc into the GNU Radio
 * streaming API. All parameters are forwarded to gr::gfdm::advanced_receiver_kernel_cc.
 *
 */
class GFDM_API advanced_receiver_sb_cc : virtual public gr::sync_block
{
public:
    typedef std::shared_ptr<advanced_receiver_sb_cc> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of gfdm::advanced_receiver_sb_cc.
     *
     * To avoid accidental use of raw pointers, gfdm::advanced_receiver_sb_cc's
     * constructor is in a private implementation
     * class. gfdm::advanced_receiver_sb_cc::make is the public interface for
     * creating new instances.
     */
    static sptr make(int timeslots,
                     int subcarriers,
                     int overlap,
                     int ic_iter,
                     std::vector<gr_complex> frequency_taps,
                     gr::digital::constellation_sptr constellation,
                     std::vector<int> subcarrier_map,
                     int do_phase_compensation);

    virtual void set_phase_compensation(int do_phase_compensation) = 0;
    virtual int get_phase_compensation() = 0;
    virtual void set_ic(int ic_iter) = 0;
    virtual int get_ic(void) = 0;
    virtual bool activate_pilot_estimation(bool activate) = 0;

    virtual void
    set_pilots(const std::vector<std::tuple<unsigned, unsigned, gr_complex>> pilots) = 0;

    virtual std::vector<std::tuple<unsigned, unsigned, gr_complex>> pilots() const = 0;
};

} // namespace gfdm
} // namespace gr

#endif /* INCLUDED_GFDM_ADVANCED_RECEIVER_SB_CC_H */
