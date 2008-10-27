// string_table.h -- A shared string table for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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
#include "dsodefs.h"

namespace gnash
{

class string_table;

// So many strings are duplicated (such as standard property names)
// that a string table could give significant memory savings.
/// A general use string table.
class DSOEXPORT string_table
{
public:
	/// A little helper for indexing.
	struct svt
	{
		std::string mValue;
		std::size_t mId;
		std::string mComp;

		svt() : mValue(""), mId(0), mComp("") {/**/}

		svt(const std::string &val, std::size_t id) :
			mValue(val), mId(id), mComp(val) {/**/}
	};

public:
	typedef boost::multi_index_container<svt,
		boost::multi_index::indexed_by<
			boost::multi_index::hashed_non_unique<
				boost::multi_index::member<svt, std::string, &svt::mComp> >,
			boost::multi_index::hashed_non_unique< // caseless
				boost::multi_index::member<svt, std::size_t, &svt::mId> > 
	> > table;

	typedef std::size_t key;

	/// \brief
	/// Find a string. If insert_unfound is true, the string will
	/// be inserted if the value is not found in the table already.
	/// @param to_find
	/// The string to be found. Case-sensitive comparison using < operator
	///
	/// @param insert_unfound
	/// If this is set to false, a search is performed, but no update.
	/// By update, any unfound string is added to the table.
	///
	/// @return
	/// A key which can be used in value or 0 if the string is
	/// not yet in the table and insert_unfound was false.
	key find(const std::string& to_find, bool insert_unfound = true);

	/// \brief
	/// Find a string which is the concatentation of two known strings
	/// with a dot between them. (Used for namespaces.)
	/// Otherwise, just like find.
	key find_dot_pair(key left, key right, bool insert_unfound = true);

	/// Find a string by its key.
	///
	/// @return
	/// The string which matches key or "" if an invalid key is given.
	const std::string& value(key to_find)
	{
		if (mTable.empty() || !to_find)
			return mEmpty;
		table::nth_index<1>::type::iterator r = 
			mTable.get<1>().find(to_find);
		return (r == mTable.get<1>().end()) ? mEmpty : r->mValue;
	}

	/// \brief
	/// Force insert a string with auto-assigned id. Does not prevent
	/// duplicate insertions.
	///
	/// @return The assigned key
	key insert(const std::string& to_insert);

	/// Insert a group of strings with their ids preset.
    //
    /// This allows
	/// for switches and enums and such, but be careful you don't set two
	/// strings with the same id, as this does not check for such occurrences.
	/// Converts the strings to lower case if mSetToLower is true.
	/// In any case, sets mSetToLower to false at the end.
	///
	/// @param pList
	/// An array of svt objects, these should be fully constructed, including
	/// their ids.
    ///
	/// @param size
    /// Number of elements in the svt objects array
    ///
	void insert_group(svt* pList, std::size_t size);

	/// \brief
	/// Call this just before calling insert_group if the next group should
	/// be set to lower_case before addition.
	void lower_next_group() { mSetToLower = true; }

	/// Insert a string when you will handle the locking yourself.
    //
	/// @param to_insert
    /// String to insert
    ///
	/// @param lock
	/// Use lock_mutex to obtain the correct mutex to use for this -- using
	/// a different mutex will not be thread safe.
	///
	/// @return The assigned key
	key already_locked_insert(const std::string& to_insert, boost::mutex& lock);

	/// @return A mutex which can be used to lock the string table to inserts.
	boost::mutex& lock_mutex() { return mLock; }

	/// Make the comparisons case-insensitive.
	void set_insensitive() { mCaseInsensitive = true; }

	/// Construct the empty string_table
	string_table() :
		mTable(),
		mLock(),
		mHighestKey(0),
		mSetToLower(false),
		mCaseInsensitive(false)
	{/**/}

private:
	table mTable;
	static const std::string mEmpty; // The empty string, universally.
	boost::mutex mLock;
	std::size_t mHighestKey;
	bool mSetToLower; // If true, affects the next group addition.
	bool mCaseInsensitive;
};

} /* namespace gnash */
#endif /* GNASH_STRING_TABLE_H */
