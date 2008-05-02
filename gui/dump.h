// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#ifndef __DUMP_H__
#define __DUMP_H__

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "dsodefs.h" // for DSOEXPORT
#include "gui.h" // for inheritance

namespace gnash
{

typedef bool (*callback_t)(void*, int, void *data);

class DSOEXPORT DumpGui : public Gui
{
 public:
    DumpGui(unsigned long xid, float scale, bool loop, unsigned int depth);
    ~DumpGui();
    void beforeRendering();
    bool createMenu() { return true; }
    bool createMenuBar() { return true; }
    bool createWindow(int width, int height);
    bool createWindow(const char* /*title*/, int width, int height)
        { return createWindow(width, height); }
    bool init(int argc, char **argv[]);
    void quit();
    void renderBuffer() {return; }
    void render() { return; }
    void render(int /*minx*/, int /*miny*/, int /*maxx*/, int /*maxy*/)
         { render(); }
    bool run();
    void setInterval(unsigned int interval);
    void setTimeout(unsigned int timeout);
    bool setupEvents() { return true; }
    void setFullscreen() { return; }
    void setInvalidatedRegion(const rect& /*bounds*/) { return; }
    void setInvalidatedRegions(const InvalidatedRanges& /*ranges*/) { return; }
    void setCursor(gnash_cursor_type /*newcursor*/) { return; }
    void setRenderHandlerSize(int width, int height);
    void unsetFullscreen() { return; }
    bool want_multiple_regions() { return true; }
    bool want_redraw() { return false; }
    void writeFrame();

 private:
    unsigned int _bpp;                  /* bits per pixel */
    char _pixelformat[16];              /* colorspace name (eg, "RGB24") */
    char* _file_output;                 /* path to output file */
    std::ofstream* _file_stream;        /* stream for output file */
    unsigned int _timeout;              /* maximum length of movie */
    unsigned int _framecount;           /* number of frames rendered */
    void init_dumpfile();               /* convenience method to create dump file */
    render_handler *_agg_renderer;      /* pointer to AGG renderer */
    unsigned char *_offscreenbuf;       /* our "window" */
    int _offscreenbuf_size;             /* size of window (bytes) */

};

// end of namespace gnash 
}

// end of __DUMP_H__
#endif
