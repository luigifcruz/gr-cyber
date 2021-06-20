/* -*- c++ -*- */
/*
 * Copyright 2021 LuigiCruz.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_CYBER_ETHER_SINK_H
#define INCLUDED_CYBER_ETHER_SINK_H

#include <gnuradio/sync_block.h>
#include <cyber/api.h>

namespace gr {
namespace cyber {

/*!
 * \brief <+description of block+>
 * \ingroup cyber
 *
 */
class CYBER_API ether_sink : virtual public gr::sync_block
{
public:
    typedef std::shared_ptr<ether_sink> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of cyber::ether_sink.
     *
     * To avoid accidental use of raw pointers, cyber::ether_sink's
     * constructor is in a private implementation
     * class. cyber::ether_sink::make is the public interface for
     * creating new instances.
     */
    static sptr make(bool ui_enable);
};

} // namespace cyber
} // namespace gr

#endif /* INCLUDED_CYBER_ETHER_SINK_H */
