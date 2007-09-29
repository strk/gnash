// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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

#include "asClass.h"
#include "ClassHierarchy.h"
#include "VM.h"

namespace gnash {

#define STV(x) VM::get().getStringTable().value(x).c_str()

void
asNamespace::stubPrototype(string_table::key name)
{
	asClass *pClass = VM::get().getClassHierarchy()->newClass();
	pClass->setName(name);
	addClass(name, pClass);
}

bool
asMethod::addValue(string_table::key name, asNamespace *ns, uint32_t slotId,
	asClass *type, as_value& val, bool isconst, ClassHierarchy *CH)
{
	asBoundValue *bv = CH->newBoundValue();
	bv->setType(type);
	bv->setValue(val);
	return addBinding(name, asBinding(ns, bv, slotId, isconst, false));
}

bool
asMethod::addSlot(string_table::key name, asNamespace *ns, uint32_t slotId,
	asClass *type, ClassHierarchy *CH)
{
	asBoundValue *bv = CH->newBoundValue();
	bv->setType(type);
	return addBinding(name, asBinding(ns, bv, slotId, false));
}

bool
asMethod::addMethod(string_table::key name, asNamespace *ns, asMethod *method)
{
	return addBinding(name, asBinding(ns, method, false));
}

bool
asMethod::addGetter(string_table::key name, asNamespace *ns, asMethod *method,
	ClassHierarchy *CH)
{
	asBinding *bv = getBinding(name);
	if (bv)
	{
		asBoundAccessor *a = bv->getAccessor();
		if (!a)
		{
			// Okay if this is already bound to a value.
			asBoundValue *v = bv->getValue();
			if (!v)
			{
				// Let caller do the logging.
				return false;
			}
			a = CH->newBoundAccessor();
			a->setValue(v);
			bv->reset(a, bv->isStatic());
		}
		return a->setGetter(method);
	}
	asBoundAccessor *a = CH->newBoundAccessor();
	a->setGetter(method);
	return addBinding(name, asBinding(ns, a, false));
}

bool
asMethod::addSetter(string_table::key name, asNamespace *ns, asMethod *method,
	ClassHierarchy *CH)
{
	asBinding *bv = getBinding(name);
	if (bv)
	{
		asBoundAccessor *a = bv->getAccessor();
		if (!a)
		{
			asBoundValue *v = bv->getValue();
			if (!v)
			{
				// Let caller do the logging.
				return false;
			}
			a = CH->newBoundAccessor();
			a->setValue(v);
			bv->reset(a, bv->isStatic());
		}
		return a->setSetter(method);
	}
	asBoundAccessor *a = CH->newBoundAccessor();
	addBinding(name, asBinding(ns, a, false));
	return a->setSetter(method);
}

bool
asMethod::addMemberClass(string_table::key name, asNamespace *ns,
	uint32_t slotId, asClass *type)
{
	return addBinding(name, asBinding(ns, type, slotId, false));
}

// TODO: Figure out how this differs from addMethod
bool
asMethod::addSlotFunction(string_table::key name, asNamespace *ns,
	uint32_t slotId, asMethod *method)
{
	return addBinding(name, asBinding(ns, method, slotId, false));
}

bool
asClass::addValue(string_table::key name, asNamespace *ns, uint32_t slotId,
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
asClass::addSlot(string_table::key name, asNamespace *ns, uint32_t slotId,
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
asClass::addGetter(string_table::key name, asNamespace *ns, asMethod *method,
	bool isstatic, ClassHierarchy *CH)
{
	asBinding *bv;
	if (!isstatic)
		bv = getBinding(name);
	else
		bv = getStaticBinding(name);
	if (bv)
	{
		asBoundAccessor *a = bv->getAccessor();
		if (!a)
		{
			// Okay if this is already bound to a value.
			asBoundValue *v = bv->getValue();
			if (!v)
			{
				// Let caller do the logging.
				return false;
			}
			a = CH->newBoundAccessor();
			a->setValue(v);
			bv->reset(a, bv->isStatic());
		}
		return a->setGetter(method);
	}
	asBoundAccessor *a = CH->newBoundAccessor();
	a->setGetter(method);
	if (!isstatic)
		return addBinding(name, asBinding(ns, a, isstatic));
	return addStaticBinding(name, asBinding(ns, a, isstatic));
}

bool
asClass::addSetter(string_table::key name, asNamespace *ns, asMethod *method,
	bool isstatic, ClassHierarchy *CH)
{
	asBinding *bv;
	if (!isstatic)
		bv = getBinding(name);
	else
		bv = getStaticBinding(name);

	if (bv)
	{
		asBoundAccessor *a = bv->getAccessor();
		if (!a)
		{
			asBoundValue *v = bv->getValue();
			if (!v)
			{
				// Let caller do the logging.
				return false;
			}
			a = CH->newBoundAccessor();
			a->setValue(v);
			bv->reset(a, bv->isStatic());
		}
		return a->setSetter(method);
	}
	asBoundAccessor *a = CH->newBoundAccessor();
	a->setSetter(method);
	if (!isstatic)
		return addBinding(name, asBinding(ns, a, isstatic));
	else
		return addStaticBinding(name, asBinding(ns, a, isstatic));
}

bool
asClass::addMemberClass(string_table::key name, asNamespace *ns,
	uint32_t slotId, asClass *type, bool isstatic)
{
	if (!isstatic)
		return addBinding(name, asBinding(ns, type, slotId, isstatic));
	return addStaticBinding(name, asBinding(ns, type, slotId, isstatic));
}

// TODO: Figure out how this differs from addMethod
bool
asClass::addSlotFunction(string_table::key name, asNamespace *ns,
	uint32_t slotId, asMethod *method, bool isstatic)
{
	if (!isstatic)
		return addBinding(name, asBinding(ns, method, slotId, isstatic));
	return addStaticBinding(name, asBinding(ns, method, slotId, isstatic));
}

void
asNamespace::dump()
{
	container::iterator i;
	for (i = mClasses.begin(); i != mClasses.end(); ++i)
	{
		if (!((*i->second).isDeclared()))
		{
			if (i->second->isSystem())
				fprintf(stderr, "(Known to internals) ");
			fprintf(stderr, "%s.%s\n", STV(getURI()),
				STV(i->second->getName()));
//		else
//			(*i->second).dump();
		}
	}
}

void
asClass::dump()
{
	binding_container::iterator i;
	fprintf(stderr, "Class: %s\n", STV(getName()));
	for (i = mBindings.begin(); i != mBindings.end(); ++i)
	{
		fprintf(stderr, "    ");
		i->second.dump(i->first);
	}
}

void
asBinding::dump(string_table::key name)
{
	if (mNamespace->isProtected())
		fprintf(stderr, "+");
	if (mNamespace->isPrivate())
		fprintf(stderr, "#");
	if (mConst)
		fprintf(stderr, "@");
	fprintf(stderr, "%s: ", STV(name));
	switch (mType)
	{
	case T_CLASS:
	{
		fprintf(stderr, "Class %s", STV(mClass->getName()));
		break;
	}
	case T_METHOD:
	{
//		if (mConst)
			fprintf(stderr, "Member Function: %s", STV(mMethod->getReturnType()->getName()));
//		else
//			fprintf(stderr, "Function Slot");
		break;
	}
	case T_VALUE:
	{
		if (mValue->getType())
			fprintf(stderr, "Value of type %s", STV(mValue->getType()->getName()));
		else
			fprintf(stderr, "Value of unknown type.");
		break;
	}
	case T_ACCESS:
	{
		fprintf(stderr, "GetSet: ");
		if (mAccess->getGetter())
		{
			fprintf(stderr, "g:%s ", STV(mAccess->getGetter()->getReturnType()->getName()));
		}
		if (mAccess->getSetter())
		{
			fprintf(stderr, "s ");
		}
		if (mAccess->getValue())
		{
			fprintf(stderr, "dv: ");
			if (mAccess->getValue()->getType())
				fprintf(stderr, "%s ", STV(mAccess->getValue()->getType()->getName()));
			else
				fprintf(stderr, "(unknown type)");
		}
		break;
	}
	}
	fprintf(stderr, "\n");
}

} /* namespace gnash */
