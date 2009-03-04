// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifndef __CYGNAL_H__
#define __CYGNAL_H__

/// \namespace cygnal
///
/// This namespace is for all the Cygnal specific classes not used by
/// anything else in Gnash.
namespace cygnal {
    
/// \class cygnal::ThreadCounter of threads currently
///     active. This is primarily so the counter can be wrapped with a
///     mutex to be thread safe, as threads delete themseleves.
class DSOEXPORT ThreadCounter
{
  public:
    ThreadCounter() : _tids(0) {};
    void increment() { boost::mutex::scoped_lock lk(_tid_mutex); ++_tids; };
    void decrement() { boost::mutex::scoped_lock lk(_tid_mutex); --_tids; };
    int num_of_tids() { return _tids; };
  private:
    boost::mutex  _tid_mutex;
    int           _tids;
    boost::thread _tid_handle;
};
  
// End of gnash namespace 
}

// __CYGNAL_H__
#endif


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
