/* -*- c++ -*- */
/*
 * Copyright 2016 Andrej Rode
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_GFDM_UTILS_H
#define INCLUDED_GFDM_UTILS_H

#include <gnuradio/fft/fft.h>
#include <gnuradio/filter/firdes.h>
#include <gfdm/api.h>

namespace gr {
namespace gfdm {

class GFDM_API rrc_filter_sparse
{
private:
    std::vector<gr_complex> d_filter_taps;

public:
    rrc_filter_sparse(
        int ntaps, double alpha, int filter_width, int nsubcarrier, int ntimeslots);
    void get_taps(std::vector<gr_complex>& out);
    ~rrc_filter_sparse();
};

} /* namespace gfdm */
} /* namespace gr */
#endif
