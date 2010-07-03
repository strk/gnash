// string_table.cpp -- A shared string table for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#include "string_table.h"
#include <boost/algorithm/string/case_conv.hpp>

namespace gnash {

const std::string string_table::mEmpty;

string_table::key
string_table::find(const std::string& t_f, bool insert_unfound)
{
    if (t_f.empty()) return 0;

	table::nth_index<0>::type::iterator i = mTable.get<0>().find(t_f);

	if (i == mTable.end())
	{
		if (insert_unfound)
		{

			// First we lock.
			//boost::mutex::scoped_lock aLock(mLock);
			// Then we see if someone else managed to sneak past us.
			i = mTable.get<0>().find(t_f);
			// If they did, use that value.
			if (i != mTable.end())
				return i->mId;

            return already_locked_insert(t_f, mLock);
		}
        return 0;
	}

	return i->mId;
}

string_table::key
string_table::insert(const std::string& to_insert)
{
	//boost::mutex::scoped_lock aLock(mLock);
    return already_locked_insert(to_insert, mLock);
}

void
string_table::insert_group(const svt* l, std::size_t size)
{
	//boost::mutex::scoped_lock aLock(mLock);
    for (std::size_t i = 0; i < size; ++i) {
        // Copy to avoid changing the original table.
        const svt s = l[i];

        // The keys don't have to be consecutive, so any time we find a key
        // that is too big, jump a few keys to avoid rewriting this on every
        // item.
       if (s.mId > mHighestKey) mHighestKey = s.mId + 256;
       mTable.insert(s);
    }
    
    for (std::size_t i = 0; i < size; ++i) {
        const svt s = l[i];
        const std::string& t = boost::to_lower_copy(s.mValue);
        if (t != s.mValue) {
            _caseTable[s.mId] = insert(t);
        }
    }

}

string_table::key
string_table::already_locked_insert(const std::string& to_insert, boost::mutex&)
{
	svt theSvt (to_insert, ++mHighestKey);

	const key ret = mTable.insert(theSvt).first->mId;

    const std::string i = boost::to_lower_copy(to_insert);
    if (i != to_insert)  {
        const key k = find(i, true);
        _caseTable[ret] = k;
    }

    return ret;
}

bool
string_table::noCase(key a, key b) const
{
    std::map<key, key>::const_iterator i = _caseTable.find(a);
    const key k1  = i == _caseTable.end() ? a : i->second;

    std::map<key, key>::const_iterator j = _caseTable.find(b);
    const key k2  = j == _caseTable.end() ? b : j->second;
    return k2 == k1;
}

bool
noCaseEqual(string_table& st, string_table::key a, string_table::key b)
{
    if (a == b) return true;
    return st.noCase(a, b);
}
          
}
