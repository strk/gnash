// plugin.cpp:  Windows "win32" flash player Mozilla plugin, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#define NO_NSPR_10_SUPPORT

#include <windows.h>
#include <windowsx.h>

#include "URL.h"

#include "gnash.h"
#include "ogl.h"
#include "movie_definition.h"

#include "plugin.h"

using namespace std;
using namespace gnash; 

static PRLock* s_player = NULL;

// general initialization and shutdown

NPError NS_PluginInitialize()
{
	s_player = PR_NewLock();
	assert(s_player);
//	dbglogfile << "NS_PluginInitialize " << endl;
	return NPERR_NO_ERROR;
}
 
void NS_PluginShutdown()
{
  if (s_player)
	{
		PR_DestroyLock(s_player);
		s_player = NULL;
	}
//	dbglogfile << "NS_PluginShutdown " << endl;
}
 
 // construction and destruction of our plugin instance object

nsPluginInstanceBase * NS_NewPluginInstance(nsPluginCreateData * aCreateDataStruct)
{
//	dbglogfile << "NS_NewPluginInstance " << endl;
	if (!aCreateDataStruct)
	{
		return NULL;
	}
 
	nsPluginInstance * plugin = new nsPluginInstance(aCreateDataStruct->instance);
	return plugin;
}
 
void NS_DestroyPluginInstance(nsPluginInstanceBase * aPlugin)
{
//	dbglogfile << "NS_DestroyPluginInstance " << endl;
	if (aPlugin)
	{
		delete (nsPluginInstance *)aPlugin;
	}
}
 
 // nsPluginInstance class implementation

nsPluginInstance::nsPluginInstance(NPP aInstance) : nsPluginInstanceBase(),
	mInstance(aInstance),
	mInitialized(FALSE),
	m_swf_file(""),
	m_thread(NULL),
	m_shutdown(FALSE),
	mouse_x(0),
	mouse_y(0),
	mouse_buttons(0),
	lpOldProc(NULL)
{
//	dbglogfile << "nsPluginInstance " << endl;
	mhWnd = NULL;
}
 
nsPluginInstance::~nsPluginInstance()
{
//	dbglogfile << "~nsPluginInstance " << endl;
}
 
 static LRESULT CALLBACK PluginWinProc(HWND, UINT, WPARAM, LPARAM);
 
NPBool nsPluginInstance::init(NPWindow* aWindow)
{
//	dbglogfile << "***init*** " << aWindow << endl;

	if (aWindow == NULL)
	{
		return FALSE;
	}
 
	mX = aWindow->x;
	mY = aWindow->y;
	mWidth = aWindow->width;
	mHeight = aWindow->height; 

	if (mhWnd == (HWND) aWindow->window)
	{
		return true;
	}

	mhWnd = (HWND) aWindow->window;

  // subclass window so we can intercept window messages and
  // do our drawing to it
  lpOldProc = SubclassWindow(mhWnd, (WNDPROC)PluginWinProc);
 
  // associate window with our nsPluginInstance object so we can access 
  // it in the window procedure
  SetWindowLong(mhWnd, GWL_USERDATA, (LONG) this);
 
  mInitialized = TRUE;
  return TRUE;
}

NPError nsPluginInstance::NewStream(NPMIMEType type, NPStream * stream,
                            NPBool seekable, uint16_t * stype)
{
//	dbglogfile << "NewStream" << stream->url << endl;
	return NPERR_NO_ERROR;
}

void playerThread(void *arg)
{
	nsPluginInstance *inst = (nsPluginInstance *)arg;    
	inst->main_loop();
}

NPError nsPluginInstance::DestroyStream(NPStream * stream, NPError reason)
{
//	dbglogfile << "DestroyStream" << stream->url << endl;

	m_swf_file = stream->url;
	int start = m_swf_file.find("file:///", 0);
	if (start >= 0)
	{
		m_swf_file = stream->url + 8;
	}

	assert(m_thread == NULL);

	m_thread = PR_CreateThread(PR_USER_THREAD, playerThread, this,
			      PR_PRIORITY_NORMAL, PR_GLOBAL_THREAD,
			      PR_JOINABLE_THREAD, 0);

	return NPERR_NO_ERROR;
}

void nsPluginInstance::shut()
{
	if (m_thread)
	{
//		dbglogfile << "Waiting for the thread to terminate..." << endl;
		m_shutdown = true;

		PR_JoinThread(m_thread);
		m_thread = NULL;
	}

	// subclass it back
	SubclassWindow(mhWnd, lpOldProc);
	mhWnd = NULL;
	mInitialized = FALSE;
 }
 
 NPBool nsPluginInstance::isInitialized()
 {
   return mInitialized;
 }
 
 const char * nsPluginInstance::getVersion()
 {
   return NPN_UserAgent(mInstance);
 }

// Enable OpenGL
void nsPluginInstance::EnableOpenGL()
{
	PIXELFORMATDESCRIPTOR pfd;
	int format;
	
	// get the device context (DC)
	mhDC = GetDC(mhWnd);
	
	// set the pixel format for the DC
	ZeroMemory( &pfd, sizeof( pfd ) );
	pfd.nSize = sizeof( pfd );
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	format = ChoosePixelFormat(mhDC, &pfd );
	SetPixelFormat(mhDC, format, &pfd );
	
	// create and enable the render context (RC)
	mhRC = wglCreateContext(mhDC);
	wglMakeCurrent(mhDC, mhRC);
}

