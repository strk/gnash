// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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

#include "gnash.h"
#include "stream.h"
#include "string_table.h"
#include "Namespace.h"

namespace gnash {

typedef std::vector<Namespace *> abcNamespaceSet;

class abc_block;

namespace abc_parsing {

class abc_Trait;

class abc_Multiname
{
public:
    typedef enum
    {
		KIND_Qname = 0x07,
		KIND_QnameA = 0x0D,
		KIND_RTQname = 0x0F,
		KIND_RTQnameA = 0x10,
		KIND_RTQnameL = 0x11,
		KIND_RTQnameLA = 0x12,
		KIND_Multiname = 0x09,
		KIND_MultinameA = 0x0E,
		KIND_MultinameL = 0x1B,
		KIND_MultinameLA = 0x1C
	} kinds;
	typedef enum
	{
		FLAG_ATTR = 0x01,
		FLAG_QNAME = 0x02,
		FLAG_RTNS = 0x04,
		FLAG_RTNAME = 0x08,
		FLAG_NSSET = 0x10
	} flags;

	uint8_t mFlags;
	string_table::key mName;
	Namespace* mNamespace;
	std::vector<Namespace*> *mNamespaceSet;
};

class abc_Method
{
public:
	typedef enum
	{
		FLAG_ARGS = 0x01,
		FLAG_ACTIVATION = 0x02,
		FLAG_MORE = 0x04,
		FLAG_OPTIONAL = 0x08,
		FLAG_IGNORE = 0x10,
		FLAG_NATIVE = 0x20,
		FLAG_DEFAULT_NS = 0x40,
		FLAG_PARAM_NAMES = 0x80
	} flags;

	class optional_parameter
	{
	public:
		uint32_t mIndex;
		uint8_t mKind;
	};

	abc_Multiname *mReturnType;
	std::vector<abc_Multiname*> mParameters; // The types, not the names.
	uint8_t mFlags;
	std::vector<optional_parameter> mOptionalParameters;
};

class abc_Instance
{
public:
	typedef enum
	{
		FLAG_SEALED = 0x01,
		FLAG_FINAL = 0x02,
		FLAG_INTERFACE = 0x04,
		FLAG_PROTECTED_NS = 0x08,
		FLAG_DYNAMIC = 0x00 // No other flags set
	} flags;

	abc_Multiname *mName;
	abc_Multiname *mSuperType;
	flags mFlags;
	Namespace *mProtectedNamespace;
	std::vector<abc_Multiname*> mInterfaces;
	abc_Method *mMethod;
	std::vector<abc_Trait> mTraits;
};

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

	kinds mKind;
	uint32_t mNameIndex;
	uint32_t mNamespaceIndex;
	uint32_t mNamespaceSetIndex;
	uint32_t mSlotId;
	uint32_t mTypeIndex;
	uint32_t mValueIndex;
	uint8_t mValueIndexTypeIndex;
	uint32_t mClassInfoIndex;
	uint32_t mMethodInfoIndex;

	bool read(stream* in);
};

class abc_Class
{
public:
	abc_Method *mMethod;
	std::vector<abc_Trait> mTraits;
};

class abc_Script
{
public:
	abc_Method *mMethod;
	std::vector<abc_Trait> mTraits;
};

class abc_Exception
{
public:
	uint32_t mStart;
	uint32_t mEnd;
	uint32_t mCatch;
	abc_Multiname* mType;
	abc_Multiname* mName;
};

class abc_MethodBody
{
public:
	abc_Method *mMethod;
	std::vector<abc_Exception> mExceptions;
	std::vector<abc_Trait> mTraits;
	std::vector<char> mCode;
};

}; // namespace abc_parsing

typedef std::vector<Namespace*> NamespaceSet;
			
class abc_block
{
private:
	std::vector<int32_t> mIntegerPool;
	std::vector<uint32_t> mUIntegerPool;
	std::vector<long double> mDoublePool;
	std::vector<std::string> mStringPool;
	std::vector<string_table::key> mStringPoolTableIds;
	std::vector<Namespace*> mNamespacePool;
	std::vector<NamespaceSet> mNamespaceSetPool;
	std::vector<abc_parsing::abc_Method> mMethods;
	std::vector<abc_parsing::abc_Multiname> mMultinamePool;
	std::vector<abc_parsing::abc_Instance> mInstances;
	std::vector<abc_parsing::abc_Class> mClasses; 
	std::vector<abc_parsing::abc_Script> mScripts;
	std::vector<abc_parsing::abc_MethodBody> mBodies;

	string_table* mStringTable;

public:
	bool read(stream* in);

	abc_block();
};

}; /* namespace gnash */

#endif /* GNASH_ABC_BLOCK_H */

