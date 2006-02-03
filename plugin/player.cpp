// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <SDL.h>
#include <SDL_thread.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "plugin.h"
#include "gnash.h"
#include "ogl.h"
#include "utility.h"
#include "container.h"
#include "tu_file.h"
#include "tu_types.h"
#include "xmlsocket.h"
#include "ogl_sdl.h"

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

#define OVERSIZE	1.0f

static int runThread(void *nothing);
static int doneYet = 0;

static float	s_scale = 1.0f;
static bool	s_antialiased = false;
static int	s_bit_depth = 16;
static bool	s_verbose = false;
static bool	s_background = true;
static bool	s_measure_performance = false;
static bool	s_event_thread = false;
static bool	s_start_waiting = false;

int drawGLScene(GLvoid);

static void
message_log(const char* message)
// Process a log message.
{
    if (s_verbose) {
        fputs(message, stdout);
        fflush(stdout); // needed on osx for some reason
    }
}

static void
log_callback(bool error, const char* message)
// Error callback for handling messages.
{
    if (error) {
        // Log, and also print to stderr.
        message_log(message);
        fputs(message, stderr);
    } else {
        message_log(message);
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
    bool do_sound = false;
    bool do_loop = true;
    bool sdl_abort = false;
    int  delay = 31;
    float	tex_lod_bias;
    
    const char *infile = inst->getFilename();
    
    printf("%s: Playing %s\n", __PRETTY_FUNCTION__, infile);
    
    // -1.0 tends to look good.
    tex_lod_bias = -1.2f;  
    
    if (infile == NULL) {
        printf("no input file\n");
        exit(1);
    }
    
#if 0
    int stall = 0;
    while (stall++ < 3) {
        printf("Stalling for GDB at pid %ld\n", getpid());
        sleep(10);
    }
#endif
    gnash::set_verbose_action(true);
    gnash::set_verbose_parse(true);    

    gnash::register_file_opener_callback(file_opener);
    gnash::register_fscommand_callback(fs_callback);
    if (s_verbose == true) {
        gnash::register_log_callback(log_callback);
    }
    //gnash::set_antialiased(s_antialiased);
    
    gnash::sound_handler  *sound = NULL;
    gnash::render_handler *render = NULL;
#ifdef HAVE_SDL_MIXER_H
    if (do_sound) {
	sound = gnash::create_sound_handler_sdl();
	gnash::set_sound_handler(sound);
    }
#endif
    render = gnash::create_render_handler_ogl();
    gnash::set_render_handler(render);
    
    // Get info about the width & height of the movie.
    int	movie_version = 0;
    int	movie_width = 0;
    int	movie_height = 0;
    float	movie_fps = 30.0f;
    gnash::get_movie_info(infile, &movie_version, &movie_width, &movie_height, &movie_fps, NULL, NULL);
    if (movie_version == 0) {
        fprintf(stderr, "error: can't get info about %s\n", infile);
        exit(1);
    }
    
    int	width = int(movie_width * s_scale);
    int	height = int(movie_height * s_scale);
    
    printf("Passed in width is %d, height is %d\n", inst->getWidth(),
	   inst->getHeight());
    printf("Calculated width is %d, height is %d\n", width, height);
    //atexit(SDL_Quit);
    
    SDL_EnableKeyRepeat(250, 33);  
    
    printf("%s: at line %d\n", __PRETTY_FUNCTION__, __LINE__);
    
    // Load the actual movie.
    gnash::movie_definition*	md = gnash::create_library_movie(infile);
    if (md == NULL) {
        fprintf(stderr, "error: can't create a movie from '%s'\n", infile);
        exit(1);
    }

    gnash::movie_interface*	m = create_library_movie_inst(md);
    if (m == NULL) {
        fprintf(stderr, "error: can't create movie instance\n");
        exit(1);
    }
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
    int	last_logged_fps = last_ticks;
    
    for (;;) {
        Uint32	ticks;
	ticks = SDL_GetTicks();
        int	delta_ticks = ticks - last_ticks;
        float	delta_t = delta_ticks / 1000.f;
        last_ticks = ticks;
        
        // Check auto timeout counter.
        if (exit_timeout > 0
            && ticks - start_ticks > (Uint32) (exit_timeout * 1000)) {
            // Auto exit now.
            break;
        }
        
//        drawGLScene();
        m = gnash::get_current_root();
        gnash::delete_unused_root();
        
	m->set_display_viewport(0, 0, width, height);
//	m->set_background_alpha(s_background ? 1.0f : 0.05f);
	m->notify_mouse_state(mouse_x, mouse_y, mouse_buttons);    
        m->advance(delta_t * speed_scale);
        
//     if (do_render) {
//       glDisable(GL_DEPTH_TEST);	// Disable depth testing.
//       glDrawBuffer(GL_BACK);
//     }
	
	SDL_mutexP(mutex);
        m->display();
        frame_counter++;        
	SDL_GL_SwapBuffers();
        SDL_mutexV(mutex);
	//glPopAttrib ();
	
	// Don't hog the CPU.
	SDL_Delay(delay);
    }    
	
//    SDL_KillThread(thread);	// kill the network read thread
//    SDL_Quit();
    
    if (md) {
	md->drop_ref();
    }
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
    
    printf("Initializing event thread...\n");
    
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
    
    printf("Enabling Event Wait Mode...\n");
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

int
playerThread(void *arg)
{
    printf("%s:\n", __PRETTY_FUNCTION__);
    nsPluginInstance *inst = (nsPluginInstance *)arg;
    int retries;
    
     if (!GLinitialized) {
        initGL(inst);
	GLinitialized = true;
     }
    
     while (retries++ < 2) {
#if 0
        drawGLScene();
#else
        main_loop(inst);
#endif
        SDL_Delay(20);      // don't trash the CPU
        // So we don't run forever for now.
        printf("%s(%d): FIXME: loop timed out\n",
               __PRETTY_FUNCTION__, __LINE__);
        break;
    }     
   
    return 0;
}

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
