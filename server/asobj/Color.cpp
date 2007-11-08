// Color.cpp:  ActionScript class for colors, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
#include "config.h"
#endif

#include "Color.h"
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

static as_value color_getrgb(const fn_call& fn);
static as_value color_gettransform(const fn_call& fn);
static as_value color_setrgb(const fn_call& fn);
static as_value color_settransform(const fn_call& fn);
static as_value color_ctor(const fn_call& fn);

static void
attachColorInterface(as_object& o)
{
	VM& vm = o.getVM();

	// Color.setRGB
	//log_debug("Registering as native 700, 0");
	vm.registerNative(color_setrgb, 700, 0);
	o.init_member("setRGB", vm.getNative(700, 0));

	// Color.setTransform
	//log_debug("Registering as native 700, 1");
	vm.registerNative(color_settransform, 700, 1);
	o.init_member("setTransform", vm.getNative(700, 1));

	// Color.getRGB
	//log_debug("Registering as native 700, 2");
	vm.registerNative(color_getrgb, 700, 2);
	o.init_member("getRGB", vm.getNative(700, 2));

	// Color.getTransform
	//log_debug("Registering as native 700, 3");
	vm.registerNative(color_gettransform, 700, 3);
	o.init_member("getTransform", vm.getNative(700, 3));

}

static as_object*
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

class color_as_object: public as_object
{

public:

	color_as_object()
		:
		as_object(getColorInterface()),
		_sprite(0)
	{}

	color_as_object(sprite_instance* sp)
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
			assert ( ! _sprite->isUnloaded() );
			_sprite->setReachable();
		}
		markAsObjectReachable();
	}

	// override from as_object ?
	//std::string get_text_value() const { return "Color"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }

	sprite_instance* getSprite() const
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
		if ( ! checkSprite() ) return; // nothing to do

#ifdef GNASH_DEBUG_COLOR
		stringstream ss; ss << newTrans;
		int r = (int)newTrans.m_[0][1];
		int g = (int)newTrans.m_[1][1];
		int b = (int)newTrans.m_[2][1];
		log_debug ("Color.setnewTransform set newTrans to = %d/%d/%d (%s)", r, g, b, ss.str().c_str());
#endif

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
		if ( _sprite->isUnloaded() )
		{
			_sprite = 0;
			return false;
		}
		return true;
	}

	mutable sprite_instance* _sprite;

};

static as_value
color_getrgb(const fn_call& fn)
{
	boost::intrusive_ptr<color_as_object> obj = ensureType<color_as_object>(fn.this_ptr);

	sprite_instance* sp = obj->getSprite();
	if ( ! sp ) return as_value();

	const cxform& trans = obj->getTransform();

	int r = (int)trans.m_[0][1];
	int g = (int)trans.m_[1][1];
	int b = (int)trans.m_[2][1];
#ifdef GNASH_DEBUG_COLOR
	log_debug ("Color.getRGB found Color transform with rgb = %d/%d/%d (%f,%f,%f)", r, g, b, trans.m_[0][1], trans.m_[1][1], trans.m_[2][1]);
#endif

	int32_t rgb = (r<<16) | (g<<8) | b;

	return as_value(rgb);
}

static as_value
color_gettransform(const fn_call& fn)
{
	boost::intrusive_ptr<color_as_object> obj = ensureType<color_as_object>(fn.this_ptr);

	sprite_instance* sp = obj->getSprite();
	if ( ! sp )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss; fn.dump_args(ss);
		log_aserror(_("Color.getTransform(%s) : no or unloaded sprite associated with the Color object"), ss.str().c_str());
		);
		return as_value();
	}

	cxform cx = obj->getTransform();

	// TODO: convert to as_object...
	as_object* ret = new as_object(getObjectInterface());

	ret->init_member("ra", cx.m_[0][0]*100);
	ret->init_member("ga", cx.m_[1][0]*100);
	ret->init_member("ba", cx.m_[2][0]*100);
	ret->init_member("aa", cx.m_[3][0]*100);

	ret->init_member("rb", cx.m_[0][1]);
	ret->init_member("gb", cx.m_[1][1]);
	ret->init_member("bb", cx.m_[2][1]);
	ret->init_member("ab", cx.m_[3][1]);

	return ret;
}

