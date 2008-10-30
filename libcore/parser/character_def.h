// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

// Forward declarations

namespace gnash {
	class character;
	class SWFMatrix;
	class rect;
}

namespace gnash {


class render_cache_manager; 


/// Immutable data representing the template of a movie element.
//
/// This is not really a public interface.  It's here so it
/// can be mixed into movie_definition and sprite_definition,
/// without using multiple inheritance.
///
class character_def : public ExportableResource
{
private:
	int	m_id;
		
	// don't assign-to
	character_def& operator= (const character_def&);
public:
	character_def()
		:
		m_id(-1),
		m_render_cache(NULL)
		{
		}

	
	virtual ~character_def();
	
	virtual void display(character* /*instance_info*/)
	{
	}

    	/// Return true if the specified point is on the interior of our shape.
	//
	/// Point coordinates are local coords (TWIPS)
	///
	/// @param wm
	///	Current world SWFMatrix of the instance we want to check.
	///	This is needed to properly scale non-scalable strokes.
	///
	virtual bool point_test_local(boost::int32_t /*x*/, boost::int32_t /*y*/, SWFMatrix& /*wm*/)
	{
		return false;
	}

	/// Should stick the result in a boost::intrusive_ptr immediately.
	//
	/// default is to make a generic_character
	///
	virtual character* create_character_instance(character* parent,
			int id);
	
	// Declared as virtual here because generic_character needs access to it
	virtual const rect&	get_bound() const = 0;
	
public:  
  
  /// Cache holder for renderer (contents depend on renderer handler)
  /// Will be deleted by destructor of the character_def.
  /// We could store by auto_ptr, but I'm afraid that would mean
  /// including render_handler.h in this header, which I don't like.
  /// (REF: PIMPL)
  ///
  render_cache_manager* m_render_cache;

protected:

	/// Copy a character definition
	//
	/// The copy will have a NULL render cache object.
	/// The only known use of copy constructor is from
	/// duplicateMovieClip, in particular during copy
	/// of the drawable object, which is a subclass
	/// of a shape_character_def
	///
	/// The choice of NOT copying the cache manager
	/// is a choice of simplicity. We can't copy the
	/// pointer as the character_def destructor will
	/// destroy it, and we don't want to destroy it twice.
	/// We don't want to make a copy of the whole cache
	/// as it might be a waste of resource, we don't want
	/// to share ownership as some character_def ended up
	/// NOT being immutable any more !! :(
	///
	/// By setting the cache to NULL we'll leave reconstruction
	/// of a cache to the renderers.
	///
	/// TODO: improve by implementing copy on write for the cache ?
	///
	character_def(const character_def& o)
		:
		m_id(o.m_id),
		m_render_cache(NULL)
	{}

	
};


}	// namespace gnash

#endif // GNASH_CHARACTER_DEF_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
