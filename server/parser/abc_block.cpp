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

#include "abc_block.h"
#include "stream.h"

namespace gnash {

namespace abc_parsing {

bool
abc_Trait::read(stream* in)
{
	mNameIndex = in->read_V32();

	uint8_t kind = in->read_u8();
	mKind = static_cast<kinds> (kind & 0x0F);

	switch (mKind)
	{
	case KIND_SLOT:
	case KIND_CONST:
	{
		mSlotId = in->read_V32();
		mTypeIndex = in->read_V32();
		mValueIndex = in->read_V32();
		mValueIndexTypeIndex = in->read_u8();
		break;
	}
	case KIND_METHOD:
	case KIND_GETTER:
	case KIND_SETTER:
	{
		// Ignore the 'disp_id'
		in->skip_V32();

		mMethodInfoIndex = in->read_V32();
		break;
	}
	case KIND_CLASS:
	{
		mSlotId = in->read_V32();
		mClassInfoIndex = in->read_V32();
		break;
	}
	case KIND_FUNCTION:
	{
		mSlotId = in->read_V32();
		mMethodInfoIndex = in->read_V32();
		break;
	}
	default:
	{
		return false;
	}
	} // end of switch

	// Ignore the metadata, but it must be read to know how long it is.
	if ((kind >> 4) & 0x04) // has metadata
	{
		uint32_t metaCount = in->read_V32();
		for (unsigned int k = 0; k < metaCount; ++k)
		{
			in->skip_V32();
		}
	}

	return true; // Here, we were successful.
}

}; // namespace abc_parsing

// Load up all of the data.
bool
abc_block::read(stream* in)
{
	using namespace abc_parsing;

	std::vector<abc_Trait> traitVec;

	// Minor version.
	static_cast<void>(in->read_u16());
	// Major version.
	static_cast<void>(in->read_u16());

	// A block of signed integers. Count overshoots by 1,
	// and the 0 is used to signal a no-op.
	uint32_t intPoolCount = in->read_V32();
	mIntegerPool.resize(intPoolCount);
	for (unsigned int i = 1; i < intPoolCount; ++i)
	{
		mIntegerPool[i] = in->read_s32();
	}
	
	// A block of unsigned integers. Count overshoots by 1,
	// and the 0 is used to signal a no-op.
	uint32_t uIntPoolCount = in->read_V32();
	mUIntegerPool.resize(uIntPoolCount);
	for (unsigned int i = 1; i < uIntPoolCount; ++i)
	{
		mUIntegerPool[i] = in->read_V32();
	}

	// A block of 64 bit doubles.  Counter overshoots by 1,
	// and the 0 is used to signal a no-op.
	uint32_t doublePoolCount = in->read_V32();
	mDoublePool.resize(doublePoolCount);
	for (unsigned int i = 1; i < doublePoolCount; ++i)
	{
		mDoublePool[i] = in->read_d64();
	}

	// A block of strings. Counter overshoots by 1, with the 0th
	// entry used to signal a no-op.
	uint32_t stringPoolCount = in->read_V32();
	mStringPool.resize(stringPoolCount);
	for (unsigned int i = 1; i < stringPoolCount; ++i)
	{
		uint32_t length = in->read_V32();
		in->read_string_with_length(length, mStringPool[i]);
	}

	// Many of the strings will correspond to string table entries --
	// fill this up as we find these.
	mStringPoolTableIds.resize(stringPoolCount);
	for (unsigned int i = 0; i < stringPoolCount; ++i)
		mStringPoolTableIds[i] = 0;

	// These are namespaces, individually. The counter overshoots by
	// 1, with the 0th entry used to signal a wildcard.
	uint32_t namespacePoolCount = in->read_V32();
	mNamespacePool.resize(namespacePoolCount);
	mNamespacePool[0].mUri = mNamespacePool[0].mPrefix = 0;
	mNamespacePool[0].mKind = Namespace::KIND_NORMAL;
	for (unsigned int i = 1; i < namespacePoolCount; ++i)
	{
		uint8_t kind = in->read_u8();
		if (kind == Namespace::KIND_PACKAGE)
			kind = Namespace::KIND_NORMAL;
		else
			kind = static_cast<Namespace::kinds> (kind);
		// All namespaces have the same structure, though the usage differs.

		uint32_t nameIndex = in->read_V32();
		// Set the name of the namespace.
		if (nameIndex && nameIndex < mStringPool.size())
		{
			// If we don't have an index for this yet, do so now.
			if (mStringPoolTableIds[nameIndex] == 0)
				mStringPoolTableIds[nameIndex] =
					mStringTable->find(mStringPool[nameIndex]);
			// And reset the nameIndex for the uri.
			nameIndex = mStringPoolTableIds[nameIndex];
		}
		else
			nameIndex = 0;
		// And set the name in the Namespace itself.
		mNamespacePool[i].mUri = nameIndex;
		// The prefix is unknown right now.
		mNamespacePool[i].mPrefix = 0;
	}

	// These are sets of namespaces, which use the individual ones above.
	uint32_t namespaceSetPoolCount = in->read_V32();
	mNamespaceSetPool.resize(namespaceSetPoolCount);
	// The base namespace set is empty.
	mNamespaceSetPool[0].resize(0);
	for (unsigned int i = 1; i < namespaceSetPoolCount; ++i)
	{
		// These counts are not inflated the way the others are.
		uint32_t count = in->read_V32();
		mNamespaceSetPool[i].resize(count);
		for (unsigned int j = 0; j < count; ++j)
		{
			uint32_t selection = in->read_V32();
			if (!selection || selection >= namespacePoolCount)
			{
				// Reached a bad selection.
				return false;
			}
			mNamespaceSetPool[i][j] = &mNamespacePool[selection];
		}
	}

	// A list of the multinames. The counter overestimates by 1, and the
	// 0th is used as a no-op.
	uint32_t multinamePoolCount = in->read_V32();
	mMultinamePool.resize(multinamePoolCount);
	for (unsigned int i = 0; i < multinamePoolCount; ++i)
	{
		uint8_t kind = in->read_u8();
		mMultinamePool[i].mKind = static_cast<abc_Multiname::kinds>(kind);
		uint32_t ns = 0;
		uint32_t name = 0;
		uint32_t nsset = 0;

		// Read, but don't upper validate until after the switch
		switch (kind)
		{
		case abc_Multiname::KIND_Qname:
		case abc_Multiname::KIND_QnameA:
		{
			ns = in->read_V32();
			name = in->read_V32();
			break;
		}
		case abc_Multiname::KIND_RTQname:
		case abc_Multiname::KIND_RTQnameA:
		{
			name = in->read_V32();
			break;
		}
		case abc_Multiname::KIND_RTQnameL:
		case abc_Multiname::KIND_RTQnameLA:
		{
			break;
		}
		case abc_Multiname::KIND_Multiname:
		case abc_Multiname::KIND_MultinameA:
		{
			name = in->read_V32();
			nsset = in->read_V32();
			// 0 is not a valid nsset.
			if (!nsset)
				return false;
			break;
		}
		case abc_Multiname::KIND_MultinameL:
		case abc_Multiname::KIND_MultinameLA:
		{
			nsset = in->read_V32();
			// 0 is not a valid nsset.
			if (!nsset)
				return false;
			break;
		}
		default:
		{
			// Unknown type.
			return false;
		} // End of cases.
		} // End of switch.

		if (name >= mStringPool.size())
			return false; // Bad name.
		if (ns >= mNamespacePool.size())
			return false; // Bad namespace.
		if (nsset >= mNamespaceSetPool.size())
			return false; // Bad namespace set.

		// The name should be in the string table.
		if (name && mStringPoolTableIds[name] == 0)
		{
			mStringPoolTableIds[name] = mStringTable->find(mStringPool[name]);
		}
		mMultinamePool[i].mName = mStringPoolTableIds[name];

		if (ns)
			mMultinamePool[i].mNamespace = &mNamespacePool[ns];
		if (nsset)
			mMultinamePool[i].mNamespaceSet = &mNamespaceSetPool[nsset];

	} // End of multiname loop.

	uint32_t methodCount = in->read_V32();
	mMethods.resize(methodCount);
	for (unsigned int i = 0; i < methodCount; ++i)
	{
		abc_Method& method = mMethods[i];

		uint32_t param_count = in->read_V32();
		uint32_t return_type = in->read_V32();

		if (return_type >= mMultinamePool.size())
			return false;

		method.mReturnType = &mMultinamePool[return_type];

		method.mParameters.resize(param_count);
		for (unsigned int j = 0; j < param_count; ++j)
		{
			// The parameter type.
			uint32_t ptype = in->read_V32();
			if (ptype >= mMultinamePool.size())
				return false;
			method.mParameters[j] = &mMultinamePool[ptype];
		}
		// We ignore the name_index
		in->skip_V32();

		uint8_t flags = in->read_u8();
		method.mFlags = flags;

		// Some parameters have default values.
		if (flags & abc_Method::FLAG_OPTIONAL)
		{
			uint32_t count = in->read_V32();
			method.mOptionalParameters.resize(count);
			for (unsigned int j = 0; j < count; ++j)
			{
				// The value index.
				method.mOptionalParameters[j].mIndex = in->read_V32();
				// The value kind.
				method.mOptionalParameters[j].mKind = in->read_u8();
			}
		}

		// The parameters are given names, which AS3 can't use. We don't
		// either, since we're not a development environment.
		if (flags & abc_Method::FLAG_PARAM_NAMES)
		{
			for (unsigned int j = 0; j < param_count; ++j)
			{
				in->skip_V32();
			}
		}
	} // End of method loop.

	// Following is MetaData, which we will ignore.
	in->skip_V32(); // A name index.
	uint32_t metaCount = in->read_V32();
	for (unsigned int i = 0; i < metaCount; ++i)
	{
		// key and values are _not_ in this order (they group together), but
		// we are just skipping anyway.
		in->skip_V32();
		in->skip_V32();
	}

	// Classes count.
	uint32_t classCount = in->read_V32();
	mClasses.resize(classCount);

	// But first, instances, which uses classCount
	mInstances.resize(classCount);
	for (unsigned int i = 0; i < classCount; ++i)
	{
		abc_Instance& instance = mInstances[i];
		uint32_t index = in->read_V32();
		if (!index || index >= mMultinamePool.size())
			return false; // Name out of bounds.
		instance.mName = &mMultinamePool[index];

		uint32_t super_index = in->read_V32();
		if (!super_index)
			instance.mSuperType = NULL;
		else if (super_index >= mMultinamePool.size())
			return false; // Bad index.
		else
			instance.mSuperType = &mMultinamePool[super_index];

		uint8_t flags = in->read_u8();
		instance.mFlags = static_cast<abc_Instance::flags> (flags);

		instance.mProtectedNamespace = NULL;
		if (flags & abc_Instance::FLAG_PROTECTED_NS) // Protected namespace
		{
			uint32_t ns_index = in->read_V32();
			if (ns_index >= mNamespacePool.size())
				return false;
			if (ns_index)
				instance.mProtectedNamespace = &mNamespacePool[ns_index];
		}

		// Get the interfaces.
		uint32_t interfaceCount = in->read_V32();
		instance.mInterfaces.resize(interfaceCount);
		for (unsigned int j = 0; j < interfaceCount; ++j)
		{
			uint32_t i_index = in->read_V32();
			if (!i_index || i_index >= mMultinamePool.size())
				return false; // Bad read.
			instance.mInterfaces[j] = &mMultinamePool[i_index];
		}

		// Reach into the methods list.
		uint32_t methodsOffset = in->read_V32();
		if (methodsOffset >= mMethods.size())
			return false; // Bad method.
		instance.mMethod = &mMethods[methodsOffset];

		// Now parse the traits.
		// How many of them.
		uint32_t traitsCount = in->read_V32();
		instance.mTraits.resize(traitsCount);
		for (unsigned int j = 0; j < traitsCount; ++j)
		{
			instance.mTraits[j].read(in);
		}
	} // end of instances list

	// Now the classes are read. TODO: Discover what these do.
	for (unsigned int i = 0; i < classCount; ++i)
	{
		uint32_t initial_method_offset = in->read_V32();
		uint32_t traitsCount = in->read_V32();
		traitVec.resize(traitsCount);
		for (unsigned int j = 0; j < traitsCount; ++j)
		{
			traitVec[j].read(in);
		}
	} // end of classes list

	// The scripts. TODO: Discover what these do.
	uint32_t scriptCount = in->read_V32();
	mScripts.resize(scriptCount);
	for (unsigned int i = 0; i < scriptCount; ++i)
	{
		uint32_t initial_method_offset = in->read_V32();
		uint32_t traitsCount = in->read_V32();
		traitVec.resize(traitsCount);
		for (unsigned int j = 0; j < traitsCount; ++j)
		{
			traitVec[j].read(in);
		}
	}

	// The method bodies. TODO: Use these.
	uint32_t methodBodyCount = in->read_V32();
	for (unsigned int i = 0; i < methodBodyCount; ++i)
	{
		uint32_t method_info = in->read_V32();
		// We don't care about the maximum stack size. Discard it.
		in->skip_V32();
		// We don't care about the maximum register size. Discard it.
		in->skip_V32();
		// What to do with the scope depth?
		in->skip_V32();
		// What to do with the max scope depth?
		in->skip_V32();
		// How long is the code?
		uint32_t code_length = in->read_V32();
		// And the code:
		// TODO: Grab code_length bytes for the code.
		uint32_t exceptions_count = in->read_V32();
		for (unsigned int j = 0; j < exceptions_count; ++j)
		{
			// Where the try block ? begins and ends.
			uint32_t start_offset = in->read_V32();
			uint32_t end_offset = in->read_V32();
			// Where the catch block is located.
			uint32_t catch_offset = in->read_V32();
			// What types should be caught.
			uint32_t catch_type = in->read_V32();
			// If caught, what is the variable name.
			uint32_t catch_var_name = in->read_V32();
		}
		uint32_t traitsCount = in->read_V32();
		traitVec.resize(traitsCount);
		for (unsigned int j = 0; j < traitsCount; ++j)
		{
			traitVec[j].read(in);
		}
	}

	// If flow reaches here, everything went fine.
	return true;
}

}; /* namespace gnash */

