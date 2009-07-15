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
#include "Renderer.h"
#include "dsodefs.h"

// Forward declarations
namespace gnash {
    class rgba;
    class GnashImage;
    namespace SWF {
        class ShapeRecord;
    }
}

namespace gnash {

	/// Return currently registered render handler
	Renderer*	get_Renderer();

	/// Rendering operations 
	//
	/// Takes care of calling the currently registered
	/// gnash::Renderer or fallback on a default
	/// behaviour if no renderer is registered
	///
	/// NOTE: A cleaner implementation would be implementing
	///       the default behaviour in the Renderer class
	///	  itself, rather then making it an "abstract" class.
	///	  Anyway, having proxy calls turned out to be somewhat
	///	  useful while tracking rendering calls, to have a central
	///	  place to add traces. We might change this in the future
	///	  to improve performance (proxy calls are not inlined).
	///
	namespace render
	{
		/// See Renderer::create_bitmap_info_rgb (in backend/Renderer.h)
		BitmapInfo* createBitmapInfo(std::auto_ptr<GnashImage> im);

		/// See Renderer::drawVideoFrame (in backend/Renderer.h)
		void drawVideoFrame(GnashImage* frame, const SWFMatrix* mat,
                const rect* bounds, bool smooth);

		/// See Renderer::begin_display (in backend/Renderer.h)
		void	begin_display(
			const rgba& background_color,
			int viewport_x0, int viewport_y0,
			int viewport_width, int viewport_height,
			float x0, float x1, float y0, float y1);

		/// See Renderer::end_display (in backend/Renderer.h)
		void	end_display();

		/// See Renderer::draw_line_strip (in backend/Renderer.h)
		void drawLine(const std::vector<point>& coords, const rgba& color,
                const SWFMatrix& mat);

		/// See Renderer::draw_poly (in backend/Renderer.h)
		DSOEXPORT void  draw_poly(const point* corners, int corner_count,
				const rgba& fill, const rgba& outline, const SWFMatrix& mat,
				bool masked);
        
        void drawShape(const SWF::ShapeRecord& shape, const cxform& cx,
                const SWFMatrix& worldMat);
      
		/// See Renderer::draw_glyph (in backend/Renderer.h)
		void drawGlyph(const SWF::ShapeRecord& rec, const rgba& color,
                const SWFMatrix& mat);

		/// See Renderer::bounds_in_clipping_area (in backend/Renderer.h)
		bool bounds_in_clipping_area(const rect& bounds);
		bool bounds_in_clipping_area(const InvalidatedRanges& ranges);
		bool bounds_in_clipping_area(const geometry::Range2d<float>& bounds);
				
		/// See Renderer::begin_submit_mask (in backend/Renderer.h)
		void	begin_submit_mask();

		/// See Renderer::end_submit_mask (in backend/Renderer.h)
		void	end_submit_mask();

		/// See Renderer::disable_mask (in backend/Renderer.h)
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
