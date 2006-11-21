// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
// 
//
//

/* $Id: processor.cpp,v 1.38 2006/11/21 00:25:47 strk Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_file.h"
#include "container.h"
#include "gnash.h"
#include "movie_definition.h"
#include "sprite_instance.h"
#include "log.h"
#include "rc.h"
#include "URL.h"
#include "GnashException.h"

#include <iostream>
#include <cstdio>
extern "C"{
	#include <unistd.h>
#ifdef HAVE_GETOPT_H
	#include <getopt.h>
#endif
#ifndef __GNUC__
	extern int optind, getopt(int, char *const *, const char *);
#endif
}

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

#ifdef HAVE_LIBXML
extern int xml_fd;		// FIXME: this is the file descriptor
				// from XMLSocket::connect(). This
				// needs to be propogated up through
				// the layers properly, but first I
				// want to make sure it all works.
#endif // HAVE_LIBXML

const char *GPROC_VERSION = "1.0";

using namespace std;
using namespace gnash;

static void usage (const char *);

namespace {
gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
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

int
main(int argc, char *argv[])
{
    assert(tu_types_validate());
    int c;

    // scan for the two main standard GNU options
    for (c = 0; c < argc; c++) {
      if (strcmp("--help", argv[c]) == 0) {
        usage(argv[0]);
	dbglogfile.removeLog();
        exit(0);
      }
      if (strcmp("--version", argv[c]) == 0) {
        cerr << "Gnash gprocessor version: " << GPROC_VERSION;
        cerr << ", Gnash version: " << VERSION << endl;
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

    while ((c = getopt (argc, argv, "hwvap")) != -1) {
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
	      dbglogfile << "Verbose output turned on" << endl;
	      break;
	  case 'a':
#if VERBOSE_ACTION
	      dbglogfile.setActionDump(true); 
#else
              dbglogfile << "Verbose actions disabled at compile time" << endl;
#endif
	      break;
	  case 'p':
#if VERBOSE_PARSE
	      dbglogfile.setParserDump(true); 
#else
              dbglogfile << "Verbose parsing disabled at compile time" << endl;
#endif
	      break;
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

    // Clean up as much as possible, so valgrind will help find actual leaks.
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
      md = gnash::create_library_movie(URL(filename));
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
    gnash::sprite_instance* m = md->create_instance();
    if (m == NULL) {
	fprintf(stderr, "error: can't create instance of movie '%s'\n", filename);
	exit(1);
    }
    
    int	kick_count = 0;
    
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
	
	size_t	last_frame = m->get_current_frame();
	m->advance(0.010f);
	m->display();
	
	if (m->get_current_frame() == md->get_frame_count() - 1) {
	    // Done.
	    break;
	}
	
	if (m->get_play_state() == gnash::sprite_instance::STOP) {
	    // Kick the movie.
	    printf("kicking movie, kick ct = %d\n", kick_count);
	    m->goto_frame(last_frame + 1);
	    m->set_play_state(gnash::sprite_instance::PLAY);
	    kick_count++;
	    
	    if (kick_count > 10) {
		printf("movie is stalled; giving up on playing it through.\n");
		break;
	    }
	} else if (m->get_current_frame() < last_frame)	{
	    // Hm, apparently we looped back.  Skip ahead...
	    printf("loop back; jumping to frame " SIZET_FMT "\n", last_frame);
	    m->goto_frame(last_frame + 1);
	} else {
	    kick_count = 0;
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
	"gprocessor -- an SWF preprocessor for Gnash.\n"
	"\n"
	"usage: %s [options] <file>\n"
	"\n"
	"Preprocesses the given SWF movie files.  Optionally write preprocessed shape\n"
	"and font data to cache files, so the associated SWF files can be loaded\n"
	"faster by gameswf.\n"
	"\n"
	"options:\n"
	"\n"
	"  --help(-h)  Print this info.\n"	
	"  --version   Print the version numbers.\n"	
	"  -w          Write a .gsc file with preprocessed info, for each input file.\n"	
	"  -v          Be verbose; i.e. print log messages to stdout\n"
#if VERBOSE_PARSE
	"  -vp         Be verbose about movie parsing\n"
#endif
#if VERBOSE_ACTION
	"  -va         Be verbose about ActionScript\n", name
#endif
	);
}


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
