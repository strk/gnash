 #include <windows.h>
 #include <windowsx.h>
 
#include "plugin.h"
#include "player.h"

#include "log.h"

using namespace std;
using namespace gnash; 

extern PRLock* s_ogl;

// general initialization and shutdown
NPError NS_PluginInitialize()
{
	s_ogl = PR_NewLock();
	assert(s_ogl);
	dbglogfile << "NS_PluginInitialize " << endl;
	return NPERR_NO_ERROR;
}
 
void NS_PluginShutdown()
{
  if (s_ogl)
	{
		PR_DestroyLock(s_ogl);
		s_ogl = NULL;
	}
	dbglogfile << "NS_PluginShutdown " << endl;
}
 
 // construction and destruction of our plugin instance object
nsPluginInstanceBase * NS_NewPluginInstance(nsPluginCreateData * aCreateDataStruct)
{
	dbglogfile << "NS_NewPluginInstance " << endl;
	if (!aCreateDataStruct)
	{
		return NULL;
	}
 
	nsPluginInstance * plugin = new nsPluginInstance(aCreateDataStruct->instance);
	return plugin;
}
 
void NS_DestroyPluginInstance(nsPluginInstanceBase * aPlugin)
{
	dbglogfile << "NS_DestroyPluginInstance " << endl;
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
	mouse_buttons(0)
{
	dbglogfile << "nsPluginInstance " << endl;
	mhWnd = NULL;
}
 
nsPluginInstance::~nsPluginInstance()
{
	dbglogfile << "~nsPluginInstance " << endl;
}
 
 static LRESULT CALLBACK PluginWinProc(HWND, UINT, WPARAM, LPARAM);
 static WNDPROC lpOldProc = NULL;
 
NPBool nsPluginInstance::init(NPWindow* aWindow)
{
	dbglogfile << "***init*** " << aWindow << endl;

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
  SetWindowLong(mhWnd, GWL_USERDATA, (LONG)this);
 
  mInitialized = TRUE;
  return TRUE;
}


uint16 nsPluginInstance::HandleEvent(void* event)
{
	dbglogfile << "event" << endl;
	return 0;
}
NPError nsPluginInstance::SetWindow(NPP instance, NPWindow* window)
{
	dbglogfile << "SetWindow" << endl;
	return NPERR_NO_ERROR;
}

NPError nsPluginInstance::NewStream(NPMIMEType type, NPStream * stream,
                            NPBool seekable, uint16 * stype)
{
	dbglogfile << "NewStream" << stream->url << endl;
	return NPERR_NO_ERROR;
}

NPError nsPluginInstance::DestroyStream(NPStream * stream, NPError reason)
{
	dbglogfile << "DestroyStream" << stream->url << endl;

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
		dbglogfile << "Waiting for the thread to terminate..." << endl;
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
 
