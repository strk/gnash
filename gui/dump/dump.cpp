// dump.cpp: headless player that dumps a video and audio stream
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Free Software
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

#ifndef RENDERER_AGG
#error Dump gui requires AGG renderer
#endif

#include "dump.h"

#include <iostream>
#include <string>
#include <fstream>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/lexical_cast.hpp>
#include <csignal>
#include <cstdlib>

#ifndef HAVE_UNISTD_H
#error Dump gui requires unistd.h
#else
#include <unistd.h>
#endif

#include "Renderer_agg.h"
#include "log.h"
#include "WallClockTimer.h"
#include "gui.h"
#include "rc.h"
#include "sound_handler.h"
#include "Renderer.h"
#include "VM.h"
#include "GnashSleep.h"
#include "RunResources.h"
#include "NullSoundHandler.h"
#include "as_environment.h"
#include "as_value.h"
#include "Movie.h"

namespace gnash {

// signals need to be able to access...
std::sig_atomic_t terminate_request = false;  

// Called on CTRL-C and alike
void terminate_signal(int /*signo*/)
{
    terminate_request = true;
    std::signal(SIGINT, SIG_DFL);
    std::signal(SIGTERM, SIG_DFL);
}

// TODO:  Let user decide bits-per-pixel
// TODO:  let user decide colorspace (see also _bpp above!)
DumpGui::DumpGui(unsigned long xid, float scale, bool loop, RunResources& r)
    :
    Gui(xid, scale, loop, r),
    _agg_renderer(0),
    _offscreenbuf_size(-1),
    _timeout(0),
    _framecount(0),
    _samplesFetched(0),
    _bpp(32),
    _pixelformat("BGRA32"),
    _fileOutput(),
    _fileOutputFPS(0), // dump at every heart-beat by default
    _fileOutputAdvance(0),
    _lastVideoFrameDump(0), // this will be computed
    _sleepUS(0),
    _started(false),
    _startTime(0)
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
    int c;
    while ((c = getopt(argc, *argv, "D:S:T:")) != -1) {
        if (c == 'D') {
            // Terminate if no filename is given.
            if (!optarg) {
                std::cout << 
                    _("# FATAL:  No filename given with -D argument.") <<
                    std::endl;      
                return false;
            }      
            std::vector<std::string> file_fps;
            boost::split(file_fps, optarg,
                boost::is_any_of("@"), boost::token_compress_on);
            _fileOutput = file_fps[0];
            if ( file_fps.size() > 1 ) {
                _fileOutputFPS = boost::lexical_cast<unsigned int>(file_fps[1]);
            }
        }
        else if (c == 'S') {
            // Terminate if no filename is given.
            if (!optarg) {
                std::cout << 
                    _("# FATAL:  No sleep ms value given with -S argument.") <<
                    std::endl;
                return false;
            }      
            // we take milliseconds
            _sleepUS = std::atoi(optarg) * 1000;
        }
        else if (c == 'T') {
            // Terminate if no filename is given.
            if (!optarg) {
                std::cerr << 
                    _("# FATAL:  No trigger value given with -T argument.\n");
                return false;
            }      
            // we take milliseconds
            _startTrigger = optarg;
        }
    }
    opterr = origopterr;

    std::signal(SIGINT, terminate_signal);
    std::signal(SIGTERM, terminate_signal);

    init_dumpfile();

    if (_startTrigger.empty()) _started = true;

    _renderer.reset(create_Renderer_agg(_pixelformat.c_str()));
    _runResources.setRenderer(_renderer);

#ifdef USE_SOUND
    sound::sound_handler* mixer = _runResources.soundHandler();
    media::MediaHandler* mh = _runResources.mediaHandler();
    _soundHandler.reset(new sound::NullSoundHandler(mh, mixer));
    _runResources.setSoundHandler(_soundHandler);
#endif

    // We know what type of renderer it is.
    _agg_renderer = static_cast<Renderer_agg_base*>(_renderer.get());
    
    return true;
}

