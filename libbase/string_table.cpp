// string_table.cpp -- A shared string table for Gnash.
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

#include "string_table.h"

using namespace gnash;

string_table::table string_table::mTable;
// By starting at 0, this can be the 'unknown' value.
string_table::key string_table::mHighestKey = 0;
boost::mutex string_table::mLock;
std::string string_table::mEmpty = "";

string_table::key
string_table::find(const std::string& to_find, bool insert_unfound)
{
	table::nth_index<0>::type::iterator i = mTable.get<0>().find(to_find);

	if (i == mTable.end() && insert_unfound)
	{
		if (insert_unfound)
		{
			// First we lock.
			//boost::mutex::scoped_lock aLock(mLock);
			// Then we see if someone else managed to sneak past us.
			i = mTable.get<0>().find(to_find);
			// If they did, use that value.
			if (i != mTable.end())
				return i->mId;
			// Otherwise, insert it.
			return mTable.insert(svt(to_find, ++mHighestKey)).first->mId;
		}
		else
			return 0;
	}

	return i->mId;
}

string_table::key
string_table::insert(const std::string& to_insert)
{
	boost::mutex::scoped_lock aLock(mLock);

	return mTable.insert(svt(to_insert, ++mHighestKey)).first->mId;
}

string_table::key
string_table::already_locked_insert(const std::string& to_insert, boost::mutex&)
{
	return mTable.insert(svt(to_insert, ++mHighestKey)).first->mId;
}

