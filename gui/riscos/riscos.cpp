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


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "gnash.h"
//#include "movie_definition.h"
#include "gui.h"
#include "rc.h"
#include "riscossup.h"
#include "Renderer.h"
#include "log.h"

#include "RunResources.h"

//#include <iostream>

namespace gnash
{

RiscosGui::~RiscosGui()
{
    if (_task)
      wimp_close_down(_task);
}

RiscosGui::RiscosGui(unsigned long xid, float scale, bool loop, RunResources& r)
 : Gui(xid, scale, loop, r), _task((wimp_t)0), _window((wimp_w)0),
   _quit(false), _timeout(0), m_draw_minx(0), m_draw_miny(0),
   m_draw_maxx(0), m_draw_maxy(0), _screen_height(480), _screen_width(640)
{
}


bool
RiscosGui::init(int argc, char **argv[])
{
    GNASH_REPORT_FUNCTION;

/*    wimp_MESSAGE_LIST(4) messages = { { message_MODE_CHANGE,
                                        message_DATA_LOAD,
                                        message_DATA_OPEN,
                                        message_QUIT } };*/
    os_error *error;

    glue.init(argc, argv);

    error = xwimp_initialise(wimp_VERSION_RO38, "Gnash",
                             (wimp_message_list *)0/*&messages*/,
                             0, &_task);
    if (error) {
      log_debug("%s\n", error->errmess);
      return false;
    }

    if (!create_window())
      return false;

#ifdef RENDERER_AGG
    os_VDU_VAR_LIST(2) vduvars = { { os_VDUVAR_SCREEN_START,
                                     os_VDUVAR_END_LIST} };
    int vduvals[2];
    error = xos_read_vdu_variables((const os_vdu_var_list *)&vduvars,
                                   vduvals);
    if (error) {
      log_debug("%s\n", error->errmess);
      return false;
    }

    os_mode mode;
    os_mode_selector *mode_block;

    /* read current screenmode details */
    error = xosscreenmode_current(&mode);
    if (error) {
      log_debug("%s", error->errmess);
      return false;
    }

    if ((unsigned int)mode >= 256) {
      mode_block = (os_mode_selector *)mode;
      _screen_width = mode_block->xres;
      _screen_height = mode_block->yres;
    }

    /** \todo Mode specifiers */

    log_debug("Framebuffer address: %p\n", (void *)vduvals[0]);
    log_debug("Screen Res: %d x %d\n", _screen_width, _screen_height);

    glue.prepFramebuffer((void *)vduvals[0], _screen_width, _screen_height);
#endif

    _renderer.reset(glue.createRenderHandler());
    //set_Renderer(_renderer);
    _runResources.setRenderer(boost::shared_ptr<Renderer>(_renderer));
    // hack?
    _renderer->set_scale(1.0f, 1.0f);

    return true;
}


bool
RiscosGui::createWindow(const char *title, int width, int height,
                     int xPosition, int yPosition)
{
//First call the old createWindow function and then set the title.
//In case there's some need to not setting the title.
    title = title; // TODO: set title string

    bool ret = createWindow(width, height);
    wimp_window_state state;
    os_error *error;

    state.w = _window;
    error = xwimp_get_window_state(&state);
    if (error) {
      log_debug("%s\n", error->errmess);
      return false;
    }

    state.visible.x1 = state.visible.x0 + (width * 2);
    state.visible.y1 = state.visible.y0 + (height * 2);

    error = xwimp_open_window((wimp_open *)&state);
    if (error) {
      log_debug("%s\n", error->errmess);
      return false;
    }

    return ret;
}

bool
RiscosGui::createWindow(int width, int height)
{
    GNASH_REPORT_FUNCTION;
    _width = width;
    _height = height;

    glue.setRenderHandlerSize(width, height);

    return true;
}

void
RiscosGui::renderBuffer()
{
    // bounding box is window-relative
    wimp_window_state state;
    os_error *error;

    state.w = _window;
    error = xwimp_get_window_state(&state);
    if (error) {
      log_debug("%s\n", error->errmess);
    }

    glue.render(state.visible.x0 / 2,
                _screen_height - (state.visible.y1 / 2),
                m_draw_minx, m_draw_miny, m_draw_maxx, m_draw_maxy);
}

void
RiscosGui::setTimeout(unsigned int timeout)
{
    _timeout = os_read_monotonic_time() + timeout / 10;
}

void
RiscosGui::setInterval(unsigned int interval)
{
    _interval = interval;
}

void
RiscosGui::setInvalidatedRegion(const SWFRect& bounds)
{
    // Note: Bounds coordinates are in TWIPS

#ifdef RENDERER_AGG
    // forward to renderer
    _renderer->set_invalidated_region(bounds);

    if (bounds.width() > 1e10f) {
      // Region is entire screen. Don't convert to integer as this will
      // overflow.

      m_draw_minx = 0;
      m_draw_miny = 0;
      m_draw_maxx = _width - 1;
      m_draw_maxy = _height - 1;
    } else {
      // remember for renderBuffer()
      _renderer->world_to_pixel(&m_draw_minx, &m_draw_miny,
                                bounds.get_x_min(), bounds.get_y_min());
      _renderer->world_to_pixel(&m_draw_maxx, &m_draw_maxy,
                                bounds.get_x_max(), bounds.get_y_max());

      // add two pixels because of anti-aliasing...
      m_draw_minx = valid_coord(m_draw_minx - 2, _width);
      m_draw_miny = valid_coord(m_draw_miny - 2, _height);
      m_draw_maxx = valid_coord(m_draw_maxx + 2, _width);
      m_draw_maxy = valid_coord(m_draw_maxy + 2, _height);
    }

//    log_debug("DrawRect: (%i, %i), (%i, %i)\n",
//            m_draw_minx, m_draw_miny, m_draw_maxx, m_draw_maxy);
#endif
}

bool
RiscosGui::run()
{
    GNASH_REPORT_FUNCTION;

    os_t t, now;
    wimp_block block;
    wimp_event_no event;
    osbool more;
    os_error *error;

    t = os_read_monotonic_time();

    while (!_quit) {
      error = xwimp_poll_idle(wimp_SAVE_FP, &block, t, NULL, &event);
      if (error) {
        log_debug("%s\n", error->errmess);
        return false;
      }

      switch (event) {
      case wimp_NULL_REASON_CODE:
        now = os_read_monotonic_time();
        if (now > t) {
          if (_timeout > now) {
            _quit = true;
          } else {
            // TODO: pay attention to interval
//            if ((os_t)_interval <= (now - t) * 10) {
              advance_movie(this);
//            }
            now = os_read_monotonic_time();
            t = now + 10;
          }
        }
        break;
      case wimp_REDRAW_WINDOW_REQUEST:
        error = xwimp_redraw_window(&block.redraw, &more);
        if (error) {
          log_debug("%s\n", error->errmess);
          return false;
        }
        while (more) {
//          SWFRect bounds(block.redraw.clip.x0 / 2, block.redraw.clip.y0 / 2,
//                      block.redraw.clip.x1 / 2, block.redraw.clip.y1 / 2);
//          log_debug("Clip SWFRect: (%d, %d)(%d, %d)\n",
//                  block.redraw.clip.x0 / 2, block.redraw.clip.y0 / 2,
//                  block.redraw.clip.x1 / 2, block.redraw.clip.y1 / 2);
          // TODO: Make this use the clipping rectangle (convert to TWIPS)
          SWFRect bounds(-1e10f, -1e10f, 1e10f, 1e10f);
#ifdef RENDERER_AGG
          setInvalidatedRegion(bounds);
#endif
          renderBuffer();
          error = xwimp_get_rectangle(&block.redraw, &more);
          if (error) {
            log_debug("%s\n", error->errmess);
            return false;
          }
        }
        break;
      case wimp_OPEN_WINDOW_REQUEST:
        error = xwimp_open_window(&block.open);
        if (error)
          log_debug("%s\n", error->errmess);
        break;
      case wimp_CLOSE_WINDOW_REQUEST:
        _quit = true;
        break;
      case wimp_POINTER_LEAVING_WINDOW:
        break;
      case wimp_POINTER_ENTERING_WINDOW:
        break;
      case wimp_MOUSE_CLICK:
        break;
      case wimp_USER_DRAG_BOX:
        break;
      case wimp_MENU_SELECTION:
        break;
      case wimp_SCROLL_REQUEST:
        break;
      case wimp_LOSE_CARET:
        break;
      case wimp_GAIN_CARET:
        break;
      case wimp_POLLWORD_NON_ZERO:
        break;
      case wimp_USER_MESSAGE:
      case wimp_USER_MESSAGE_RECORDED:
      case wimp_USER_MESSAGE_ACKNOWLEDGE:
        switch (block.message.action) {
        case message_QUIT:
          _quit = true;
          break;
        default:
//          user_message(event, &(block.message));
          break;
        }
        break;
      }
    }

    return true;
}

bool
RiscosGui::createMenu()
{
    GNASH_REPORT_FUNCTION;

    return true;
}

bool
RiscosGui::setupEvents()
{
  GNASH_REPORT_FUNCTION;

  return true;
}

/**
 * Creates a window
 *
 * \return true on success, false otherwise
 */
bool RiscosGui::create_window()
{
    wimp_WINDOW(0) window = {
                { 400, 400, 800, 800 },
                0, 0,
                wimp_TOP,
                wimp_WINDOW_MOVEABLE | wimp_WINDOW_BACK_ICON |
                wimp_WINDOW_CLOSE_ICON | wimp_WINDOW_TITLE_ICON |
                wimp_WINDOW_NEW_FORMAT,
                wimp_COLOUR_BLACK, wimp_COLOUR_LIGHT_GREY,
                wimp_COLOUR_BLACK, wimp_COLOUR_WHITE,
                wimp_COLOUR_DARK_GREY, wimp_COLOUR_DARK_GREY,
                wimp_COLOUR_CREAM,
                0,
                { 0, -81928, 1300, 0 },
                wimp_ICON_TEXT | wimp_ICON_HCENTRED,
                0,
                0,
                2, 1,
                { "Gnash" },
                0
    };
    os_error *error;

    error = xwimp_create_window((wimp_window *)&window, &_window);
    if (error) {
      log_debug("%s\n", error->errmess);
      return false;
    }

    return true;
}

int
RiscosGui::valid_coord(int coord, int max)
{
        if (coord<0) return 0;
        else if (coord>=max) return max;
        return coord;
}

// end of namespace gnash
}
