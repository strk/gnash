// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

/* $Id: StringPredicates.h,v 1.8 2007/07/01 10:54:19 bjacques Exp $ */

#ifndef GNASH_STRINGPREDICATES_H
#define GNASH_STRINGPREDICATES_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string> 
#include <cctype> // for toupper,tolower

#include <cassert> // for inlines

namespace gnash {

/// A case-insensitive string comparator (probably not very performant)
class StringNoCaseLessThen {
public:
	bool operator() (const std::string& a, const std::string& b) const
	{
		size_t a_len = a.length();
		size_t b_len = b.length();

		size_t cmplen = a_len < b_len ? a_len : b_len;

		for (size_t i=0; i<cmplen; ++i)
		{
			char cha = toupper(a[i]);
			char chb = toupper(b[i]);

			if (cha < chb) return true;
			else if (cha > chb) return false;
			assert(cha==chb);
		}

		// strings are equal for whole lenght of a,
		// a is LessThen b only if 'b' contains more
		// characters then 'a' (if same number of
		// chars 'a' is NOT less then 'b')

		if ( a_len < b_len ) return true;
		return false; // equal or greater

	}
};

/// A case-insensitive string equality operator (probably not very performant)
class StringNoCaseEqual {
public:
	bool operator() (const std::string& a, const std::string& b) const
	{
		if ( a.length() != b.length() ) return false;
		for (size_t i=0; i<a.length(); ++i)
		{
			char cha = toupper(a[i]);
			char chb = toupper(b[i]);

			if (cha != chb) return false;
		}

		return true;

	}
};


} // namespace gnash

#endif // GNASH_STRINGPREDICATES_H

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
