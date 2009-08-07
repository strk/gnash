// VideoInput.h: Video input base class.
// 
//   Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifndef GNASH_VIDEOINPUT_H
#define GNASH_VIDEOINPUT_H

#include <boost/cstdint.hpp> // for C99 int types
#include <string>
#include <vector>
#include <ostream>

namespace gnash {
namespace media {


class VideoInput {
    
public:

    VideoInput();

    // virtual classes need a virtual destructor !
    virtual ~VideoInput() {}
       
    //setters and getters
    void set_activityLevel(double a) {_activityLevel = a;};
    double get_activityLevel () {return _activityLevel;};
    
    void set_bandwidth(int b) {_bandwidth = b;};
    int get_bandwidth() {return _bandwidth;};
    
    void set_currentFPS(double f) {_currentFPS=f;};
    double get_currentFPS() {return _currentFPS;};
    
    void set_fps(double f) {_fps = f;};
    double get_fps() {return _fps;};
    
    void set_height(int h) {_height = h;};
    int get_height() {return _height;};
    
    void set_index(int i) {_index = i;};
    // livedocs say that this should return a number, but when testing with
    // flashplayer it appears to actually return a string.
    std::string get_index() {
        char buffer[2];
        sprintf(buffer, "%i", _index);
        return buffer;
    };
    
    void set_keyFrameInterval(int i) {_keyFrameInterval = i;};
    int get_keyFrameInterval() {return _keyFrameInterval;};
    
    void set_loopback(bool l) {_loopback = l;};
    bool get_loopback() {return _loopback;};
    
    void set_motionLevel(int m) {_motionLevel = m;};
    int get_motionLevel() {return _motionLevel;};
    
    void set_motionTimeout(int m) {_motionTimeout = m;};
    int get_motionTimeout() {return _motionTimeout;};
    
    void set_muted(bool m) {_muted = m;};
    bool get_muted() {return _muted;};
    
    void set_name(std::string name) {_name = name;};
    std::string get_name() {return _name;};

    std::vector<std::string> get_names() {return _names;};

    void set_quality(int q) {_quality = q;};
    int get_quality() {return _quality;};
    
    void set_width(int w) {_width = w;};
    int get_width() {return _width;};

protected:
    //specified in AS livedocs
    double _activityLevel;
    int _bandwidth;
    double _currentFPS;
    double _fps;
    int _height;
    int _index;
    int _keyFrameInterval;
    bool _loopback;
    int _motionLevel;
    int _motionTimeout;
    bool _muted;
    std::string _name;
    std::vector<std::string> _names;
    int _quality;
    int _width;

};

    
} // gnash.media namespace 
} // gnash namespace

#endif // __VIDEOINPUT_H__
