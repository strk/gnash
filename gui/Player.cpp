// Player.cpp:  Top level flash player, for gnash.
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
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef GUI_GTK
# include "gtksup.h"
# define DEFAULT_GUI guiGTK
#endif

#ifdef GUI_SDL
# include "sdlsup.h"
# define DEFAULT_GUI guiSDL
#endif

#ifdef GUI_AQUA
# include "aquasup.h"
# define DEFAULT_GUI guiAQUA
#endif

#ifdef GUI_RISCOS
# include "riscossup.h"
# define DEFAULT_GUI guiRISCOS
#endif

#ifdef GUI_FLTK
# include "fltksup.h"
# define DEFAULT_GUI guiFLTK
#endif

#ifdef GUI_KDE
# include "kdesup.h"
# define DEFAULT_GUI guiKDE
#endif

#ifdef GUI_FB
# include "fbsup.h"
# define DEFAULT_GUI guiFB
#endif

#include "NullGui.h"

#include "gnash.h" // still needed ?
#include "movie_definition.h"
#include "sound_handler.h" // for set_sound_handler and create_sound_handler_*
#include "sprite_instance.h" // for setting FlashVars
#include "movie_root.h" 
#include "Player.h"

#include "StringPredicates.h"
#include "URL.h"
#include "rc.h"
#include "GnashException.h"
#include "noseek_fd_adapter.h"
#include "VM.h"

#include "log.h"
#include <iostream>

using namespace std;
using namespace gnash;

namespace {
gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
}


/*static private*/
void
Player::setFlashVars(gnash::sprite_instance& m, const std::string& varstr)
{
	gnash::sprite_instance* si = m.get_root_movie();
	assert(si);

	typedef sprite_instance::VariableMap maptype;

	maptype vars;
	URL::parse_querystring(varstr, vars);

	si->setVariables(vars);
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
	do_sound(true),
	exit_timeout(0),
	_movie_def(0)
#ifdef GNASH_FPS_DEBUG
	,_fpsDebugTime(0.0)
#endif
	,_guiFlavor(DEFAULT_GUI)
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
	/// Initialize gnash core library
	gnashInit();

	set_use_cache_files(false);

	gnash::register_fscommand_callback(fs_callback);

}

