// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#include "NullSoundHandler.h"
#ifdef USE_FFMPEG
# include "ffmpeg/MediaHandlerFfmpeg.h"
#elif defined(USE_GST)
# include "gst/MediaHandlerGst.h"
#endif

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <sys/time.h>
#include <ctime>

#ifdef ENABLE_NLS
# include <clocale>
#endif

#include "ClockTime.h"
#include "movie_definition.h"
#include "MovieClip.h"
#include "movie_root.h"
#include "log.h"
#include "rc.h"
#include "handler.h"
#include "URL.h"
#include "GnashException.h"
#include "debugger.h"
#include "VM.h"
#include "noseek_fd_adapter.h"
#include "ManualClock.h"
#include "StringPredicates.h"
#include "IOChannel.h" // for proper dtor call
#include "GnashSleep.h" // for usleep comptibility.
#include "MovieFactory.h" 
#include "RunResources.h"

extern "C"{
#ifdef HAVE_GETOPT_H
	#include <getopt.h>
#endif
#ifndef __GNUC__
	extern char *optarg;
	extern int   optopt;
	extern int optind, getopt(int, char *const *, const char *);
#endif
}

// How many seconds to wait for a frame advancement 
// before kicking the movie (forcing it to next frame)
static const double waitforadvance = 5;

// How many time do we allow for loop backs
// (goto frame < current frame)
static const size_t allowloopbacks = 10;

// How many times to call 'advance' ?
// If 0 number of advance is unlimited
// (see other constraints)
// TODO: add a command-line switch to control this
static size_t limit_advances = 0;

// How much time to sleep between advances ?
// If set to -1 it will be computed based on FPS.
static long int delay = 0;

const char *GPROC_VERSION = "1.0";

using namespace std;
using namespace gnash;

namespace {
gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
#ifdef USE_DEBUGGER
gnash::Debugger& debugger = gnash::Debugger::getDefaultInstance();
#endif
}

namespace cygnal {

struct movie_data
{
    gnash::movie_definition* m_movie;
    std::string	m_filename;
};

static boost::intrusive_ptr<gnash::movie_definition> play_movie(
        const std::string& filename, const RunResources& runResources);

static bool s_do_output = false;
static bool s_stop_on_errors = true;

// How many time do we allow to hit the end ?
static size_t allowed_end_hits = 1;

double lastAdvanceTimer;

void
resetLastAdvanceTimer()
{
    // clocktime::getTicks() returns milliseconds
	lastAdvanceTimer = static_cast<double>(clocktime::getTicks()) / 1000.0;
}

double
secondsSinceLastAdvance()
{
    // clocktime::getTicks() returns milliseconds
	double now = static_cast<double>(clocktime::getTicks()) / 1000.0;
	return ( now - lastAdvanceTimer);
}

// A flag which will be used to interrupt playback
// by effect of a "quit" fscommand
//
static int quitrequested = false;

class FsCommandExecutor: public movie_root::AbstractFsCallback {
public:
	void notify(const std::string& command, const std::string& args)
	{
	    log_debug("fs_callback(%p): %s %s", command, args);

	    StringNoCaseEqual ncasecomp;
	   
	    if ( ncasecomp(command, "quit") ) quitrequested = true;
	}
};

class EventCallback: public movie_root::AbstractIfaceCallback {
public:
	std::string call(const std::string& event, const std::string& arg)
	{
	    log_debug("eventCallback: %s %s", event, arg);

	    static bool mouseShown = true;

	    // These should return "true" if the mouse was visible before
	    // the call.
	    if ( event == "Mouse.hide" ) {
		bool state = mouseShown;
		mouseShown = false;
		return state ? "true" : "false";
	    }

	    if ( event == "Mouse.show" ) {
		bool state = mouseShown;
		mouseShown = true;
		return state ? "true" : "false" ;
	    }
	    
	    // Some fake values for consistent test results.
	    
	    if ( event == "System.capabilities.screenResolutionX" ) {
		return "800";
	    }

	    if ( event == "System.capabilities.screenResolutionY" ) {
		return "640";
	    } 

	    if ( event == "System.capabilities.screenDPI" ) {
		return "72";
	    }        

	    if ( event == "System.capabilities.screenColor" ) {
		return "Color";
	    } 

	    if ( event == "System.capabilities.playerType" ) {
		return "StandAlone";
	    } 

	    return "";

	}

