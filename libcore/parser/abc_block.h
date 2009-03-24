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

namespace abc_parsing {

class abc_Trait;

class abc_Trait
{
public:
	typedef enum
	{
		KIND_SLOT = 0,
		KIND_CONST = 6,
		KIND_METHOD = 1,
		KIND_GETTER = 2,
		KIND_SETTER = 3,
		KIND_CLASS = 4,
		KIND_FUNCTION = 5
	} kinds;

	bool mHasValue;
	kinds mKind;
	boost::uint32_t mSlotId;
	boost::uint32_t mTypeIndex;
	boost::uint32_t mClassInfoIndex;
	as_value mValue;
	string_table::key mName;
	string_table::key mGlobalName;
	asNamespace *mNamespace;
	asMethod *mMethod;
	bool mValueSet;

	asClass *mCTarget;
	asMethod *mMTarget;
	bool mStatic;

	abc_Trait()
        :
        mHasValue(false),
        mKind(KIND_SLOT),
        mSlotId(0),
        mTypeIndex(0),
        mClassInfoIndex(0),
        mValue(),
        mName(0),
        mGlobalName(),
        mNamespace(0),
        mMethod(0),
        mValueSet(false),
        mCTarget(0),
        mMTarget(0),
        mStatic(false)
	{/**/}

	bool read(SWFStream* in, abc_block *pBlock);

	bool finalize(abc_block *pBlock, asClass *pClass, bool do_static);

	bool finalize_mbody(abc_block *pBlock, asMethod *pMethod);

	void set_target(asClass *pClass, bool do_static)
	{ mCTarget = pClass; mStatic = do_static; }

	void set_target(asMethod *pMethod)
	{ mCTarget = 0; mMTarget = pMethod; }

	bool finalize(abc_block *pBlock)
	{
		if (mCTarget)
			return finalize(pBlock, mCTarget, mStatic);
		return finalize_mbody(pBlock, mMTarget);
	}
};

} // namespace abc_parsing

typedef std::vector<asNamespace*> NamespaceSet;
			
class abc_block
{
public:
	typedef enum
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
	} constants;

	std::vector<boost::int32_t> mIntegerPool;
	std::vector<boost::uint32_t> mUIntegerPool;
	std::vector<double> mDoublePool;
	std::vector<std::string> mStringPool;
	std::vector<string_table::key> mStringPoolTableIds;
	std::vector<asNamespace*> mNamespacePool;
	std::vector<NamespaceSet> mNamespaceSetPool;
	std::vector<asMethod*> mMethods;
	std::vector<asName> mMultinamePool;
	std::vector<asClass*> mClasses; 
	std::vector<asClass*> mScripts;
	std::vector<abc_parsing::abc_Trait*> mTraits;

	string_table* mStringTable;
	SWFStream* mS; // Not stored beyond one read.

	asClass *mTheObject;
	ClassHierarchy *mCH;

	boost::uint32_t mVersion;

	asClass *locateClass(asName &m);

	asClass *locateClass(const std::string& className);

	abc_parsing::abc_Trait &newTrait()
	{
		abc_parsing::abc_Trait *p = new abc_parsing::abc_Trait;
		mTraits.push_back(p);
		return *p;
	}

public:
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

	bool read(SWFStream& in);

	bool pool_value(boost::uint32_t index, boost::uint8_t type, as_value &v);

	void update_global_name(unsigned int multiname_index);

	abc_block();

private:
	void check_multiname_name(boost::uint32_t name);

	void check_multiname_namespace(boost::uint32_t ns);

	void check_multiname_namespaceset(boost::uint32_t nsset);

	void setMultinameNames(asName *n,string_table::key ABCName);

	void setNamespaceURI(asNamespace *ns,string_table::key ABCName);

};

} /* namespace gnash */

#endif /* GNASH_ABC_BLOCK_H */

