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

#ifndef GNASH_STATIC_TEXT_H
#define GNASH_STATIC_TEXT_H

#include "smart_ptr.h" // GNASH_USE_GC
#include "character.h" // for inheritance
#include "DisplayObject.h"
#include "swf/DefineTextTag.h"

#include <boost/dynamic_bitset.hpp>
#include <cassert>

// Forward declarations
namespace gnash {
    class character_def;
    namespace SWF {
        class TextRecord;
    }
}

namespace gnash {

/// Static text fields, SWF-defined and read-only.
class StaticText : public DisplayObject
{
public:

	StaticText(SWF::DefineTextTag* def, character* parent, int id)
		:
        DisplayObject(parent, id),
        _def(def)
	{
        assert(_def);
	}

    /// Return a pointer to this if our definition contains any static text.
    //
    /// This is non-const because a TextSnapshot needs to add selection and
    /// color information to this StaticText
    //
    /// @param to       A vector of pointers to TextRecords containing text.
    /// @param numChars The total number of characters in all TextRecords is
    ///                 written to this variable.
    virtual StaticText* getStaticText(std::vector<const SWF::TextRecord*>& to,
            size_t& numChars);

	virtual void display();

    void setSelected(size_t pos, bool selected) {
        _selectedText.set(pos, selected);
    }

    const boost::dynamic_bitset<>& getSelected() const {
        return _selectedText;
    }

    void setSelectionColor(boost::uint32_t color);

protected:

    character_def* getDefinition() const {
        return _def.get();
    }

#ifdef GNASH_USE_GC
	/// Mark reachable resources (for the GC)
	void markReachableResources() const
	{
		assert(isReachable());
        _def->setReachable();
		markCharacterReachable();
	}
#endif

private:

    const boost::intrusive_ptr<SWF::DefineTextTag> _def;

    /// A bitmask indicating which static text characters are selected
    //
    /// This is only present for static text fields, and only after
    /// a TextSnapshot has queried the character for text.
    boost::dynamic_bitset<> _selectedText;

    /// The color of the background for selected characters.
    //
    /// This is alawys opaque.
    rgba _selectionColor;

};


}	// end namespace gnash


#endif // GNASH_GENERIC_CHARACTER_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
