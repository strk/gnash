// Stats.h -- classes for generic statistics gathering
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

#ifndef GNASH_STATS_H
#define GNASH_STATS_H


#include <map>
#include <iostream>
#include <iomanip>

#include "string_table.h"

namespace gnash {
namespace stats {

class KeyLookup {

    typedef std::map<string_table::key, unsigned long int> Stat;

public:

    /// @param label The label to print for dumps of this stat
    /// @param st The string table to use for resolving stats values
    /// @param dumpTrigger The number of calls to check() that should be
    ///                    triggering a dump
    /// @param restrict If non-zero dumpTrigger refers to this key lookups
    /// @param dumpCount Number of items to print in the dump (makes sense if not restricted)
    ///
    KeyLookup(const std::string& label, const string_table& st, int dumpTrigger=0,
            string_table::key restrict=0, int dumpCount=5)
        :
        _st(st),
        _dumpCount(dumpCount),
        _dumpTrigger(dumpTrigger),
        _label(label),
        _restrict(restrict)
    {}

    ~KeyLookup()
    {
        dump(_dumpCount);
    }

    void check(string_table::key k) {
        int gotTo = ++stat[k];
        if ( _restrict && k != _restrict ) return;
        if ( ! _dumpTrigger ) return;
        if ( ! ( gotTo % _dumpTrigger ) ) dump(_dumpCount);
    }

    void dump(int count) {
        typedef std::map<unsigned long int, string_table::key> Sorted;
        Sorted sorted;
        for (Stat::iterator i=stat.begin(), e=stat.end(); i!=e; ++i)
            sorted[i->second] = i->first;
        std::cerr << _label << " lookups: " << std::endl;
        for (Sorted::reverse_iterator i=sorted.rbegin(), e=sorted.rend();
                i!=e; ++i) {
            std::cerr
                      << std::setw(10)
                      << i->first
                      << ":"
                      << _st.value(i->second) << "("
                      << i->second << ")"
                      << std::endl;
            if ( ! --count ) break;
        }
    }

private:

    Stat stat;
    const string_table& _st;
    int _dumpCount;
    int _dumpTrigger;
    std::string _label;
    string_table::key _restrict;


};

} // namespace gnash.stats
} // namespace gnash

#endif 
