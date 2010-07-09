// Transform_as.cpp:  ActionScript "Transform" class, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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


#include "Transform_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "VM.h"
#include "MovieClip.h" // For MovieClip
#include "ColorTransform_as.h"
#include "GnashNumeric.h"

#include "namedStrings.h"
#include <sstream>
#include <limits>

namespace gnash {

namespace {

    as_value transform_colorTransform(const fn_call& fn);
    as_value transform_concatenatedColorTransform(const fn_call& fn);
    as_value transform_concatenatedMatrix(const fn_call& fn);
    as_value transform_matrix(const fn_call& fn);
    as_value transform_pixelBounds(const fn_call& fn);
    as_value transform_ctor(const fn_call& fn);
    void attachTransformInterface(as_object& o);
    as_object* getTransformInterface();
    as_value get_flash_geom_transform_constructor(const fn_call& fn);
    
    // Handle overflows from AS ColorTransform double.
    inline boost::int16_t
    truncateDouble(double d)
    {

        if (d > std::numeric_limits<boost::int16_t>::max() ||
            d < std::numeric_limits<boost::int16_t>::min())
        {
           return std::numeric_limits<boost::int16_t>::min();
        }
        return static_cast<boost::int16_t>(d);
    }
}



class Transform_as : public Relay
{

public:

    Transform_as(MovieClip& movieClip)
        :
        _movieClip(movieClip)
    {}

    SWFMatrix getWorldMatrix() const {
        return _movieClip.getWorldMatrix();
    }
    const SWFMatrix& getMatrix() const { return _movieClip.getMatrix(); }
    const cxform& getColorTransform() const { return _movieClip.get_cxform(); }

    cxform getWorldColorTransform() const {
        return _movieClip.get_world_cxform();
    }

    void setMatrix(const SWFMatrix& mat) { _movieClip.setMatrix(mat); }
    void setColorTransform(const cxform& cx) { _movieClip.set_cxform(cx); }

protected:

    virtual void markReachableResources() const
    {
        _movieClip.setReachable();
    }

private:

