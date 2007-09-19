// string_table.h -- A shared string table for Gnash.
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

#ifndef GNASH_STRING_TABLE_H
#define GNASH_STRING_TABLE_H

// Thread Status: SAFE, except for group functions.
// The group functions may have strange behavior when trying to automatically
// lowercase the additions.

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/thread.hpp>
#include <string>

namespace gnash
{

class string_table;

// So many strings are duplicated (such as standard property names)
// that a string table could give significant memory savings.
class string_table
{
public:
	// A little helper for indexing.
	struct svt
	{
		std::string mValue;
		std::size_t mId;
	};

public:
	typedef boost::multi_index_container<svt,
		boost::multi_index::indexed_by<
			boost::multi_index::hashed_non_unique<
				boost::multi_index::member<svt, std::string, &svt::mValue> >,
			boost::multi_index::hashed_non_unique<
				boost::multi_index::member<svt, std::size_t, &svt::mId> > 
	> > table;

	typedef std::size_t key;

	// Find a string. If insert_unfound is true, the string will
	// be inserted if the value is not found in the table already.
	key find(const std::string& to_find, bool insert_unfound = true);

	// Find a string by its key.
	const std::string& value(key to_find)
	{ 
		if (mTable.empty())
			return mEmpty;
		table::nth_index<1>::type::iterator r = mTable.get<1>().find(to_find);
		return (r == mTable.get<1>().end()) ? mEmpty : r->mValue;
	}

	// Insert a string known to not be in the table. Return the new
	// index.
	key insert(const std::string& to_insert);

	// Insert a group of strings with their ids preset. This allows
	// for switches and enums and such, but be careful you don't set two
	// strings with the same id, as this does not check for such occurrences.
	void insert_group(svt* pList, std::size_t size);

	void lower_next_group() { mSetToLower = true; }

	// Insert a string when you will handle the locking yourself.
	// Use 'lock_mutex' to obtain the correct mutex to use for this.
	key already_locked_insert(const std::string& to_insert, boost::mutex& lock);

	boost::mutex& lock_mutex() { return mLock; }

	string_table() :
		mTable(),
		mLock(),
		mHighestKey(0),
		mSetToLower(false)
	{/**/}

private:
	table mTable;
	static std::string mEmpty; // The empty string, universally.
	boost::mutex mLock;
	key mHighestKey;
	bool mSetToLower; // If true, affects the next group addition.
};

}; /* namespace gnash */
#endif /* GNASH_STRING_TABLE_H */
