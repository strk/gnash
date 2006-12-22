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

// 
//
//

#ifndef GNASH_EXECUTE_TAG_H
#define GNASH_EXECUTE_TAG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstdarg>
#include <cassert>

namespace gnash {

// Forward declarations
class sprite_instance;

/// Execute tags include things that control the operation of the movie. 
//
/// Essentially, these are the events associated with a frame.
///
class execute_tag
{
public:
	virtual ~execute_tag()
	{
	}

	virtual void execute(sprite_instance* /*m*/)
	{
	}

	virtual void execute_state(sprite_instance* /*m*/)
	{
	}

	// Is the 'frame' arg is really needed ?
	virtual void execute_state_reverse(sprite_instance* m, int /*frame*/)
	{
		execute_state(m);
	}

	virtual bool	is_remove_tag() const { return false; }

	virtual bool	is_action_tag() const { return false; }

	/// \brief
	/// Return 16-bit depth and id of character packed into one 32-bit int
	/// IFF this is a replace or add tag, otherwise return 0.
	//
	/// The default implementation returns 0
	///
	virtual uint32	get_depth_id_of_replace_or_add_tag() const
	{
		return 0;
	}
};


} // namespace gnash


#endif // GNASH_EXECUTE_TAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
