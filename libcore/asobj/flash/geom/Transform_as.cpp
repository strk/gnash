// Transform_as.cpp:  ActionScript "Transform" class, for Gnash.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "Transform_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "Object.h" // for AS inheritance
#include "VM.h" // for addStatics
#include "sprite_instance.h" // For MovieClip

#include <sstream>

namespace gnash {

static as_value Transform_colorTransform_getset(const fn_call& fn);
static as_value Transform_concatenatedColorTransform_getset(const fn_call& fn);
static as_value Transform_concatenatedMatrix_getset(const fn_call& fn);
static as_value Transform_matrix_getset(const fn_call& fn);
static as_value Transform_pixelBounds_getset(const fn_call& fn);


as_value Transform_ctor(const fn_call& fn);

static void
attachTransformInterface(as_object& o)
{
    o.init_property("colorTransform",
            Transform_colorTransform_getset,
            Transform_colorTransform_getset);
    o.init_property("concatenatedColorTransform",
            Transform_concatenatedColorTransform_getset,
            Transform_concatenatedColorTransform_getset);
    o.init_property("concatenatedMatrix",
            Transform_concatenatedMatrix_getset,
            Transform_concatenatedMatrix_getset);
    o.init_property("matrix",
            Transform_matrix_getset,
            Transform_matrix_getset);
    o.init_property("pixelBounds",
            Transform_pixelBounds_getset,
            Transform_pixelBounds_getset);
}

static void
attachTransformStaticProperties(as_object& /*o*/)
{
   
}

static as_object*
getTransformInterface()
{
	static boost::intrusive_ptr<as_object> o;

	if ( ! o )
	{
		// TODO: check if this class should inherit from Object
		//       or from a different class
		o = new as_object(getObjectInterface());
		VM::get().addStatic(o.get());

		attachTransformInterface(*o);

	}

	return o.get();
}

class Transform_as: public as_object
{

public:

	Transform_as(sprite_instance& movieClip)
		:
		as_object(getTransformInterface()),
		_movieClip(movieClip)
	{}

    const matrix& getMatrix() const { return _movieClip.get_matrix(); }
    const cxform& getColorTransform() const { return _movieClip.get_cxform(); }
    void setMatrix(const matrix& mat) { _movieClip.set_matrix(mat); }

private:

