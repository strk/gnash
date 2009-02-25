// string_table.cpp -- A shared string table for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "string_table.h"
#include <boost/algorithm/string/case_conv.hpp>

using namespace gnash;

const std::string string_table::mEmpty;

string_table::key
string_table::find(const std::string& t_f, bool insert_unfound)
{
	std::string t_fcase;
	const std::string *to_find = NULL;

	if (mCaseInsensitive)
	{
		t_fcase = t_f;
		boost::to_lower(t_fcase);
		to_find = &t_fcase;
	}
	else
		to_find = &t_f;
		
	// Empty strings all map to 0
	if (to_find->empty())
		return 0;

	table::nth_index<0>::type::iterator i =
		mTable.get<0>().find(*to_find);

	if (i == mTable.end())
	{
		if (insert_unfound)
		{
			svt theSvt;

			// First we lock.
			boost::mutex::scoped_lock aLock(mLock);
			// Then we see if someone else managed to sneak past us.
			i = mTable.get<0>().find(*to_find);
			// If they did, use that value.
			if (i != mTable.end())
				return i->mId;

			// Otherwise, insert it.
			theSvt.mValue = t_f;
			theSvt.mComp = *to_find;
			theSvt.mId = ++mHighestKey;
			mTable.insert(theSvt);
			return theSvt.mId;
		}
		else
			return 0;
	}

	return i->mId;
}

string_table::key
string_table::find_dot_pair(string_table::key left, string_table::key right, 
	bool insert_unfound)
{
	if (!right)
		return left;

	std::string isit = value(left) + "." + value(right);
	return find(isit, insert_unfound);
}

string_table::key
string_table::insert(const std::string& to_insert)
{
	boost::mutex::scoped_lock aLock(mLock);
	svt theSvt(to_insert, ++mHighestKey);

	return mTable.insert(theSvt).first->mId;
}

void
string_table::insert_group(svt* pList, std::size_t size)
{
	boost::mutex::scoped_lock aLock(mLock);

	for (std::size_t i = 0; i < size; ++i)
	{
		if (mSetToLower)
		{
			boost::to_lower(pList[i].mValue);
			boost::to_lower(pList[i].mComp);
		}
		else if (mCaseInsensitive)
		{
			boost::to_lower(pList[i].mComp);
		}

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
	svt theSvt (to_insert, ++mHighestKey);
	if (mCaseInsensitive)
		boost::to_lower(theSvt.mComp);
	return mTable.insert(theSvt).first->mId;
}

