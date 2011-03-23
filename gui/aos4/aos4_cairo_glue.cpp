// aos4_cairo_glue.cpp:  Glue between AmigaOS4 and Cairo, for Gnash.
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

#include "aos4_cairo_glue.h"
#include "log.h"
#undef ACTION_END
#include "Renderer.h"
#include "Renderer_cairo.h"
#include <cerrno>
#include <ostream>

/* START OF MENU DEFINITION */
struct NewMenu nm[] =
{
	/* Type, Label, CommKey, Flags, MutualExclude, UserData */

	{ NM_TITLE, "File",               	NULL,   0, 0L, NULL },          /* Menu 0 */
		{ NM_ITEM,  "Load File..",          "L",	0, 0L, NULL },          /* Item 0 */
		{ NM_ITEM,  "Save",                 "S",	NM_ITEMDISABLED, 0L, NULL },          /* Item 1 */
		{ NM_ITEM,  "Save As..",            "A",	NM_ITEMDISABLED, 0L, NULL },          /* Item 2 */
		{ NM_ITEM,  NM_BARLABEL,              NULL,	0, 0L, NULL },			/* Item 3 */
		{ NM_ITEM,  "Properties",           "E",	0, 0L, NULL },			/* Item 4 */
		{ NM_ITEM,  NM_BARLABEL,              NULL,	0, 0L, NULL },			/* Item 5 */
		{ NM_ITEM,  "Exit",                 "Q",	0, 0L, NULL },          /* Item 6 */
	{ NM_TITLE, "Edit",               	NULL,   0, 0L, NULL },          /* Menu 1 */
		{ NM_ITEM,  "Preferences",			"O",	0, 0L, NULL },          /* Item 0 */
	{ NM_TITLE, "View", 		            NULL,	0, 0L, NULL },		/* Menu 2 */
		{ NM_ITEM,  "Redraw",				"D",	0 , 0L, NULL },			/* Item 0 */
		{ NM_ITEM,  "Toggle fullscreen",	"F",	0 , 0L, NULL },		/* Item 1 */
		{ NM_ITEM,  "Show updated ranges",	"U",	MENUTOGGLE|CHECKIT , 0L, NULL },		/* Item 2 */
	{ NM_TITLE, "Movie Control",			NULL,	0, 0L, NULL },		/* Menu 3 */
		{ NM_ITEM,  "Play",					"Y",	0 , 0, NULL },			/* Item 0 */
		{ NM_ITEM,  "Pause",				"P",	0 , 0, NULL },			/* Item 1 */
        { NM_ITEM,  "Stop",					"T",	0 , 0, NULL },			/* Item 2 */
        { NM_ITEM,  NM_BARLABEL,              NULL,	0, 0L, NULL },			/* Item 3 */
        { NM_ITEM,  "Restart",				"R",	0 , 0, NULL },			/* Item 4 */
      { NM_TITLE, "Help",					NULL,	0, 0L, NULL },		/* Menu 4 */
          { NM_ITEM,  "About",				"?",	0 , 0, NULL },		/* Item 0 */
      { NM_END,   NULL,                     NULL,	0, 0L, NULL }           /* Terminator */
    };
/* END OF MENU DEFINITION */

using namespace std;