// Disable OpenGL
void nsPluginInstance::DisableOpenGL()
{
	wglMakeCurrent( NULL, NULL );
	wglDeleteContext(mhRC);
	ReleaseDC(mhWnd, mhDC);
}

void nsPluginInstance::main_loop()
{

	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();

	PR_Lock(s_player);

	EnableOpenGL();

	gnash::render_handler *render = gnash::create_render_handler_ogl();
	gnash::set_render_handler(render);

	gnash::media::sound_handler  *sound = NULL;
#ifdef SOUND_SDL
	// It leads to crash
//	sound = gnash::create_sound_handler_sdl();
//	gnash::set_sound_handler(sound);
#endif

    // Get info about the width & height of the movie.
	int	movie_version = 0;
	int	movie_width = 0;
	int	movie_height = 0;
	float movie_fps = 30.0f;
	gnash::get_movie_info(URL(getFilename()), &movie_version, &movie_width, &movie_height, &movie_fps, NULL, NULL);
	if (movie_version == 0)
	{
		log_error (_("can't get info about %s"), getFilename());
    return;
	}
	log_msg(_("Movie %s: width is %d, height is %d, version is %d\n"), getFilename(),
	movie_width, movie_height, movie_version);

	// new thread must have own movie instance

	// Load the actual movie.
//	gnash::movie_definition*	md = gnash::create_library_movie(URL(getFilename()));
	gnash::movie_definition*	md = gnash::create_movie(URL(getFilename()));
	if (md == NULL)
	{
		log_error (_("can't create a movie from %s"), getFilename());
		return;
	}

//	gnash::movie_interface*	m = create_library_movie_inst(md);
	gnash::movie_interface*	m = md->create_instance();
	if (m == NULL)
	{
		log_error (_("can't create movie instance"));
		return;
	}

	uint64_t	start_ticks = 0;
	start_ticks = tu_timer::get_ticks();
	uint64_t	last_ticks = start_ticks;

	float	scale = 1.0f;
	bool	background = true;

	int delay = int(1000.0f / movie_fps);

	for (;;)
	{
		// We cannot do get_current_root() because it can belong to other thread
		// m = gnash::get_current_root();
	
		wglMakeCurrent(mhDC, mhRC);

		gnash::set_current_root(m);
	
		uint64_t	ticks;
		ticks = tu_timer::get_ticks();
		int	delta_ticks = tu_timer::get_ticks() - last_ticks;
		float	delta_t = delta_ticks / 1000.f;

		// to place on the center
		int window_width = getWidth();
		int window_height = getHeight();
		float xscale = (float) window_width / (float) movie_width;
		float yscale = (float) window_height / (float) movie_height;
		scale = min(xscale, yscale);
    int width = int(movie_width * scale);
    int height = int(movie_height * scale);
	
		int x = mouse_x - ((window_width - width) >> 1);
		x = int(x / scale);
		int y = mouse_y - ((window_height - height) >> 1);
		y = int(y / scale);
		if (x >= 0 && y >= 0)
		{
			m->notify_mouse_state(x, y, mouse_buttons);    
		}

		m->set_display_viewport(window_width - width >> 1, window_height - height >> 1, width, height);
		m->set_background_alpha(background ? 1.0f : 0.05f);
		glDisable(GL_DEPTH_TEST);	// Disable depth testing.
		glDrawBuffer(GL_BACK);

//		dbglogfile << delta_ticks << " x " << delay << endl;
		if (delta_ticks >= delay)
		{
			last_ticks = ticks;
			m->advance(delta_t);
		}

		// white background
		glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);	

		m->display();
		SwapBuffers(mhDC);

		// nsPluginInstance::shut() has been called for this instance.
		if (getShutdown())
		{
			log_msg (_("player: Shutting down as requested..."));
		        break;
		}
	
		PR_Unlock(s_player);

		// Don't hog the CPU.
		PR_Sleep(10);

		PR_Lock(s_player);
	}

	if (m)
	{
		m->drop_ref();
	}
   
	// shutdown OpenGL
	DisableOpenGL();

	PR_Unlock(s_player);
	return;
}

static LRESULT CALLBACK PluginWinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// get our plugin instance object
	nsPluginInstance *plugin = (nsPluginInstance *)GetWindowLong(hWnd, GWL_USERDATA);
	if (plugin)
	{
		switch (msg)
		{
			case WM_MOUSEMOVE:
			{
				int x = GET_X_LPARAM(lParam); 
				int y = GET_Y_LPARAM(lParam); 
 				plugin->notify_mouse_state(x, y, -1);
				break;
			}

			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			{
				int x = GET_X_LPARAM(lParam); 
				int y = GET_Y_LPARAM(lParam); 
				int	buttons = (msg == WM_LBUTTONDOWN) ? 1 : 0;
 				plugin->notify_mouse_state(x, y, buttons);
				break;
			}
			default:
//				dbglogfile << "msg " << msg << endl;
				break;
		}
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
 
