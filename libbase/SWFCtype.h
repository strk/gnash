// SWFCtype.h: Case conversion code for SWF strings.
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
//

#ifndef GNASH_SWF_CTYPE_H
#define GNASH_SWF_CTYPE_H

#include <locale>

namespace gnash {

/// Facet for SWF-compatible case conversion.
class SWFCtype : public std::ctype<wchar_t>
{
public:
    SWFCtype(size_t refs = 0) : std::ctype<wchar_t>(refs) {}
    typedef std::ctype<wchar_t>::char_type char_type;
protected:
    virtual char_type do_toupper(char_type) const;
    virtual const char_type* do_toupper(char_type* low, const char_type* high) const;
    virtual char_type do_tolower(char_type) const;
    virtual const char_type* do_tolower(char_type* low, const char_type* high) const;
};

} // namespace gnash

#endif
