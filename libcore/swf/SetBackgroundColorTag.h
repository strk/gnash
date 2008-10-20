// 
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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

#ifndef GNASH_SWF_SETBACKGROUNDCOLOR_TAG_H
#define GNASH_SWF_SETBACKGROUNDCOLOR_TAG_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "ControlTag.h" // for inheritance
#include "swf.h" // for tag_type definition
#include "sprite_instance.h" // for inlines (execute)
#include "movie_definition.h" // for inlines (loader)
#include "log.h" // for log_parse
#include "RGBA.h" // for rgba class
#include "utility.h" // for frnd

// Forward declarations
namespace gnash {
	class SWFStream;
	class movie_definition;
}

namespace gnash {
namespace SWF {

/// SWF Tag SetBackgroundColor (9)
//
class SetBackgroundColorTag : public ControlTag
{

private:

	/// Actual color. So far only RGB values are used,
	/// as alpha is not encoded in the tag itself
	rgba m_color;

	/// Read SetBackgroundColorTag from the given stream
	//
	/// Tag header is assumed to have been read already
	///
	/// Can throw ParserException on premature end of input stream
	///
	void read(SWFStream& in)
	{
		// may throw ParserException
		m_color.read_rgb(in);

		IF_VERBOSE_PARSE (
		log_parse(_("  SetBackgroundColor: %s"), m_color.toString());
		);
	}


public:

	/// \brief
	/// Construct a SetBackgroundColorTag by reading it 
	/// from the given SWF stream.
	//
	/// Tag header is assumed to have been read already
	///
	/// Can throw ParserException on premature end of input stream
	///
	SetBackgroundColorTag(SWFStream& in)
	{
		read(in);
	}

	void execute(sprite_instance* m, DisplayList& /*dlist*/) const
	{
		float	current_alpha = m->get_background_alpha();
		rgba newcolor = m_color; // to avoid making m_color mutable
		newcolor.m_a = utility::frnd(current_alpha * 255.0f);
		m->set_background_color(newcolor);
	}

	void execute_state(sprite_instance* m, DisplayList& dlist) const
	{
		execute(m, dlist);
	}

	/// Set background color tag loader (SWF::SETBACKGROUNDCOLOR)
	static void loader(SWFStream& in, tag_type tag, movie_definition& m)
	{
		assert(tag == SWF::SETBACKGROUNDCOLOR); // 9

		// this one may throw, we'll let caller catch it
		SetBackgroundColorTag* t = new SetBackgroundColorTag(in);
		m.addControlTag(t); // takes ownership
	}
};




} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_SETBACKGROUNDCOLOR_TAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
