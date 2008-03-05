// styles.h	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// line style types.


#ifndef GNASH_STYLES_H
#define GNASH_STYLES_H

#include "impl.h"
#include "types.h"
#include "bitmap_character_def.h"
#include "fill_style.h"

namespace gnash {

class stream;
class movie_definition;

class base_line_style
{
public:
	virtual ~base_line_style(){};
	
};

/// For the outside of outline shapes, or just bare lines.
class line_style : public base_line_style
{
public:
	line_style();

	/// Construct a line style with explicit values
	///
	/// @param width
	///	Thickness of line, in TWIPS. 
	///	Zero for hair line
	///
	/// @param color
	///	Line color
	///
	line_style(boost::uint16_t width, const rgba& color)
		:
		m_width(width),
		m_color(color)
	{
	}

	/// Read the line style from an SWF stream
	//
	/// Stream is assumed to be positioned at 
	/// the right place.
	///
	/// Throw a ParserException if there's no enough bytes in the
	/// currently opened tag for reading. See stream::ensureBytes()
	///
	void	read(stream* in, int tag_type, movie_definition *md);
	
	/// Read two lines styles from the SWF stream
	/// at the same time -- this is used in morphing.
	void read_morph(stream* in, int tag_type, movie_definition *md,
		line_style *pOther);

	/// Return thickness of the line, in TWIPS
	boost::uint16_t	get_width() const { return m_width; }

	/// Return line color and alpha
	const rgba&	get_color() const { return m_color; }
	
private:
	friend class morph2_character_def;
	friend class triangulating_render_handler;
	
	boost::uint16_t	m_width;	// in TWIPS
	rgba	m_color;
};

class morph_line_style : public base_line_style
{
public:
	morph_line_style();
	morph_line_style(stream* in);
	
	void read(stream* in);
	
private:
	boost::uint16_t m_width[2];
	rgba   m_color[2];
};

} // namespace gnash


#endif // GNASH_STYLES_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
