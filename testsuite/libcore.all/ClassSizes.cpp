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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "as_value.h"
#include "as_object.h"
#include "Property.h"
#include "PropertyList.h"
#include "MovieClip.h"
#include "Movie.h"
#include "DisplayObject.h"
#include "RGBA.h"
#include "movie_root.h"
#include "swf/ShapeRecord.h"
#include "StaticText.h"
#include "Button.h"
#include "MorphShape.h"
#include "Shape.h"
#include "TextField.h"
#include "SWFStream.h"
#include "FillStyle.h"
#include "swf/DefineFontAlignZonesTag.h"
#include "swf/DefineShapeTag.h"
#include "swf/DefineButtonCxformTag.h"
#include "swf/CSMTextSettingsTag.h"
#include "swf/DefineFontTag.h"
#include "swf/DefineTextTag.h"
#include "swf/PlaceObject2Tag.h"
#include "swf/RemoveObjectTag.h"
#include "swf/DoActionTag.h"
#include "swf/DoInitActionTag.h"
#include "swf/DefineEditTextTag.h"
#include "swf/SetBackgroundColorTag.h"

#include <iostream>
#include <sstream>
#include <cassert>
#include <cmath>
#include <string>
#include <memory>
#include <boost/scoped_ptr.hpp>

#include "check.h"

#include <boost/preprocessor/seq/for_each.hpp>

using namespace gnash;
using namespace std;
using namespace boost;
using namespace gnash::SWF;

#define SIZE(x, _, t) \
    std::cout << BOOST_PP_STRINGIZE(t)": " << (sizeof(t)) << "\n";

// Add types in brackets to this macro to have their size printed.
#define TYPES \
(int) (float) (long) (double) \
(Property*) (auto_ptr<Property>) (scoped_ptr<Property>) \
(boost::shared_ptr<Property>) (intrusive_ptr<as_object>) (GcResource) \
(rgba) (SWFMatrix) (SWFRect) (LineStyle) (FillStyle) (SWFCxForm) \
(as_value) \
(DynamicShape)(ShapeRecord)(TextRecord) \
(Property) (PropertyList) \
(DefinitionTag) (DefineTextTag) (DefineFontTag) (DefineMorphShapeTag) \
(as_object) \
(DisplayObject) (StaticText) (MorphShape) (Shape) \
(InteractiveObject) (MovieClip) (TextField) (Button) (Movie) \
(movie_root) (PropFlags) (ObjectURI)

int
main(int /*argc*/, char** /*argv*/)
{
    std::cout << "Gnash class sizes:\n";
    BOOST_PP_SEQ_FOR_EACH(SIZE, _, TYPES)
}

