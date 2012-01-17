// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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

#ifndef GNASH_AS_EXCEPTION_H
#define GNASH_AS_EXCEPTION_H

namespace gnash {
    namespace abc {
        class Namespace;
        class Class;
    }
}

namespace gnash {

class asException
{
public:
	void setStart(boost::uint32_t i) { _start = i; }
	void setEnd(boost::uint32_t i) { mEnd = i; }
	void setCatch(boost::uint32_t i) { mCatch = i; }
	void catchAny() { mCatchAny = true; }
	void setCatchType(abc::Class* p) { mCatchType = p; }
	void setNamespace(abc::Namespace* n) { _namespace = n; }
	void setName(string_table::key name) { _name = name; }

private:
	boost::uint32_t _start;
	boost::uint32_t mEnd;
	boost::uint32_t mCatch;
	bool mCatchAny;
	abc::Class *mCatchType;
    abc::Namespace *_namespace;
	string_table::key _name;
};

} // namespace gnash
#endif
