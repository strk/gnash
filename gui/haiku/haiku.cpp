// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc.
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


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "RunResources.h"

#include <boost/format.hpp>

#include "adipe.h"
#include "haikusup.h"


#include <interface/Font.h>
#include <Application.h>
#include <Alert.h>
#include <Window.h>
#include <Screen.h>

using namespace std;

#ifndef RENDERER_AGG
#error Haiku gui requires AGG renderer
#endif

namespace gnash
{

const int32 GNASH_PULSE = 'GPLS';
const int32 GNASH_SEND_PULSE = 'GSPL';
const int32 GNASH_QUITTING = 'GQUI';


class BeWin : public BWindow
{
    HaikuGui *_gui;
    BRect _unfullscreen_frame;
    bool _fullscreen;
public:
    BeWin(BRect frame, const char *title, HaikuGui *gui)
    : BWindow(frame, title, B_TITLED_WINDOW, 0), _gui(gui),
        _fullscreen(false)
    {
    }

    bool QuitRequested()
    {
        be_app_messenger.SendMessage(B_QUIT_REQUESTED);
        BWindow::QuitRequested();
        return false;
    }

    void FrameResized(float width, float height)
    {
        BMessage msg(GNASH_RESIZE);
        if (msg.AddFloat("w", width) != B_OK
            || msg.AddFloat("h", height) != B_OK)
            QQ(1);
        else
            be_app_messenger.SendMessage(&msg);
    }

    void setFullscreenEtc()
    {
        QQ(8);
        if (_fullscreen == true)
            return;
        _fullscreen = true;

        BScreen s(this);
        BRect r(s.Frame());
        _unfullscreen_frame = Frame();
        MoveTo(0,0);
        ResizeTo(r.Width(), r.Height());
    }

    void ScreenChanged(BRect frame, color_space mode)
    {
        (void) mode;

        QQ(8);
        if (_fullscreen == false)
            return;
        // XXX perform in beapp thread
        MoveTo(0,0);
        ResizeTo(frame.Width(), frame.Height());
    }

    void unsetFullscreenEtc()
    {
        QQ(8);
        if (_fullscreen == false)
            return;
        _fullscreen = false;
        MoveTo(_unfullscreen_frame.left, _unfullscreen_frame.top);
        ResizeTo(_unfullscreen_frame.Width(), _unfullscreen_frame.Height());
    }


};

class BeApp : public BApplication
{
    HaikuGui *_gui;
    BWindow *_win;
    thread_id _pulse_tid;
    time_t _start_t;
    time_t _timeout;
    int32 _mouse_moves_this_pulse;
    int32 _mousemovedx, _mousemovedy;
public:
    BeApp(HaikuGui *gui)
    : BApplication("application/gnash-player"),
        _gui(gui), _win(NULL),
        _pulse_tid(-1),
        _start_t(time(NULL)),
        _timeout(0),
        _mouse_moves_this_pulse(0),
        _mousemovedx(0), _mousemovedy(0)
    {
        _pulse_tid = spawn_thread(SendPulses, "Pulses", B_NORMAL_PRIORITY, this);
        if (_pulse_tid <= 0) {
            throw runtime_error(_("spawn_thread failed"));
        }
        if (B_OK != resume_thread(_pulse_tid)) {
            throw runtime_error(_("resume_thread failed"));
        }

    }

    static int32
    SendPulses(void *data)
    {
        BeApp *that =
            static_cast<BeApp*>(data);
        assert(that != NULL);
        int32 i = that->SendPulsesEtc();
        if (i != 0) {
            boost::format fmt(_("pulses thread returned %d"));
            fmt = fmt % i;
            log_error(fmt.str());
        }
        return i;
    }
    
    
    int32
    SendPulsesEtc()
    {
        BMessage m(GNASH_PULSE);
        int32 code;
        thread_id sender;
    
        while (true)
        {
            if (B_OK != be_app_messenger.SendMessage(&m))
                return -1;
    
            unsigned int interval =
                _gui->getInterval();
            if (interval == 0)
               interval = 15;
            // interval in miliseconds, 1000 * interval in microseconds
            usleep(1000 * interval);
    
            do {
                code = receive_data(&sender, NULL, 0);
            } while (code == B_INTERRUPTED);
            switch (code)
            {
                case GNASH_SEND_PULSE:
                    break;
                case GNASH_QUITTING:
                    return 0;
                default:
                    return -1;
            };
        }
        return 0;
    }

    void setTimeout(time_t timeout)
    {
        _timeout = timeout;
    }

