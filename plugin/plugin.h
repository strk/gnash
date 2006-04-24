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

#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* Xlib/Xt stuff */
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/cursorfont.h>
#include <GL/glx.h>
#ifdef HAVE_GTK2
#include <gtk/gtk.h>
#endif
#include <string>
#include <map>
#include "pluginbase.h"
#include "log.h"
#include "prlock.h"
#include "prcvar.h"
#include "prthread.h"

/* ascii codes for various special keys */
#define ESCAPE 27
#define PAGE_UP 73
#define PAGE_DOWN 81
#define UP_ARROW 72
#define DOWN_ARROW 80
#define LEFT_ARROW 75
#define RIGHT_ARROW 77

extern NPBool      plugInitialized;
extern Display     *gxDisplay;
extern PRLock      *glMutex;
extern PRLock      *playerMutex;
extern PRCondVar   *playerCond;

class nsPluginInstance : public nsPluginInstanceBase
{
public:
    nsPluginInstance(NPP aInstance);
    virtual ~nsPluginInstance();

    NPBool init(NPWindow *aWindow);
    NPBool isInitialized() {return plugInitialized;}
    NPError GetValue(NPPVariable variable, void *value);
    NPError SetWindow(NPWindow *aWindow);
    NPError NewStream(NPMIMEType type, NPStream *stream, NPBool seekable,
                      uint16 *stype);
    NPError DestroyStream(NPStream * stream, NPError reason);
    void URLNotify(const char *url, NPReason reason, void *notifyData);
    int32 WriteReady(NPStream *stream);
    int32 Write(NPStream *stream, int32 offset, int32 len, void *buffer);
    NPError WriteStatus(char *msg) const;
    void shut();
#ifdef HAVE_GTK2
    GtkWidget *getWidget() { return _gtkwidget; };
    void setWidget(GtkWidget *win) { _gtkwidget = win; };
#endif
    
    int startProc(std::string filespec);
    int startProc(std::string filespec, Window win);
    // accessors
    const char  *getVersion();
    Window      getWindow()     { return _window; };
    Display     *getDisplay()   { return gxDisplay; };
    unsigned int getDepth()     { return mDepth; };
    int         getWidth()      { return mWidth; };
    int         getHeight()     { return mHeight; };
    const char *getFilename()   { return _swf_file.c_str(); };
    PRUintn    getThreadKey()   { return _thread_key; };
    NPBool     getShutdown()    { return _shutdown; };

    // Set the current GL context
    inline void setGL() {
        gnash::log_trace("%s: gxDisplay = %p, _window = %p, _glxContext = %p for instance %p",
                         __PRETTY_FUNCTION__, gxDisplay, (void *)_window,
                         (void *)_glxContext, this);
        if (gxDisplay && _glxContext && _window) {
            glXMakeCurrent(gxDisplay, _window, _glxContext);
            XSync(gxDisplay, False);
        }
    }
    inline void unsetGL() {
        gnash::log_trace("%s: for instance %p", __PRETTY_FUNCTION__, this);
        if (gxDisplay) {
            glXMakeCurrent(gxDisplay, None, NULL);
        }
    }
    // Protect the GL state from multiple threads
    inline void lockGL() {
        gnash::log_trace("%s: for instance %p", __PRETTY_FUNCTION__, this);
        if (glMutex) {
            PR_Lock(glMutex);
        } else {
            gnash::log_error("%s, bad mutex pointer in instance %p!",
                             __PRETTY_FUNCTION__, this);
        }
    }
    inline void freeGL() {
        gnash::log_trace("%s: for instance %p", __PRETTY_FUNCTION__, this);
        if (glMutex) {
            PR_Unlock(glMutex);
        } else {
            gnash::log_error("%s, bad mutex pointer in instance %p!",
                             __PRETTY_FUNCTION__, this);
        }
    }

    // Protect the X context
    inline void lockX() {
        gnash::log_trace("%s: for instance %p", __PRETTY_FUNCTION__, this);
        if (gxDisplay) {
            XLockDisplay(gxDisplay);
        }
    }
    inline void freeX() {
        gnash::log_trace("%s: for instance %p", __PRETTY_FUNCTION__, this);
        if (gxDisplay) {
            XUnlockDisplay(gxDisplay);
        }
    }
    
    void swapBuffers() {
        gnash::log_trace("%s: for instance %p", __PRETTY_FUNCTION__, this);
        if (gxDisplay && _window) {
//             glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//             glFlush();
            glXSwapBuffers(gxDisplay, _window);
        }
    }
    void lockDisplay() {
        gnash::log_trace("%s: for instance %p", __PRETTY_FUNCTION__, this);
        lockGL();
        lockX();
        setGL();
    }
    
    void freeDisplay() {
        gnash::log_trace("%s: for instance %p", __PRETTY_FUNCTION__, this);
        unsetGL();
        freeX();
        freeGL();
    }    
    void destroyContext();
    int resizeWindow(int width,int height);
    void condWait() {
        gnash::log_trace("%s: for instance %p", __PRETTY_FUNCTION__, this);
        PR_WaitCondVar(playerCond, PR_INTERVAL_NO_TIMEOUT);
//        PR_WaitCondVar(_playerCond, PR_INTERVAL_NO_WAIT);
    }

    
    void drawTestScene();
    void initGL();

private:
    // This is a data is unique for each thread
    NPP                 mInstance;
    Window              _window;
    Widget              mXtwidget;
    XFontStruct         *mFontInfo;
    std::string         _swf_file;
    int                 mX;
    int                 mY;
    unsigned int        mWidth;
    unsigned int        mHeight;
    Visual              *mVisual;
    Colormap            mColormap;
    unsigned int        mDepth;
    std::map<std::string, std::string> _options;
    GLXContext          _glxContext;
    int                 _streamfd;
    NPBool              _shutdown;
    NPBool              _glInitialized;
    PRThread            *_thread;
    PRUintn             _thread_key;
    std::string         _procname;
    pid_t               _childpid;
#ifdef HAVE_GTK2
//    NPBool              _newwin;
    GtkWidget           *_gtkwidget;
    unsigned long       _delete_signal_id;
#endif
};

// end of __PLUGIN_H__
#endif

// Local Variables:
// mode: C++
// End:
