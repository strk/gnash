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
#include "dsodefs.h"


namespace gnash {

	/// Return currently registered render handler
	render_handler*	get_render_handler();

	/// Rendering operations 
	//
	/// Takes care of calling the currently registered
	/// gnash::render_handler or fallback on a default
	/// behaviour if no renderer is registered
	///
	/// NOTE: A cleaner implementation would be implementing
	///       the default behaviour in the render_handler class
	///	  itself, rather then making it an "abstract" class.
	///	  Anyway, having proxy calls turned out to be somewhat
	///	  useful while tracking rendering calls, to have a central
	///	  place to add traces. We might change this in the future
	///	  to improve performance (proxy calls are not inlined).
	///
	namespace render
	{
		/// See render_handler::create_bitmap_info_rgb (in backend/render_handler.h)
		bitmap_info*	create_bitmap_info_rgb(image::rgb* im);

		/// See render_handler::create_bitmap_info_rgba (in backend/render_handler.h)
		bitmap_info*	create_bitmap_info_rgba(image::rgba* im);

		/// See render_handler::delete_bitmap_info (in backend/render_handler.h)
		void	delete_bitmap_info(bitmap_info* bi);

		/// The different video frame formats
		//
		/// TODO: remove this duplication, original is in gnash::render_handler
		///
		enum video_frame_format
		{
			NONE,
			YUV,
			RGB
		};

		/// See render_handler::videoFrameFormat (in backend/render_handler.h)
		int videoFrameFormat();

		/// See render_handler::drawVideoFrame (in backend/render_handler.h)
		void drawVideoFrame(image::ImageBase* frame, const matrix* mat, const rect* bounds);

		/// See render_handler::begin_display (in backend/render_handler.h)
		void	begin_display(
			const rgba& background_color,
			int viewport_x0, int viewport_y0,
			int viewport_width, int viewport_height,
			float x0, float x1, float y0, float y1);

		/// See render_handler::end_display (in backend/render_handler.h)
		void	end_display();

		/// \brief
		/// Draw triangles using the current fill-style 0.
		/// Clears the style list after rendering.
		//
		/// coords is a list of (x,y) coordinate pairs, in
		/// triangle-strip order.  The type of the array should
		/// be float[vertex_count*2]
		///
		void	draw_mesh_strip(const boost::int16_t coords[],
				int vertex_count);

		/// See render_handler::draw_line_strip (in backend/render_handler.h)
		void	draw_line_strip(const boost::int16_t coords[],
				int vertex_count, const rgba& color, const matrix& mat);

		/// See render_handler::draw_poly (in backend/render_handler.h)
		DSOEXPORT void  draw_poly(const point* corners, int corner_count,
				const rgba& fill, const rgba& outline, const matrix& mat,
				bool masked);
      
		/// See render_handler::draw_shape_character (in backend/render_handler.h)
		void draw_shape_character(shape_character_def *def,
				character *inst);
      
		/// See render_handler::draw_glyph (in backend/render_handler.h)
		void draw_glyph(shape_character_def *def, const matrix& mat,
				const rgba& color);

		/// See render_handler::bounds_in_clipping_area (in backend/render_handler.h)
		bool bounds_in_clipping_area(const rect& bounds);
		bool bounds_in_clipping_area(const InvalidatedRanges& ranges);
		bool bounds_in_clipping_area(const geometry::Range2d<float>& bounds);
				
		/// See render_handler::begin_submit_mask (in backend/render_handler.h)
		void	begin_submit_mask();

		/// See render_handler::end_submit_mask (in backend/render_handler.h)
		void	end_submit_mask();

		/// See render_handler::disable_mask (in backend/render_handler.h)
		void	disable_mask();

	}	// end namespace render

}	// end namespace gnash


#endif // GNASH_RENDER_H

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
