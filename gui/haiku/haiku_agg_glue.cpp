// haiku_agg_glue.cpp:  Glue between Haiku and Anti-Grain Geometry, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc.
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

#include "haiku_agg_glue.h"
#include "log.h"
#include "Renderer.h"
#include "Renderer_agg.h"
#include "GnashException.h"
#include "gui.h"
#include <cerrno>
#include <ostream>

#include "adipe.h"

#include <interface/Font.h>
#include <Application.h>
#include <interface/Bitmap.h>
#include <View.h>
#include <Window.h>

#include <sys/mman.h>

using namespace std;

namespace gnash
{


class BeV : public BView
{
    void Draw(BRect updateRect)
    {
        (void) updateRect;

        if (_bitmap == NULL)
            return ;
        _bitmap->SetBits(_buf, _width * _height * 4, 0, B_RGBA32);
        MovePenTo(BPoint(0,0));
        DrawBitmap(_bitmap);
    }
    void *_buf;
    int32 _width;
    int32 _height;
    BBitmap *_bitmap;
    Gui *_gui;
    ulong _pressedbuttons;
public:
    BeV(Gui *gui)
    : BView(BRect(0,0,0,0), "BeV", B_FOLLOW_ALL, B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE),
    _buf(NULL), _width(0), _height(0), _bitmap(NULL), _gui(gui), _pressedbuttons(0)
    {
        SetEventMask(B_POINTER_EVENTS | B_KEYBOARD_EVENTS);
    }

    virtual ~BeV()
    {
        delete _bitmap;
    }

    void SetBuffer(void *buf, int32 width, int32 height)
    {
        _buf = buf;
        _width = width;
        _height = height;
        LockLooper();
        ResizeTo(width-1,height-1);
        UnlockLooper();
        delete _bitmap; _bitmap = NULL;
        _bitmap = new BBitmap(BRect(0,0,_width-1, _height-1), B_RGBA32);
    }

enum gnash::key::code gnash_keycode(int code)
{
    //key::code
    //SDLGui::sdl_to_gnash_key(SDL_KeyboardEvent * key)
    //{
    gnash::key::code c(gnash::key::INVALID);

    // TODO: take care of CAPS_LOCK and NUM_LOCK and SHIFT
    // int mod = key->keysym.mod;
    //int code = key->keysym.sym;

    if(code>= 32 && code <= 127) {
        c = (gnash::key::code)(code);
        /*} else if(code >= SDLK_KP0 && code <= SDLK_KP9) {
          c = (gnash::key::code)(code - SDLK_KP0 + gnash::key::KP_0); 
          }else if(code >= SDLK_F1 && code <= SDLK_F15) {
          c = (gnash::key::code)(code - SDLK_F1 + gnash::key::F1);*/
}else 
{
    switch(code) {
        case B_UP_ARROW:       c = gnash::key::UP;       break;
        case B_DOWN_ARROW:     c = gnash::key::DOWN;     break;
        case B_RIGHT_ARROW:    c = gnash::key::RIGHT;    break;
        case B_LEFT_ARROW:     c = gnash::key::LEFT;     break;
        case B_INSERT:   c = gnash::key::INSERT;   break;
        case B_HOME:     c = gnash::key::HOME;     break;
        case B_END:      c = gnash::key::END;      break;
        case B_PAGE_UP:   c = gnash::key::PGUP;     break;
        case B_PAGE_DOWN: c = gnash::key::PGDN;     break;
                  //case SDLK_RSHIFT:
                  //case SDLK_LSHIFT:   c = gnash::key::SHIFT;    break;
                  //case SDLK_RCTRL:
                  //case SDLK_LCTRL:    c = gnash::key::CONTROL;  break;
                  //case SDLK_RALT:
                  //case SDLK_LALT:     c = gnash::key::ALT;      break;
        default: c = gnash::key::INVALID; break;
    }
}

return c;
//}
}



    void KeyDown(const char *bytes, int32 numBytes)
    {
        for (int32 i = 0; i < numBytes; ++i)
        {
            BMessage msg(GNASH_KEY_EVENT);
            if (B_OK != msg.AddInt32("key", gnash_keycode(bytes[i]))
                    || B_OK != msg.AddInt32("modifiers", 0)
                    || B_OK != msg.AddBool("pressed", true))
                QQ(1);
            else
                be_app_messenger.SendMessage(&msg);
            //XXX parse multibyte character codes
        }
    }

    void KeyUp(const char *bytes, int32 numBytes)
    {
        for (int32 i = 0; i < numBytes; ++i)
        {
            BMessage msg(GNASH_KEY_EVENT);
            if (B_OK != msg.AddInt32("key", gnash_keycode(bytes[i]))
                    || B_OK != msg.AddInt32("modifiers", 0)
                    || B_OK != msg.AddBool("pressed", false))
                QQ(1);
            else
                be_app_messenger.SendMessage(&msg);
            //XXX parse multibyte character codes
        }
    }

