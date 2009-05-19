//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include "aos4_glue.h"
#include "gnash.h"
#undef ACTION_END
#include "render_handler.h"
#include "render_handler_agg.h"

#include <proto/intuition.h>
#include <proto/graphics.h>
#include <graphics/blitattr.h>
#include <proto/Picasso96API.h>
#include <proto/layers.h>
#include <proto/dos.h>

#include <boost/cstdint.hpp> // for boost::?int??_t 
#include <vector>

namespace gnash
{
	class AOS4AggGlue : public AOS4Glue
	{
	  public:
    	AOS4AggGlue();
	    virtual ~AOS4AggGlue();

    	bool 			 init(int argc, char **argv[]);
	    render_handler	*createRenderHandler(int depth);
    	render_handler	*createRenderHandler();
	    bool 			 prepDrawingArea(int width, int height);
	    void 			 render();
    	void 			 render(int minx, int miny, int maxx, int maxy);
	    void 			 setInvalidatedRegions(const InvalidatedRanges& ranges);
		struct Window 	*getWindow(void);
		void 			 resize(int width, int height);
	 private:
    	geometry::Range2d<int> _validbounds;
	    std::vector< geometry::Range2d<int> > _drawbounds;
    	unsigned char   	*_offscreenbuf;
	    render_handler  	*_agg_renderer;
    	struct Window   	*_window;
    	int 			 	 _width;
	    int				 	 _height;
    	int				 	 _stride;
		int				 	 _btype;
		RGBFTYPE		 	 _ftype;
	};
}
