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

#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#define TEST_GRAPHIC

/* Xlib/Xt stuff */
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/cursorfont.h>
#include <GL/glx.h>
#include <string>
#include <map>
#include "pluginbase.h"
#include <SDL.h>
#include <SDL_thread.h>

/* ascii codes for various special keys */
#define ESCAPE 27
#define PAGE_UP 73
#define PAGE_DOWN 81
#define UP_ARROW 72
#define DOWN_ARROW 80
#define LEFT_ARROW 75
#define RIGHT_ARROW 77

extern Display     *gxDisplay;
extern SDL_mutex   *glMutex;

class nsPluginInstance : public nsPluginInstanceBase
{
public:
    nsPluginInstance(NPP aInstance);
    virtual ~nsPluginInstance();

    NPBool init(NPWindow *aWindow);
    NPBool isInitialized() {return _plugInitialized;}
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

    // accessors
    const char  *getVersion();
    Window      getWindow()     { return mWindow; };
//    Display     *getDisplay()   { return gxDisplay; };
    unsigned int getDepth()     { return mDepth; };
    int         getWidth()      { return mWidth; };
    int         getHeight()     { return mHeight; };
    const char *getFilename()   { return swf_file.c_str(); };

    // Set the current GL context
    void setGL() {
//         printf("%s(%d): Entering\n", __PRETTY_FUNCTION__, __LINE__);
        glXMakeCurrent(gxDisplay, mWindow, mContext);
    }
    // Protect the GL state from multiple threads
    void lockGL() {
//         printf("%s(%d): Entering\n", __PRETTY_FUNCTION__, __LINE__);
        SDL_mutexP(glMutex);
    }
    void freeGL() {
//         printf("%s(%d): Entering\n", __PRETTY_FUNCTION__, __LINE__);
        SDL_mutexV(glMutex);
    }

    // Protect the X context
    void lockX() {
//         printf("%s(%d): Entering\n", __PRETTY_FUNCTION__, __LINE__);
        XLockDisplay(gxDisplay);
    }
    void freeX() {
//         printf("%s(%d): Entering\n", __PRETTY_FUNCTION__, __LINE__);
        XUnlockDisplay(gxDisplay);
    }
    
    void swapBuffers() {
//         printf("%s(%d): Entering\n", __PRETTY_FUNCTION__, __LINE__);
        glXSwapBuffers(gxDisplay, mWindow);
    }
    void drawTestScene();
	bool getShutting() { return bShutting; }

    void initGL();
    void destroyContext();
    int resizeWindow(int width,int height);

private:
    // This is a data is unique for each thread
    NPP                 mInstance;
    Widget              mXtwidget;
    XFontStruct         *mFontInfo;
    std::string         swf_file;
    int                 mX;
    int                 mY;
    unsigned int        mWidth;
    unsigned int        mHeight;
    Visual              *mVisual;
    Colormap            mColormap;
    unsigned int        mDepth;
    bool                bShutting;
    std::map<std::string, std::string> _options;
    SDL_Thread          *mThread;
    GLXContext          mContext;
    Window              mWindow;
    int                 _streamfd;
    NPBool              _glInitialized;
    
    // This data is shared amongst all instantiations of this class
    static NPBool       _plugInitialized;
//    static XtAppContext _xContext;
//    static int          _instantiations;
};

// end of __PLUGIN_H__
#endif

// Local Variables:
// mode: C++
// End:
