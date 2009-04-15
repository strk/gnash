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

#ifndef GNASH_DEFINITION_TAG_H
#define GNASH_DEFINITION_TAG_H

#include "ExportableResource.h"

#include <boost/cstdint.hpp>
#include <vector>
#include <boost/noncopyable.hpp>

// Forward declarations

namespace gnash {
	class DisplayObject;
	class SWFMatrix;
	class rect;
    namespace SWF {
        class TextRecord;
    }
}

namespace gnash {
namespace SWF {

/// Immutable data representing the definition of a movie display element.
class DefinitionTag : public ExportableResource, boost::noncopyable
{
public:

	virtual ~DefinitionTag() {};
	
	/// Create a DisplayObject with the given parent and id
	//
    /// Calling this function creates a new DisplayObject from the
    /// DefinitionTag and adds it as a child of the specified parent
    /// DisplayObject.
	virtual DisplayObject* createDisplayObject(DisplayObject* parent,
            int id) const = 0;
	
};

} // namespace SWF
} // namespace gnash

#endif 

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
