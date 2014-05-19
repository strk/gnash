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

#include "Method.h"
#include "Class.h"
#include "CodeStream.h"
#include "abc_function.h"
#include "Global_as.h"
#include "VM.h"
#include "namedStrings.h"

#include <functional>

namespace gnash {
namespace abc {

Method::Method()
    :
    _methodID(0),
    _prototype(0),
	_minArguments(0),
	_maxArguments(0),
    _bodyLength(0),
    _isNative(false),
    _implementation(0),
    _flags(0),
    _body(0),
    _maxRegisters(0),
    _scopeDepth(0),
    _maxScope(0),
    _maxStack(0),
    _needsActivation(false)
{
}

void
Method::print_body()
{
		if (!_body) {
			log_parse("Method has no body.");
			return;
		}
		std::stringstream ss("Method Body:");
		for(boost::uint32_t i = 0; i < _bodyLength ; ++i) {
			const boost::uint8_t opcode = _body->read_as3op();
			ss << "0x" << std::uppercase << std::hex << (opcode | 0x0) << " ";
		}
		_body->seekTo(0);
		log_parse("%s", ss.str());
}

void
Method::setOwner(Class *pOwner)
{
	log_debug("Method::setOwner");
	if (!_prototype) {
		log_debug("ERROR _prototype is null.");
	}
	_prototype->set_member(NSV::PROP_PROTOTYPE, pOwner->getPrototype());
}

void
Method::initTraits(AbcBlock& bl)
{
    std::for_each(_traits.begin(), _traits.end(),
            std::bind(&Trait::finalize, _1, &bl));
}

void
Method::setReturnType(Class* /*type*/)
{
	/* No-op */
}

bool
Method::addValue(string_table::key name, Namespace *ns,
        boost::uint32_t slotId, Class *type, as_value& val, bool isconst)
{
    Global_as* g = VM::get().getGlobal();
	if (val.is_object()) {
		val.to_object(*g)->set_member(NSV::INTERNAL_TYPE,
                size_t(type->getName()));
    }

	string_table::key nsname = ns ? ns->getURI() : string_table::key(0);

	int flags = PropFlags::dontDelete;

	if (isconst) flags |= PropFlags::readOnly;

    const ObjectURI uri(name, nsname);

	if (slotId == 0) {
		_prototype->init_member(uri, val, flags);
	}
	else {
		_prototype->init_member(uri, val, flags, slotId);
	}
	return true;
}

bool
Method::addGetter(string_table::key name, Namespace *ns, Method *method)
{
	string_table::key nsname = ns ? ns->getURI() : string_table::key(0);

    const ObjectURI uri(name, nsname);

	Property *getset = _prototype->getOwnProperty(uri);

	if (getset)
		getset->setGetter(method->getPrototype());
	else
	{
		int flags = PropFlags::dontDelete | PropFlags::dontEnum;
		_prototype->init_property(uri, *method->getPrototype(), 
			*method->getPrototype(), flags);
	}
	return true;
}

bool
Method::addSetter(string_table::key name, Namespace *ns, Method *method)
{
	string_table::key nsname = ns ? ns->getURI() : string_table::key(0);
    
    const ObjectURI uri(name, nsname);

	Property *getset = _prototype->getOwnProperty(uri);

	if (getset)
		getset->setSetter(method->getPrototype());
	else
	{
		int flags = PropFlags::dontDelete | PropFlags::dontEnum;
		_prototype->init_property(uri, *method->getPrototype(), 
			*method->getPrototype(), flags);
	}
	return true;
}

bool
Method::addMemberScript(string_table::key name, Namespace *ns,
	boost::uint32_t slotId, Class *type)
{
	return addSlot(name, ns, slotId, type);
}

bool
Method::addSlot(string_table::key name, Namespace* ns, boost::uint32_t slotId,
	Class* /*type*/)
{
	string_table::key nsname = ns ? ns->getURI() : string_table::key(0);
	int flags = PropFlags::dontDelete;

	_prototype->init_member(ObjectURI(name, nsname), as_value(), flags, slotId);
	return true;
}

bool
Method::addSlotFunction(string_table::key name, Namespace *ns,
	boost::uint32_t slotId, Method *method)
{
	Class a;
	a.setName(NSV::CLASS_FUNCTION);
	as_value b(method->getPrototype());
	return addValue(name, ns, slotId, &a, b, false);
}

void
Method::initPrototype(abc::Machine* machine)
{
	_prototype = new abc::abc_function(this,machine);
}

bool
Method::addMethod(string_table::key /*name*/, Namespace* /*ns*/, Method*
        /*method*/)
{
//	string_table::key nsname = ns ? ns->getURI() : string_table::key(0);
//	as_value val(method->getPrototype());
// 	as value val = new as_value(abc_function(Method->getBody,getVM(_prototype).getMachine()));
// 	_prototype->init_member(name, val, PropFlags::readOnly |
// 		PropFlags::dontDelete | PropFlags::dontEnum, nsname);
// 	return true;
return false;
}

} // namespace abc
} // namespace gnash
