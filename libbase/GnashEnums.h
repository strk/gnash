// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#ifndef GNASH_FILETYPES_H
#define GNASH_FILETYPES_H

namespace gnash {

// The file types that Gnash can handle
enum FileType {
    GNASH_FILETYPE_JPEG,
    GNASH_FILETYPE_PNG,
    GNASH_FILETYPE_GIF,
    GNASH_FILETYPE_SWF,
    GNASH_FILETYPE_FLV,
    GNASH_FILETYPE_UNKNOWN
};

enum Quality
{
    QUALITY_LOW,
    QUALITY_MEDIUM,
    QUALITY_HIGH,
    QUALITY_BEST
};

}

#endif
