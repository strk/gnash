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

#ifndef GNASH_CHARACTER_DEF_H
#define GNASH_CHARACTER_DEF_H

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


class render_cache_manager; 


/// Immutable data representing the template of a movie element.
//
/// This is not really a public interface.  It's here so it
/// can be mixed into movie_definition and sprite_definition,
/// without using multiple inheritance.
///
class character_def : public ExportableResource, boost::noncopyable
{
public:

    character_def()
		:
		m_render_cache(0),
		_id(-1)
    {
    }

	
	virtual ~character_def();
	
	/// Should stick the result in a boost::intrusive_ptr immediately.
	//
	/// default is to make a DisplayObject
	///
	virtual DisplayObject* createDisplayObject(DisplayObject* parent, int id) = 0;
	
	// Declared as virtual here because DisplayObject needs access to it
	virtual const rect&	get_bound() const = 0;
	
    /// Cache holder for renderer (contents depend on renderer handler)
    /// Will be deleted by destructor of the character_def.
    /// We could store by auto_ptr, but I'm afraid that would mean
    /// including render_handler.h in this header, which I don't like.
    /// (REF: PIMPL)
    ///
    render_cache_manager* m_render_cache;

private:

    int	_id;
	
};


}	// namespace gnash

#endif 

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
