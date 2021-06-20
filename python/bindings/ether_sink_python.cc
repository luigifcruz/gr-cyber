/*
 * Copyright 2021 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

/***********************************************************************************/
/* This file is automatically generated using bindtool and can be manually edited  */
/* The following lines can be configured to regenerate this file during cmake      */
/* If manual edits are made, the following tags should be modified accordingly.    */
/* BINDTOOL_GEN_AUTOMATIC(0)                                                       */
/* BINDTOOL_USE_PYGCCXML(0)                                                        */
/* BINDTOOL_HEADER_FILE(ether_sink.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(a6b0316980f67439a2e8a82cb0d67241)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <cyber/ether_sink.h>
// pydoc.h is automatically generated in the build directory
#include <ether_sink_pydoc.h>

void bind_ether_sink(py::module& m)
{

    using ether_sink    = gr::cyber::ether_sink;


    py::class_<ether_sink,
        gr::sync_block,
        gr::block,
        gr::basic_block,
        std::shared_ptr<ether_sink>>(m, "ether_sink", D(ether_sink))

        .def(py::init(&ether_sink::make),
           py::arg("ui_enable"),
           D(ether_sink,make)
        )




        ;




}








