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

/* $Id: noseek_fd_adapter.h,v 1.7 2008/01/21 20:55:45 rsavoye Exp $ */

#ifndef STDIN_ADAPTER_H
#define STDIN_ADAPTER_H

#include "tu_config.h"

#include <string>


class tu_file;


/// Code to use volatile (non-back-seekable) streams as tu_files
/// by storing read bytes in a temporary file.
namespace noseek_fd_adapter
{

/// \brief
/// Returns a read-only tu_file stream that fetches data
/// from an file descriptor open for read.
//
/// The caller owns the returned tu_file*.  
///
/// Specify a cachefilename if you want to be able to access
/// the full cache after deletion of the returned tu_file.
///
DSOEXPORT tu_file* make_stream(int fd, const char* cachefilename=NULL);

}

#endif // STDIN_ADAPTER_H

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
