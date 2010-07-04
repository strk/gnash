// string_table.h -- A shared string table for Gnash.
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

#ifndef GNASH_STRING_TABLE_H
#define GNASH_STRING_TABLE_H

// Thread Status: SAFE, except for group functions.
// The group functions may have strange behavior when trying to automatically
// lowercase the additions.

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/thread.hpp>
#include <string>
#include "dsodefs.h"

namespace gnash {

// So many strings are duplicated (such as standard property names)
// that a string table could give significant memory savings.
/// A general use string table.
class DSOEXPORT string_table
{
public:

    struct StringID {};
    struct StringValue {};

	/// A little helper for indexing.
	struct svt
	{
		svt(const std::string& val, std::size_t i)
            :
			value(val),
            id(i)
        {}

		std::string value;
		std::size_t id;
	};

	typedef boost::multi_index_container<svt,
		boost::multi_index::indexed_by<
			boost::multi_index::hashed_unique<
                boost::multi_index::tag<StringValue>,
				boost::multi_index::member<svt, std::string, &svt::value> >,
			boost::multi_index::hashed_unique<
                boost::multi_index::tag<StringID>,
				boost::multi_index::member<svt, std::size_t, &svt::id> > 
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

	/// Find a string by its key.
	///
	/// @return
	/// The string which matches key or "" if an invalid key is given.
	const std::string& value(key to_find)
	{
		if (_table.empty() || !to_find) return _empty;

		table::index<StringID>::type::iterator r =
            _table.get<StringID>().find(to_find);
		return (r == _table.get<StringID>().end()) ? _empty : r->value;
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
	///
	/// @param pList
	/// An array of svt objects, these should be fully constructed, including
	/// their ids.
    ///
	/// @param size
    /// Number of elements in the svt objects array
    ///
	void insert_group(const svt* pList, std::size_t size);

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

	/// Construct the empty string_table
	string_table()
        :
		_highestKey(0)
	{}

    key noCase(key a) const;

private:

	table _table;
	static const std::string _empty;
	boost::mutex _lock;
	std::size_t _highestKey;

    std::map<key, key> _caseTable;
};
    
bool noCaseEqual(string_table& st, string_table::key a, string_table::key b);

}
#endif 
