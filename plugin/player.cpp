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
// Linking Gnash statically or dynamically with other modules is making
// a combined work based on Gnash. Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
// 
// In addition, as a special exception, the copyright holders of Gnash give
// you permission to combine Gnash with free software programs or
// libraries that are released under the GNU LGPL and/or with Mozilla, 
// so long as the linking with Mozilla, or any variant of Mozilla, is
// through its standard plug-in interface. You may copy and distribute
// such a system following the terms of the GNU GPL for Gnash and the
// licenses of the other code concerned, provided that you include the
// source code of that other code when and as the GNU GPL requires
// distribution of source code. 
// 
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is
// their choice whether to do so.  The GNU General Public License gives
// permission to release a modified version without this exception; this
// exception also makes it possible to release a modified version which
// carries forward this exception.
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// This has to be defined or we have typedef problems with Mozilla's
// headers for 64 bit types. According the the header file, the fix is
// to define this constant to turn off the older behaviour that we
// don't care about.
#define NO_NSPR_10_SUPPORT

//#include <SDL.h>
//#include <SDL_thread.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <signal.h>

#include "log.h"
#include "gnash.h"
#include "plugin.h"
#include "ogl.h"
#include "utility.h"
#include "container.h"
#include "tu_file.h"
#include "tu_types.h"
#include "xmlsocket.h"
#include "Movie.h"

// Mozilla SDK headers
#include "prinit.h"
#include "plugin.h"
#include "prlock.h"
#include "prcvar.h"
#include "prthread.h"

// Define is you just want a hard coded OpenGL graphic
//#define TEST_GRAPHIC

#ifdef HAVE_LIBXML

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
extern int xml_fd;              // FIXME: this is the file descriptor
				// from XMLSocket::connect(). This
				// needs to be propogated up through
				// the layers properly, but first I
				// want to make sure it all works.
#endif // HAVE_LIBXML

bool GLinitialized = false;
bool processing = false;

using namespace std;
using namespace gnash;


#define OVERSIZE	1.0f

static int runThread(void *nothing);
static int doneYet = 0;

static float	s_scale = 1.0f;
//static bool	s_antialiased = false;
//static int	s_bit_depth = 16;
static bool	s_verbose = false;
static bool	s_background = true;
//static bool	s_measure_performance = false;
//static bool	s_event_thread = false;
static bool	s_start_waiting = false;

//SDL_mutex *Pmutex;
static void interupt_handler (int);

static void
message_log(const char* message)
// Process a log message.
{
    if (s_verbose) {
	fputs(message, stdout);
        fflush(stdout); // needed on osx for some reason
    }
}

static tu_file*
file_opener(const char* url)
// Callback function.  This opens files for the library.
{
    return new tu_file(url, "rb");
}


static void
fs_callback(gnash::movie_interface* movie, const char* command, const char* args)
// For handling notification callbacks from ActionScript.
{
    message_log("fs_callback: '"); // __GNASH_PLAYER_H__
    message_log(command);
    message_log("' '");
    message_log(args);
    message_log("'\n");
}

