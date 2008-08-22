// 
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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

#include "asClass.h"
#include "as_object.h"
#include "ClassHierarchy.h"
#include "VM.h"
#include "namedStrings.h"
#include "as_value.h"
#include "abc_function.h"

namespace gnash {
#define STV(x) VM::get().getStringTable().value(x).c_str()

asMethod::asMethod():mBody()
{
	//mArguments();
	mMinArguments = 0;
	mMaxArguments = 0;
}

void
asMethod::setOwner(asClass *pOwner)
{
	log_debug("asMethod::setOwner");
	if(!mPrototype){
		log_debug("ERROR mPrototype is null.");
	}
	log_debug("Prototype text value: %s",mPrototype->get_text_value());
	mPrototype->set_member(NSV::PROP_PROTOTYPE, pOwner->getPrototype());
}

void
asMethod::setReturnType(asClass */*type*/)
{
	/* No-op */
}

bool
asMethod::addValue(string_table::key name, asNamespace *ns, boost::uint32_t slotId,
	asClass *type, as_value& val, bool isconst)
{
	if (val.is_object())
		val.to_object()->set_member(string_table::key(NSV::INTERNAL_TYPE), 
			std::size_t(type->getName()));

	string_table::key nsname = ns ? ns->getURI() : string_table::key(0);

	int flags = as_prop_flags::dontDelete;

	if (isconst)
		flags |= as_prop_flags::readOnly;

	mPrototype->init_member(name, val, flags, nsname, slotId);
	return true;
}

bool
asClass::addValue(string_table::key name, asNamespace *ns, boost::uint32_t slotId,
	asClass *type, as_value& val, bool isconst, bool isstatic)
{
	if (val.is_object())
		val.to_object()->set_member(NSV::INTERNAL_TYPE, 
			std::size_t(type->getName()));

	string_table::key nsname = ns ? ns->getURI() : string_table::key(0);

	int flags = as_prop_flags::dontDelete;
	if (isconst)
		flags |= as_prop_flags::readOnly;
	if (isstatic)
		flags |= as_prop_flags::staticProp;

	mPrototype->init_member(name, val, flags, nsname, slotId);
	return true;
}

bool
asMethod::addMemberClass(string_table::key name, asNamespace *ns,
	boost::uint32_t slotId, asClass *type)
{
	return addSlot(name, ns, slotId, type);
}

bool
asClass::addMemberClass(string_table::key name, asNamespace *ns,
	boost::uint32_t slotId, asClass *type, bool isstatic)
{
	return addSlot(name, ns, slotId, type, isstatic);
}

bool
asMethod::addSlot(string_table::key name, asNamespace* ns, boost::uint32_t slotId,
	asClass */*type*/)
{
	string_table::key nsname = ns ? ns->getURI() : string_table::key(0);
	int flags = as_prop_flags::dontDelete;

	mPrototype->init_member(name, as_value(), flags, nsname, slotId);
	return true;
}

bool
asMethod::addSlotFunction(string_table::key name, asNamespace *ns,
	boost::uint32_t slotId, asMethod *method)
{
	asClass a;
	a.setName(NSV::CLASS_FUNCTION);
	as_value b(method->getPrototype());
	return addValue(name, ns, slotId, &a, b, false);
}

bool
asClass::addSlotFunction(string_table::key name, asNamespace *ns,
	boost::uint32_t slotId, asMethod *method, bool isstatic)
{
	asClass a;
	a.setName(NSV::CLASS_FUNCTION);
	as_value b(method->getPrototype());
	return addValue(name, ns, slotId, &a, b, false, isstatic);
}

bool
asClass::addSlot(string_table::key name, asNamespace* ns, boost::uint32_t slotId,
	asClass */*type*/, bool isstatic)
{
	string_table::key nsname = ns ? ns->getURI() : string_table::key(0);
//	int flags = as_prop_flags::dontDelete;
//	if (isstatic)
//		flags |= as_prop_flags::staticProp;
//	log_debug("Before init_member.");
	//TODO: Set flags.
	mPrototype->init_member(name, as_value(), 0, nsname, slotId);
	return true;
}

bool
asMethod::addMethod(string_table::key name, asNamespace* ns, asMethod* method)
{
//	string_table::key nsname = ns ? ns->getURI() : string_table::key(0);
//	as_value val(method->getPrototype());
// 	as value val = new as_value(abc_function(asMethod->getBody,mPrototype->getVM().getMachine()));
// 	mPrototype->init_member(name, val, as_prop_flags::readOnly |
// 		as_prop_flags::dontDelete | as_prop_flags::dontEnum, nsname);
// 	return true;
return false;
}

bool
asClass::addMethod(string_table::key name, asNamespace* ns, asMethod* method,
	bool isstatic)
{
	log_debug("in add method");
	as_value val = as_value(new abc_function(method->getBody(),mPrototype->getVM().getMachine()));
	mPrototype->init_member(name, val);
//	int flags = as_prop_flags::readOnly | as_prop_flags::dontDelete
//		| as_prop_flags::dontEnum;
//	if (isstatic)
//		flags |= as_prop_flags::staticProp;

	return true;
}

bool
asClass::addGetter(string_table::key name, asNamespace *ns, asMethod *method,
	bool isstatic)
{
	string_table::key nsname = ns ? ns->getURI() : string_table::key(0);

	Property *getset = mPrototype->getOwnProperty(name, nsname);

	if (getset)
		getset->setGetter(method->getPrototype());
	else
	{
		int flags = as_prop_flags::dontDelete | as_prop_flags::dontEnum;
		if (isstatic)
			flags |= as_prop_flags::staticProp;
		mPrototype->init_property(name, *method->getPrototype(), 
			*method->getPrototype(), flags, nsname);
	}
	return true;
}

bool
asClass::addSetter(string_table::key name, asNamespace *ns, asMethod *method,
	bool isstatic)
{
	string_table::key nsname = ns ? ns->getURI() : string_table::key(0);

	Property *getset = mPrototype->getOwnProperty(name, nsname);

	if (getset)
		getset->setSetter(method->getPrototype());
	else
	{
		int flags = as_prop_flags::dontDelete | as_prop_flags::dontEnum;
		if (isstatic)
			flags |= as_prop_flags::staticProp;
		mPrototype->init_property(name, *method->getPrototype(), 
			*method->getPrototype(), flags, nsname);
	}
	return true;
}

bool
asMethod::addGetter(string_table::key name, asNamespace *ns, asMethod *method)
{
	string_table::key nsname = ns ? ns->getURI() : string_table::key(0);

	Property *getset = mPrototype->getOwnProperty(name, nsname);

	if (getset)
		getset->setGetter(method->getPrototype());
	else
	{
		int flags = as_prop_flags::dontDelete | as_prop_flags::dontEnum;
		mPrototype->init_property(name, *method->getPrototype(), 
			*method->getPrototype(), flags, nsname);
	}
	return true;
}

bool
asMethod::addSetter(string_table::key name, asNamespace *ns, asMethod *method)
{
	string_table::key nsname = ns ? ns->getURI() : string_table::key(0);

	Property *getset = mPrototype->getOwnProperty(name, nsname);

	if (getset)
		getset->setSetter(method->getPrototype());
	else
	{
		int flags = as_prop_flags::dontDelete | as_prop_flags::dontEnum;
		mPrototype->init_property(name, *method->getPrototype(), 
			*method->getPrototype(), flags, nsname);
	}
	return true;
}

void
asNamespace::stubPrototype(string_table::key name)
{
	asClass *pClass = VM::get().getClassHierarchy()->newClass();
	pClass->setName(name);
	addClass(name, pClass);
}

#if 0 // TODO
void
asClass::buildFromPrototype(as_object *o, string_table::key name,
	ClassHierarchy *pCH)
{
	setName(name);
	PropertyList *pList = &o->_members;
	PropertyList::iterator i = pList->begin();

	for ( ; i != pList->end(); ++i)
	{
		Property *pProp = i->second;
		fprintf(stderr, "Evaluating property %s.\n", STV(i->first));
		if (pProp->isDestructive())
		{
			fprintf(stderr, "%s is destructive.\n", STV(i->first));
		}
		if (pProp->isGetterSetter())
		{
			fprintf(stderr, "%s is a getset.\n", STV(i->first));
		}
		if (pProp->isReadOnly())
		{
			fprintf(stderr, "%s is read only.\n", STV(i->first));
		}
	}
}

bool
asClass::addValue(string_table::key name, asNamespace *ns, boost::uint32_t slotId,
	asClass *type, as_value& val, bool isconst, bool isstatic,
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
asClass::addSlot(string_table::key name, asNamespace *ns, boost::uint32_t slotId,
	asClass *type, bool isstatic, ClassHierarchy *CH)
{
	asBoundValue *bv = CH->newBoundValue();
	bv->setType(type);
	if (!isstatic)
		return addBinding(name, asBinding(ns, bv, slotId, isstatic));
	return addStaticBinding(name, asBinding(ns, bv, slotId, isstatic));
}

bool
asClass::addMethod(string_table::key name, asNamespace *ns, asMethod *method,
	bool isstatic)
{
	if (!isstatic)
		return addBinding(name, asBinding(ns, method, isstatic));
	else
		return addStaticBinding(name, asBinding(ns, method, isstatic));
}

bool
asClass::addMemberClass(string_table::key name, asNamespace *ns,
	boost::uint32_t slotId, asClass *type, bool isstatic)
{
	if (!isstatic)
		return addBinding(name, asBinding(ns, type, slotId, isstatic));
	return addStaticBinding(name, asBinding(ns, type, slotId, isstatic));
}

// TODO: Figure out how this differs from addMethod
bool
asClass::addSlotFunction(string_table::key name, asNamespace *ns,
	boost::uint32_t slotId, asMethod *method, bool isstatic)
{
	if (!isstatic)
		return addBinding(name, asBinding(ns, method, slotId, isstatic));
	return addStaticBinding(name, asBinding(ns, method, slotId, isstatic));
}
#endif /* 0 */
} /* namespace gnash */
