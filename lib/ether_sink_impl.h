/* -*- c++ -*- */
/*
 * Copyright 2021 LuigiCruz.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_CYBER_ETHER_SINK_IMPL_H
#define INCLUDED_CYBER_ETHER_SINK_IMPL_H

#include <cyber/ether_sink.h>

#include <render/base.hpp>
#include <samurai/base/cbuffer.hpp>
#include <jetstream/fft/base.hpp>
#include <jetstream/lineplot/base.hpp>
#include <jetstream/waterfall/base.hpp>
#include <jetstream/histogram/base.hpp>

namespace gr {
namespace cyber {

class ether_sink_impl : public ether_sink
{
private:
    std::thread dsp, ui;
    std::atomic<bool> streaming{false};

    // Render
    std::shared_ptr<Render::Instance> render;

    // Jetstream
    std::shared_ptr<Jetstream::Engine> engine;
    std::shared_ptr<Jetstream::FFT::Generic> fft;
    std::shared_ptr<Jetstream::Lineplot::Generic> lpt;
    std::shared_ptr<Jetstream::Waterfall::Generic> wtf;

    // Samurai
    Samurai::CircularBuffer<std::complex<float>> buffer;
    std::vector<std::complex<float>> stream;

public:
    ether_sink_impl(bool ui_enable);
    ~ether_sink_impl();

    // Where all the action really happens
    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items);
};

} // namespace cyber
} // namespace gr

#endif /* INCLUDED_CYBER_ETHER_SINK_IMPL_H */