static as_value
color_setrgb(const fn_call& fn)
{
	boost::intrusive_ptr<color_as_object> obj = ensureType<color_as_object>(fn.this_ptr);

	if ( fn.nargs < 1 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Color.setRGB() : missing argument"));
		);
		return as_value();
	}

	int32_t color = fn.arg(0).to_int(fn.env());

	int r = (color&0xFF0000) >> 16;
	int g = (color&0x00FF00) >> 8;
	int b = (color&0x0000FF);

	cxform newTrans = obj->getTransform();
	newTrans.m_[0][1] = r;
	newTrans.m_[1][1] = g;
	newTrans.m_[2][1] = b;
	newTrans.m_[0][0] = 0;
	newTrans.m_[1][0] = 0;
	newTrans.m_[2][0] = 0;

	obj->setTransform(newTrans);

	return as_value();
}

static inline void
parseColorTransProp (as_object& obj, as_environment& env, string_table::key key, float *target, bool scale)
{
	as_value tmp;
	double d;

	if ( ! obj.get_member(key, &tmp) ) return;
	d = tmp.to_number(&env);
	if ( scale ) *target = d/100.0;
	else *target = d;
}

static as_value
color_settransform(const fn_call& fn)
{
	boost::intrusive_ptr<color_as_object> obj = ensureType<color_as_object>(fn.this_ptr);

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
		log_aserror(_("Color.setTransform(%s) : first argument doesn't cast to an object"), ss.str().c_str());
		);
		return as_value();
	}

	sprite_instance* sp = obj->getSprite();
	if ( ! sp )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss; fn.dump_args(ss);
		log_aserror(_("Color.setTransform(%s) : no or unloaded sprite associated with the Color object"), ss.str().c_str());
		);
		return as_value();
	}

	string_table& st = obj->getVM().getStringTable();
	as_environment& env = fn.env();

	cxform newTrans = obj->getTransform();

	// multipliers
	parseColorTransProp(*trans, env, st.find("ra"), &newTrans.m_[0][0], true);
	parseColorTransProp(*trans, env, st.find("ga"), &newTrans.m_[1][0], true);
	parseColorTransProp(*trans, env, st.find("ba"), &newTrans.m_[2][0], true);
	parseColorTransProp(*trans, env, st.find("aa"), &newTrans.m_[3][0], true);

	// offsets
	parseColorTransProp(*trans, env, st.find("rb"), &newTrans.m_[0][1], false);
	parseColorTransProp(*trans, env, st.find("gb"), &newTrans.m_[1][1], false);
	parseColorTransProp(*trans, env, st.find("bb"), &newTrans.m_[2][1], false);
	parseColorTransProp(*trans, env, st.find("ab"), &newTrans.m_[3][1], false);

	obj->setTransform(newTrans);

#ifdef GNASH_DEBUG_COLOR 
	std::stringstream ss; 
	as_value tmp;
	if ( trans->get_member(st.find("ra"), &tmp) ) ss << " ra:" << tmp.to_number();
	if ( trans->get_member(st.find("ga"), &tmp) ) ss << " ga:" << tmp.to_number();
	if ( trans->get_member(st.find("ba"), &tmp) ) ss << " ba:" << tmp.to_number();
	if ( trans->get_member(st.find("aa"), &tmp) ) ss << " aa:" << tmp.to_number();
	if ( trans->get_member(st.find("rb"), &tmp) ) ss << " rb:" << tmp.to_number();
	if ( trans->get_member(st.find("gb"), &tmp) ) ss << " gb:" << tmp.to_number();
	if ( trans->get_member(st.find("bb"), &tmp) ) ss << " bb:" << tmp.to_number();
	if ( trans->get_member(st.find("ab"), &tmp) ) ss << " ab:" << tmp.to_number();
	log_debug("Color.setTransform(%s) : TESTING", ss.str().c_str());
#endif

	return as_value();
}

static as_value
color_ctor(const fn_call& fn)
{
	sprite_instance* sp=0;
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
			as_environment& env = fn.env();
			character* ch = env.find_target(fn.arg(0).to_string(&env));
			if ( ch ) sp = ch->to_movie();
		}

		IF_VERBOSE_ASCODING_ERRORS(
		if ( ! sp )
		{
			std::stringstream ss; fn.dump_args(ss);
			log_aserror(_("new Color(%s) : first argument doesn't evaluate or point to a MovieClip"),
				ss.str().c_str());
		}
		)
	}

	boost::intrusive_ptr<as_object> obj = new color_as_object(sp);
	
	return as_value(obj.get()); // will keep alive
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


} // end of gnash namespace
