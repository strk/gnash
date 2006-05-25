// styles.h	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Fill and line style types.


#ifndef GNASH_STYLES_H
#define GNASH_STYLES_H

#include "impl.h"
#include "types.h"
#include "bitmap_character_def.h"

namespace gnash {
struct stream;


struct gradient_record
{
	gradient_record();
	void	read(stream* in, int tag_type);
	
	//data:
	uint8_t	m_ratio;
	rgba	m_color;
};

struct base_fill_style
{
	virtual ~base_fill_style() {};
	virtual void apply(int fill_side, float ratio) const = 0;
};

class fill_style : public base_fill_style
// For the interior of outline shapes.
{
public:
	fill_style();
	virtual ~fill_style();
	
	void	read(stream* in, int tag_type, movie_definition* m);
	rgba	sample_gradient(int ratio) const;
	gnash::bitmap_info*	create_gradient_bitmap() const;
	virtual void	apply(int fill_side, float ratio) const;
	
	rgba	get_color() const { return m_color; }
	void	set_color(rgba new_color) { m_color = new_color; }
	int	get_type() const { return m_type; }
	
	// For shape morphing
	void	set_lerp(const fill_style& a, const fill_style& b, float t);
	
private:
	friend class morph2_character_def;
	
	int	m_type;
	rgba	m_color;
	matrix	m_gradient_matrix;
	std::vector<gradient_record>	m_gradients;
	smart_ptr<gnash::bitmap_info>	m_gradient_bitmap_info;
	smart_ptr<bitmap_character_def>	m_bitmap_character;
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
	virtual void apply(int fill_side, float morph) const;
	rgba get_color(float morph) const;
	void set_colors(rgba new_color_orig, rgba new_color_target);
private:
	int m_type;
	rgba m_color[2];
	matrix m_gradient_matrix[2];
	std::vector<gradient_record> m_gradients[2];
	smart_ptr<bitmap_info> m_gradient_bitmap_info[2];
	smart_ptr<bitmap_character_def> m_bitmap_character;
	matrix m_bitmap_matrix[2];
};

struct base_line_style
{
	virtual ~base_line_style(){};
	virtual void apply(float ratio) const = 0;
};

class line_style : public base_line_style
// For the outside of outline shapes, or just bare lines.
{
public:
	line_style();
	void	read(stream* in, int tag_type);
	virtual void	apply(float ratio) const;
	
	uint16_t	get_width() const { return m_width; }
	const rgba&	get_color() const { return m_color; }
	
private:
	friend class morph2_character_def;
	
	uint16_t	m_width;	// in TWIPS
	rgba	m_color;
};

class morph_line_style : public base_line_style
{
public:
	morph_line_style();
	morph_line_style(stream* in);
	
	void read(stream* in);
	virtual void apply(float morph) const;
	
private:
	uint16_t m_width[2];
	rgba   m_color[2];
};
}


#endif // GNASH_STYLES_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