int
main_loop(nsPluginInstance *inst)
{
    assert(tu_types_validate());
    float	exit_timeout = 0;
    bool	do_sound = false;
    int		delay = 31;
    int		retries = 0;
    float	tex_lod_bias;
    struct sigaction  act;

//    Pmutex = SDL_CreateMutex();
    
    const char *infile = inst->getFilename();
    
    log_msg("%s: Playing %s\n", __PRETTY_FUNCTION__, infile);
    
    // -1.0 tends to look good.
    tex_lod_bias = -1.2f;  
    
    if (infile == NULL) {
        log_msg("no input file\n");
        exit(1);
    }
    
#if 0
    int stall = 0;
    while (stall++ < 3) {
        printf("Stalling for GDB at pid %ld\n", getpid());
        sleep(10);
    }
#endif
#if 0
    gnash::set_verbose_action(true);
    gnash::set_verbose_parse(true);
#endif
// Uncomment this if you don't want debug logs stored to disk
//    dbglogfile.setWriteDisk(false);
    
    gnash::register_file_opener_callback(file_opener);
    gnash::register_fscommand_callback(fs_callback);
    
    gnash::sound_handler  *sound = NULL;
    gnash::render_handler *render = NULL;
#ifdef HAVE_SDL_MIXER_H
    if (do_sound) {
	sound = gnash::create_sound_handler_sdl();
	gnash::set_sound_handler(sound);
    }
#endif
    inst->lockDisplay();
    render = gnash::create_render_handler_ogl();
    gnash::set_render_handler(render);
    inst->freeDisplay();

    // Get info about the width & height of the movie.
    int	movie_version = 0;
    int	movie_width = 0;
    int	movie_height = 0;
    float movie_fps = 30.0f;
    gnash::get_movie_info(infile, &movie_version, &movie_width, &movie_height, &movie_fps, NULL, NULL);
    if (movie_version == 0) {
        fprintf(stderr, "error: can't get info about %s\n", infile);
        exit(1);
    }
    log_msg("Movie %s: width is %d, height is %d, version is %d\n", infile,
	    movie_width, movie_height, movie_version);

#if 1
    int	width = int(movie_width * s_scale);
    int	height = int(movie_height * s_scale);
#else
    int	width = inst->getWidth();
    int	height = inst->getHeight();
#endif
    log_msg("Passed in width is %d, height is %d\n", inst->getWidth(),
	   inst->getHeight());
    log_msg("Calculated width is %d, height is %d\n",
	    int(movie_width * s_scale), int(movie_height * s_scale));
    
    // Load the actual movie.
    inst->lockDisplay();
    gnash::movie_definition*	md = gnash::create_library_movie(infile);
    inst->freeDisplay();
    if (md == NULL) {
        fprintf(stderr, "error: can't create a movie from '%s'\n", infile);
        exit(1);
    }

    inst->lockDisplay();
    gnash::movie_interface*	m = create_library_movie_inst(md);
    inst->freeDisplay();
    if (m == NULL) {
        fprintf(stderr, "error: can't create movie instance\n");
//	inst->freeDisplay();
	exit(1);
    }
//    inst->freeDisplay();
    gnash::set_current_root(m);

    // Mouse state.

    int	mouse_x = 0;
    int	mouse_y = 0;
    int	mouse_buttons = 0;
    
    float	speed_scale = 1.0f;
    Uint32	start_ticks = 0;
    start_ticks = SDL_GetTicks();
    Uint32	last_ticks = start_ticks;
    int	frame_counter = 0;
//    int	last_logged_fps = last_ticks;

    // Trap ^C so we can kill all the threads
    act.sa_handler = interupt_handler;
//    act.sa_flags = SA_NOCLDSTOP;
    sigaction (SIGSEGV, &act, NULL);

    for (;;) {
	Uint32	ticks;
	ticks = SDL_GetTicks();
	int	delta_ticks = ticks - last_ticks;
	float	delta_t = delta_ticks / 1000.f;
	last_ticks = ticks;
        
        // Check auto timeout counter.
	if (exit_timeout > 0
	    && ticks - start_ticks > (Uint32) (exit_timeout * 1000)) {
	    dbglogfile << "Auto exiting now..." << endl;
	    break;
	}
        m = gnash::get_current_root();
        gnash::delete_unused_root();
	width = inst->getWidth();
	height = inst->getHeight();
	inst->lockDisplay();
	m->set_display_viewport(0, 0, width, height);
	inst->resizeWindow(width,height);
	inst->freeDisplay();

// // 	GLfloat ratio = (GLfloat)width / (GLfloat)height;
// // 	glViewport(0, 0, (GLint)width, (GLint)height);
// // 	gluPerspective(45.0f, ratio, 0.1f, 100.0f);

	m->set_background_alpha(s_background ? 1.0f : 0.05f);
	m->notify_mouse_state(mouse_x, mouse_y, mouse_buttons);    
        m->advance(delta_t * speed_scale);
//     if (do_render) {
//       glDisable(GL_DEPTH_TEST);	// Disable depth testing.
//       glDrawBuffer(GL_BACK);
//     }
	
        
#ifdef TEST_GRAPHIC
	dbglogfile << "We made it!!!" << endl;
	inst->drawTestScene();
#else
	dbglogfile << "Display rendered graphic!!!" << endl;
	inst->lockDisplay();
	m->display();
	inst->swapBuffers();
	inst->freeDisplay();
#endif

	frame_counter++;

#ifndef TEST_GRAPHIC
#if 1				// FIXME: run forever ?
	// See if we should exit
 	if (m->get_current_frame() + 1 == md->get_frame_count()) {
	    dbglogfile << "Reached the end of the movie..." << endl;
 	    break;
 	}
#endif
#else
	if (retries++ > 5) {
	    break;   
	}
#endif
	NPBool die = inst->getShutdown();
	if (die) {
	    dbglogfile << "Shutting down as requested..." << endl;
	    break;
	}
	
//	void *pd = PR_GetThreadPrivate(inst->getThreadKey());
//	dbglogfile << "Thread Data is: " << (char *)pd << endl;
	//glPopAttrib ();
	
	// Don't hog the CPU.
#ifdef TEST_GRAPHIC
	dbglogfile << "About to sleep for 1 second...!!!" << endl;
	sleep(1);
#else
	dbglogfile << "About to sleep for " << delay
		   << " milliseconds...!!!" << endl;
	PR_Sleep(delay);
#endif
    }
//    SDL_KillThread(thread);	// kill the network read thread
//    SDL_Quit();
    
    if (m) {
	m->drop_ref();
    }
    
    delete sound;
    delete render;
	
    // Clean up as much as possible, so valgrind will help find actual leaks.
    gnash::clear();
    
    return 0;
}

