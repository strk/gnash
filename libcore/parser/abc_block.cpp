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

//#define ERR(x) IF_VERBOSE_MALFORMED_SWF(log_swferror x;);
#define ERR(x) printf x; fflush(stdout);

namespace gnash {


namespace abc_parsing {

bool
abc_Trait::finalize(abc_block *pBlock, asClass *pClass, bool do_static)
{
	LOG_DEBUG_ABC("In finalize class name=%s trait kind=0x%X",pBlock->mStringPool[pClass->getName()],mKind | 0x0);
	switch (mKind)
	{
	case KIND_SLOT:
	case KIND_CONST:
	{
		// Validate the type.
		asClass *pType;
		if (mTypeIndex){
			LOG_DEBUG_ABC("Trait type is %s",pBlock->mStringPool[pBlock->mMultinamePool[mTypeIndex].getABCName()]);
			pType = pBlock->locateClass(pBlock->mMultinamePool[mTypeIndex]);
		}
		else{
			pType = pBlock->mTheObject;
		}
		if (!pType)
		{
			ERR((_("ABC: Finalizing trait yielded bad type for slot.\n")));
			return false;
		}
		// The name has been validated in read.
		// TODO: Find a better way to initialize trait values.
		if (!mHasValue){
			mValue = as_value((as_object*)0); // NULL value, right ?
		}
		LOG_DEBUG_ABC("Adding property=%s with value=%s slot=%u",pBlock->mStringPool[mName],mValue.toDebugString(),mSlotId);
		pClass->addValue(mGlobalName, mNamespace, mSlotId, pType, 
			mValue, mKind == KIND_CONST, do_static);
		break;
	}
	case KIND_METHOD:
	{
		pClass->addMethod(mGlobalName, mNamespace, mMethod, false);
		break;
	}
	case KIND_GETTER:
	{
		LOG_DEBUG_ABC("Finalize getter not implemented.");
		break;
		pClass->addGetter(mName, mNamespace, mMethod, do_static);
		break;
	}
	case KIND_SETTER:
	{
		LOG_DEBUG_ABC("Finalize setter not implemented.");
		break;
		pClass->addSetter(mName, mNamespace, mMethod, do_static);
		break;
	}
	case KIND_CLASS:
	{
		LOG_DEBUG_ABC("Finalize class not implemented.");
		break;
		pClass->addMemberClass(mName, mNamespace, mSlotId,
			pBlock->mClasses[mClassInfoIndex], do_static);
		break;
	}
	case KIND_FUNCTION:
	{
		LOG_DEBUG_ABC("Finalize function not implemented.");
		break;
		pClass->addSlotFunction(mName, mNamespace, mSlotId, mMethod, do_static);
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
abc_Trait::finalize_mbody(abc_block *pBlock, asMethod *pMethod)
{
	LOG_DEBUG_ABC("Finalizing method");
	switch (mKind)
	{
	case KIND_SLOT:
	case KIND_CONST:
	{
		// Validate the type.
		asClass *pType;
		if (mTypeIndex)
			pType = pBlock->locateClass(pBlock->mMultinamePool[mTypeIndex]);
		else
			pType = pBlock->mTheObject;
		if (!pType)
		{
			ERR((_("ABC: Finalizing trait yielded bad type for slot.\n")));
			return false;
		}
		// The name has been validated in read.
		// TODO: Find a better way to initialize trait values.
		if (!mHasValue){
			mValue = as_value((as_object*)0); // NULL value, right ?
		}
		LOG_DEBUG_ABC("Adding property=%s with value=%s slot=%u",pBlock->mStringPool[mName],mValue.toDebugString(),mSlotId);
		pMethod->addValue(mGlobalName, mNamespace, mSlotId, pType, 
			mValue, mKind == KIND_CONST);
		break;
	}
	case KIND_METHOD:
	{
		LOG_DEBUG_ABC("Finalize method trait not implemented.  Returning");
		break;
		pMethod->addMethod(mName, mNamespace, mMethod);
		break;
	}
	case KIND_GETTER:
	{
		LOG_DEBUG_ABC("Finalize getter trait not implemented.  Returning");
		break;
		pMethod->addGetter(mName, mNamespace, mMethod);
		break;
	}
	case KIND_SETTER:
	{
		LOG_DEBUG_ABC("Finalize setter trait not implemented.  Returning");
		break;
		pMethod->addSetter(mName, mNamespace, mMethod);
		break;
	}
	case KIND_CLASS:
	{
		LOG_DEBUG_ABC("Finalize class trait not implemented.  Returning");
		break;
		pMethod->addMemberClass(mName, mNamespace, mSlotId,
			pBlock->mClasses[mClassInfoIndex]);
		break;
	}
	case KIND_FUNCTION:
	{
		LOG_DEBUG_ABC("Finalize function trait not implemented.  Returning");
		break;
		pMethod->addSlotFunction(mName, mNamespace, mSlotId, mMethod);
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
abc_Trait::read(SWFStream* in, abc_block *pBlock)
{
	boost::uint32_t name = in->read_V32();
	if (name >= pBlock->mMultinamePool.size())
	{
		ERR((_("ABC: Bad name for trait.\n")));
		return false;
	}
	if (!pBlock->mMultinamePool[name].isQName())
	{
		ERR((_("ABC: Trait name must be fully qualified.\n")));
		return false;
	}
	asName multiname = pBlock->mMultinamePool[name];
	mName = pBlock->mMultinamePool[name].getABCName();
	mGlobalName = pBlock->mMultinamePool[name].getGlobalName();
	mNamespace = pBlock->mMultinamePool[name].getNamespace();

	boost::uint8_t kind = in->read_u8();
	mKind = static_cast<kinds> (kind & 0x0F);

	LOG_DEBUG_ABC("Trai name=%s Trait kind is 0x%X", pBlock->mStringPool[multiname.getABCName()],kind | 0x0);
	switch (mKind)
	{
	case KIND_SLOT:
	case KIND_CONST:
	{
		mSlotId = in->read_V32();
		mTypeIndex = in->read_V32();
		boost::uint32_t vindex = in->read_V32();
		LOG_DEBUG_ABC("Slot ID=%u Type=%s Pool index=%u",mSlotId,pBlock->mStringPool[pBlock->mMultinamePool[mTypeIndex].getABCName()],vindex);
		if (vindex)
		{
			if (!pBlock->pool_value(vindex, in->read_u8(), mValue))
				return false; // Message done by pool_value
			mHasValue = true;
		}
		else
			mHasValue = false;
		break;
	}
	case KIND_METHOD:
	case KIND_GETTER:
	case KIND_SETTER:
	{
		// Ignore the 'disp_id'
		in->skip_V32();

		boost::uint32_t moffset = in->read_V32();
		LOG_DEBUG_ABC("Method index=%u",moffset);
		if (moffset >= pBlock->mMethods.size())
		{
			ERR((_("Bad method id in trait.\n")));
			return false;
		}
		mMethod = pBlock->mMethods[moffset];
		break;
	}
	case KIND_CLASS:
	{
		mSlotId = in->read_V32();
		mClassInfoIndex = in->read_V32();
		LOG_DEBUG_ABC("Slot id: %u Class index: %u Class Name: %s",mSlotId,mClassInfoIndex,pBlock->mStringPool[pBlock->mClasses[mClassInfoIndex]->getName()]);
		if (mClassInfoIndex >= pBlock->mClasses.size())
		{
			ERR((_("Bad Class id in trait.\n")));
			return false;
		}
		break;
	}
	case KIND_FUNCTION:
	{
		mSlotId = in->read_V32();
		boost::uint32_t moffset = in->read_V32();
		if (moffset >= pBlock->mMethods.size())
		{
			ERR((_("Bad method id in trait.\n")));
			return false;
		}
		mMethod = pBlock->mMethods[moffset];
		break;
	}
	default:
	{
		ERR((_("ABC: Unknown type of trait.\n")));
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

} // namespace abc_parsing

using namespace abc_parsing;

void
abc_block::check_multiname_name(boost::uint32_t name){

	if (name >= mStringPool.size()){
		throw ParserException("ABC: Out of bounds string for Multiname.\n");
	}
}

void
abc_block::check_multiname_namespace(boost::uint32_t ns){
	if (ns >= mNamespacePool.size()){
		throw ParserException("ABC: Out of bounds namespace for Multiname.\n");
	}
}

void
abc_block::check_multiname_namespaceset(boost::uint32_t nsset){
	if (!nsset){
		throw ParserException("ABC: 0 selection for namespace set is invalid.\n");
	}
	if (nsset >= mNamespaceSetPool.size()){
		throw ParserException("ABC: Out of bounds namespace set for Multiname.\n");
	}
}

void
abc_block::setMultinameNames(asName *n,string_table::key ABCName){
	
	n->setABCName(ABCName);
	std::string name = mStringPool[ABCName];
	string_table::key global_key = mStringTable->find(name,true);
	LOG_DEBUG_ABC("Global key %u",global_key);
	n->setGlobalName(global_key);
	LOG_DEBUG_ABC("Multiname: %s ABCName set to %u global name set to %u",name,n->getABCName(),n->getGlobalName());
}

void
abc_block::setNamespaceURI(asNamespace *ns,string_table::key ABCName){
	
	ns->setAbcURI(ABCName);
	std::string name = mStringPool[ABCName];
	string_table::key global_key = mStringTable->find(name,false);
	ns->setURI(global_key);
	LOG_DEBUG_ABC("Namespace: %s AbcURI=%u URI=%u.",name,ABCName,global_key);
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
	if (m.mNamespaceSet && !m.mNamespaceSet->empty())
	{
		std::vector<asNamespace*>::iterator i;
		for (i = m.mNamespaceSet->begin(); i != m.mNamespaceSet->end(); ++i)
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
	ERR((_("Abc Version: %d.%d\n"), (mVersion & 0xFFFF0000) >> 16,
		(mVersion & 0x0000FFFF)));
	return true;
}

/// Read the pool of integer constants.
bool
abc_block::read_integer_constants()
{
	// count overestimates by 1.
	boost::uint32_t count = mS->read_V32();
	mIntegerPool.resize(count);
	if (count)
		mIntegerPool[0] = 0;
	for (unsigned int i = 1; i < count; ++i)
	{
		mIntegerPool[i] = static_cast<boost::int32_t> (mS->read_V32());
	}
	return true;
}

/// Read the pool of unsigned integer constants.
bool
abc_block::read_unsigned_integer_constants()
{
	// count overestimates by 1.
	boost::uint32_t count = mS->read_V32();
	mUIntegerPool.resize(count);
	if (count)
		mUIntegerPool[0] = 0;
	for (unsigned int i = 1; i < count; ++i)
	{
		mUIntegerPool[i] = mS->read_V32();
	}
	return true;
}

/// Read the pool of 64-bit double constants.
bool
abc_block::read_double_constants()
{
	boost::uint32_t count = mS->read_V32();
	mDoublePool.resize(count);
	if (count)
		mDoublePool[0] = 0.0;
	for (unsigned int i = 1; i < count; ++i)
	{
		mDoublePool[i] = mS->read_d64();
		LOG_DEBUG_ABC("Double %u=%lf",i,mDoublePool[i]);
	}
	return true;
}

/// Read the pool of string constants.
bool
abc_block::read_string_constants()
{
	LOG_DEBUG_ABC("Begin reading string constants.");
	boost::uint32_t count = mS->read_V32();
	LOG_DEBUG_ABC("There are %u string constants.",count);
	mStringPool.resize(count);
	mStringPoolTableIds.resize(count);
	if (count)
	{
		mStringPool[0] = "";
		mStringPoolTableIds[0] = 0;
	}
	for (unsigned int i = 1; i < count; ++i)
	{
		boost::uint32_t length = mS->read_V32();
		mS->read_string_with_length(length, mStringPool[i]);
		LOG_DEBUG_ABC("Adding string constant to string pool: index=%u %s",i,mStringPool[i]);
		mStringPoolTableIds[i] = 0;
	}
	return true;
}

/// Read the pool of namespaces
/// Any two namespaces with the same uri here are the same namespace,
/// excepting private namespaces.
bool
abc_block::read_namespaces()
{	
	LOG_DEBUG_ABC("Begin reading namespaces.");
	boost::uint32_t count = mS->read_V32();
	LOG_DEBUG_ABC("There are %u namespaces.",count);
	mNamespacePool.resize(count);
	if (count)
	{
		mNamespacePool[0] = mCH->getGlobalNs();
	}
	for (unsigned int i = 1; i < count; ++i)
	{
		boost::uint8_t kind = mS->read_u8();
		boost::uint32_t nameIndex = mS->read_V32();
		LOG_DEBUG_ABC("Namespace %u kind=0x%X index=%u name=%s",i,kind | 0x0,nameIndex,mStringPool[nameIndex]);

		if (nameIndex >= mStringPool.size())
		{
			ERR((_("ABC: Out of bounds string given for namespace.\n")));
			return false;
		}
		
		if (kind == PRIVATE_NS)
		{
			mNamespacePool[i] = mCH->anonNamespace(nameIndex);
			mNamespacePool[i]->setPrivate();
		}
		else
		{
			asNamespace *n = mCH->findNamespace(nameIndex);
			if (n == NULL)
				n = mCH->addNamespace(nameIndex);
			mNamespacePool[i] = n;
		}
		if (kind == PROTECTED_NS)
		{
			mNamespacePool[i]->setProtected();
		}
		setNamespaceURI(mNamespacePool[i],nameIndex);
	}
	return true;
}

/// Read the set of sets of namespaces.
bool
abc_block::read_namespace_sets()
{
	boost::uint32_t count = mS->read_V32();
	mNamespaceSetPool.resize(count);
	if (count)
	{
		mNamespaceSetPool[0].resize(0);
	}
	for (unsigned int i = 1; i < count; ++i)
	{
		boost::uint32_t icount = mS->read_V32();
		mNamespaceSetPool[i].resize(icount);
		for (unsigned int j = 0; j < icount; ++j)
		{
			boost::uint32_t selection = mS->read_V32();
			if (!selection || selection >= mNamespacePool.size())
			{
				ERR((_("ABC: Out of bounds namespace for namespace set.\n")));
				return false;
			}
			mNamespaceSetPool[i][j] = mNamespacePool[selection];
		}
	}
	return true;
}

/// Read the multinames.
bool
abc_block::read_multinames()
{
	boost::uint32_t count = mS->read_V32();
	LOG_DEBUG_ABC("There are %u multinames.",count);
	mMultinamePool.resize(count);
	if (count)
	{
//		mMultinamePool[0].setABCName(0);
		setMultinameNames(&mMultinamePool[0],0);
		mMultinamePool[0].setNamespace(mCH->getGlobalNs());
	}
	for (unsigned int i = 1; i < count; ++i)
	{
		boost::uint8_t kind = mS->read_u8();
		boost::uint32_t ns = 0;
		boost::uint32_t name = 0;
		boost::uint32_t nsset = 0;

		LOG_DEBUG_ABC("Multiname %u has kind=0x%X",i,kind | 0x0);

		// Read, but don't upper validate until after the switch.
		switch (kind)
		{
        case asName::KIND_Qname:
        case asName::KIND_QnameA:
        {
	    ns = mS->read_V32();
	    check_multiname_namespace(ns);
            name = mS->read_V32();
		check_multiname_name(name);
		LOG_DEBUG_ABC("\tnamespace_index=%u name_index=%u name=%s",ns,name,mStringPool[name]);
            break;
        }
        case asName::KIND_RTQname:
        case asName::KIND_RTQnameA:
        {
            name = mS->read_V32();
            check_multiname_name(name);
            break;
        }
        case asName::KIND_RTQnameL:
        case asName::KIND_RTQnameLA:
        {

            break;
        }
        case asName::KIND_Multiname:
        case asName::KIND_MultinameA:
        {
            name = mS->read_V32();
            check_multiname_name(name);
            nsset = mS->read_V32();
            check_multiname_namespaceset(nsset);
            break;
        }
        case asName::KIND_MultinameL:
        case asName::KIND_MultinameLA:
        {
            nsset = mS->read_V32();
            check_multiname_namespaceset(nsset);
            break;
        }
        default:
        {
            // Unknown type.
            ERR((_("Action Block: Unknown multiname type (%d).\n"), kind));
            return false;
        } // End of cases.
        } // End of switch.

		mMultinamePool[i].mFlags = kind;
		setMultinameNames(&mMultinamePool[i],name);
		LOG_DEBUG_ABC("Done setting multinames: abc=%u global=%u",mMultinamePool[i].getABCName(),mMultinamePool[i].getGlobalName());
		mMultinamePool[i].setNamespace(mNamespacePool[ns]);

		if (nsset)
			mMultinamePool[i].mNamespaceSet = &mNamespaceSetPool[nsset];
	} // End of main loop.
	return true;
}

bool
abc_block::pool_value(boost::uint32_t index, boost::uint8_t type, as_value &v)
{
	if (!index)
		return true;

	LOG_DEBUG_ABC("Pool value: index is %u type is 0x%X",index,type | 0x0);
	switch (type)
	{
	case POOL_STRING: 
	{
		if (index >= mStringPool.size())
		{
			ERR((_("Action Block: Bad index in optional argument.\n")));
			return false;
		}
		v.set_string(mStringPool[index]);
		break;
	}
	case POOL_INTEGER: 
	{
		if (index >= mIntegerPool.size())
	    {
			ERR((_("Action Block: Bad index in optional argument.\n")));
			return false;
		}
		v.set_int(mIntegerPool[index]);
		break;
	}
	case POOL_UINTEGER:
	{
		if (index >= mUIntegerPool.size())
		{
			ERR((_("Action Block: Bad index in optional argument.\n")));
			return false;
		}
		v.set_int(mUIntegerPool[index]);
		break;
	}
	case POOL_DOUBLE: 
	{
		if (index >= mDoublePool.size())
		{
			ERR((_("Action Block: Bad index in optional argument.\n")));
			return false;
		}
		v.set_double(static_cast<double>(mDoublePool[index]));
		break;
	}
	case POOL_NAMESPACE: // Namespace
	{
		if (index >= mNamespacePool.size())
		{
			ERR((_("ABC: Bad index in optional argument, namespaces.\n")));
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
		ERR((_("ABC: Bad default value type (%X), but continuing.\n"), type));
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
	LOG_DEBUG_ABC("Begin read_method_infos.\n");

	boost::uint32_t count = mS->read_V32();
    LOG_DEBUG_ABC("Method count: %u", count);

	mMethods.resize(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		LOG_DEBUG_ABC(" Reading method %u",i);
		asMethod *pMethod = mCH->newMethod();
		pMethod->mMethodID = i;
//		LOG_DEBUG_ABC("Min arg count: %d max: %d",pMethod->minArgumentCount(),pMethod->maxArgumentCount());
		mMethods[i] = pMethod;
		boost::uint32_t param_count = mS->read_V32();
		boost::uint32_t return_type = mS->read_V32();

		LOG_DEBUG_ABC("  Param count: %u return type(index): %s(%u)",param_count,mStringPool[mMultinamePool[return_type].getABCName()],return_type);
		pMethod->setMinArgumentCount(param_count);
		pMethod->setMaxArgumentCount(param_count);

		if (return_type >= mMultinamePool.size())
		{
			ERR((_("ABC: Out of bounds return type for method info.\n")));
			return false;
		}
		asClass *rtClass = locateClass(mMultinamePool[return_type]);
		if (!rtClass)
		{
			ERR((_("ABC: Unknown return type.\n")));
			return false;
		}

		pMethod->setReturnType(rtClass);

		for (unsigned int j = 0; j < param_count; ++j)
		{
			LOG_DEBUG_ABC("  Reading parameter %u",j);
			// The parameter type.
			boost::uint32_t ptype = mS->read_V32();
			LOG_DEBUG_ABC("   Parameter type(index): %s(%u)",mStringPool[mMultinamePool[ptype].getABCName()],ptype);
			if (ptype >= mMultinamePool.size())
			{
				ERR((_("ABC: Out of bounds parameter type in method.\n")));
				return false;
			}
			asClass *param_type = locateClass(mMultinamePool[ptype]);
//			LOG_DEBUG_ABC("Done creating asClass object.\n");
			if (!param_type)
			{
				ERR((_("ABC: Unknown parameter type.\n")));
				return false;
			}
//			LOG_DEBUG_ABC("Trying to add argument to method.\n");
			pMethod->pushArgument(param_type);
//			LOG_DEBUG_ABC("Done adding argument to method object.");
		}
//		LOG_DEBUG_ABC("End loop j.\n");
		// A skippable name index.
//		mS->skip_V32();
		boost::uint32_t method_name = mS->read_V32();
		LOG_DEBUG_ABC(  "Method name=%s %d",mStringPool[method_name],method_name);
		boost::uint8_t flags = mS->read_u8();
		LOG_DEBUG_ABC("  Flags: %X",flags | 0x0);
//		LOG_DEBUG_ABC("Check if flags and optional args.");
		// If there are default parameters, read them now.
		// Runtime will do validation of whether or not these can actually
		// be assigned to the corresponding parameters.
		if (flags & METHOD_OPTIONAL_ARGS)
		{
//			LOG_DEBUG_ABC("We have flags and optional args.");
			boost::uint32_t ocount = mS->read_V32();
			LOG_DEBUG_ABC("  Optional args: %u",ocount);
			pMethod->setMinArgumentCount(pMethod->maxArgumentCount() - ocount);
			for (unsigned int j = 0; j < ocount; ++j)
			{
				LOG_DEBUG_ABC("  Reading optional arg: %u",j);
				boost::uint32_t index = mS->read_V32();
				boost::uint8_t kindof = mS->read_u8();
				LOG_DEBUG_ABC("   Index: %u Kindof: %u",index,kindof);
				as_value v;
				if (!pool_value(index, kindof, v))
					return false; // message done by pool_value
				pMethod->pushOptional(v);
			}
			LOG_DEBUG_ABC("Done handling optional args.");
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
	LOG_DEBUG_ABC("There are %u instances.",count);
	mClasses.resize(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		asClass *pClass;
		//Read multiname index.
		boost::uint32_t index = mS->read_V32();
		// 0 is allowed as a name, typically for the last entry.
		if (index >= mMultinamePool.size())
		{
			ERR((_("ABC: Out of bounds instance name.\n")));
			return false;
		}
		// This must be a QName.
		if (!mMultinamePool[index].isQName())
		{
			ERR((_("ABC: QName required for instance.\n")));
			return false;
		}
		if (mMultinamePool[index].getNamespace() == NULL)
		{
			ERR((_("ABC: No namespace to use for storing class.\n")));
			return false;
		}
		pClass = locateClass(mMultinamePool[index]);
		if (!pClass)
		{
			pClass = mCH->newClass();
			if (!mMultinamePool[index].getNamespace()->addClass(
				mMultinamePool[index].getABCName(), pClass))
			{
				ERR((_("Duplicate class registration.\n")));
				return false;
			}
		}
		pClass->setDeclared();
		mClasses[i] = pClass;
		boost::uint32_t super_index = mS->read_V32();;
		if (super_index && super_index >= mMultinamePool.size())
		{
			ERR((_("ABC: Out of bounds super type.\n")));
			return false;
		}
		if (!super_index)
		{
			pClass->setSuper(mTheObject);
		}
		else
		{
			asClass *pSuper = locateClass(mMultinamePool[super_index]);
			if (!pSuper)
			{
				ERR((_("ABC: Super type not found (%s), faking.\n"),
					mStringTable->value(mMultinamePool[super_index].getABCName()).c_str()));
				// While testing, we will add a fake type, rather than abort.
				pSuper = mCH->newClass();
				pSuper->setName(mMultinamePool[super_index].getABCName());
				mCH->getGlobalNs()->addClass(mMultinamePool[super_index].getABCName(), pSuper);
				// return false;
			}

			if (pSuper->isFinal())
			{
				ERR((_("ABC: Can't extend a class which is final.\n")));
				return false;
			}

			if (pSuper->isInterface())
			{
				ERR((_("ABC: Can't extend an interface type.\n")));
				return false;
			}

			if (pSuper == pClass)
			{
				ERR((_("ABC: Class cannot be its own supertype.\n")));
				return false;
			}
			pClass->setSuper(pSuper);
			pSuper->setInherited();
		}

		boost::uint8_t flags = mS->read_u8();
		LOG_DEBUG_ABC("Instance %u multiname index=%u name=%s super index=%u flags=%X",i,index,mStringPool[mMultinamePool[index].getABCName()],super_index,flags | 0x0);

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
			if (ns_index >= mNamespacePool.size())
			{
				ERR((_("ABC: Out of bounds namespace for protected.\n")));
				return false;
			}
			// Set the protected namespace's parent, if it exists.
			if (pClass->getSuper()->hasProtectedNs())
				mNamespacePool[ns_index]->setParent(pClass->getSuper()->getProtectedNs());
			pClass->setProtectedNs(mNamespacePool[ns_index]);
		}

		// This is the list of interfaces which the instances has agreed to
		// implement. They must be interfaces, and they must exist.
		boost::uint32_t intcount = mS->read_V32();
		LOG_DEBUG_ABC("This instance has %u interfaces.",intcount);
		for (unsigned int j = 0; j < intcount; ++j)
		{
			boost::uint32_t i_index = mS->read_V32();
			LOG_DEBUG_ABC("Interface %u has multiname index=%u",i,i_index);
			// 0 is allowed as an interface, typically for the last one.
			if (i_index >= mMultinamePool.size())
			{
				ERR((_("ABC: Out of bounds name for interface.\n")));
				return false;
			}
			asClass *pInterface = locateClass(mMultinamePool[i_index]);
			// These may be undefined still, so don't check interface just yet.
			if (0) //!pInterface || !pInterface->isInterface())
			{
				ERR((_("ABC: Can't implement a non-interface type.\n")));
				return false;
			}
			pClass->pushInterface(pInterface);
		}
		// The next thing should be the constructor.
		// TODO: What does this mean exactly? How does it differ from the one in
		// the class info block?
		boost::uint32_t moffset = mS->read_V32();
		LOG_DEBUG_ABC("Moffset: %u",moffset);
		if (moffset >= mMethods.size())
		{
			ERR((_("ABC: Out of bounds method for initializer.\n")));
			return false;
		}
		// Don't validate for previous owner.
		pClass->setConstructor(mMethods[moffset]);

		/*	Calling the asMethod::setOwner always results in a segmentation fault, 
		since it tries to modify asMethod.mPrototype, which is never
		initialized.  The parser seems to work ok without this call.*/
//		mMethods[moffset]->setOwner(pClass);

		// Next come the 'traits' of the instance. (The members.)
		boost::uint32_t tcount = mS->read_V32();
		LOG_DEBUG_ABC("Trait count: %u",tcount);
		for (unsigned int j = 0; j < tcount; ++j)
		{
			abc_Trait &aTrait = newTrait();
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
	LOG_DEBUG_ABC("Begin reading classes.");
	boost::uint32_t count = mClasses.size();
	LOG_DEBUG_ABC("There are %u classes.",count);
	for (unsigned int i = 0; i < count; ++i)
	{
		asClass *pClass = mClasses[i];
		boost::uint32_t moffset = mS->read_V32();
		LOG_DEBUG_ABC("Class %u static constructor index=%u",i,moffset);
		if (moffset >= mMethods.size())
		{
			ERR((_("ABC: Out of bound static constructor for class.\n")));
			return false;
		}
		// Don't validate for previous owner.
		pClass->setStaticConstructor(mMethods[moffset]);

		/*	Calling the asMethod::setOwner always results in a segmentation fault, 
		since it tries to modify asMethod.mPrototype, which is never
		initialized.  The parser seems to work ok without this call.*/
//		mMethods[moffset]->setOwner(pClass);
		
		boost::uint32_t tcount = mS->read_V32();
		LOG_DEBUG_ABC("This class has %u traits.",tcount);
		for (unsigned int j = 0; j < tcount; ++j)
		{
			abc_Trait &aTrait = newTrait();
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
	LOG_DEBUG_ABC("Begin reading scripts.");
	boost::uint32_t count = mS->read_V32();
	LOG_DEBUG_ABC("There are %u scripts.",count);
	mScripts.resize(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		asClass *pScript = mCH->newClass();
		mScripts[i] = pScript;

		boost::uint32_t moffset = mS->read_V32();
		LOG_DEBUG_ABC("Reading script %u initializer method index=%u",i,moffset);
		if (moffset >= mMethods.size())
		{
			ERR((_("ABC: Out of bounds method for script.\n")));
			return false;
		}

		/*Calling the asMethod::setOwner always results in a segmentation fault,
		since it tries to modify asMethod.mPrototype, which is never
		initialized.  The parser seems to work ok without this call.*/
		// Don't validate for previous owner.
//		mMethods[moffset]->setOwner(pScript);

		pScript->setConstructor(mMethods[moffset]);
		pScript->setSuper(mTheObject);

		boost::uint32_t tcount = mS->read_V32();
		for (unsigned int j = 0; j < tcount; ++j)
		{
			
			abc_Trait &aTrait = newTrait();
			aTrait.set_target(pScript, false);
			if (!(aTrait.read(mS, this)))
				return false;
			LOG_DEBUG_ABC("Trait: %u name: %s(%u) kind: %u value: %s ",j,mStringPool[aTrait.mName],aTrait.mName,aTrait.mKind,aTrait.mValue.to_string());
			pScript->mTraits.push_back(aTrait);
		}
	} // end of scripts loop
	return true;
}

/// Read the method bodies and attach them to the methods.
bool
abc_block::read_method_bodies()
{
	boost::uint32_t count = mS->read_V32();
	LOG_DEBUG_ABC("There are %u method bodies.",count);
	for (unsigned int i = 0; i < count; ++i)
	{
		boost::uint32_t moffset = mS->read_V32();
		LOG_DEBUG_ABC("Method body %u method offset=%u",i,moffset);
		if (moffset >= mMethods.size())
		{
			ERR((_("ABC: Out of bounds for method body.\n")));
			return false;
		}
		if (mMethods[moffset]->getBody())
		{
			ERR((_("ABC: Only one body per method.\n")));
			return false;
		}
		//TODO: Read values.

		// Maximum stack size.
		mS->skip_V32();
		// Maximum register size.
		mMethods[moffset]->setMaxRegisters(mS->read_V32());
		// Scope depth.
		mS->skip_V32();
		// Max scope depth.
		mS->skip_V32();
		// Code length
		boost::uint32_t clength = mS->read_V32();
		mMethods[moffset]->setBodyLength(clength);
		// The code.
		//TODO: Clean this up.
		std::string body;
		mS->read_string_with_length(clength,body);

		mMethods[moffset]->setBody(new CodeStream(body));
		
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
			if (catch_type >= mMultinamePool.size())
			{
				ERR((_("ABC: Out of bound type for exception.\n")));
//				return false;
			}
			if (!catch_type)
			{
				pExcept->catchAny();
			}
			else
			{
				asClass *pType = locateClass(mMultinamePool[catch_type]);
				if (!pType)
				{
					ERR((_("ABC: Unknown type of object to catch. (%s)\n"),
						mStringTable->value(mMultinamePool[catch_type].getABCName()).c_str()));
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
				if (cvn >= mMultinamePool.size())
				{
					ERR((_("ABC: Out of bound name for caught exception.\n")));
//					return false;
				}
				pExcept->setName(mMultinamePool[cvn].getABCName());
				pExcept->setNamespace(mMultinamePool[cvn].getNamespace());
			}
		} // end of exceptions

		boost::uint32_t tcount = mS->read_V32();
		for (unsigned int j = 0; j < tcount; ++j)
		{
			abc_Trait &aTrait = newTrait();
			aTrait.set_target(mMethods[moffset]);
			if (!aTrait.read(mS, this)) // TODO: 'method body activation traits'
				return false;
			LOG_DEBUG_ABC("Trait: %u name: %s kind: %u value: %s ",j,mStringPool[aTrait.mName],aTrait.mKind,aTrait.mValue.to_string());
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
	LOG_DEBUG_ABC("Done reading unsigned integer constants.\n");
	if (!read_double_constants()) return false;
	LOG_DEBUG_ABC("Done reading double constants.\n");
	if (!read_string_constants()) return false;
	LOG_DEBUG_ABC("Done reading string constants.\n");
	if (!read_namespaces()) return false;
	LOG_DEBUG_ABC("Done reading namespaces.\n");
	if (!read_namespace_sets()) return false;
	LOG_DEBUG_ABC("Done reading namespace sets.\n");
	if (!read_multinames()) return false;
	LOG_DEBUG_ABC("Done reading multinames.\n");
	if (!read_method_infos()) return false;
	LOG_DEBUG_ABC("Done reading method infos.\n");
	if (!skip_metadata()) return false;
	LOG_DEBUG_ABC("Done reading metadata.\n");
	if (!read_instances()) return false;
	LOG_DEBUG_ABC("Done reading instances.\n");
	if (!read_classes()) return false;
	LOG_DEBUG_ABC("Done reading classes.\n");
	if (!read_scripts()) return false;
	LOG_DEBUG_ABC("Done reading scripts.\n");
	if (!read_method_bodies()) return false;
	LOG_DEBUG_ABC("Done reading stuff.\n");

	for(unsigned int i=0;i<mMethods.size();i++){
		LOG_DEBUG_ABC("Method %d body:",i);
		IF_VERBOSE_PARSE(mMethods[i]->print_body());
	}
/*	The loop below causes a segmentation fault, because it tries to modify 
	asMethod.mPrototype, which is never initialized.  The parser seems 
	to work ok without this call.*/
/*	std::vector<abc_Trait*>::iterator i = mTraits.begin();
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

asClass *abc_block::locateClass(const std::string& className){
	
	std::vector<asClass*>::iterator i = mClasses.begin();
	for( ; i!=mClasses.end(); ++i){
		if(mStringPool[(*i)->getName()].compare(className) == 0){
			return *i;
		}
	}	
	return NULL;
}

void abc_block::update_global_name(unsigned int multiname_index){
	
	asName* multiname = &mMultinamePool[multiname_index];
	string_table::key new_key = mStringTable->find(mStringPool[multiname->getABCName()],false);
	multiname->setGlobalName(new_key);	
}

abc_block::abc_block() : mStringTable(&VM::get().getStringTable())
{
	mCH = VM::get().getClassHierarchy();
	// TODO: Make this the real 'Object' prototype.
	mCH->getGlobalNs()->stubPrototype(NSV::CLASS_OBJECT);
	mTheObject = mCH->getGlobalNs()->getClass(NSV::CLASS_OBJECT);
}

} /* namespace gnash */

