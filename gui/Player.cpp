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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.

// 
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef GUI_KDE
# ifdef GUI_GTK
#  include "gtksup.h"
#  define GUI_CLASS GtkGui
# elif defined(GUI_SDL)
#  include "sdlsup.h"
#  define GUI_CLASS SDLGui
# endif
#else
# ifdef HAVE_KDE
#  include "kdesup.h"
#  define GUI_CLASS KdeGui
# else
#  error "KDE development packages not installed!"
# endif
#endif


#ifdef GUI_FB
# include "fbsup.h"
# define GUI_CLASS FBGui 
#endif


#if defined(_WIN32) || defined(WIN32)
# include "getopt_win32.h"
#endif

#include "NullGui.h"

#include "gnash.h"
#include "movie_definition.h"
#include "sprite_instance.h" // for setting FlashVars
#include "Player.h"

#include "URL.h"
#include "rc.h"
#include "GnashException.h"
#include "noseek_fd_adapter.h"

#include "log.h"
#include <iostream>

using namespace std;
using namespace gnash;



/*static private*/
void
Player::setFlashVars(gnash::movie_interface& m, const string& varstr)
{
	gnash::sprite_instance* si = m.get_root_movie();
	assert(si);

	typedef map<string, string> maptype;

	maptype vars;
	URL::parse_querystring(varstr, vars);

	for (maptype::const_iterator it=vars.begin(), itEnd=vars.end();
		it != itEnd; ++it)
	{
		const string& name = it->first;
		const string& val = it->second;
		si->set_variable(name.c_str(), val.c_str());
	}
}

Player::Player()
	:
#if defined(RENDERER_CAIRO)
	bit_depth(32),
#else
	bit_depth(16),
#endif
	scale(1.0f),
	delay(0),
	width(0),
	height(0),
	windowid(0),
	do_loop(true),
	do_render(true),
	do_sound(false),
	exit_timeout(0),
	_movie_def(0)
{
	init();
}

float
Player::setScale(float newscale)
{
	float oldscale=scale;
	scale=newscale;
	return oldscale;
}

void
Player::init()
{
	set_use_cache_files(false);

	gnash::register_fscommand_callback(fs_callback);

}

void
Player::init_logfile()
{
    dbglogfile.setWriteDisk(false);
    rcfile.loadFiles();
//    rcfile.dump();

    if (rcfile.useWriteLog()) {
        dbglogfile.setWriteDisk(true);
    }
    
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
    
    if (rcfile.getTimerDelay() > 0) {
        delay = rcfile.getTimerDelay();
        dbglogfile << "Timer delay set to " << delay << "milliseconds" << endl;
    }    

    // Remove the logfile that's created by default, since leaving a short
    // file is confusing.
    if (dbglogfile.getWriteDisk() == false) {
        dbglogfile.removeLog();
    }

}

void
Player::init_sound()
{
	if (do_sound)
	{
#ifdef SOUND_SDL
		_sound_handler.reset( gnash::create_sound_handler_sdl() );
#elif defined(SOUND_GST)
		_sound_handler.reset( gnash::create_sound_handler_gst() );
#else
		log_error("Sound requested but no sound support compiled in");
		return;
#endif

		gnash::set_sound_handler(_sound_handler.get());
	}
}


void
Player::init_gui()
{
	if ( do_render )
	{
		_gui.reset(new GUI_CLASS(windowid, scale, do_loop, bit_depth));
	}
	else
	{
		_gui.reset(new NullGui(do_loop));
	}
}

movie_definition* 
Player::load_movie()
{
	gnash::movie_definition* md=NULL;

    try {
	if ( _infile == "-" )
	{
		tu_file* in = noseek_fd_adapter::make_stream(fileno(stdin));
		md = gnash::create_movie(in, _url);
	}
	else
	{
		// _url should be always set at this point...
		md = gnash::create_library_movie(URL(_infile), _url.c_str());
	}
    } catch (const GnashException& er) {
	fprintf(stderr, "%s\n", er.what());
	md = NULL;
    }

	if ( ! md )
	{
		fprintf(stderr, "Could not load movie '%s'\n", _infile.c_str());
		return NULL;
	}

	return md;
}

int
Player::run(int argc, char* argv[], const char* infile, const char* url)
{
    
	bool background = true;
	unsigned int  delay = 0;

	assert(tu_types_validate());

	// Call this at run() time, so the caller has
	// a cache of setting some parameter before calling us...
	// (example: setDoSound(), setWindowId() etc.. ) 
	init_logfile();
	init_sound();
	init_gui();
   
	// No file name was supplied
	assert (infile);
	_infile = infile;

	// Set base url
	if ( _baseurl.empty() )
	{
		if ( url ) _baseurl = url;
		else if ( ! strcmp(infile, "-") ) _baseurl = URL("./").str();
		else _baseurl = infile;
	}

	// Set _root._url (either explicit of from infile)
	if ( url ) {
		_url=std::string(url);
	}  else {
		_url=std::string(infile);
	}


	// Initialize gui (we need argc/argv for this)
	// note that this will also initialize the renderer
	// which is *required* during movie loading
	_gui->init(argc, &argv);

	// Set base url for this movie (needed before parsing)
	gnash::set_base_url(URL(_baseurl));

	// Load the actual movie.
	_movie_def = load_movie();
	if ( ! _movie_def )
	{
		return EXIT_FAILURE;
	}


    // Get info about the width & height of the movie.
    int movie_width = static_cast<int>(_movie_def->get_width_pixels());
    int movie_height = static_cast<int>(_movie_def->get_height_pixels());
    float movie_fps = _movie_def->get_frame_rate();

    if (!width) {
      width = int(movie_width * scale);
    }
    if (!height) {
      height = int(movie_height * scale);
    }


    // Now that we know about movie size, create gui window.
    _gui->createWindow(infile, width, height);

    gnash::movie_interface *m = create_library_movie_inst(_movie_def);
    assert(m);

    // Parse parameters
    for ( map<string,string>::const_iterator it=params.begin(),
		itEnd=params.end(); it != itEnd; ++it)
    {
	// todo: use a case-insensitive string type
    	if ( it->first == "flashvars" || it->first == "FlashVars" )
	{
		setFlashVars(*m, it->second);
		continue;
	}

	// too much noise...
        //log_warning("Unused parameter %s = %s",
	//	it->first.c_str(), it->second.c_str());
    }


    gnash::set_current_root(m);

    m->set_display_viewport(0, 0, width, height);
    m->set_background_alpha(background ? 1.0f : 0.05f);

    if (!delay) {
      delay = (unsigned int) (1000 / movie_fps) ; // milliseconds per frame
    }
    _gui->setInterval(delay);

    if (exit_timeout) {
      _gui->setTimeout((unsigned int)(exit_timeout * 1000));
    }

    // @@ is it ok for 'app' to be NULL ?
    // (this would be the case when USE_KDE is not defined)
    _gui->run();

    // Clean up as much as possible, so valgrind will help find actual leaks.
    gnash::clear();

    return EXIT_SUCCESS;
}

/*static private*/
void
Player::fs_callback(gnash::movie_interface* movie, const char* command, const char* args)
// For handling notification callbacks from ActionScript.
{
    log_msg("fs_callback(%p): %s %s'", (void*)movie, command, args);
}

