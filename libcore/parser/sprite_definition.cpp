// sprite_definition.cpp:  for Gnash.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "smart_ptr.h" // GNASH_USE_GC
#include "MovieClip.h"
#include "sprite_definition.h"
#include "ControlTag.h" // for dtor visibility
#include "as_function.h" // for dtor visibility
#include "SWFStream.h" // for use
#include "GnashAlgorithm.h"

#include <vector>
#include <string>
#include <cassert>


// Define the following macro to get a dump the prototype 
// members of classes registered to definitions.
//#define DEBUG_REGISTER_CLASS 1

namespace gnash {

DisplayObject*
sprite_definition::createDisplayObject(DisplayObject* parent, int id) const
{
#ifdef DEBUG_REGISTER_CLASS
	log_debug(_("Instantiating sprite_def %p"), (void*)this);
#endif
	MovieClip* si = new MovieClip(this, parent->get_root(), parent, id);
	return si;
}

sprite_definition::~sprite_definition()
{
	// Release our playlist data.
	for (PlayListMap::iterator i=m_playlist.begin(), e=m_playlist.end();
            i != e; ++i)
	{
		PlayList& pl = i->second;
        deleteAllChecked(pl);
    }
}

/*private*/
// only called from constructors
void
sprite_definition::read(SWFStream& in, const RunInfo& runInfo)
{
    unsigned long tag_end = in.get_tag_end_position();

    in.ensureBytes(2);
    m_frame_count = in.read_u16();

    IF_VERBOSE_PARSE (
        log_parse(_("  frames = %d"), m_frame_count);
    );

	m_loading_frame = 0;

	while (in.tell() < tag_end) {

		SWF::TagType tag = in.open_tag();

		SWF::TagLoadersTable::loader_function lf = 0;

		if (tag == SWF::END) {

			if (in.tell() != tag_end) {
                IF_VERBOSE_MALFORMED_SWF(
                    // Safety break, so we don't read past
                    // the end of the  movie.
                    log_swferror(_("Hit end tag, "
                        "before the advertised DEFINESPRITE end; "
                        "stopping for safety."));
                    );
				in.close_tag();
                break;
			}
		}

		else if (tag == SWF::SHOWFRAME) {
			// show frame tag -- advance to the next frame.
            ++m_loading_frame;

			IF_VERBOSE_PARSE (
				log_parse(_("  show_frame %d/%d"
					" (sprite)"),
					m_loading_frame,
					m_frame_count);
		    	);

			if (m_loading_frame == m_frame_count) {

				// better break then sorry
				in.close_tag();
				if (in.open_tag() != SWF::END)
				{
					IF_VERBOSE_MALFORMED_SWF(
                        log_swferror(_("last SHOWFRAME of a "
                            "DEFINESPRITE tag "
                            "isn't followed by an END."
                            " Stopping for safety."));
                    );
					in.close_tag();
					return;
				}
			}
		}
		else if (_tag_loaders.get(tag, &lf)) {
		    // call the tag loader.  The tag loader should add
		    // DisplayObjects or tags to the movie data structure.
		    (*lf)(in, tag, *this, runInfo);
		}
		else {
			// no tag loader for this tag type.
			// FIXME, should this be a log_swferror instead?
                    log_error(_("*** no tag loader for type %d (sprite)"),
                              tag);
		}

		in.close_tag();
	}

    if (m_frame_count > m_loading_frame) {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("%d frames advertised in header, but "
                "only %d SHOWFRAME tags found in define "
                "sprite."), m_frame_count, m_loading_frame );
        );

        // this should be safe 
        m_loading_frame = m_frame_count;
    }

    IF_VERBOSE_PARSE(
        log_parse(_("  -- sprite END --"));
    );
}

void
sprite_definition::add_frame_name(const std::string& name)
{

    // It's fine for loaded frames to exceed frame count. Should be
    // adjusted at the end of parsing.
    _namedFrames.insert(std::make_pair(name, m_loading_frame));
}

bool
sprite_definition::get_labeled_frame(const std::string& label,
        size_t& frame_number) const
{
    NamedFrameMap::const_iterator it = _namedFrames.find(label);
    if ( it == _namedFrames.end() ) return false;
    frame_number = it->second;
    return true;
}

sprite_definition::sprite_definition(movie_definition& m, SWFStream& in, 
        const RunInfo& runInfo)
	:
	// FIXME: use a class-static TagLoadersTable for sprite_definition
	_tag_loaders(SWF::TagLoadersTable::getInstance()),
	m_movie_def(m),
	m_frame_count(0),
	m_loading_frame(0),
	registeredClass(0),
	_loadingSoundStream(-1)
{
	read(in, runInfo);
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
	log_debug(_("Registered class %p for sprite_def %p"), (void*)registeredClass.get(), (void*)this);
	as_object* proto = registeredClass->getPrototype().get();
	log_debug(_(" Exported interface: "));
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
