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

/// For the outside of outline shapes, or just bare lines.
class line_style 
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
	line_style(boost::uint16_t width, const rgba& color, bool scaleThicknessVertically, bool scaleThicknessHorizontally)
		:
		m_width(width),
		m_color(color),
		_scaleVertically(scaleThicknessVertically),
		_scaleHorizontally(scaleThicknessHorizontally)
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
	boost::uint16_t	getThickness() const
	{
		return m_width;
	}

	/// Return true if line thickness should be scaled vertically
	bool scaleThicknessVertically() const
	{
		return _scaleVertically;
	}

	/// Return true if line thickness should be scaled horizontally
	bool scaleThicknessHorizontally() const
	{
		return _scaleHorizontally;
	}

	/// Return line color and alpha
	const rgba&	get_color() const { return m_color; }

	/// Set this style to the interpolation of the given one
	//
	/// @param ls1
	///	First line_style to interpolate.
	///
	/// @param ls2
	///	Second line_style to interpolate.
	///
	/// @ratio
	///	The interpolation factor (0..1).
	///	When 0, this will be equal to ls1, when 1
	///	this will be equal to ls2.
	///
        void set_lerp(const line_style& ls1, const line_style& ls2, float ratio);
	
private:
	
	boost::uint16_t	m_width;	// in TWIPS
	rgba	m_color;
	bool _scaleVertically;
	bool _scaleHorizontally;
};

} // namespace gnash


#endif // GNASH_STYLES_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
