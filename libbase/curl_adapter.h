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

/* $Id: curl_adapter.h,v 1.9 2008/01/21 20:55:44 rsavoye Exp $ */

#ifndef CURL_ADAPTER_H
#define CURL_ADAPTER_H

#include "tu_config.h"

#include <string>


class tu_file;


/// Code to wrap libcurl around a tu_file stream.
namespace curl_adapter
{

/// \brief
/// Returns a read-only tu_file stream that fetches data
/// from an url.
//
/// The caller owns the returned tu_file*.  
///
DSOEXPORT tu_file* make_stream(const char* url);

/// \brief
/// Returns a read-only tu_file stream that fetches data
/// from an url getting posted to.
//
/// The caller owns the returned tu_file*.  
///
/// @param url
///	The url to post to.
///
/// @param postdata
///	The url-encoded post data
///
DSOEXPORT tu_file* make_stream(const char* url, const std::string& postdata);

}

#endif // CURL_ADAPTER_H

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
