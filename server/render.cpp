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

		class bogus_yuv : public YUV_video
		{
		public:
			bogus_yuv(): YUV_video(0, 0) { assert(0); }
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

		YUV_video*	create_YUV_video(int width, int height)
		{
			if (s_render_handler) return s_render_handler->create_YUV_video(width, height);
			else return new bogus_yuv;
		}

		void	delete_YUV_video(YUV_video* yuv)
		{
			if (s_render_handler) s_render_handler->delete_YUV_video(yuv);
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


		void	draw_line_strip(const int16_t coords[], int vertex_count, const rgba color)
		{
#ifdef DEBUG_RENDER_CALLS
			GNASH_REPORT_FUNCTION;
#endif
			if (s_render_handler) s_render_handler->draw_line_strip(coords, vertex_count, color);
    }


    void  draw_poly(const point* corners, int corner_count, const rgba fill, 
      const rgba outline) 
		{
#ifdef DEBUG_RENDER_CALLS
			GNASH_REPORT_FUNCTION;
#endif
			if (s_render_handler) s_render_handler->draw_poly(corners, corner_count,
        fill, outline);
    }
    
    
    void draw_shape_character(shape_character_def *def, 
      character *inst) 
    {
#ifdef DEBUG_RENDER_CALLS
			GNASH_REPORT_FUNCTION;
#endif
			if (s_render_handler) s_render_handler->draw_shape_character(def, inst);
    }
    
    void draw_glyph(shape_character_def *def,
      const matrix& mat,
      rgba color,
      float pixel_scale) 
    {
#ifdef DEBUG_RENDER_CALLS
			GNASH_REPORT_FUNCTION;
#endif
			if (s_render_handler) s_render_handler->draw_glyph(def, mat, color, pixel_scale);
    }
    
    
    bool allow_glyph_textures() {
      if (s_render_handler) 
        return s_render_handler->allow_glyph_textures();
      else
        return true;
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
