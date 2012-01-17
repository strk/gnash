// ColorTransform_as.h:  ActionScript "ColorTransform" class, for Gnash.
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
//

#ifndef GNASH_ASOBJ_COLORTRANSFORM_H
#define GNASH_ASOBJ_COLORTRANSFORM_H

#include "Relay.h"

namespace gnash {
    class as_object;
    struct ObjectURI;
}

namespace gnash {

// This is used directly by flash.geom.Transform, as it is
// much more efficient than a pseudo-ActionScript implementation.
class ColorTransform_as : public Relay
{

public:

	ColorTransform_as(double rm, double gm, double bm, double am,
	                  double ro, double go, double bo, double ao);

    // Tests show that the ColorTransform
    // object has its own properties on initialization, so they have
    // getter-setters and are *not* simple properties. Storing and
    // manipulating as doubles (they cannot be anything else - see ctor) is
    // better for speed and memory than using as_value.
    void setAlphaMultiplier(double am) { _alphaMultiplier = am; }
    void setRedMultiplier(double rm) { _redMultiplier = rm; }
    void setBlueMultiplier(double bm) { _blueMultiplier = bm; }
    void setGreenMultiplier(double gm) { _greenMultiplier = gm; }

    void setAlphaOffset(double ao) { _alphaOffset = ao; }
    void setRedOffset(double ro) { _redOffset = ro; }
    void setBlueOffset(double bo) { _blueOffset = bo; }
    void setGreenOffset(double go) { _greenOffset = go; }

    double getAlphaMultiplier() const { return _alphaMultiplier; }
    double getRedMultiplier() const { return _redMultiplier; }
    double getBlueMultiplier() const { return _blueMultiplier; }
    double getGreenMultiplier() const { return _greenMultiplier; }

    double getAlphaOffset() const { return _alphaOffset; }
    double getRedOffset() const { return _redOffset; }
    double getBlueOffset() const { return _blueOffset; }
    double getGreenOffset() const { return _greenOffset; }

    void concat(const ColorTransform_as& other);

private:

    double _alphaMultiplier;
    double _alphaOffset;
    double _blueMultiplier;
    double _blueOffset;
    double _greenMultiplier;
    double _greenOffset;
    double _redMultiplier;
    double _redOffset;

};

/// Initialize the global ColorTransform class
void colortransform_class_init(as_object& where, const ObjectURI& uri);

void registerColorTransformNative(as_object& global);

} // end of gnash namespace

#endif
