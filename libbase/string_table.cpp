// string_table.cpp -- A shared string table for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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
#ifdef HAVE_CONFIG_H
#include "gnashconfig.h" // GNASH_STATS_STRING_TABLE_NOCASE
#endif

#include <boost/algorithm/string/case_conv.hpp>

//#define DEBUG_STRING_TABLE 1
//#define GNASH_STATS_STRING_TABLE_NOCASE 1
//#define GNASH_PARANOIA_LEVEL 3

#ifdef GNASH_STATS_STRING_TABLE_NOCASE
# include "Stats.h"
#endif

namespace gnash {

const std::string string_table::_empty;

string_table::key
string_table::find(const std::string& t_f, bool insert_unfound)
{
    if (t_f.empty()) return 0;

	table::index<StringValue>::type::iterator i = 
        _table.get<StringValue>().find(t_f);

	if (i == _table.get<StringValue>().end()) {

		if (insert_unfound) {
			// First we lock.
			std::lock_guard<std::mutex> lock(_lock);
			// Then we see if someone else managed to sneak past us.
			i = _table.get<StringValue>().find(t_f);
			// If they did, use that value.
			if (i != _table.end()) return i->id;

            return already_locked_insert(t_f);
		}
        return 0;
	}

	return i->id;
}

string_table::key
string_table::insert(const std::string& to_insert)
{
    std::lock_guard<std::mutex> lock(_lock);
    return already_locked_insert(to_insert);
}

void
string_table::insert_group(const svt* l, std::size_t size)
{
    std::lock_guard<std::mutex> lock(_lock);
    for (std::size_t i = 0; i < size; ++i) {
        // Copy to avoid changing the original table.
        const svt s = l[i];

        // The keys don't have to be consecutive, so any time we find a key
        // that is too big, jump a few keys to avoid rewriting this on every
        // item.
       if (s.id > _highestKey) _highestKey = s.id + 256;
       _table.insert(s);
    }
    
    for (std::size_t i = 0; i < size; ++i) {
        const svt s = l[i];
        const std::string& t = boost::to_lower_copy(s.value);
        if (t != s.value) {
            _caseTable[s.id] = already_locked_insert(t);
        }
    }
#ifdef DEBUG_STRING_TABLE
    std::cerr << "string_table group insert end -- size is " << _table.size() << " _caseTable size is " << _caseTable.size() << std::endl; 
#endif


}

string_table::key
string_table::already_locked_insert(const std::string& to_insert)
{
	const key ret = _table.insert(svt(to_insert, ++_highestKey)).first->id;

#ifdef DEBUG_STRING_TABLE
    int tscp = 100; // table size checkpoint
    size_t ts = _table.size();
    if ( ! (ts % tscp) ) { std::cerr << "string_table size grew to " << ts << std::endl; }
#endif

    const std::string lower = boost::to_lower_copy(to_insert);

    // Insert the caseless equivalent if it's not there. We're locked for
    // the whole of this function, so we can do what we like.
    if (lower != to_insert) {

        // Find the caseless value in the table
        table::index<StringValue>::type::iterator it = 
            _table.get<StringValue>().find(lower);

        const key nocase = (it == _table.end()) ? 
            _table.insert(svt(lower, ++_highestKey)).first->id : it->id;

#ifdef DEBUG_STRING_TABLE
        ++ts;
        if ( ! (ts % tscp) ) { std::cerr << "string_table size grew to " << ts << std::endl; }
#endif // DEBUG_STRING_TABLE

        _caseTable[ret] = nocase;

    }

    return ret;
}

void
string_table::setHighestKnownLowercase(key k)
{
    _highestKnownLowercase = k;
}

string_table::key
string_table::noCase(key a) const
{
#ifdef GNASH_STATS_STRING_TABLE_NOCASE
    static stats::KeyLookup kcl("string_table::noCase(maplookups)", *this);
#endif // GNASH_STATS_STRING_TABLE_NOCASE

    // Avoid checking keys known to be lowercase
    if ( a <= _highestKnownLowercase ) {
#if GNASH_PARANOIA_LEVEL > 2
        assert(_caseTable.find(a) == _caseTable.end());
#endif
        return a;
    }

// MOVE this block around for special needs
#ifdef GNASH_STATS_STRING_TABLE_NOCASE
    kcl.check(a);
#endif 

    // TODO: an even/odd based rule to tell what's lowercase already
    //       would speed things up even for unknown 
    //       strings.

    std::map<key, key>::const_iterator i = _caseTable.find(a);
    if ( i != _caseTable.end() ) return i->second;

    return a;
}

bool
equal(string_table& st, string_table::key a, string_table::key b,
        bool caseless)
{
    if (a == b) return true;
    return caseless && (st.noCase(a) == st.noCase(b));
}
          
}
