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

#ifndef GNASH_ABC_BLOCK_H
#define GNASH_ABC_BLOCK_H

#include <vector>
#include <string>
#include <boost/scoped_array.hpp>

#include "string_table.h"
#include "asClass.h"
#include "asName.h"

namespace gnash {
	class SWFStream; // for read signature
}

namespace gnash {

typedef std::vector<asNamespace *> abcNamespaceSet;

class abc_block;
class ClassHierarchy;
class asMethod;
class asClass;

namespace abc {

class Trait;

class Trait
{
public:

    enum Kind
	{
		KIND_SLOT = 0,
		KIND_CONST = 6,
		KIND_METHOD = 1,
		KIND_GETTER = 2,
		KIND_SETTER = 3,
		KIND_CLASS = 4,
		KIND_FUNCTION = 5
	};

	bool _hasValue;
	Kind _kind;
	boost::uint32_t _slotID;
	boost::uint32_t _typeIndex;
	boost::uint32_t _classInfoIndex;
	as_value _value;
	string_table::key _name;
	string_table::key _globalName;
	asNamespace* _namespace;
	asMethod* _method;
	bool _valueSet;

	asClass* _classTarget;
	asMethod* _methodTarget;
	bool _static;

	Trait()
        :
        _hasValue(false),
        _kind(KIND_SLOT),
        _slotID(0),
        _typeIndex(0),
        _classInfoIndex(0),
        _value(),
        _name(0),
        _globalName(),
        _namespace(0),
        _method(0),
        _valueSet(false),
        _classTarget(0),
        _methodTarget(0),
        _static(false)
	{}

	bool read(SWFStream* in, abc_block *pBlock);

	bool finalize(abc_block* pBlock, asClass* pClass, bool do_static);

	bool finalize_mbody(abc_block* pBlock, asMethod* pMethod);

	void set_target(asClass* pClass, bool do_static) {
        _classTarget = pClass;
        _static = do_static;
    }

	void set_target(asMethod *pMethod) {
        _classTarget = 0;
        _methodTarget = pMethod;
    }

	bool finalize(abc_block* pBlock)
	{
		if (_classTarget) {
			return finalize(pBlock, _classTarget, _static);
        }
		return finalize_mbody(pBlock, _methodTarget);
	}
};

} // namespace abc

typedef std::vector<asNamespace*> NamespaceSet;
			
class abc_block
{
public:

	abc_block();

    asClass* locateClass(asName &m);

	asClass* locateClass(const std::string& className);

	abc::Trait &newTrait()
	{
		abc::Trait *p = new abc::Trait;
		_traits.push_back(p);
		return *p;
	}
	
    bool read(SWFStream& in);

	bool pool_value(boost::uint32_t index, boost::uint8_t type, as_value &v);

	void update_global_name(unsigned int multiname_index);

    const std::vector<asClass*>& scripts() const {
        return _scripts;
    }

    boost::uint32_t uIntegerPoolAt(size_t i) const {
        assert(i < _uIntegerPool.size());
        return _uIntegerPool[i];
    }

    const std::string& stringPoolAt(size_t i) const {
        assert(i < _stringPool.size());
        return _stringPool[i];
    }

    boost::int32_t integerPoolAt(size_t i) const {
        assert(i < _integerPool.size());
        return _integerPool[i];
    }

    double doublePoolAt(size_t i) const {
        assert(i < _doublePool.size());
        return _doublePool[i];
    }

    asMethod* methodPoolAt(size_t i) const {
        assert(i < _methods.size());
        return _methods[i];
    }

    asName multinamePoolAt(size_t i) const {
        assert(i < _multinamePool.size());
        return _multinamePool[i];
    }

    asClass* classPoolAt(size_t i) const {
        assert(i < _classes.size());
        return _classes[i];
    }

    asNamespace* namespacePoolAt(size_t i) const {
        assert(i < _namespacePool.size());
        return _namespacePool[i];
    }

    void prepare(Machine* mach);

private:

    friend class abc::Trait;

	bool read_version();
	bool read_integer_constants();
	bool read_unsigned_integer_constants();
	bool read_double_constants();
	bool read_string_constants();
	bool read_namespaces();
	bool read_namespace_sets();
	bool read_multinames();
	bool read_method_infos();
	bool skip_metadata();
	bool read_instances();
	bool read_classes();
	bool read_scripts();
	bool read_method_bodies();

	void check_multiname_name(boost::uint32_t name);

	void check_multiname_namespace(boost::uint32_t ns);

	void check_multiname_namespaceset(boost::uint32_t nsset);

	void setMultinameNames(asName *n,string_table::key ABCName);

	void setNamespaceURI(asNamespace *ns,string_table::key ABCName);

	enum Constants
	{
		PRIVATE_NS = 0x05,
		PROTECTED_NS = 0x18,
		METHOD_ARGS = 0x01,
		METHOD_ACTIVATION = 0x02,
		METHOD_MORE = 0x04,
		METHOD_OPTIONAL_ARGS = 0x08,
		METHOD_IGNORE = 0x10,
		METHOD_NATIVE = 0x20,
		METHOD_DEFAULT_NS = 0x40,
		METHOD_ARG_NAMES = 0x80,
		INSTANCE_SEALED = 0x01,
		INSTANCE_FINAL = 0x02,
		INSTANCE_INTERFACE = 0x04,
		INSTANCE_DYNAMIC = 0x00,
		INSTANCE_PROTECTED_NS = 0x08,
		POOL_STRING = 0x01,
		POOL_INTEGER = 0x03,
		POOL_UINTEGER = 0x04,
		POOL_DOUBLE = 0x06,
		POOL_NAMESPACE = 0x08,
		POOL_FALSE = 0x0A,
		POOL_TRUE = 0x0B,
		POOL_NULL = 0x0C
	};

	std::vector<boost::int32_t> _integerPool;
	std::vector<boost::uint32_t> _uIntegerPool;
	std::vector<double> _doublePool;
	std::vector<std::string> _stringPool;
	std::vector<string_table::key> _stringPoolTableIDs;
	std::vector<asNamespace*> _namespacePool;
	std::vector<NamespaceSet> _namespaceSetPool;
	std::vector<asMethod*> _methods;
	std::vector<asName> _multinamePool;
	std::vector<asClass*> _classes; 
	std::vector<asClass*> _scripts;
	std::vector<abc::Trait*> _traits;

	string_table* _stringTable;
	SWFStream* mS; // Not stored beyond one read.

	asClass *mTheObject;
	ClassHierarchy *mCH;

	boost::uint32_t mVersion;


};

} 

#endif /* GNASH_ABC_BLOCK_H */

