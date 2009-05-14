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


#include "asMethod.h"
#include "asClass.h"
#include "CodeStream.h"
#include "abc_function.h"

namespace gnash {

asMethod::asMethod()
    :
	_minArguments(0),
	_maxArguments(0),
    _body(0)
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
	if(!_prototype){
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

} // namespace gnash
