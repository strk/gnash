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
	static render_handler* s_render_handler = NULL;

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

		bitmap_info*	create_bitmap_info_rgb(image::ImageRGB* im)
		{
#ifdef DEBUG_RENDER_CALLS
			GNASH_REPORT_FUNCTION;
			//abort();
#endif
			if (s_render_handler) return s_render_handler->create_bitmap_info_rgb(im);
			else return new bogus_bi;
		}

		bitmap_info*	create_bitmap_info_rgba(image::ImageRGBA* im)
		{
#ifdef DEBUG_RENDER_CALLS
			GNASH_REPORT_FUNCTION;
			//abort();
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

		// Draws the video frames
		void drawVideoFrame(image::ImageBase* frame, const matrix* mat, const rect* bounds){
			if (s_render_handler) return s_render_handler->drawVideoFrame(frame, mat, bounds);
		}


		// Bracket the displaying of a frame from a movie.
		// Fill the background color, and set up default
		// transforms, etc.
		void	begin_display(
			const rgba& background_color,
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


		void	draw_line_strip(const boost::int16_t coords[], int vertex_count, const rgba& color, const matrix& mat)
		{
#ifdef DEBUG_RENDER_CALLS
			GNASH_REPORT_FUNCTION;
#endif
			if (s_render_handler) s_render_handler->draw_line_strip(coords, vertex_count, color, mat);
    }


    void  draw_poly(const point* corners, int corner_count, const rgba& fill, 
      const rgba& outline, const matrix& mat, bool masked) 
		{
#ifdef DEBUG_RENDER_CALLS
			GNASH_REPORT_FUNCTION;
#endif
			if (s_render_handler) s_render_handler->draw_poly(corners, corner_count,
        fill, outline, mat, masked);
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
      const rgba& color)
    {
#ifdef DEBUG_RENDER_CALLS
			GNASH_REPORT_FUNCTION;
#endif
			if (s_render_handler) s_render_handler->draw_glyph(def, mat, color);
    }
    
    bool bounds_in_clipping_area(const rect& bounds) {
    	return bounds_in_clipping_area(bounds.getRange());
      if (s_render_handler) 
        return s_render_handler->bounds_in_clipping_area(bounds);
      else
        return true;
    }
    
    bool bounds_in_clipping_area(const InvalidatedRanges& ranges) {
      if (s_render_handler) 
        return s_render_handler->bounds_in_clipping_area(ranges);
      else
        return true;
		}
    
    bool bounds_in_clipping_area(const geometry::Range2d<float>& bounds) {
      if (s_render_handler) 
        return s_render_handler->bounds_in_clipping_area(bounds);
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
	}
}


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
