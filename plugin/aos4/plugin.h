// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#ifndef GNASH_AOS4_PLUGIN_H
#define GNASH_AOS4_PLUGIN_H
 
#include <string>
#include "pluginbase.h"

#include "prinit.h"
#include "prlock.h"
#include "prcvar.h"
#include "prerr.h"
#include "prerror.h"
#include "prthread.h"

#include "log.h"
#include "rc.h"
#include "Player.h"
#include "URL.h"
#include "sound_handler.h"
#include "Renderer.h"
#include "Renderer_agg.h"
#include "MovieClip.h"
#include "movie_definition.h"
#include "movie_root.h"
#include "SystemClock.h"
#include "VM.h"

class nsPluginInstance : public nsPluginInstanceBase
{
public:
    nsPluginInstance(nsPluginCreateData*);
    virtual ~nsPluginInstance();

    // We are required to implement these three methods.
    NPBool init(NPWindow* aWindow);
    void shut(void);
    NPBool isInitialized(void) { return _initialized; }

    NPError NewStream(NPMIMEType type, NPStream *stream, NPBool seekable,
            uint16_t *stype);
    NPError DestroyStream(NPStream *stream, NPError reason);
    int32 Write(NPStream *stream, int32 offset, int32 len, void *buffer);

    // locals
    typedef std::map<std::string, std::string> VariableMap;
    const char* getVersion();
    void threadMain(void);

    struct Window *getWindow() { return _window; }
    int getWidth() { return _width; };
    int getHeight() { return _height; };
    int getRowStride() { return _rowstride; }
    APTR getMemDC() { return _hMemDC; } //was HDC
    struct BitMap * getBitmap() { return _bmp; }
    unsigned char* getMemAddr() { return _memaddr; }
    size_t getMemSize() { return _rowstride * _height; }
    void notify_mouse_state(int x, int y, int buttons)
    {
        mouse_x = x;
        mouse_y = y;
        if (buttons >= 0) {
            mouse_buttons = buttons;
        }
    }

private:
    NPP         _instance;
    struct Window *_window;
    NPBool      _initialized;
    NPBool      _shutdown;
    ULONG       _oldWndProc;

    NPStream*   _stream;
    std::string _url;
    VariableMap _flashVars;
    PRThread*   _thread;
    uint32_t    _x;
    uint32_t    _y;
    uint32_t    _width;
    uint32_t    _height;
    uint32_t    _rowstride;
    APTR        _hMemDC; //was HDC
    APTR  		_bmpInfo; //was BITMAPINFO
    struct BitMap *_bmp;
    unsigned char* _memaddr;

    //std::unique_ptr<gnash::media::sound_handler> _sound_handler;
    gnash::Renderer* _Renderer;

    // Mouse state.
    int mouse_x;
    int mouse_y;
    int mouse_buttons;

    static void FSCommand_callback(gnash::MovieClip* movie, const std::string& command, const std::string& args);
};
 
#endif // __PLUGIN_H__