    void MouseMoved(BPoint point, uint32 transit, const BMessage *message)
    {
        (void) transit;
        (void) message;

        BMessage msg(GNASH_MOUSE_MOVED);
        if (B_OK != msg.AddInt32("x", point.x)
            || B_OK != msg.AddInt32("y", point.y))
            QQ(1);
        else
            be_app_messenger.SendMessage(&msg);
    }

    void MouseDown(BPoint where)
    {
        (void) where;

        MakeFocus();
        BPoint mouseWhere;
        ulong buttons;
        GetMouse(&mouseWhere, &buttons);
        // XXX many buttons clicked simultaneously
        BMessage msg(GNASH_MOUSE_CLICKED);
        if (B_OK != msg.AddBool("pressed", true)
            || B_OK != msg.AddInt32("mask", buttons))
            QQ(1);
        else
            be_app_messenger.SendMessage(&msg);
        _pressedbuttons = buttons;
    }

    void MouseUp(BPoint where)
    {
        (void) where;

        BPoint mouseWhere;
        ulong buttons;
        GetMouse(&mouseWhere, &buttons);

        BMessage msg(GNASH_MOUSE_CLICKED);
        if (B_OK != msg.AddBool("pressed", false)
            || B_OK != msg.AddInt32("mask", _pressedbuttons & ~buttons))
            QQ(1);
        else
            be_app_messenger.SendMessage(&msg);
        _pressedbuttons = buttons;
    }
};

HaikuAggGlue::HaikuAggGlue(Gui *gui, unsigned long xid)
    :
    _offscreenbuf(NULL),
    _sharebuf(NULL),
    //_screen(NULL),
    _width(0),
    _height(0),
    _bufsize(0),
    _agg_renderer(NULL),
    _view(NULL),
    _gui(gui),
    _xid(xid),
    _sharefd(-1),
    _viewhidden(false)
{
    //GNASH_REPORT_FUNCTION;
}

HaikuAggGlue::~HaikuAggGlue()
{
    //GNASH_REPORT_FUNCTION;

    if (_sharefd != -1)
    {
        if (munmap(_sharebuf, _bufsize) == -1)
            perror("munmap");
        if (close(_sharefd) == -1)
            perror("close");
    }

    delete [] _offscreenbuf;
}

bool
HaikuAggGlue::init(int /*argc*/, char*** /*argv*/, BWindow **win, std::string sharefilename)
{
    //    GNASH_REPORT_FUNCTION;
    _win = win;
    assert(_win);

    if (_xid != 0)
    {
        _sharefilename = sharefilename;

        _sharefd = open(_sharefilename.c_str(), O_RDWR);
        if (_sharefd == -1)
            return false;
        if (unlink(_sharefilename.c_str()) == -1)
            QQ(1);
    }


    return true;
}


Renderer*
HaikuAggGlue::createRenderHandler(int bpp)
{
    _bpp = bpp;

    assert(_bpp == 32); // XXX

    const char *pixelformat = "";

    switch (_bpp) {
        case 32:
            pixelformat = "BGRA32";
            // other choices include RGBA32
            break;
        case 24:
            pixelformat = "RGB24";
            break;
        case 16:
            pixelformat = "RGBA16";
            break;
      default:
            log_error (_("AGG's bit depth must be 16, 24 or 32 bits, not %d."), _bpp);
            abort();
    }
    _agg_renderer = create_Renderer_agg(pixelformat);
    if ( ! _agg_renderer )
    {
        boost::format fmt = boost::format(
                    _("Could not create AGG renderer with pixelformat %s")
                ) % pixelformat;
        throw GnashException(fmt.str());
    }
    return _agg_renderer;
}


void
HaikuAggGlue::ViewNoMore()
{
    _view = NULL;
}

void
HaikuAggGlue::ViewNeeded()
{
    QQ(8);
    if (*_win)
    {
        QQ(8);
        if (_view == NULL)
        {
            QQ(8);
            _view = new BeV(_gui);
            (*_win)->AddChild(_view);
        }
        _view->SetBuffer(_offscreenbuf, _width, _height);
    }
}

bool
HaikuAggGlue::prepDrawingArea(int width, int height, std::uint32_t sdl_flags)
{
    (void) sdl_flags;

    assert(width > 0);
    assert(height > 0);
    if (_width == width && _height == height)
        QQ(1);
        //return true;

    int depth_bytes = _bpp / 8;  // TODO: <Udo> is this correct? Gives 1 for 15 bit modes!

    assert(_bpp % 8 == 0);

    std::uint32_t rmask, gmask, bmask, amask;

    switch(_bpp) {
        case 32: // RGBA32
            // BGRA32?
            rmask = 0xFF;
            gmask = 0xFF << 8;
            bmask = 0xFF << 16;
            amask = 0xFF << 24;
            break;
        case 24: // RGB24
            rmask = 0xFF;
            gmask = 0xFF << 8;
            bmask = 0xFF << 16;
            amask = 0;
            break;
        case 16: // RGB565: 5 bits for red, 6 bits for green, and 5 bits for blue
            rmask = 0x1F << 11;
            gmask = 0x3F << 5;
            bmask = 0x1F;
            amask = 0;
            break;
        default:
            abort();
    }

#define CHUNK_SIZE (100 * 100 * depth_bytes)

    int bufsize = static_cast<int>(width * height * depth_bytes / CHUNK_SIZE + 1) * CHUNK_SIZE;

    if (_xid != 0)
    {
        int pagesize = getpagesize();
        bufsize = ((bufsize + pagesize - 1) / pagesize) * pagesize;
    }

    if (_bufsize != (unsigned)bufsize)
    {
        if (_xid != 0 && _bufsize != 0)
        {
            if (msync(_sharebuf, _bufsize, MS_INVALIDATE) != 0)
                perror("msync");
            if (munmap(_sharebuf, _bufsize) != 0)
                perror("munmap");
        }

        delete [] _offscreenbuf;
        _bufsize = bufsize;
        _offscreenbuf = new unsigned char[bufsize];
        //BlankScreen();

        if (_xid != 0)
        {
	        if (ftruncate(_sharefd, _bufsize) != 0)
		        perror("ftruncate");
            _sharebuf =
                static_cast<unsigned char*>(
                mmap(
                    (caddr_t)0,
                    _bufsize,
                    PROT_READ|PROT_WRITE,
                    MAP_SHARED,
                    _sharefd,
                    0
            ));
            if (_sharebuf == (void*) -1)
            {
                perror("mmap");
                exit(1);
            }
            memset(_sharebuf, 0xcc, _bufsize);
        }

        log_debug (_("SDL-AGG: %i byte offscreen buffer allocated"), bufsize);
    }
    _width = width;
    _height = height;


    // Only the AGG renderer has the function init_buffer, which is *not* part of
    // the renderer api. It allows us to change the renderers movie size (and buffer
    // address) during run-time.
    Renderer_agg_base * renderer =
        static_cast<Renderer_agg_base *>(_agg_renderer);
    renderer->init_buffer(_offscreenbuf, bufsize, width, height,
            width*((_bpp+7)/8));

    if (_view != NULL)
        ViewNeeded();

    _validbounds.setTo(0, 0, width-1, height-1);

    return true;
}

// Modified from fb_gui
void
HaikuAggGlue::setInvalidatedRegions(const InvalidatedRanges& ranges)
{
    _agg_renderer->set_invalidated_regions(ranges);
    _drawbounds.clear();
    
    for (unsigned int rno=0; rno<ranges.size(); rno++) {
        geometry::Range2d<int> bounds = Intersection(
            // twips changed to pixels here
            _agg_renderer->world_to_pixel(ranges.getRange(rno)),
            _validbounds);
            
        // it may happen that a particular range is out of the screen, which
        // will lead to bounds==null.
        if (bounds.isNull()) continue;
        _drawbounds.push_back(bounds);
    }
}

void
HaikuAggGlue::render()
{
    if ( _drawbounds.size() == 0 ) return; // nothing to do..
    
    for (unsigned int bno=0; bno < _drawbounds.size(); bno++) {
        geometry::Range2d<int>& bounds = _drawbounds[bno];
        render(bounds.getMinX(), bounds.getMinY(),
            bounds.getMaxX(), bounds.getMaxY() );
    }
}

void
HaikuAggGlue::render(int minx, int miny, int maxx, int maxy)
{
    if (*_win)
    {
        assert(_view);
        _view->LockLooper();
        // Update only the invalidated rectangle
        _view->Invalidate(BRect(minx,miny,maxx,maxy));
        _view->UnlockLooper();
    }
    if (_xid != 0)
    {
        if (_viewhidden == false)
            memmove(_sharebuf, _offscreenbuf, _bufsize);
    }
}

void
HaikuAggGlue::Shown()
{
    _viewhidden = false;
}

void
HaikuAggGlue::Hidden()
{
    _viewhidden = true;
}

//void
//HaikuAggGlue::BlankScreen()
//{
//    memset(_offscreenbuf, 0xcc, _bufsize);
//}


} // namespace gnash

