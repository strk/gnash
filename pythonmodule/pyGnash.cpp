// pyGnash.cpp: The python module interface to Gnash
//
//   Copyright (C) 2008, 2009 Free Software Foundation, Inc.
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

class PyObjectFromPyFile
{
public:
    static PyObject& execute(PyObject& o) {
        return o;
    }
};

using namespace boost::python;

namespace gnash {

BOOST_PYTHON_MODULE(gnash) {

    lvalue_from_pytype<PyObjectFromPyFile, &PyFile_Type> ();

    class_<pythonwrapper::GnashPlayer>("Player", "Wrapper round Gnash player functions.")
 
        .def("loadMovie", &pythonwrapper::GnashPlayer::loadMovie, 
                    "Load the file object passed")
        .def("initVM", &pythonwrapper::GnashPlayer::initVM)

        .def("setRenderer", &pythonwrapper::GnashPlayer::setRenderer,
        			"Sets the renderer to use. Pass a string naming the "
        			"desired renderer. Valid renderers are: "
        			"AGG_RGB555, AGG_RGB565, AGG_RGBA16, AGG_RGB24, AGG_BGR24, "
        			"AGG_RGBA32, AGG_BGRA32, AGG_ARGB32, AGG_ABGR32, OpenGL, "
        			"Cairo.")

        .def("currentFrame", &pythonwrapper::GnashPlayer::getCurrentFrame,
        			"Get the frame of the movie that the player has reached.")

        .def("advanceClock", &pythonwrapper::GnashPlayer::advanceClock)
        .def("advance", &pythonwrapper::GnashPlayer::advance)
        .def("restart", &pythonwrapper::GnashPlayer::restart,
        			"Restart the movie.")
        .def("pressKey", &pythonwrapper::GnashPlayer::pressKey,
        			"Send a key press event to the player.")
        .def("setVerbosity", &pythonwrapper::GnashPlayer::setVerbosity)
        .def("render", &pythonwrapper::GnashPlayer::render,
        			"Instruct the renderer to draw the current frame. "
        			"Pass 'True' to enforce a full redraw, 'False' to redraw "
        			"only invalidated bounds.")
        .def("movePointer", &pythonwrapper::GnashPlayer::movePointer,
        			"Move pointer to specified coordinates. Returns true "
        			"if the move triggered an action requiring a redraw.")
        .def("mouseClick", &pythonwrapper::GnashPlayer::mouseClick)
            
        .def("swfFrameRate", &pythonwrapper::GnashPlayer::getSWFFrameRate)
        .def("swfFrameCount", &pythonwrapper::GnashPlayer::getSWFFrameCount)
        .def("swfVersion", &pythonwrapper::GnashPlayer::getSWFVersion)
        .def("swfWidth", &pythonwrapper::GnashPlayer::getSWFWidth)
        .def("swfHeight", &pythonwrapper::GnashPlayer::getSWFHeight)
        .def("swfURL", &pythonwrapper::GnashPlayer::getSWFURL)
        .def("swfBytesTotal", &pythonwrapper::GnashPlayer::getSWFBytesTotal,
        			"Length of the loaded movie in bytes, as reported in "
        			"the headers."
	    )
        .def("swfBytesLoaded", &pythonwrapper::GnashPlayer::getSWFBytesLoaded,
        			"The number of bytes of the movie that have been loaded.")

        .def("getTopmostMouseEntity", &pythonwrapper::GnashPlayer::getTopmostMouseEntity,
        		return_value_policy<manage_new_object>(),
        			"The active gnash.Character() under the pointer.")
        			
        // Log messages
        .def("logSize", &pythonwrapper::GnashPlayer::logSize,
        			"The number of messages in the log")
        .staticmethod("logSize")
        .def("getLogMessage", &pythonwrapper::GnashPlayer::getLogMessage,
        			"Retrieve the earliest message in the log")  
        .staticmethod("getLogMessage")
        ;

    class_<pythonwrapper::GnashCharacter>("Character", "Wrapper round a Gnash character.")
        .def("name", &pythonwrapper::GnashCharacter::name)
        .def("target", &pythonwrapper::GnashCharacter::target)
        .def("ratio", &pythonwrapper::GnashCharacter::ratio)

        .def("id", &pythonwrapper::GnashCharacter::id)
        .def("depth", &pythonwrapper::GnashCharacter::depth)
        .def("textName", &pythonwrapper::GnashCharacter::textName)
        .def("clipDepth", &pythonwrapper::GnashCharacter::clipDepth)
        .def("height", &pythonwrapper::GnashCharacter::height)
        .def("width", &pythonwrapper::GnashCharacter::height)    
        .def("visible", &pythonwrapper::GnashCharacter::visible)
        
        .def("advance", &pythonwrapper::GnashCharacter::visible)
        
        .def("getParent", &pythonwrapper::GnashCharacter::getParent,
        		return_value_policy<manage_new_object>(),
        			"The parent gnash.Character() of the present character, "
        			"NULL if there is no parent.")
        ;

}

} // end namespace gnash
