// Color.cpp:  ActionScript class for colors, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include "Color_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "Object.h" // for getObjectInterface
#include "cxform.h" // for composition
#include "VM.h"
#include "MovieClip.h"

#include <sstream>


namespace gnash {

// Forward declarations.
namespace {
    as_value color_getrgb(const fn_call& fn);
    as_value color_gettransform(const fn_call& fn);
    as_value color_setrgb(const fn_call& fn);
    as_value color_settransform(const fn_call& fn);
    as_value color_ctor(const fn_call& fn);

    as_object* getColorInterface();
    inline void parseColorTransProp(as_object& obj, string_table::key key,
            boost::int16_t& target, bool scale);
}

void
registerColorNative(as_object& o)
{
	VM& vm = getVM(o);

	vm.registerNative(color_setrgb, 700, 0);
	vm.registerNative(color_settransform, 700, 1);
	vm.registerNative(color_getrgb, 700, 2);
	vm.registerNative(color_gettransform, 700, 3);
}

// extern (used by Global.cpp)
void
color_class_init(as_object& global, const ObjectURI& uri)
{
    Global_as* gl = getGlobal(global);
    as_object* proto = getColorInterface();
    as_object* cl = gl->createClass(&color_ctor, proto);

    // This has to be done after createClass is called, as that modifies
    // proto.
    const int protect = as_object::DefaultFlags | PropFlags::readOnly;
    proto->set_member_flags(NSV::PROP_uuPROTOuu, protect); 
    proto->set_member_flags(NSV::PROP_CONSTRUCTOR, protect); 

	// Register _global.Color
	global.init_member(getName(uri), cl, as_object::DefaultFlags,
            getNamespace(uri));

}


namespace {

void
attachColorInterface(as_object& o)
{
	VM& vm = getVM(o);

    const int flags = PropFlags::dontEnum |
                      PropFlags::dontDelete |
                      PropFlags::readOnly;

	o.init_member("setRGB", vm.getNative(700, 0), flags);
	o.init_member("setTransform", vm.getNative(700, 1), flags);
	o.init_member("getRGB", vm.getNative(700, 2), flags);
	o.init_member("getTransform", vm.getNative(700, 3), flags);

}

as_object*
getColorInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
        as_object* proto = getObjectInterface();
		o = new as_object(proto);

		attachColorInterface(*o);
	}
	return o.get();
}


as_value
color_getrgb(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = ensureType<as_object>(fn.this_ptr);

    MovieClip* sp = obj->getMember(NSV::PROP_TARGET).to_sprite();
    if (!sp) return as_value();

	const cxform& trans = sp->get_user_cxform();

    const int r = trans.rb;
    const int g = trans.gb;
    const int b = trans.bb;

    const boost::int32_t rgb = (r<<16) | (g<<8) | b;

	return as_value(rgb);
}

as_value
color_gettransform(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = ensureType<as_object>(fn.this_ptr);

    MovieClip* sp = obj->getMember(NSV::PROP_TARGET).to_sprite();
    if (!sp) return as_value();

	const cxform& cx = sp->get_user_cxform();

	// Convert to as_object

    Global_as* gl = getGlobal(fn);
    as_object* proto = getObjectInterface();
	as_object* ret = gl->createObject(proto);

	ret->init_member("ra", double(cx.ra / 2.56));
	ret->init_member("ga", double(cx.ga / 2.56));
	ret->init_member("ba", double(cx.ba / 2.56));
	ret->init_member("aa", double(cx.aa / 2.56));

	ret->init_member("rb", int(cx.rb));
	ret->init_member("gb", int(cx.gb));
	ret->init_member("bb", int(cx.bb));
	ret->init_member("ab", int(cx.ab));

	return ret;
}

as_value
color_setrgb(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = ensureType<as_object>(fn.this_ptr);
	
    if (!fn.nargs) {
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Color.setRGB() : missing argument"));
		);
		return as_value();
	}

    MovieClip* sp = obj->getMember(NSV::PROP_TARGET).to_sprite();
    if (!sp) return as_value();

	boost::int32_t color = fn.arg(0).to_int();

	const int r = (color & 0xff0000) >> 16;
	const int g = (color & 0x00ff00) >> 8;
	const int b = (color & 0x0000ff);

	cxform newTrans = sp->get_user_cxform();
	newTrans.rb = static_cast<boost::int16_t>(r);
	newTrans.gb = static_cast<boost::int16_t>(g);
	newTrans.bb = static_cast<boost::int16_t>(b);
	newTrans.ra = 0;
	newTrans.ga = 0;
	newTrans.ba = 0;

    sp->set_user_cxform(newTrans);

	return as_value();
}
as_value
color_settransform(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = ensureType<as_object>(fn.this_ptr);

	if (!fn.nargs) {
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Color.setTransform() : missing argument"));
		);
		return as_value();
	}

	boost::intrusive_ptr<as_object> trans = fn.arg(0).to_object(*getGlobal(fn));

    if (!trans) {
		IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss; fn.dump_args(ss);
            log_aserror(_("Color.setTransform(%s) : first argument doesn't "
                                "cast to an object"), ss.str());
		);
		return as_value();
	}

    MovieClip* sp = obj->getMember(NSV::PROP_TARGET).to_sprite();
    if (!sp) return as_value();

	string_table& st = getStringTable(*obj);

	cxform newTrans = sp->get_user_cxform();

	// multipliers
	parseColorTransProp(*trans, st.find("ra"), newTrans.ra, true);
	parseColorTransProp(*trans, st.find("ga"), newTrans.ga, true);
	parseColorTransProp(*trans, st.find("ba"), newTrans.ba, true);
	parseColorTransProp(*trans, st.find("aa"), newTrans.aa, true);

	// offsets
	parseColorTransProp(*trans, st.find("rb"), newTrans.rb, false);
	parseColorTransProp(*trans, st.find("gb"), newTrans.gb, false);
	parseColorTransProp(*trans, st.find("bb"), newTrans.bb, false);
	parseColorTransProp(*trans, st.find("ab"), newTrans.ab, false);

	sp->set_user_cxform(newTrans);

	return as_value();
}

/// The first argument is set as the target property.
//
/// The target property is used to change the MovieClip's color.
/// The pp calls ASSetPropFlags on all Color properties during construction,
/// adding the readOnly flag. Because Gnash adds the __constructor__ property
/// during construction, we have no control over its flags.
as_value
color_ctor(const fn_call& fn)
{
	
    as_object* proto = getColorInterface();
    boost::intrusive_ptr<as_object> obj = new as_object(proto);
    
    as_value target;
    if (fn.nargs) target = fn.arg(0);

    const int flags = as_object::DefaultFlags | PropFlags::readOnly;

    obj->init_member(NSV::PROP_TARGET, target, flags); 

	return as_value(obj.get()); // will keep alive
}

inline void
parseColorTransProp (as_object& obj, string_table::key key, boost::int16_t&
        target, bool scale)
{
	as_value tmp;
	if (!obj.get_member(key, &tmp)) return;
    
	const double d = tmp.to_number();
	if ( scale ) {   
        target = static_cast<boost::int16_t>(d * 2.56);
    }
	else {
        target = static_cast<boost::int16_t>(d);
    }
}


} // anonymous namespace 
} // end of gnash namespace
