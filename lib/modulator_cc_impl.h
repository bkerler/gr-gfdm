/* -*- c++ -*- */
/*
 * Copyright 2016 Andrej Rode.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_GFDM_MODULATOR_CC_IMPL_H
#define INCLUDED_GFDM_MODULATOR_CC_IMPL_H

#include <gnuradio/fft/fft.h>
#include <gfdm/modulator_cc.h>
#include <volk/volk.h>
#include <volk/volk_alloc.hh>
#include <memory>

namespace gr {
namespace gfdm {

class modulator_cc_impl : public modulator_cc
{
private:
    int d_ntimeslots;
    int d_nsubcarrier;
    int d_filter_width;
    int d_N;
    int d_fft_len;
    int d_sync_fft_len;
    std::string d_len_tag_key;
    volk::vector<gr_complex> d_filter_taps;
    std::unique_ptr<fft::fft_complex_fwd> d_sc_fft;
    gr_complex* d_sc_fft_in;
    gr_complex* d_sc_fft_out;
    std::unique_ptr<fft::fft_complex_rev> d_sync_ifft;
    gr_complex* d_sync_ifft_in;
    gr_complex* d_sync_ifft_out;
    std::unique_ptr<fft::fft_complex_rev> d_out_ifft;
    gr_complex* d_out_ifft_in;
    gr_complex* d_out_ifft_out;

    volk::vector<gr_complex> d_sc_tmp;

    // Nothing to declare in this block.
    void modulate_gfdm_frame(gr_complex* out, const gr_complex* in);

protected:
    int calculate_output_stream_length(const gr_vector_int& ninput_items);
    virtual void update_length_tags(int n_produced, int n_ports);

public:
    modulator_cc_impl(int nsubcarrier,
                      int ntimeslots,
                      double filter_alpha,
                      int fft_len,
                      int sync_fft_len,
                      const std::string& len_tag_key);
    ~modulator_cc_impl();

    // Where all the action really happens
    int work(int noutput_items,
             gr_vector_int& ninput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items);
};

} // namespace gfdm
} // namespace gr

#endif /* INCLUDED_GFDM_MODULATOR_CC_IMPL_H */
