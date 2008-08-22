

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A module to take care of all of gnash's loaded fonts.

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h" // HAVE_ZLIB_H, USE_SWFTREE
#endif

#ifdef HAVE_PTHREADS
#include <pthread.h>
#endif

#include "font.h"
#include "impl.h"
#include "log.h"
#include "render.h"
#include "shape_character_def.h"
#include "styles.h"
#include "movie_definition.h"

// Define to the name of a default font.
#define DEFAULT_FONT_NAME "_sans"

namespace gnash {
namespace fontlib {

namespace {
	std::vector< boost::intrusive_ptr<font> >	s_fonts;
	boost::intrusive_ptr<font> _defaultFont;
}


	//
	// Public interface
	//


	void	clear()
	// Release all the fonts we know about.
	{
		s_fonts.clear();
	}

boost::intrusive_ptr<font>
get_default_font()
{
	if ( _defaultFont ) return _defaultFont;
	_defaultFont = new font(DEFAULT_FONT_NAME);
	return _defaultFont;
}

	int	get_font_count()
	// Return the number of fonts in our library.
	{
		return s_fonts.size();
	}


	font*	get_font(int index)
	// Retrieve one of our fonts, by index.
	{
		if (index < 0 || index >= (int) s_fonts.size())
		{
			return NULL;
		}

		return s_fonts[index].get();
	}


	font*	get_font(const std::string& name, bool bold, bool italic)
	{
		// Dumb linear search.
		for (unsigned int i = 0; i < s_fonts.size(); i++)
		{
			font*	f = s_fonts[i].get();
			assert(f);
			if ( f->matches(name, bold, italic) )
			{
				return f;
			}
		}
		font* f = new font(name, bold, italic);
		s_fonts.push_back(f);
		return f;
	}

	void	add_font(font* f)
	// Add the given font to our library.
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