    sprite_instance& _movieClip;

};


static as_value
Transform_colorTransform_getset(const fn_call& fn)
{
	boost::intrusive_ptr<Transform_as> ptr = ensureType<Transform_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Transform_concatenatedColorTransform_getset(const fn_call& fn)
{
	boost::intrusive_ptr<Transform_as> ptr = ensureType<Transform_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Transform_concatenatedMatrix_getset(const fn_call& fn)
{
	boost::intrusive_ptr<Transform_as> ptr = ensureType<Transform_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Transform_matrix_getset(const fn_call& fn)
{

    const double factor = 65536.0;

    // TODO: What happens if you do: "mat = mc.transform.matrix; mat.a = 6;"
    // (where mc is a MovieClip)? Nothing (probable), or does it change mc (how
    // would that work?)?
    // This should work by passing a new matrix, in which case we should just
    // set our _movieClip's matrix from the AS matrix.
	boost::intrusive_ptr<Transform_as> ptr = ensureType<Transform_as>(fn.this_ptr);

    VM& vm = ptr->getVM();
    string_table& st = vm.getStringTable();

    if (!fn.nargs)
    {

        // This is silly. Should be easier to do, even if it's necessary
        // somewhere in the chain to go through all the objects.

        // Getter
        as_value flash;
        if (!vm.getGlobal()->get_member(st.find("flash"), &flash))
        {
            log_error("No flash object found!");
            return as_value();
        }
        boost::intrusive_ptr<as_object> flashObj = flash.to_object();

        if (!flashObj)
        {
            log_error("flash isn't an object!");
            return as_value();
        }
        
        as_value geom;
        if (!flashObj->get_member(st.find("geom"), &geom))
        {
            log_error("No flash.geom object found!");
            return as_value();
        }
        boost::intrusive_ptr<as_object> geomObj = geom.to_object();

        if (!geomObj)
        {
            log_error("flash.geom isn't an object!");
            return as_value();
        }
       
        as_value matrixVal1;
        if (!geomObj->get_member(st.find("Matrix"), &matrixVal1))
        {
            log_error("No flash.geom.Matrix object found!");
            return as_value();
        }

        boost::intrusive_ptr<as_function> matrixCtor = matrixVal1.to_as_function();
        if (!matrixCtor)
        {
            log_error("flash.geom.Matrix isn't a function!");
            return as_value();
        }

        std::auto_ptr<std::vector<as_value> > args(new std::vector<as_value>);
        const matrix& m = ptr->getMatrix();

        log_debug("Sprite matrix: %d, %d, %d, %d, %d, %d", m.sx, m.shx
            , m.sy, m.shy, m.tx, m.ty);

        args->push_back(m.sx / factor);
        args->push_back(m.shx / factor);
        args->push_back(m.shy / factor);
        args->push_back(m.sy / factor);
        args->push_back(TWIPS_TO_PIXELS(m.tx));
        args->push_back(TWIPS_TO_PIXELS(m.ty));                                

        boost::intrusive_ptr<as_object> matrixObj =
            matrixCtor->constructInstance(fn.env(), args);

        return as_value(matrixObj.get());
    }

    // Setter
    
    boost::intrusive_ptr<as_object> obj = fn.arg(0).to_object();
    if (!obj)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("Transform.matrix(%s): argument is not an object", ss.str());
        );
        return as_value()
    }
    
    as_value a, b, c, d, tx, ty;
    obj->get_member(NSV::PROP_A, &a);
    obj->get_member(NSV::PROP_B, &b);
    obj->get_member(NSV::PROP_C, &c);
    obj->get_member(NSV::PROP_D, &d);
    obj->get_member(NSV::PROP_TX, &tx);
    obj->get_member(NSV::PROP_TY, &ty);

    matrix m;
    m.sx = a.to_number() * factor;
    m.shx = b.to_number() * factor;
    m.shy = c.to_number() * factor;
    m.sy = d.to_number() * factor;
    m.set_x_translation(PIXELS_TO_TWIPS(tx.to_number()));
    m.set_y_translation(PIXELS_TO_TWIPS(ty.to_number()));

    log_debug("Sprite matrix: %d, %d, %d, %d, %d, %d", m.sx, m.shx
        , m.sy, m.shy, m.tx, m.ty);

    ptr->setMatrix(m);

    return as_value();

}

static as_value
Transform_pixelBounds_getset(const fn_call& fn)
{
	boost::intrusive_ptr<Transform_as> ptr = ensureType<Transform_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}



as_value
Transform_ctor(const fn_call& fn)
{

    if (!fn.nargs)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("flash.geom.Transform(%s): needs one argument", ss.str());
        );
        return as_value();
    }

    // TODO: what about more than one argument? 
	if (fn.nargs > 1)
	{
		std::stringstream ss;
		fn.dump_args(ss);
		LOG_ONCE( log_unimpl("Transform(%s): %s", ss.str(), _("arguments discarded")) );
	}

    boost::intrusive_ptr<sprite_instance> mc = ensureType<sprite_instance>(fn.arg(0).to_object());

	boost::intrusive_ptr<as_object> obj = new Transform_as(*mc);

    // We have a movie clip. Do we construct the various properties, or are they
    // constructed on demand?
	return as_value(obj.get()); // will keep alive
}

as_function* getFlashGeomTransformConstructor()
{
    static builtin_function* cl = NULL;
    if ( ! cl )
    {
        cl=new builtin_function(&Transform_ctor, getTransformInterface());
        VM::get().addStatic(cl);
        attachTransformStaticProperties(*cl);
    }
    return cl;
}

static as_value
get_flash_geom_transform_constructor(const fn_call& /*fn*/)
{
    log_debug("Loading flash.geom.Transform class");

    return getFlashGeomTransformConstructor();
}

// extern 
void Transform_class_init(as_object& where)
{
	// This is going to be the Transform "class"/"function"
	// in the 'where' package
	boost::intrusive_ptr<builtin_function> cl;
	cl=new builtin_function(&Transform_ctor, getTransformInterface());
	attachTransformStaticProperties(*cl);

	// Register _global.Transform
    string_table& st = where.getVM().getStringTable();
    where.init_destructive_property(st.find("Transform"), get_flash_geom_transform_constructor);}

} // end of gnash namespace
