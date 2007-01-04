// styles.h	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Fill and line style types.

/* $Id: styles.h,v 1.17 2007/01/04 04:03:33 strk Exp $ */

#ifndef GNASH_STYLES_H
#define GNASH_STYLES_H

#include "impl.h"
#include "types.h"
#include "bitmap_character_def.h"

namespace gnash {

class stream;

class gradient_record
{
public:
	gradient_record();
	void	read(stream* in, int tag_type);
	
	//data:
	uint8_t	m_ratio;
	rgba	m_color;
};

class DSOLOCAL base_fill_style
{
public:
	virtual ~base_fill_style() {};
};

/// For the interior of outline shapes.
class DSOEXPORT fill_style : public base_fill_style
{
public:

	fill_style();

	virtual ~fill_style();
	
	void	read(stream* in, int tag_type, movie_definition* m);


    /// \brief
    /// Make a bitmap_info* corresponding to our gradient.
    /// We can use this to set the gradient fill style.
	gnash::bitmap_info*	create_gradient_bitmap() const;
	
	/// \brief
	/// Makes sure that m_gradient_bitmap_info is not NULL. Calls 
  /// create_gradient_bitmap() if necessary and returns m_gradient_bitmap_info.
	gnash::bitmap_info* need_gradient_bitmap() const; 
	
	rgba	get_color() const { return m_color; }

	void	set_color(rgba new_color) { m_color = new_color; }

	int	get_type() const { return m_type; }
	
    /// Sets this style to a blend of a and b.  t = [0,1] (for shape morphing)
	void	set_lerp(const fill_style& a, const fill_style& b, float t);
	
	/// Returns the bitmap info for all styles except solid fills
	//
	/// NOTE: calling this method against a solid fill style will
	///       result in a failed assertion.
	/// 
	/// NOTE2: this function can return NULL if the character_id
	///        specified for the style in the SWF does not resolve
	///        to a character defined in the characters dictionary.
	///        (it happens..)
	///
	bitmap_info* get_bitmap_info() const;
	
	/// Returns the bitmap transformation matrix
	matrix get_bitmap_matrix() const; 
	
	/// Returns the gradient transformation matrix
	matrix get_gradient_matrix() const; 
	
	/// Returns the number of color stops in the gradient
	int get_color_stop_count() const;
	
	/// Returns the color stop value at a specified index
	const gradient_record& get_color_stop(int index) const;
	
private:

	/// Return the color at the specified ratio into our gradient.
	//
	/// @param ratio
	///	Ratio is in the range [0, 255].
	///
	rgba sample_gradient(uint8_t ratio) const;

	friend class morph2_character_def;
	friend class triangulating_render_handler;
	
	int	m_type;
	rgba	m_color;
	matrix	m_gradient_matrix;
	std::vector<gradient_record>	m_gradients;
	boost::intrusive_ptr<gnash::bitmap_info>	m_gradient_bitmap_info;
	boost::intrusive_ptr<bitmap_character_def>	m_bitmap_character;
	matrix	m_bitmap_matrix;
};


class morph_fill_style : public base_fill_style
{

public:

	morph_fill_style();

	morph_fill_style(stream* in, movie_definition* m);

	virtual ~morph_fill_style();
	
	void read(stream* in, movie_definition* m);

	rgba sample_gradient(int ratio, float morph);

	bitmap_info* create_gradient_bitmap(float morph) const;

	//virtual void apply(int fill_side, float morph) const;
	rgba get_color(float morph) const;
	void set_colors(rgba new_color_orig, rgba new_color_target);
private:
	int m_type;
	rgba m_color[2];
	matrix m_gradient_matrix[2];
	std::vector<gradient_record> m_gradients[2];
	boost::intrusive_ptr<bitmap_info> m_gradient_bitmap_info[2];
	boost::intrusive_ptr<bitmap_character_def> m_bitmap_character;
	matrix m_bitmap_matrix[2];
};

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
	void	read(stream* in, int tag_type);
	
	uint16_t	get_width() const { return m_width; }
	const rgba&	get_color() const { return m_color; }
	
private:
	friend class morph2_character_def;
	friend class triangulating_render_handler;
	
	uint16_t	m_width;	// in TWIPS
	rgba	m_color;
};

class morph_line_style : public base_line_style
{
public:
	morph_line_style();
	morph_line_style(stream* in);
	
	void read(stream* in);
	
private:
	uint16_t m_width[2];
	rgba   m_color[2];
};

} // namespace gnash


#endif // GNASH_STYLES_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