namespace gnash
{
AOS4CairoGlue::AOS4CairoGlue()
	:
_offscreenbuf(NULL),
_window(NULL),
_screen(NULL),
_cairo_renderer(NULL),
_fullscreen(FALSE)
{
//    GNASH_REPORT_FUNCTION;
}

AOS4CairoGlue::~AOS4CairoGlue()
{
//    GNASH_REPORT_FUNCTION;
    if ( _cairo_surface ) cairo_surface_destroy(_cairo_surface);
    if ( _cairo_handle ) cairo_destroy (_cairo_handle);

    delete [] _offscreenbuf;
	if (_window)
	{
		IIntuition->CloseWindow(_window);
		_window = NULL;
	}

	_screen = NULL;
}

bool
AOS4CairoGlue::init(int argc, char** argv[])
{
//    GNASH_REPORT_FUNCTION;
    return true;
}


Renderer*
AOS4CairoGlue::createRenderHandler(int depth)
{
//    GNASH_REPORT_FUNCTION;
    _bpp = depth;

    _cairo_renderer = renderer::cairo::create_handler();
    switch (_bpp) {
      case 32:
        _btype        = BLITT_ARGB32;
        _ftype		  = RGBFB_A8R8G8B8;
        _ctype		  = CAIRO_FORMAT_ARGB32;
        break;
      case 24:
        _btype        = BLITT_RGB24;
        _ftype 		  = RGBFB_R8G8B8;
        _ctype		  = CAIRO_FORMAT_RGB24;
        break;
      case 16:
        _btype        = BLITT_CHUNKY;
        _ftype		  = RGBFB_R5G6B5;
        _ctype		  = (cairo_format_t)4; // No 16 bit on cairo??
        break;
      default:
        log_error (_("Cairo's bit depth must be 16, 24 or 32 bits, not %d."), _bpp);
        abort();
    }

    return _cairo_renderer;
}

Renderer*
AOS4CairoGlue::createRenderHandler()
{
    _bpp = 24;

    _cairo_renderer = renderer::cairo::create_handler();
	_btype        = BLITT_RGB24;
	_ftype		  = RGBFB_R8G8B8;
    _ctype		  = CAIRO_FORMAT_RGB24;

    return _cairo_renderer;
}

void
AOS4CairoGlue::setFullscreen()
{

	saveOrigiginalDimension(_width,_height);

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
AOS4CairoGlue::saveOrigiginalDimension(int width, int height)
{
	_orig_width  = width;
	_orig_height = height;
}

void
AOS4CairoGlue::unsetFullscreen()
{
	if (_window) 
		IIntuition->CloseWindow(_window);
	_window = NULL;
	
    _fullscreen = false;

	_width  = _orig_width;
	_height = _orig_height;

    resize(_width, _height);
}

bool
AOS4CairoGlue::prepDrawingArea(int width, int height)
{
	struct Screen *_menu_screen; /* Screen pointer for the menu definition */
    APTR vi;
	uint32_t left = 0, top = 0;
	int bufsize;

	_width = width;
	_height = height;

	if (NULL == _window)
	{
	    if ( ( _menu_screen = IIntuition->LockPubScreen ( "Workbench") ) )
	    {
		    left=(_menu_screen->Width-_width)/2;
		    top=(_menu_screen->Height-_height)/2;

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

    _stride = width * _bpp;

#define CHUNK_SIZE (100 * 100 * _bpp)

    bufsize = static_cast<int>(width * height * _bpp / CHUNK_SIZE + 1) * CHUNK_SIZE;

    _offscreenbuf = new unsigned char[bufsize];
	memset(_offscreenbuf,0x0,bufsize);

    _cairo_surface =
      cairo_image_surface_create_for_data (_offscreenbuf, _ctype,
                                           width, height, _stride);

    _cairo_handle = cairo_create(_cairo_surface);

    renderer::cairo::set_context(_cairo_renderer, _cairo_handle);


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
AOS4CairoGlue::getWindow(void)
{
	return _window;
}

struct Menu *
AOS4CairoGlue::getMenu(void)
{
	return _menu;
}

void
AOS4CairoGlue::setInvalidatedRegions(const InvalidatedRanges& ranges)
{
    _cairo_renderer->set_invalidated_regions(ranges);
    _drawbounds.clear();

    for (unsigned int rno=0; rno<ranges.size(); rno++)
    {
		// twips changed to pixels here
        geometry::Range2d<int> bounds = Intersection(_cairo_renderer->world_to_pixel(ranges.getRange(rno)),_validbounds);

        // it may happen that a particular range is out of the screen, which
        // will lead to bounds==null.
        if (bounds.isNull()) continue;

        _drawbounds.push_back(bounds);
    }
}

void
AOS4CairoGlue::render()
{
	if ( _drawbounds.size() == 0 ) return; // nothing to do..

    for (unsigned int bno=0; bno < _drawbounds.size(); bno++)
	{
		geometry::Range2d<int>& bounds = _drawbounds[bno];
		render(bounds.getMinX(), bounds.getMinY(), bounds.getMaxX(), bounds.getMaxY() );
    }
}


void
AOS4CairoGlue::render(int minx, int miny, int maxx, int maxy)
{
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

}

void
AOS4CairoGlue::resize(int width, int height)
{
    GNASH_REPORT_FUNCTION;
    if (!_offscreenbuf) {
      // If initialisation has not taken place yet, we don't want to touch this.
      return;
    }

    delete [] _offscreenbuf;
    prepDrawingArea(width, height);
}


} // namespace gnash
