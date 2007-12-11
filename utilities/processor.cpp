// processor.cpp:  Flash movie processor (gprocessor command), for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

/* $Id: processor.cpp,v 1.77 2007/12/11 00:14:23 strk Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <cstdio>
#include <sys/time.h>
#include <time.h>

#ifdef ENABLE_NLS
#include <locale.h>
#endif

#include "gettext.h"
#include "tu_file.h"
#include "gnash.h"
#include "movie_definition.h"
#include "sprite_instance.h"
#include "movie_root.h"
#include "log.h"
#include "rc.h"
#include "URL.h"
#include "GnashException.h"
#include "debugger.h"
#include "VM.h"
#include "noseek_fd_adapter.h"
#include "ManualClock.h"

extern "C"{
	#include <unistd.h>
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

static void usage (const char *);

namespace {
gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
#ifdef USE_DEBUGGER
gnash::Debugger& debugger = gnash::Debugger::getDefaultInstance();
#endif
}

struct movie_data
{
    gnash::movie_definition*	m_movie;
    std::string	m_filename;
};

static gnash::movie_definition*	play_movie(const char* filename);

static bool s_do_output = false;
static bool s_stop_on_errors = true;

// How many time do we allow to hit the end ?
static size_t allowed_end_hits = 1;

double lastAdvanceTimer;

void
resetLastAdvanceTimer()
{
	using namespace tu_timer;
	lastAdvanceTimer = ticks_to_seconds(get_ticks());
}

double
secondsSinceLastAdvance()
{
	using namespace tu_timer;
	double now = ticks_to_seconds(get_ticks());
	return ( now - lastAdvanceTimer);
}

// A flag which will be used to interrupt playback
// by effect of a "quit" fscommand
//
static int quitrequested = false;

void execFsCommand(sprite_instance* movie, const char* command, const char* args)
{
    log_msg(_("fs_callback(%p): %s %s"), (void*)movie, command, args);

    if ( ! strcasecmp(command, "quit") ) quitrequested=true;
}

int
main(int argc, char *argv[])
{
    /// Initialize gnash core library
    gnashInit();

    // Enable native language support, i.e. internationalization
#ifdef ENABLE_NLS
    setlocale (LC_MESSAGES, "");
    bindtextdomain (PACKAGE, LOCALEDIR);
    textdomain (PACKAGE);
#endif
    int c;

    // scan for the two main standard GNU options
    for (c = 0; c < argc; c++) {
      if (strcmp("--help", argv[c]) == 0) {
        usage(argv[0]);
	dbglogfile.removeLog();
        exit(0);
      }
      if (strcmp("--version", argv[c]) == 0) {
        log_msg (_("Gnash gprocessor version: %s, Gnash version: %s"),
		   GPROC_VERSION, VERSION);
        dbglogfile.removeLog();
        exit(0);
      }
    }
 
    std::vector<const char*> infiles;
 
    RcInitFile& rcfile = RcInitFile::getDefaultInstance();
    rcfile.loadFiles();
    
    if (rcfile.verbosityLevel() > 0) {
        dbglogfile.setVerbosity(rcfile.verbosityLevel());
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
	      usage (argv[0]);
              dbglogfile.removeLog();
	      exit(0);
	  case 'w':
	      s_do_output = true;
	      break;
	  case 'v':
	      dbglogfile.setVerbosity();
	      log_msg (_("Verbose output turned on"));
	      break;
          case 'g':
#ifdef USE_DEBUGGER
              debugger.enabled(true);
              debugger.console();
              log_msg (_("Setting debugger ON"));
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
              delay = atoi(optarg)*1000; // delay is in microseconds
              // this will be recognized as a request to run at FPS speed
              if ( delay < 0 ) delay = -1;
	      break;
	  case 'f':
              limit_advances = strtol(optarg, NULL, 0);
	      break;
	  case ':':
              fprintf(stderr, "Missing argument for switch ``%c''\n", optopt); 
	      exit(1);
	  case '?':
	  default:
              fprintf(stderr, "Unknown switch ``%c''\n", optopt); 
	      exit(1);
	}
    }
    
    
    // get the file name from the command line
    while (optind < argc) {
        infiles.push_back(argv[optind]);
	optind++;
    }

    // No file names were supplied
    if (infiles.size() == 0) {
	printf("no input files\n");
	usage(argv[0]);
        dbglogfile.removeLog();
	exit(1);
    }

    gnash::set_use_cache_files(false);	// don't load old cache files!
        
    std::vector<movie_data>	data;

    if (infiles.size() > 1)
    {
    	// this is due to set_base_url setting, only allowed once
    	fprintf(stderr, "Multiple input files not supported.\n");
	usage(argv[0]);
	dbglogfile.removeLog();
	exit(1);
    }

    register_fscommand_callback(execFsCommand);

    // Play through all the movies.
    for (int i = 0, n = infiles.size(); i < n; i++) {

        set_base_url(URL(infiles[i]));

	gnash::movie_definition*	m = play_movie(infiles[i]);
	if (m == NULL) {
	    if (s_stop_on_errors) {
		// Fail.
		fprintf(stderr, "error playing through movie '%s', quitting\n", infiles[i]);
		exit(1);
	    }
	}
	
	movie_data	md;
	md.m_movie = m;
	md.m_filename = std::string(infiles[i]);
	data.push_back(md);
    }
    
    // Signal core lib we're willing to quit.
    gnash::clear();
    
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
gnash::movie_definition*
play_movie(const char* filename)
{
    gnash::movie_definition* md;
    try
    {
      if ( ! strcmp(filename, "-") )
      {
         std::auto_ptr<tu_file> in ( noseek_fd_adapter::make_stream(fileno(stdin)) );
         md = gnash::create_movie(in, filename, false);
      }
      else
      {
         md = gnash::create_library_movie(URL(filename), NULL, false);
      }
    }
    catch (GnashException& ge)
    {
      md = NULL;
      fprintf(stderr, "%s\n", ge.what());
    }
    if (md == NULL) {
	fprintf(stderr, "error: can't play movie '%s'\n", filename);
	exit(1);
    }

    float fps = md->get_frame_rate();
    long fpsDelay = long(1000000/fps);
    long clockAdvance = fpsDelay/1000;
    long localDelay = delay == -1 ? fpsDelay : delay; // microseconds

    printf("Will sleep %ld microseconds between iterations - fps is %g, clockAdvance is %lu\n", localDelay, fps, clockAdvance);

    // Use a clock advanced at every iteration to match exact FPS speed.
    ManualClock cl;
    gnash::movie_root& m = VM::init(*md, cl).getRoot();

    md->completeLoad();

    std::auto_ptr<movie_instance> mi ( md->create_movie_instance() );

    m.setRootMovie( mi.release() );
    if ( quitrequested )  // setRootMovie would execute actions in first frame
    {
        quitrequested = false;
        return md;
    }
    
    resetLastAdvanceTimer();
    int	kick_count = 0;
    int stop_count=0;
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
	m.advance(0.010f);

	if ( quitrequested ) 
	{
		quitrequested = false;
		return md;
	}

	m.display(); // FIXME: for which reason are we calling display here ??
	++nadvances;
	if ( limit_advances && nadvances >= limit_advances)
	{
		printf("exiting after " SIZET_FMT " advances\n", nadvances);
		break;
	}

	size_t curr_frame = m.get_current_frame();
	
	// We reached the end, done !
	if (curr_frame >= md->get_frame_count() - 1 )
	{
		if ( allowed_end_hits && ++end_hitcount >= allowed_end_hits )
		{
			printf("exiting after " SIZET_FMT 
			       " times last frame was reached\n", end_hitcount);
	    		break;
		}
	}

	// We didn't advance 
	if (curr_frame == last_frame)
	{
		// Max stop counts reached, kick it
		if ( secondsSinceLastAdvance() > waitforadvance )
		{
			stop_count=0;

			// Kick the movie.
			if ( last_frame + 1 > md->get_frame_count() -1 )
			{
				fprintf(stderr, "Exiting after %g seconds in STOP mode at last frame\n", waitforadvance);
				break;
			}
			fprintf(stderr, "Kicking movie after %g seconds in STOP mode, kick ct = %d\n", waitforadvance, kick_count);
			fflush(stderr);
			m.goto_frame(last_frame + 1);
			m.set_play_state(gnash::sprite_instance::PLAY);
			kick_count++;

			if (kick_count > 10) {
				printf("movie is stalled; giving up on playing it through.\n");
				break;
			}

	    		resetLastAdvanceTimer(); // It's like we advanced
		}
	}
	
	// We looped back.  Skip ahead...
	else if (m.get_current_frame() < last_frame)
	{
	    if ( last_frame > latest_frame ) latest_frame = last_frame;
	    if ( ++loop_back_count > allowloopbacks )
	    {
		    printf(SIZET_FMT " loop backs; jumping one-after "
				    "latest frame (" SIZET_FMT ")\n",
				    loop_back_count, latest_frame+1);
		    m.goto_frame(latest_frame + 1);
		    loop_back_count = 0;
	    }
	}
	else
	{
	    kick_count = 0;
	    stop_count = 0;
	    resetLastAdvanceTimer();
	}

	printf("iteration, timer: %lu, localDelay: %ld\n", cl.elapsed(), localDelay);
	usleep(localDelay);
    }
    
    return md;
}

static void
usage (const char *name)
{
    printf(
	_("gprocessor -- an SWF preprocessor for Gnash.\n"
	"\n"
	"usage: %s [options] <file>\n"
	"\n"
	"Preprocesses the given SWF movie files.  Optionally write preprocessed shape\n"
	"and font data to cache files, so the associated SWF files can be loaded\n"
	"faster.\n"
	"\n"
        "%s%s%s%s"), name, _(
	"options:\n"
	"\n"
	"  --help(-h)  Print this info.\n"	
	"  --version   Print the version numbers.\n"	
	"  -w          Write a .gsc file with preprocessed info, for each input file.\n"	
	"  -v          Be verbose; i.e. print log messages to stdout\n"
          ),
#if VERBOSE_PARSE
	_("  -vp         Be verbose about movie parsing\n"),
#else
	"",
#endif
#if VERBOSE_ACTION
	_("  -va         Be verbose about ActionScript\n"),
#else
	"",
#endif
	_(
	"  -d [<ms>]\n"
	"              Milliseconds delay between advances (0 by default).\n"
	"              If '-1' the delay will be computed from the FPS.\n"
	"  -r <times>  Allow the given number of complete runs.\n"
	"              Keep looping undefinitely if set to 0.\n"
	"              Default is 1 (end as soon as the last frame is reached).\n"
	"  -f <frames>  \n"
	"              Allow the given number of frame advancements.\n"
	"              Keep advancing untill any other stop condition\n"
        "              is encountered if set to 0 (default).\n")
	);
}


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
