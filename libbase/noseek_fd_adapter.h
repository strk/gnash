// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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


#ifndef STDIN_ADAPTER_H
#define STDIN_ADAPTER_H

#include "dsodefs.h"



namespace gnash {
class IOChannel;


/// Code to use volatile (non-back-seekable) streams as IOChannel
/// objects (which are seekable by definition) by storing read bytes
/// in a temporary file.
namespace noseek_fd_adapter {

/// \brief
/// Returns a read-only IOChannel that fetches data
/// from an file descriptor open for read.
//
/// The caller owns the returned IOChannel.  
///
/// Specify a cachefilename if you want to be able to access
/// the full cache after deletion of the returned IOChannel.
///
DSOEXPORT IOChannel* make_stream(int fd, const char* cachefilename = 0);

} // namespace gnash::noseek_fd_adapter
} // namespace gnash
 

#endif // STDIN_ADAPTER_H

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
