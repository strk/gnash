// plugin.cpp:  Windows "win32" flash player Mozilla plugin, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <cstdlib>

#define FLASH_MAJOR_VERSION "9"
#define FLASH_MINOR_VERSION "0"
#define FLASH_REV_NUMBER "82"

#define MIME_TYPES_HANDLED "application/x-shockwave-flash"
// The name must be this value to get flash movies that check the
// plugin version to load.
#define PLUGIN_NAME "Shockwave Flash"
#define MIME_TYPES_DESCRIPTION MIME_TYPES_HANDLED":swf:"PLUGIN_NAME

// Some javascript plugin detectors use the description
// to decide the flash version to display. They expect the
// form (major version).(minor version) r(revision).
// e.g. "8.0 r99."
#ifndef FLASH_MAJOR_VERSION
#define FLASH_MAJOR_VERSION DEFAULT_FLASH_MAJOR_VERSION
#endif
#ifndef FLASH_MINOR_VERSION
#define FLASH_MINOR_VERSION DEFAULT_FLASH_MINOR_VERSION
#endif
#ifndef FLASH_REV_NUMBER
#define FLASH_REV_NUMBER DEFAULT_FLASH_REV_NUMBER
#endif
#define FLASH_VERSION FLASH_MAJOR_VERSION"."\
    FLASH_MINOR_VERSION" r"FLASH_REV_NUMBER"."

#define PLUGIN_DESCRIPTION \
  "Shockwave Flash "FLASH_VERSION" Gnash "VERSION", the GNU SWF Player. \
  Copyright &copy; 2006, 2007, 2008 \
  <a href=\"http://www.fsf.org\">Free Software Foundation</a>, Inc.<br> \
  Gnash comes with NO WARRANTY, to the extent permitted by law. \
  You may redistribute copies of Gnash under the terms of the \
  <a href=\"http://www.gnu.org/licenses/gpl.html\">GNU General Public \
  License</a>. For more information about Gnash, see <a \
  href=\"http://www.gnu.org/software/gnash/\"> \
  http://www.gnu.org/software/gnash</a>. \
  Compatible Shockwave Flash "FLASH_VERSION

#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <windowsx.h>
#include <wingdi.h>

#include <cstdarg>
#include <boost/cstdint.hpp>
#include <fstream>

#include "plugin.h"

static int module_initialized = FALSE;
static PRLock* playerLock = NULL;
static int instances = 0;

#ifdef NPAPI_CONST
const
#endif
char* NPP_GetMIMEDescription(void);
static void playerThread(void *arg);
static LRESULT CALLBACK PluginWinProc(HWND, UINT, WPARAM, LPARAM);

#define DBG(x, ...) __DBG(x, ## __VA_ARGS__)
inline void
__DBG(const char *fmt, ...)
{
    char buf[1024];
    va_list ap;
    
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
    va_end(ap);
    OutputDebugString(buf);
}

// general initialization and shutdown

NPError
NS_PluginInitialize(void)
{
    DBG("NS_PluginInitialize\n");
    if (!playerLock) {
        playerLock = PR_NewLock();
    }
    module_initialized = TRUE;
    return NPERR_NO_ERROR;
}
 
void
NS_PluginShutdown(void)
{
    DBG("NS_PluginShutdown\n");
    if (!module_initialized) return;
    if (playerLock) {
		PR_DestroyLock(playerLock);
		playerLock = NULL;
    }
}

/// \brief Return the MIME Type description for this plugin.
#ifdef NPAPI_CONST
const
#endif
char*
NPP_GetMIMEDescription(void)
{
    if (!module_initialized) return NULL;
    return MIME_TYPES_HANDLED;
}