    MovieClip& _movieClip;

};


// extern 
void
transform_class_init(as_object& where, const ObjectURI& uri)
{
    // TODO: this may not be correct, but it should be enumerable.
    const int flags = 0;
    where.init_destructive_property(uri, get_flash_geom_transform_constructor,
            flags);

}

namespace {

as_value
transform_colorTransform(const fn_call& fn)
{

    const double factor = 256.0;

    Transform_as* relay = ensure<ThisIsNative<Transform_as> >(fn);

    if (!fn.nargs) {

        // If it's not found, construction will fail.
        as_value colorTrans(fn.env().find_object("flash.geom.ColorTransform"));

        as_function* colorTransformCtor = colorTrans.to_function();

        if (!colorTransformCtor) {
            log_error("Failed to construct flash.geom.ColorTransform!");
            return as_value();
        }

        // Construct a ColorTransform from the sprite cxform.
        const cxform& c = relay->getColorTransform();

        fn_call::Args args;
        args += c.ra / factor, c.ga / factor, c.ba / factor, c.aa / factor,
             c.rb, c.gb, c.bb, c.ab;

        as_object* colorTransformObj =
            constructInstance(*colorTransformCtor, fn.env(), args);

        return as_value(colorTransformObj);
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

    as_object* obj = fn.arg(0).to_object(getGlobal(fn));
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
    ColorTransform_as* transform;

    if (!isNativeType(obj, transform)) {

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
  
    relay->setColorTransform(c);
    
    return as_value();
}

as_value
transform_concatenatedColorTransform(const fn_call& fn)
{
    const double factor = 256.0;

    Transform_as* relay = ensure<ThisIsNative<Transform_as> >(fn);

    if (!fn.nargs) {

        // If it's not found, construction will fail.
        as_value colorTrans(fn.env().find_object("flash.geom.ColorTransform"));

        as_function* colorTransformCtor = colorTrans.to_function();

        if (!colorTransformCtor) {
            log_error("Failed to construct flash.geom.ColorTransform!");
            return as_value();
        }

        // Construct a ColorTransform from the sprite cxform.
        const cxform& c = relay->getWorldColorTransform();

        fn_call::Args args;
        args += c.ra / factor, c.ga / factor, c.ba / factor, c.aa / factor,
             c.rb, c.gb, c.bb, c.ab;

        as_object* colorTransformObj =
            constructInstance(*colorTransformCtor, fn.env(), args);

        return as_value(colorTransformObj);
    }

    return as_value();
}

as_value
transform_concatenatedMatrix(const fn_call& fn)
{

    const double factor = 65536.0;

    Transform_as* relay = ensure<ThisIsNative<Transform_as> >(fn);

    if (!fn.nargs)
    {

        // If it's not found, construction will fail.
        as_value matrix(fn.env().find_object("flash.geom.Matrix"));

        as_function* matrixCtor = matrix.to_function();

        if (!matrixCtor) {
            log_error("Failed to construct flash.geom.Matrix!");
            return as_value();
        }

        const SWFMatrix& m = relay->getWorldMatrix();

        fn_call::Args args;
        args += m.sx / factor,
                m.shx / factor,
                m.shy / factor,
                m.sy / factor,
                twipsToPixels(m.tx),
                twipsToPixels(m.ty);

        as_object* matrixObj = constructInstance(*matrixCtor, fn.env(), args);

        return as_value(matrixObj);
    }

    return as_value();
}

as_value
transform_matrix(const fn_call& fn)
{

    const double factor = 65536.0;

    // TODO: What happens if you do: "mat = mc.transform.matrix; mat.a = 6;"
    // (where mc is a MovieClip)? Nothing (probable), or does it change mc (how
    // would that work?)?
    // This should work by passing a new matrix, in which case we should just
    // set our _movieClip's matrix from the AS matrix.
    Transform_as* relay = ensure<ThisIsNative<Transform_as> >(fn);

    if (!fn.nargs)
    {

        // If it's not found, construction will fail.
        as_value matrix(fn.env().find_object("flash.geom.Matrix"));

        as_function* matrixCtor = matrix.to_function();

        if (!matrixCtor) {
            log_error("Failed to construct flash.geom.Matrix!");
            return as_value();
        }

        const SWFMatrix& m = relay->getMatrix();

        fn_call::Args args;
        args += m.sx / factor,
                m.shx / factor,
                m.shy / factor,
                m.sy / factor,
                twipsToPixels(m.tx),
                twipsToPixels(m.ty);

        as_object* matrixObj = constructInstance(*matrixCtor, fn.env(), args);

        return as_value(matrixObj);
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


    as_object* obj = fn.arg(0).to_object(getGlobal(fn));
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

    relay->setMatrix(m);

    return as_value();

}

as_value
transform_pixelBounds(const fn_call& fn)
{
    Transform_as* relay = ensure<ThisIsNative<Transform_as> >(fn);
    UNUSED(relay);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}



as_value
transform_ctor(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);

    if (!fn.nargs) {

        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("flash.geom.Transform(%s): needs one argument",
                ss.str());
        );
        throw ActionTypeError();
    }

    // TODO: what about more than one argument? 
    if (fn.nargs > 1) {
        std::stringstream ss;
        fn.dump_args(ss);
        LOG_ONCE(log_unimpl("Transform(%s): %s", ss.str(),
                    _("arguments discarded")) );
    }

    // TODO: does this have to be a MovieClip or can it be any DisplayObject?
    as_object* o = fn.arg(0).to_object(getGlobal(fn));
    MovieClip* mc = get<MovieClip>(o);

    if (!mc) return as_value();

    obj->setRelay(new Transform_as(*mc));

    return as_value(); 
}

as_value
get_flash_geom_transform_constructor(const fn_call& fn)
{
    log_debug("Loading flash.geom.Transform class");
    Global_as& gl = getGlobal(fn);
    as_object* proto = gl.createObject();
    attachTransformInterface(*proto);
    return gl.createClass(&transform_ctor, proto);
}

void
attachTransformInterface(as_object& o)
{
    const int protectedFlags = 0;

    o.init_property("matrix", transform_matrix, transform_matrix,
            protectedFlags);
    o.init_readonly_property("concatenatedMatrix", transform_concatenatedMatrix,
            protectedFlags);
    o.init_property("colorTransform", transform_colorTransform,
            transform_colorTransform, protectedFlags);
    o.init_readonly_property("concatenatedColorTransform",
            transform_concatenatedColorTransform, protectedFlags);
    o.init_property("pixelBounds", transform_pixelBounds,
            transform_pixelBounds, protectedFlags);
}

} // anonymous namespace

} // end of gnash namespace
