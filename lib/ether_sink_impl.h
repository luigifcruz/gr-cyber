/* -*- c++ -*- */
/*
 * Copyright 2021 LuigiCruz.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_CYBER_ETHER_SINK_IMPL_H
#define INCLUDED_CYBER_ETHER_SINK_IMPL_H

#include <thread>

#include <cyber/ether_sink.h>

namespace gr {
namespace cyber {

class ether_sink_impl : public ether_sink
{
private:
    std::thread dsp;
    std::thread ui;
    std::atomic<bool> streaming{false};
    std::atomic<Jetstream::I64> position{0};

    std::shared_ptr<Jetstream::Window<Jetstream::Device::CPU>> win;
    std::shared_ptr<Jetstream::Multiply<Jetstream::Device::CPU>> mul;
    std::shared_ptr<Jetstream::FFT<Jetstream::Device::CPU>> fft;
    std::shared_ptr<Jetstream::Amplitude<Jetstream::Device::CPU>> amp;
    std::shared_ptr<Jetstream::Scale<Jetstream::Device::CPU>> scl;
    std::shared_ptr<Jetstream::Lineplot<Jetstream::Device::CPU>> lpt;
    std::shared_ptr<Jetstream::Waterfall<Jetstream::Device::CPU>> wtf;
    std::shared_ptr<Jetstream::Spectrogram<Jetstream::Device::CPU>> scp;

    std::unique_ptr<Jetstream::Memory::Vector<
      Jetstream::Device::CPU, Jetstream::CF32>> stream;
    Jetstream::Memory::CircularBuffer<Jetstream::CF32> buffer;

    ImVec2 GetRelativeMousePos();

public:
    ether_sink_impl(int fftSize, int bufferMultiplier);
    ~ether_sink_impl();

    // Where all the action really happens
    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items);
};

} // namespace cyber
} // namespace gr

#endif /* INCLUDED_CYBER_ETHER_SINK_IMPL_H */
