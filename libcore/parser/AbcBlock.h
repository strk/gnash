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

#include "string_table.h"
#include "asName.h"
#include "asNamespace.h"
#include "as_value.h"

#include <vector>
#include <string>
#include <boost/scoped_array.hpp>
#include <stdexcept>

namespace gnash {
	class SWFStream; // for read signature
}

namespace gnash {

class AbcBlock;
class ClassHierarchy;
class asMethod;
class asClass;
class Machine;

namespace abc {

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

	bool read(SWFStream* in, AbcBlock *pBlock);

	bool finalize(AbcBlock* pBlock, asClass* pClass, bool do_static);

	bool finalize_mbody(AbcBlock* pBlock, asMethod* pMethod);

	void set_target(asClass* pClass, bool do_static) {
        _classTarget = pClass;
        _static = do_static;
    }

	void set_target(asMethod *pMethod) {
        _classTarget = 0;
        _methodTarget = pMethod;
    }

	bool finalize(AbcBlock* pBlock)
	{
		if (_classTarget) {
			return finalize(pBlock, _classTarget, _static);
        }
		return finalize_mbody(pBlock, _methodTarget);
	}
};

/// Output stream operator for abc::Trait::Kind
std::ostream& operator<<(std::ostream& o, const Trait::Kind k);

} // namespace abc

namespace {

template<typename T>
inline void checkBounds(size_t i, const T& container)
{
    if (i >= container.size()) {
        throw std::range_error("Attempt to access pool out of range");
    }
}

}

/// ABC blocks have their own "names" for all resources. In Gnash, these are
/// a string table index. They are different from global names. These are used
/// to locate resources inside the ABC block.
// 
/// Namespaces
//
/// ABC blocks have a set of "namespace" resources. Some namespaces are
/// private. We make these into anonymous namespaces.
// 
/// We assume all non-private namespaces are public. Some are "package"
/// namespaces; these seem to coincide with the built-in packages or 0,
/// the global namespace.
/// 
/// We always search for these public namespaces by global URI in our
/// ClassHierarchy. If we use ABC names, "flash.text" will not find the built-in
/// flash.text namespace. Using the global name means that we 'import' the
/// built-in namespace into our own resources.
//
/// Instances / Classes 
//
/// Likewise, classes are always given a global name, not an ABC name. This is
/// because they become globally available, including (we assume) to other ABC
/// blocks, so using an ABC name means they cannot be located externally.
/// Even if ABC block resources should not be available to other blocks (which
/// seems unlikely), using an ABC name for classes risks name conflicts with
/// the built-in classes already in a namespace: ABC names and global names
/// can have the same index even when the names are different.
//
/// Class lookup
//
/// This is particularly important for locateClass (called by instantiateClass
/// from SymbolClass tag execution). The SymbolClass tag identifies a class
/// using a global name, which may be qualified with a namespace. If it is
/// not qualified, we look in the global namespace 0.
// 
/// When we call locateClass, we use global names, not ABC names, because
/// classes are identified by global names (see above). However, we
/// still look only in the ABC block's namespaces. The block's first namespace
/// is always the global namespace; other package namespaces are imported
/// according to the block's namespace constants.
class AbcBlock
{
public:
    
    enum NamespaceConstant
	{
		PRIVATE_NS = 0x05,
        CONSTANT_NS = 0x08,
		PACKAGE_NS = 0x16,
		PACKAGE_INTERNAL_NS = 0x17,
		PROTECTED_NS = 0x18,
		EXPLICIT_NS = 0x19,
		STATIC_PROTECTED_NS = 0x1A
    };

    enum MethodConstant
    {
        METHOD_ARGS = 0x01,
		METHOD_ACTIVATION = 0x02,
		METHOD_MORE = 0x04,
		METHOD_OPTIONAL_ARGS = 0x08,
		METHOD_IGNORE = 0x10,
		METHOD_NATIVE = 0x20,
		METHOD_DEFAULT_NS = 0x40,
		METHOD_ARG_NAMES = 0x80
    };

    enum InstanceConstant
    {
		INSTANCE_SEALED = 0x01,
		INSTANCE_FINAL = 0x02,
		INSTANCE_INTERFACE = 0x04,
		INSTANCE_DYNAMIC = 0x00,
		INSTANCE_PROTECTED_NS = 0x08
    };

    enum PoolConstant
    {
		POOL_STRING = 0x01,
		POOL_INTEGER = 0x03,
		POOL_UINTEGER = 0x04,
		POOL_DOUBLE = 0x06,
		POOL_NAMESPACE = 0x08,
		POOL_FALSE = 0x0A,
		POOL_TRUE = 0x0B,
		POOL_NULL = 0x0C
	};
    
    typedef std::vector<asNamespace*> NamespaceSet;

	AbcBlock();

    asClass* locateClass(asName &m);

	asClass* locateClass(const std::string& className);

	abc::Trait &newTrait()
	{
		abc::Trait *p = new abc::Trait;
		_traits.push_back(p);
		return *p;
	}
	
    bool read(SWFStream& in);

	void update_global_name(unsigned int multiname_index);

    const std::vector<asClass*>& scripts() const {
        return _scripts;
    }

    boost::uint32_t uIntegerPoolAt(size_t i) const {
        checkBounds(i, _uIntegerPool);
        return _uIntegerPool[i];
    }

    const std::string& stringPoolAt(size_t i) const {
        checkBounds(i, _stringPool);
        return _stringPool[i];
    }

    boost::int32_t integerPoolAt(size_t i) const {
        checkBounds(i, _integerPool);
        return _integerPool[i];
    }

    double doublePoolAt(size_t i) const {
        checkBounds(i, _doublePool);
        return _doublePool[i];
    }

    asMethod* methodPoolAt(size_t i) const {
        checkBounds(i, _methods);
        return _methods[i];
    }

    asName multinamePoolAt(size_t i) const {
        checkBounds(i, _multinamePool);
        return _multinamePool[i];
    }

    asClass* classPoolAt(size_t i) const {
        checkBounds(i, _classes);
        return _classes[i];
    }

    asNamespace* namespacePoolAt(size_t i) const {
        checkBounds(i, _namespacePool);
        return _namespacePool[i];
    }

    void prepare(Machine* mach);

private:
	
    friend class abc::Trait;

	bool pool_value(boost::uint32_t index, PoolConstant type, as_value &v);

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
	SWFStream* _stream; // Not stored beyond one read.

	asClass *mTheObject;
	ClassHierarchy *mCH;

	boost::uint32_t mVersion;


};

std::ostream& operator<<(std::ostream& o, AbcBlock::NamespaceConstant c);
std::ostream& operator<<(std::ostream& o, AbcBlock::MethodConstant c);
std::ostream& operator<<(std::ostream& o, AbcBlock::InstanceConstant c);
std::ostream& operator<<(std::ostream& o, AbcBlock::PoolConstant c);

} 


#endif /* GNASH_ABC_BLOCK_H */