bool
DumpGui::run()
{
    if ( _fileOutputFPS ) {
        _fileOutputAdvance = static_cast<int>(1000/_fileOutputFPS);
    } else {
        _fileOutputAdvance = _interval;
        _fileOutputFPS = static_cast<int>(1000/_fileOutputAdvance);
    }

    log_debug("DumpGui entering main loop with interval of %d ms", _interval);

    // heart-beat interval, in milliseconds
    // TODO: extract this value from the swf's FPS
    //       by default and allow overriding it
    //
    unsigned int clockAdvance = _interval;

    const bool doDisplay = _fileStream.is_open();

    terminate_request = false;

    _startTime = _clock.elapsed();

    while (!terminate_request) {

        _clock.advance(clockAdvance); 

        // advance movie now
        advanceMovie(doDisplay);

        if (_started) {

            writeSamples();

            // Dump a video frame if it's time for it or no frame
            // was dumped yet
            size_t elapsed = _clock.elapsed();
            if (!_framecount || 
                    (elapsed - _lastVideoFrameDump) >= _fileOutputAdvance) {
                writeFrame();
            }

            // check if we've reached a timeout
            if (_timeout && _clock.elapsed() >= _timeout) {
                break;
            }
        }

        if (_sleepUS) gnashSleep(_sleepUS);

        if (!_started && !_startTrigger.empty()) {

            // Check whether to start
            std::string path;
            std::string var;
            if (parsePath(_startTrigger, path, var)) {
                movie_root& mr = *getStage();
                const as_environment& env = mr.getRootMovie().get_environment();
                as_object* o = findObject(env, path);
                if (o) {
                    as_value val;
                    o->get_member(getURI(mr.getVM(), "_ready"), &val);
                    if (val.equals(true, 8)) {
                        log_debug("Starting dump");
                        _started = true;
                        _startTime = _clock.elapsed();
                        _lastVideoFrameDump = _startTime;
                    }
                }
            }
        }
    }

    const std::uint32_t total_time = _clock.elapsed() - _startTime;

    std::cout << "TIME=" << total_time << std::endl;
    std::cout << "FPS_ACTUAL=" << _fileOutputFPS << std::endl;
    
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
    _interval = interval;
}

bool
DumpGui::createWindow(int width, int height) 
{
    _width = width;
    _height = height;
    _validbounds.setTo(0, 0, _width - 1, _height - 1);
    setRenderHandlerSize(_width, _height);
    return true;
}

void
DumpGui::writeFrame()
{
    if (!_fileStream.is_open()) return;

    _fileStream.write(reinterpret_cast<char*>(_offscreenbuf.get()),
            _offscreenbuf_size);

    _lastVideoFrameDump = _clock.elapsed();
    ++_framecount;
}

void
DumpGui::writeSamples()
{
#ifdef USE_SOUND
    sound::sound_handler* sh = _runResources.soundHandler();

    unsigned int ms = _clock.elapsed() - _startTime;

    // We need to fetch as many samples
    // as needed for a theoretical 44100hz loop.
    // That is 44100 samples each second.
    // 44100/1000 = x/ms
    //  x = (44100*ms) / 1000
    const unsigned int nSamples = (441 * ms) / 10;

    // We double because sound_handler interface takes
    // "mono" samples... (eh.. would be wise to change)
    unsigned int toFetch = nSamples * 2;

    // Now substract what we fetched already
    toFetch -= _samplesFetched;

    // And update _samplesFetched..
    _samplesFetched += toFetch;

    std::int16_t samples[1024];
    while (toFetch) {
        unsigned int n = std::min(toFetch, 1024u);
        // Fetching samples should trigger writing to file
        sh->fetchSamples(samples, n);
        toFetch -= n;
    }
#endif

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
        std::cerr << "# FATAL:  Unable to write file '" << _fileOutput
            << "'" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Yes, this should go to cout.  The user needs to know this
    // information in order to process the file.  Print out in a
    // format that is easy to source into shell.
    std::cout << 
        "# Gnash created a raw dump file with the following properties:\n" <<
        "COLORSPACE=" << _pixelformat << "\n" <<
        "NAME=" << _fileOutput << "\n";
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

    std::cout << "WIDTH=" << _width  << "\n" <<
        "HEIGHT=" << _height  << std::endl;

    const int row_size = width * ((_bpp+7)/8);
    const int newBufferSize = row_size * height;

    // Reallocate the buffer when it shrinks or grows.
    if (newBufferSize != _offscreenbuf_size) {
        try {
            _offscreenbuf.reset(new unsigned char[newBufferSize]);
            log_debug("DUMP-AGG: %i bytes offscreen buffer allocated",
                    newBufferSize);
        }
        catch (const std::bad_alloc& e) {
            log_error("Could not allocate %i bytes for offscreen buffer: %s",
                  newBufferSize, e.what());
                  
              // TODO: what to do here? An assertion in Renderer_agg.cpp
              // fails if we just return.
              return;
        }
  
        _offscreenbuf_size = newBufferSize;

    }

    _agg_renderer->init_buffer(_offscreenbuf.get(), _offscreenbuf_size, _width,
         _height, row_size);
}

void 
DumpGui::beforeRendering()
{
}

void
DumpGui::quitUI()
{
    terminate_request = true;
}

} // end of namespace gnash

