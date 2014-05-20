// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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

#include "Class.h"
#include "as_object.h"
#include "ClassHierarchy.h"
#include "VM.h"
#include "namedStrings.h"
#include "as_value.h"
#include "Namespace.h"
#include "Global_as.h"
#include "as_class.h"

#include "Method.h"
#include "abc_function.h"

#include <functional>

namespace gnash {
namespace abc {

bool
Class::addValue(string_table::key name, Namespace *ns,
        std::uint32_t slotId, Class *type, as_value& val, bool isconst,
        bool isstatic)
{
    Global_as* g = VM::get().getGlobal();

    if (val.is_object()) {
		val.to_object(*g)->set_member(NSV::INTERNAL_TYPE, type->getName());
    }

	string_table::key nsname = ns ? ns->getURI() : string_table::key(0);

	int flags = PropFlags::dontDelete;
	if (isconst)
		flags |= PropFlags::readOnly;
	if (isstatic)
		flags |= PropFlags::staticProp;

    const ObjectURI uri(name, nsname);

	if (slotId == 0) {
		_prototype->init_member(uri, val, flags);
	}
	else {
		_prototype->init_member(uri, val, flags, slotId);
	}
	return true;
}

void
Class::initPrototype()
{
    Global_as& gl = *VM::get().getGlobal();
    _prototype = new as_class(gl, this);
}

   
void
Class::initTraits(AbcBlock& bl)
{
    std::for_each(_instanceTraits.begin(), _instanceTraits.end(),
            std::bind(&Trait::finalize, _1, &bl));

    std::for_each(_staticTraits.begin(), _staticTraits.end(),
            std::bind(&Trait::finalize, _1, &bl));
}

bool
Class::addMemberScript(string_table::key name, Namespace *ns,
	std::uint32_t slotId, Class *type, bool isstatic)
{
	return addSlot(name, ns, slotId, type, isstatic);
}

bool
Class::addSlotFunction(string_table::key name, Namespace *ns,
	std::uint32_t slotId, Method *method, bool isstatic)
{
	Class a;
	a.setName(NSV::CLASS_FUNCTION);
	as_value b(method->getPrototype());
	return addValue(name, ns, slotId, &a, b, false, isstatic);
}

bool
Class::addSlot(string_table::key name, Namespace* ns,
        std::uint32_t slotId, Class* /*type*/, bool /*isstatic*/)
{
	string_table::key nsname = ns ? ns->getURI() : 0;

	//TODO: Set flags.
	if (slotId == 0) {
		_prototype->init_member(ObjectURI(name, nsname), as_value(), 0);
	}
	else {
		_prototype->reserveSlot(ObjectURI(name, nsname), slotId);
	}
	return true;
}

bool
Class::addMethod(string_table::key name, Namespace* /*ns*/,
        Method* method, bool /*isstatic*/)
{
	as_value val = new abc::abc_function(method,
            getVM(*_prototype).getMachine());
	_prototype->init_member(name, val);
	return true;
}


bool
Class::addGetter(string_table::key name, Namespace *ns, Method *method,
	bool isstatic)
{
	string_table::key nsname = ns ? ns->getURI() : string_table::key(0);

    const ObjectURI uri(name, nsname);

	Property *getset = _prototype->getOwnProperty(uri);

	if (getset) getset->setGetter(method->getPrototype());
	else {
		int flags = PropFlags::dontDelete | PropFlags::dontEnum;
		if (isstatic) flags |= PropFlags::staticProp;
		_prototype->init_property(uri, *method->getPrototype(), 
			*method->getPrototype(), flags);
	}
	return true;
}

bool
Class::addSetter(string_table::key name, Namespace *ns, Method *method,
	bool isstatic)
{
	string_table::key nsname = ns ? ns->getURI() : string_table::key(0);
    const ObjectURI uri(name, nsname);

	Property *getset = _prototype->getOwnProperty(uri);

	if (getset) getset->setSetter(method->getPrototype());
	else
	{
		int flags = PropFlags::dontDelete | PropFlags::dontEnum;
		if (isstatic) flags |= PropFlags::staticProp;
		_prototype->init_property(uri, *method->getPrototype(), 
			*method->getPrototype(), flags);
	}
	return true;
}

#if 0 // TODO
bool
Class::addValue(string_table::key name, Namespace *ns, std::uint32_t slotId,
	Class *type, as_value& val, bool isconst, bool isstatic,
	ClassHierarchy *CH)
{
	asBoundValue *bv = CH->newBoundValue();
	bv->setType(type);
	bv->setValue(val);
	if (!isstatic)
		return addBinding(name, asBinding(ns, bv, slotId, isconst, isstatic));
	return addStaticBinding(name, asBinding(ns, bv, slotId, isconst, isstatic));
}

bool
Class::addSlot(string_table::key name, Namespace *ns, std::uint32_t slotId,
	Class *type, bool isstatic, ClassHierarchy *CH)
{
	asBoundValue *bv = CH->newBoundValue();
	bv->setType(type);
	if (!isstatic)
		return addBinding(name, asBinding(ns, bv, slotId, isstatic));
	return addStaticBinding(name, asBinding(ns, bv, slotId, isstatic));
}

bool
Class::addMethod(string_table::key name, Namespace *ns, Method *method,
	bool isstatic)
{
	if (!isstatic)
		return addBinding(name, asBinding(ns, method, isstatic));
	else
		return addStaticBinding(name, asBinding(ns, method, isstatic));
}

bool
Class::addMemberScript(string_table::key name, Namespace *ns,
	std::uint32_t slotId, Class *type, bool isstatic)
{
	if (!isstatic)
		return addBinding(name, asBinding(ns, type, slotId, isstatic));
	return addStaticBinding(name, asBinding(ns, type, slotId, isstatic));
}

// TODO: Figure out how this differs from addMethod
bool
Class::addSlotFunction(string_table::key name, Namespace *ns,
	std::uint32_t slotId, Method *method, bool isstatic)
{
	if (!isstatic)
		return addBinding(name, asBinding(ns, method, slotId, isstatic));
	return addStaticBinding(name, asBinding(ns, method, slotId, isstatic));
}
#endif 


} // namespace abc 
} // namespace gnash 