#if 0
/// \brief Retrieve values from the plugin for the Browser
///
/// This C++ function is called by the browser to get certain
/// information is needs from the plugin. This information is the
/// plugin name, a description, etc...
///
/// This is actually not used on Win32 (XP_WIN), only on Unix (XP_UNIX).
NPError
NS_PluginGetValue(NPPVariable aVariable, void *aValue)
{
    NPError err = NPERR_NO_ERROR;

    if (!module_initialized) return NPERR_NO_ERROR;
    DBG("aVariable = %d\n", aVariable);
    switch (aVariable) {
        case NPPVpluginNameString:
            *static_cast<char **> (aValue) = PLUGIN_NAME;
            break;

        // This becomes the description field you see below the opening
        // text when you type about:plugins and in
        // navigator.plugins["Shockwave Flash"].description, used in
        // many flash version detection scripts.
        case NPPVpluginDescriptionString:
            *static_cast<const char **>(aValue) = PLUGIN_DESCRIPTION;
            break;

        case NPPVpluginNeedsXEmbed:
#ifdef HAVE_GTK2
            *static_cast<PRBool *>(aValue) = PR_TRUE;
#else
            *static_cast<PRBool *>(aValue) = PR_FALSE;
#endif
            break;

        case NPPVpluginTimerInterval:

        case NPPVpluginKeepLibraryInMemory:

        default:
            err = NPERR_INVALID_PARAM;
            break;
    }
    return err;
}
#endif

// construction and destruction of our plugin instance object

nsPluginInstanceBase*
NS_NewPluginInstance(nsPluginCreateData* aCreateDataStruct)
{
    DBG("NS_NewPluginInstance\n");
    if (!module_initialized) return NULL;
    if (instances > 0) {
        return NULL;
    }
    instances++; // N.B. This is a threading race condition. FIXME.
    if (!playerLock) {
        playerLock = PR_NewLock();
    }
    if (!aCreateDataStruct) {
        return NULL;
    }
    return new nsPluginInstance(aCreateDataStruct);
}
 
void
NS_DestroyPluginInstance(nsPluginInstanceBase* aPlugin)
{
    DBG("NS_DestroyPluginInstance\n");
    if (!module_initialized) return;
    if (aPlugin) {
        delete (nsPluginInstance *) aPlugin;
    }
    if (playerLock) {
		PR_DestroyLock(playerLock);
		playerLock = NULL;
    }
    instances--;
}
 
// nsPluginInstance class implementation

/// \brief Constructor
nsPluginInstance::nsPluginInstance(nsPluginCreateData* data) :
    nsPluginInstanceBase(),
    _instance(data->instance),
    _window(NULL),
    _initialized(FALSE),
    _shutdown(FALSE),
    _stream(NULL),
    _url(""),
    _thread(NULL),
    _width(0),
    _height(0),
    _rowstride(0),
    _hMemDC(NULL),
    _bmp(NULL),
    _memaddr(NULL),
    mouse_x(0),
    mouse_y(0),
    mouse_buttons(0),
    _oldWndProc(NULL)
{
    DBG("nsPluginInstance::nsPluginInstance\n");
}
 
/// \brief Destructor
nsPluginInstance::~nsPluginInstance()
{
    DBG("nsPluginInstance::~nsPluginInstance\n");
    if (_memaddr) {
        // Deleting _bmp should free this memory.
        _memaddr = NULL;
    }
    if (_hMemDC) {
        DeleteObject(_hMemDC);
        _hMemDC = NULL;
    }
    if (_bmp) {
        DeleteObject(_bmp);
        _bmp = NULL;
    }
}
 
