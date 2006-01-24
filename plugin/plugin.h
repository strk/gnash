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

/* Xlib/Xt stuff */
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/cursorfont.h>
#include <string>
#include "pluginbase.h"

/* ascii codes for various special keys */
#define ESCAPE 27
#define PAGE_UP 73
#define PAGE_DOWN 81
#define UP_ARROW 72
#define DOWN_ARROW 80
#define LEFT_ARROW 75
#define RIGHT_ARROW 77

struct WinData{
    Window        _window;
    Display      *_display;
    int           _x;
    int           _y;
    int           _width;
    int           _height;
    Visual       *_visual;
    Colormap      _colormap;
    unsigned int  _depth;
    const char   *_file;
};

class nsPluginInstance : public nsPluginInstanceBase
{
public:
    nsPluginInstance(NPP aInstance);
    virtual ~nsPluginInstance();

    NPBool init(NPWindow *aWindow);
    void shut();
    NPBool isInitialized() {return mInitialized;}
    NPError GetValue(NPPVariable variable, void *value);
    NPError SetWindow(NPWindow *aWindow);
    NPError NewStream(NPMIMEType type, NPStream *stream, NPBool seekable,
                      uint16 *stype);
    NPError DestroyStream(NPStream * stream, NPError reason);
    void URLNotify(const char *url, NPReason reason, void *notifyData);
    int32 WriteReady(NPStream *stream);
    int32 Write(NPStream *stream, int32 offset, int32 len, void *buffer);
    NPError WriteStatus(char *msg) const;

    void draw();
    
    // accessors
    const char  *getVersion();
    WinData     *getWinData()   { return &windata; };
    Window      getWindow()     { return mWindow; };
    Display     *getDisplay()   { return mDisplay; };
    unsigned int getDepth()     { return mDepth; };
    int         getWidth()      { return mWidth; };
    int         getHeight()     { return mHeight; };
    
private:
    NPP           mInstance;
    NPBool        mInitialized;
    Widget        mXtwidget;
    XFontStruct   *mFontInfo;
    std::string      swf_file;
    struct WinData   windata;
    Window        mWindow;
    Display       *mDisplay;
    int           mX;
    int           mY;
    int           mWidth;
    int           mHeight;
    Visual        *mVisual;
    Colormap      mColormap;
    unsigned int  mDepth;
    GC            mGC;
    int           thr_count;
};

// end of __PLUGIN_H__
#endif

// Local Variables:
// mode: C++
// End:
