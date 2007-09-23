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
#include "VM.h"
#include "log.h"

#define ERR(x) IF_VERBOSE_MALFORMED_SWF(log_swferror x;);

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
		if (mValueIndex)
			mValueIndexTypeIndex = in->read_u8();
		else
			mValueIndexTypeIndex = 0;
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
		ERR((_("Action Block: Unknown trait kind (%d).\n"), mKind));
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

	// Minor version, major version.
	uint32_t version = (in->read_u16()) | (in->read_u16() << 16);
	ERR((_("Abc Version: %d.%d\n"), (version & 0xFFFF0000) >> 16, (version & 0x0000FFFF)));

	// A block of signed integers. Count overshoots by 1,
	// and the 0 is used to signal a no-op.
	uint32_t intPoolCount = in->read_V32();
	mIntegerPool.resize(intPoolCount);
	if (intPoolCount)
		mIntegerPool[0] = 0;
	for (unsigned int i = 1; i < intPoolCount; ++i)
	{
		mIntegerPool[i] = static_cast<int32_t> (in->read_V32());
	}
	
	// A block of unsigned integers. Count overshoots by 1,
	// and the 0 is used to signal a no-op.
	uint32_t uIntPoolCount = in->read_V32();
	mUIntegerPool.resize(uIntPoolCount);
	if (uIntPoolCount)
		mUIntegerPool[0] = 0;
	for (unsigned int i = 1; i < uIntPoolCount; ++i)
	{
		mUIntegerPool[i] = in->read_V32();
	}

	// A block of 64 bit doubles.  Counter overshoots by 1,
	// and the 0 is used to signal a no-op.
	uint32_t doublePoolCount = in->read_V32();
	mDoublePool.resize(doublePoolCount);
	if (doublePoolCount)
		mDoublePool[0] = 0.0;
	for (unsigned int i = 1; i < doublePoolCount; ++i)
	{
		mDoublePool[i] = in->read_d64();
	}

	// A block of strings. Counter overshoots by 1, with the 0th
	// entry used to signal a no-op.
	uint32_t stringPoolCount = in->read_V32();
	mStringPool.resize(stringPoolCount);
	mStringPoolTableIds.resize(stringPoolCount);
	if (stringPoolCount)
	{
		mStringPool[0] = "";
		mStringPoolTableIds[0] = 0;
	}
	for (unsigned int i = 1; i < stringPoolCount; ++i)
	{
		uint32_t length = in->read_V32();
		in->read_string_with_length(length, mStringPool[i]);
		mStringPoolTableIds[i] = 0;
	}

	// These are namespaces, individually. The counter overshoots by
	// 1, with the 0th entry used to signal a wildcard.
	uint32_t namespacePoolCount = in->read_V32();
	mNamespacePool.resize(namespacePoolCount);
	if (namespacePoolCount)
	{
		mNamespacePool[0].mUri = mNamespacePool[0].mPrefix = 0;
		mNamespacePool[0].mKind = Namespace::KIND_NORMAL;
	}
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
		else if (nameIndex >= mStringPool.size())
		{
			ERR((_("Action Block: Out of Bound string for namespace.\n")));
			return false;
		}

		// And set the name in the Namespace itself.
		mNamespacePool[i].mUri = nameIndex;
		// The prefix is unknown right now.
		mNamespacePool[i].mPrefix = 0;
	}

	// These are sets of namespaces, which use the individual ones above.
	uint32_t namespaceSetPoolCount = in->read_V32();
	mNamespaceSetPool.resize(namespaceSetPoolCount);
	if (namespaceSetPoolCount)
	{
		// The base namespace set is empty.
		mNamespaceSetPool[0].resize(0);
	}
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
				ERR((_("Action Block: Out of Bound namespace in namespace set.\n")));
				return false;
			}
			mNamespaceSetPool[i][j] = &mNamespacePool[selection];
		}
	}

	// A list of the multinames. The counter overestimates by 1, and the
	// 0th is used as a no-op.
	uint32_t multinamePoolCount = in->read_V32();

	// Any two namespaces with the same uri here are the same namespace,
	// excepting private nameSpaces.  This will be handled at verify time.
	// TODO: Handle this.
	mMultinamePool.resize(multinamePoolCount);
	for (unsigned int i = 1; i < multinamePoolCount; ++i)
	{
		uint8_t kind = in->read_u8();
		uint32_t ns = 0;
		uint32_t name = 0;
		uint32_t nsset = 0;

		mMultinamePool[i].mFlags = 0;

		// Read, but don't upper validate until after the switch
		switch (kind)
		{
		case abc_Multiname::KIND_Qname:
		case abc_Multiname::KIND_QnameA:
		{
			ns = in->read_V32();
			name = in->read_V32();
			mMultinamePool[i].mFlags |= abc_Multiname::FLAG_QNAME;
			if (kind == abc_Multiname::KIND_QnameA)
				mMultinamePool[i].mFlags |= abc_Multiname::FLAG_ATTR;
			break;
		}
		case abc_Multiname::KIND_RTQname:
		case abc_Multiname::KIND_RTQnameA:
		{
			name = in->read_V32();
			mMultinamePool[i].mFlags |= abc_Multiname::FLAG_QNAME
				| abc_Multiname::FLAG_RTNS;
			if (kind == abc_Multiname::KIND_RTQnameA)
				mMultinamePool[i].mFlags |= abc_Multiname::FLAG_ATTR;
			break;
		}
		case abc_Multiname::KIND_RTQnameL:
		case abc_Multiname::KIND_RTQnameLA:
		{
			mMultinamePool[i].mFlags |= abc_Multiname::FLAG_QNAME
				| abc_Multiname::FLAG_RTNAME
				| abc_Multiname::FLAG_RTNS;
			if (kind == abc_Multiname::KIND_RTQnameLA)
				mMultinamePool[i].mFlags |= abc_Multiname::FLAG_ATTR;
			break;
		}
		case abc_Multiname::KIND_Multiname:
		case abc_Multiname::KIND_MultinameA:
		{
			name = in->read_V32();
			nsset = in->read_V32();
			// 0 is not a valid nsset.
			if (!nsset)
			{
				ERR((_("Action Block: 0 selection for namespace set is invalid.\n")));
				return false;
			}
			mMultinamePool[i].mFlags |= abc_Multiname::FLAG_NSSET;
			if (kind == abc_Multiname::KIND_MultinameA)
				mMultinamePool[i].mFlags |= abc_Multiname::FLAG_ATTR;
			break;
		}
		case abc_Multiname::KIND_MultinameL:
		case abc_Multiname::KIND_MultinameLA:
		{
			nsset = in->read_V32();
			// 0 is not a valid nsset.
			if (!nsset)
			{
				ERR((_("Action Block: 0 selection for namespace set is invalid.\n")));
				return false;
			}
			mMultinamePool[i].mFlags |= abc_Multiname::FLAG_RTNAME
				| abc_Multiname::FLAG_NSSET;
			if (kind == abc_Multiname::KIND_MultinameLA)
				mMultinamePool[i].mFlags |= abc_Multiname::FLAG_ATTR;
			break;
		}
		default:
		{
			// Unknown type.
			ERR((_("Action Block: Unknown multiname type (%d).\n"), kind));
			return false;
		} // End of cases.
		} // End of switch.

		if (name >= mStringPool.size())
		{
			ERR((_("Action Block: Out of Bound string for Multiname.\n")));
			return false; // Bad name.
		}
		if (ns >= mNamespacePool.size())
		{
			ERR((_("Action Block: Out of Bound namespace for Multiname.\n")));
			return false; // Bad namespace.
		}
		if (nsset >= mNamespaceSetPool.size())
		{
			ERR((_("Action Block: Out of Bound namespace set for Multiname.\n")));
			return false; // Bad namespace set.
		}

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
		{
			ERR((_("Action Block: Out of Bound return type for "
				"method info (%d).\n"), return_type));
			return false;
		}

		method.mReturnType = &mMultinamePool[return_type];

		method.mParameters.resize(param_count);
		for (unsigned int j = 0; j < param_count; ++j)
		{
			// The parameter type.
			uint32_t ptype = in->read_V32();
			if (ptype >= mMultinamePool.size())
			{
				ERR((_("Action Block: Out of Bound parameter type "
					"for method info (%d).\n"), ptype));
				return false;
			}
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
	uint32_t metaCount = in->read_V32();
	for (unsigned int i = 0; i < metaCount; ++i)
	{
		in->skip_V32(); // A name index.
		uint32_t metaInternalCount = in->read_V32();
		for (unsigned int j = 0; j < metaInternalCount; ++j)
		{
			// key and values are _not_ in this order (they group together), but
			// we are just skipping anyway.
			in->skip_V32();
			in->skip_V32();
		}
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
		// 0 is allowed as a name, typically for the last entry.
		if (index >= mMultinamePool.size())
		{
			ERR((_("Action Block: Out of Bound instance name (%d).\n"), index));
			return false; // Name out of bounds.
		}
		instance.mName = &mMultinamePool[index];
		// This must be a QName, not some other type.
		if (!(instance.mName->mFlags & abc_Multiname::FLAG_QNAME))
		{
			ERR((_("Action Block: Qname required for instance.\n")));
			return false; // Name not Qname
		}
		
		uint32_t super_index = in->read_V32();
		if (!super_index)
			instance.mSuperType = NULL;
		else if (super_index >= mMultinamePool.size())
		{
			ERR((_("Action Block: Out of Bound super type (%d).\n"), super_index));
			return false; // Bad index.
		}
		else
			instance.mSuperType = &mMultinamePool[super_index];

		as_object *global = VM::get().getGlobal();
		as_value vstore;
		if (super_index && !global->get_member(instance.mSuperType->mName, &vstore))
		{
			ERR((_("Action Block: Non-existent super type (%d).\n"),
				super_index));
			fprintf(stderr, "Super type doesn't exist, but continuing (%s).\n",
				mStringTable->value(instance.mSuperType->mName).c_str());
			//return false;
		}

		// vstore now contains the object (we hope) which is the class that
		// we want to be the super of this instance.
		if (!vstore.is_object())
		{
			ERR((_("Action Block: Super prospect is not an object.\n")));
			//return false;
		}

		uint8_t flags = in->read_u8();
		instance.mFlags = static_cast<abc_Instance::flags> (flags);

		instance.mProtectedNamespace = NULL;
		if (flags & abc_Instance::FLAG_PROTECTED_NS) // Protected namespace
		{
			uint32_t ns_index = in->read_V32();
			if (ns_index >= mNamespacePool.size())
			{
				ERR((_("Action Block: Out of Bound namespace for "
					"instance protected namespace (%d).\n"), ns_index));
				return false;
			}
			if (ns_index)
				instance.mProtectedNamespace = &mNamespacePool[ns_index];
		}

		// Get the interfaces.
		uint32_t interfaceCount = in->read_V32();
		instance.mInterfaces.resize(interfaceCount);
		for (unsigned int j = 0; j < interfaceCount; ++j)
		{
			uint32_t i_index = in->read_V32();
			// 0 is allowed as an interface, typically for the last one.
			if (i_index >= mMultinamePool.size())
			{
				ERR((_("Action Block: Out of Bound name for interface. "
					"(%d)\n"), i_index));
				return false; // Bad read.
			}
			instance.mInterfaces[j] = &mMultinamePool[i_index];
		}

		// Reach into the methods list.
		uint32_t methodsOffset = in->read_V32();
		if (methodsOffset >= mMethods.size())
		{
			ERR((_("Action Block: Out of Bound method for interface. (%d)\n"),
				methodsOffset));
			return false; // Bad method.
		}
		instance.mMethod = &mMethods[methodsOffset];

		// Now parse the traits.
		// How many of them.
		uint32_t traitsCount = in->read_V32();
		instance.mTraits.resize(traitsCount);
		for (unsigned int j = 0; j < traitsCount; ++j)
		{
			if (!instance.mTraits[j].read(in))
				return false;
		}

		// And now we add this instance to the type names of the VM...
		// TODO: Quit faking!
		as_value fake;
		if (global->get_member(instance.mName->mName, &fake))
		{
			fprintf(stderr, "Already registered, but doing it again?"
				"Type is %s\n", mStringTable->value(instance.mName->mName).c_str());
		}
		global->set_member(instance.mName->mName, as_value(2));
	} // end of instances list

	// Now the classes are read. TODO: Discover what these do.
	for (unsigned int i = 0; i < classCount; ++i)
	{
		abc_Class& cClass = mClasses[i];
		uint32_t method_offset = in->read_V32();
		if (method_offset >= mMethods.size())
		{
			ERR((_("Action Block: Out of Bound method for class (%d).\n"),
				method_offset));
			return false;
		}
		cClass.mMethod = &mMethods[method_offset];

		uint32_t traitsCount = in->read_V32();
		cClass.mTraits.resize(traitsCount);
		for (unsigned int j = 0; j < traitsCount; ++j)
		{
			if (!cClass.mTraits[j].read(in))
				return false;
		}
	} // end of classes list

	// The scripts. TODO: Discover what these do.
	uint32_t scriptCount = in->read_V32();
	mScripts.resize(scriptCount);
	for (unsigned int i = 0; i < scriptCount; ++i)
	{
		abc_Script& script = mScripts[i];
		uint32_t method_offset = in->read_V32();
		if (method_offset >= mMethods.size())
		{
			ERR((_("Action Block: Out of Bound method for script (%d).\n"),
				method_offset));
			return false;
		}
		script.mMethod = &mMethods[method_offset];

		uint32_t traitsCount = in->read_V32();
		script.mTraits.resize(traitsCount);
		for (unsigned int j = 0; j < traitsCount; ++j)
		{
			if (!script.mTraits[j].read(in))
				return false;
		}
	}

	// The method bodies. TODO: Use these.
	uint32_t methodBodyCount = in->read_V32();
	mBodies.resize(methodBodyCount);
	for (unsigned int i = 0; i < methodBodyCount; ++i)
	{
		abc_MethodBody& method = mBodies[i];

		uint32_t method_info = in->read_V32();
		if (method_info >= mMethods.size())
		{
			ERR((_("Action Block: Out of Bound method for method body. "
				"(%d)\n"), method_info));
			return false; // Too big.
		}
		method.mMethod = &mMethods[method_info];

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
		method.mCode.resize(code_length);
		unsigned int got_length;
		if ((got_length = in->read(method.mCode.data(), code_length)) != code_length)
		{
			ERR((_("Action Block: Not enough body. Wanted %d but got %d.\n"),
				code_length, got_length));
			return false; // Not enough bytes.
		}

		// TODO: Grab code_length bytes for the code.
		uint32_t exceptions_count = in->read_V32();
		method.mExceptions.resize(exceptions_count);
		for (unsigned int j = 0; j < exceptions_count; ++j)
		{
			abc_Exception& exceptor = method.mExceptions[j];

			// Where the try block begins and ends.
			exceptor.mStart = in->read_V32();
			exceptor.mEnd = in->read_V32();
			
			// Where to go if this exception is activated.
			exceptor.mCatch = in->read_V32();
			
			// What types should be caught.
			uint32_t catch_type = in->read_V32();
			if (catch_type >= mMultinamePool.size())
			{
				ERR((_("Action Block: Out of Bound type for exception "
					"(%d).\n"), catch_type));
				return false; // Bad type.
			}
			exceptor.mType = catch_type ? &mMultinamePool[catch_type] : NULL;

			// If caught, what is the variable name.
			if (version != (46 << 16) | 15) // In version 46.15, no names.
			{
				uint32_t cvn = in->read_V32();
				if (cvn >= mMultinamePool.size())
				{
					ERR((_("Action Block: Out of Bound name for caught "
						"exception. (%d)\n"), cvn));
					return false; // Bad name
				}
				exceptor.mName = cvn ? &mMultinamePool[cvn] : NULL;
			}
			else
				exceptor.mName = NULL;
		} // End of exceptions

		uint32_t traitsCount = in->read_V32();
		method.mTraits.resize(traitsCount);
		for (unsigned int j = 0; j < traitsCount; ++j)
		{
			if (!method.mTraits[j].read(in))
				return false;
		}
	} // End of method bodies

	// Everything has been read. It needs to be verified, with symbol tables
	// built to make it all run.
	
	// If flow reaches here, everything went fine.
	return true;
}

abc_block::abc_block() : mStringTable(&VM::get().getStringTable())
{
	/**/
}

}; /* namespace gnash */

