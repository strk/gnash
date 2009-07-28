// TextSnapshot_as.h:  ActionScript 3 "TextSnapshot" class, for Gnash.
//
//   Copyright (C) 2009 Free Software Foundation, Inc.
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

#ifndef GNASH_ASOBJ3_TEXTSNAPSHOT_H
#define GNASH_ASOBJ3_TEXTSNAPSHOT_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "as_object.h"


namespace gnash {

	class StaticText;
    class Array_as;
    namespace SWF {
        class TextRecord;
    }

// Forward declarations
class TextSnapshot_as: public as_object
{

public:

    typedef std::vector<const SWF::TextRecord*> Records;

    /// Should remain in the order of insertion
    /// We should only ever iterate from begin to end, so there's no
    /// performance issue.
    typedef std::vector<std::pair<StaticText*, Records> > TextFields;

    /// Construct a TextSnapshot_as from a MovieClip.
    //
    /// @param mc       The MovieClip to search for static text. If 0, the
    ///                 TextSnapshot is invalid, which should be reflected in
    ///                 AS return values.
    TextSnapshot_as(const MovieClip* mc);

    static void init(as_object& where, const ObjectURI& uri);

    std::string getText(boost::int32_t start, boost::int32_t end,
            bool nl) const;

    boost::int32_t findText(boost::int32_t start, const std::string& text,
            bool ignoreCase) const;

    bool valid() const { return _valid; }

    size_t getCount() const { return _count; }

    void setSelected(size_t start, size_t end, bool selected);
    
    bool getSelected(size_t start, size_t end) const;

    std::string getSelectedText(bool newlines) const;

    void getTextRunInfo(size_t start, size_t end, Array_as& ri) const;

protected:

    void markReachableResources() const;

private:

    /// Generate a string from the TextRecords in this TextSnapshot.
    //
    /// @param to           The string to write to
    /// @param newline      If true, newlines are written after every
    ///                     StaticText in this TextSnapshot
    /// @param selectedOnly Only write DisplayObject that are selected to.
    /// @param start        The start index
    /// @param len          The number of StaticText DisplayObjects to traverse.
    ///                     This includes non-selected DisplayObjects.
    void makeString(std::string& to, bool newline = false,
            bool selectedOnly = false,
            std::string::size_type start = 0,
            std::string::size_type len = std::string::npos) const;

    TextFields _textFields;

    /// Whether the object is valid, i.e. it was constructed with a MovieClip.
    //
    /// This should be deducible from another member, but since there seems
    /// to be no point in storing the MovieClip this bool will do instead.
    const bool _valid;

    /// The number of DisplayObjects
    //
    /// There is no need to store this, but it is quicker than counting
    /// afresh every time.
    const size_t _count;

};

} // gnash namespace

// GNASH_ASOBJ3_TEXTSNAPSHOT_H
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

