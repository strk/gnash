// types.h	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Some basic types.

#ifndef __TYPES_H__
#define __TYPES_H__


#include "gnash.h"
#include "tu_types.h"


namespace gnash {
	extern bool	s_verbose_action;
	extern bool	s_verbose_parse;
	extern bool	s_verbose_debug;
};

#define IF_VERBOSE_ACTION(exp) if (gnash::s_verbose_action) { exp; }
#define IF_VERBOSE_PARSE(exp) if (gnash::s_verbose_parse) { exp; }
#define IF_VERBOSE_DEBUG(exp) if (gnash::s_verbose_debug) { exp; }


#define TWIPS_TO_PIXELS(x)	((x) / 20.f)
#define PIXELS_TO_TWIPS(x)	((x) * 20.f)


namespace gnash {
	struct stream;	// forward declaration

	struct rgba
	{
		Uint8	m_r, m_g, m_b, m_a;

		rgba() : m_r(255), m_g(255), m_b(255), m_a(255) {}

		rgba(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
			:
			m_r(r), m_g(g), m_b(b), m_a(a)
		{
		}

		void	read(stream* in, int tag_type);
		void	read_rgba(stream* in);
		void	read_rgb(stream* in);

		void	set(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
		{
			m_r = r;
			m_g = g;
			m_b = b;
			m_a = a;
		}

		void	set_lerp(const rgba& a, const rgba& b, float f);

		void	print();
	};


};	// end namespace gnash


#endif // __TYPES_H__


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