NPBool
nsPluginInstance::init(NPWindow* aWindow)
{
    DBG("nsPluginInstance::init\n");

    if (!aWindow) {
        DBG("aWindow == NULL\n");
        return FALSE;
    }

    _x = aWindow->x;
    _y = aWindow->y;
    _width = aWindow->width;
    _height = aWindow->height; 
    _window = (HWND) aWindow->window;
    // Windows DIB row stride is always a multiple of 4 bytes.
    _rowstride = /* 24 bits */ 3 * _width;
    _rowstride += _rowstride % 4;

    memset(&_bmpInfo, 0, sizeof(BITMAPINFOHEADER));
    _bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    _bmpInfo.bmiHeader.biWidth = _width; 
    // Negative height means first row comes first in memory.
    _bmpInfo.bmiHeader.biHeight = -1 * _height; 
    _bmpInfo.bmiHeader.biPlanes = 1; 
    _bmpInfo.bmiHeader.biBitCount = 24; 
    _bmpInfo.bmiHeader.biCompression = BI_RGB; 
    _bmpInfo.bmiHeader.biSizeImage = 0; 
    _bmpInfo.bmiHeader.biXPelsPerMeter = 0; 
    _bmpInfo.bmiHeader.biYPelsPerMeter = 0; 
    _bmpInfo.bmiHeader.biClrUsed = 0; 
    _bmpInfo.bmiHeader.biClrImportant = 0; 

    HDC hDC = GetDC(_window);
    _hMemDC = CreateCompatibleDC(hDC);
    _bmp = CreateDIBSection(_hMemDC, &_bmpInfo,
            DIB_RGB_COLORS, (void **) &_memaddr, 0, 0);
    SelectObject(_hMemDC, _bmp);

    DBG("aWindow->type: %s (%u)\n", 
            (aWindow->type == NPWindowTypeWindow) ? "NPWindowTypeWindow" :
            (aWindow->type == NPWindowTypeDrawable) ? "NPWindowTypeDrawable" :
            "unknown",
            aWindow->type);

    // subclass window so we can intercept window messages and
    // do our drawing to it
    _oldWndProc = SubclassWindow(_window, (WNDPROC) PluginWinProc);
 
    // associate window with our nsPluginInstance object so we can access 
    // it in the window procedure
    SetWindowLong(_window, GWL_USERDATA, (LONG) this);

    _initialized = TRUE;
    return TRUE;
}

void
nsPluginInstance::shut(void)
{
    DBG("nsPluginInstance::shut\n");

    DBG("Acquiring playerLock mutex for shutdown.\n");
    PR_Lock(playerLock);
    _shutdown = TRUE;
    DBG("Releasing playerLock mutex for shutdown.\n");
    PR_Unlock(playerLock);

    if (_thread) {
        DBG("Waiting for thread to terminate.\n");
        PR_JoinThread(_thread);
        _thread = NULL; 
    }

    // subclass it back
    SubclassWindow(_window, _oldWndProc);

    _initialized = FALSE;
}

NPError
nsPluginInstance::NewStream(NPMIMEType type, NPStream *stream,
        NPBool seekable, boost::uint16_t *stype)
{
    DBG("nsPluginInstance::NewStream\n");
    DBG("stream->url: %s\n", stream->url);

    if (!_stream) {
        _stream = stream;
        _url = stream->url;
#if 0
        if (seekable) {
            *stype = NP_SEEK;
        }
#endif
    }

    return NPERR_NO_ERROR;
}

NPError
nsPluginInstance::DestroyStream(NPStream *stream, NPError reason)
{
    DBG("nsPluginInstance::DestroyStream\n");
    DBG("stream->url: %s\n", stream->url);

    // N.B. We can only support one Gnash VM/thread right now. 
    if (!_thread) {
        _thread = PR_CreateThread(PR_USER_THREAD, playerThread, this,
                PR_PRIORITY_NORMAL, PR_GLOBAL_THREAD, PR_JOINABLE_THREAD, 0);
    }

    return NPERR_NO_ERROR;
}

int32
nsPluginInstance::Write(NPStream *stream, int32 offset, int32 len,
        void *buffer)
{
    DBG("nsPluginInstance::Write\n");
    DBG("stream->url: %s, offset: %ld, len: %ld\n",
            stream->url, offset, len);
}

static void
playerThread(void *arg)
{
    nsPluginInstance *plugin = (nsPluginInstance *) arg;

    plugin->threadMain();
}