static int
runThread(void *nothing)
{
#ifdef HAVE_LIBXML
    
    //int i = 123;
    int val;
    int count = 0;
    SDL_Event *ptr;
#if 1
    SDL_Event ev;
    ev.type = SDL_USEREVENT;
    ev.user.code  = 0;
    ev.user.data1 = 0;
    ev.user.data2 = 0;
    ptr = &ev;
#else
    ptr = (SDL_Event *)ev_ptr;
    ptr->type = SDL_USEREVENT;
    ptr->user.code  = 0;
    ptr->user.data1 = 0;
    ptr->user.data2 = 0;
#endif
    
    log_msg("Initializing event thread...\n");
    
    while (gnash::check_sockets(xml_fd) == -1) {
        sleep(10); // Delay to give the socket time to
        // connect.
        continue;
    }
    
    // give everything a chance to initialize.
    // since all the frames need to be set up,
    // and this is just a more runtime performance
    // issue with CPU load.
    sleep(20);
    
    log_msg("Enabling Event Wait Mode...\n");
    s_start_waiting = true;
    
    while (!doneYet) {
        //ptr->user.data1 = (void *)i;
        if ((val = gnash::check_sockets(xml_fd)) == -1) {
            return -1; // we shouldn't be seeing any errors
        }
        // Don't push an event if there is already one in the
        // queue. XMLSocket::onData() will come around and get
        // the data anyway.
        count = SDL_PeepEvents(ptr, 1, SDL_PEEKEVENT, SDL_USEREVENT);
        // printf("%d User Events in queue\n", count);
        if ((count == 0) && (val >= 0)) {
            //printf("Pushing User Event on queue\n");
            SDL_PushEvent(ptr);
            SDL_Delay(300);	// was 300
        }
    }
#endif // HAVE_LIBXML
    
    return 0;
}

void
playerThread(void *arg)
{
    nsPluginInstance *inst = (nsPluginInstance *)arg;    
    log_trace("%s: instance is %p for %s\n", __PRETTY_FUNCTION__, inst,
	   inst->getFilename());
    
//    SDL_CondWait(gCond, playerMutex);
//    inst->condWait();

    main_loop(inst);

    log_msg("%s: Done this = %p...\n", __PRETTY_FUNCTION__, inst);

    return;
}

void
interupt_handler (int sig)
{
    dbglogfile << "Got a signal #" << sig << endl;
    
    exit(-1);
}

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
