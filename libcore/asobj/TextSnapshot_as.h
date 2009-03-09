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
//

#ifndef GNASH_ASOBJ_TEXTSNAPSHOT_H
#define GNASH_ASOBJ_TEXTSNAPSHOT_H

#include "as_object.h"

namespace gnash {

class generic_character;

class TextSnapshot_as: public as_object
{

public:

    /// Should remain in the order of insertion
    /// We should only ever iterate from begin to end, so there's no
    /// performance issue.
    typedef std::vector<std::pair<generic_character*, std::string> > TextFields;

    TextSnapshot_as(const MovieClip* mc);

    std::string::size_type getCount() {
        std::string snapshot;
        makeString(snapshot);
        return snapshot.size();
    }

    const std::string getText(boost::int32_t start, boost::int32_t end,
            bool nl) const;

    boost::int32_t findText(boost::int32_t start, const std::string& text,
            bool ignoreCase) const;

    static void init(as_object& global);

    static void construct(const std::string& snapshot);

    bool valid() const { return _valid; }

protected:

    void markReachableResources() const;

private:

    void makeString(std::string& to, bool newline = false) const;

    TextFields _textFields;

    /// Whether the object is valid, i.e. it was constructed with a MovieClip.
    //
    /// This should be deducible from another member, but since there seems
    /// to be no point in storing the MovieClip this bool will do instead.
    bool _valid;

};

} // end of gnash namespace

#endif

