// sprite_definition.cpp:  for Gnash.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sprite_instance.h"
#include "sprite_definition.h"
#include "execute_tag.h" // for dtor visibility
#include "as_function.h" // for dtor visibility

#include <vector>
#include <string>
#include <cassert>

using namespace std;

namespace gnash {

character*
sprite_definition::create_character_instance(character* parent,
		int id)
{
#ifdef DEBUG_REGISTER_CLASS
	log_msg(_("Instantiating sprite_def %p"), (void*)this);
#endif
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
	log_parse(_("  frames = " SIZET_FMT), m_frame_count);
		);

	m_loading_frame = 0;

	while ((uint32_t) in->get_position() < (uint32_t) tag_end)
	{
		SWF::tag_type tag_type = in->open_tag();

		SWF::TagLoadersTable::loader_function lf = NULL;

		IF_VERBOSE_MALFORMED_SWF(
		if (tag_type == SWF::DEFINESPRITE)
		{
			log_swferror(_("DefineSprite tag inside sprite "
				"definition"));
		}
		);

		if (tag_type == SWF::SHOWFRAME)
		{
			// show frame tag -- advance to the next frame.
		    	++m_loading_frame;

			// Close current frame definition in Timeline object
			_timeline.closeFrame();


			IF_VERBOSE_PARSE (
				log_parse(_("  show_frame "
					SIZET_FMT "/" SIZET_FMT
					" (sprite)"),
					m_loading_frame,
					m_frame_count);
		    	);

			if ( m_loading_frame == m_frame_count )
			{
				// better break then sorry

				in->close_tag();
				if ( in->open_tag() != SWF::END )
				{
					IF_VERBOSE_MALFORMED_SWF(
					log_swferror(_("last SHOWFRAME of a "
						"DEFINESPRITE tag "
						"isn't followed by an END."
						" Stopping for safety."));
					);
					in->close_tag();
					return;
				}
			}
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
			// FIXME, should this be a log_swferror instead?
                    log_error(_("*** no tag loader for type %d (sprite)"),
                              tag_type);
		}

		in->close_tag();
	}

		IF_VERBOSE_PARSE (
	log_parse(_("  -- sprite END --"));
		);
}

/*virtual*/
void
sprite_definition::add_frame_name(const std::string& name)
{
	//log_msg(_("labelframe: frame %d, name %s"), m_loading_frame, name);
	assert(m_loading_frame < m_frame_count);
    m_named_frames[name] = m_loading_frame;
}

bool
sprite_definition::get_labeled_frame(const std::string& label, size_t& frame_number)
{
    NamedFrameMap::iterator it = m_named_frames.find(label);
    if ( it == m_named_frames.end() ) return false;
    frame_number = it->second;
    return true;
}

sprite_definition::sprite_definition(movie_definition* m, stream* in)
	:
	// FIXME: use a class-static TagLoadersTable for sprite_definition
	_tag_loaders(SWF::TagLoadersTable::getInstance()),
	m_movie_def(m),
	m_frame_count(0),
	m_loading_frame(0),
	registeredClass(0)
{
	assert(m_movie_def);

	// create empty sprite_definition (it is used for createEmptyMovieClip() method)
	if (in == NULL)
	{
		m_frame_count = 1;
		m_loading_frame = 1;

		m_playlist.resize(1);
		m_playlist[0].push_back(new execute_tag());
	}
	else
	{
		read(in);
	}
}

/*
 * This function is not inlined to avoid having to include as_function.h
 * from sprite_definition.h. We need as_function.h for visibility of
 * as_function destructor by boost::intrusive_ptr
 */
void
sprite_definition::registerClass(as_function* the_class)
{
	registeredClass = the_class;
#ifdef DEBUG_REGISTER_CLASS
	log_msg(_("Registered class %p for sprite_def %p"), (void*)registeredClass.get(), (void*)this);
	as_object* proto = registeredClass->getPrototype();
	log_msg(_(" Exported interface: "));
	proto->dump_members();
#endif
}

#ifdef GNASH_USE_GC
void
sprite_definition::markReachableResources() const
{
	if ( registeredClass.get() ) registeredClass->setReachable();
}
#endif // GNASH_USE_GC

} // namespace gnash
