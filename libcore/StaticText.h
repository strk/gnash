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
#include "shape_character_def.h" // for add_invalidated_bounds 
#include "DisplayObject.h"

#include <boost/scoped_ptr.hpp>
#include <boost/dynamic_bitset.hpp>
#include <cassert>

namespace gnash {

    // Forward declarations
    class character_def;

    namespace SWF {
        class TextRecord;
    }

}

namespace gnash {

/// For characters that don't store unusual state in their instances.
//
/// @@AFAICT this is only used for shape characters
///
class StaticText : public DisplayObject
{
public:

	StaticText(character_def* def, character* parent, int id)
		:
        DisplayObject(parent, id),
        _def(def)
	{
        assert(_def);
	}

    virtual DisplayObject* getStaticText(
            std::vector<const SWF::TextRecord*>& to);

	virtual void display();

    const boost::dynamic_bitset<>& getSelected() const {
        return _selectedText;
    }

protected:

    character_def* getDefinition() const {
        return _def.get();
    }

#ifdef GNASH_USE_GC
	/// Mark reachabe resources (for the GC)
	//
	/// These are:
	///	- this char's definition (m_def)
	///
	void markReachableResources() const
	{
		assert(isReachable());
		markCharacterReachable();
	}
#endif // GNASH_USE_GC

private:

    /// A bitmask indicating which static text characters are selected
    //
    /// This is only present for static text fields, and only after
    /// a TextSnapshot has queried the character for text.
    boost::dynamic_bitset<> _selectedText;

    const boost::intrusive_ptr<character_def> _def;

};


}	// end namespace gnash


#endif // GNASH_GENERIC_CHARACTER_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
