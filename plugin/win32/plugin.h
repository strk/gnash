#ifndef __PLUGIN_H__
#define __PLUGIN_H__
 
#include "pluginbase.h"
#include <string>

// Mozilla SDK headers
#include "prinit.h"
#include "prlock.h"
#include "prcvar.h"
#include "prerr.h"
#include "prerror.h"
#include "prthread.h"

class nsPluginInstance : public nsPluginInstanceBase
{
	public:
	nsPluginInstance(NPP aInstance);
	~nsPluginInstance();
 
	NPBool init(NPWindow* aWindow);
	void shut();
	NPBool isInitialized();
	uint16 HandleEvent(void* event);
	NPError SetWindow(NPP instance, NPWindow* window);
	NPError nsPluginInstance::NewStream(NPMIMEType type, NPStream * stream,
                            NPBool seekable, uint16 * stype);
	NPError nsPluginInstance::DestroyStream(NPStream * stream, NPError reason);



	// locals
	const char* getVersion();
	HWND getWindow() { return mhWnd; }
	int getWidth() { return mWidth; };
	int getHeight() { return mHeight; };
	const PRThread* getThread() { return m_thread; };
	const char* getFilename() { return m_swf_file.c_str(); };
  NPBool getShutdown() { return m_shutdown; };
	void notify_mouse_state(int x, int y, int buttons)
	{
		mouse_x = x;
		mouse_y = y;
		if (buttons >= 0)
		{
			mouse_buttons = buttons;
		}
	}

	// Mouse state.
	int	mouse_x;
	int	mouse_y;
	int	mouse_buttons;
 
	private:

	NPP mInstance;
	NPBool mInitialized;
	HWND mhWnd;
	int mX;
	int mY;
	unsigned int mWidth;
	unsigned int mHeight;
	std::string m_swf_file;
	PRThread* m_thread;
	NPBool m_shutdown;

 };
 
 #endif // __PLUGIN_H__
 
