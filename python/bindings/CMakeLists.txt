# Copyright 2020 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Check if there is C++ code at all
########################################################################
if(NOT cyber_sources)
    MESSAGE(STATUS "No C++ sources... skipping python bindings")
    return()
endif(NOT cyber_sources)

########################################################################
# Check for pygccxml
########################################################################
GR_PYTHON_CHECK_MODULE_RAW(
    "pygccxml"
    "import pygccxml"
    PYGCCXML_FOUND
    )

include(GrPybind)

########################################################################
# Python Bindings
########################################################################

list(APPEND cyber_python_files
    ether_sink_python.cc python_bindings.cc)

GR_PYBIND_MAKE_OOT(cyber 
   ../..
   gr::cyber
   "${cyber_python_files}")

install(TARGETS cyber_python DESTINATION ${GR_PYTHON_DIR}/cyber COMPONENT pythonapi)