void
nsPluginInstance::threadMain(void)
{
    DBG("nsPluginInstance::threadMain started\n");
    DBG("URL: %s\n", _url.c_str());

	PR_Lock(playerLock);

    // Initialize Gnash core library.
    DBG("Gnash core initialized.\n");
 
    // Init logfile.
    gnash::RcInitFile& rcinit = gnash::RcInitFile::getDefaultInstance();
    std::string logfilename = std::string(std::getenv("TEMP")) +
        std::string("\\npgnash.log");
    rcinit.setDebugLog(logfilename);
    gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
    dbglogfile.setWriteDisk(true);
    dbglogfile.setVerbosity(GNASH_DEBUG_LEVEL);
    DBG("Gnash logging initialized: %s\n", logfilename.c_str());

    // Init sound.
    _sound_handler.reset(gnash::sound::create_sound_handler_sdl());
    gnash::set_sound_handler(_sound_handler.get());
    DBG("Gnash sound initialized.\n");

    // Init GUI.
    int old_mouse_x = 0, old_mouse_y = 0, old_mouse_buttons = 0;
    _render_handler =
        (gnash::render_handler *) gnash::create_render_handler_agg("BGR24");
    // _memaddr = (unsigned char *) malloc(getMemSize());
    static_cast<gnash::render_handler_agg_base *>(_render_handler)->init_buffer(
            getMemAddr(), getMemSize(), _width, _height, _rowstride);
    gnash::set_render_handler(_render_handler);
    DBG("Gnash GUI initialized: %ux%u\n", _width, _height);

    gnash::URL url(_url);

    VariableMap vars;
    gnash::URL::parse_querystring(url.querystring(), vars);
    for (VariableMap::iterator i = vars.begin(), ie = vars.end(); i != ie; i++) {
        _flashVars[i->first] = i->second;
    }

    gnash::set_base_url(url);

    gnash::movie_definition* md = NULL;
    try {
        md = gnash::createMovie(url, _url.c_str(), false);
    } catch (const gnash::GnashException& err) {
        md = NULL;
    }
    if (!md) {
        /*
         * N.B. Can't use the goto here, as C++ complains about "jump to
         * label 'done' from here crosses initialization of ..." a bunch
         * of things.  Sigh.  So, instead, I duplicate the cleanup code
         * here.  TODO: Remove this duplication.
         */
        // goto done;

        PR_Unlock(playerLock);

        DBG("Clean up Gnash.\n");
        gnash::clear();

        DBG("nsPluginInstance::threadMain exiting\n");
        return;
    }
    DBG("Movie created: %s\n", _url.c_str());

    int movie_width = static_cast<int>(md->get_width_pixels());
    int movie_height = static_cast<int>(md->get_height_pixels());
    float movie_fps = md->get_frame_rate();
    DBG("Movie dimensions: %ux%u (%.2f fps)\n",
            movie_width, movie_height, movie_fps);

    gnash::SystemClock clock; // use system clock here...
    gnash::movie_root& root = gnash::VM::init(*md, clock).getRoot();
    DBG("Gnash VM initialized.\n");
    
    // Register this plugin as listener for FsCommands from the core
    // (movie_root)
#if 0
    /* Commenting out for now as registerFSCommandCallback() has changed. */
    root.registerFSCommandCallback(FSCommand_callback);
#endif
    
    // Register a static function to handle ActionScript events such
    // as Mouse.hide, Stage.align etc.
    // root.registerEventCallback(&staticEventHandlingFunction);

    md->completeLoad();
    DBG("Movie loaded.\n");

    std::auto_ptr<gnash::Movie> mr(md->createMovie());
    mr->setVariables(_flashVars);
    root.setRootMovie(mr.release());
    root.set_display_viewport(0, 0, _width, _height);
    root.set_background_alpha(1.0f);
    gnash::Movie* mi = root.getRootMovie();
    DBG("Movie instance created.\n");

    ShowWindow(_window, SW_SHOW);

    for (;;) {
        // DBG("Inside main thread loop.\n");

        if (_shutdown) {
            DBG("Main thread shutting down.\n");
            break;
        }

        size_t cur_frame = mi->get_current_frame();
        // DBG("Got current frame number: %d.\n", cur_frame);
        size_t tot_frames = mi->get_frame_count();
        // DBG("Got total frame count: %d.\n", tot_frames);

        // DBG("Advancing one frame.\n");
        root.advance();
        // DBG("Going to next frame.\n");
        root.goto_frame(cur_frame + 1);
        // DBG("Ensuring frame is loaded.\n");
        root.get_movie_definition()->ensure_frame_loaded(tot_frames);
        // DBG("Setting play state to PLAY.\n");
        root.set_play_state(gnash::MovieClip::PLAY);

        if (old_mouse_x != mouse_x || old_mouse_y != mouse_y) {
            old_mouse_x = mouse_x;
            old_mouse_y = mouse_y;
            root.notify_mouse_moved(mouse_x, mouse_y);
        }
        if (old_mouse_buttons != mouse_buttons) {
            old_mouse_buttons = mouse_buttons;
            int mask = 1;
            root.notify_mouse_clicked(mouse_buttons > 0, mask);
        }

        root.display();

        RECT rt;
        GetClientRect(_window, &rt);
        InvalidateRect(_window, &rt, FALSE);

#if 0
        InvalidatedRanges ranges;
        ranges.setSnapFactor(1.3f);
        ranges.setSingleMode(false);
        root.add_invalidated_bounds(ranges, false);
        ranges.growBy(40.0f);
        ranges.combine_ranges();

        if (!ranges.isNull()) {
            InvalidateRect(_window, &rt, FALSE);
        }

        root.display();
#endif

        // DBG("Unlocking playerLock mutex.\n");
        PR_Unlock(playerLock);
        // DBG("Sleeping.\n");
        PR_Sleep(PR_INTERVAL_MIN);
        // DBG("Acquiring playerLock mutex.\n");
        PR_Lock(playerLock);
    }

done:
	PR_Unlock(playerLock);

    DBG("Clean up Gnash.\n");

    /*
     * N.B.  As per server/impl.cpp:clear(), all of Gnash's threads aren't
     * guaranteed to be terminated by this, yet.  Therefore, when Firefox
     * unloads npgnash.dll after calling NS_PluginShutdown(), and there are
     * still Gnash threads running, they will try and access memory that was
     * freed as part of the unloading of npgnash.dll, resulting in a process
     * abend.
     */

    gnash::clear();

    DBG("nsPluginInstance::threadMain exiting\n");
}