    virtual ~BeApp()
    {
        status_t st, exit_value;
        do {
            st = send_data(_pulse_tid, GNASH_QUITTING, NULL, 0);
        } while (st == B_INTERRUPTED);
        if (st == B_OK)
            wait_for_thread(_pulse_tid, &exit_value);
    }

    bool QuitRequested()
    {
        BApplication::QuitRequested();
        return true;
    }

    bool Timedout()
    {
        return _timeout && time(NULL) - _start_t > _timeout;
    }

    void setFullscreenEtc()
    {
        assert(_win != NULL);
        static_cast<BeWin*>(_win)->setFullscreenEtc();
    }

    void unsetFullscreenEtc()
    {
        assert(_win != NULL);
        static_cast<BeWin*>(_win)->unsetFullscreenEtc();
    }

    void MessageReceived(BMessage *msg)
    {
        switch (msg->what)
        {
            case GNASH_PULSE:
                if (_mouse_moves_this_pulse > 3)
                    _gui->notifyMouseMove(_mousemovedx, _mousemovedy);
                _mouse_moves_this_pulse = 0;
                _gui->GnashPulse();
                if (B_OK != send_data(_pulse_tid, GNASH_SEND_PULSE, NULL, 0)) {
                    log_error(_("send_data failed"));
                    Quit();
                }
                break;
            case GNASH_RESIZE:
            {
                float width, height;
                if (msg->FindFloat("w", &width) != B_OK
                    || msg->FindFloat("h", &height) != B_OK)
                    QQ(1);
                else
                    _gui->resize_view(width+1, height+1);
                break;
            }
            case GNASH_HIDDEN:
            {
                _gui->Hidden();
                break;
            }
            case GNASH_SHOWN:
            {
                _gui->Shown();
                break;
            }
            case GNASH_SET_FULLSCREEN:
                _gui->setFullscreenEtc();
                break;
            case GNASH_UNSET_FULLSCREEN:
                _gui->unsetFullscreenEtc();
                break;
            case GNASH_MOUSE_CLICKED:
            {
                bool pressed;
                int32 mask;
                if (B_OK != msg->FindBool("pressed", &pressed)
                    || B_OK != msg->FindInt32("mask", &mask))
                    QQ(1);
                else
                    _gui->notifyMouseClick(pressed);
                break;
            }
            case GNASH_MOUSE_MOVED:
            {
                ++ _mouse_moves_this_pulse;
                int32 x, y;
                if (B_OK != msg->FindInt32("x", &x)
                    || B_OK != msg->FindInt32("y", &y))
                    QQ(1);
                else
                {
                    if (_mouse_moves_this_pulse > 3)
                    {
                        _mousemovedx = x;
                        _mousemovedy = y;
                    } else
                        _gui->notifyMouseMove(x, y);
                }
                break;
            }
            case GNASH_KEY_EVENT:
            {
                int32 key, modifiers;
                bool pressed;
                if (B_OK != msg->FindInt32("key", &key)
                    || B_OK != msg->FindInt32("modifiers", &modifiers)
                    || B_OK != msg->FindBool("pressed", &pressed))
                    QQ(1);
                else
                    _gui->notify_key_event(static_cast<gnash::key::code>(key), modifiers, pressed);
                break;
            }
            default:
                BApplication::MessageReceived(msg);
        };
    }

    void
    ReadyToRun()
    {
    }

    bool
    CreateWindow(int width, int height, int xPosition, int yPosition,
                const char *title)
    {
        assert(_win == NULL);
        QQ(8);
        _win = new BeWin(BRect(xPosition, yPosition, xPosition+width-1, yPosition+height-1),
            title, _gui);
        _win->Show();
        return true;
    }

