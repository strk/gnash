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

#ifndef GNASH_ABC_BLOCK_H
#define GNASH_ABC_BLOCK_H

#include "string_table.h"
#include "MultiName.h"
#include "Namespace.h"
#include "as_value.h"

#include <vector>
#include <string>
#include <boost/scoped_array.hpp>
#include <stdexcept>

namespace gnash {
    namespace abc {
        class AbcBlock;
        class Machine;
        class Class;
        class Method;
    }
    class SWFStream; // for read signature
    class ClasstHierarchy;
}

namespace gnash {

/// ABC-only resources for parsing and execution.
namespace abc {

/// Class describing a static property
//
/// Traits are non-dynamic properties. That is, they are not deletable or
/// modifiable in certain ways through ActionScript. They exist for reasons
/// of performance. A property lookup on an object always checks the Traits
/// before dynamic properties.
//
/// Traits can belong to Methods, Classes, and Scripts. Classes have both
/// instance and Class traits.
//
/// TODO: Traits currently need finalization. This performs two tasks. At least
/// one, and possibly both, are wrong:
/// 1. Trait definitions contain references to AbcBlock definitions. Currently
///    these references are resolved during finalization. It may be possible
///    to do this during parsing.
/// 2. Traits should be made available to ActionScript. Currently this is done
///    by attaching them to an object. This is plain wrong and doesn't even
///    work in many cases.
//
/// TODO: As Traits are stored in the correct Class, Method etc, they do not
///       need to store a target.
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

	bool read(SWFStream* in, AbcBlock *block);

	bool finalize(AbcBlock* block, abc::Class* cl, bool do_static);

	bool finalize_mbody(AbcBlock* block, Method* m);

	void set_target(abc::Class* cl, bool do_static) {
        _classTarget = cl;
        _static = do_static;
    }

	void set_target(Method *m) {
        _classTarget = 0;
        _methodTarget = m;
    }

	bool finalize(AbcBlock* block)
	{
		if (_classTarget) {
			return finalize(block, _classTarget, _static);
        }
		return finalize_mbody(block, _methodTarget);
	}

private:

    friend class AbcBlock;

	bool _hasValue;
	Kind _kind;
	boost::uint32_t _slotID;
	boost::uint32_t _typeIndex;
	boost::uint32_t _classInfoIndex;
	as_value _value;

	URI _name;
    string_table::key _globalName;

	Namespace* _namespace;
	Method* _method;
	bool _valueSet;

	abc::Class* _classTarget;
	Method* _methodTarget;
	bool _static;

};

/// Output stream operator for abc::Trait::Kind
std::ostream& operator<<(std::ostream& o, const Trait::Kind k);

namespace {

template<typename T>
inline void checkBounds(size_t i, const T& container)
{
    if (i >= container.size()) {
        throw std::range_error("Attempt to access pool out of range");
    }
}

}


/// The ActionScript bytecode of a single ABC tag in a SWF.
//
/// ABC blocks have their own "names" for all resources. In Gnash, these are
/// a string table index. They are different from global names. These are used
/// to locate resources inside the ABC block.
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
/// Instances / Scriptes 
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
/// This is particularly important for locateClass (called by instantiateScript
/// from SymbolScript tag execution). The SymbolScript tag identifies a class
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
    
    typedef std::vector<Namespace*> NamespaceSet;

	AbcBlock();

    abc::Class* locateClass(MultiName &m);

	abc::Class* locateClass(const std::string& className);

    bool read(SWFStream& in);

	void update_global_name(unsigned int multiname_index);

    /// Scripts can contain several classes.
    //
    /// TODO: why on earth are Scripts implemented using Classes?
    const std::vector<abc::Class*>& scripts() const {
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

    Method* methodPoolAt(size_t i) const {
        checkBounds(i, _methods);
        return _methods[i];
    }

    MultiName multinamePoolAt(size_t i) const {
        checkBounds(i, _multinamePool);
        return _multinamePool[i];
    }

    abc::Class* classPoolAt(size_t i) const {
        checkBounds(i, _classes);
        return _classes[i];
    }

    Namespace* namespacePoolAt(size_t i) const {
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

	void setMultinameNames(MultiName *n, abc::URI ABCName);

	void setNamespaceURI(Namespace *ns, abc::URI ABCName);

	std::vector<boost::int32_t> _integerPool;
	std::vector<boost::uint32_t> _uIntegerPool;
	std::vector<double> _doublePool;
	std::vector<std::string> _stringPool;
	std::vector<Namespace*> _namespacePool;
	std::vector<NamespaceSet> _namespaceSetPool;
	std::vector<Method*> _methods;
	std::vector<MultiName> _multinamePool;
	std::vector<Class*> _classes; 
	std::vector<Class*> _scripts;

	string_table* _stringTable;
	SWFStream* _stream; // Not stored beyond one read.

	abc::Class *mTheObject;
	ClassHierarchy *mCH;

	boost::uint32_t mVersion;


};

std::ostream& operator<<(std::ostream& o, AbcBlock::NamespaceConstant c);
std::ostream& operator<<(std::ostream& o, AbcBlock::MethodConstant c);
std::ostream& operator<<(std::ostream& o, AbcBlock::InstanceConstant c);
std::ostream& operator<<(std::ostream& o, AbcBlock::PoolConstant c);

} // namespace abc
} // namespace gnash


#endif 

