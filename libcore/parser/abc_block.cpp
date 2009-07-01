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

// The AS3 abc block format reader.
//

#include "gnashconfig.h"
#include "abc_block.h"
#include "SWFStream.h" // for use
#include "VM.h"
#include "log.h"
#include "ClassHierarchy.h"
#include "asClass.h"
#include "namedStrings.h"
#include "CodeStream.h"
#include "action_buffer.h"
#include "Machine.h"
#include "Global.h"

namespace gnash {

namespace abc {

bool
Trait::finalize(abc_block *pBlock, asClass *pClass, bool do_static)
{
	log_abc("Finalize class %s (%s), trait kind: %s", 
            pBlock->_stringPool[pClass->getName()], pClass, _kind);

	switch (_kind)
	{
	case KIND_SLOT:
	case KIND_CONST:
	{
		// Validate the type.
		asClass *pType;
		if (_typeIndex) {
			log_abc("Trait type: %s", 
                pBlock->_stringPool[
                    pBlock->_multinamePool[_typeIndex].getABCName()]);
			pType = pBlock->locateClass(pBlock->_multinamePool[_typeIndex]);
		}
		else {
			pType = pBlock->mTheObject;
		}

		if (!pType) {
			log_error(_("ABC: Finalizing trait yielded bad type for slot."));
			return false;
		}

		// The name has been validated in read.
		// TODO: Find a better way to initialize trait values.
		if (!_hasValue) {
            as_object* null = 0;
			_value = null; 
		}

		log_abc("Adding property=%s with value=%s slot=%u",
                pBlock->_stringPool[_name], _value, _slotID);

		pClass->addValue(_globalName, _namespace, _slotID, pType, 
			_value, _kind == KIND_CONST, do_static);
		break;
	}
	case KIND_METHOD:
	{
		pClass->addMethod(_globalName, _namespace, _method, false);
		break;
	}
	case KIND_GETTER:
	{
		pClass->addGetter(_name, _namespace, _method, do_static);
		break;
	}
	case KIND_SETTER:
	{
		pClass->addSetter(_name, _namespace, _method, do_static);
		break;
	}
	case KIND_CLASS:
	{
		log_abc("Adding class %s, value %s, slot=%u",
                pBlock->_stringPool[_name], _value, _slotID);

		pClass->addMemberClass(_globalName, _namespace, _slotID, 
			pBlock->_classes[_classInfoIndex], do_static);
		break;
	}
	case KIND_FUNCTION:
	{
		pClass->addSlotFunction(_name, _namespace, _slotID, _method, do_static);
		break;
	}
	default:
		// Not here -- validated already in read.
		return false;
		break;
	} // end of switch
	return true;
}

bool
Trait::finalize_mbody(abc_block *pBlock, asMethod *pMethod)
{
	log_abc("Finalizing method");
	switch (_kind)
	{
	case KIND_SLOT:
	case KIND_CONST:
	{
		// Validate the type.
		asClass *pType;
		if (_typeIndex)
			pType = pBlock->locateClass(pBlock->_multinamePool[_typeIndex]);
		else
			pType = pBlock->mTheObject;
		if (!pType)
		{
			log_error(_("ABC: Finalizing trait yielded bad type for slot."));
			return false;
		}
		// The name has been validated in read.
		// TODO: Find a better way to initialize trait values.
		if (!_hasValue) {
			_value = as_value((as_object*)0); // NULL value, right ?
		}
		log_abc("Adding property=%s with value=%s slot=%u", pBlock->_stringPool[_name], _value.toDebugString(), _slotID);
		pMethod->addValue(_globalName, _namespace, _slotID, pType, 
			_value, _kind == KIND_CONST);
		break;
	}
	case KIND_METHOD:
	{
		log_abc("Finalize method trait not implemented.  Returning");
		break;
		pMethod->addMethod(_name, _namespace, _method);
		break;
	}
	case KIND_GETTER:
	{
		log_abc("Finalize getter trait not implemented.  Returning");
		break;
		pMethod->addGetter(_name, _namespace, _method);
		break;
	}
	case KIND_SETTER:
	{
		log_abc("Finalize setter trait not implemented.  Returning");
		break;
		pMethod->addSetter(_name, _namespace, _method);
		break;
	}
	case KIND_CLASS:
	{
		log_abc("Finalize class trait not implemented.  Returning");
		break;
		pMethod->addMemberClass(_name, _namespace, _slotID, 
			pBlock->_classes[_classInfoIndex]);
		break;
	}
	case KIND_FUNCTION:
	{
		log_abc("Finalize function trait not implemented.  Returning");
		break;
		pMethod->addSlotFunction(_name, _namespace, _slotID, _method);
		break;
	}
	default:
		// Not here -- validated already in read.
		return false;
		break;
	} // end of switch
	return true;
}

/// Read an AS3 'trait'
bool
Trait::read(SWFStream* in, abc_block *pBlock)
{
	boost::uint32_t name = in->read_V32();
	if (name >= pBlock->_multinamePool.size())
	{
		log_error(_("ABC: Bad name for trait."));
		return false;
	}
	if (!pBlock->_multinamePool[name].isQName())
	{
		log_error(_("ABC: Trait name must be fully qualified."));
		return false;
	}
	asName multiname = pBlock->_multinamePool[name];
	_name = pBlock->_multinamePool[name].getABCName();
	_globalName = pBlock->_multinamePool[name].getGlobalName();
	_namespace = pBlock->_multinamePool[name].getNamespace();

	boost::uint8_t kind = in->read_u8();
	_kind = static_cast<Kind>(kind & 0x0F);

	log_abc("Trait name: %s, Trait kind: %s",
            pBlock->_stringPool[multiname.getABCName()], _kind);

    switch (_kind)
	{
	case KIND_SLOT:
	case KIND_CONST:
	{
		_slotID = in->read_V32();
		_typeIndex = in->read_V32();
		boost::uint32_t vindex = in->read_V32();
		log_abc("Slot ID=%u Type=%s Pool index=%u", _slotID, pBlock->_stringPool[pBlock->_multinamePool[_typeIndex].getABCName()], vindex);
		if (vindex)
		{
			if (!pBlock->pool_value(vindex, in->read_u8(), _value))
				return false; // Message done by pool_value
			_hasValue = true;
		}
		else
			_hasValue = false;
		break;
	}
	case KIND_METHOD:
	case KIND_GETTER:
	case KIND_SETTER:
	{
		// Ignore the 'disp_id'
		in->skip_V32();

		boost::uint32_t offset = in->read_V32();
		log_abc("Method index=%u", offset);
		if (offset >= pBlock->_methods.size()) {
			log_error(_("Bad method id in trait."));
			return false;
		}
		_method = pBlock->_methods[offset];
		break;
	}
	case KIND_CLASS:
	{
		_slotID = in->read_V32();
		_classInfoIndex = in->read_V32();
		log_abc("Slot id: %u Class index: %u Class Name: %s", _slotID, 
                _classInfoIndex, 
                pBlock->_stringPool[pBlock->
                        _classes[_classInfoIndex]->getName()]);

        if (_classInfoIndex >= pBlock->_classes.size()) {
			log_error(_("Bad Class id in trait."));
			return false;
		}
		break;
	}
	case KIND_FUNCTION:
	{
		_slotID = in->read_V32();
		boost::uint32_t offset = in->read_V32();
		if (offset >= pBlock->_methods.size()) {
			log_error(_("Bad method id in trait."));
			return false;
		}
		_method = pBlock->_methods[offset];
		break;
	}
	default:
	{
		log_error(_("ABC: Unknown type of trait."));
//		return false;
	}
	} // end of switch statement

	// Ignore the metadata, but it must be read to know how to ignore it.
	if ((kind >> 4) & 0x04) // has metadata
	{
		boost::uint32_t mcount = in->read_V32();
		for (unsigned int i = 0; i < mcount; ++i)
		{
			in->skip_V32();
		}
	}
	return true;
}

std::ostream&
operator<<(std::ostream& o, const Trait::Kind k)
{
    switch (k) {
        case abc::Trait::KIND_SLOT:
            return o << "slot";
        case abc::Trait::KIND_CONST:
            return o << "const";
        case abc::Trait::KIND_METHOD:
            return o << "method";
        case abc::Trait::KIND_GETTER:
            return o << "getter";
        case abc::Trait::KIND_SETTER:
            return o << "setter";
        case abc::Trait::KIND_CLASS:
            return o << "class";
        case abc::Trait::KIND_FUNCTION:
            return o << "function";
        default:
            return o << "Unknown kind " << static_cast<int>(k);
    }
}

} // abc

using namespace abc;

abc_block::abc_block()
    :
    _stringTable(&VM::get().getStringTable())
{
	mCH = VM::get().getMachine()->classHierarchy();
	// TODO: Make this the real 'Object' prototype.
	mCH->getGlobalNs()->stubPrototype(*mCH, NSV::CLASS_OBJECT);
	mTheObject = mCH->getGlobalNs()->getClass(NSV::CLASS_OBJECT);
}

void
abc_block::prepare(Machine* mach)
{

    std::for_each(_classes.begin(), _classes.end(),
            std::mem_fun(&asClass::initPrototype));

    // The last (entry) script has Global as its prototype.
    // This can be deduced because the global classes are initialized with a
    // slot on script 0 (entry script). OpNewClass then attempts to set the
    // corresponding slot once the class has been constructed. At this point,
    // global should verifiably be on the stack, so the slots are expected
    // to be set on the global object.
    // It seems likely, though testing it is not straightforward, that all
    // scripts have Global as a target object (prototype), so for now we
    // will do that.
    std::for_each(_scripts.begin(), _scripts.end(),
            boost::bind(&asClass::setPrototype, _1, mach->global()));
 
    std::for_each(_methods.begin(), _methods.end(),
            boost::bind(&asMethod::initPrototype, _1, mach));

    std::for_each(_traits.begin(), _traits.end(),
            boost::bind(&Trait::finalize, _1, this));

    _traits.clear();

}

void
abc_block::check_multiname_name(boost::uint32_t name)
{

	if (name >= _stringPool.size()) {
		throw ParserException("ABC: Out of bounds string for Multiname.");
	}
}

void
abc_block::check_multiname_namespace(boost::uint32_t ns)
{
	if (ns >= _namespacePool.size()) {
		throw ParserException("ABC: Out of bounds namespace for Multiname.");
	}
}

void
abc_block::check_multiname_namespaceset(boost::uint32_t nsset)
{
	if (!nsset)
    {
		throw ParserException("ABC: 0 selection for namespace set is invalid.");
	}
	if (nsset >= _namespaceSetPool.size())
    {
		throw ParserException("ABC: Out of bounds namespace set for Multiname.");
	}
}

void
abc_block::setMultinameNames(asName *n, string_table::key ABCName)
{
	
	n->setABCName(ABCName);
	std::string name = _stringPool[ABCName];
	string_table::key global_key = _stringTable->find(name, true);
	log_abc("Global key %u", global_key);
	n->setGlobalName(global_key);
	log_abc("Multiname: %s ABCName set to %u, global name set to %u",
            name, n->getABCName(), n->getGlobalName());
}

void
abc_block::setNamespaceURI(asNamespace *ns, string_table::key ABCName)
{
	ns->setAbcURI(ABCName);
	std::string name = _stringPool[ABCName];
	string_table::key global_key = _stringTable->find(name);
	ns->setURI(global_key);
	log_abc("Namespace: %s AbcURI=%u URI=%u.", name, ABCName, global_key);
}

asClass *
abc_block::locateClass(asName &m)
{
	asClass *found = NULL;

	if (m.getNamespace())
	{
		found = m.getNamespace()->getClass(m.getABCName());
		if (found)
			return found;
	}
	if (m.namespaceSet() && !m.namespaceSet()->empty())
	{
		std::vector<asNamespace*>::const_iterator i;
		for (i = m.namespaceSet()->begin(); i != m.namespaceSet()->end(); ++i)
		{
			found = (*i)->getClass(m.getABCName());
			if (found)
				return found;
		}
	}


    // Look in known built-in classes.
    asNamespace* nsToFind = m.getNamespace();

    // If there is no namespace specified, look in global only.
    // TODO: check if this is correct, or if there should always be
    // a namespace.
    if (!nsToFind) {
        return mCH->getGlobalNs()->getClass(m.getGlobalName());
    }

    // Else look in the specified namespace only.
    asNamespace* ns = mCH->findNamespace(nsToFind->getURI());
    return ns ? ns->getClass(m.getGlobalName()) : 0;

}

/// Read the ActionBlock version number.
bool
abc_block::read_version()
{
	// Minor version, major version.
	mVersion = (_stream->read_u16()) | (_stream->read_u16() << 16);
	log_debug(_("Abc Version: %d.%d"), (mVersion & 0xFFFF0000) >> 16, 
		(mVersion & 0x0000FFFF));
	return true;
}

/// Read the pool of integer constants.
bool
abc_block::read_integer_constants()
{
	// count overestimates by 1.
	boost::uint32_t count = _stream->read_V32();
	_integerPool.resize(count);
	if (count)
		_integerPool[0] = 0;
	for (unsigned int i = 1; i < count; ++i)
	{
		_integerPool[i] = static_cast<boost::int32_t> (_stream->read_V32());
	}
	return true;
}

/// Read the pool of unsigned integer constants.
bool
abc_block::read_unsigned_integer_constants()
{
	// count overestimates by 1.
	boost::uint32_t count = _stream->read_V32();
	_uIntegerPool.resize(count);
	if (count)
		_uIntegerPool[0] = 0;
	for (unsigned int i = 1; i < count; ++i)
	{
		_uIntegerPool[i] = _stream->read_V32();
	}
	return true;
}

/// Read the pool of 64-bit double constants.
bool
abc_block::read_double_constants()
{
	boost::uint32_t count = _stream->read_V32();
	_doublePool.resize(count);
	if (count)
		_doublePool[0] = 0.0;
	for (unsigned int i = 1; i < count; ++i)
	{
		_doublePool[i] = _stream->read_d64();
		log_abc("Double %u=%lf", i, _doublePool[i]);
	}
	return true;
}

/// Read the pool of string constants.
bool
abc_block::read_string_constants()
{
	log_abc("Begin reading string constants.");
	boost::uint32_t count = _stream->read_V32();
	log_abc("There are %u string constants.", count);
	_stringPool.resize(count);
	_stringPoolTableIDs.resize(count);
	if (count)
	{
		_stringPool[0] = "";
		_stringPoolTableIDs[0] = 0;
	}
	for (unsigned int i = 1; i < count; ++i)
	{
		boost::uint32_t length = _stream->read_V32();
		_stream->read_string_with_length(length, _stringPool[i]);
		log_abc("Adding string constant to string pool: index=%u %s", i, _stringPool[i]);
		_stringPoolTableIDs[i] = 0;
	}
	return true;
}

/// Read the pool of namespaces
/// Any two namespaces with the same uri here are the same namespace, 
/// excepting private namespaces.
bool
abc_block::read_namespaces()
{	
	log_abc("Begin reading namespaces.");
	boost::uint32_t count = _stream->read_V32();
	log_abc("There are %u namespaces.", count);
	_namespacePool.resize(count);
	if (count)
	{
		_namespacePool[0] = mCH->getGlobalNs();
	}
	for (unsigned int i = 1; i < count; ++i)
	{
		boost::uint8_t kind = _stream->read_u8();
		boost::uint32_t nameIndex = _stream->read_V32();
		log_abc("Namespace %u: kind %s, index %u, name %s", i,
                static_cast<int>(kind), nameIndex, _stringPool[nameIndex]);

		if (nameIndex >= _stringPool.size()) {
			log_error(_("ABC: Bad string given for namespace."));
			return false;
		}
		
		if (kind == PRIVATE_NS)
		{
			_namespacePool[i] = mCH->anonNamespace(nameIndex);
			_namespacePool[i]->setPrivate();
		}
		else
		{
			asNamespace *n = mCH->findNamespace(nameIndex);
			if (!n) n = mCH->addNamespace(nameIndex);
			_namespacePool[i] = n;
		}
		if (kind == PROTECTED_NS) _namespacePool[i]->setProtected();
		if (kind == PACKAGE_NS) _namespacePool[i]->setPackage();
		setNamespaceURI(_namespacePool[i], nameIndex);
	}
	return true;
}

/// Read the set of sets of namespaces.
bool
abc_block::read_namespace_sets()
{
	boost::uint32_t count = _stream->read_V32();
	_namespaceSetPool.resize(count);
	if (count)
	{
		_namespaceSetPool[0].resize(0);
	}
	for (unsigned int i = 1; i < count; ++i)
	{
		boost::uint32_t icount = _stream->read_V32();
		_namespaceSetPool[i].resize(icount);
		for (unsigned int j = 0; j < icount; ++j)
		{
			boost::uint32_t selection = _stream->read_V32();
			if (!selection || selection >= _namespacePool.size())
			{
				log_error(_("ABC: Bad namespace for namespace set."));
				return false;
			}
			_namespaceSetPool[i][j] = _namespacePool[selection];
		}
	}
	return true;
}

/// Read the multinames.
bool
abc_block::read_multinames()
{
	boost::uint32_t count = _stream->read_V32();
	log_abc("There are %u multinames.", count);
	_multinamePool.resize(count);
	if (count)
	{
//		_multinamePool[0].setABCName(0);
		setMultinameNames(&_multinamePool[0], 0);
		_multinamePool[0].setNamespace(mCH->getGlobalNs());
	}
	for (unsigned int i = 1; i < count; ++i)
	{
        asName::Kind kind = static_cast<asName::Kind>(_stream->read_u8());
		boost::uint32_t ns = 0;
		boost::uint32_t name = 0;
		boost::uint32_t nsset = 0;

		log_abc("Multiname %u has kind %s", i, static_cast<int>(kind));

		// Read, but don't upper validate until after the switch.
		switch (kind)
		{
            case asName::KIND_Qname:
            case asName::KIND_QnameA:
                ns = _stream->read_V32();
                check_multiname_namespace(ns);
                name = _stream->read_V32();
                check_multiname_name(name);
                log_abc("\tnamespace_index=%u name_index=%u name=%s",
                        ns, name, _stringPool[name]);
                break;
            
            case asName::KIND_RTQname:
            case asName::KIND_RTQnameA:
                name = _stream->read_V32();
                check_multiname_name(name);
                break;
            
            case asName::KIND_RTQnameL:
            case asName::KIND_RTQnameLA:
                break;
            
            case asName::KIND_Multiname:
            case asName::KIND_MultinameA:
                name = _stream->read_V32();
                check_multiname_name(name);
                nsset = _stream->read_V32();
                check_multiname_namespaceset(nsset);
                break;
            
            case asName::KIND_MultinameL:
            case asName::KIND_MultinameLA:
                nsset = _stream->read_V32();
                check_multiname_namespaceset(nsset);
                break;
            
            default:
                // Unknown type.
                log_error(_("Action Block: Unknown multiname type (%d)."),
                        kind);
                return false;
        } 

		_multinamePool[i].setFlags(kind);
		setMultinameNames(&_multinamePool[i], name);
		log_abc("Done setting multinames: abc=%u global=%u",
               _multinamePool[i].getABCName(),
               _multinamePool[i].getGlobalName());

        _multinamePool[i].setNamespace(_namespacePool[ns]);

		if (nsset) {
			_multinamePool[i].namespaceSet(&_namespaceSetPool[nsset]);
        }
	} // End of main loop.
	return true;
}

bool
abc_block::pool_value(boost::uint32_t index, boost::uint8_t type, as_value &v)
{
	if (!index)
		return true;

	log_abc("Pool value: index is %u type is 0x%X", index, type | 0x0);
	switch (type)
	{
	case POOL_STRING: 
	{
		if (index >= _stringPool.size())
		{
			log_error(_("Action Block: Bad index in optional argument."));
			return false;
		}
		v.set_string(_stringPool[index]);
		break;
	}
	case POOL_INTEGER: 
	{
		if (index >= _integerPool.size())
	    {
			log_error(_("Action Block: Bad index in optional argument."));
			return false;
		}
		v.set_int(_integerPool[index]);
		break;
	}
	case POOL_UINTEGER:
	{
		if (index >= _uIntegerPool.size())
		{
			log_error(_("Action Block: Bad index in optional argument."));
			return false;
		}
		v.set_int(_uIntegerPool[index]);
		break;
	}
	case POOL_DOUBLE: 
	{
		if (index >= _doublePool.size())
		{
			log_error(_("Action Block: Bad index in optional argument."));
			return false;
		}
		v.set_double(static_cast<double>(_doublePool[index]));
		break;
	}
	case POOL_NAMESPACE: // Namespace
	{
		if (index >= _namespacePool.size())
		{
			log_error(_("ABC: Bad index in optional argument, namespaces."));
			return false;
		}
		break;
	}
	case POOL_FALSE: // False value
	{
		v.set_bool(false);
		break;
	}
	case POOL_TRUE: // True value
	{
		v.set_bool(true);
		break;
	}
	case POOL_NULL: // NULL value
	{
		v.set_null();
		break;
	}
	default: // All others are bogus.
	{
		log_error(_("ABC: Bad default value type (%X), but continuing."), type);
		return true;
		break;
	}
	} // end of switch
	return true;
}

/// Read the method infos.
bool
abc_block::read_method_infos()
{
	log_abc("Begin read_method_infos.");

	boost::uint32_t count = _stream->read_V32();
    log_abc("Method count: %u", count);

	_methods.resize(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		log_abc(" Reading method %u", i);
		asMethod *pMethod = mCH->newMethod();
		pMethod->setMethodID(i);
		_methods[i] = pMethod;
		boost::uint32_t param_count = _stream->read_V32();
		boost::uint32_t return_type = _stream->read_V32();

        const std::string& rt = return_type ? 
            _stringPool[_multinamePool[return_type].getABCName()] :
            "*";

		log_abc("  Param count: %u, return type: %s", param_count, 
                rt, return_type);

		pMethod->setMinArgumentCount(param_count);
		pMethod->setMaxArgumentCount(param_count);

		if (return_type >= _multinamePool.size()) {
			log_error(_("ABC: Bad return type for method info."));
			return false;
		}

        if (!return_type) {
            pMethod->setReturnType(0);
        }
        else {
            // TODO: this can be 'void', which clearly isn't a class, so this
            // seems bogus. As setReturnType is a no-op, we should log it
            // and ignore it.
            asClass* rtClass = locateClass(_multinamePool[return_type]);
            if (!rtClass) {
                log_abc(_("ABC: Unknown return type."));
            }

            pMethod->setReturnType(rtClass);
        }
		for (size_t j = 0; j < param_count; ++j) {
			log_abc("  Reading parameter %u", j);
			// The parameter type.
			boost::uint32_t ptype = _stream->read_V32();
            
            const std::string& pt = return_type ? 
                _stringPool[_multinamePool[ptype].getABCName()] :
                "*";
			
            log_abc("   Parameter type(index): %s(%u)", pt, ptype);

			if (ptype >= _multinamePool.size()) {
				log_error(_("ABC: Bad parameter type in method."));
				return false;
			}
			
            // A value of 0 is legitimate, meaning 'any (*)'. 
            if (ptype) {
                asClass* param_type = locateClass(_multinamePool[ptype]);

                if (!param_type) {
                    log_abc((_("ABC: Unknown parameter type.")));
                }
                
                // This currently also pushes 0, meaning 'any'; perhaps it
                // should throw a VerifyError if the class is not known.
                pMethod->pushArgument(param_type);
            }
            else {
                pMethod->pushArgument(0);
            }
		}

		boost::uint32_t method_name = _stream->read_V32();
		log_abc(  "Method name=%s %d", _stringPool[method_name], method_name);
		boost::uint8_t flags = _stream->read_u8();
		log_abc("  Flags: %X", flags | 0x0);
//		log_abc("Check if flags and optional args.");
		// If there are default parameters, read them now.
		// Runtime will do validation of whether or not these can actually
		// be assigned to the corresponding parameters.
		if (flags & METHOD_OPTIONAL_ARGS)
		{
//			log_abc("We have flags and optional args.");
			boost::uint32_t ocount = _stream->read_V32();
			log_abc("  Optional args: %u", ocount);
			pMethod->setMinArgumentCount(pMethod->maxArgumentCount() - ocount);
			for (unsigned int j = 0; j < ocount; ++j)
			{
				log_abc("  Reading optional arg: %u", j);
				boost::uint32_t index = _stream->read_V32();
				boost::uint8_t kindof = _stream->read_u8();
				log_abc("   Index: %u Kindof: %u", index, kindof);
				as_value v;
				if (!pool_value(index, kindof, v))
					return false; // message done by pool_value
				pMethod->pushOptional(v);
			}
			log_abc("Done handling optional args.");
		}

		// If there are names present for the parameters, skip them.
		if (flags & METHOD_ARG_NAMES)
		{
			for (unsigned int j = 0; j < param_count; ++j)
			{
				_stream->skip_V32();
			}
		}
	} // End of method loop.
	return true;
}

/// Skip the metadata, which is useless to us.
bool
abc_block::skip_metadata()
{
	boost::uint32_t count = _stream->read_V32();
	for (unsigned int i = 0; i < count; ++i)
	{
		_stream->skip_V32(); // A name index.
		boost::uint32_t icount = _stream->read_V32();
		for (unsigned int j = 0; j < icount; ++j)
		{
			// key/values may not be stored together, but this still works.
			_stream->skip_V32();
			_stream->skip_V32();
		}
	}
	return true;
}

/// Load the instances from the block.
bool
abc_block::read_instances()
{
	boost::uint32_t count = _stream->read_V32();
	log_abc("There are %u instances.", count);
	_classes.resize(count);
	for (size_t i = 0; i < count; ++i) {
		asClass* pClass;
		//Read multiname index.
		boost::uint32_t index = _stream->read_V32();
		// 0 is allowed as a name, typically for the last entry.
		if (index >= _multinamePool.size()) {
			log_error(_("ABC: Out of bounds instance name."));
			return false;
		}
		// This must be a QName.
		if (!_multinamePool[index].isQName())
		{
			log_error(_("ABC: QName required for instance."));
			return false;
		}
		if (_multinamePool[index].getNamespace() == NULL)
		{
			log_error(_("ABC: No namespace to use for storing class."));
			return false;
		}
		pClass = locateClass(_multinamePool[index]);
		if (!pClass)
		{
			pClass = mCH->newClass();
			if (!_multinamePool[index].getNamespace()->addClass(
				_multinamePool[index].getABCName(), pClass))
			{
				log_error(_("Duplicate class registration."));
				return false;
			}
		}
		pClass->setDeclared();
		_classes[i] = pClass;
		boost::uint32_t super_index = _stream->read_V32();;
		if (super_index && super_index >= _multinamePool.size())
		{
			log_error(_("ABC: Bad super type."));
			return false;
		}
		if (!super_index)
		{
			pClass->setSuper(mTheObject);
		}
		else
		{
			asClass *pSuper = locateClass(_multinamePool[super_index]);
			if (!pSuper)
			{
				log_error(_("ABC: Super type not found (%s), faking."), 
					_stringTable->value(
                        _multinamePool[super_index].getABCName()));

				// While testing, we will add a fake type, rather than abort.
				pSuper = mCH->newClass();
				pSuper->setName(_multinamePool[super_index].getABCName());
				mCH->getGlobalNs()->addClass(
                        _multinamePool[super_index].getABCName(), pSuper);
				// return false;
			}

			if (pSuper->isFinal())
			{
				log_error(_("ABC: Can't extend a class which is final."));
				return false;
			}

			if (pSuper->isInterface())
			{
				log_error(_("ABC: Can't extend an interface type."));
				return false;
			}

			if (pSuper == pClass)
			{
				log_error(_("ABC: Class cannot be its own supertype."));
				return false;
			}
			pClass->setSuper(pSuper);
			pSuper->setInherited();
		}

		boost::uint8_t flags = _stream->read_u8();
		log_abc("Instance %u(%s) multiname index=%u name=%s super index=%u "
                "flags=%X", i, pClass, index, 
                _stringPool[_multinamePool[index].getABCName()],
                super_index, flags | 0x0);

		if (flags & INSTANCE_SEALED) pClass->setSealed();
		if (flags & INSTANCE_FINAL) pClass->setFinal();
		if (flags & INSTANCE_INTERFACE) pClass->setInterface();
		if ((flags & 7) == INSTANCE_DYNAMIC) pClass->setDynamic();

		if (flags & INSTANCE_PROTECTED_NS) {
			boost::uint32_t ns_index = _stream->read_V32();
			if (ns_index >= _namespacePool.size()) {
				log_error(_("ABC: Bad namespace for protected."));
				return false;
			}
			// Set the protected namespace's parent, if it exists.
			if (pClass->getSuper()->hasProtectedNs())
				_namespacePool[ns_index]->setParent(
                        pClass->getSuper()->getProtectedNs());
			pClass->setProtectedNs(_namespacePool[ns_index]);
		}

		// This is the list of interfaces which the instances has agreed to
		// implement. They must be interfaces, and they must exist.
		boost::uint32_t intcount = _stream->read_V32();
		log_abc("This instance has %u interfaces.", intcount);
		for (size_t j = 0; j < intcount; ++j) {
			boost::uint32_t i_index = _stream->read_V32();
			log_abc("Interface %u has multiname index=%u", i, i_index);
			// 0 is allowed as an interface, typically for the last one.
			if (i_index >= _multinamePool.size()) {
				log_error(_("ABC: Bad name for interface."));
				return false;
			}
			asClass *pInterface = locateClass(_multinamePool[i_index]);
			// These may be undefined still, so don't check interface just yet.
			if (0) //!pInterface || !pInterface->isInterface())
			{
				log_error(_("ABC: Can't implement a non-interface type."));
				return false;
			}
			pClass->pushInterface(pInterface);
		}

		// The next thing should be the constructor.
		// TODO: What does this mean exactly? How does it differ from the one in
		// the class info block?
		boost::uint32_t offset = _stream->read_V32();
		log_abc("Moffset: %u", offset);
		if (offset >= _methods.size()) {
			log_error(_("ABC: Out of bounds method for initializer."));
			return false;
		}
		// Don't validate for previous owner.
		pClass->setConstructor(_methods[offset]);

		/*	Calling the asMethod::setOwner always results in a segmentation fault, 
		since it tries to modify asMethod.mPrototype, which is never
		initialized.  The parser seems to work ok without this call.*/
//		_methods[offset]->setOwner(pClass);

		// Next come the 'traits' of the instance. (The members.)
		boost::uint32_t tcount = _stream->read_V32();
		log_abc("Trait count: %u", tcount);
		for (unsigned int j = 0; j < tcount; ++j)
		{
			Trait &aTrait = newTrait();
			aTrait.set_target(pClass, false);
			if (!aTrait.read(_stream, this))
				return false;
		}
	} // End of instances loop.
	return true;
}

/// Read the class data
bool
abc_block::read_classes()
{
	// Count was found in read_instances().
	log_abc("Begin reading classes.");
	boost::uint32_t count = _classes.size();
	log_abc("There are %u classes.", count);
	
    for (size_t i = 0; i < count; ++i) {
		asClass* pClass = _classes[i];
		boost::uint32_t offset = _stream->read_V32();
		log_abc("Class %u(%s) static constructor index=%u", i, pClass, offset);

        if (offset >= _methods.size()) {
			log_error(_("ABC: Out of bound static constructor for class."));
			return false;
		}

		// Don't validate for previous owner.
		pClass->setStaticConstructor(_methods[offset]);

		/*	Calling the asMethod::setOwner always results in a segmentation fault, 
		since it tries to modify asMethod.mPrototype, which is never
		initialized.  The parser seems to work ok without this call.*/
//		_methods[offset]->setOwner(pClass);
		
		boost::uint32_t tcount = _stream->read_V32();
		log_abc("This class has %u traits.", tcount);
		for (size_t j = 0; j < tcount; ++j) {
			Trait &aTrait = newTrait();
			aTrait.set_target(pClass, true);
			if (!(aTrait.read(_stream, this)))
				return false;
		}
	} 
	return true;
}

/// Read the scripts (global functions)
/// The final script is the entry point for the block.
bool
abc_block::read_scripts()
{
	log_abc("Begin reading scripts.");
	boost::uint32_t count = _stream->read_V32();
	log_abc("There are %u scripts.", count);
	_scripts.resize(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		asClass* pScript = mCH->newClass();
		_scripts[i] = pScript;

		boost::uint32_t offset = _stream->read_V32();
		log_abc("Reading script %u(%s) initializer method index=%u", i,
                pScript, offset);
		if (offset >= _methods.size())
		{
			log_error(_("ABC: Out of bounds method for script."));
			return false;
		}

		// Calling the asMethod::setOwner always results in a segmentation
        // fault, since it tries to modify asMethod.mPrototype, which is never
		// initialized.  The parser seems to work ok without this call.
		// Don't validate for previous owner.
//		_methods[offset]->setOwner(pScript);

		pScript->setConstructor(_methods[offset]);
		pScript->setSuper(mTheObject);

		boost::uint32_t tcount = _stream->read_V32();
		for (unsigned int j = 0; j < tcount; ++j)
		{
			
			Trait &aTrait = newTrait();
			aTrait.set_target(pScript, false);
			if (!(aTrait.read(_stream, this))) {
				return false;
            }
			log_abc("Trait: %u name: %s(%u) kind: %s value: %s ", j, 
                    _stringPool[aTrait._name], aTrait._name, aTrait._kind,
                    aTrait._value.to_string());

			pScript->_traits.push_back(aTrait);
		}
	} // end of scripts loop
	return true;
}

/// Read the method bodies and attach them to the methods.
bool
abc_block::read_method_bodies()
{
	boost::uint32_t count = _stream->read_V32();
	log_abc("There are %u method bodies.", count);
	for (unsigned int i = 0; i < count; ++i)
	{
		boost::uint32_t offset = _stream->read_V32();
		log_abc("Method body %u method offset=%u", i, offset);

		if (offset >= _methods.size()) {
			log_error(_("ABC: Out of bounds for method body."));
			return false;
		}

        asMethod& method = *_methods[offset];

		if (method.getBody()) {
			log_error(_("ABC: Only one body per method."));
			return false;
		}

		// Maximum stack size.
        method.setMaxStack(_stream->read_V32());
		
        // Maximum register size.
		method.setMaxRegisters(_stream->read_V32());
		
        // Scope depth.
		method.setScopeDepth(_stream->read_V32());
		
        // Max scope depth.
		method.setMaxScope(_stream->read_V32());
		
        // Code length
		boost::uint32_t clength = _stream->read_V32();
		method.setBodyLength(clength);

		// The code.
		//TODO: Clean this up.
		std::string body;
		_stream->read_string_with_length(clength, body);

		method.setBody(new CodeStream(body));
		
		boost::uint32_t ecount = _stream->read_V32();
		for (unsigned int j = 0; j < ecount; ++j)
		{
			asException *pExcept = mCH->newException();

			// Where the try block begins and ends.
			pExcept->setStart(_stream->read_V32());
			pExcept->setEnd(_stream->read_V32());

			// Where to go when the exception is activated.
			pExcept->setCatch(_stream->read_V32());

			// What types should be caught.
			boost::uint32_t catch_type = _stream->read_V32();
			if (catch_type >= _multinamePool.size()) {
				log_error(_("ABC: Out of bound type for exception."));
//				return false;
			}
			if (!catch_type) {
				pExcept->catchAny();
			}
			else
			{
				asClass *pType = locateClass(_multinamePool[catch_type]);
				if (!pType)
				{
					log_error(_("ABC: Unknown type of object to catch. (%s)"), 
						_stringTable->value(
                            _multinamePool[catch_type].getABCName()));

                    // return false;
					// Fake it, for now:
					pExcept->catchAny();
				}
				else {
					pExcept->setCatchType(pType);
				}
			}

			// A variable name for the catch type.
			// In version 46.15, no names.
			if (mVersion != ((46 << 16) | 15))
			{
				boost::uint32_t cvn = _stream->read_V32();
				if (cvn >= _multinamePool.size())
				{
					log_error(_("ABC: Out of bound name for caught exception."));
//					return false;
				}
				pExcept->setName(_multinamePool[cvn].getABCName());
				pExcept->setNamespace(_multinamePool[cvn].getNamespace());
			}
		} // end of exceptions

		boost::uint32_t tcount = _stream->read_V32();
		for (unsigned int j = 0; j < tcount; ++j)
		{
			Trait &aTrait = newTrait();
			aTrait.set_target(_methods[offset]);
			if (!aTrait.read(_stream, this)) {
                // TODO: 'method body activation traits'
				return false;
            }
			log_abc("Trait: %u name: %s kind: %s value: %s ", j, 
                    _stringPool[aTrait._name], aTrait._kind, 
                    aTrait._value.to_string());
		}
	} // end of bodies loop
	return true;
}

// Load up all of the data.
bool
abc_block::read(SWFStream& in)
{
    // This isn't very nice:
	_stream = &in;

	if (!read_version()) return false;
	if (!read_integer_constants()) return false;
	if (!read_unsigned_integer_constants()) return false;
	log_abc("Done reading unsigned integer constants.");
	
    if (!read_double_constants()) return false;
	log_abc("Done reading double constants.");
	
    if (!read_string_constants()) return false;
	log_abc("Done reading string constants.");
	
    if (!read_namespaces()) return false;
	log_abc("Done reading namespaces.");
	
    if (!read_namespace_sets()) return false;
	log_abc("Done reading namespace sets.");
	
    if (!read_multinames()) return false;
	log_abc("Done reading multinames.");
	
    if (!read_method_infos()) return false;
	log_abc("Done reading method infos.");
	
    if (!skip_metadata()) return false;
	log_abc("Done reading metadata.");
	
    if (!read_instances()) return false;
	log_abc("Done reading instances.");
	
    if (!read_classes()) return false;
	log_abc("Done reading classes.");
	
    if (!read_scripts()) return false;
	log_abc("Done reading scripts.");
	
    if (!read_method_bodies()) return false;
	log_abc("Done reading stuff.");

	for (size_t i=0; i < _methods.size(); ++i) {
		log_abc("Method %d body:", i);
		IF_VERBOSE_PARSE(_methods[i]->print_body());
	}
/*	The loop below causes a segmentation fault, because it tries to modify 
	asMethod.mPrototype, which is never initialized.  The parser seems 
	to work ok without this call.*/
/*	std::vector<Trait*>::iterator i = mTraits.begin();
	for ( ; i != mTraits.end(); ++i)
	{
		if (!(*i)->finalize(this))
			return false;
	}
	mTraits.clear();
*/
	//mCH->dump();
	return true;
}

asClass*
abc_block::locateClass(const std::string& className)
{

    // TODO: this is rubbish and should be done properly. Machine.cpp also
    // has completeName for runtime names, so should probably use common
    // code (construction of asName?).

    const std::string::size_type pos = className.rfind(".");
    
    asName a;

    if (pos != std::string::npos) {
        const std::string& nsstr = className.substr(0, pos);
        const std::string& clstr = className.substr(pos + 1);
        std::vector<std::string>::iterator it = 
            std::find(_stringPool.begin(), _stringPool.end(), clstr);

        if (it == _stringPool.end()) return 0;
        for (std::vector<asNamespace*>::iterator i = _namespacePool.begin();
                i != _namespacePool.end(); ++i) {
            log_abc("Looking in ns: %s", _stringPool[(*i)->getAbcURI()]);
            if (_stringPool[(*i)->getAbcURI()] == nsstr) {
                a.setNamespace(*i);
                break;
            }
        }
        a.setABCName(it - _stringPool.begin());
    }
    else {
        std::vector<std::string>::iterator it = 
            std::find(_stringPool.begin(), _stringPool.end(), className);
        if (it == _stringPool.end()) return 0;
        a.setABCName(it - _stringPool.begin());
    }
    
    return locateClass(a);

}

void
abc_block::update_global_name(unsigned int multiname_index)
{
	
	asName* multiname = &_multinamePool[multiname_index];
	string_table::key new_key = 
        _stringTable->find(_stringPool[multiname->getABCName()], false);
	multiname->setGlobalName(new_key);	
}

} /* namespace gnash */

