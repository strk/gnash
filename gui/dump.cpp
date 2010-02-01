// dump.cpp: headless player that dumps a video and audio stream
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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
#include "Renderer.h"
#include "VM.h"
#include "GnashSleep.h"
#include "RunResources.h"

#include <iostream>
#include <string>
#include <fstream>

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
#include "Renderer_agg.h"

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

// TODO:  Let user decide bits-per-pixel
// TODO:  let user decide colorspace (see also _bpp above!)
DumpGui::DumpGui(unsigned long xid, float scale, bool loop, RunResources& r) :
    Gui(xid, scale, loop, r),
    _agg_renderer(0),
    _offscreenbuf(NULL),
    _offscreenbuf_size(-1),
    _timeout(0),
    _framecount(0),
    _bpp(32),
    _pixelformat("BGRA32")
{
    if (loop) {
        std::cerr << "# WARNING:  Gnash was told to loop the movie\n";
    }
    if (_xid) {
        std::cerr << "# WARNING:  Ignoring request to display in X11 window\n";
    }
}

DumpGui::~DumpGui()
{
    std::cout << "FRAMECOUNT=" << _framecount << "" << std::endl;
}

bool
DumpGui::init(int argc, char **argv[])
{

    int origopterr = opterr;

    if (_xid) {
        log_error(_("Ignoring request to display in X11 window"));
    }

    optind = 0;
    opterr = 0;
    char c;
    while ((c = getopt (argc, *argv, "D:")) != -1) {
        if (c == 'D') {
            // Terminate if no filename is given.
            if (!optarg) {
                std::cout << 
                    _("# FATAL:  No filename given with -D argument.") <<
                    std::endl;      
                return false;
            }      
            _fileOutput = optarg;
        }
    }
    opterr = origopterr;

#ifdef HAVE_SIGNAL_H
    signal(SIGINT, terminate_signal);
    signal(SIGTERM, terminate_signal);
#endif

    init_dumpfile();

    _renderer.reset(create_Renderer_agg(_pixelformat.c_str()));
    _runResources.setRenderer(_renderer);

    // We know what type of renderer it is.
    _agg_renderer = static_cast<Renderer_agg_base*>(_renderer.get());
    
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
        timer_start = static_cast<double>(tv.tv_sec) +
	static_cast<double>(tv.tv_usec) / 1000000.0;
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
    double interval_s = static_cast<double>(_interval) / 1000.0;
    double timer_exit = timer_start + (static_cast<double>(_timeout) / 1000.0);

    while (!_terminate_request) {
  
        timer_nextframe += interval_s;

        // polling loop
        while (timer_current < timer_nextframe) {
            // sleep for 95% of remaining usecs, floored at 50
            sleep_usecs = static_cast<int>(((timer_nextframe - timer_current) * 950000.0));
            gnashSleep((sleep_usecs < 50) ? 50 : sleep_usecs);
            if (gettimeofday(&tv, NULL) == 0) {
                timer_current = static_cast<double>(tv.tv_sec) +
		static_cast<double>(tv.tv_usec) / 1000000.0;
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
            (static_cast<double>((_framecount-1)) / (timer_current - timer_start)) << std::endl;
    }
    
    // In this Gui, quit() does not exit, but it is necessary to catch the
    // last frame for screenshots.
    quit();
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
    std::cout << "FPS_DESIRED=" << (1000.0 / static_cast<double>(interval)) << std::endl;
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
    if (!_fileStream) return;

    _fileStream.write(reinterpret_cast<char*>(_offscreenbuf.get()),
            _offscreenbuf_size);
    ++_framecount;
}

void
DumpGui::init_dumpfile()
{
    // May be empty if only screenshots are required.
    if (_fileOutput.empty()) {
        std::cerr << "No video dump requested.\n";
        return;
    }

    _fileStream.open(_fileOutput.c_str());
    
    if (!_fileStream) {
        log_error(_("Unable to write file '%s'."), _fileOutput);
        std::cerr << "# FATAL:  Unable to write file '" << _fileOutput << "'" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    // Yes, this should go to cout.  The user needs to know this
    // information in order to process the file.  Print out in a
    // format that is easy to source into shell.
    std::cout << 
        "# Gnash created a raw dump file with the following properties:" << std::endl <<
        "COLORSPACE=" << _pixelformat << std::endl <<
        "NAME=" << _fileOutput  << std::endl;
    
}

void
DumpGui::setRenderHandlerSize(int width, int height)
{
    assert(width > 0);
    assert(height > 0);
    assert(_agg_renderer);
    
    if (_offscreenbuf.get() && (width == _width) && (height == _height)) {
        return;
    }
	   
    _width = width;
    _height = height;
    std::cout << "WIDTH=" << _width  << std::endl <<
        "HEIGHT=" << _height  << std::endl;

    int row_size = width*((_bpp+7)/8);
    int newBufferSize = row_size * height;
  	
    // Reallocate the buffer when it shrinks or grows.
    if (newBufferSize != _offscreenbuf_size) {

        try {
              _offscreenbuf.reset(new unsigned char[newBufferSize]);
              log_debug("DUMP-AGG: %i bytes offscreen buffer allocated", newBufferSize);
        }
        catch (std::bad_alloc &e)
        {
            log_error("Could not allocate %i bytes for offscreen buffer: %s",
                  newBufferSize, e.what());
                  
              // TODO: what to do here? An assertion in Renderer_agg.cpp
              // fails if we just return.
              return;
        }
  
        _offscreenbuf_size = newBufferSize;

    }

    _agg_renderer->init_buffer(_offscreenbuf.get(),
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
DumpGui::quitUI()
{
    _terminate_request = true;
}

} // end of namespace gnash

