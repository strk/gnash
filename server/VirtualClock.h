// VirtualClock.h -- virtual clock for gnash core lib
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef GNASH_VIRTUAL_CLOCK_H
#define GNASH_VIRTUAL_CLOCK_H

// What's the policy for when to include config.h ? --strk Dec 7 2007;
#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif


namespace gnash
{

/// A class used to virtualize time flow
//
/// This class will be used to fetch current time
/// everytime it is needed by the core lib.
///
class VirtualClock
{
public:

    /// Return number of milliseconds elapsed since start.
    //
    /// Subclass this to provide time to the core lib.
    //
    /// NOTE:
    /// 32bit unsigned int has an upper limit of 4294967295
    /// which means about 49 days before overlflow.
    ///
    virtual unsigned long int elapsed() const=0;

    /// Restart the clock
    virtual void restart()=0;

    virtual ~VirtualClock() {}
};

} // namespace gnash

#endif // GNASH_VIRTUAL_CLOCK_H

