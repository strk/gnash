// aos4_agg_glue.cpp:  Glue between AmigaOS4 and Anti-Grain Geometry, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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
//

#include "aos4_agg_glue.h"
#include "log.h"
#undef ACTION_END
#include "Renderer.h"
#include "Renderer_agg.h"
#include <cerrno>
#include <ostream>

/* START OF MENU DEFINITION */
struct NewMenu nm[] =
{
	/* Type, Label, CommKey, Flags, MutualExclude, UserData */

	{ NM_TITLE, (CONST_STRPTR)"File",               	NULL,   0, 0L, NULL },          /* Menu 0 */
		{ NM_ITEM,  (CONST_STRPTR)"Load File..",          (CONST_STRPTR)"L",	0, 0L, NULL },          /* Item 0 */
		{ NM_ITEM,  (CONST_STRPTR)"Save",                 (CONST_STRPTR)"S",	NM_ITEMDISABLED, 0L, NULL },          /* Item 1 */
		{ NM_ITEM,  (CONST_STRPTR)"Save As..",            (CONST_STRPTR)"A",	NM_ITEMDISABLED, 0L, NULL },          /* Item 2 */
		{ NM_ITEM,  NM_BARLABEL,              NULL,	0, 0L, NULL },			/* Item 3 */
		{ NM_ITEM,  (CONST_STRPTR)"Properties",           (CONST_STRPTR)"E",	0, 0L, NULL },			/* Item 4 */
		{ NM_ITEM,  NM_BARLABEL,              NULL,	0, 0L, NULL },			/* Item 5 */
		{ NM_ITEM,  (CONST_STRPTR)"Exit",                 (CONST_STRPTR)"Q",	0, 0L, NULL },          /* Item 6 */
	{ NM_TITLE, (CONST_STRPTR)"Edit",               	NULL,   0, 0L, NULL },          /* Menu 1 */
		{ NM_ITEM,  (CONST_STRPTR)"Preferences",			(CONST_STRPTR)"O",	0, 0L, NULL },          /* Item 0 */
	{ NM_TITLE, (CONST_STRPTR)"View", 		            NULL,	0, 0L, NULL },		/* Menu 2 */
		{ NM_ITEM,  (CONST_STRPTR)"Redraw",				(CONST_STRPTR)"D",	0 , 0L, NULL },			/* Item 0 */
		{ NM_ITEM,  (CONST_STRPTR)"Toggle fullscreen",	(CONST_STRPTR)"F",	0 , 0L, NULL },		/* Item 1 */
		{ NM_ITEM,  (CONST_STRPTR)"Show updated ranges",	(CONST_STRPTR)"U",	MENUTOGGLE|CHECKIT , 0L, NULL },		/* Item 2 */
	{ NM_TITLE, (CONST_STRPTR)"Movie Control",			NULL,	0, 0L, NULL },		/* Menu 3 */
		{ NM_ITEM,  (CONST_STRPTR)"Play",					(CONST_STRPTR)"Y",	0 , 0, NULL },			/* Item 0 */
		{ NM_ITEM,  (CONST_STRPTR)"Pause",				(CONST_STRPTR)"P",	0 , 0, NULL },			/* Item 1 */
        { NM_ITEM,  (CONST_STRPTR)"Stop",					(CONST_STRPTR)"T",	0 , 0, NULL },			/* Item 2 */
        { NM_ITEM,  NM_BARLABEL,              NULL,	0, 0L, NULL },			/* Item 3 */
        { NM_ITEM,  (CONST_STRPTR)"Restart",				(CONST_STRPTR)"R",	0 , 0, NULL },			/* Item 4 */
      { NM_TITLE, (CONST_STRPTR)"Help",					NULL,	0, 0L, NULL },		/* Menu 4 */
          { NM_ITEM,  (CONST_STRPTR)"About",				(CONST_STRPTR)"?",	0 , 0, NULL },		/* Item 0 */
      { NM_END,   NULL,                     NULL,	0, 0L, NULL }           /* Terminator */
    };
/* END OF MENU DEFINITION */

using namespace std;

