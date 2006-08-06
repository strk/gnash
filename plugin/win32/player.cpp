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

// This has to be defined or we have typedef problems with Mozilla's
// headers for 64 bit types. According the the header file, the fix is
// to define this constant to turn off the older behaviour that we
// don't care about.
#define NO_NSPR_10_SUPPORT

#	include <SDL.h>
#	include "URL.h"

#include "tu_file.h"
#include "xmlsocket.h"
#include "ogl.h"

// Mozilla SDK headers
#include "prinit.h"
#include "plugin.h"
#include "prlock.h"
#include "prcvar.h"
#include "prthread.h"

#include <GL/gl.h>

using namespace std;
using namespace gnash;

#define OVERSIZE	1.0f

static float	s_scale = 1.0f;
static bool	s_verbose = false;
static bool	s_background = true;

PRLock* s_ogl = NULL;

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


// Enable OpenGL
void EnableOpenGL(HWND hWnd, HDC * hDC, HGLRC * hRC)
{
	PIXELFORMATDESCRIPTOR pfd;
	int format;
	
	// get the device context (DC)
	*hDC = GetDC( hWnd );
	
	// set the pixel format for the DC
	ZeroMemory( &pfd, sizeof( pfd ) );
	pfd.nSize = sizeof( pfd );
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	format = ChoosePixelFormat( *hDC, &pfd );
	SetPixelFormat( *hDC, format, &pfd );
	
	// create and enable the render context (RC)
	*hRC = wglCreateContext( *hDC );
	wglMakeCurrent( *hDC, *hRC );
}

// Disable OpenGL
void DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
	wglMakeCurrent( NULL, NULL );
	wglDeleteContext( hRC );
	ReleaseDC( hWnd, hDC );
}

int
main_loop(nsPluginInstance *inst)
{
	PR_Lock(s_ogl);

	HWND hWnd = (HWND) inst->getWindow();
	HDC hDC;
	HGLRC hRC;
	EnableOpenGL( hWnd, &hDC, &hRC );

	assert(tu_types_validate());
	bool	do_sound = false;
	int		delay = 100;	// was 31

	gnash::register_fscommand_callback(fs_callback);
    
	gnash::render_handler *render = gnash::create_render_handler_ogl();
	gnash::set_render_handler(render);

	gnash::sound_handler  *sound = NULL;
#ifdef SOUND_SDL
	if (do_sound)
	{
		sound = gnash::create_sound_handler_sdl();
		gnash::set_sound_handler(sound);
	}
#endif

    // Get info about the width & height of the movie.
	int	movie_version = 0;
	int	movie_width = 0;
	int	movie_height = 0;
	float movie_fps = 30.0f;
	gnash::get_movie_info(URL(inst->getFilename()), &movie_version, &movie_width, &movie_height, &movie_fps, NULL, NULL);
	if (movie_version == 0)
	{
		dbglogfile << "error: can't get info about " << inst->getFilename() << endl;
    return -1;
	}
	log_msg("Movie %s: width is %d, height is %d, version is %d\n", inst->getFilename(),
	movie_width, movie_height, movie_version);

	// Load the actual movie.
	gnash::movie_definition*	md = gnash::create_library_movie(URL(inst->getFilename()));
	if (md == NULL)
	{
		dbglogfile << "error: can't create a movie from " << inst->getFilename() << endl;
		return -1;
	}

	gnash::movie_interface*	m = create_library_movie_inst(md);
	if (m == NULL)
	{
		dbglogfile << "error: can't create movie instance" << endl;
		return -1;
	}
	gnash::set_current_root(m);

	uint32_t	start_ticks = 0;
	start_ticks = SDL_GetTicks();
	uint32_t	last_ticks = start_ticks;

	for (;;)
	{
	
		uint32_t	ticks;
		ticks = SDL_GetTicks();
		int	delta_ticks = ticks - last_ticks;
		float	delta_t = delta_ticks / 1000.f;
		last_ticks = ticks;

		m = gnash::get_current_root();
		gnash::delete_unused_root();

		// to place on the center
		int window_width = inst->getWidth();
		int window_height = inst->getHeight();
		float xscale = (float) window_width / (float) movie_width;
		float yscale = (float) window_height / (float) movie_height;
		s_scale = min(xscale, yscale);
    int width = int(movie_width * s_scale);
    int height = int(movie_height * s_scale);
	
		int mouse_x = inst->mouse_x - ((window_width - width) >> 1);
		mouse_x = int(mouse_x / s_scale);
		int mouse_y = inst->mouse_y - ((window_height - height) >> 1);
		mouse_y = int(mouse_y / s_scale);
		if (mouse_x >= 0 && mouse_y >= 0)
		{
			m->notify_mouse_state(mouse_x, mouse_y, inst->mouse_buttons);    
		}

		m->set_display_viewport(window_width - width >> 1, window_height - height >> 1, width, height);
		m->set_background_alpha(s_background ? 1.0f : 0.05f);
		glDisable(GL_DEPTH_TEST);	// Disable depth testing.
		glDrawBuffer(GL_BACK);

		m->advance(delta_t);

		// white background
		glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);	

		m->display();
		SwapBuffers(hDC);

		// nsPluginInstance::shut() has been called for this instance.
		if (inst->getShutdown())
		{
			dbglogfile << "player: Shutting down as requested..." << endl;
	    break;
		}
	
		// Don't hog the CPU.
		PR_Unlock(s_ogl);
		PR_Sleep(5);
		PR_Lock(s_ogl);
	}

	if (m)
	{
		m->drop_ref();
	}
    
	delete sound;
	delete render;
	
	// Clean up as much as possible, so valgrind will help find actual leaks.
	gnash::clear();

	// shutdown OpenGL
	DisableOpenGL( hWnd, hDC, hRC );

	PR_Unlock(s_ogl);
	return 0;
}

void playerThread(void *arg)
{
	nsPluginInstance *inst = (nsPluginInstance *)arg;    
	log_trace("%s: instance is %p for %s\n", __PRETTY_FUNCTION__, inst,
	inst->getFilename());
   
	main_loop(inst);

	log_msg("%s: Done this = %p...\n", __PRETTY_FUNCTION__, inst);
}

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
