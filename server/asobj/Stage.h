// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#ifndef __GNASH_ASOBJ_STAGE_H__
#define __GNASH_ASOBJ_STAGE_H__

#include "as_object.h" // for inheritance

#include <list>

namespace gnash {

/// This is the Stage ActionScript object.
//
/// It is currently not used as it should, in particular
/// it should control resize behaviour, while it's not.
///
class Stage: public as_object
{

public:

	typedef enum {
		showAll,
		noScale,
		exactFill,
		noBorder
	} ScaleMode;
    
    typedef enum {
		normal,
		fullScreen
	} DisplayState;


	Stage();

	// override from as_object ?
	//std::string get_text_value() const { return "Stage"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }
	
	/// Recive a resize event.
	void onResize();

	/// Get current stage width, in pixels
	unsigned getWidth() const;

	/// Get current stage height, in pixels
	unsigned getHeight() const;

	/// Set scale mode 
	void setScaleMode(ScaleMode mode);

	/// \brief
	/// Return the string representation for current
	/// scale mode.
	//
	/// Valid values are:
	///	- showAll
	///	- noBorder
	///	- exactFit
	///	- noScale
	///
	const char* getScaleModeString();

    /// Get present align mode
    const std::string& getAlignMode() const { return _alignMode; }

    /// Set align mode
    void setAlignMode(const std::string& mode) { _alignMode = mode; }

	/// Set display state 
	void setDisplayState(DisplayState state);
	
	const char* getDisplayStateString();	

private:

	/// Notify all listeners about a resize event
	void notifyResize();

	ScaleMode _scaleMode;
	
	std::string _alignMode;
	
	DisplayState _displayState;
};


/// Initialize the global Stage class
void stage_class_init(as_object& global);
  
} // end of gnash namespace

// __GNASH_ASOBJ_STAGE_H__
#endif

