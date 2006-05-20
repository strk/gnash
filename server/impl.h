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
//

#ifndef GNASH_IMPL_H
#define GNASH_IMPL_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gnash.h"
#include "action.h"
#include "types.h"
#include "log.h"
#include "container.h"
#include "utility.h"
#include "smart_ptr.h"
#include "movie_interface.h"
#include "character.h"
#include "resource.h" // for sound_sample inheritance
#include "swf/TagLoadersTable.h"

#include <cstdarg>
#include <cassert>

namespace gnash {

// Forward declarations
struct action_buffer;
struct bitmap_character_def;
struct bitmap_info;
struct character;
struct character_def;
struct display_info;
class font;
struct movie_root;
struct stream;
struct swf_event;

struct sound_sample : public resource //virtual public ref_counted
{
    virtual sound_sample*	cast_to_sound_sample() { return this; }
};

void save_extern_movie(movie_interface* m);


//v for extern movies

movie_interface *create_library_movie_inst(movie_definition* md);

movie_interface *get_current_root();
void set_current_root(movie_interface* m);
const char* get_workdir();
void set_workdir(const char* dir);
void delete_unused_root();

// Information about how to display a character.
struct display_info
{
    movie*	m_parent;
    int	m_depth;
    cxform	m_color_transform;
    matrix	m_matrix;
    float	m_ratio;
    uint16_t 	m_clip_depth;

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


//
// Loader callbacks.
//
	
// Register a loader function for a certain tag type.  Most
// standard tags are handled within gnash.  Host apps might want
// to call this in order to handle special tag types.

#if 0
/// Signature of an SWF tag loader
typedef void (*loader_function)(stream* input, int tag_type, movie_definition* m);

/// These are the registered tag loaders
extern hash<int, loader_function> s_tag_loaders;
#else
extern SWF::TagLoadersTable s_tag_loaders;
#endif

/// Register a tag loader for the given tag
void	register_tag_loader(SWF::tag_type t,
		SWF::TagLoadersTable::loader_function lf);
	
//
// swf_event
//

/// For embedding event handlers in place_object_2
struct swf_event
{
    // NOTE: DO NOT USE THESE AS VALUE TYPES IN AN
    // std::vector<>!  They cannot be moved!  The private
    // operator=(const swf_event&) should help guard
    // against that.

    event_id	m_event;
    action_buffer	m_action_buffer;
    as_value	m_method;

    swf_event()
	{
	}

    void	attach_to(character* ch) const
	{
	    ch->set_event_handler(m_event, m_method);
	}

    void read(stream* in, uint32_t flags);

private:
    // DON'T USE THESE
    swf_event(const swf_event& s) { assert(0); }
    void	operator=(const swf_event& s) { assert(0); }
};


}	// end namespace gnash


#endif // GNASH_IMPL_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
