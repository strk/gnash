// processor.cpp:  Flash movie processor (gprocessor command), for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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
#include "config.h"
#endif

#include <iostream>
#include <cstdio>
#include <sys/time.h>
#include <time.h>

#include "gettext.h"
#include "tu_file.h"
#include "container.h"
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

bool gofast = false;		// FIXME: this flag gets set based on
				// an XML message written using
				// SendCommand(""). This way a movie
				// can optimize it's own performance
				// when needed,
bool nodelay = false;           // FIXME: this flag gets set based on
				// an XML message written using
				// SendCommand(""). This way a movie
				// can optimize it's own performance
				// when needed,

extern int xml_fd;		// FIXME: this is the file descriptor
				// from XMLSocket::connect(). This
				// needs to be propogated up through
				// the layers properly, but first I
				// want to make sure it all works.

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
static int write_cache_file(const movie_data& md);

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

int
main(int argc, char *argv[])
{
    assert(tu_types_validate());

    /// Initialize gnash core library
    gnashInit();

    // Enable native language support, i.e. internationalization
    setlocale (LC_MESSAGES, "");
    bindtextdomain (PACKAGE, LOCALEDIR);
    textdomain (PACKAGE);

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

    while ((c = getopt (argc, argv, ":hwvapr:gf:")) != -1) {
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
    
    // Now append processed data.
    if (s_do_output) {
	for (int i = 0, n = data.size(); i < n; i++) {
	    int	error = write_cache_file(data[i]);
	    if (error) {
		if (s_stop_on_errors) {
		    // Fail.
		    fprintf(stderr, "error processing movie '%s', quitting\n", data[i].m_filename.c_str());
		    exit(1);
		}
	    }
	}
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
      md = gnash::create_library_movie(URL(filename), NULL, false);
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

    gnash::movie_root& m = VM::init(*md).getRoot();

    md->completeLoad();
    
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
	m.advance(0.010f);
	m.display();
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
    }
    
    return md;
}

int
// Write a cache file for the given movie.
write_cache_file(const movie_data& md)
{
    // Open cache file.
    std::string	cache_filename(md.m_filename);
    cache_filename += ".gsc";
    tu_file	out(cache_filename.c_str(), "wb");	// "gsc" == "gnash cache"
    if (out.get_error() == TU_FILE_NO_ERROR) {
	// Write out the data.
	gnash::cache_options	opt;
	md.m_movie->output_cached_data(&out, opt);
	if (out.get_error() == TU_FILE_NO_ERROR) {
	    printf(
		"wrote '%s'\n",
		cache_filename.c_str());
	} else {
	    fprintf(stderr, "error: write failure to '%s'\n", cache_filename.c_str());
	}
    } else {
	fprintf(stderr, "error: can't open '%s' for cache file output\n", cache_filename.c_str());
	return 1;
    }
    
    // // xxx temp debug code: dump cached data to stdout
    // tu_file	tu_stdout(stdout, false);
    // tu_stdout.copy_from(&cached_data);
    
    return 0;
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
