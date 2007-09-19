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
#include <boost/algorithm/string/case_conv.hpp>

using namespace gnash;

std::string string_table::mEmpty = "";

string_table::key
string_table::find(const std::string& to_find, bool insert_unfound)
{
	table::nth_index<0>::type::iterator i = mTable.get<0>().find(to_find);

	if (i == mTable.end() && insert_unfound)
	{
		if (insert_unfound)
		{
			svt theSvt;

			// First we lock.
			boost::mutex::scoped_lock aLock(mLock);
			// Then we see if someone else managed to sneak past us.
			i = mTable.get<0>().find(to_find);
			// If they did, use that value.
			if (i != mTable.end())
				return i->mId;
			// Otherwise, insert it.
			theSvt.mValue = to_find;
			theSvt.mId = ++mHighestKey;
			return mTable.insert(theSvt).first->mId;
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
	svt theSvt;
	theSvt.mValue = to_insert;
	theSvt.mId = ++mHighestKey;

	return mTable.insert(theSvt).first->mId;
}

void
string_table::insert_group(svt* pList, std::size_t size)
{
	boost::mutex::scoped_lock aLock(mLock);

	for (std::size_t i = 0; i < size; ++i)
	{
		if (mSetToLower)
			boost::to_lower(pList[i].mValue);
		// The keys don't have to be consecutive, so any time we find a key
		// that is too big, jump a few keys to avoid rewriting this on every item.
		if (pList[i].mId > mHighestKey)
			mHighestKey = pList[i].mId + 256;
		mTable.insert(pList[i]);
	}
	mSetToLower = false;
}

string_table::key
string_table::already_locked_insert(const std::string& to_insert, boost::mutex&)
{
	svt theSvt;
	theSvt.mValue = to_insert;
	theSvt.mId = ++mHighestKey;
	return mTable.insert(theSvt).first->mId;
}

