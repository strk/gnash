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

#ifndef GNASH_DEFINITION_TAG_H
#define GNASH_DEFINITION_TAG_H


#include <boost/noncopyable.hpp>
#include <cstdint>

#include "ControlTag.h"
#include "dsodefs.h" // for DSOTEXPORT

// Forward declarations

namespace gnash {
	class DisplayObject;
	class Global_as;
	class SWFMatrix;
	class SWFRect;
    namespace SWF {
        class TextRecord;
    }
}

namespace gnash {
namespace SWF {

/// Immutable data representing the definition of a movie display element.
//
/// TODO: rename this class so it's not the same as the SWF spec. It doesn't
/// exactly correspond to the DefinitionTag defined there.
class DefinitionTag : public ControlTag
{
public:

	virtual ~DefinitionTag() {};
	
	/// Create a DisplayObject with the given parent.
	//
    /// This function will determine the correct prototype and associated
    /// object using the passed global.
    //
    /// @param gl       The global object used to set prototype and
    ///                 associated object.
    //
    /// Calling this function creates a new DisplayObject from the
    /// DefinitionTag and adds it as a child of the specified parent
    /// DisplayObject.
	virtual DisplayObject* createDisplayObject(Global_as& gl,
            DisplayObject* parent) const = 0;

    /// Executing a DefinitionTag adds its id to list of known characters
    //
    /// The process is different for imported DefinitionTags, which are added
    /// with a new id.
	DSOTEXPORT virtual void executeState(MovieClip* m,  DisplayList& /*dlist*/) const;

    /// The immutable id of the DefinitionTag.
    //
    /// @return     the id of the DefinitionTag as parsed from a SWF.
    std::uint16_t id() const {
        return _id;
    }

protected:

    DefinitionTag(std::uint16_t id) : _id(id) {}

private:

    const std::uint16_t _id;
	
};

} // namespace SWF
} // namespace gnash

#endif 

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
