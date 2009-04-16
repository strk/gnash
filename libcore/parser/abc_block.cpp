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

namespace gnash {

namespace abc {

bool
Trait::finalize(abc_block *pBlock, asClass *pClass, bool do_static)
{
	log_abc("In finalize class name=%s trait kind=0x%X", 
            pBlock->_stringPool[pClass->getName()], _kind | 0x0);
	switch (_kind)
	{
	case KIND_SLOT:
	case KIND_CONST:
	{
		// Validate the type.
		asClass *pType;
		if (_typeIndex) {
			log_abc("Trait type is %s", 
                    pBlock->_stringPool[pBlock->_multinamePool[_typeIndex].getABCName()]);
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
			_value = as_value((as_object*)0); // NULL value, right ?
		}
		log_abc("Adding property=%s with value=%s slot=%u", pBlock->_stringPool[_name], _value.toDebugString(), _slotID);
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
		log_abc("Finalize getter not implemented.");
		break;
		pClass->addGetter(_name, _namespace, _method, do_static);
		break;
	}
	case KIND_SETTER:
	{
		log_abc("Finalize setter not implemented.");
		break;
		pClass->addSetter(_name, _namespace, _method, do_static);
		break;
	}
	case KIND_CLASS:
	{
		log_abc("Finalize class not implemented.");
		break;
		pClass->addMemberClass(_name, _namespace, _slotID, 
			pBlock->_classes[_classInfoIndex], do_static);
		break;
	}
	case KIND_FUNCTION:
	{
		log_abc("Finalize function not implemented.");
		break;
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

	log_abc("Trai name=%s Trait kind is 0x%X", pBlock->_stringPool[multiname.getABCName()], kind | 0x0);
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
		if (offset >= pBlock->_methods.size())
		{
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
		if (offset >= pBlock->_methods.size())
		{
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

} // abc

using namespace abc;

abc_block::abc_block()
    :
    _stringTable(&VM::get().getStringTable())
{
	mCH = VM::get().getClassHierarchy();
	// TODO: Make this the real 'Object' prototype.
	mCH->getGlobalNs()->stubPrototype(NSV::CLASS_OBJECT);
	mTheObject = mCH->getGlobalNs()->getClass(NSV::CLASS_OBJECT);
}

void
abc_block::prepare(Machine* mach)
{
    std::for_each(_classes.begin(), _classes.end(),
            std::mem_fun(&asClass::initPrototype));

    std::for_each(_scripts.begin(), _scripts.end(),
            std::mem_fun(&asClass::initPrototype));
 
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
	log_abc("Multiname: %s ABCName set to %u global name set to %u", name, n->getABCName(), n->getGlobalName());
}

void
abc_block::setNamespaceURI(asNamespace *ns, string_table::key ABCName)
{
	
	ns->setAbcURI(ABCName);
	std::string name = _stringPool[ABCName];
	string_table::key global_key = _stringTable->find(name, false);
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
	// One last chance: Look globally.
	found = mCH->getGlobalNs()->getClass(m.getABCName());
	if (found)
		return found;

	// Fake it here for a while.
	if (m.getNamespace())
	{
		m.getNamespace()->stubPrototype(m.getABCName());
		found = m.getNamespace()->getClass(m.getABCName());
		return found;
	}
	else
	{
		// Fake in global.
		mCH->getGlobalNs()->stubPrototype(m.getABCName());
		found = mCH->getGlobalNs()->getClass(m.getABCName());
		return found;
	}
	return NULL;
}

/// Read the ActionBlock version number.
bool
abc_block::read_version()
{
	// Minor version, major version.
	mVersion = (mS->read_u16()) | (mS->read_u16() << 16);
	log_error(_("Abc Version: %d.%d"), (mVersion & 0xFFFF0000) >> 16, 
		(mVersion & 0x0000FFFF));
	return true;
}

/// Read the pool of integer constants.
bool
abc_block::read_integer_constants()
{
	// count overestimates by 1.
	boost::uint32_t count = mS->read_V32();
	_integerPool.resize(count);
	if (count)
		_integerPool[0] = 0;
	for (unsigned int i = 1; i < count; ++i)
	{
		_integerPool[i] = static_cast<boost::int32_t> (mS->read_V32());
	}
	return true;
}

/// Read the pool of unsigned integer constants.
bool
abc_block::read_unsigned_integer_constants()
{
	// count overestimates by 1.
	boost::uint32_t count = mS->read_V32();
	_uIntegerPool.resize(count);
	if (count)
		_uIntegerPool[0] = 0;
	for (unsigned int i = 1; i < count; ++i)
	{
		_uIntegerPool[i] = mS->read_V32();
	}
	return true;
}

/// Read the pool of 64-bit double constants.
bool
abc_block::read_double_constants()
{
	boost::uint32_t count = mS->read_V32();
	_doublePool.resize(count);
	if (count)
		_doublePool[0] = 0.0;
	for (unsigned int i = 1; i < count; ++i)
	{
		_doublePool[i] = mS->read_d64();
		log_abc("Double %u=%lf", i, _doublePool[i]);
	}
	return true;
}

/// Read the pool of string constants.
bool
abc_block::read_string_constants()
{
	log_abc("Begin reading string constants.");
	boost::uint32_t count = mS->read_V32();
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
		boost::uint32_t length = mS->read_V32();
		mS->read_string_with_length(length, _stringPool[i]);
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
	boost::uint32_t count = mS->read_V32();
	log_abc("There are %u namespaces.", count);
	_namespacePool.resize(count);
	if (count)
	{
		_namespacePool[0] = mCH->getGlobalNs();
	}
	for (unsigned int i = 1; i < count; ++i)
	{
		boost::uint8_t kind = mS->read_u8();
		boost::uint32_t nameIndex = mS->read_V32();
		log_abc("Namespace %u kind=0x%X index=%u name=%s", i, kind | 0x0, nameIndex, _stringPool[nameIndex]);

		if (nameIndex >= _stringPool.size())
		{
			log_error(_("ABC: Out of bounds string given for namespace."));
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
			if (n == NULL)
				n = mCH->addNamespace(nameIndex);
			_namespacePool[i] = n;
		}
		if (kind == PROTECTED_NS)
		{
			_namespacePool[i]->setProtected();
		}
		setNamespaceURI(_namespacePool[i], nameIndex);
	}
	return true;
}

/// Read the set of sets of namespaces.
bool
abc_block::read_namespace_sets()
{
	boost::uint32_t count = mS->read_V32();
	_namespaceSetPool.resize(count);
	if (count)
	{
		_namespaceSetPool[0].resize(0);
	}
	for (unsigned int i = 1; i < count; ++i)
	{
		boost::uint32_t icount = mS->read_V32();
		_namespaceSetPool[i].resize(icount);
		for (unsigned int j = 0; j < icount; ++j)
		{
			boost::uint32_t selection = mS->read_V32();
			if (!selection || selection >= _namespacePool.size())
			{
				log_error(_("ABC: Out of bounds namespace for namespace set."));
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
	boost::uint32_t count = mS->read_V32();
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
        asName::Kind kind = static_cast<asName::Kind>(mS->read_u8());
		boost::uint32_t ns = 0;
		boost::uint32_t name = 0;
		boost::uint32_t nsset = 0;

		log_abc("Multiname %u has kind=0x%X", i, kind | 0x0);

		// Read, but don't upper validate until after the switch.
		switch (kind)
		{
            case asName::KIND_Qname:
            case asName::KIND_QnameA:
                ns = mS->read_V32();
                check_multiname_namespace(ns);
                name = mS->read_V32();
                check_multiname_name(name);
                log_abc("\tnamespace_index=%u name_index=%u name=%s",
                        ns, name, _stringPool[name]);
                break;
            
            case asName::KIND_RTQname:
            case asName::KIND_RTQnameA:
                name = mS->read_V32();
                check_multiname_name(name);
                break;
            
            case asName::KIND_RTQnameL:
            case asName::KIND_RTQnameLA:
                break;
            
            case asName::KIND_Multiname:
            case asName::KIND_MultinameA:
                name = mS->read_V32();
                check_multiname_name(name);
                nsset = mS->read_V32();
                check_multiname_namespaceset(nsset);
                break;
            
            case asName::KIND_MultinameL:
            case asName::KIND_MultinameLA:
                nsset = mS->read_V32();
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

	boost::uint32_t count = mS->read_V32();
    log_abc("Method count: %u", count);

	_methods.resize(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		log_abc(" Reading method %u", i);
		asMethod *pMethod = mCH->newMethod();
		pMethod->setMethodID(i);
//		log_abc("Min arg count: %d max: %d", pMethod->minArgumentCount(), pMethod->maxArgumentCount());
		_methods[i] = pMethod;
		boost::uint32_t param_count = mS->read_V32();
		boost::uint32_t return_type = mS->read_V32();

		log_abc("  Param count: %u return type(index): %s(%u)", param_count, _stringPool[_multinamePool[return_type].getABCName()], return_type);
		pMethod->setMinArgumentCount(param_count);
		pMethod->setMaxArgumentCount(param_count);

		if (return_type >= _multinamePool.size())
		{
			log_error(_("ABC: Out of bounds return type for method info."));
			return false;
		}
		asClass *rtClass = locateClass(_multinamePool[return_type]);
		if (!rtClass)
		{
			log_error(_("ABC: Unknown return type."));
			return false;
		}

		pMethod->setReturnType(rtClass);

		for (unsigned int j = 0; j < param_count; ++j)
		{
			log_abc("  Reading parameter %u", j);
			// The parameter type.
			boost::uint32_t ptype = mS->read_V32();
			log_abc("   Parameter type(index): %s(%u)", _stringPool[_multinamePool[ptype].getABCName()], ptype);
			if (ptype >= _multinamePool.size())
			{
				log_error(_("ABC: Out of bounds parameter type in method."));
				return false;
			}
			asClass *param_type = locateClass(_multinamePool[ptype]);
//			log_abc("Done creating asClass object.");
			if (!param_type)
			{
				log_error((_("ABC: Unknown parameter type.")));
				return false;
			}
//			log_abc("Trying to add argument to method.");
			pMethod->pushArgument(param_type);
//			log_abc("Done adding argument to method object.");
		}
//		log_abc("End loop j.");
		// A skippable name index.
//		mS->skip_V32();
		boost::uint32_t method_name = mS->read_V32();
		log_abc(  "Method name=%s %d", _stringPool[method_name], method_name);
		boost::uint8_t flags = mS->read_u8();
		log_abc("  Flags: %X", flags | 0x0);
//		log_abc("Check if flags and optional args.");
		// If there are default parameters, read them now.
		// Runtime will do validation of whether or not these can actually
		// be assigned to the corresponding parameters.
		if (flags & METHOD_OPTIONAL_ARGS)
		{
//			log_abc("We have flags and optional args.");
			boost::uint32_t ocount = mS->read_V32();
			log_abc("  Optional args: %u", ocount);
			pMethod->setMinArgumentCount(pMethod->maxArgumentCount() - ocount);
			for (unsigned int j = 0; j < ocount; ++j)
			{
				log_abc("  Reading optional arg: %u", j);
				boost::uint32_t index = mS->read_V32();
				boost::uint8_t kindof = mS->read_u8();
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
				mS->skip_V32();
			}
		}
	} // End of method loop.
	return true;
}

/// Skip the metadata, which is useless to us.
bool
abc_block::skip_metadata()
{
	boost::uint32_t count = mS->read_V32();
	for (unsigned int i = 0; i < count; ++i)
	{
		mS->skip_V32(); // A name index.
		boost::uint32_t icount = mS->read_V32();
		for (unsigned int j = 0; j < icount; ++j)
		{
			// key/values may not be stored together, but this still works.
			mS->skip_V32();
			mS->skip_V32();
		}
	}
	return true;
}

/// Load the instances from the block.
bool
abc_block::read_instances()
{
	boost::uint32_t count = mS->read_V32();
	log_abc("There are %u instances.", count);
	_classes.resize(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		asClass *pClass;
		//Read multiname index.
		boost::uint32_t index = mS->read_V32();
		// 0 is allowed as a name, typically for the last entry.
		if (index >= _multinamePool.size())
		{
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
		boost::uint32_t super_index = mS->read_V32();;
		if (super_index && super_index >= _multinamePool.size())
		{
			log_error(_("ABC: Out of bounds super type."));
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
					_stringTable->value(_multinamePool[super_index].getABCName()));
				// While testing, we will add a fake type, rather than abort.
				pSuper = mCH->newClass();
				pSuper->setName(_multinamePool[super_index].getABCName());
				mCH->getGlobalNs()->addClass(_multinamePool[super_index].getABCName(), pSuper);
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

		boost::uint8_t flags = mS->read_u8();
		log_abc("Instance %u multiname index=%u name=%s super index=%u flags=%X", i, index, _stringPool[_multinamePool[index].getABCName()], super_index, flags | 0x0);

		if (flags & INSTANCE_SEALED)
			pClass->setSealed();
		if (flags & INSTANCE_FINAL)
			pClass->setFinal();
		if (flags & INSTANCE_INTERFACE)
			pClass->setInterface();
		if ((flags & 7) == INSTANCE_DYNAMIC)
			pClass->setDynamic();

		if (flags & INSTANCE_PROTECTED_NS) // Protected Namespace
		{
			boost::uint32_t ns_index = mS->read_V32();
			if (ns_index >= _namespacePool.size())
			{
				log_error(_("ABC: Out of bounds namespace for protected."));
				return false;
			}
			// Set the protected namespace's parent, if it exists.
			if (pClass->getSuper()->hasProtectedNs())
				_namespacePool[ns_index]->setParent(pClass->getSuper()->getProtectedNs());
			pClass->setProtectedNs(_namespacePool[ns_index]);
		}

		// This is the list of interfaces which the instances has agreed to
		// implement. They must be interfaces, and they must exist.
		boost::uint32_t intcount = mS->read_V32();
		log_abc("This instance has %u interfaces.", intcount);
		for (unsigned int j = 0; j < intcount; ++j)
		{
			boost::uint32_t i_index = mS->read_V32();
			log_abc("Interface %u has multiname index=%u", i, i_index);
			// 0 is allowed as an interface, typically for the last one.
			if (i_index >= _multinamePool.size())
			{
				log_error(_("ABC: Out of bounds name for interface."));
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
		boost::uint32_t offset = mS->read_V32();
		log_abc("Moffset: %u", offset);
		if (offset >= _methods.size())
		{
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
		boost::uint32_t tcount = mS->read_V32();
		log_abc("Trait count: %u", tcount);
		for (unsigned int j = 0; j < tcount; ++j)
		{
			Trait &aTrait = newTrait();
			aTrait.set_target(pClass, false);
			if (!aTrait.read(mS, this))
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
	for (unsigned int i = 0; i < count; ++i)
	{
		asClass *pClass = _classes[i];
		boost::uint32_t offset = mS->read_V32();
		log_abc("Class %u static constructor index=%u", i, offset);
		if (offset >= _methods.size())
		{
			log_error(_("ABC: Out of bound static constructor for class."));
			return false;
		}
		// Don't validate for previous owner.
		pClass->setStaticConstructor(_methods[offset]);

		/*	Calling the asMethod::setOwner always results in a segmentation fault, 
		since it tries to modify asMethod.mPrototype, which is never
		initialized.  The parser seems to work ok without this call.*/
//		_methods[offset]->setOwner(pClass);
		
		boost::uint32_t tcount = mS->read_V32();
		log_abc("This class has %u traits.", tcount);
		for (unsigned int j = 0; j < tcount; ++j)
		{
			Trait &aTrait = newTrait();
			aTrait.set_target(pClass, true);
			if (!(aTrait.read(mS, this)))
				return false;
		}
	} // end of classes loop
	return true;
}

/// Read the scripts (global functions)
/// The final script is the entry point for the block.
bool
abc_block::read_scripts()
{
	log_abc("Begin reading scripts.");
	boost::uint32_t count = mS->read_V32();
	log_abc("There are %u scripts.", count);
	_scripts.resize(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		asClass *pScript = mCH->newClass();
		_scripts[i] = pScript;

		boost::uint32_t offset = mS->read_V32();
		log_abc("Reading script %u initializer method index=%u", i, offset);
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

		boost::uint32_t tcount = mS->read_V32();
		for (unsigned int j = 0; j < tcount; ++j)
		{
			
			Trait &aTrait = newTrait();
			aTrait.set_target(pScript, false);
			if (!(aTrait.read(mS, this))) {
				return false;
            }
			log_abc("Trait: %u name: %s(%u) kind: %u value: %s ", j, 
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
	boost::uint32_t count = mS->read_V32();
	log_abc("There are %u method bodies.", count);
	for (unsigned int i = 0; i < count; ++i)
	{
		boost::uint32_t offset = mS->read_V32();
		log_abc("Method body %u method offset=%u", i, offset);
		if (offset >= _methods.size())
		{
			log_error(_("ABC: Out of bounds for method body."));
			return false;
		}
		if (_methods[offset]->getBody())
		{
			log_error(_("ABC: Only one body per method."));
			return false;
		}
		//TODO: Read values.

		// Maximum stack size.
		mS->skip_V32();
		// Maximum register size.
		_methods[offset]->setMaxRegisters(mS->read_V32());
		// Scope depth.
		mS->skip_V32();
		// Max scope depth.
		mS->skip_V32();
		// Code length
		boost::uint32_t clength = mS->read_V32();
		_methods[offset]->setBodyLength(clength);
		// The code.
		//TODO: Clean this up.
		std::string body;
		mS->read_string_with_length(clength, body);

		_methods[offset]->setBody(new CodeStream(body));
		
		boost::uint32_t ecount = mS->read_V32();
		for (unsigned int j = 0; j < ecount; ++j)
		{
			asException *pExcept = mCH->newException();

			// Where the try block begins and ends.
			pExcept->setStart(mS->read_V32());
			pExcept->setEnd(mS->read_V32());

			// Where to go when the exception is activated.
			pExcept->setCatch(mS->read_V32());

			// What types should be caught.
			boost::uint32_t catch_type = mS->read_V32();
			if (catch_type >= _multinamePool.size())
			{
				log_error(_("ABC: Out of bound type for exception."));
//				return false;
			}
			if (!catch_type)
			{
				pExcept->catchAny();
			}
			else
			{
				asClass *pType = locateClass(_multinamePool[catch_type]);
				if (!pType)
				{
					log_error(_("ABC: Unknown type of object to catch. (%s)"), 
						_stringTable->value(_multinamePool[catch_type].getABCName()));
					// return false;
					// Fake it, for now:
					pExcept->catchAny();
				}
				else
				{
					pExcept->setCatchType(pType);
				}
			}

			// A variable name for the catch type.
			// In version 46.15, no names.
			if (mVersion != ((46 << 16) | 15))
			{
				boost::uint32_t cvn = mS->read_V32();
				if (cvn >= _multinamePool.size())
				{
					log_error(_("ABC: Out of bound name for caught exception."));
//					return false;
				}
				pExcept->setName(_multinamePool[cvn].getABCName());
				pExcept->setNamespace(_multinamePool[cvn].getNamespace());
			}
		} // end of exceptions

		boost::uint32_t tcount = mS->read_V32();
		for (unsigned int j = 0; j < tcount; ++j)
		{
			Trait &aTrait = newTrait();
			aTrait.set_target(_methods[offset]);
			if (!aTrait.read(mS, this)) {
                // TODO: 'method body activation traits'
				return false;
            }
			log_abc("Trait: %u name: %s kind: %u value: %s ", j, 
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
	mS = &in;

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

	for(unsigned int i=0;i<_methods.size();i++) {
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
	
	std::vector<asClass*>::iterator i = _classes.begin();
	for( ; i!=_classes.end(); ++i) {
		if (_stringPool[(*i)->getName()] == className) {
			return *i;
		}
	}	
	return NULL;
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

