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


#ifndef GNASH_STRINGPREDICATES_H
#define GNASH_STRINGPREDICATES_H

#include <string>
#include <algorithm>
#include <boost/algorithm/string/compare.hpp>
#include <boost/algorithm/string/predicate.hpp>

namespace gnash {

/// A case-insensitive string comparator
class StringNoCaseLessThan {
public:
	bool operator() (const std::string& a, const std::string& b) const
	{
		return std::lexicographical_compare(a.begin(), a.end(),
						    b.begin(), b.end(),
						    nocase_less());
	}
private:

	class nocase_less
	{
	public:
		nocase_less(const std::locale& locale = std::locale())
			: _locale(locale)
		{}

		bool operator() (const char& a, const char& b) const
		{
			return std::toupper<char>(a, _locale) <
			       std::toupper<char>(b, _locale);
		}
	private:
		const std::locale& _locale;
	};
};


/// A case-insensitive string equality operator
class StringNoCaseEqual {
public:

    typedef bool result_type;

	bool operator() (const std::string& a, const std::string& b) const
	{
		return boost::iequals(a, b);
	}
};

} // namespace gnash

#endif // GNASH_STRINGPREDICATES_H

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
