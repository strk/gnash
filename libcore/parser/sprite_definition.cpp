// sprite_definition.cpp:  for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include "RunResources.h"
#include "MovieClip.h"
#include "sprite_definition.h"
#include "ControlTag.h" // for dtor visibility
#include "as_function.h"
#include "SWFStream.h"
#include "GnashAlgorithm.h"
#include "SWFParser.h"
#include "namedStrings.h"
#include "Global_as.h"

#include <vector>
#include <string>
#include <cassert>


namespace gnash {

DisplayObject*
sprite_definition::createDisplayObject(Global_as& gl, DisplayObject* parent)
    const
{
    // Should not call MovieClip constructor (probably), but should
    // attach MovieClip.prototype
    as_object* obj = getObjectWithPrototype(gl, NSV::CLASS_MOVIE_CLIP);
    DisplayObject* mc = new MovieClip(obj, this, parent->get_root(), parent);
	return mc;
}

sprite_definition::~sprite_definition()
{
}

/*private*/
// only called from constructors
void
sprite_definition::read(SWFStream& in, const RunResources& runResources)
{
    const size_t tag_end = in.get_tag_end_position();

    in.ensureBytes(2);
    m_frame_count = in.read_u16();

    IF_VERBOSE_PARSE (
        log_parse(_("  frames = %d"), m_frame_count);
    );

	m_loading_frame = 0;

    SWFParser parser(in, this, runResources);

    // This can throw a ParserException; we will let the SWFMovieDefintion
    // catch it, as a failure means the whole stream is invalid.
    parser.read(tag_end - in.tell());

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
        const RunResources& runResources, boost::uint16_t id)
	:
    movie_definition(id),
	m_movie_def(m),
	m_frame_count(0),
	m_loading_frame(0),
	_loadingSoundStream(-1)
{
	read(in, runResources);
}

} // namespace gnash
