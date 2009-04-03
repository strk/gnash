// render.h Rendering interface for Gnash
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


#ifndef GNASH_RENDER_H
#define GNASH_RENDER_H

#include "gnash.h"
#include "render_handler.h"
#include "dsodefs.h"

// Forward declarations
namespace gnash {
    class rgba;
    class GnashImage;
}

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
		BitmapInfo* createBitmapInfo(std::auto_ptr<GnashImage> im);

		/// See render_handler::drawVideoFrame (in backend/render_handler.h)
		void drawVideoFrame(GnashImage* frame, const SWFMatrix* mat,
                const rect* bounds, bool smooth);

		/// See render_handler::begin_display (in backend/render_handler.h)
		void	begin_display(
			const rgba& background_color,
			int viewport_x0, int viewport_y0,
			int viewport_width, int viewport_height,
			float x0, float x1, float y0, float y1);

		/// See render_handler::end_display (in backend/render_handler.h)
		void	end_display();

		/// See render_handler::draw_line_strip (in backend/render_handler.h)
		void drawLine(const std::vector<point>& coords, const rgba& color,
                const SWFMatrix& mat);

		/// See render_handler::draw_poly (in backend/render_handler.h)
		DSOEXPORT void  draw_poly(const point* corners, int corner_count,
				const rgba& fill, const rgba& outline, const SWFMatrix& mat,
				bool masked);
      
		/// See render_handler::draw_shape_DisplayObject (in backend/render_handler.h)
		void drawShape(shape_character_def *def, DisplayObject *inst);
      
		/// See render_handler::draw_glyph (in backend/render_handler.h)
		void draw_glyph(shape_character_def *def, const SWFMatrix& mat,
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
