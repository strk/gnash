// render.h	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Interface to renderer back-end.


#ifndef GNASH_RENDER_H
#define GNASH_RENDER_H


#include "types.h"
#include "gnash.h"
#include "image.h"


namespace gnash {
	render_handler*	get_render_handler();

	namespace render
	{
		bitmap_info*	create_bitmap_info_empty();
		bitmap_info*	create_bitmap_info_alpha(int w, int h, unsigned char* data);
		bitmap_info*	create_bitmap_info_rgb(image::rgb* im);
		bitmap_info*	create_bitmap_info_rgba(image::rgba* im);
		void	delete_bitmap_info(bitmap_info* bi);

		// Bracket the displaying of a frame from a movie.
		// Fill the background color, and set up default
		// transforms, etc.
		void	begin_display(
			rgba background_color,
			int viewport_x0, int viewport_y0,
			int viewport_width, int viewport_height,
			float x0, float x1, float y0, float y1);
		void	end_display();

		// Geometric and color transforms for mesh and line_strip rendering.
		void	set_matrix(const matrix& m);
		void	set_cxform(const cxform& cx);

		// Draw triangles using the current fill-style 0.
		// Clears the style list after rendering.
		//
		// coords is a list of (x,y) coordinate pairs, in
		// triangle-strip order.  The type of the array should
		// be float[vertex_count*2]
		void	draw_mesh_strip(const Sint16 coords[], int vertex_count);

		// Draw a line-strip using the current line style.
		// Clear the style list after rendering.
		//
		// Coords is a list of (x,y) coordinate pairs, in
		// sequence.
		void	draw_line_strip(const Sint16 coords[], int vertex_count);

		void	fill_style_disable(int fill_side);
		void	fill_style_color(int fill_side, rgba color);
		void	fill_style_bitmap(int fill_side, const bitmap_info* bi, const matrix& m, render_handler::bitmap_wrap_mode wm);

		void	line_style_disable();
		void	line_style_color(rgba color);
		void	line_style_width(float width);

		void	begin_submit_mask();
		void	end_submit_mask();
		void	disable_mask();

		// Special function to draw a rectangular bitmap;
		// intended for textured glyph rendering.  Ignores
		// current transforms.
		void	draw_bitmap(const matrix& m, const bitmap_info* bi, const rect& coords, const rect& uv_coords, rgba color);

	};	// end namespace render
};	// end namespace gnash


#endif // GNASH_RENDER_H

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
