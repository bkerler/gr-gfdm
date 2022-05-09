/* -*- c++ -*- */
/*
 * Copyright 2019 - 2022 Johannes Demel.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#ifndef INCLUDED_GFDM_SHORT_BURST_SHAPER_H
#define INCLUDED_GFDM_SHORT_BURST_SHAPER_H

#include <gnuradio/tagged_stream_block.h>
#include <gfdm/api.h>

namespace gr {
namespace gfdm {

/*!
 * \brief Control burst transmit behavior
 * \ingroup gfdm
 *
 * In its simplest form, this block adds zero padding around every frame.
 * Besides, it scales the input according to the scale value.
 *
 * However, this block received some more optional features.
 * It is possible to emit time tags for a UHD sink to control TX time.
 * It is possible to calculate a DSP latency time if a tag with key "time" is found.
 * This key needs to carry a uint64 pmt value with nanosecs from std::chrono.
 *
 * This block is separate from the rest of the system. It doesn't necessarily require
 * GFDM. Every bursty transmission may benefit from this block.
 *
 * \param pre_padding Number of zeros prepend to burst
 * \param post_padding Number of zeros appended to burst
 * \param scale Scaling value.
 * \param nports Number of parallel streams to support.
 * \param length_tag_name Tag key to update and use
 * \param use_timed_commands Enable timed commands for USRP sink.
 * \param timing_advance [in seconds] Add duration to current time for timed bursts.
 * \param cycle_interval [in seconds] Allow a new burst every cycle interval
 * \param enable_dsp_latency_reporting Debug parameter to read timestamp tags and print
 * elapsed time to logging.
 */
class GFDM_API short_burst_shaper : virtual public gr::tagged_stream_block
{
public:
    typedef std::shared_ptr<short_burst_shaper> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of gfdm::short_burst_shaper.
     *
     * To avoid accidental use of raw pointers, gfdm::short_burst_shaper's
     * constructor is in a private implementation
     * class. gfdm::short_burst_shaper::make is the public interface for
     * creating new instances.
     */
    static sptr make(int pre_padding,
                     int post_padding,
                     gr_complex scale,
                     const unsigned nports = 1,
                     const std::string& length_tag_name = "packet_len",
                     bool use_timed_commands = false,
                     double timing_advance = 1.0e-3,
                     double cycle_interval = 250e-6,
                     bool enable_dsp_latency_reporting = false);

    /*!
     * \brief Return multiplicative constant
     */
    virtual gr_complex scale() const = 0;

    /*!
     * \brief Set multiplicative constant
     */
    virtual void set_scale(gr_complex scale) = 0;

    /*!
     * \brief Return command timing advance
     */
    virtual double timing_advance() const = 0;

    /*!
     * \brief Set command timing advance
     */
    virtual void set_timing_advance(double timing_advance) = 0;

    /*!
     * \brief Return TX cycle interval
     */
    virtual double cycle_interval() const = 0;

    /*!
     * \brief Set TX cycle interval
     */
    virtual void set_cycle_interval(double cycle_interval) = 0;
};

} // namespace gfdm
} // namespace gr

#endif /* INCLUDED_GFDM_SHORT_BURST_SHAPER_H */
