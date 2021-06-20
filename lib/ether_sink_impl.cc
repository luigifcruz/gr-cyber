/* -*- c++ -*- */
/*
 * Copyright 2021 LuigiCruz.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "ether_sink_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
namespace cyber {

using input_type = std::complex<float>;
ether_sink::sptr ether_sink::make(bool ui_enable)
{
    return gnuradio::make_block_sptr<ether_sink_impl>(ui_enable);
}

ether_sink_impl::ether_sink_impl(bool ui_enable)
    : gr::sync_block("ether_sink",
                     gr::io_signature::make(1, 1, sizeof(input_type)),
                     gr::io_signature::make(0, 0, 0)),
    buffer(1024*1024*8)
{
    ui = std::thread([&]{
        // Configure Render
        Render::Instance::Config renderCfg;
        renderCfg.size = {3130, 1140};
        renderCfg.resizable = true;
        renderCfg.imgui = true;
        renderCfg.vsync = true;
        renderCfg.title = "CyberEther Sink";
        render = Render::Instantiate(Render::API::GLES, renderCfg);

        // Configure Jetstream Modules
        auto device = Jetstream::Locale::CUDA;
        engine = std::make_shared<Jetstream::Engine>();
        stream = std::vector<std::complex<float>>(2048 * 2);

        Jetstream::FFT::Config fftCfg;
        fftCfg.input0 = {Jetstream::Locale::CPU, stream};
        fftCfg.policy = {Jetstream::Launch::ASYNC, {}};
        fft = Jetstream::FFT::Instantiate(device, fftCfg);

        Jetstream::Lineplot::Config lptCfg;
        lptCfg.render = render;
        lptCfg.input0 = fft->output();
        lptCfg.policy = {Jetstream::Launch::ASYNC, {fft}};
        lpt = Jetstream::Lineplot::Instantiate(device, lptCfg);

        Jetstream::Waterfall::Config wtfCfg;
        wtfCfg.render = render;
        wtfCfg.input0 = fft->output();
        wtfCfg.policy = {Jetstream::Launch::ASYNC, {fft}};
        wtf = Jetstream::Waterfall::Instantiate(device, wtfCfg);

        // Add Jetstream modules to the execution pipeline.
        engine->push_back(fft);
        engine->push_back(lpt);
        engine->push_back(wtf);

        render->create();

        while (render->keepRunning() && streaming) {
            render->start();

            ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

            {
                ImGui::Begin("Lineplot");
                auto regionSize = ImGui::GetContentRegionAvail();
                auto [width, height] = lpt->size({(int)regionSize.x, (int)regionSize.y});
                ImGui::Image((void*)(intptr_t)lpt->tex().lock()->raw(), ImVec2(width, height));
                ImGui::End();
            }

            {
                ImGui::Begin("Waterfall");
                auto regionSize = ImGui::GetContentRegionAvail();
                auto [width, height] = wtf->size({(int)regionSize.x, (int)regionSize.y});
                ImGui::Image((void*)(intptr_t)wtf->tex().lock()->raw(), ImVec2(width, height));
                ImGui::End();
            }

            {
                ImGui::Begin("Control");

                auto [min, max] = fft->amplitude();
                if (ImGui::DragFloatRange2("dBFS Range", &min, &max,
                            1, -300, 0, "Min: %.0f dBFS", "Max: %.0f dBFS")) {
                    fft->amplitude({min, max});
                }

                auto interpolate = wtf->interpolate();
                if (ImGui::Checkbox("Interpolate Waterfall", &interpolate)) {
                    wtf->interpolate(interpolate);
                }

                ImGui::End();
            }

            ImGui::Begin("Samurai Info");
            if (streaming) {
                float bufferUsageRatio = (float)buffer.Occupancy() / (float)buffer.Capacity();
                ImGui::ProgressBar(bufferUsageRatio, ImVec2(0.0f, 0.0f), "");
                ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                ImGui::Text("Buffer Usage");
            }
            ImGui::End();

            render->synchronize();
            JETSTREAM_CHECK_THROW(engine->present());
            render->end();
        }

        render->destroy();
    });

    dsp = std::thread([&]{
        streaming = true;
        while (streaming) {
            buffer.Get(stream.data(), stream.size());
            JETSTREAM_CHECK_THROW(engine->compute());
        }
    });
}

ether_sink_impl::~ether_sink_impl() {
    streaming = false;
    dsp.join();
    ui.join();
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
