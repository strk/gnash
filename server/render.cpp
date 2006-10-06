// render.cpp	-- Willem Kokke <willem@mindparity.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Interface to back-end renderer.


#include "render.h"

// Define this to have all renderer calls print a message (with -vv)
#undef DEBUG_RENDER_CALLS 

#ifdef DEBUG_RENDER_CALLS
	#include "log.h"
#endif

#include <cassert>

namespace gnash {
	static render_handler* s_render_handler;

	void set_render_handler(render_handler* r)
	{
		s_render_handler = r;
	}

	render_handler* get_render_handler()
	{
		return s_render_handler;
	}


	namespace render
	{

		/// A fake bitmap_info created when no renderer
		/// is registered.
		///
		/// Note that if you register a renderer *after* one of
		/// these bogus bitmap_info has been created and attached
		/// as the cache of a movie element, things would likely
		/// screw up.
		///
		class bogus_bi : public bitmap_info
		{
		public:
			bogus_bi() {}
		};


		bitmap_info*	create_bitmap_info_alpha(int w, int h, unsigned char* data)
		{
#ifdef DEBUG_RENDER_CALLS
			GNASH_REPORT_FUNCTION;
#endif
			if (s_render_handler) return s_render_handler->create_bitmap_info_alpha(w, h, data);
			else return new bogus_bi;
		}

		bitmap_info*	create_bitmap_info_rgb(image::rgb* im)
		{
#ifdef DEBUG_RENDER_CALLS
			GNASH_REPORT_FUNCTION;
			//assert(0);
#endif
			if (s_render_handler) return s_render_handler->create_bitmap_info_rgb(im);
			else return new bogus_bi;
		}

		bitmap_info*	create_bitmap_info_rgba(image::rgba* im)
		{
#ifdef DEBUG_RENDER_CALLS
			GNASH_REPORT_FUNCTION;
			//assert(0);
#endif
			if (s_render_handler) return s_render_handler->create_bitmap_info_rgba(im);
			else return new bogus_bi;
		}

		void	delete_bitmap_info(bitmap_info* bi)
		{
#ifdef DEBUG_RENDER_CALLS
			GNASH_REPORT_FUNCTION;
#endif
			if (s_render_handler) s_render_handler->delete_bitmap_info(bi);
		}

		// Bracket the displaying of a frame from a movie.
		// Fill the background color, and set up default
		// transforms, etc.
		void	begin_display(
			rgba background_color,
			int viewport_x0, int viewport_y0,
			int viewport_width, int viewport_height,
			float x0, float x1, float y0, float y1)
		{
#ifdef DEBUG_RENDER_CALLS
			GNASH_REPORT_FUNCTION;
#endif
			if (s_render_handler)
			{
				s_render_handler->begin_display(
					background_color, viewport_x0, viewport_y0,
					viewport_width, viewport_height,
					x0, x1, y0, y1);
			}
// 			else
// 			{
// 				log_error("begin_display called, but no render_handler was registered by the app!\n");
// 			}
		}


		void	end_display()
		{
#ifdef DEBUG_RENDER_CALLS
			GNASH_REPORT_FUNCTION;
#endif
			if (s_render_handler) s_render_handler->end_display();
		}


		// Geometric and color transforms for mesh and line_strip rendering.
		void	set_matrix(const matrix& m)
		{
#ifdef DEBUG_RENDER_CALLS
			GNASH_REPORT_FUNCTION;
#endif
			if (s_render_handler) s_render_handler->set_matrix(m);
		}
		void	set_cxform(const cxform& cx)
		{
#ifdef DEBUG_RENDER_CALLS
			GNASH_REPORT_FUNCTION;
#endif
			if (s_render_handler) s_render_handler->set_cxform(cx);
		}

		// Draw triangles using the current fill-style 0.
		// Clears the style list after rendering.
		//
		// coords is a list of (x,y) coordinate pairs, in
		// triangle-strip order.  The type of the array should
		// be float[vertex_count*2]
		void	draw_mesh_strip(const int16_t coords[], int vertex_count)
		{
#ifdef DEBUG_RENDER_CALLS
			GNASH_REPORT_FUNCTION;
#endif
			if (s_render_handler) s_render_handler->draw_mesh_strip(coords, vertex_count);
		}

		// Draw a line-strip using the current line style.
		// Clear the style list after rendering.
		//
		// Coords is a list of (x,y) coordinate pairs, in
		// sequence.
		void	draw_line_strip(const int16_t coords[], int vertex_count)
		{
#ifdef DEBUG_RENDER_CALLS
			GNASH_REPORT_FUNCTION;
#endif
			if (s_render_handler) s_render_handler->draw_line_strip(coords, vertex_count);
		}

// 		// Set line and fill styles for mesh & line_strip
// 		// rendering.
// 		enum bitmap_wrap_mode
// 		{
// 			WRAP_REPEAT,
// 			WRAP_CLAMP
// 		};

		void	fill_style_disable(int fill_side)
		{
#ifdef DEBUG_RENDER_CALLS
			GNASH_REPORT_FUNCTION;
#endif
			if (s_render_handler) s_render_handler->fill_style_disable(fill_side);
		}

		void	fill_style_color(int fill_side, rgba color)
		{
#ifdef DEBUG_RENDER_CALLS
			GNASH_REPORT_FUNCTION;
#endif
			if (s_render_handler) s_render_handler->fill_style_color(fill_side, color);
		}

		void	fill_style_bitmap(int fill_side, const bitmap_info* bi, const matrix& m, render_handler::bitmap_wrap_mode wm)
		{
#ifdef DEBUG_RENDER_CALLS
			GNASH_REPORT_FUNCTION;
#endif
			if (s_render_handler) s_render_handler->fill_style_bitmap(fill_side, bi, m, wm);
		}

		void	line_style_disable()
		{
#ifdef DEBUG_RENDER_CALLS
			GNASH_REPORT_FUNCTION;
#endif
			if (s_render_handler) s_render_handler->line_style_disable();
		}

		void	line_style_color(rgba color)
		{
#ifdef DEBUG_RENDER_CALLS
			GNASH_REPORT_FUNCTION;
#endif
			if (s_render_handler) s_render_handler->line_style_color(color);
		}

		void	line_style_width(float width)
		{
#ifdef DEBUG_RENDER_CALLS
			GNASH_REPORT_FUNCTION;
#endif
			if (s_render_handler) s_render_handler->line_style_width(width);
		}

		void	begin_submit_mask()
		{
#ifdef DEBUG_RENDER_CALLS
			GNASH_REPORT_FUNCTION;
#endif
			if (s_render_handler) s_render_handler->begin_submit_mask();
		}

		void	end_submit_mask()
		{
#ifdef DEBUG_RENDER_CALLS
			GNASH_REPORT_FUNCTION;
#endif
			if (s_render_handler) s_render_handler->end_submit_mask();
		}

		void	disable_mask()
		{
#ifdef DEBUG_RENDER_CALLS
			GNASH_REPORT_FUNCTION;
#endif
			if (s_render_handler) s_render_handler->disable_mask();
		}

		// Special function to draw a rectangular bitmap;
		// intended for textured glyph rendering.  Ignores
		// current transforms.
		void	draw_bitmap(const matrix& m, const bitmap_info* bi, const rect& coords, const rect& uv_coords, rgba color)
		{
#ifdef DEBUG_RENDER_CALLS
			GNASH_REPORT_FUNCTION;
#endif
			if (s_render_handler)
			{
				s_render_handler->draw_bitmap(
					m, bi, coords, uv_coords, color);
			}
		}
	}
}


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
