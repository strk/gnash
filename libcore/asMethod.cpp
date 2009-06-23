// 
//   Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include "asMethod.h"
#include "asClass.h"
#include "CodeStream.h"
#include "abc_function.h"
#include "VM.h"

namespace gnash {

asMethod::asMethod()
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
    _maxStack(0)
{
}

void
asMethod::print_body()
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
asMethod::setOwner(asClass *pOwner)
{
	log_debug("asMethod::setOwner");
	if (!_prototype) {
		log_debug("ERROR _prototype is null.");
	}
	log_debug("Prototype text value: %s",_prototype->get_text_value());
	_prototype->set_member(NSV::PROP_PROTOTYPE, pOwner->getPrototype());
}

void
asMethod::setReturnType(asClass* /*type*/)
{
	/* No-op */
}

bool
asMethod::addValue(string_table::key name, asNamespace *ns,
        boost::uint32_t slotId, asClass *type, as_value& val, bool isconst)
{
	if (val.is_object()) {
		val.to_object()->set_member(NSV::INTERNAL_TYPE,
                size_t(type->getName()));
    }

	string_table::key nsname = ns ? ns->getURI() : string_table::key(0);

	int flags = as_prop_flags::dontDelete;

	if (isconst) flags |= as_prop_flags::readOnly;

	if (slotId == 0) {
		_prototype->init_member(name, val, flags, nsname);
	}
	else {
		_prototype->init_member(name, val, flags, nsname, slotId);
	}
	return true;
}

bool
asMethod::addGetter(string_table::key name, asNamespace *ns, asMethod *method)
{
	string_table::key nsname = ns ? ns->getURI() : string_table::key(0);

	Property *getset = _prototype->getOwnProperty(name, nsname);

	if (getset)
		getset->setGetter(method->getPrototype());
	else
	{
		int flags = as_prop_flags::dontDelete | as_prop_flags::dontEnum;
		_prototype->init_property(name, *method->getPrototype(), 
			*method->getPrototype(), flags, nsname);
	}
	return true;
}

bool
asMethod::addSetter(string_table::key name, asNamespace *ns, asMethod *method)
{
	string_table::key nsname = ns ? ns->getURI() : string_table::key(0);

	Property *getset = _prototype->getOwnProperty(name, nsname);

	if (getset)
		getset->setSetter(method->getPrototype());
	else
	{
		int flags = as_prop_flags::dontDelete | as_prop_flags::dontEnum;
		_prototype->init_property(name, *method->getPrototype(), 
			*method->getPrototype(), flags, nsname);
	}
	return true;
}

bool
asMethod::addMemberClass(string_table::key name, asNamespace *ns,
	boost::uint32_t slotId, asClass *type)
{
	return addSlot(name, ns, slotId, type);
}

bool
asMethod::addSlot(string_table::key name, asNamespace* ns, boost::uint32_t slotId,
	asClass */*type*/)
{
	string_table::key nsname = ns ? ns->getURI() : string_table::key(0);
	int flags = as_prop_flags::dontDelete;

	_prototype->init_member(name, as_value(), flags, nsname, slotId);
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

void
asMethod::initPrototype(Machine* machine)
{
	_prototype = new abc_function(this,machine);
}

bool
asMethod::addMethod(string_table::key /*name*/, asNamespace* /*ns*/, asMethod*
        /*method*/)
{
//	string_table::key nsname = ns ? ns->getURI() : string_table::key(0);
//	as_value val(method->getPrototype());
// 	as value val = new as_value(abc_function(asMethod->getBody,_prototype->getVM().getMachine()));
// 	_prototype->init_member(name, val, as_prop_flags::readOnly |
// 		as_prop_flags::dontDelete | as_prop_flags::dontEnum, nsname);
// 	return true;
return false;
}


} // namespace gnash
