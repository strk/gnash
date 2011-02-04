// 
//   Copyright (C) 2009, 2010, 2011 Free Software Foundation, Inc.
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

#ifndef __SERVERSO_H__
#define __SERVERSO_H__

#include <iostream>

#include "sol.h"
#include "element.h"

/// \namespace cygnal
///
/// This namespace is for all the Cygnal specific classes not used by
/// anything else in Gnash.
namespace cygnal {
    
/// \class cygnal::ServerSO
///	This class handles storing SharedObject on the server side.
///     The SOL class is used to optionally read and write a disk 
///     file similar to the client side.
class DSOEXPORT ServerSO : public cygnal::SOL
{
public:
    ServerSO();
    ~ServerSO();

    /// \brief Dump the internal data of this class in a human readable form.
    /// @remarks This should only be used for debugging purposes.
    void dump() const { dump(std::cerr); }
    
    /// \overload dump(std::ostream& os) const
    void dump(std::ostream& os) const;
    
};

/// \brief Dump to the specified output stream.
inline std::ostream& operator << (std::ostream& os, const ServerSO &so)
{
    so.dump(os);
    return os;
}

// End of gnash namespace 
}

// __SERVERSO_H__
#endif


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
