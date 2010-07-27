// 
//   Copyright (C) 2010 Free Software Foundation, Inc
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

#include <sys/types.h>
#include <fcntl.h>

#include "log.h"
#include "InputDevice.h"

namespace gnash {

static const char *MOUSE_DEVICE = "/dev/usb/tkpanel0";

bool
EventDevice::init(const std::string &filespec, size_t size)
{
    GNASH_REPORT_FUNCTION;
    
#ifdef ENABLE_FAKE_FRAMEBUFFER
    return false;
#endif
    
    _type = TouchDevice::KEYBOARD;
    
    std::string dev;

    char* devname = std::getenv("POINTING_DEVICE");
    if (devname) {
        dev = devname;
    } else {
        dev = "/dev/input/event0";
    }

    // Try to open mouse device, be error tolerant (FD is kept open all the time)
    _fd = open(dev.c_str(), O_RDONLY);
  
    if (_fd < 0) {
        log_debug(_("Could not open %s: %s"), dev.c_str(), strerror(errno));    
        return false;
    }
  
    log_debug(_("Pointing device %s open"), dev.c_str());
  
    if (fcntl(_fd, F_SETFL, fcntl(input_fd, F_GETFL) | O_NONBLOCK) < 0) {
        log_error(_("Could not set non-blocking mode for pointing device: %s"),
                  strerror(errno));
        close(_fd);
        _fd = -1;
        return false; 
    }
  
    return true;
}

bool
EventDevice::check()
{
    GNASH_REPORT_FUNCTION;
#ifdef ENABLE_FAKE_FRAMEBUFFER
    return false;
#endif
    
    bool activity = false;
  
    if (_fd < 0) {
        return false;           // no keyboard
    }

    struct input_event ev;  // time,type,code,value
  
    while (read(_fd, &ev, sizeof ev) == (sizeof ev)) {  
        if (ev.type == EV_KEY) {
    
            // code == scan code of the key (KEY_xxxx defines in input.h)
      
            // value == 0  key has been released
            // value == 1  key has been pressed
            // value == 2  repeated key reporting (while holding the key) 

            if (ev.code==KEY_LEFTSHIFT) {
                keyb_lshift = ev.value;
            } else if (ev.code==KEY_RIGHTSHIFT) {
                keyb_rshift = ev.value;
            } else if (ev.code==KEY_LEFTCTRL) {
                keyb_lctrl = ev.value;
            } else if (ev.code==KEY_RIGHTCTRL) {
                keyb_rctrl = ev.value;
            } else if (ev.code==KEY_LEFTALT) {
                keyb_lalt = ev.value;
            } else if (ev.code==KEY_RIGHTALT) {
                keyb_ralt = ev.value;
            } else {
                gnash::key::code  c = scancode_to_gnash_key(ev.code, 
                                                    keyb_lshift || keyb_rshift);
                // build modifier
                int modifier = gnash::key::GNASH_MOD_NONE;
                
                if (keyb_lshift || keyb_rshift) {
                    modifier = modifier | gnash::key::GNASH_MOD_SHIFT;
                }
                
                if (keyb_lctrl || keyb_rctrl) {
                    modifier = modifier | gnash::key::GNASH_MOD_CONTROL;
                }
                
                if (keyb_lalt || keyb_ralt) {
                    modifier = modifier | gnash::key::GNASH_MOD_ALT;
                }
                
                // send event
                if (c != gnash::key::INVALID) {
                    Gui::notify_key_event(c, modifier, ev.value);
                    activity=true;
                }
                
            } // if normal key

        } // if EV_KEY      
  
    } // while
  
    return activity;
}

gnash::key::code
EventDevice::scancode_to_gnash_key(int code, bool shift)
{ 
    // NOTE: Scancodes are mostly keyboard oriented (ie. Q, W, E, R, T, ...)
    // while Gnash codes are mostly ASCII-oriented (A, B, C, D, ...) so no
    // direct conversion is possible.
    
    // TODO: This is a very *incomplete* list and I also dislike this method
    // very much because it depends on the keyboard layout (ie. pressing "Z"
    // on a german keyboard will print "Y" instead). So we need some 
    // alternative...
    
    switch (code) {
      case KEY_1      : return !shift ? gnash::key::_1 : gnash::key::EXCLAM;
      case KEY_2      : return !shift ? gnash::key::_2 : gnash::key::DOUBLE_QUOTE; 
      case KEY_3      : return !shift ? gnash::key::_3 : gnash::key::HASH; 
      case KEY_4      : return !shift ? gnash::key::_4 : gnash::key::DOLLAR; 
      case KEY_5      : return !shift ? gnash::key::_5 : gnash::key::PERCENT; 
      case KEY_6      : return !shift ? gnash::key::_6 : gnash::key::AMPERSAND; 
      case KEY_7      : return !shift ? gnash::key::_7 : gnash::key::SINGLE_QUOTE; 
      case KEY_8      : return !shift ? gnash::key::_8 : gnash::key::PAREN_LEFT; 
      case KEY_9      : return !shift ? gnash::key::_9 : gnash::key::PAREN_RIGHT; 
      case KEY_0      : return !shift ? gnash::key::_0 : gnash::key::ASTERISK;
                            
      case KEY_A      : return shift ? gnash::key::A : gnash::key::a;
      case KEY_B      : return shift ? gnash::key::B : gnash::key::b;
      case KEY_C      : return shift ? gnash::key::C : gnash::key::c;
      case KEY_D      : return shift ? gnash::key::D : gnash::key::d;
      case KEY_E      : return shift ? gnash::key::E : gnash::key::e;
      case KEY_F      : return shift ? gnash::key::F : gnash::key::f;
      case KEY_G      : return shift ? gnash::key::G : gnash::key::g;
      case KEY_H      : return shift ? gnash::key::H : gnash::key::h;
      case KEY_I      : return shift ? gnash::key::I : gnash::key::i;
      case KEY_J      : return shift ? gnash::key::J : gnash::key::j;
      case KEY_K      : return shift ? gnash::key::K : gnash::key::k;
      case KEY_L      : return shift ? gnash::key::L : gnash::key::l;
      case KEY_M      : return shift ? gnash::key::M : gnash::key::m;
      case KEY_N      : return shift ? gnash::key::N : gnash::key::n;
      case KEY_O      : return shift ? gnash::key::O : gnash::key::o;
      case KEY_P      : return shift ? gnash::key::P : gnash::key::p;
      case KEY_Q      : return shift ? gnash::key::Q : gnash::key::q;
      case KEY_R      : return shift ? gnash::key::R : gnash::key::r;
      case KEY_S      : return shift ? gnash::key::S : gnash::key::s;
      case KEY_T      : return shift ? gnash::key::T : gnash::key::t;
      case KEY_U      : return shift ? gnash::key::U : gnash::key::u;
      case KEY_V      : return shift ? gnash::key::V : gnash::key::v;
      case KEY_W      : return shift ? gnash::key::W : gnash::key::w;
      case KEY_X      : return shift ? gnash::key::X : gnash::key::x;
      case KEY_Y      : return shift ? gnash::key::Y : gnash::key::y;
      case KEY_Z      : return shift ? gnash::key::Z : gnash::key::z;

      case KEY_F1     : return gnash::key::F1; 
      case KEY_F2     : return gnash::key::F2; 
      case KEY_F3     : return gnash::key::F3; 
      case KEY_F4     : return gnash::key::F4; 
      case KEY_F5     : return gnash::key::F5; 
      case KEY_F6     : return gnash::key::F6; 
      case KEY_F7     : return gnash::key::F7; 
      case KEY_F8     : return gnash::key::F8; 
      case KEY_F9     : return gnash::key::F9;
      case KEY_F10    : return gnash::key::F10;
      case KEY_F11    : return gnash::key::F11;
      case KEY_F12    : return gnash::key::F12;
    
      case KEY_KP0    : return gnash::key::KP_0; 
      case KEY_KP1    : return gnash::key::KP_1; 
      case KEY_KP2    : return gnash::key::KP_2; 
      case KEY_KP3    : return gnash::key::KP_3; 
      case KEY_KP4    : return gnash::key::KP_4; 
      case KEY_KP5    : return gnash::key::KP_5; 
      case KEY_KP6    : return gnash::key::KP_6; 
      case KEY_KP7    : return gnash::key::KP_7; 
      case KEY_KP8    : return gnash::key::KP_8; 
      case KEY_KP9    : return gnash::key::KP_9;

      case KEY_KPMINUS       : return gnash::key::KP_SUBTRACT;
      case KEY_KPPLUS        : return gnash::key::KP_ADD;
      case KEY_KPDOT         : return gnash::key::KP_DECIMAL;
      case KEY_KPASTERISK    : return gnash::key::KP_MULTIPLY;
      case KEY_KPENTER       : return gnash::key::KP_ENTER;
    
      case KEY_ESC           : return gnash::key::ESCAPE;
      case KEY_MINUS         : return gnash::key::MINUS;
      case KEY_EQUAL         : return gnash::key::EQUALS;
      case KEY_BACKSPACE     : return gnash::key::BACKSPACE;
      case KEY_TAB           : return gnash::key::TAB;
      case KEY_LEFTBRACE     : return gnash::key::LEFT_BRACE;
      case KEY_RIGHTBRACE    : return gnash::key::RIGHT_BRACE;
      case KEY_ENTER         : return gnash::key::ENTER;
      case KEY_LEFTCTRL      : return gnash::key::CONTROL;
      case KEY_SEMICOLON     : return gnash::key::SEMICOLON;
          //case KEY_APOSTROPHE    : return gnash::key::APOSTROPHE;  
          //case KEY_GRAVE         : return gnash::key::GRAVE;
      case KEY_LEFTSHIFT     : return gnash::key::SHIFT;
      case KEY_BACKSLASH     : return gnash::key::BACKSLASH;
      case KEY_COMMA         : return gnash::key::COMMA;
      case KEY_SLASH         : return gnash::key::SLASH;
      case KEY_RIGHTSHIFT    : return gnash::key::SHIFT;
      case KEY_LEFTALT       : return gnash::key::ALT;
      case KEY_SPACE         : return gnash::key::SPACE;
      case KEY_CAPSLOCK      : return gnash::key::CAPSLOCK;
      case KEY_NUMLOCK       : return gnash::key::NUM_LOCK;
          //case KEY_SCROLLLOCK    : return gnash::key::SCROLLLOCK;
    
      case KEY_UP            : return gnash::key::UP;
      case KEY_DOWN          : return gnash::key::DOWN;
      case KEY_LEFT          : return gnash::key::LEFT;
      case KEY_RIGHT         : return gnash::key::RIGHT;
      case KEY_PAGEUP        : return gnash::key::PGUP;
      case KEY_PAGEDOWN      : return gnash::key::PGDN;
      case KEY_INSERT        : return gnash::key::INSERT;
      case KEY_DELETE        : return gnash::key::DELETEKEY;
      case KEY_HOME          : return gnash::key::HOME;
      case KEY_END           : return gnash::key::END;
    
    }
  
    return gnash::key::INVALID;  
}

// end of namespace
}

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
