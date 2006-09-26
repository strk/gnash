// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//

#ifndef RENDER_HANDLER_H
#define RENDER_HANDLER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h" // for DSOEXPORT

// Forward declarations.
namespace gnash {
	class bitmap_info;
	class rect;
	class rgba;
	class matrix;
	class cxform;
}
// @@ forward decl to avoid including base/image.h; TODO change the
// render_handler interface to not depend on these classes at all.
namespace image { class image_base; class rgb; class rgba; }

namespace gnash {

/// You must define a subclass of render_handler, and pass an
/// instance to set_render_handler().
class DSOEXPORT render_handler
{
public:
	virtual ~render_handler() {}

	// Your handler should return these with a ref-count of 0.  (@@ is that the right policy?)

	/// \brief
	///  Create a bitmap_info so that it contains an alpha texture
	/// with the given data (1 byte per texel).
	//
	/// Munges *data (in order to make mipmaps)!!
	///
	virtual bitmap_info*	create_bitmap_info_alpha(int w, int h, unsigned char* data) = 0;

	/// \brief
	/// Given an image, returns a pointer to a bitmap_info class
	/// that can later be passed to fill_styleX_bitmap(), to set a
	/// bitmap fill style.
	virtual bitmap_info*	create_bitmap_info_rgb(image::rgb* im) = 0;

	/// \brief
	/// Given an image, returns a pointer to a bitmap_info class
	/// that can later be passed to fill_style_bitmap(), to set a
	/// bitmap fill style.
	//
	/// This version takes an image with an alpha channel.
	///
	virtual bitmap_info*	create_bitmap_info_rgba(image::rgba* im) = 0;

	/// Delete the given bitmap info class.
	virtual void	delete_bitmap_info(bitmap_info* bi) = 0;
		
	/// Bracket the displaying of a frame from a movie.
	//
	/// Fill the background color, and set up default
	/// transforms, etc.
	///
	virtual void	begin_display(
		rgba background_color,
		int viewport_x0, int viewport_y0,
		int viewport_width, int viewport_height,
		float x0, float x1, float y0, float y1) = 0;

	virtual void	end_display() = 0;
		
	/// Geometric transforms for mesh and line_strip rendering.
	virtual void	set_matrix(const matrix& m) = 0;

	/// Color transforms for mesh and line_strip rendering.
	virtual void	set_cxform(const cxform& cx) = 0;
		
	/// Draw triangles using the current fill-style 0.
	//
	/// Clears the style list after rendering.
	///
	/// coords is a list of (x,y) coordinate pairs, in
	/// triangle-strip order.  The type of the array should
	/// be int16_t[vertex_count*2]
	///
	virtual void	draw_mesh_strip(const void* coords, int vertex_count) = 0;
		
	/// Draw a line-strip using the current line style.
	//
	/// Clear the style list after rendering.
	///
	/// Coords is a list of (x,y) coordinate pairs, in
	/// sequence.  Each coord is a 16-bit signed integer.
	///
	virtual void	draw_line_strip(const void* coords, int vertex_count) = 0;
		
	/// Set line and fill styles for mesh & line_strip rendering.
	enum bitmap_wrap_mode
	{
		WRAP_REPEAT,
		WRAP_CLAMP
	};
	virtual void	fill_style_disable(int fill_side) = 0;
	virtual void	fill_style_color(int fill_side, rgba color) = 0;
	virtual void	fill_style_bitmap(int fill_side, const bitmap_info* bi, const matrix& m, bitmap_wrap_mode wm) = 0;
		
	virtual void	line_style_disable() = 0;
	virtual void	line_style_color(rgba color) = 0;
	virtual void	line_style_width(float width) = 0;
		
	/// Special function to draw a rectangular bitmap.
	//
	/// Intended for textured glyph rendering.  Ignores
	/// current transforms.
	///
	virtual void	draw_bitmap(
		const matrix&		m,
		const bitmap_info*	bi,
		const rect&		coords,
		const rect&		uv_coords,
		rgba			color) = 0;
		
	virtual void	set_antialiased(bool enable) = 0;
		
	virtual void begin_submit_mask() = 0;
	virtual void end_submit_mask() = 0;
	virtual void disable_mask() = 0;
};



}	// namespace gnash

#endif // RENDER_HANDLER_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