void
Player::init_logfile()
{
    dbglogfile.setWriteDisk(false);
//    rcfile.loadFiles();
//    rcfile.dump();

    RcInitFile& rcfile = RcInitFile::getDefaultInstance();
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
    
    // If a delay was not specified yet use
    // any eventual setting for it found in 
    // the RcInitFile
    //
    // TODO: we should remove all uses of the rcfile
    //       from Player class..
    //
    if (!delay && rcfile.getTimerDelay() > 0) {
        delay = rcfile.getTimerDelay();
        log_msg (_("Timer delay set to %d milliseconds"), delay);
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
    if (do_sound) {
#ifdef SOUND_SDL
        _sound_handler.reset( gnash::create_sound_handler_sdl() );
#elif defined(SOUND_GST)
        _sound_handler.reset( gnash::create_sound_handler_gst() );
#else
        log_error(_("Sound requested but no sound support compiled in"));
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
		_gui = getGui(_guiFlavor); // throws on unsupported gui

		RcInitFile& rcfile = RcInitFile::getDefaultInstance();
		if ( rcfile.startStopped() )
		{
			_gui->stop();
		}

	}
	else
	{
		_gui.reset(new NullGui(do_loop));
	}

#ifdef GNASH_FPS_DEBUG
	if ( _fpsDebugTime )
	{
		log_debug(_("Activating FPS debugging every %g seconds"), _fpsDebugTime);
		_gui->setFpsTimerInterval(_fpsDebugTime);
	}
#endif // def GNASH_FPS_DEBUG
}

movie_definition* 
Player::load_movie()
{
	gnash::movie_definition* md=NULL;

    try {
	if ( _infile == "-" )
	{
		std::auto_ptr<tu_file> in ( noseek_fd_adapter::make_stream(fileno(stdin)) );
		md = gnash::create_movie(in, _url, false);
	}
	else
	{
		// _url should be always set at this point...
		md = gnash::create_library_movie(URL(_infile), _url.c_str(), false);
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

/* \brief Run, used to open a new flash file. Using previous initialization */
int
Player::run(int argc, char* argv[], const char* infile, const char* url)
{
    
	bool background = true;

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
	if ( ! _gui->init(argc, &argv) )
	{
		return EXIT_FAILURE;
	}

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
      width = size_t(movie_width * scale);
    }
    if (!height) {
      height = size_t(movie_height * scale);
    }

    if ( ! width || ! height )
    {
        log_error(_("Input movie has collapsed dimensions " SIZET_FMT "/" SIZET_FMT ". Giving up."), width, height);
	return EXIT_FAILURE;
    }

    // Now that we know about movie size, create gui window.
    _gui->createWindow(_url.c_str(), width, height);

    movie_root& root = VM::init(*_movie_def).getRoot();
    sprite_instance* m = root.get_root_movie();

    // Start loader thread
    _movie_def->completeLoad();

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
        //log_debug(_("Unused parameter %s = %s"),
	//	it->first.c_str(), it->second.c_str());
    }

    // Parse querystring
    setFlashVars(*m, URL(_url).querystring());

    root.set_display_viewport(0, 0, width, height);
    root.set_background_alpha(background ? 1.0f : 0.05f);

    if (!delay) {
      delay = (unsigned int) (1000 / movie_fps) ; // milliseconds per frame
    }
    _gui->setInterval(delay);

    if (exit_timeout) {
      _gui->setTimeout((unsigned int)(exit_timeout * 1000));
    }

    _gui->run();

    std::cerr << "Main loop ended, cleaning up" << std::endl;

    // Clean up as much as possible, so valgrind will help find actual leaks.
    gnash::clear();

    return EXIT_SUCCESS;
}

/*static private*/
void
Player::fs_callback(gnash::sprite_instance* movie, const char* command, const char* args)
// For handling notification callbacks from ActionScript.
{
    log_msg(_("fs_callback(%p): %s %s"), (void*)movie, command, args);
}

/* private */
std::auto_ptr<Gui>
Player::getGui(GuiFlavor which)
{
	switch (which)
	{

		case guiGTK:
#ifdef GUI_GTK // if we've been built with support for GTK
			return std::auto_ptr<Gui>(new GtkGui(windowid, scale, do_loop, bit_depth));
#endif

		case guiKDE:
#ifdef GUI_KDE // if we've been built with support for KDE
			return std::auto_ptr<Gui>(new KdeGui(windowid, scale, do_loop, bit_depth));
#else
			throw GnashException("Support for KDE gui was not compiled in");
#endif

		case guiSDL:
#ifdef GUI_SDL // if we've been built with support for SDL
			return std::auto_ptr<Gui>(new SDLGui(windowid, scale, do_loop, bit_depth));
#else
			throw GnashException("Support for SDL gui was not compiled in");
#endif

		case guiAQUA:
#ifdef GUI_AQUA // if we've been built with support for AQUA
			return std::auto_ptr<Gui>(new AquaGui(windowid, scale, do_loop, bit_depth));
#else
			throw GnashException("Support for AQUA gui was not compiled in");
#endif

		case guiRISCOS:
#ifdef GUI_RISCOS // if we've been built with support for RISCOS
			return std::auto_ptr<Gui>(new RiscosGui(windowid, scale, do_loop, bit_depth));
#else
			throw GnashException("Support for RISCOS gui was not compiled in");
#endif

		case guiFLTK:
#ifdef GUI_FLTK // if we've been built with support for FLTK
			return std::auto_ptr<Gui>(new FltkGui(windowid, scale, do_loop, bit_depth));
#else
			throw GnashException("Support for FLTK gui was not compiled in");
#endif

		case guiFB:
#ifdef GUI_FB // if we've been built with support for FB
			return std::auto_ptr<Gui>(new FBGui(windowid, scale, do_loop, bit_depth));
#else
			throw GnashException("Support for FB gui was not compiled in");
#endif

	}

	std::stringstream ss;
	ss << "Unknown gui flavor " << which << " requested";
	throw GnashException(ss.str());
}

Player::GuiFlavor
Player::parseGuiFlavorByName(const std::string& flavorName)
{
	StringNoCaseEqual match;

	if ( match(flavorName, "GTK") ) return guiGTK;
	if ( match(flavorName, "KDE") ) return guiKDE;
	if ( match(flavorName, "SDL") ) return guiSDL;
	if ( match(flavorName, "FLTK") ) return guiFLTK;
	if ( match(flavorName, "FB") ) return guiFB;
	if ( match(flavorName, "AQUA") ) return guiAQUA;
	if ( match(flavorName, "RISCOS") ) return guiRISCOS;

	std::stringstream ss;
	ss << "Unknown Gui flavor " << flavorName;
	throw GnashException(ss.str());
}
