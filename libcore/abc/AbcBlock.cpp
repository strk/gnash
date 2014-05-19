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

// The AS3 abc block format reader.
//

#include "gnashconfig.h"
#include "AbcBlock.h"
#include "SWFStream.h" // for use
#include "VM.h"
#include "log.h"
#include "ClassHierarchy.h"
#include "Class.h"
#include "namedStrings.h"
#include "CodeStream.h"
#include "action_buffer.h"
#include "Machine.h"
#include "Global_as.h"

#include <functional>

namespace gnash {

namespace abc {

bool
Trait::finalize(AbcBlock *block, abc::Class* script, bool do_static)
{
	log_abc("Finalize class %s (%s), trait kind: %s", 
            block->_stringTable->value(script->getName()), script, _kind);

	switch (_kind)
	{
        case KIND_SLOT:
        case KIND_CONST:
        {
            // Validate the type.
            abc::Class* type;
            if (_typeIndex) {
                log_abc("Trait type: %s", 
                    block->_stringPool[
                        block->_multinamePool[_typeIndex].getABCName()]);
                type = block->locateClass(block->_multinamePool[_typeIndex]);
            }
            else {
                type = block->mTheObject;
            }

            if (!type) {
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
                    block->_stringPool[_name], _value, _slotID);

            script->addValue(_globalName, _namespace, _slotID, type, 
                _value, _kind == KIND_CONST, do_static);
            break;
        }

        case KIND_METHOD:
            script->addMethod(_globalName, _namespace, _method, false);
            break;
        
        case KIND_GETTER:
            script->addGetter(_name, _namespace, _method, do_static);
            break;

        case KIND_SETTER:
            script->addSetter(_name, _namespace, _method, do_static);
            break;
        
        case KIND_CLASS:
            log_abc("Adding class %s, value %s, slot=%u",
                    block->_stringPool[_name], _value, _slotID);
            script->addMemberScript(_globalName, _namespace, _slotID, 
                block->_classes[_classInfoIndex], do_static);
            break;

        case KIND_FUNCTION:
            script->addSlotFunction(_name, _namespace, _slotID, _method,
                    do_static);
            break;

        default:
            return false;
	} 
	return true;
}

bool
Trait::finalize_mbody(AbcBlock *block, Method *pMethod)
{
	log_abc("Finalizing method trait: kind %s", _kind);
	switch (_kind)
	{
        case KIND_SLOT:
        case KIND_CONST:
        {
            // Validate the type.
            abc::Class *type;
            if (_typeIndex) {
                type = block->locateClass(block->_multinamePool[_typeIndex]);
            }
            else {
                type = block->mTheObject;
            }

            if (!type) {
                log_error(_("ABC: Finalizing trait yielded bad type for slot."));
                return false;
            }

            // The name has been validated in read.
            // TODO: Find a better way to initialize trait values.
            if (!_hasValue) {
                _value = as_value((as_object*)0); // NULL value, right ?
            }
            log_abc("Adding property=%s with value=%s slot=%u",
                    block->_stringPool[_name], _value.toDebugString(), _slotID);
            pMethod->addValue(_globalName, _namespace, _slotID, type, 
                _value, _kind == KIND_CONST);
            break;
        }
        case KIND_METHOD:
        {
            pMethod->addMethod(_name, _namespace, _method);
            break;
        }
        case KIND_GETTER:
        {
            pMethod->addGetter(_name, _namespace, _method);
            break;
        }
        case KIND_SETTER:
        {
            pMethod->addSetter(_name, _namespace, _method);
            break;
        }
        case KIND_CLASS:
        {
            pMethod->addMemberScript(_name, _namespace, _slotID, 
                block->_classes[_classInfoIndex]);
            break;
        }
        case KIND_FUNCTION:
        {
            pMethod->addSlotFunction(_name, _namespace, _slotID, _method);
            break;
        }
        default:
            // Not here -- validated already in read.
            return false;
	} 
	return true;
}

/// Read an AS3 'trait'
bool
Trait::read(SWFStream* in, AbcBlock *block)
{
	boost::uint32_t name = in->read_V32();
	if (name >= block->_multinamePool.size())
	{
		log_error(_("ABC: Bad name for trait."));
		return false;
	}
	if (!block->_multinamePool[name].isQName())
	{
		log_error(_("ABC: Trait name must be fully qualified."));
		return false;
	}
	MultiName multiname = block->_multinamePool[name];
	_name = block->_multinamePool[name].getABCName();
	_globalName = block->_multinamePool[name].getGlobalName();
	_namespace = block->_multinamePool[name].getNamespace();

	boost::uint8_t kind = in->read_u8();
	_kind = static_cast<Kind>(kind & 0x0F);

	log_abc("Trait name: %s, Trait kind: %s",
            block->_stringPool[multiname.getABCName()], _kind);

    switch (_kind)
	{
        case KIND_SLOT:
        case KIND_CONST:
        {
            _slotID = in->read_V32();
            _typeIndex = in->read_V32();
            boost::uint32_t vindex = in->read_V32();
            log_abc("Slot ID=%u Type=%s Pool index=%u", _slotID,
                    block->_stringPool[
                    block->_multinamePool[_typeIndex].getABCName()], vindex);
            
            if (vindex) {
                const AbcBlock::PoolConstant c =
                    static_cast<AbcBlock::PoolConstant>(in->read_u8());

                if (!block->pool_value(vindex, c, _value))
                    return false; // Message done by pool_value
                _hasValue = true;
            }
            else _hasValue = false;
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
            if (offset >= block->_methods.size()) {
                log_error(_("Bad method id in trait."));
                return false;
            }
            _method = block->_methods[offset];
            break;
        }
        case KIND_CLASS:
        {
            _slotID = in->read_V32();
            _classInfoIndex = in->read_V32();
            log_abc("Slot id: %u Class index: %u Class Name: %s", _slotID, 
                _classInfoIndex, 
                block->_stringTable->value(
                    block->_classes[_classInfoIndex]->getName()));

            if (_classInfoIndex >= block->_classes.size()) {
                log_error(_("Bad Class id in trait."));
                return false;
            }
            break;
        }
        case KIND_FUNCTION:
        {
            _slotID = in->read_V32();
            boost::uint32_t offset = in->read_V32();
            if (offset >= block->_methods.size()) {
                log_error(_("Bad method id in trait."));
                return false;
            }
            _method = block->_methods[offset];
            break;
        }
        default:
        {
            log_error(_("ABC: Unknown type of trait."));
        }
	}

	// Ignore the metadata, but it must be read to know how to ignore it.
	if ((kind >> 4) & 0x04) {
		boost::uint32_t mcount = in->read_V32();
		for (size_t i = 0; i < mcount; ++i) {
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

AbcBlock::AbcBlock()
    :
    _stringTable(&VM::get().getStringTable())
{
	mCH = &VM::get().getMachine()->global()->classHierarchy();
	// TODO: Make this the real 'Object' prototype.
	mCH->getGlobalNs()->stubPrototype(*mCH, NSV::CLASS_OBJECT);
	mTheObject = mCH->getGlobalNs()->getScript(NSV::CLASS_OBJECT);
}

void
AbcBlock::prepare(Machine* mach)
{

    std::for_each(_classes.begin(), _classes.end(),
            std::mem_fun(&abc::Class::initPrototype));
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
            std::bind(&abc::Class::setPrototype, _1, mach->global()));
 
    std::for_each(_methods.begin(), _methods.end(),
            std::bind(&Method::initPrototype, _1, mach));
    
    // TODO: Remove this, initialize traits only when needed; possibly 
    // consruct them on parsing without the need for a finalize method.
    std::for_each(_methods.begin(), _methods.end(),
            std::bind(&abc::Method::initTraits, _1, *this));
    std::for_each(_classes.begin(), _classes.end(),
            std::bind(&abc::Class::initTraits, _1, *this));
    std::for_each(_scripts.begin(), _scripts.end(),
            std::bind(&abc::Class::initTraits, _1, *this));

}

void
AbcBlock::check_multiname_name(boost::uint32_t name)
{

	if (name >= _stringPool.size()) {
		throw ParserException("ABC: Out of bounds string for Multiname.");
	}
}

void
AbcBlock::check_multiname_namespace(boost::uint32_t ns)
{
	if (ns >= _namespacePool.size()) {
		throw ParserException("ABC: Out of bounds namespace for Multiname.");
	}
}

void
AbcBlock::check_multiname_namespaceset(boost::uint32_t nsset)
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
AbcBlock::setMultinameNames(MultiName *n, abc::URI ABCName)
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
AbcBlock::setNamespaceURI(Namespace *ns, abc::URI ABCName)
{
	std::string name = _stringPool[ABCName];
	string_table::key global_key = _stringTable->find(name);
	ns->setURI(global_key);
	log_abc("Namespace: %s AbcURI=%u URI=%u.", name, ABCName, global_key);
}

abc::Class*
AbcBlock::locateClass(const std::string& className)
{

    const std::string::size_type pos = className.rfind(".");
    
    MultiName a;

    const std::string& nsstr = (pos != std::string::npos) ? 
        className.substr(0, pos) : "";

    const std::string& clstr = (pos != std::string::npos) ? 
        className.substr(pos + 1) : className;
    
    a.setGlobalName(_stringTable->find(clstr));

    for (std::vector<Namespace*>::iterator i = _namespacePool.begin();
            i != _namespacePool.end(); ++i) {
        
        const size_t key = (*i)->getURI();

        if (key == _stringTable->find(nsstr)) {
            a.setNamespace(*i);
            break;
        }
    }
    
    return locateClass(a);

}

abc::Class*
AbcBlock::locateClass(MultiName& m)
{
	abc::Class* found = 0;

	if (m.getNamespace())
	{
		found = m.getNamespace()->getScript(m.getGlobalName());
		if (found) return found;
	}
	if (m.namespaceSet() && !m.namespaceSet()->empty())
	{
		std::vector<Namespace*>::const_iterator i;
		for (i = m.namespaceSet()->begin(); i != m.namespaceSet()->end(); ++i) {

			found = (*i)->getScript(m.getGlobalName());
			if (found) return found;
		}
	}

    log_abc("Could not locate class in ABC block resources!");

    return 0;

}

/// Read the ActionBlock version number.
bool
AbcBlock::read_version()
{
	// Minor version, major version.
	mVersion = (_stream->read_u16()) | (_stream->read_u16() << 16);
	log_debug(_("Abc Version: %d.%d"), (mVersion & 0xFFFF0000) >> 16, 
		(mVersion & 0x0000FFFF));
	return true;
}

/// Read the pool of integer constants.
bool
AbcBlock::read_integer_constants()
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
AbcBlock::read_unsigned_integer_constants()
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
AbcBlock::read_double_constants()
{
	boost::uint32_t count = _stream->read_V32();
	_doublePool.resize(count);
	if (count) _doublePool[0] = 0.0;

	for (size_t i = 1; i < count; ++i)
	{
		_doublePool[i] = _stream->read_d64();
		log_abc("Double %u=%lf", i, _doublePool[i]);
	}
	return true;
}

/// Read the pool of string constants.
bool
AbcBlock::read_string_constants()
{
	log_abc("Begin reading string constants.");
	boost::uint32_t count = _stream->read_V32();
	log_abc("There are %u string constants.", count);
	_stringPool.resize(count);

    if (count) {
		_stringPool[0] = "";
	}

    for (size_t i = 1; i < count; ++i) {
		boost::uint32_t length = _stream->read_V32();
		_stream->read_string_with_length(length, _stringPool[i]);
		log_abc("Adding string constant to string pool: index=%u %s",
                i, _stringPool[i]);
	}
	return true;
}

/// Read the pool of namespaces
/// Any two namespaces with the same uri here are the same namespace, 
/// excepting private namespaces.
bool
AbcBlock::read_namespaces()
{	
	log_abc("Begin reading namespaces.");
	boost::uint32_t count = _stream->read_V32();
	log_abc("There are %u namespaces.", count);
	_namespacePool.resize(count);
	if (count) {
		_namespacePool[0] = mCH->getGlobalNs();
	}

	for (size_t i = 1; i < count; ++i)
	{
		NamespaceConstant kind =
            static_cast<NamespaceConstant>(_stream->read_u8());

		boost::uint32_t nameIndex = _stream->read_V32();
		log_abc("Namespace %u: %s, index %u, name %s", i, kind,
                nameIndex, _stringPool[nameIndex]);

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
            string_table::key gn = _stringTable->find(_stringPool[nameIndex]);
			Namespace *n = mCH->findNamespace(gn);
			if (!n) n = mCH->addNamespace(gn);
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
AbcBlock::read_namespace_sets()
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
AbcBlock::read_multinames()
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
        MultiName::Kind kind = static_cast<MultiName::Kind>(_stream->read_u8());
		boost::uint32_t ns = 0;
		boost::uint32_t name = 0;
		boost::uint32_t nsset = 0;

		log_abc("Multiname %u has kind %s", i, static_cast<int>(kind));

		// Read, but don't upper validate until after the switch.
		switch (kind)
		{
            case MultiName::KIND_Qname:
            case MultiName::KIND_QnameA:
                ns = _stream->read_V32();
                check_multiname_namespace(ns);
                name = _stream->read_V32();
                check_multiname_name(name);
                log_abc("\tnamespace_index=%u name_index=%u name=%s",
                        ns, name, _stringPool[name]);
                break;
            
            case MultiName::KIND_RTQname:
            case MultiName::KIND_RTQnameA:
                name = _stream->read_V32();
                check_multiname_name(name);
                break;
            
            case MultiName::KIND_RTQnameL:
            case MultiName::KIND_RTQnameLA:
                break;
            
            case MultiName::KIND_Multiname:
            case MultiName::KIND_MultinameA:
                name = _stream->read_V32();
                check_multiname_name(name);
                nsset = _stream->read_V32();
                check_multiname_namespaceset(nsset);
                break;
            
            case MultiName::KIND_MultinameL:
            case MultiName::KIND_MultinameLA:
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
AbcBlock::pool_value(boost::uint32_t index, PoolConstant type, as_value &v)
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
		v.set_double(_integerPool[index]);
		break;
	}
	case POOL_UINTEGER:
	{
		if (index >= _uIntegerPool.size())
		{
			log_error(_("Action Block: Bad index in optional argument."));
			return false;
		}
		v.set_double(_uIntegerPool[index]);
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
AbcBlock::read_method_infos()
{
	log_abc("Begin read_method_infos.");

	boost::uint32_t count = _stream->read_V32();
    log_abc("Method count: %u", count);

	_methods.resize(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		log_abc(" Reading method %u", i);
		Method *pMethod = mCH->newMethod();
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
            abc::Class* rtScript = locateClass(_multinamePool[return_type]);
            if (!rtScript) {
                log_abc(_("ABC: Unknown return type."));
            }

            pMethod->setReturnType(rtScript);
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
                abc::Class* param_type = locateClass(_multinamePool[ptype]);

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
		if (flags & METHOD_OPTIONAL_ARGS) {
			
            boost::uint32_t ocount = _stream->read_V32();
			log_abc("  Optional args: %u", ocount);
			pMethod->setMinArgumentCount(pMethod->maxArgumentCount() - ocount);
			
            for (unsigned int j = 0; j < ocount; ++j) {
				log_abc("  Reading optional arg: %u", j);
				boost::uint32_t index = _stream->read_V32();
				
                PoolConstant kindof =
                    static_cast<PoolConstant>(_stream->read_u8());

				log_abc("   Index: %u Kindof: %u", index, kindof);
				as_value v;
				if (!pool_value(index, kindof, v)) {
					return false; 
                }
				pMethod->pushOptional(v);
			}
			log_abc("Done handling optional args.");
		}

        if (flags & METHOD_ACTIVATION) {
            log_abc("Method needs activation");
            pMethod->setNeedsActivation();
        }

		// If there are names present for the parameters, skip them.
		if (flags & METHOD_ARG_NAMES) {
			for (size_t j = 0; j < param_count; ++j) {
				_stream->skip_V32();
			}
		}
	} // End of method loop.
	return true;
}

/// Skip the metadata, which is useless to us.
bool
AbcBlock::skip_metadata()
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
AbcBlock::read_instances()
{
	boost::uint32_t count = _stream->read_V32();
	log_abc("There are %u instances.", count);
	_classes.resize(count);
	for (size_t i = 0; i < count; ++i) {
		//Read multiname index.
		boost::uint32_t index = _stream->read_V32();
		// 0 is allowed as a name, typically for the last entry.
		if (index >= _multinamePool.size()) {
			log_error(_("ABC: Out of bounds instance name."));
			return false;
		}
        
        MultiName& multiname = _multinamePool[index];
		
        // This must be a QName.
		if (!multiname.isQName()) {
			log_error(_("ABC: QName required for instance."));
			return false;
		}
		
        if (!multiname.getNamespace()) {
			log_error(_("ABC: No namespace to use for storing class."));
			return false;
		}

        abc::Class* cl = locateClass(multiname);
		
        if (!cl) {

            const string_table::key className = multiname.getGlobalName();

			cl = mCH->newClass();
            cl->setName(className);

			if (!multiname.getNamespace()->addScript(className, cl)) {

				log_error(_("Duplicate class registration."));
				return false;
			}
            log_debug("Adding class %s (%s) to namespace %s",
                    _stringTable->value(multiname.getGlobalName()),
                    _stringPool[multiname.getABCName()],
                    _stringTable->value(multiname.getNamespace()->getURI()));
            log_debug("Namespace now:");
            multiname.getNamespace()->dump(*_stringTable);

		}
		cl->setDeclared();
		_classes[i] = cl;
		boost::uint32_t super_index = _stream->read_V32();

		if (super_index && super_index >= _multinamePool.size()) {
			log_error(_("ABC: Bad super type."));
			return false;
		}

		if (!super_index) {
			cl->setSuper(mTheObject);
		}
		else {
			abc::Class *pSuper = locateClass(_multinamePool[super_index]);
			if (!pSuper)
			{
				log_error(_("ABC: Super type not found (%s)"), 
					_stringPool[_multinamePool[super_index].getABCName()]);
			    return false;
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

			if (pSuper == cl)
			{
				log_error(_("ABC: Class cannot be its own supertype."));
				return false;
			}
			cl->setSuper(pSuper);
			pSuper->setInherited();
		}

		boost::uint8_t flags = _stream->read_u8();
		log_abc("Instance %u(%s) multiname index=%u name=%s super index=%u "
                "flags=%X", i, cl, index, 
                _stringPool[_multinamePool[index].getABCName()],
                super_index, flags | 0x0);

		if (flags & INSTANCE_SEALED) cl->setSealed();
		if (flags & INSTANCE_FINAL) cl->setFinal();
		if (flags & INSTANCE_INTERFACE) cl->setInterface();
		if ((flags & 7) == INSTANCE_DYNAMIC) cl->setDynamic();

		if (flags & INSTANCE_PROTECTED_NS) {
			boost::uint32_t ns_index = _stream->read_V32();
			if (ns_index >= _namespacePool.size()) {
				log_error(_("ABC: Bad namespace for protected."));
				return false;
			}
			// Set the protected namespace's parent, if it exists.
			if (cl->getSuper()->hasProtectedNs())
				_namespacePool[ns_index]->setParent(
                        cl->getSuper()->getProtectedNs());
			cl->setProtectedNs(_namespacePool[ns_index]);
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
			abc::Class *pInterface = locateClass(_multinamePool[i_index]);
			// These may be undefined still, so don't check interface just yet.
			if (0) //!pInterface || !pInterface->isInterface())
			{
				log_error(_("ABC: Can't implement a non-interface type."));
				return false;
			}
			cl->pushInterface(pInterface);
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
		cl->setConstructor(_methods[offset]);

		// Next come the 'traits' of the instance. (The members.)
		boost::uint32_t tcount = _stream->read_V32();
		log_abc("Trait count: %u", tcount);
		for (unsigned int j = 0; j < tcount; ++j)
		{
			Trait t;
			t.set_target(cl, false);
			if (!t.read(_stream, this)) return false;
            cl->addInstanceTrait(t);
		}
	} 
	return true;
}

/// Read the class data
bool
AbcBlock::read_classes()
{
	// Count was found in read_instances().
	log_abc("Begin reading classes.");
	boost::uint32_t count = _classes.size();
	log_abc("There are %u classes.", count);
	
    for (size_t i = 0; i < count; ++i) {
		abc::Class* cl = _classes[i];
		boost::uint32_t offset = _stream->read_V32();
		log_abc("Class %u(%s) static constructor index=%u", i, cl, offset);

        if (offset >= _methods.size()) {
			log_error(_("ABC: Out of bound static constructor for class."));
			return false;
		}

		// Don't validate for previous owner.
		cl->setStaticConstructor(_methods[offset]);

		boost::uint32_t tcount = _stream->read_V32();
		log_abc("This class has %u traits.", tcount);
		for (size_t j = 0; j < tcount; ++j) {
            Trait t;
			t.set_target(cl, true);
			if (!(t.read(_stream, this))) return false;
            cl->addStaticTrait(t);
		}
	} 
	return true;
}

/// Read the scripts (global functions)
/// The final script is the entry point for the block.
bool
AbcBlock::read_scripts()
{
	log_abc("Begin reading scripts.");

	const boost::uint32_t scriptcount = _stream->read_V32();
	log_abc("There are %u scripts.", scriptcount);

	_scripts.resize(scriptcount);
	for (size_t i = 0; i < scriptcount; ++i) {

		abc::Class* script = mCH->newClass();
		_scripts[i] = script;

		boost::uint32_t offset = _stream->read_V32();
		log_abc("Reading script %u(%s) initializer method index=%u", i,
                script, offset);
		if (offset >= _methods.size()) {
			log_error(_("ABC: Out of bounds method for script."));
			return false;
		}

		script->setConstructor(_methods[offset]);

		const boost::uint32_t tcount = _stream->read_V32();
		for (size_t j = 0; j < tcount; ++j) {
			
            Trait t;
			t.set_target(script, false);
			if (!(t.read(_stream, this))) {
				return false;
            }
			log_abc("Trait: %u name: %s(%u) kind: %s value: %s ", j, 
                    _stringPool[t._name], t._name, t._kind, t._value);

            // TODO: this should not use Class!
			script->addStaticTrait(t);
		}
	} 
	return true;
}

/// Read the method bodies and attach them to the methods.
bool
AbcBlock::read_method_bodies()
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

        Method& method = *_methods[offset];

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
		
        // Exception count and exceptions
        
        // Note: catch type and variable name are documented to be
        // indices in the string pool, but they are in fact indices
        // in the multiname pool.
		const boost::uint32_t ecount = _stream->read_V32();
		for (unsigned int j = 0; j < ecount; ++j) {
			asException *ex = mCH->newException();

			// Where the try block begins and ends.
			ex->setStart(_stream->read_V32());
			ex->setEnd(_stream->read_V32());

			// Where to go when the exception is activated.
			ex->setCatch(_stream->read_V32());

			// What types should be caught.
			boost::uint32_t catch_type = _stream->read_V32();
			if (catch_type >= _multinamePool.size()) {
				log_error(_("ABC: Out of bound type for exception."));
				return false;
			}
			if (!catch_type) {
				ex->catchAny();
			}
			else {
				abc::Class *type = locateClass(_multinamePool[catch_type]);
				if (!type) {

					log_error(_("ABC: Unknown type of object to catch. (%s)"), 
						_stringTable->value(
                            _multinamePool[catch_type].getABCName()));

                    // return false;
					// Fake it, for now:
					ex->catchAny();
				}
				else {
					ex->setCatchType(type);
				}
			}

			// A variable name for the catch type.
			// In version 46.15, no names.
			if (mVersion != ((46 << 16) | 15)) {
				boost::uint32_t cvn = _stream->read_V32();
				if (cvn >= _multinamePool.size()) {
					log_error(_("ABC: Out of bound name for caught "
                                "exception."));
					return false;
				}
				ex->setName(_multinamePool[cvn].getABCName());
				ex->setNamespace(_multinamePool[cvn].getNamespace());
			}
		} 

        // Traits
		boost::uint32_t tcount = _stream->read_V32();
		for (unsigned int j = 0; j < tcount; ++j)
		{
			Trait t;
			t.set_target(_methods[offset]);
			
            if (!t.read(_stream, this)) {
				return false;
            }

			log_abc("Activation trait: %u name: %s, kind: %s, value: %s ", j, 
                    _stringPool[t._name], t._kind, t._value);
            _methods[offset]->addTrait(t);
		}
	} 
	return true;
}

// Load up all of the data.
bool
AbcBlock::read(SWFStream& in)
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
	return true;
}


void
AbcBlock::update_global_name(unsigned int multiname_index)
{
	
	MultiName* multiname = &_multinamePool[multiname_index];
	string_table::key new_key = 
        _stringTable->find(_stringPool[multiname->getABCName()], false);
	multiname->setGlobalName(new_key);	
}

std::ostream&
operator<<(std::ostream& o, AbcBlock::NamespaceConstant c)
{
    switch (c)
    {
        case AbcBlock::PRIVATE_NS:
            return o << "private namespace";
        case AbcBlock::CONSTANT_NS:
            return o << "constant namespace";
        case AbcBlock::PACKAGE_NS:
            return o << "package namespace";
        case AbcBlock::PACKAGE_INTERNAL_NS:
            return o << "package internal namespace";
        case AbcBlock::PROTECTED_NS:
            return o << "protected namespace";
        case AbcBlock::EXPLICIT_NS:
            return o << "explicit namespace";
        case AbcBlock::STATIC_PROTECTED_NS:
            return o << "static protected namespace";
        default:
            return o << "invalid namespace constant";
    }
}

std::ostream&
operator<<(std::ostream& o, AbcBlock::MethodConstant c)
{
    switch (c)
    {
        case AbcBlock::METHOD_ARGS:
            return o << "method arg";
        case AbcBlock::METHOD_ACTIVATION:
            return o << "method activation";
        case AbcBlock::METHOD_MORE:
            return o << "method more";
        case AbcBlock::METHOD_OPTIONAL_ARGS:
            return o << "method optional args";
        case AbcBlock::METHOD_IGNORE:
            return o << "method ignore";
        case AbcBlock::METHOD_NATIVE:
            return o << "method native";
        case AbcBlock::METHOD_DEFAULT_NS:
            return o << "default namespace";
        case AbcBlock::METHOD_ARG_NAMES:
            return o << "method arg names";
        default:
            return o << "invalid method constant";
    }
}

std::ostream&
operator<<(std::ostream& o, AbcBlock::InstanceConstant c)
{
    switch (c)
    {
        case AbcBlock::INSTANCE_SEALED:
            return o << "instance sealed";
        case AbcBlock::INSTANCE_FINAL:
            return o << "instance final";
        case AbcBlock::INSTANCE_INTERFACE:
            return o << "instance interface";
        case AbcBlock::INSTANCE_DYNAMIC:
            return o << "instance dynamic";
        case AbcBlock::INSTANCE_PROTECTED_NS:
            return o << "instance protected namespace";
        default:
            return o << "invalid instance constant";
    }
}

std::ostream&
operator<<(std::ostream& o, AbcBlock::PoolConstant c)
{
    switch (c)
    {
        case AbcBlock::POOL_STRING:
            return o << "pool string";
        case AbcBlock::POOL_INTEGER:
            return o << "pool integer";
        case AbcBlock::POOL_UINTEGER:
            return o << "pool uinteger";
        case AbcBlock::POOL_DOUBLE:
            return o << "pool double";
        case AbcBlock::POOL_NAMESPACE:
            return o << "pool namespace";
        case AbcBlock::POOL_FALSE:
            return o << "pool false";
        case AbcBlock::POOL_TRUE:
            return o << "pool true";
        case AbcBlock::POOL_NULL:
            return o << "pool null";
        default:
            return o << "invalid pool constant";

    }
}

} // namespace abc
} // namespace gnash

