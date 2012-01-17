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

#ifndef _AMFUTF8_H_
#define _AMFUTF8_H_

#include <vector>

namespace cygnal
{

// This represents an regular UTF8 type in an AMF message
typedef struct {
    short length;
    char  *data;
} amfutf8_t;

// This represents an Long UTF8 type in an AMF message
typedef struct {
    int length;
    char  *data;
} amflongutf8_t;

typedef std::vector<char> utf8_t;
typedef std::vector<char> longutf8_t;

} // end of amf namespace

// end of _AMFUTF8_H_
#endif
