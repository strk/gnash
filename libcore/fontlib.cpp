

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A module to take care of all of gnash's loaded fonts.

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h" // HAVE_ZLIB_H, USE_SWFTREE
#endif

#include "Font.h"
#include "log.h"
#include "DefineShapeTag.h"
#include "LineStyle.h"
#include "movie_definition.h"

// Define to the name of a default font.
#define DEFAULT_FONT_NAME "_sans"

namespace gnash {
namespace fontlib {

namespace {
	std::vector< boost::intrusive_ptr<Font> >	s_fonts;
	boost::intrusive_ptr<Font> _defaultFont;
}


	//
	// Public interface
	//


void
clear()
{
    s_fonts.clear();
}

boost::intrusive_ptr<Font>
get_default_font()
{
	if ( _defaultFont ) return _defaultFont;
	_defaultFont = new Font(DEFAULT_FONT_NAME);
	return _defaultFont;
}

Font*
get_font(const std::string& name, bool bold, bool italic)
{
    // Dumb linear search.
    for (unsigned int i = 0; i < s_fonts.size(); i++)
    {
        Font*	f = s_fonts[i].get();
        assert(f);
        if ( f->matches(name, bold, italic) )
        {
            return f;
        }
    }
    Font* f = new Font(name, bold, italic);
    s_fonts.push_back(f);
    return f;
}

void
add_font(Font* f)
{
    assert(f);
#ifndef NDEBUG
    // Make sure font isn't already in the list.
    for (unsigned int i = 0; i < s_fonts.size(); i++)
    {
        assert(s_fonts[i] != f);
    }
#endif // not NDEBUG

    s_fonts.push_back(f);
}



}	// end namespace fontlib
}	// end namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
