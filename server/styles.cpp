// styles.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Fill and line style types.


#include "styles.h"
#include "impl.h"
#include "log.h"
#include "render.h"
#include "stream.h"
#include "movie_definition.h"
#include "swf.h"
#include "GnashException.h"

namespace gnash {


//
// line_style
//

	
line_style::line_style()
    :
    m_width(0)
{
}

void
line_style::read_morph(stream* in, int tag_type, movie_definition *md,
	line_style *pOther)
{
	if (tag_type == SWF::DEFINEMORPHSHAPE)
	{
		m_width = in->read_u16();
		pOther->m_width = in->read_u16();
		m_color.read(in, tag_type);
		pOther->m_color.read(in, tag_type);
		return;
	}

	// MorphShape 2 from here down.
	in->ensureBytes(4);
	m_width = in->read_u16();
	pOther->m_width = in->read_u16();
	// TODO: Same as in read(...), use these.
	// 0 -- Round caps, 1 -- No caps, 2 -- square caps
	boost::uint8_t caps = in->read_uint(2);
	// 0 -- Round join, 1 -- Bevel join, 2 -- Miter join
	boost::uint8_t joins = in->read_uint(2);
	bool has_fill = in->read_uint(1);
	bool no_hscale = in->read_uint(1);
	bool no_vscale = in->read_uint(1);
	bool pixel_hinting = in->read_uint(1);
	static_cast<void> (in->read_uint(5));
	bool no_close = in->read_uint(1);
	bool end_cap_style = in->read_uint(2); // As caps above.
	if (joins == 2)
	{
		float f_miter = in->read_short_ufixed();
	}
	if (has_fill)
	{
		// TODO: Throwing this away is not the right thing.
		// What is?
		// A fill style is here.
		fill_style f, g;
		f.read(in, tag_type, md, &g);
		m_color = f.get_color();
		pOther->m_color = g.get_color();
	}
	else
	{
		m_color.read(in, tag_type);
		pOther->m_color.read(in, tag_type);
	}
}

void
line_style::read(stream* in, int tag_type, movie_definition *md)
{
    if (!(tag_type == SWF::DEFINESHAPE4 || tag_type == SWF::DEFINESHAPE4_))
	{
	    in->ensureBytes(2);
		m_width = in->read_u16();
		m_color.read(in, tag_type);
		return;
	}

	// TODO: Unfinished. Temporary to allow define shape 4 to work in many
	// cases, but does not work correctly in all cases.
	in->ensureBytes(2);
	m_width = in->read_u16();
	// 0 -- Round caps, 1 -- No caps, 2 -- square caps
	boost::uint8_t caps = in->read_uint(2);
	// 0 -- Round join, 1 -- Bevel join, 2 -- Miter join
	boost::uint8_t joins = in->read_uint(2);
	bool has_fill = in->read_uint(1);
	bool no_hscale = in->read_uint(1);
	bool no_vscale = in->read_uint(1);
	bool pixel_hinting = in->read_uint(1);
	static_cast<void> (in->read_uint(5));
	bool no_close = in->read_uint(1);
	bool end_cap_style = in->read_uint(2); // As caps above.
	if (joins == 2)
	{
		/*float f_miter =*/static_cast<void>(in->read_short_ufixed());
	}
	if (has_fill)
	{
		// TODO: Throwing this away is not the right thing.
		// What is?
		// A fill style is here.
		fill_style f;
		f.read(in, tag_type, md);
		m_color = f.get_color();
	}
	else
	{
		m_color.read(in, tag_type);
	}
}


// end of namespace
}


// Local Variables:
// mode: C++
// End:
