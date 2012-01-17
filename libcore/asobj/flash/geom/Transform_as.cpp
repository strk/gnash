// Transform_as.cpp:  ActionScript "Transform" class, for Gnash.
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

#include "Transform_as.h"

#include <sstream>

#include "as_object.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "as_function.h"
#include "GnashException.h"
#include "VM.h"
#include "MovieClip.h"
#include "ColorTransform_as.h"
#include "ASConversions.h"
#include "GnashNumeric.h"
#include "namedStrings.h"

namespace gnash {

namespace {
    as_value transform_colorTransform(const fn_call& fn);
    as_value transform_concatenatedColorTransform(const fn_call& fn);
    as_value transform_concatenatedMatrix(const fn_call& fn);
    as_value transform_matrix(const fn_call& fn);
    as_value transform_pixelBounds(const fn_call& fn);
    as_value transform_ctor(const fn_call& fn);
    void attachTransformInterface(as_object& o);
    as_value get_flash_geom_transform_constructor(const fn_call& fn);
}



class Transform_as : public Relay
{
public:

    Transform_as(MovieClip& movieClip)
        :
        _movieClip(movieClip)
    {}

    SWFMatrix worldMatrix() const {
        return getWorldMatrix(_movieClip);
    }
    
    const SWFMatrix& matrix() const {
        return getMatrix(_movieClip);
    }
    
    const SWFCxForm& colorTransform() const {
        return getCxForm(_movieClip);
    }

    SWFCxForm worldColorTransform() const {
        return getWorldCxForm(_movieClip);
    }

    void setMatrix(const SWFMatrix& mat) { _movieClip.setMatrix(mat); }
    void setColorTransform(const SWFCxForm& cx) { _movieClip.setCxForm(cx); }

protected:

    virtual void setReachable() {
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
        as_value colorTrans(findObject(fn.env(), "flash.geom.ColorTransform"));

        as_function* colorTransformCtor = colorTrans.to_function();

        if (!colorTransformCtor) {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("Failed to construct flash.geom.ColorTransform!"));
            );
            return as_value();
        }

        // Construct a ColorTransform from the sprite SWFCxForm.
        const SWFCxForm& c = relay->colorTransform();

        fn_call::Args args;
        args += c.ra / factor, c.ga / factor, c.ba / factor, c.aa / factor,
             c.rb, c.gb, c.bb, c.ab;

        as_object* colorTransformObj =
            constructInstance(*colorTransformCtor, fn.env(), args);

        return as_value(colorTransformObj);
    }

    // Setter
    if (fn.nargs > 1) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror(_("Transform.colorTransform(%s): extra arguments "
                          "discarded"), ss.str());
        );
    }

    as_object* obj = toObject(fn.arg(0), getVM(fn));
    if (!obj) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror(_("Transform.colorTransform(%s): argument is not an "
                          "object"), ss.str());
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
            log_aserror(_("Transform.colorTransform(%s): argument is not a "
                          "ColorTransform"), ss.str());
        );
        return as_value();
    }
  
    const SWFCxForm c = toCxForm(*transform);
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
        as_value colorTrans(findObject(fn.env(), "flash.geom.ColorTransform"));

        as_function* colorTransformCtor = colorTrans.to_function();

        if (!colorTransformCtor) {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("Failed to construct flash.geom.ColorTransform!"));
            );
            return as_value();
        }

        // Construct a ColorTransform from the sprite SWFCxForm.
        const SWFCxForm& c = relay->worldColorTransform();

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

    if (!fn.nargs) {

        // If it's not found, construction will fail.
        as_value matrix(findObject(fn.env(), "flash.geom.Matrix"));

        as_function* matrixCtor = matrix.to_function();

        if (!matrixCtor) {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("Failed to construct flash.geom.Matrix!"));
            );
            return as_value();
        }

        const SWFMatrix& m = relay->worldMatrix();

        fn_call::Args args;
        args += m.a() / factor,
                m.b() / factor,
                m.c() / factor,
                m.d() / factor,
                twipsToPixels(m.tx()),
                twipsToPixels(m.ty());

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

    if (!fn.nargs) {

        // If it's not found, construction will fail.
        as_value matrix(findObject(fn.env(), "flash.geom.Matrix"));

        as_function* matrixCtor = matrix.to_function();

        if (!matrixCtor) {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror("Failed to construct flash.geom.Matrix!");
            );
            return as_value();
        }

        const SWFMatrix& m = relay->matrix();

        fn_call::Args args;
        args += m.a() / factor,
                m.b() / factor,
                m.c() / factor,
                m.d() / factor,
                twipsToPixels(m.tx()),
                twipsToPixels(m.ty());

        as_object* matrixObj = constructInstance(*matrixCtor, fn.env(), args);

        return as_value(matrixObj);
    }

    // Setter
    if (fn.nargs > 1) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror(_("Transform.matrix(%s): extra arguments discarded"),
                ss.str());
        );
    }

    as_object* obj = toObject(fn.arg(0), getVM(fn));
    if (!obj) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror(_("Transform.matrix(%s): argument is not an object"),
                ss.str());
        );
        return as_value();
    }

    const SWFMatrix m = toSWFMatrix(*obj);
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
            log_aserror(_("flash.geom.Transform(%s): needs one argument"),
                ss.str());
        );
        throw ActionTypeError();
    }

    // TODO: what about more than one argument? 
    if (fn.nargs > 1) {
        std::stringstream ss;
        fn.dump_args(ss);
        LOG_ONCE(log_unimpl(_("Transform(%s): %s"), ss.str(),
                    _("arguments discarded")) );
    }

    // TODO: does this have to be a MovieClip or can it be any DisplayObject?
    as_object* o = toObject(fn.arg(0), getVM(fn));
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
    as_object* proto = createObject(gl);
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
