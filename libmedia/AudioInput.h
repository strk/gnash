// AudioInput.h: Audio input base class
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

#ifndef GNASH_AUDIOINPUT_H
#define GNASH_AUDIOINPUT_H

#include <boost/cstdint.hpp> // for C99 int types
#include <string>
#include <vector>

namespace gnash {
namespace media {

/// \class AudioInput
/// This is the base class that talks to Microphone_as.cpp. It is basically
/// exactly what's specified in the livedocs. Most of the real work is done
/// in the AudioInputGst or AudioInputFFMPEG source files, respectively.
class AudioInput {
	
public:

	AudioInput();

	// virtual classes need a virtual destructor !
	virtual ~AudioInput() {}
	
	//setters and getters
	void set_activityLevel(double a) {_activityLevel = a; };
	double get_activityLevel() {return _activityLevel;};
	
	void set_gain(double g) { _gain = g;};
	double get_gain() { return _gain; };
	
	void set_index(int i) {_index = i;};
	int get_index() {return _index;};
	
	void set_muted(bool b) {_muted = b;};
	bool get_muted() {return _muted;};
	
	void set_name(std::string name) {_name = name;};
	std::string get_name() {return _name;};
	
	std::vector<std::string> get_names() {return _names;}
	
	void set_rate(int r) {_rate = r;};
	int get_rate() {return _rate;};
	
	void set_silenceLevel(double s) {_silenceLevel = s; };
	double get_silenceLevel() {return _silenceLevel;};
	
	void set_silenceTimeout(int s) {_silenceTimeout = s;};
	int get_silenceTimeout() {return _silenceTimeout;};
	
	void set_useEchoSuppression(bool e) {_useEchoSuppression = e;};
	bool get_useEchoSuppression() {return _useEchoSuppression;};
	
protected:
	double _activityLevel;
	double _gain;
	int _index;
	bool _muted;
	std::string _name;
	std::vector<std::string> _names;
	int _rate;
	double _silenceLevel;
	int _silenceTimeout;
	bool _useEchoSuppression;
};

	
} // gnash.media namespace 
} // gnash namespace

#endif // __AUDIOINPUT_H__