const char*
nsPluginInstance::getVersion()
{
    return NPN_UserAgent(_instance);
}

void
nsPluginInstance::FSCommand_callback(gnash::MovieClip* movie, const std::string& command, const std::string& args)
// For handling notification callbacks from ActionScript.
{
    gnash::log_debug(_("FSCommand_callback(%p): %s %s"), (void*) movie, command, args);
}

static LRESULT CALLBACK
PluginWinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // get our plugin instance object
    nsPluginInstance *plugin =
        (nsPluginInstance *) GetWindowLong(hWnd, GWL_USERDATA);

    if (plugin) {
        switch (msg) {
            case WM_PAINT:
            {
                if (plugin->getMemAddr() == NULL) {
                    break;
                }

                PAINTSTRUCT ps;
                HDC hDC = BeginPaint(hWnd, &ps);

                RECT rt;
                GetClientRect(hWnd, &rt);
                int w = rt.right - rt.left;
                int h = rt.bottom - rt.top;

                BitBlt(hDC, rt.left, rt.top, w, h,
                        plugin->getMemDC(), 0, 0, SRCCOPY);

                EndPaint(hWnd, &ps);
                return 0L;
            }
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
                int buttons = (msg == WM_LBUTTONDOWN) ? 1 : 0;

                plugin->notify_mouse_state(x, y, buttons);
                break;
            }
            default:
//                dbglogfile << "msg " << msg << endl;
                break;
        }
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}