    BWindow**
    GetWinAddr()
    {
        return &_win;
    }
};


HaikuGui::HaikuGui(unsigned long xid, float scale, bool loop, RunResources& r)
    : Gui(xid, scale, loop, r), _app(NULL),
      _glue(this, xid), _timeout(0)
{
    QQ(8);
}

HaikuGui::~HaikuGui()
{
    GNASH_REPORT_FUNCTION;

    delete _app;
}

bool
HaikuGui::init(int argc, char **argv[])
{
    QQ(8);
    GNASH_REPORT_FUNCTION;
    _app = new BeApp(this);

//    if (_xid)
//            log_error (_("Ignoring request to display in X11 window"));

    char c;
    int origopterr = opterr;

    optind = 0;
    opterr = 0;

    while ((c = getopt(argc, *argv, "D:")) != -1) {
        if (c == 'D') {
            // Terminate if no filename is given.
            if (!optarg) {
                std::cout << 
                    _("# FATAL:  No filename given with -D argument.") << std::endl;      
                return false;
            }
            _sharefilename = optarg;
        }
    }


    opterr = origopterr;

    _glue.init(argc, argv, _app->GetWinAddr(), _sharefilename);

    _renderer.reset(_glue.createRenderHandler(32));
    if ( ! _renderer ) return false;

    return true;
}

bool
HaikuGui::createWindow(const char *title, int width, int height,
              int xPosition, int yPosition)
{
    GNASH_REPORT_FUNCTION;
    if (xPosition == -1 && yPosition == -1)
    {
        xPosition = 200;
        yPosition = 200;
    }
    _width = width;
    _height = height;



    if (_xid == 0)
    {
        bool b = _app->CreateWindow(width, height, xPosition, yPosition,
            title);
        if (b == false)
            return false;
    } else {
    }
    _glue.prepDrawingArea(_width, _height, 0/*sdl_flags*/);
    _glue.ViewNeeded();
    _runResources.setRenderer(_renderer);

    return true;
}

bool
HaikuGui::run()
{
    GNASH_REPORT_FUNCTION;
    _app->Run();
    return ! _app->Timedout();
}

void
HaikuGui::renderBuffer()
{
    _glue.render();
}

void
HaikuGui::setInterval(unsigned int interval)
{
    _interval = interval;
}

void
HaikuGui::setTimeout(unsigned int timeout)
{
    _timeout = timeout;
    _app->setTimeout((timeout+999)/1000);
}

void
HaikuGui::error(const std::string &msg)
{
    BAlert *alert =
        new BAlert("Error", msg.c_str(), "Dismiss", NULL, NULL,
                B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_STOP_ALERT);
    alert->Go();
}

bool
HaikuGui::yesno(const std::string& question)
{
    BAlert *alert =
        new BAlert("yes/no", question.c_str(), "Yes", "No", NULL,
                B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_WARNING_ALERT);
    int32 answer = alert->Go();
    return answer == 0;
}

void
HaikuGui::setInvalidatedRegions(const InvalidatedRanges& ranges)
{
    _glue.setInvalidatedRegions(ranges);
}

bool
HaikuGui::createMenu()
{
    GNASH_REPORT_FUNCTION;
    return false;
}

bool
HaikuGui::createMenuBar()
{
    GNASH_REPORT_FUNCTION;
    return false;
}

bool
HaikuGui::setupEvents()
{
    GNASH_REPORT_FUNCTION;
    return false;
}

void
HaikuGui::setFullscreen()
{
    BMessage m(GNASH_SET_FULLSCREEN);
    be_app_messenger.SendMessage(&m);
}

void
HaikuGui::setFullscreenEtc()
{
    //_glue.BlankScreen();
    if (_xid != 0)
    {
        bool b = _app->CreateWindow(_width, _height, 0, 0,
                "Gnash");
        if (b == false)
        {
            return;
        }
        //_glue.prepDrawingArea(_width+5, _height, 0/*sdl_flags*/);
        //_glue.prepDrawingArea(_width, _height, 0/*sdl_flags*/);
        _glue.ViewNeeded();
    }
    _app->setFullscreenEtc();
}

void
HaikuGui::unsetFullscreen()
{
    BMessage m(GNASH_UNSET_FULLSCREEN);
    be_app_messenger.SendMessage(&m);
}

void
HaikuGui::unsetFullscreenEtc()
{
    //_glue.BlankScreen();
    _app->unsetFullscreenEtc();
    if (_xid != 0)
    {
        (*_app->GetWinAddr())->LockLooper();
        (*_app->GetWinAddr())->Quit();
        (*_app->GetWinAddr()) = NULL;
        _glue.ViewNoMore();
    }
    //std::cerr << _width << " " << _height << std::endl;
    //_glue.prepDrawingArea(_width, _height, 0/*sdl_flags*/);
}

void HaikuGui::GnashPulse()
{
    if (_app->Timedout())
    {
        _app->Quit();
        return ;
    }

    Gui::advance_movie(this);
}

unsigned int
HaikuGui::getInterval()
{
    return _interval;
}

void
HaikuGui::resize_view(int width, int height)
{
    std::cerr << width << " " << height << std::endl;
    _glue.prepDrawingArea(width, height, 0);
    Gui::resize_view(width, height);
}

void
HaikuGui::Shown()
{
    _glue.Shown();
}

void
HaikuGui::Hidden()
{
    _glue.Hidden();
}



};

