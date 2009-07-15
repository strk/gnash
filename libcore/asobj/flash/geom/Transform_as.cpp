// Transform_as.cpp:  ActionScript "Transform" class, for Gnash.
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

#include "Transform_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "Object.h" // for AS inheritance
#include "VM.h" // for addStatics
#include "MovieClip.h" // For MovieClip
#include "ColorTransform_as.h"
#include "GnashNumeric.h"

#include <sstream>
#include <limits>

namespace gnash {

namespace {

    as_value Transform_colorTransform(const fn_call& fn);
    as_value Transform_concatenatedColorTransform(const fn_call& fn);
    as_value Transform_concatenatedMatrix(const fn_call& fn);
    as_value Transform_matrix(const fn_call& fn);
    as_value Transform_pixelBounds(const fn_call& fn);
    as_value Transform_ctor(const fn_call& fn);
    void attachTransformInterface(as_object& o);
    as_object* getTransformInterface();
    as_value get_flash_geom_transform_constructor(const fn_call& fn);
}



class Transform_as: public as_object
{

public:

	Transform_as(MovieClip& movieClip)
		:
		as_object(getTransformInterface()),
		_movieClip(movieClip)
	{}

    const SWFMatrix& getMatrix() const { return _movieClip.getMatrix(); }
    const cxform& getColorTransform() const { return _movieClip.get_cxform(); }
    void setMatrix(const SWFMatrix& mat) { _movieClip.setMatrix(mat); }
    void setColorTransform(const cxform& cx) { _movieClip.set_cxform(cx); }

protected:

    void markReachableResources() const
    {
        _movieClip.setReachable();
        markAsObjectReachable();
    }

private:

    MovieClip& _movieClip;

};

// Handle overflows from AS ColorTransform double. Doubtful
// whether it will really be inlined, but that's the compiler's
// business.
static inline boost::int16_t
truncateDouble(double d)
{

    if (d > std::numeric_limits<boost::int16_t>::max() ||
        d < std::numeric_limits<boost::int16_t>::min())
    {
       return std::numeric_limits<boost::int16_t>::min();
    }
    return static_cast<boost::int16_t>(d);
}

// extern 
void
transform_class_init(as_object& where)
{

	// Register _global.Transform
    string_table& st = getStringTable(where);
    
    // TODO: this may not be correct, but it should be enumerable.
    const int flags = 0;
    where.init_destructive_property(st.find("Transform"), 
		    get_flash_geom_transform_constructor, flags);

}

namespace {

as_value
Transform_colorTransform(const fn_call& fn)
{

    const double factor = 256.0;

	boost::intrusive_ptr<Transform_as> ptr = 
        ensureType<Transform_as>(fn.this_ptr);

    if (!fn.nargs) {

        // If it's not found, construction will fail.
        as_value colorTrans(fn.env().find_object("flash.geom.ColorTransform"));

        boost::intrusive_ptr<as_function> colorTransformCtor =
            colorTrans.to_as_function();

        if (!colorTransformCtor) {
            log_error("Failed to construct flash.geom.ColorTransform!");
            return as_value();
        }

        // Construct a ColorTransform from the sprite cxform.
        std::auto_ptr<std::vector<as_value> > args(new std::vector<as_value>);
        const cxform& c = ptr->getColorTransform();

        args->push_back(c.ra / factor);
        args->push_back(c.ga / factor);
        args->push_back(c.ba / factor);
        args->push_back(c.aa / factor);
        args->push_back(c.rb);
        args->push_back(c.gb);
        args->push_back(c.bb);
        args->push_back(c.ab);

        boost::intrusive_ptr<as_object> colorTransformObj =
            colorTransformCtor->constructInstance(fn.env(), args);

        return as_value(colorTransformObj.get());
    }

    // Setter

    if (fn.nargs > 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("Transform.colorTransform(%s): extra arguments "
                "discarded", ss.str());
        );
    }

    boost::intrusive_ptr<as_object> obj = fn.arg(0).to_object(*getGlobal(fn));
    if (!obj)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("Transform.colorTransform(%s): argument is not an "
                "object", ss.str());
        );
        return as_value();
    }
    
    // TODO: check whether this is necessary (probable), 
    // or whether it can be any object.
    boost::intrusive_ptr<ColorTransform_as> transform =
        dynamic_cast<ColorTransform_as*>(obj.get());
    if (!transform)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("Transform.colorTransform(%s): argument is not a "
                "ColorTransform", ss.str());
        );
        return as_value();
    }
    
    cxform c;
    c.ra = truncateDouble(transform->getRedMultiplier() * factor);
    c.ga = truncateDouble(transform->getGreenMultiplier() * factor);
    c.ba = truncateDouble(transform->getBlueMultiplier() * factor);
    c.aa = truncateDouble(transform->getAlphaMultiplier() * factor);
    c.rb = truncateDouble(transform->getRedOffset());
    c.gb = truncateDouble(transform->getGreenOffset());
    c.bb = truncateDouble(transform->getBlueOffset());
    c.ab = truncateDouble(transform->getAlphaOffset());
  
    ptr->setColorTransform(c);
    
    return as_value();
}

as_value
Transform_concatenatedColorTransform(const fn_call& fn)
{
	boost::intrusive_ptr<Transform_as> ptr = 
        ensureType<Transform_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
Transform_concatenatedMatrix(const fn_call& fn)
{
	boost::intrusive_ptr<Transform_as> ptr = 
        ensureType<Transform_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
Transform_matrix(const fn_call& fn)
{

    const double factor = 65536.0;

    // TODO: What happens if you do: "mat = mc.transform.matrix; mat.a = 6;"
    // (where mc is a MovieClip)? Nothing (probable), or does it change mc (how
    // would that work?)?
    // This should work by passing a new matrix, in which case we should just
    // set our _movieClip's matrix from the AS matrix.
	boost::intrusive_ptr<Transform_as> ptr = 
        ensureType<Transform_as>(fn.this_ptr);

    if (!fn.nargs)
    {

        // If it's not found, construction will fail.
        as_value matrix(fn.env().find_object("flash.geom.Matrix"));

        boost::intrusive_ptr<as_function> matrixCtor = matrix.to_as_function();

        if (!matrixCtor) {
            log_error("Failed to construct flash.geom.Matrix!");
            return as_value();
        }

        std::auto_ptr<std::vector<as_value> > args(new std::vector<as_value>);
        const SWFMatrix& m = ptr->getMatrix();

        args->push_back(m.sx / factor);
        args->push_back(m.shx / factor);
        args->push_back(m.shy / factor);
        args->push_back(m.sy / factor);
        args->push_back(twipsToPixels(m.tx));
        args->push_back(twipsToPixels(m.ty));                                

        boost::intrusive_ptr<as_object> matrixObj =
            matrixCtor->constructInstance(fn.env(), args);

        return as_value(matrixObj.get());
    }

    // Setter

    if (fn.nargs > 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("Transform.matrix(%s): extra arguments discarded",
                ss.str());
        );
    }


    boost::intrusive_ptr<as_object> obj = fn.arg(0).to_object(*getGlobal(fn));
    if (!obj)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("Transform.matrix(%s): argument is not an object",
                ss.str());
        );
        return as_value();
    }
    
    // TODO: does this have to be an AS matrix or can it be any object
    // (more likely)? 
    as_value a, b, c, d, tx, ty;
    obj->get_member(NSV::PROP_A, &a);
    obj->get_member(NSV::PROP_B, &b);
    obj->get_member(NSV::PROP_C, &c);
    obj->get_member(NSV::PROP_D, &d);
    obj->get_member(NSV::PROP_TX, &tx);
    obj->get_member(NSV::PROP_TY, &ty);

    SWFMatrix m;
    m.sx = a.to_number() * factor;
    m.shx = b.to_number() * factor;
    m.shy = c.to_number() * factor;
    m.sy = d.to_number() * factor;
    m.set_x_translation(pixelsToTwips(tx.to_number()));
    m.set_y_translation(pixelsToTwips(ty.to_number()));

    ptr->setMatrix(m);

    return as_value();

}

as_value
Transform_pixelBounds(const fn_call& fn)
{
	boost::intrusive_ptr<Transform_as> ptr = 
        ensureType<Transform_as>(fn.this_ptr);

    UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}



as_value
Transform_ctor(const fn_call& fn)
{

    if (!fn.nargs) {

        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("flash.geom.Transform(%s): needs one argument",
                ss.str());
        );
        return as_value();
    }

    // TODO: what about more than one argument? 
	if (fn.nargs > 1) {
		std::stringstream ss;
		fn.dump_args(ss);
		LOG_ONCE(log_unimpl("Transform(%s): %s", ss.str(),
                    _("arguments discarded")) );
	}

    // TODO: does this have to be a MovieClip or can it be any DisplayObject?
    boost::intrusive_ptr<MovieClip> mc =
        ensureType<MovieClip>(fn.arg(0).to_object(*getGlobal(fn)));

	boost::intrusive_ptr<as_object> obj = new Transform_as(*mc);

    // We have a movie clip. Do we construct the various properties, or are they
    // constructed on demand?
	return as_value(obj.get()); // will keep alive
}

as_value
get_flash_geom_transform_constructor(const fn_call& fn)
{
    log_debug("Loading flash.geom.Transform class");
    Global_as* gl = getGlobal(fn);
    return gl->createClass(&Transform_ctor, getTransformInterface());
}

as_object*
getTransformInterface()
{
	static boost::intrusive_ptr<as_object> o;

	if (!o) {

		// TODO: check if this class should inherit from Object
		//       or from a different class
		o = new as_object(getObjectInterface());
		VM::get().addStatic(o.get());

		attachTransformInterface(*o);

	}

	return o.get();
}

void
attachTransformInterface(as_object& o)
{
    const int protectedFlags = as_prop_flags::isProtected;

    o.init_property("matrix", Transform_matrix, Transform_matrix,
            protectedFlags);
    o.init_property("concatenatedMatrix", Transform_concatenatedMatrix,
            Transform_concatenatedMatrix, protectedFlags);
    o.init_property("colorTransform", Transform_colorTransform,
            Transform_colorTransform, protectedFlags);
    o.init_property("concatenatedColorTransform",
            Transform_concatenatedColorTransform,
            Transform_concatenatedColorTransform, protectedFlags);
    o.init_property("pixelBounds", Transform_pixelBounds,
            Transform_pixelBounds, protectedFlags);
}

} // anonymous namespace

} // end of gnash namespace
