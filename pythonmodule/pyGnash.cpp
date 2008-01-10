// pyGnash.cpp: The python module interface to Gnash
//
//   Copyright (C) 2008 Free Software Foundation, Inc.
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
//
#include <boost/python.hpp>
#include "gnashpython.h"

// Warning! These Python bindings are not mature. They are unlikely to 
// stay as they are. Do not rely on this interface!

using namespace boost::python;

namespace gnash {

BOOST_PYTHON_MODULE(gnash) {
  class_<pythonwrapper::GnashPlayer>("Player", "Allows direct control of a Gnash player.")

    .def("setBaseURL", &pythonwrapper::GnashPlayer::setBaseURL)  
    .def("loadMovie", &pythonwrapper::GnashPlayer::createMovieDefinition)
    .def("initVM", &pythonwrapper::GnashPlayer::initVM)

    .def("addRenderer", &pythonwrapper::GnashPlayer::initRenderer)

    .def("currentFrame", &pythonwrapper::GnashPlayer::getCurrentFrame)

    .def("advanceClock", &pythonwrapper::GnashPlayer::advanceClock)
    .def("advance", &pythonwrapper::GnashPlayer::advance)
    .def("restart", &pythonwrapper::GnashPlayer::restart)
    .def("pressKey", &pythonwrapper::GnashPlayer::pressKey)
    .def("allowRescale", &pythonwrapper::GnashPlayer::allowRescale)
    .def("render", &pythonwrapper::GnashPlayer::render)
        
    .def("swfFrameRate", &pythonwrapper::GnashPlayer::getSWFFrameRate)
    .def("swfFrameCount", &pythonwrapper::GnashPlayer::getSWFFrameCount)
    .def("swfVersion", &pythonwrapper::GnashPlayer::getSWFVersion)
    .def("swfWidth", &pythonwrapper::GnashPlayer::getSWFWidth)
    .def("swfHeight", &pythonwrapper::GnashPlayer::getSWFHeight)
    .def("swfURL", &pythonwrapper::GnashPlayer::getSWFURL)
    .def("swfBytesTotal", &pythonwrapper::GnashPlayer::getSWFBytesTotal,
    	"Length of the loaded movie in bytes as reported in the headers."
	)
    .def("swfBytesLoaded", &pythonwrapper::GnashPlayer::getSWFBytesLoaded,
    	"The number of bytes of the movie that have been loaded")

    .def("getCharacterById", &pythonwrapper::GnashPlayer::getCharacterById,
    		return_value_policy<manage_new_object>())
    .def("getTopmostMouseEntity", &pythonwrapper::GnashPlayer::getTopmostMouseEntity,
    		return_value_policy<manage_new_object>())
    ;

  class_<pythonwrapper::GnashCharacter>("Character", "A character from the movie."
  							"This class doesn't work")
    ;

}

} // end namespace gnash
