// dump.cpp: headless player that dumps a video and audio stream
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
//

// This "gui" is actually a file dumper.  It will write the raw rendered frames
// to a file.  It will not display anything on the screen, but it will print out
// information about the file to cout.  This information is in a /bin/sh sourceable
// format to make it easy to script.

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "log.h"

#include "gui.h"
#include "rc.h"
#include "sound_handler.h"
#include "gnash.h" // for get_sound_handler
#include "render_handler.h"
#include "VM.h"

#include <iostream>
#include <string>

#ifndef RENDERER_AGG
#error Dump gui requires AGG renderer
#endif

#ifndef HAVE_UNISTD_H
#error Dump gui requires unistd.h header (POSIX)
#endif

#ifndef HAVE_SYS_TIME_H
#error Dump gui requires sys/time.h header (POSIX)
#endif

// For gettimeofday  TODO:  Use something better for main loop;
//                          something that works on Windows?
#include <sys/time.h>
#include <unistd.h>

// Only include signal handlers on OS' that support it
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#include "dump.h"
#include "render_handler_agg.h"

namespace gnash 
{

bool _terminate_request = false;  // signals need to be able to access...

#ifdef HAVE_SIGNAL_H
// Called on CTRL-C and alike
void terminate_signal(int /*signo*/) {
    _terminate_request = true;
    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
}
#endif

DumpGui::DumpGui(unsigned long xid, float scale, bool loop, unsigned int depth) :
    Gui(xid, scale, loop, depth)
{

    _agg_renderer = NULL;
    _offscreenbuf = NULL;
    _offscreenbuf_size = -1;
    _file_output = NULL;
    _file_stream = NULL;
    _timeout = 0;
    _framecount = 0;
    _bpp = 32;                  // TODO:  Let user decide bits-per-pixel

    // TODO:  let user decide colorspace (see also _bpp above!)
    strncpy(_pixelformat, "BGRA32", sizeof(_pixelformat));
    if (loop) {
        std::cout << "# WARNING:  Gnash was told to loop the movie" << std::endl;
    }
    if (_xid) {
        std::cout << "# WARNING:  Ignoring request to display in X11 window" << std::endl;
    }
}

DumpGui::~DumpGui()
{
    if (_offscreenbuf) {
        free(_offscreenbuf);
        _offscreenbuf=NULL;
    }
    if ((DumpGui::_file_stream != NULL) && (DumpGui::_file_stream->is_open())) {
        DumpGui::_file_stream->close();
        std::cout << "# Finished writing file" << std::endl;
    }
    std::cout << "FRAMECOUNT=" << _framecount << "" << std::endl;
}

bool
DumpGui::init(int argc, char **argv[])
{

    char c;
    int origopterr = opterr;

    if (_xid) {
        log_error (_("Ignoring request to display in X11 window"));
    }

    optind = 0;
    opterr = 0;
    while ((c = getopt (argc, *argv, "D:")) != -1) {
        if (c == 'D') {
            _file_output = optarg;
        }
    }
    opterr = origopterr;

    if (_file_output == NULL) {
        std::cout << 
            _("# FATAL:  No filename given with -D argument.") << std::endl;
        return false;
    }

#ifdef HAVE_SIGNAL_H
    signal(SIGINT, terminate_signal);
    signal(SIGTERM, terminate_signal);
#endif

    init_dumpfile();
    _agg_renderer = create_render_handler_agg(_pixelformat);
    set_render_handler(_agg_renderer);

    return true;
}

bool
DumpGui::run()
{

    struct timeval tv;

    // TODO:  Polling is awful; should be an OS-agnostic u-sec callback
    //        timer system for C++ out there (boost?).   This code is
    //        from the fb gui, and made a bit nicer by lengthening the
    //        usleep values with educated guesses.  It's possible for
    //        slow systems to fall behind with this code, which would
    //        cause the audio stream to get out of sync (bad?).

    double timer_start;
    unsigned int sleep_usecs;

    if (gettimeofday(&tv, NULL) == 0) {
        timer_start = (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;
    }
    else {
        log_error(_("Unable to call gettimeofday."));
        return false;
    }
    
    _terminate_request = false;

    // first frame
    Gui::advance_movie(this);
    writeFrame();

    double timer_current = timer_start;
    double timer_nextframe = timer_start;
    double interval_s = (double)_interval / 1000.0;
    double timer_exit = timer_start + ((double)_timeout / 1000.0);

    while (!_terminate_request) {
  
        timer_nextframe += interval_s;

        // polling loop
        while (timer_current < timer_nextframe) {
            // sleep for 95% of remaining usecs, floored at 50
            sleep_usecs = (int)((timer_nextframe - timer_current) * 950000.0);
            usleep((sleep_usecs < 50) ? 50 : sleep_usecs);
            if (gettimeofday(&tv, NULL) == 0) {
                timer_current = (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;
            } else {
                log_error(_("Unable to call gettimeofday."));
                return false;
            }
        }

        // advance movie now
        Gui::advance_movie(this);
        writeFrame();

        // check if we've reached a timeout
        if (_timeout && (timer_current > timer_exit)) {
            _terminate_request = true;
        }
    }

    if ((timer_current - timer_start) != 0.0) {
        std::cout << "TIME=" << (timer_current - timer_start) << std::endl;
        std::cout << "FPS_ACTUAL=" << 
            ((double)(_framecount-1) / (timer_current - timer_start)) << std::endl;
    }
    return true;
}

void
DumpGui::setTimeout(unsigned int timeout)
{
    _timeout = timeout;
}

void
DumpGui::setInterval(unsigned int interval)
{
    std::cout << "INTERVAL=" << interval << std::endl;
    std::cout << "FPS_DESIRED=" << (1000.0 / (double)interval) << std::endl;
    _interval = interval;
}

bool
DumpGui::createWindow(int width, int height) 
{

    _width = width;
    _height = height;
    _validbounds.setTo(0, 0, _width-1, _height-1);
    setRenderHandlerSize(_width, _height);
    return true;
}

void
DumpGui::writeFrame()
{
    assert(_file_stream != NULL);
    if (_file_stream->is_open()) {
        _file_stream->write((char*)_offscreenbuf, _offscreenbuf_size);
        //_file_stream->flush();
        _framecount++;
    } else {
        std::cout << _("# FATAL:  Unable to write to closed output file.") << "" << std::endl;
        log_error(_("Unable to write to closed output file."));
        quit();
    }
}

void
DumpGui::init_dumpfile()
{
    if (_file_output == NULL) {
        log_error(_("Please supply a dump filename for gnash-dump."));
        exit(1);
    } else {
        _file_stream = new std::ofstream();
        _file_stream->open(_file_output);
        if (_file_stream->fail()) {
            log_error(_("Unable to write file '%s'."), _file_output);
            std::cout << "# FATAL:  Unable to write file '" << _file_output << "'" << std::endl;
            exit(1);
        } else {
            // Yes, this should go to cout.  The user needs to know this
            // information in order to process the file.  Print out in a
            // format that is easy to source into shell.
            std::cout << 
                "# Gnash created a raw dump file with the following properties:" << std::endl <<
                "COLORSPACE=" << _pixelformat << std::endl <<
                "NAME=" << _file_output  << std::endl;
        }
    }
}

void
DumpGui::setRenderHandlerSize(int width, int height)
{
    assert(width>0);
    assert(height>0);
    assert(_agg_renderer!=NULL);
    
    if ((_offscreenbuf != NULL) && (width == _width) && (height == _height))
        return;
	   
    _width = width;
    _height = height;
    std::cout << "WIDTH=" << _width  << std::endl <<
        "HEIGHT=" << _height  << std::endl;
    if (_offscreenbuf) {  
        free(_offscreenbuf);
        _offscreenbuf = NULL;
    }

    int row_size = width*((_bpp+7)/8);
    int new_bufsize = row_size * height;
  	
    // TODO: At the moment we only increase the buffer and never decrease it. Should be
    // changed sometime; movies which change the stage size will probably cause
    // all sorts of havok with raw data, but that's not our problem...
    if (new_bufsize > _offscreenbuf_size) {
        // TODO: C++ conform alternative to realloc?
        _offscreenbuf	= static_cast<unsigned char *>( realloc(_offscreenbuf, new_bufsize) );
        if (!_offscreenbuf) {
            log_debug("Could not allocate %i bytes for offscreen buffer: %s",
                      new_bufsize, strerror(errno)
                      );
            return;
        }
  
        log_debug("DUMP-AGG: %i bytes offscreen buffer allocated", new_bufsize);
  
        _offscreenbuf_size = new_bufsize;
        memset(_offscreenbuf, 0, new_bufsize);
    }
  	
    static_cast<render_handler_agg_base *> (_agg_renderer)->init_buffer
        (_offscreenbuf,
         _offscreenbuf_size,
         _width,
         _height,
         row_size
         );
}

void 
DumpGui::beforeRendering()
{
}

void
DumpGui::quit()
{
    _terminate_request = true;
}

} // end of namespace gnash

