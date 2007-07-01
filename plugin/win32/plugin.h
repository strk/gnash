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
//
//

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
	NPError SetWindow(NPP instance, NPWindow* window);
	NPError NewStream(NPMIMEType type, NPStream * stream,
                            NPBool seekable, uint16_t * stype);
	NPError DestroyStream(NPStream * stream, NPError reason);

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
	void DisableOpenGL();
	void EnableOpenGL();
	void main_loop();


	// Mouse state.
	int	mouse_x;
	int	mouse_y;
	int	mouse_buttons;
 
	private:

	NPP mInstance;
	NPBool mInitialized;

	HWND mhWnd;
	HDC mhDC;
	HGLRC mhRC;

	int mX;
	int mY;
	unsigned int mWidth;
	unsigned int mHeight;
	std::string m_swf_file;
	PRThread* m_thread;
	NPBool m_shutdown;

 WNDPROC lpOldProc;

 };
 
 #endif // __PLUGIN_H__
 
