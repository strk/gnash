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


#ifndef GNASH_IMPL_H
#define GNASH_IMPL_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "tu_config.h"

#include "gnash.h"
//#include "cxform.h" // for composition of display_info
//#include "matrix.h" // for display_info composition
#include "types.h"
#include "utility.h"
#include "smart_ptr.h"
#include "swf/TagLoadersTable.h"

#include <cstdarg>
#include <cassert>

namespace gnash {

// Forward declarations
//class action_buffer;
class bitmap_character_def;
class bitmap_info;
class character;
class character_def;
class font;
class movie_root;
class stream;
class sprite_instance;

DSOEXPORT void save_extern_movie(sprite_instance* m);


// for extern movies

DSOEXPORT movie_root* get_current_root();
DSOEXPORT const char* get_workdir();
DSOEXPORT void set_workdir(const char* dir);

#if 0
// Information about how to display a character.
class display_info
{
public:
    movie*	m_parent;
    int	m_depth;
    cxform	m_color_transform;
    matrix	m_matrix;
    float	m_ratio;
    boost::uint16_t 	m_clip_depth;

    display_info()
	:
	m_parent(NULL),
	m_depth(0),
	m_ratio(0.0f),
	m_clip_depth(0)
	{
	}

    void	concatenate(const display_info& di)
	// Concatenate the transforms from di into our
	// transforms.
	{
	    m_depth = di.m_depth;
	    m_color_transform.concatenate(di.m_color_transform);
	    m_matrix.concatenate(di.m_matrix);
	    m_ratio = di.m_ratio;
	    m_clip_depth = di.m_clip_depth;
	}
};
#endif


//
// Loader callbacks.
//
	
// Register a loader function for a certain tag type.  Most
// standard tags are handled within gnash.  Host apps might want
// to call this in order to handle special tag types.

/// Register a tag loader for the given tag
void	register_tag_loader(SWF::tag_type t,
		SWF::TagLoadersTable::loader_function lf);
	
}	// end namespace gnash


#endif // GNASH_IMPL_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
