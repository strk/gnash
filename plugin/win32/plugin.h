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
                            NPBool seekable, uint16 * stype);
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
 
