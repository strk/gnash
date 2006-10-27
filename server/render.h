// render.h	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Interface to renderer back-end.


#ifndef GNASH_RENDER_H
#define GNASH_RENDER_H


#include "types.h"
#include "gnash.h"
#include "image.h"
#include "render_handler.h"


namespace gnash {

	/// Return currently registered render handler
	render_handler*	get_render_handler();

	/// Rendering operations 
	//
	/// Takes care of calling the currently registered
	/// gnash::render_handler or fallback on a default
	/// behaviour if no renderer is registered
	///
	namespace render
	{

		/// \brief
		/// Create a bitmap_info so that it contains an alpha texture
		/// with the given data (1 byte per texel).
		bitmap_info*	create_bitmap_info_alpha(int w, int h,
					unsigned char* data);

		/// \brief
		/// Given an image, returns a pointer to a bitmap_info struct
		/// that can later be passed to fill_styleX_bitmap(), to set a
		/// bitmap fill style.
		///
		bitmap_info*	create_bitmap_info_rgb(image::rgb* im);

		/// \brief
		/// Given an image, returns a pointer to a bitmap_info struct
		/// that can later be passed to fill_style_bitmap(), to set a
		/// bitmap fill style.
		//
		/// This version takes an image with an alpha channel.
		///
		bitmap_info*	create_bitmap_info_rgba(image::rgba* im);

		/// Delete the given bitmap info struct.
		void	delete_bitmap_info(bitmap_info* bi);

		YUV_video*	create_YUV_video(int width, int height);
		void	delete_YUV_video(YUV_video* yuv);

		/// \brief
		/// Bracket the displaying of a frame from a movie.
		/// Fill the background color, and set up default
		/// transforms, etc.
		///
		void	begin_display(
			rgba background_color,
			int viewport_x0, int viewport_y0,
			int viewport_width, int viewport_height,
			float x0, float x1, float y0, float y1);
		void	end_display();

		// Geometric and color transforms for mesh
		// and line_strip rendering.
		void	set_matrix(const matrix& m);
		void	set_cxform(const cxform& cx);

		/// \brief
		/// Draw triangles using the current fill-style 0.
		/// Clears the style list after rendering.
		//
		/// coords is a list of (x,y) coordinate pairs, in
		/// triangle-strip order.  The type of the array should
		/// be float[vertex_count*2]
		void	draw_mesh_strip(const int16_t coords[],
				int vertex_count);

		void	draw_line_strip(const int16_t coords[],
				int vertex_count, const rgba color);

    void  draw_poly(const point* corners, int corner_count, const rgba fill, 
      const rgba outline);
      
    void draw_shape_character(shape_character_def *def, 
      character *inst);
      
    void draw_glyph(shape_character_def *def,
      const matrix& mat,
      rgba color,
      float pixel_scale);

    bool allow_glyph_textures();				
				
				

		void	begin_submit_mask();
		void	end_submit_mask();
		void	disable_mask();

		/// Special function to draw a rectangular bitmap;
		//
		/// intended for textured glyph rendering.  Ignores
		/// current transforms.
		void	draw_bitmap(const matrix& m, const bitmap_info* bi,
				const rect& coords,
				const rect& uv_coords, rgba color);

	}	// end namespace render

}	// end namespace gnash


#endif // GNASH_RENDER_H

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
