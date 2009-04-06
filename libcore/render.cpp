// render.cpp Rendering interface for Gnash
// 
//   Copyright (C) 2008, 2009 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Based on original by Willem Kokke <willem@mindparity.com> 2003

#include "render.h"
#include "GnashImage.h"

// Define this to have all renderer calls print a message (with -vv)
#undef DEBUG_RENDER_CALLS 

#ifdef DEBUG_RENDER_CALLS
	#include "log.h"
#endif

#include <cassert>
#include <memory>

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

	/// A fake BitmapInfo created when no renderer
	/// is registered.
	///
	/// Note that if you register a renderer *after* one of
	/// these bogus BitmapInfo has been created and attached
	/// as the cache of a movie element, things would likely
	/// screw up.
	///
	class bogus_bi : public BitmapInfo
	{
	public:
		bogus_bi() {}
	};

	BitmapInfo* createBitmapInfo(std::auto_ptr<GnashImage> im)
	{
	
        if (!s_render_handler)
        {
            return new bogus_bi;
        }

        switch (im->type())
        {
            default:
                log_error ("Attempt to create a bitmap_info "
                           "from unsupported image type");
                return NULL;

            case GNASH_IMAGE_RGB:
            case GNASH_IMAGE_RGBA:
            {
                return s_render_handler->createBitmapInfo(im);
            }
        }

	}

	// Draws the video frames
	void drawVideoFrame(GnashImage* frame, const SWFMatrix* mat,
            const rect* bounds, bool smooth)
    {
		if (s_render_handler) {
            return s_render_handler->drawVideoFrame(frame, mat, bounds, smooth);
        }
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


	void drawLine(const std::vector<point>& coords, const rgba& color, const SWFMatrix& mat)
	{
#ifdef DEBUG_RENDER_CALLS
		GNASH_REPORT_FUNCTION;
#endif
		if (s_render_handler) s_render_handler->drawLine(coords, color, mat);
}


void  draw_poly(const point* corners, int corner_count, const rgba& fill, 
  const rgba& outline, const SWFMatrix& mat, bool masked) 
	{
#ifdef DEBUG_RENDER_CALLS
		GNASH_REPORT_FUNCTION;
#endif
		if (s_render_handler) s_render_handler->draw_poly(corners, corner_count,
    fill, outline, mat, masked);
}

void
drawMorph(const morph_character_def& def, const MorphShape& inst) 
{
#ifdef DEBUG_RENDER_CALLS
		GNASH_REPORT_FUNCTION;
#endif
		if (s_render_handler) s_render_handler->drawMorph(def, inst);
}


void
drawShape(const shape_character_def& def, const DisplayObject& inst) 
{
#ifdef DEBUG_RENDER_CALLS
		GNASH_REPORT_FUNCTION;
#endif
		if (s_render_handler) s_render_handler->drawShape(def, inst);
}

void draw_glyph(shape_character_def *def,
  const SWFMatrix& mat,
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


} // namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
