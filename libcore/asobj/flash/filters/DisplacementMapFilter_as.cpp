// DisplacementMapFilter_as.cpp:  ActionScript "DisplacementMapFilter" class, for Gnash.
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

#include <boost/intrusive_ptr.hpp>

#include "DisplacementMapFilter_as.h"

#include <sstream>

#include "as_object.h"
#include "as_function.h" 
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "BitmapFilter_as.h"
#include "Filters.h"

namespace gnash {

namespace {

    as_value displacementmapfilter_clone(const fn_call& fn);
    as_value displacementmapfilter_alpha(const fn_call& fn);
    as_value displacementmapfilter_color(const fn_call& fn);
    as_value displacementmapfilter_componentX(const fn_call& fn);
    as_value displacementmapfilter_componentY(const fn_call& fn);
    as_value displacementmapfilter_mapBitmap(const fn_call& fn);
    as_value displacementmapfilter_mapPoint(const fn_call& fn);
    as_value displacementmapfilter_mode(const fn_call& fn);
    as_value displacementmapfilter_scaleX(const fn_call& fn);
    as_value displacementmapfilter_scaleY(const fn_call& fn);
    as_value displacementmapfilter_ctor(const fn_call& fn);
    void attachDisplacementMapFilterInterface(as_object& o);

}

/// TODO: this should probably look something like the other filter classes.
class DisplacementMapFilter_as : public Relay
{
public:
	DisplacementMapFilter_as() {}
};

// This is an AS3 class, and there are no tests for it.
void
displacementmapfilter_class_init(as_object& where, const ObjectURI& uri)
{
    registerBitmapClass(where, displacementmapfilter_ctor,
            attachDisplacementMapFilterInterface, uri);
}

namespace {

void
attachDisplacementMapFilterInterface(as_object& o)
{
    Global_as& gl = getGlobal(o);
    o.init_member("clone", gl.createFunction(displacementmapfilter_clone));
    o.init_property("alpha", displacementmapfilter_alpha, displacementmapfilter_alpha);
    o.init_property("color", displacementmapfilter_color, displacementmapfilter_color);
    o.init_property("componentX", displacementmapfilter_componentX, displacementmapfilter_componentX);
    o.init_property("componentY", displacementmapfilter_componentY, displacementmapfilter_componentY);
    o.init_property("mapBitmap", displacementmapfilter_mapBitmap, displacementmapfilter_mapBitmap);
    o.init_property("mapPoint", displacementmapfilter_mapPoint, displacementmapfilter_mapPoint);
    o.init_property("mode", displacementmapfilter_mode, displacementmapfilter_mode);
    o.init_property("scaleX", displacementmapfilter_scaleX, displacementmapfilter_scaleX);
    o.init_property("scaleY", displacementmapfilter_scaleY, displacementmapfilter_scaleY);
}


as_value
displacementmapfilter_clone(const fn_call& fn)
{
	DisplacementMapFilter_as* ptr =
        ensure<ThisIsNative<DisplacementMapFilter_as> >(fn);
	UNUSED(ptr);
	LOG_ONCE(log_unimpl(__FUNCTION__) );
	return as_value();
}

as_value
displacementmapfilter_alpha(const fn_call& fn)
{
	DisplacementMapFilter_as* ptr =
        ensure<ThisIsNative<DisplacementMapFilter_as> >(fn);
	UNUSED(ptr);
	LOG_ONCE(log_unimpl(__FUNCTION__) );
	return as_value();
}

as_value
displacementmapfilter_color(const fn_call& fn)
{
	DisplacementMapFilter_as* ptr =
        ensure<ThisIsNative<DisplacementMapFilter_as> >(fn);
	UNUSED(ptr);
	LOG_ONCE(log_unimpl(__FUNCTION__) );
	return as_value();
}

as_value
displacementmapfilter_componentX(const fn_call& fn)
{
	DisplacementMapFilter_as* ptr =
        ensure<ThisIsNative<DisplacementMapFilter_as> >(fn);
	UNUSED(ptr);
	LOG_ONCE(log_unimpl(__FUNCTION__) );
	return as_value();
}

as_value
displacementmapfilter_componentY(const fn_call& fn)
{
	DisplacementMapFilter_as* ptr =
        ensure<ThisIsNative<DisplacementMapFilter_as> >(fn);
	UNUSED(ptr);
	LOG_ONCE(log_unimpl(__FUNCTION__) );
	return as_value();
}

as_value
displacementmapfilter_mapBitmap(const fn_call& fn)
{
	DisplacementMapFilter_as* ptr =
        ensure<ThisIsNative<DisplacementMapFilter_as> >(fn);
	UNUSED(ptr);
	LOG_ONCE(log_unimpl(__FUNCTION__) );
	return as_value();
}

as_value
displacementmapfilter_mapPoint(const fn_call& fn)
{
	DisplacementMapFilter_as* ptr =
        ensure<ThisIsNative<DisplacementMapFilter_as> >(fn);
	UNUSED(ptr);
	LOG_ONCE(log_unimpl(__FUNCTION__) );
	return as_value();
}

as_value
displacementmapfilter_mode(const fn_call& fn)
{
	DisplacementMapFilter_as* ptr =
        ensure<ThisIsNative<DisplacementMapFilter_as> >(fn);
	UNUSED(ptr);
	LOG_ONCE(log_unimpl(__FUNCTION__) );
	return as_value();
}

as_value
displacementmapfilter_scaleX(const fn_call& fn)
{
	DisplacementMapFilter_as* ptr =
        ensure<ThisIsNative<DisplacementMapFilter_as> >(fn);
	UNUSED(ptr);
	LOG_ONCE(log_unimpl(__FUNCTION__) );
	return as_value();
}

as_value
displacementmapfilter_scaleY(const fn_call& fn)
{
	DisplacementMapFilter_as* ptr =
        ensure<ThisIsNative<DisplacementMapFilter_as> >(fn);
	UNUSED(ptr);
	LOG_ONCE(log_unimpl(__FUNCTION__) );
	return as_value();
}

as_value
displacementmapfilter_ctor(const fn_call& fn)
{
	as_object* obj = ensure<ValidThis>(fn);
	obj->setRelay(new DisplacementMapFilter_as);
        return as_value(); 
}

}


} // end of gnash namespace