    bool yesNo(const std::string& /*query*/)
    {
        return true;
    }
};

EventCallback eventCallback;
FsCommandExecutor execFsCommand;

int
vm_main(int argc, char *argv[])
{
    // Enable native language support, i.e. internationalization
#ifdef ENABLE_NLS
    std::setlocale (LC_ALL, "");
    bindtextdomain (PACKAGE, LOCALEDIR);
    textdomain (PACKAGE);
#endif
    int c;

    // scan for the two main standard GNU options
    for (c = 0; c < argc; c++) {
      if (strcmp("--help", argv[c]) == 0) {
        exit(EXIT_SUCCESS);
      }
      if (strcmp("--version", argv[c]) == 0) {
        printf (_("Gnash gprocessor version: %s, Gnash version: %s\n"),
		   GPROC_VERSION, VERSION);
        exit(EXIT_SUCCESS);
      }
    }
 
    std::vector<std::string> infiles;
 
    //RcInitFile& rcfile = RcInitFile::getDefaultInstance();
    //rcfile.loadFiles();
    
    if (rcfile.verbosityLevel() > 0) {
        dbglogfile.setVerbosity(rcfile.verbosityLevel());
    }

    dbglogfile.setLogFilename(rcfile.getDebugLog());

    if (rcfile.useWriteLog()) {
        dbglogfile.setWriteDisk(true);
    }
    
    if (rcfile.useActionDump()) {
        dbglogfile.setActionDump(true);
        dbglogfile.setVerbosity();
    }
    
    if (rcfile.useParserDump()) {
        dbglogfile.setParserDump(true);
        dbglogfile.setVerbosity();
    }

    while ((c = getopt (argc, argv, ":hwvapr:gf:d:")) != -1) {
	switch (c) {
	  case 'h':
              dbglogfile.removeLog();
	      exit(EXIT_SUCCESS);
	  case 'w':
	      s_do_output = true;
	      break;
	  case 'v':
	      dbglogfile.setVerbosity();
	      log_debug ("Verbose output turned on");
	      break;
          case 'g':
#ifdef USE_DEBUGGER
              debugger.enabled(true);
              debugger.console();
              log_debug ("Setting debugger ON");
#else
              log_error (_("The debugger has been disabled at configuration time"));
#endif
	  case 'a':
#if VERBOSE_ACTION
	      dbglogfile.setActionDump(true); 
#else
              log_error (_("Verbose actions disabled at compile time"));
#endif
	      break;
	  case 'p':
#if VERBOSE_PARSE
	      dbglogfile.setParserDump(true); 
#else
              log_error (_("Verbose parsing disabled at compile time"));
#endif
	      break;
	  case 'r':
              allowed_end_hits = strtol(optarg, NULL, 0);
	      break;
	  case 'd':
              delay = strtol(optarg, NULL, 0)*1000; // delay is in microseconds
              // this will be recognized as a request to run at FPS speed
              if ( delay < 0 ) delay = -1;
	      break;
	  case 'f':
              limit_advances = strtol(optarg, NULL, 0);
	      break;
	  case ':':
              fprintf(stderr, "Missing argument for switch ``%c''\n", optopt); 
	      exit(EXIT_FAILURE);
	  case '?':
	  default:
              fprintf(stderr, "Unknown switch ``%c''\n", optopt); 
	      exit(EXIT_FAILURE);
	}
    }
    
    
    // get the file name from the command line
    while (optind < argc) {
        infiles.push_back(argv[optind]);
	    optind++;
    }

    // No file names were supplied
    if (infiles.empty()) {
	    std::cerr << "no input files" << std::endl;
        dbglogfile.removeLog();
	    exit(EXIT_FAILURE);
    }

    if (infiles.size() > 1) {
        // We're not ready for multiple runs yet.
        std::cerr << "Multiple input files not supported." << std::endl;
        dbglogfile.removeLog();
        exit(EXIT_FAILURE);
    }


#ifdef USE_FFMPEG
    std::auto_ptr<media::MediaHandler> handler(
            new gnash::media::ffmpeg::MediaHandlerFfmpeg() );
#elif defined(USE_GST)
    std::auto_ptr<media::MediaHandler> handler(
            new gnash::media::gst::MediaHandlerGst() );
#else
    std::cerr << "Neither SOUND_SDL nor SOUND_GST defined" << std::endl;
    exit(EXIT_FAILURE);
#endif
    gnash::media::MediaHandler::set(handler);

    boost::shared_ptr<sound::sound_handler> soundHandler(
            new sound::NullSoundHandler());

    std::vector<movie_data>	data;

    // Play through all the movies.
    for (std::vector<std::string>::const_iterator i = infiles.begin(), 
            e = infiles.end(); i != e; ++i) {

        RunResources runResources(*i);
        runResources.setSoundHandler(soundHandler);
	
	boost::intrusive_ptr<gnash::movie_definition> m =
            play_movie(*i, runResources);
	if (!m) {
	    if (s_stop_on_errors) {
		// Fail.
                std::cerr << "error playing through movie " << *i << std::endl;
		std::exit(EXIT_FAILURE);
	    }
        }
	
        movie_data	md;
        md.m_movie = m.get();
        md.m_filename = *i;
        data.push_back(md);
    }
    
    
    return 0;
}

// Load the named movie, make an instance, and play it, virtually.
// I.e. run through and render all the frames, even though we are not
// actually doing any output (our output handlers are disabled).
//
// What this does is warm up all the cached data in the movie, so that
// if we save that data for later, we won't have to tesselate shapes
// or build font textures again.
//
// Return the movie definition.
boost::intrusive_ptr<gnash::movie_definition>
play_movie(const std::string& filename, const RunResources& runResources)
{
    boost::intrusive_ptr<gnash::movie_definition> md;

    URL url(filename);

    try {
      if (filename == "-") {
         std::auto_ptr<IOChannel> in (
                 noseek_fd_adapter::make_stream(fileno(stdin)) );
         md = gnash::MovieFactory::makeMovie(in, filename, runResources, false);
      } else {
         if ( url.protocol() == "file" ) {
             const std::string& path = url.path();
#if 1 // add the *directory* the movie was loaded from to the local sandbox path
             size_t lastSlash = path.find_last_of('/');
             std::string dir = path.substr(0, lastSlash+1);
             rcfile.addLocalSandboxPath(dir);
             log_debug("%s appended to local sandboxes", dir.c_str());
#else // add the *file* to be loaded to the local sandbox path
             rcfile.addLocalSandboxPath(path);
             log_debug("%s appended to local sandboxes", path.c_str());
#endif
         }
         md = gnash::MovieFactory::makeMovie(url, runResources, NULL, false);
      }
    }
    catch (GnashException& ge) {
      md = NULL;
      fprintf(stderr, "%s\n", ge.what());
    }
    if (md == NULL) {
        std::cerr << "error: can't play movie: "<< filename << std::endl;
	    std::exit(EXIT_FAILURE);
    }

    float fps = md->get_frame_rate();
    long fpsDelay = long(1000000/fps);
    long clockAdvance = fpsDelay/1000;
    long localDelay = delay == -1 ? fpsDelay : delay; // microseconds

    log_debug("Will sleep %ld microseconds between iterations - "
		"fps is %g, clockAdvance is %lu", localDelay, fps, clockAdvance);


    // Use a clock advanced at every iteration to match exact FPS speed.
    ManualClock cl;
    gnash::movie_root m(cl, runResources);
    
    // Register processor to receive ActionScript events (Mouse, Stage
    // System etc).
    m.registerEventCallback(&eventCallback);
    m.registerFSCommandCallback(&execFsCommand);

    md->completeLoad();

    m.init(md.get(), MovieClip::MovieVariables(), MovieClip::MovieVariables());
    if ( quitrequested ) { // setRootMovie would execute actions in first frame
        quitrequested = false;
        return md;
    }

    log_debug("iteration, timer: %lu, localDelay: %ld\n",
            cl.elapsed(), localDelay);
    gnashSleep(localDelay);
    
    resetLastAdvanceTimer();
    int	kick_count = 0;
    size_t loop_back_count=0;
    size_t latest_frame=0;
    size_t end_hitcount=0;
    size_t nadvances=0;
    // Run through the movie.
    for (;;) {
        // @@ do we also have to run through all sprite frames
        // as well?
        //
        // @@ also, ActionScript can rescale things
        // dynamically -- we can't really do much about that I
        // guess?
        //
        // @@ Maybe we should allow the user to specify some
        // safety margin on scaled shapes.
        
        size_t	last_frame = m.get_current_frame();
        //printf("advancing clock by %lu\n", clockAdvance);
        cl.advance(clockAdvance);
        m.advance();

        if ( quitrequested ) 
        {
            quitrequested = false;
            return md;
        }

        m.display(); // FIXME: for which reason are we calling display here ??
        ++nadvances;
        if ( limit_advances && nadvances >= limit_advances) {
            log_debug("exiting after %d advances", nadvances);
            break;
        }

        size_t curr_frame = m.get_current_frame();
        
        // We reached the end, done !
        if (curr_frame >= md->get_frame_count() - 1 ) {
            if ( allowed_end_hits && ++end_hitcount >= allowed_end_hits )
            {
                log_debug("exiting after %d" 
			    " times last frame was reached", end_hitcount);
                    break;
            }
        }

        // We didn't advance 
        if (curr_frame == last_frame) {
            // Max stop counts reached, kick it
            if ( secondsSinceLastAdvance() > waitforadvance ) {
                // Kick the movie.
                if ( last_frame + 1 > md->get_frame_count() -1 ) {
                    fprintf(stderr, "Exiting after %g seconds in STOP mode at last frame\n", waitforadvance);
                    break;
                }
                fprintf(stderr, "Kicking movie after %g seconds in STOP mode, kick ct = %d\n", waitforadvance, kick_count);
                fflush(stderr);
                m.goto_frame(last_frame + 1);
                m.getRootMovie().setPlayState(gnash::MovieClip::PLAYSTATE_PLAY);
                kick_count++;

                if (kick_count > 10) {
                    printf("movie is stalled; giving up on playing it through.\n");
                    break;
                }

                    resetLastAdvanceTimer(); // It's like we advanced
            }
        }
        
        // We looped back.  Skip ahead...
        else if (m.get_current_frame() < last_frame) {
            if ( last_frame > latest_frame ) latest_frame = last_frame;
            if ( ++loop_back_count > allowloopbacks ) {
                log_debug("%d loop backs; jumping one-after "
			    "latest frame (%d)",
                        loop_back_count, latest_frame+1);
                m.goto_frame(latest_frame + 1);
                loop_back_count = 0;
            }
        } else {
            kick_count = 0;
            resetLastAdvanceTimer();
        }

        log_debug("iteration, timer: %lu, localDelay: %ld\n",
                cl.elapsed(), localDelay);
        gnashSleep(localDelay);
    }
 
    // Clear resources.
    MovieFactory::clear();
    
    return md;
}

}

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
