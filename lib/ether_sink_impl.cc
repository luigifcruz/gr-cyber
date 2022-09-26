/* -*- c++ -*- */
/*
 * Copyright 2021 LuigiCruz.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "ether_sink_impl.h"
#include <gnuradio/io_signature.h>
#include <jetstream/render/base.hh>

using namespace Jetstream;

namespace gr {
namespace cyber {

using input_type = std::complex<float>;
ether_sink::sptr ether_sink::make(int fftSize, int bufferMultiplier)
{
    return gnuradio::make_block_sptr<ether_sink_impl>(fftSize, bufferMultiplier);
}

ether_sink_impl::ether_sink_impl(int fftSize, int bufferMultiplier)
    : gr::sync_block("ether_sink",
                     gr::io_signature::make(1, 1, sizeof(input_type)),
                     gr::io_signature::make(0, 0, 0)),
    buffer(fftSize*2048*bufferMultiplier)
{
    // Initialize Backend
    Backend::Initialize<Device::Metal>({});

    // Initialize Render
    Render::Window::Config renderCfg;
    renderCfg.size = {3130, 1140};
    renderCfg.resizable = true;
    renderCfg.imgui = true;
    renderCfg.vsync = true;
    renderCfg.title = "CyberEther";
    Render::Initialize<Device::Metal>(renderCfg);

    // Allocate Radio Buffer
    stream = std::make_unique<Memory::Vector<Device::CPU, CF32>>(fftSize);

    // Configure Jetstream
    win = Block<Window, Device::CPU>({
        .size = stream->size(),
    }, {});

    mul = Block<Multiply, Device::CPU>({
        .size = stream->size(),
    }, {
        .factorA = *stream,
        .factorB = win->getWindowBuffer(),
    });

    fft = Block<FFT, Device::CPU>({
        .size = stream->size(),
    }, {
        .buffer = mul->getProductBuffer(),
    });

    amp = Block<Amplitude, Device::CPU>({
        .size = stream->size(),
    }, {
        .buffer = fft->getOutputBuffer(),
    });

    scl = Block<Scale, Device::CPU>({
        .size = stream->size(),
        .range = {-100.0, 0.0},
    }, {
        .buffer = amp->getOutputBuffer(),
    });

    lpt = Block<Lineplot, Device::CPU>({}, {
        .buffer = scl->getOutputBuffer(),
    });

    wtf = Block<Waterfall, Device::CPU>({}, {
        .buffer = scl->getOutputBuffer(),
    });

    scp = Block<Spectrogram, Device::CPU>({}, {
        .buffer = scl->getOutputBuffer(),
    });

    Render::Create();

    streaming = true;

    ui = std::thread([&]{
        while (streaming) {
            if (!Render::KeepRunning()) {
                exit(0);
            }

            Render::Begin();

            ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

            {
                ImGui::Begin("Waterfall");

                auto [x, y] = ImGui::GetContentRegionAvail();
                auto [width, height] = wtf->viewSize({(U64)x, (U64)y});
                ImGui::Image(wtf->getTexture().raw(), ImVec2(width, height));

                if (ImGui::IsItemHovered() && ImGui::IsAnyMouseDown()) {
                    if (position == 0) {
                        position = (GetRelativeMousePos().x / wtf->zoom()) + wtf->offset();
                    }
                    wtf->offset(position - (GetRelativeMousePos().x / wtf->zoom()));
                } else {
                    position = 0;
                }

                ImGui::End();
            }

            {
                ImGui::Begin("Spectrogram");

                auto [x, y] = ImGui::GetContentRegionAvail();
                auto [width, height] = scp->viewSize({(U64)x, (U64)y});
                ImGui::Image(scp->getTexture().raw(), ImVec2(width, height));

                ImGui::End();
            }

            {
                ImGui::Begin("Lineplot");

                auto [x, y] = ImGui::GetContentRegionAvail();
                auto [width, height] = lpt->viewSize({(U64)x, (U64)y});
                ImGui::Image(lpt->getTexture().raw(), ImVec2(width, height));

                ImGui::End();
            }

            {
                ImGui::Begin("Control");

                auto [min, max] = scl->range();
                if (ImGui::DragFloatRange2("dBFS Range", &min, &max,
                            1, -300, 0, "Min: %.0f dBFS", "Max: %.0f dBFS")) {
                    scl->range({min, max});
                }

                auto interpolate = wtf->interpolate();
                if (ImGui::Checkbox("Interpolate Waterfall", &interpolate)) {
                    wtf->interpolate(interpolate);
                }

                auto zoom = wtf->zoom();
                if (ImGui::DragFloat("Waterfall Zoom", &zoom, 0.01, 1.0, 5.0, "%f", 0)) {
                    wtf->zoom(zoom);
                }

                ImGui::End();
            }

            {
                ImGui::Begin("Buffer Info");

                float bufferThroughputMB = (buffer.GetThroughput() / (1024 * 1024));
                ImGui::Text("Throughput %.0f MB/s", bufferThroughputMB);

                float bufferCapacityMB = ((F32)buffer.GetCapacity() * sizeof(input_type) / (1024 * 1024));
                ImGui::Text("Capacity %.0f MB", bufferCapacityMB);

                ImGui::Text("Overflows %llu", buffer.GetOverflows());

                ImGui::Separator();
                ImGui::Spacing();

                float bufferUsageRatio = (F32)buffer.GetOccupancy() / buffer.GetCapacity();
                ImGui::ProgressBar(bufferUsageRatio, ImVec2(0.0f, 0.0f), "");
                ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                ImGui::Text("Usage");

                ImGui::End();
            }

            Jetstream::Present();
            Render::End();
        }
    });

    dsp = std::thread([&]{
        while (streaming) {
            buffer.Get(stream->data(), stream->size());
            Jetstream::Compute();
        }
    });
}

ether_sink_impl::~ether_sink_impl() {
    streaming = false;

    dsp.join();
    ui.join();

    Render::Destroy();
    Backend::Destroy<Device::Metal>();
}

ImVec2 ether_sink_impl::GetRelativeMousePos() {
    ImVec2 mousePositionAbsolute = ImGui::GetMousePos();
    ImVec2 screenPositionAbsolute = ImGui::GetItemRectMin();
    return ImVec2(mousePositionAbsolute.x - screenPositionAbsolute.x,
                  mousePositionAbsolute.y - screenPositionAbsolute.y);
}

int ether_sink_impl::work(int noutput_items,
                          gr_vector_const_void_star& input_items,
                          gr_vector_void_star& output_items)
{
    const input_type* in = reinterpret_cast<const input_type*>(input_items[0]);

    buffer.Put((std::complex<float>*)in, noutput_items);

    // Tell runtime system how many output items we produced.
    return noutput_items;
}

} /* namespace cyber */
} /* namespace gr */
