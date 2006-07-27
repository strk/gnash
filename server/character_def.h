// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
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
	struct cache_options;
	//class sprite_instance;
}

namespace gnash {

/// Immutable data representing the template of a movie element.
//
/// This is not really a public interface.  It's here so it
/// can be mixed into movie_definition and sprite_definition,
/// without using multiple inheritance.
///
struct character_def : public resource
{
private:
	int	m_id;
	
public:
	character_def()
		:
		m_id(-1)
		{
		}
	
	virtual ~character_def() {}
	
	virtual void display(character* /*instance_info*/)
	{
	}

	virtual bool point_test_local(float /*x*/, float /*y*/)
	{
		return false;
	}

	virtual float get_height_local()
	{
		return 0.0f;
	}

	virtual float get_width_local()
	{
		return 0.0f;
	}
	
	/// Should stick the result in a smart_ptr immediately.
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
};


}	// namespace gnash

#endif // GNASH_CHARACTER_DEF_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
