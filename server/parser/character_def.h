// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
//

#ifndef GNASH_CHARACTER_DEF_H
#define GNASH_CHARACTER_DEF_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "resource.h" // for inheritance from resource class

// Forward declarations
class tu_file;

namespace gnash {
	class character;
	class cache_options;
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
class character_def : public resource
{
private:
	int	m_id;
		
public:
	character_def()
		:
		m_id(-1),
		m_render_cache(NULL)
		{
		}
	
	virtual ~character_def() {}
	
	virtual void display(character* /*instance_info*/)
	{
	}

    	/// Return true if the specified point is on the interior of our shape.
	//
	/// Point coordinates are local coords (TWIPS)
	///
	virtual bool point_test_local(float /*x*/, float /*y*/)
	{
		return false;
	}

	virtual float get_height_local() const
	{
		return 0.0f;
	}

	virtual float get_width_local() const
	{
		return 0.0f;
	}

	/// Should stick the result in a boost::intrusive_ptr immediately.
	//
	/// default is to make a generic_character
	///
	virtual character* create_character_instance(character* parent,
			int id);
	
	// From resource interface.
	virtual character_def*	cast_to_character_def()
	{
		return this;
	}
	
	// Declared as virtual here because generic_character needs access to it
	virtual const rect&	get_bound() const = 0;
	
	//
	// Caching.
	//
	
	virtual void output_cached_data(tu_file* /*out*/,
			const cache_options& /*options*/)
	{
	}

	virtual void	input_cached_data(tu_file* /*in*/)
	{
	}
		
public:  
  
  /// Cache holder for renderer (contents depend on renderer handler)
  render_cache_manager* m_render_cache;
	
};


}	// namespace gnash

#endif // GNASH_CHARACTER_DEF_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
