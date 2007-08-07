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
line_style::read(stream* in, int tag_type)
{
    in->ensureBytes(2);
    m_width = in->read_u16();
    m_color.read(in, tag_type);
}


// end of namespace
}


// Local Variables:
// mode: C++
// End:
