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
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "Object.h" // for getObjectInterface
#include "cxform.h" // for composition
#include "VM.h"

#include <sstream>

// Define this to debug color settings
//#define GNASH_DEBUG_COLOR 1

namespace gnash {

// Forward declarations.
namespace {
    as_value color_getrgb(const fn_call& fn);
    as_value color_gettransform(const fn_call& fn);
    as_value color_setrgb(const fn_call& fn);
    as_value color_settransform(const fn_call& fn);
    as_value color_ctor(const fn_call& fn);

    as_object* getColorInterface();
}



class Color_as: public as_object
{

public:

	Color_as()
		:
		as_object(getColorInterface()),
		_sprite(0)
	{}

	Color_as(MovieClip* sp)
		:
		as_object(getColorInterface()),
		_sprite(sp)
	{}

	/// Mark associated sprite as reachable
	//
	/// Drop sprite instance reference if sprite
	/// was unloaded.
	///
	void markReachableResources() const
	{
		if ( checkSprite() )
		{
			assert ( ! _sprite->unloaded() );
			_sprite->setReachable();
		}
		markAsObjectReachable();
	}

	MovieClip* getSprite() const
	{
		checkSprite();
		return _sprite;
	}

	cxform getTransform() const
	{
		cxform ret;
		if ( checkSprite() ) ret = _sprite->get_user_cxform();
		return ret;
	}

	void setTransform(const cxform& newTrans) 
	{
		if (!checkSprite()) return;
		_sprite->set_user_cxform(newTrans);
	}

private:

	/// Drop reference to sprite if unloaded
	//
	/// Return true if we have a non-unloaded sprite
	///
	bool checkSprite() const
	{
		if ( ! _sprite ) return false;
		if ( _sprite->unloaded() )
		{
			_sprite = 0;
			return false;
		}
		return true;
	}

	mutable MovieClip* _sprite;

};

void registerColorNative(as_object& o)
{
	VM& vm = o.getVM();

	vm.registerNative(color_setrgb, 700, 0);
	vm.registerNative(color_settransform, 700, 1);
	vm.registerNative(color_getrgb, 700, 2);
	vm.registerNative(color_gettransform, 700, 3);
}

// extern (used by Global.cpp)
void color_class_init(as_object& global)
{
	// This is going to be the global Color "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(&color_ctor, getColorInterface());
	}

	// Register _global.Color
	global.init_member("Color", cl.get());

}


namespace {

void
attachColorInterface(as_object& o)
{
	VM& vm = o.getVM();

    const int flags = as_prop_flags::dontEnum |
                      as_prop_flags::dontDelete |
                      as_prop_flags::readOnly;

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
		o = new as_object(getObjectInterface());
		attachColorInterface(*o);
	}
	return o.get();
}


as_value
color_getrgb(const fn_call& fn)
{
	boost::intrusive_ptr<Color_as> obj = ensureType<Color_as>(fn.this_ptr);

	MovieClip* sp = obj->getSprite();
	if ( ! sp ) return as_value();

	const cxform& trans = obj->getTransform();

	int r = (int)trans.rb;
	int g = (int)trans.gb;
	int b = (int)trans.bb;

	boost::int32_t rgb = (r<<16) | (g<<8) | b;

	return as_value(rgb);
}

as_value
color_gettransform(const fn_call& fn)
{
	boost::intrusive_ptr<Color_as> obj = ensureType<Color_as>(fn.this_ptr);

	MovieClip* sp = obj->getSprite();
	if ( ! sp )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss; fn.dump_args(ss);
		log_aserror(_("Color.getTransform(%s) : no or unloaded sprite associated with the Color object"), ss.str());
		);
		return as_value();
	}

	const cxform& cx = obj->getTransform();

	// Convert to as_object

	as_object* ret = new as_object(getObjectInterface());

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
	boost::intrusive_ptr<Color_as> obj = ensureType<Color_as>(fn.this_ptr);

	if ( fn.nargs < 1 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Color.setRGB() : missing argument"));
		);
		return as_value();
	}

	boost::int32_t color = fn.arg(0).to_int();

	int r = (color&0xFF0000) >> 16;
	int g = (color&0x00FF00) >> 8;
	int b = (color&0x0000FF);

	cxform newTrans = obj->getTransform();
	newTrans.rb = (boost::int16_t)r;
	newTrans.gb = (boost::int16_t)g;
	newTrans.bb = (boost::int16_t)b;
	newTrans.ra = 0;
	newTrans.ga = 0;
	newTrans.ba = 0;

	obj->setTransform(newTrans);

	return as_value();
}

inline void
parseColorTransProp (as_object& obj, string_table::key key,
        boost::int16_t *target, bool scale)
{
	as_value tmp;
	double d;

	if ( ! obj.get_member(key, &tmp) ) {
        return;
    }
    
	d = tmp.to_number();
	if ( scale ) {   
        *target = (boost::int16_t)(d * 2.56);
    }
	else {
        *target = (boost::int16_t)d;
    }
}

as_value
color_settransform(const fn_call& fn)
{
	boost::intrusive_ptr<Color_as> obj = ensureType<Color_as>(fn.this_ptr);

	if ( fn.nargs < 1 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Color.setTransform() : missing argument"));
		);
		return as_value();
	}

	boost::intrusive_ptr<as_object> trans = fn.arg(0).to_object();
	if ( ! trans )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss; fn.dump_args(ss);
		log_aserror(_("Color.setTransform(%s) : first argument doesn't "
                            "cast to an object"), ss.str());
		);
		return as_value();
	}

	MovieClip* sp = obj->getSprite();
	if ( ! sp )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss; fn.dump_args(ss);
		log_aserror(_("Color.setTransform(%s) : no or unloaded sprite "
                        "associated with the Color object"), ss.str());
		);
		return as_value();
	}

	string_table& st = obj->getVM().getStringTable();

	cxform newTrans = obj->getTransform();

	// multipliers
	parseColorTransProp(*trans, st.find("ra"), &newTrans.ra, true);
	parseColorTransProp(*trans, st.find("ga"), &newTrans.ga, true);
	parseColorTransProp(*trans, st.find("ba"), &newTrans.ba, true);
	parseColorTransProp(*trans, st.find("aa"), &newTrans.aa, true);

	// offsets
	parseColorTransProp(*trans, st.find("rb"), &newTrans.rb, false);
	parseColorTransProp(*trans, st.find("gb"), &newTrans.gb, false);
	parseColorTransProp(*trans, st.find("bb"), &newTrans.bb, false);
	parseColorTransProp(*trans, st.find("ab"), &newTrans.ab, false);

	obj->setTransform(newTrans);

#ifdef GNASH_DEBUG_COLOR 
	std::stringstream ss; 
	as_value tmp;
	if (trans->get_member(st.find("ra"), &tmp)) ss << " ra:" << tmp.to_number();
	if (trans->get_member(st.find("ga"), &tmp)) ss << " ga:" << tmp.to_number();
	if (trans->get_member(st.find("ba"), &tmp)) ss << " ba:" << tmp.to_number();
	if (trans->get_member(st.find("aa"), &tmp)) ss << " aa:" << tmp.to_number();
	if (trans->get_member(st.find("rb"), &tmp)) ss << " rb:" << tmp.to_number();
	if (trans->get_member(st.find("gb"), &tmp)) ss << " gb:" << tmp.to_number();
	if (trans->get_member(st.find("bb"), &tmp)) ss << " bb:" << tmp.to_number();
	if (trans->get_member(st.find("ab"), &tmp)) ss << " ab:" << tmp.to_number();
	log_debug("Color.setTransform(%s) : TESTING", ss.str());
#endif

	return as_value();
}

as_value
color_ctor(const fn_call& fn)
{
	MovieClip* sp=0;
	if ( fn.nargs )
	{
		const as_value& arg = fn.arg(0);

		// TODO: check what should happen if the argument is
		//       a not-unloaded sprite but another exist with same
		//       target at lower depth (always looking up would return
		//       the lowest depth)
		sp = arg.to_sprite();
		if ( ! sp )
		{
			// must be a target..
			DisplayObject* ch = fn.env().find_target(arg.to_string());
			if ( ch )
			{
				sp = ch->to_movie();
				IF_VERBOSE_ASCODING_ERRORS(
				if ( ! sp )
				{
				std::stringstream ss; fn.dump_args(ss);
				log_aserror(_("new Color(%s) : first argument evaluates "
                                "to DisplayObject %s which is a %s (not a sprite)"),
					            ss.str(), ch->getTarget(), typeName(*ch));
				}
				);
			}
			else
			{
				IF_VERBOSE_ASCODING_ERRORS(
				std::stringstream ss; fn.dump_args(ss);
				log_aserror(_("new Color(%s) : first argument doesn't evaluate or point to a DisplayObject"),
					ss.str());
				)
			}
		}
	}

	boost::intrusive_ptr<as_object> obj = new Color_as(sp);
	
	return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // end of gnash namespace
