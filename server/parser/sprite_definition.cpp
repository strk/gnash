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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// This needs to be included first for NetBSD systems or we get a weird
// problem with pthread_t being defined too many times if we use any
// STL containers.
#ifdef HAVE_PTHREADS
#include <pthread.h>
#endif

#include "sprite_instance.h"
#include "sprite_definition.h"
#include "execute_tag.h" // for dtor visibility

#include <vector>
#include <string>
#include <cassert>

using namespace std;

namespace gnash {

character*
sprite_definition::create_character_instance(character* parent,
		int id)
{
	sprite_instance* si = new sprite_instance(this,
		parent->get_root(), parent, id);
	return si;
}

sprite_definition::~sprite_definition()
{
	// Release our playlist data.
	for (int i = 0, n = m_playlist.size(); i < n; i++)
	{
		for (int j = 0, m = m_playlist[i].size(); j < m; j++)
		{
		    delete m_playlist[i][j];
		}
	}
}

/*private*/
// only called from constructors
void
sprite_definition::read(stream* in)
{
	int tag_end = in->get_tag_end_position();

	m_frame_count = in->read_u16();

	// ALEX: some SWF files have been seen that have 0-frame sprites.
	// The Macromedia player behaves as if they have 1 frame.
	if (m_frame_count < 1)
	{
		m_frame_count = 1;
	}

	// need a playlist for each frame
	m_playlist.resize(m_frame_count);

		IF_VERBOSE_PARSE (
	log_parse("  frames = %u", m_frame_count);
		);

	m_loading_frame = 0;

	while ((uint32_t) in->get_position() < (uint32_t) tag_end)
	{
		SWF::tag_type tag_type = in->open_tag();

		SWF::TagLoadersTable::loader_function lf = NULL;

		if (tag_type == SWF::DEFINESPRITE)
		{
			log_error("DefineSprite tag inside sprite "
				"definition - Malformed SWF!");
		}

		if (tag_type == SWF::SHOWFRAME)
		{
			// show frame tag -- advance to the next frame.
			IF_VERBOSE_PARSE (
		    log_parse("  show_frame (sprite)");
		    	);
		    m_loading_frame++;
		}
		else if (_tag_loaders.get(tag_type, &lf))
		{
		    // call the tag loader.  The tag loader should add
		    // characters or tags to the movie data structure.
		    (*lf)(in, tag_type, this);
		}
		else
		{
			// no tag loader for this tag type.
                    log_error("*** no tag loader for type %d (sprite)",
                              tag_type);
		}

		in->close_tag();
	}

		IF_VERBOSE_PARSE (
	log_parse("  -- sprite END --");
		);
}

/*virtual*/
void
sprite_definition::add_frame_name(const char* name)
{
	assert((int)m_loading_frame >= 0 && m_loading_frame < m_frame_count);

	tu_string n = name;
	size_t currently_assigned = 0;
	if (m_named_frames.get(n, &currently_assigned) == true)
	{
		log_error("add_frame_name(%d, '%s') -- frame name "
			"already assigned to frame %u; overriding\n",
			m_loading_frame,
			name, currently_assigned);
	}

	// stores 0-based frame #
	m_named_frames[n] = m_loading_frame;
}

sprite_definition::sprite_definition(movie_definition* m, stream* in)
	:
	_tag_loaders(s_tag_loaders),  // FIXME: use a class-static TagLoadersTable for sprite_definition
	m_movie_def(m),
	m_frame_count(0),
	m_loading_frame(0)
{
	// create empty sprite_definition (it is used for createEmptyMovieClip() method)
	if (m_movie_def == NULL && in == NULL)
	{
		m_frame_count = 1;
		m_loading_frame = 1;

		m_playlist.resize(1);
		m_playlist[0].push_back(new execute_tag());
	}
	else
	{
		assert(m_movie_def);
		read(in);
	}
}



} // namespace gnash