namespace gnash
{

AOS4AggGlue::AOS4AggGlue()
	:
_offscreenbuf(NULL),
_window(NULL),
_screen(NULL),
_agg_renderer(NULL),
_fullscreen(FALSE)
{
}

AOS4AggGlue::~AOS4AggGlue()
{
    delete [] _offscreenbuf;
	if (_window) 
	{
		IIntuition->CloseWindow(_window);
		_window = NULL;
	}
	
	_screen = NULL;
}

bool
AOS4AggGlue::init(int /*argc*/, char*** /*argv*/)
{
    return true;
}


Renderer*
AOS4AggGlue::createRenderHandler(int bpp)
{
    _bpp = bpp;

    switch (_bpp) {
      case 32:
        _agg_renderer = create_Renderer_agg("RGBA32");
        _btype        = BLITT_ARGB32;
        _ftype		  = RGBFB_R8G8B8;
        break;
      case 24:
        _agg_renderer = create_Renderer_agg("RGB24");
        _btype        = BLITT_RGB24;
        _ftype 		  = RGBFB_R8G8B8;
        break;
      case 16:
        _agg_renderer = create_Renderer_agg("RGBA16");
        _btype        = BLITT_CHUNKY;
        _ftype		  = RGBFB_R5G6B5;
        break;
      default:
        log_error (_("AGG's bit depth must be 16, 24 or 32 bits, not %d."), _bpp);
        abort();
    }
    return _agg_renderer;
}

Renderer*
AOS4AggGlue::createRenderHandler()
{
//    GNASH_REPORT_FUNCTION;
    _bpp = 24;

    _agg_renderer = create_Renderer_agg("RGB24");
    _btype        = BLITT_RGB24;

    return _agg_renderer;
}

void
AOS4AggGlue::setFullscreen()
{

	saveOrigiginalDimension(_width,_height,_xPosition,_yPosition);

	_screen = IIntuition->LockPubScreen("Workbench");
	int new_width   = _screen->Width;
	int new_height  = _screen->Height;
	IIntuition->UnlockPubScreen(NULL,_screen);
	if (_window) 
		IIntuition->CloseWindow(_window);
	_window = NULL;

    _fullscreen = true;
	
	resize(new_width, new_height);

}

void 
AOS4AggGlue::saveOrigiginalDimension(int width, int height, int xPosition, int yPosition)
{
	_orig_width     = width;
	_orig_height    = height;
	_orig_xPosition = xPosition;
	_orig_yPosition = yPosition;
}

void
AOS4AggGlue::unsetFullscreen()
{
	if (_window) 
		IIntuition->CloseWindow(_window);
	_window = NULL;
	
    _fullscreen = false;

	_width  = _orig_width;
	_height = _orig_height;
	_xPosition = _orig_xPosition;
	_yPosition = _orig_yPosition;

    resize(_width, _height);
}

bool
AOS4AggGlue::prepDrawingArea(int width, int height)
{
    int depth_bytes = _bpp / 8;  // TODO: <Udo> is this correct? Gives 1 for 15 bit modes!
	struct Screen *_menu_screen; /* Screen pointer for the menu definition */
    APTR vi;
	uint32 left = 0, top = 0;
	
    assert(_bpp % 8 == 0);

	_width = width;
	_height = height;

	if (NULL == _window)
	{
	    if ( ( _menu_screen = IIntuition->LockPubScreen ( "Workbench") ) )
	    {
		    if ((_xPosition == 0) && (_yPosition==0))
		    {
		    	left = (_menu_screen->Width-_width)/2;
			    top  = (_menu_screen->Height-_height)/2;
			}
			else
			{
				left = _xPosition;
				top  = _yPosition;
			}

        	vi = IGadTools->GetVisualInfoA(_menu_screen,NULL);
	        if (vi)
			{
				_menu = IGadTools->CreateMenusA(nm,NULL);
				if (_menu)
				{
					if (!IGadTools->LayoutMenus(_menu,vi,GTMN_NewLookMenus,TRUE,TAG_END))
					{
			        	log_error (_("Cannot layout Menu!!\n"));
					}
				}
				else
		        	log_error (_("Cannot create Menu!!\n"));
			}
			else
	        	log_error (_("Cannot get Visual Info!!\n"));
		}
		else
        	log_error (_("Cannot get WB Screen pointer!!\n"));

		_window = IIntuition->OpenWindowTags (NULL,
			WA_Activate, 		TRUE,
           	WA_Left,            left,
           	WA_Top,             top,
			WA_InnerWidth,  	width,
			WA_InnerHeight,		height,
			WA_MaxWidth,		~0,
			WA_MaxHeight,		~0,
			WA_SmartRefresh, 	TRUE,
			WA_NoCareRefresh,	TRUE,
			WA_RMBTrap, 		FALSE,
			WA_ReportMouse, 	TRUE,
			WA_IDCMP, 			IDCMP_MOUSEBUTTONS|
								IDCMP_RAWKEY|
								IDCMP_MOUSEMOVE|
								IDCMP_CLOSEWINDOW|
								IDCMP_NEWSIZE|
								IDCMP_SIZEVERIFY|
								IDCMP_MENUPICK,
			WA_Borderless,		(_fullscreen==false) ? FALSE : TRUE,
			WA_DepthGadget, 	(_fullscreen==false) ? TRUE : FALSE,
			WA_DragBar, 		(_fullscreen==false) ? TRUE : FALSE,
			WA_SizeGadget,		(_fullscreen==false) ? TRUE : FALSE,
			WA_SizeBBottom,		(_fullscreen==false) ? TRUE : FALSE,
			WA_CloseGadget, 	(_fullscreen==false) ? TRUE : FALSE,
			WA_NewLookMenus,    TRUE,
		TAG_DONE);

		if (_window)
		{
			if (_menu) IIntuition->SetMenuStrip(_window, _menu); /* Set up the menu if available */
		}
	}

    if (!_window) 
    {
        log_error (_("prepDrawingArea() failed.\n"));
        exit(EXIT_FAILURE);
    }
	
    _stride = width * depth_bytes;

#define CHUNK_SIZE (100 * 100 * depth_bytes)

    int bufsize = static_cast<int>(width * height * depth_bytes / CHUNK_SIZE + 1) * CHUNK_SIZE;

    _offscreenbuf = new unsigned char[bufsize];
	memset(_offscreenbuf,0x0,bufsize);
	
    log_debug (_("AOS4-AGG: %i byte offscreen buffer allocated"), bufsize);


    // Only the AGG renderer has the function init_buffer, which is *not* part of
    // the renderer api. It allows us to change the renderers movie size (and buffer
    // address) during run-time.
    Renderer_agg_base *renderer = static_cast<Renderer_agg_base *>(_agg_renderer);
    renderer->init_buffer(_offscreenbuf, bufsize, width, height, width*((_bpp+7)/8));

	struct RenderInfo ri;
	ri.Memory 		= _offscreenbuf;
	ri.BytesPerRow  = _stride;
	ri.RGBFormat 	= _ftype;

	if (_window)
	{
		if (!_fullscreen)
			IP96->p96WritePixelArray(&ri,0,0,_window->RPort,_window->BorderLeft,_window->BorderTop,_width,_height);
		else
			IP96->p96WritePixelArray(&ri,0,0,_window->RPort,0,0,_width,_height);
	}

    _validbounds.setTo(0, 0, width, height);

    return true;
}

struct Window *
AOS4AggGlue::getWindow(void)
{
	return _window;
}

struct Menu *
AOS4AggGlue::getMenu(void)
{
	return _menu;
}

void

AOS4AggGlue::setInvalidatedRegions(const InvalidatedRanges& ranges)
{
    _agg_renderer->set_invalidated_regions(ranges);
    _drawbounds.clear();

    for (unsigned int rno=0; rno<ranges.size(); rno++) 
    {
		// twips changed to pixels here
        geometry::Range2d<int> bounds = Intersection(_agg_renderer->world_to_pixel(ranges.getRange(rno)),_validbounds);

        // it may happen that a particular range is out of the screen, which
        // will lead to bounds==null.
        if (bounds.isNull()) continue;

        _drawbounds.push_back(bounds);
    }
}

void
AOS4AggGlue::render()
{
	if ( _drawbounds.size() == 0 ) return; // nothing to do..

    for (unsigned int bno=0; bno < _drawbounds.size(); bno++) 
	{
		geometry::Range2d<int>& bounds = _drawbounds[bno];
		render(bounds.getMinX(), bounds.getMinY(), bounds.getMaxX(), bounds.getMaxY() );
    }
}

void
AOS4AggGlue::render(int minx, int miny, int maxx, int maxy)
{
	if (!_window) return;
	
	// Update only the invalidated rectangle
	struct RenderInfo ri;
	ri.Memory 		= _offscreenbuf;
	ri.BytesPerRow = _stride;
	ri.RGBFormat 	= _ftype;

	if (!_fullscreen)
		IP96->p96WritePixelArray(&ri,minx , miny, _window->RPort, 
								minx+_window->BorderLeft,
								miny+_window->BorderTop,
								maxx - minx ,
								maxy - miny);
	else
		IP96->p96WritePixelArray(&ri,minx , miny, _window->RPort, 
								minx,
								miny,
								maxx - minx ,
								maxy - miny);
	
/*
	IGraphics->BltBitMapTags(
					BLITA_Source,			_offscreenbuf,
                    BLITA_SrcType,			_btype,
                    BLITA_SrcBytesPerRow,	_stride,
                    BLITA_Dest,				_window->RPort,
                    BLITA_DestType,			BLITT_RASTPORT,
                    BLITA_DestX,			minx,
                    BLITA_DestY,			miny,
                    BLITA_Width,			maxx,
                    BLITA_Height,			maxy,
                    TAG_END);
*/
}

void
AOS4AggGlue::resize(int width, int height)
{
    if (!_offscreenbuf) {
      // If initialisation has not taken place yet, we don't want to touch this.
      return;
    }

    delete [] _offscreenbuf;
    prepDrawingArea(width, height);
}

} // namespace gnash
