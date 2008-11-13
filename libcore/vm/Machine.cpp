// Machine.cpp A machine to run AS3 code, with AS2 code in the future
//
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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
//

#include "Machine.h"
#include "as_object.h"
#include "ClassHierarchy.h"
#include "namedStrings.h"
#include "array.h"
#include "abc_block.h"
#include "fn_call.h"
#include "abc_function.h"
#include "action.h"
#include "Object.h"
#include "VM.h"

//#define PRETEND
namespace gnash {
/// The type of exceptions thrown by ActionScript.
class ASException
{
public:
	as_value mValue;

	ASException(as_value &v) { mValue = v; }
	ASException() { mValue.set_undefined(); }
};

class ASReferenceError : public ASException
{
public:
	ASReferenceError() : ASException()
	{/**/}
};

class ASTypeError : public ASException
{
public:
	ASTypeError() : ASException()
	{/**/}
};

// Functions for getting pool constants.
static inline std::string& pool_string(boost::uint32_t index, abc_block *pool)
{
	if (!pool)
		throw ASException();
	return pool->mStringPool.at(index);
}

static inline int pool_int(boost::uint32_t index, abc_block *pool)
{
	if (!pool)
		throw ASException();
	return pool->mIntegerPool.at(index);
}

static inline unsigned int pool_uint(boost::uint32_t index, abc_block *pool)
{
	if (!pool)
		throw ASException();
	return pool->mUIntegerPool.at(index);
}

static inline double pool_double(boost::uint32_t index, abc_block *pool)
{
	if (!pool)
		throw ASException();
	return pool->mDoublePool.at(index);
}

static inline asNamespace* pool_namespace(boost::uint32_t index, abc_block *pool)
{
	if (!pool)
		throw ASException();
	return pool->mNamespacePool.at(index);
}

static inline asMethod* pool_method(boost::uint32_t index, abc_block* pool)
{
	if (!pool)
		throw ASException();
	return pool->mMethods.at(index);
}

static inline asClass* pool_class(boost::uint32_t index, abc_block* pool)
{
	if (!pool)
		throw ASException();
	return pool->mClasses.at(index);
}

// Don't make this a reference or you'll taint the pool.
static inline asName pool_name(boost::uint32_t index, abc_block* pool)
{
	if (!pool)
		throw ASException();
	asName multiname = pool->mMultinamePool.at(index);
	LOG_DEBUG_AVM("Searching multiname pool for property id=%u abc name=%u global name = %u abc string=%s flags=0x%X name_space=%u",index,multiname.getABCName(),multiname.getGlobalName(),pool->mStringPool[multiname.getABCName()],multiname.mFlags | 0x0,multiname.getNamespace()->getURI());
	return multiname;
}

/// ENSURE_NUMBER makes sure that the given argument is a number,
/// calling the valueOf method if necessary -- it's a macro so that
/// the valueOf method may be pushed if needed, and then whatever
/// opcode asked for this will be re-entered.
#define ENSURE_NUMBER(vte)													\
{																			\
	as_value *e = &vte;														\
	if (e->is_object())														\
	{																		\
		Property *b = e->to_object()->findProperty(NSV::PROP_VALUE_OF, 0);	\
		if (b)																\
		{																	\
			mStream->seekTo(opStart);										\
			pushGet(e->to_object().get(), *e, b);							\
			break;															\
		}																	\
	}																		\
}												   /* end of ENSURE_NUMBER */

/// ENSURE_OBJECT will throw an exception if the argument isn't an
/// object. It's a macro to match with the other ENSURE_ macros.
#ifdef PRETEND
#define ENSURE_OBJECT(vte)													\
{																			\
	if (!vte.is_object())													\
		throw ASException();												\
}												   /* end of ENSURE_OBJECT */
#else
#define ENSURE_OBJECT(vte)
#endif

/// ENSURE_STRING makes sure that the given argument is a string,
/// calling the toString method if necessary -- it's a macro so that
/// the toString may be pushed if needed, and then whatever opcode
/// asked for this will be re-entered.
#define ENSURE_STRING(vte)													\
{																			\
	as_value *c = &vte; /* Don't call vte multiple times */					\
	if (c->is_object())														\
	{																		\
		Property *d = c->to_object()->findProperty(NSV::PROP_TO_STRING, 0);	\
		if (d)																\
		{																	\
			mStream->seekTo(opStart);										\
			pushGet(c->to_object().get(), *c, d);							\
			break;															\
		}																	\
	}																		\
}												   /* end of ENSURE_STRING */

/// ABSTRACT_COMPARE is the abstract comparison as described in the ECMA
/// standard.  The 'truth_of_undefined' is used to specify which value
/// should be set for NaN values. It's a macro so that calls may be
/// pushed in the ENSURE_STRING and ENSURE_NUMBER macros.
#define ABSTRACT_COMPARE(store, rv1, rv2, truth_of_undefined) 				\
{ 																			\
	as_value &a = rv1; /* Don't call rv1 multiple times */					\
	as_value &b = rv2; /* Don't call rv2 multiple times */					\
	if (a.ptype() == PTYPE_STRING && b.ptype() == PTYPE_STRING) 			\
	{ 																		\
		ENSURE_STRING(a); 													\
		ENSURE_STRING(b); 													\
		store = a.to_string() < b.to_string(); 								\
	} 																		\
	else 																	\
	{ 																		\
		ENSURE_NUMBER(a); 													\
		ENSURE_NUMBER(b); 													\
		double ad = a.to_number(); double bd = b.to_number();				\
		if (isNaN(ad) || isNaN(bd))											\
			store = truth_of_undefined; 									\
		else if (isinf(ad) && ad > 0)	 									\
			store = false; 													\
		else if (isinf(bd) && bd > 0)	 									\
			store = true; 													\
		else if (isinf(bd) && bd < 0)	 									\
			store = false; 													\
		else if (isinf(ad) && ad < 0)										\
			store = true;													\
		else 																\
			store = ad < bd; 												\
	} 																		\
}												/* end of ABSTRACT_COMPARE */

inline bool abstractEquality(const as_value& a, const as_value& b,
       bool strictness_on)
{
    if ( strictness_on ) return a.strictly_equals(b);
    else return a.equals(b);
}								

#define ABSTRACT_TYPELATE(st, checkval, matchval)							\
{																			\
	bool *store = &st;														\
	as_value &a = checkval; /* Don't call checkval multiple times */		\
	as_value &b = matchval; /* Don't call matchval multiple times */		\
	*store = true;															\
	if (b.is_object())														\
	{																		\
		as_value v;															\
		b.to_object()->get_member(NSV::INTERNAL_TYPE, &v);					\
		if (!a.conforms_to(mST.find(v.to_string())))						\
			*store = false;													\
	}																		\
	else if (b.is_string())													\
	{																		\
		if (!a.conforms_to(mST.find(b.to_string())))						\
			*store = false;													\
	}																		\
	else																	\
		*store = false;														\
}											   /* end of ABSTRACT_TYPELATE */

#define JUMPIF(jtruth)														\
{																			\
	boost::int32_t jumpOffset = mStream->read_S24();								\
	if (jtruth)																\
		mStream->seekBy(jumpOffset);										\
	break;																	\
}														  /* end of JUMPIF */

void
Machine::execute()
{
	boost::uint8_t opcode;
	
	for ( ; ; )
	{
		std::size_t opStart = mStream->tellg();
//		std::size_t opStart = mStreamStack.top(0)->tell();
	if (1/*mIsAS3*/)
	{
//	opcode = mStreamStack.top(0)->read_as3op();
	opcode = mStream->read_as3op();
	LOG_DEBUG_AVM("** Executing opcode: %X **",opcode | 0x0);
//	continue;
	switch ((opcode /*= mStream->read_as3op()*/)) // Assignment intentional
	{
	default:
		throw ASException();
		break;
	case 0:
	{
// This is not actually an opcode -- it occurs when the stream is
// empty. We may need to return from a function, or we may be done.
//		break;
		return;
	}
/// 0x01 ABC_ACTION_BKPT
/// Do: Enter the debugger if one has been invoked.
/// This is a no-op. Enable it if desired.
/// 0x02 ABC_ACTION_NOP
/// Do: Nothing.
/// 0xF3 ABC_ACTION_TIMESTAMP
	case SWF::ABC_ACTION_NOP:
	case SWF::ABC_ACTION_BKPT:
	case SWF::ABC_ACTION_TIMESTAMP:
	{
		break;
	}
/// 0x03 ABC_ACTION_THROW
/// Stack In:
///  obj -- an object
/// Stack Out:
///  .
/// Do: Throw obj as an exception
/// Equivalent: ACTIONTHROW
	case SWF::ABC_ACTION_THROW:
	{
		throw ASException(mStack.pop());
		break;
	}
/// 0x04 ABC_ACTION_GETSUPER
/// Stream:
///  name_id -- V32 index to multiname 'name'
/// Stack In:
///  [ns [n]] -- Namespace stuff.
///  obj -- an object
/// Stack Out:
///  obj.super.name
///  May be the same as the value of obj.name (E.g. inherited variables)
	case SWF::ABC_ACTION_GETSUPER:
	{
		// Get the name.
		asName a = pool_name(mStream->read_V32(), mPoolObject);
		// Finish it, if necessary.
		mStack.drop(completeName(a));
		// Get the target object.
		ENSURE_OBJECT(mStack.top(0));
		as_object *super = mStack.top(0).to_object()->get_prototype().get();
		// If we don't have a super, throw.
		if (!super)
			throw ASReferenceError();
		Property *b = super->findProperty(a.getABCName(), 
			a.getNamespace()->getURI());
		// The object is on the top already.
		pushGet(super, mStack.top(0), b);
		break;
	}
/// 0x05 ABC_ACTION_SETSUPER
/// Stream: UV32 index to multiname 'name'
/// Stack In:
///  val -- an object
///  [ns [n]] -- Namespace stuff.
///  obj -- an object
/// Stack Out:
///  .
/// Do: Set obj.super.name to val, if allowable.
	case SWF::ABC_ACTION_SETSUPER:
	{
		// Get and finish the name.
		asName a = pool_name(mStream->read_V32(), mPoolObject);
		as_value vobj = mStack.pop(); // The value

		mStack.drop(completeName(a));

		ENSURE_OBJECT(mStack.top(0));
		as_object* super = mStack.pop().to_object()->get_prototype().get();
		if (!super)
			throw ASReferenceError();
		Property* b = super->findProperty(a.getABCName(), 
			a.getNamespace()->getURI());
		mStack.push(vobj);
		pushSet(super, vobj, b);
		break;
	}
/// 0x06 ABC_ACTION_DXNS
/// Default XML Namespace
/// Stream: UV32 index to string pool 'nsname'
/// Do: Create a new public namespace with name nsname, and make this the
///  default XML namespace. 
	case SWF::ABC_ACTION_DXNS:
	{
		boost::uint32_t soffset = mStream->read_V32();
		std::string& uri = pool_string(soffset, mPoolObject);
		mDefaultXMLNamespace = mCH->anonNamespace(mST.find(uri));
		break;
	}
/// 0x07 ABC_ACTION_DXNSLATE
/// Stack In:
///  nsname -- a string object
/// Stack Out:
///  .
/// Do: Same as ABC_ACTION_DXNS, but the uri is in the stack, not the stream.
	case SWF::ABC_ACTION_DXNSLATE:
	{
		ENSURE_STRING(mStack.top(0));
		const std::string& uri = mStack.top(0).to_string();
		mDefaultXMLNamespace = mCH->anonNamespace(mST.find(uri));
		mStack.drop(1);
		break;
	}
/// 0x08 ABC_ACTION_KILL
/// Stream: UV32 frame pointer offset 'offset'
/// Frame: 
///  Kill at offset
/// Equivalent: ACTION_DELETE
	case SWF::ABC_ACTION_KILL:
	{
		boost::uint32_t regNum = mStream->read_V32();
		mRegisters[regNum].set_undefined();
		break;
	}
/// 0x09 ABC_ACTION_LABEL
/// Do: Unknown purpose, Tamarin does nothing.
	case SWF::ABC_ACTION_LABEL:
	{
		break;
	}
/// 0x0C ABC_ACTION_IFNLT
/// Stream: S24 jump offset 'jump'
/// Stack In:
///  b -- an object
///  a -- an object
/// Stack Out:
///  .
/// Do: If !(a < b) move by jump in stream, as ABC_ACTION_JUMP does.
	case SWF::ABC_ACTION_IFNLT:
	{
		bool truth;
		ABSTRACT_COMPARE(truth, mStack.top(1), mStack.top(0), false);
		mStack.drop(2);
		JUMPIF(!truth); // truth is: a < b
		break;
	}
/// 0x0D ABC_ACTION_IFNLE
/// Stream: S24 jump offset 'jump'
/// Stack In:
///  b -- an object
///  a -- an object
/// Stack Out:
///  .
/// Do: If !(a <= b) move by jump in stream, as ABC_ACTION_JUMP does.
	case SWF::ABC_ACTION_IFNLE:
	{
		bool truth;
		ABSTRACT_COMPARE(truth, mStack.top(0), mStack.top(1), true);
		mStack.drop(2);
		JUMPIF(truth); // truth is: b < a
		break;
	}
/// 0x0E ABC_ACTION_IFNGT
/// Stream: S24 jump offset 'jump'
/// Stack In:
///  b -- an object
///  a -- an object
/// Stack Out:
///  .
/// Do: If !(a > b) move by jump in stream, as ABC_ACTION_JUMP does.
	case SWF::ABC_ACTION_IFNGT:
	{
		bool truth;
		ABSTRACT_COMPARE(truth, mStack.top(0), mStack.top(1), false);
		mStack.drop(2);
		JUMPIF(!truth); // truth is: b < a
		break;
	}
/// 0x0F ABC_ACTION_IFNGE
/// Stream: S24 jump offset 'jump'
/// Stack In:
///  a -- an object
///  b -- an object
/// Stack Out:
///  .
/// Do: If !(a >= b) move by jump in stream, as ABC_ACTION_JUMP does.
	case SWF::ABC_ACTION_IFNGE:
	{
		bool truth;
		ABSTRACT_COMPARE(truth, mStack.top(1), mStack.top(0), true);
		mStack.drop(2);
		JUMPIF(truth); // truth is: a < b
		break;
	}
/// 0x10 ABC_ACTION_JUMP
/// Stream: S24 jump offset 'jump'
/// Do: If jump is negative, check for interrupts. Move by jump in stream.
/// Equivalent: ACTION_BRANCHALWAYS
	case SWF::ABC_ACTION_JUMP:
	{
		boost::int32_t bytes = mStream->read_S24();
		LOG_DEBUG_AVM("Jumping %d bytes.",bytes);
		mStream->seekBy(bytes);

		break;
	}
/// 0x11 ABC_ACTION_IFTRUE
/// Stream: S24 jump offset 'jump'
/// Stack In:
///  a -- an object
/// Stack Out:
///  .
/// Do: If a is 'true', move by jump in stream, as ABC_ACTION_JUMP does.
/// Equivalent: ACTION_BRANCHIFTRUE
	case SWF::ABC_ACTION_IFTRUE:
	{
		boost::int32_t bytes = mStream->read_S24();
		if(pop_stack().to_bool()){
			LOG_DEBUG_AVM("Jumping %d bytes.",bytes);
			mStream->seekBy(bytes);
		}
		else{
			LOG_DEBUG_AVM("Would have jumpied %d bytes.", bytes);
		}
		break;
	}
/// 0x12 ABC_ACTION_IFFALSE
/// Stream: S24 jump offset 'jump'
/// Stack In:
///  a -- an object
/// Stack Out:
///  .
/// Do: If a is 'false', move by jump in stream, as ABC_ACTION_JUMP does.
	case SWF::ABC_ACTION_IFFALSE:
	{
		boost::int32_t bytes = mStream->read_S24();
		bool truth = pop_stack().to_bool();
		if(!truth){
			LOG_DEBUG_AVM("Jumping...");
			LOG_DEBUG_AVM("%d bytes.",bytes);
			mStream->seekBy(bytes);
		}
		break;
	}
/// 0x13 ABC_ACTION_IFEQ
/// Stream: S24 jump offset 'jump'
/// Stack In:
///  b -- an object
///  a -- an object
/// Stack Out:
///  .
/// Do: If a == b (weakly), move by jump in stream, as ABC_ACTION_JUMP does.
	case SWF::ABC_ACTION_IFEQ:
	{
		boost::int32_t bytes = mStream->read_S24();
		as_value b = pop_stack();
		as_value a = pop_stack();
		if(a.equals(b)){
			LOG_DEBUG_AVM("Jumping %d bytes.",bytes);
			mStream->seekBy(bytes);
		}
		else{
			LOG_DEBUG_AVM("Would have jumped %d bytes", bytes);
		}
		break;
	}
/// 0x14 ABC_ACTION_IFNE
/// Stream: S24 jump offset 'jump'
/// Stack In:
///  b -- an object
///  a -- an object
/// Stack Out:
///  .
/// Do: If a != b (weakly), move by jump in stream, as ABC_ACTION_JUMP does.
	case SWF::ABC_ACTION_IFNE:
	{
		as_value a = pop_stack();
		as_value b = pop_stack();
		boost::int32_t bytes = mStream->read_S24();
		if(!a.equals(b)){
			LOG_DEBUG_AVM("Jumping... %d bytes.",bytes);
			mStream->seekBy(bytes);
		}
		else{
			LOG_DEBUG_AVM("Would have jumped %d bytes",bytes);
		}
		break;
	}
/// 0x15 ABC_ACTION_IFLT
/// Stream: S24 jump offset 'jump'
/// Stack In:
///  b -- an object
///  a -- an object
/// Stack Out:
///  .
/// Do: If a < b move by jump in stream, as ABC_ACTION_JUMP does.
	case SWF::ABC_ACTION_IFLT:
	{
		as_value b = pop_stack();
		as_value a = pop_stack();
		boost::int32_t bytes = mStream->read_S24();
		bool jump = a.newLessThan(b).to_bool();
		if(jump){
			LOG_DEBUG_AVM("Jumping... %d bytes.",bytes);
			mStream->seekBy(bytes);
		}
		else{
			LOG_DEBUG_AVM("Would have jumped %d bytes",bytes);
		}
		break;
	}
/// 0x16 ABC_ACTION_IFLE
/// Stream: S24 jump offset 'jump'
/// Stack In:
///  b -- an object
///  a -- an object
/// Stack Out:
///  .
/// Do: If a <= b move by jump in stream, as ABC_ACTION_JUMP does.
	case SWF::ABC_ACTION_IFLE:
	{
		bool truth;
		ABSTRACT_COMPARE(truth, mStack.top(0), mStack.top(1), true);
		mStack.drop(2);
		JUMPIF(!truth); // truth is: b < a
		break;
	}
/// 0x17 ABC_ACTION_IFGT
/// Stream: S24 jump offset 'jump'
/// Stack In:
///  b -- an object
///  a -- an object
/// Stack Out:
///  .
/// Do: If a > b move by jump in stream, as ABC_ACTION_JUMP does.
	case SWF::ABC_ACTION_IFGT:
	{
		boost::int32_t bytes = mStream->read_S24();
		bool truth;
		// If b < a, then a > b, with undefined as false
		ABSTRACT_COMPARE(truth, mStack.top(0), mStack.top(1), false);
		mStack.drop(2);
		if(truth){
			LOG_DEBUG_AVM("Jumping %d bytes.",bytes);
			mStream->seekBy(bytes);
		}
		else{
			LOG_DEBUG_AVM("Would have jumped %d bytes.",bytes);
		}
		break;
	}
/// 0x18 ABC_ACTION_IFGE
/// Stream: S24 jump offset 'jump'
/// Stack In:
///  b -- an object
///  a -- an object
/// Stack Out:
///  .
/// Do: If a >= b move by jump in stream, as ABC_ACTION_JUMP does.
	case SWF::ABC_ACTION_IFGE:
	{
		bool truth;
		ABSTRACT_COMPARE(truth, mStack.top(0), mStack.top(1), true);
		mStack.drop(2);
		JUMPIF(!truth); // truth is: a < b
		break;
	}
/// 0x19 ABC_ACTION_IFSTRICTEQ
/// Stream: S24 jump offset 'jump'
/// Stack In:
///  b -- an object
///  a -- an object
/// Stack Out:
///  .
/// Do: If a == b (strictly), move by jump in stream, as ABC_ACTION_JUMP
	case SWF::ABC_ACTION_IFSTRICTEQ:
	{
		bool truth = abstractEquality(mStack.top(1), mStack.top(0), true);
		mStack.drop(2);
		JUMPIF(truth);
		break;
	}
/// 0x1A ABC_ACTION_IFSTRICTNE
/// Stream: S24 jump offset 'jump'
/// Stack In:
///  b -- an object
///  a -- an object
/// Stack Out:
///  .
/// Do: If a != b (strongly), move by jump in stream, as ABC_ACTION_JUMP
	case SWF::ABC_ACTION_IFSTRICTNE:
	{
		bool truth = abstractEquality(mStack.top(1), mStack.top(0), true);
		mStack.drop(2);
		JUMPIF(!truth);
		break;
	}
/// 0x18 ABC_ACTION_LOOKUPSWITCH
/// Stream: 3 bytes | V32 count as 'case_count - 1' | case_count of S24
///  as 'cases'
/// Stack In:
///  index -- an integer object
/// Stack Out:
///  .
/// Do: If index >= case_count, reset stream to position on op entry.
///  Otherwise, move by cases[index] - 1 from stream position on op entry.
	case SWF::ABC_ACTION_LOOKUPSWITCH:
	{
		std::size_t npos = mStream->tellg();
		if (!mStack.top(0).is_number())
			throw ASException();

		boost::uint32_t index = mStack.top(0).to_number<boost::uint32_t>();
		mStack.drop(1);

		mStream->seekBy(3); // Skip the intial offset.
		boost::uint32_t cases = mStream->read_V32();
		// Read from our original position and use it to skip if the case
		// is out of range.
		if (index > cases)
		{
			mStream->seekTo(npos);
			mStream->seekTo(npos + mStream->read_S24());
		}
		else
		{
			mStream->seekTo(npos + 3 * (index + 1));
			boost::uint32_t newpos = mStream->read_S24();
			mStream->seekTo(npos - 1 + newpos);
		}
		break;
	}
/// 0x30 ABC_ACTION_PUSHSCOPE
	case SWF::ABC_ACTION_PUSHSCOPE:
	{
		push_scope_stack(pop_stack());
		break;
	}
/// 0x1C ABC_ACTION_PUSHWITH
/// Stack In:
///  scope -- a scope
/// Stack Out:
///  .
/// Do: Enter scope with previous scope as its base.
/// If 0x1C, start a new base if the previous one was global.
	case SWF::ABC_ACTION_PUSHWITH:
	{
		// A scope object is just a regular object.
// 		ENSURE_OBJECT(mStack.top(0));
// 		as_object *a = mStack.top(0).to_object().get();
// 
// 		if (!mScopeStack.empty())
// 			a->set_prototype(mScopeStack.top(0).mScope);
// 		else
// 			a->set_prototype(NULL);
// 
// 		if (opcode == SWF::ABC_ACTION_PUSHWITH &&
// 				mScopeStack.totalSize() == mScopeStack.size())
// 		{
// 			mScopeStack.push(Scope(0, a));
// 		}
// 		else
// 		{
// 			mScopeStack.push(Scope(mScopeStack.size(), a));
// 		}
// 		mCurrentScope = a;
// 		mStack.drop(1);
		break;
	}
/// 0x1D ABC_ACTION_POPSCOPE
/// Do: exit current scope. Clear the base if the depth is now 
///  shallower than the base's depth.
	case SWF::ABC_ACTION_POPSCOPE:
	{
//		LOG_AVM_UNIMPLEMENTED();
		pop_scope_stack();
// 		Scope &s = mScopeStack.pop();
// 		mScopeStack.setDownstop(s.mHeightAfterPop);
// 		if (mScopeStack.empty())
// 			mCurrentScope = NULL;
// 		else
// 			mCurrentScope = mScopeStack.top(0).mScope;
		break;
	}
/// 0x1E ABC_ACTION_NEXTNAME
/// Stack In:
///  index -- an integer object
///  obj -- an object
/// Stack Out:
///  name -- the key name of the property at index in obj
	case SWF::ABC_ACTION_NEXTNAME:
	{
		ENSURE_NUMBER(mStack.top(0));
		ENSURE_OBJECT(mStack.top(1));
		as_object *obj = mStack.top(1).to_object().get();
		boost::uint32_t index = mStack.top(0).to_number<boost::uint32_t>();
		mStack.drop(1);
		const Property *b = obj->getByIndex(index);
		if (b)
			mStack.top(0) = mST.value(b->getName());
		else
			mStack.top(0) = "";
		break;
	}
/// 0x1F ABC_ACTION_HASNEXT
/// Stack In:
///  index -- an integer object
///  obj -- an object
/// Stack Out:
///  next_index -- next index after index in obj, or 0 if none.
/// Do: If the index is 0, return the first logical property.
/// We'll do this by name, since the name id can be used for this
/// directly.
	case SWF::ABC_ACTION_HASNEXT:
	{
		ENSURE_NUMBER(mStack.top(0));
		ENSURE_OBJECT(mStack.top(1));
		as_object *obj = mStack.top(1).to_object().get();
		boost::uint32_t index = mStack.top(0).to_number<boost::uint32_t>();
		mStack.drop(1);
		mStack.top(0) = obj->nextIndex(index);
		break;
	}
/// 0x20 ABC_ACTION_PUSHNULL
/// Stack Out:
///  n -- a Null object.
	case SWF::ABC_ACTION_PUSHNULL:
	{
		as_value value = as_value();
		value.set_null();
		push_stack(value);
		break;
	}
/// 0x21 ABC_ACTION_PUSHUNDEFINED
/// Stack Out:
///  n -- an Undefined object.
	case SWF::ABC_ACTION_PUSHUNDEFINED:
	{
		mStack.grow(1);
		mStack.top(0).set_undefined();
		break;
	}
/// 0x23 ABC_ACTION_NEXTVALUE
/// Stack In:
///  index -- an integer object
///  obj -- an object (namespaces okay)
/// Stack Out:
///  value -- the value of the key value pair in obj at index.
	case SWF::ABC_ACTION_NEXTVALUE:
	{
		ENSURE_NUMBER(mStack.top(0));
		ENSURE_OBJECT(mStack.top(1));
		as_object *obj = mStack.top(1).to_object().get();
		boost::uint32_t index = mStack.top(0).to_number<boost::uint32_t>();
		const Property *b = obj->getByIndex(index);
		mStack.drop(1);
		if (!b)
			mStack.top(0).set_undefined();
		else
		{
			mStack.drop(1);
			pushGet(obj, mStack.top(0), const_cast<Property*>(b));
		}
		break;
	}
/// 0x24 ABC_ACTION_PUSHBYTE
/// Stream: S8 as 'byte'
/// Stack Out:
///  byte -- as a raw byte
	case SWF::ABC_ACTION_PUSHBYTE:
	{
		int8_t b = mStream->read_s8();
		push_stack(as_value(b));
		break;
	}
/// 0x25 ABC_ACTION_PUSHSHORT
/// Stream: V32 as 'value'
/// Stack Out:
///  value -- as a raw integer
	case SWF::ABC_ACTION_PUSHSHORT:
	{
		signed short s = static_cast<signed short>(mStream->read_V32());
		push_stack(as_value(s));
		break;
	}
/// 0x26 ABC_ACTION_PUSHTRUE
/// Stack Out:
///  true -- the True object
	case SWF::ABC_ACTION_PUSHTRUE:
	{
		mStack.grow(1);
		mStack.top(0).set_bool(true);
		break;
	}
/// 0x27 ABC_ACTION_PUSHFALSE
/// Stack Out:
///  false -- the False object
	case SWF::ABC_ACTION_PUSHFALSE:
	{
		push_stack(as_value(false));
		break;
	}
/// 0x28 ABC_ACTION_PUSHNAN
/// Stack Out:
///  NaN -- the NaN object
	case SWF::ABC_ACTION_PUSHNAN:
	{
		mStack.grow(1);
		mStack.top(0).set_nan();
		break;
	}
/// 0x29 ABC_ACTION_POP
/// Stack In:
///  a -- anything
/// Stack Out:
///  .
	case SWF::ABC_ACTION_POP:
	{
		pop_stack();
		break;
	}
/// 0x2A ABC_ACTION_DUP
/// Stack In:
///  a -- anything
/// Stack Out:
///  a
///  a
	case SWF::ABC_ACTION_DUP:
	{
		mStack.grow(1);
		mStack.top(0) = mStack.top(1);
		break;
	}
/// 0x2B ABC_ACTION_SWAP
/// Stack In:
///  a -- anything
///  b -- anything
/// Stack Out:
///  b
///  a
	case SWF::ABC_ACTION_SWAP:
	{
		as_value inter = mStack.top(0);
		mStack.top(0) = mStack.top(1);
		mStack.top(1) = inter;
		break;
	}
/// 0x2C ABC_ACTION_PUSHSTRING
/// Stream: V32 string pool index 'index'
/// Stack Out:
///  value -- String object from string_pool[index]
	case SWF::ABC_ACTION_PUSHSTRING:
	{
		push_stack(as_value(pool_string(mStream->read_V32(), mPoolObject)));
		break;
	}
/// 0x2D ABC_ACTION_PUSHINT
/// Stream: V32 int pool index 'index'
/// Stack Out:
///  value -- Integer object from integer_pool[index]
	case SWF::ABC_ACTION_PUSHINT:
	{
		push_stack(pool_int(mStream->read_V32(), mPoolObject));
		break;
	}
/// 0x2E ABC_ACTION_PUSHUINT
/// Stream: V32 uint pool index 'index'
/// Stack Out:
///  value -- Unsigned Integer object from unsigned_integer_pool[index]
	case SWF::ABC_ACTION_PUSHUINT:
	{
		mStack.grow(1);
		mStack.top(0) = pool_uint(mStream->read_V32(), mPoolObject);
		break;
	}
/// 0x2F ABC_ACTION_PUSHDOUBLE
/// Stream: V32 double pool index 'index'
/// Stack Out:
///  value -- Double object from double_pool[index]
	case SWF::ABC_ACTION_PUSHDOUBLE:
	{
		mStack.grow(1);
		mStack.top(0) = pool_double(mStream->read_V32(), mPoolObject);
		break;
	}
/// 0x31 ABC_ACTION_PUSHNAMESPACE
/// Stream: V32 namespace pool index 'index'
/// Stack Out:
///  ns -- Namespace object from namespace_pool[index]
	case SWF::ABC_ACTION_PUSHNAMESPACE:
	{
		asNamespace *ns = pool_namespace(mStream->read_V32(), mPoolObject);
		mStack.grow(1);
		mStack.top(0) = *ns;
		break;
	}
/// 0x32 ABC_ACTION_HASNEXT2
/// Stream: V32 frame location 'objloc' | V32 frame location 'indexloc'
/// Stack Out:
///  truth -- True if frame[objloc] has key/val pair after frame[indexloc],
///   following delagates (__proto__) objects if needed. False, otherwise.
/// Frame:
///  Change at objloc to object which possessed next value.
///  Change at indexloc to index (as object) of the next value.
/// N.B.: A value of '0' for indexloc initializes to the first logical
/// property.
	case SWF::ABC_ACTION_HASNEXT2:
	{
		boost::int32_t oindex = mStream->read_V32();
		boost::int32_t iindex = mStream->read_V32();
		as_value &objv = mRegisters[oindex];
		as_value &indexv = mRegisters[iindex];
		LOG_DEBUG_AVM("Index is %u",indexv.to_number());
//		ENSURE_OBJECT(objv);
//		ENSURE_NUMBER(indexv);
		as_object *obj = objv.to_object().get();
		boost::uint32_t index = indexv.to_number<boost::uint32_t>();
		LOG_DEBUG_AVM("Object is %s index is %u",objv.toDebugString(),index);
		as_object *owner = NULL;
		int next = obj->nextIndex(index, &owner);
		LOG_DEBUG_AVM("Next index is %d",next);
//		mStack.grow(1);
		if (next)
		{
//			mStack.top(0).set_bool(true);
			push_stack(as_value(true));
			if (owner)
				mRegisters[oindex] = owner;
			else
				mRegisters[oindex].set_null();
			mRegisters[iindex] = next;
		}
		else
		{
			push_stack(as_value(false));
			mRegisters[oindex].set_null();
			mRegisters[iindex] = 0;
		}
		break;
	}
/// 0x40 ABC_ACTION_NEWFUNCTION
/// Stream: V32 'index'
/// Stack Out:
///  func -- the function object
/// Do: Information about function is in the pool at index. Construct the
///  function from this information and bind the current scope.
	case SWF::ABC_ACTION_NEWFUNCTION:
	{
		boost::int32_t method_index = mStream->read_V32();
		LOG_DEBUG_AVM("Creating new abc_function: method index=%u",method_index);
		asMethod *m = pool_method(method_index, mPoolObject);
		abc_function* new_function = m->getPrototype();
		//TODO: SafeStack contains all the scope objects in for all functions in the call stack.
		//We should only copy the relevent scope objects to the function's scope stacks.
		new_function->mScopeStack = getScopeStack();
		push_stack(as_value(new_function));
		break;
	}
/// 0x41 ABC_ACTION_CALL
/// Stream: V32 'arg_count'
/// Stack In:
///  argN ... arg1 -- the arg_count arguments to pass
///  obj -- the object to which the function belongs
///  func -- the function to be called
/// Stack Out:
///  value -- the value returned by obj->func(arg1, ...., argN)
	case SWF::ABC_ACTION_CALL:
	{
		boost::uint32_t argc = mStream->read_V32();
		ENSURE_OBJECT(mStack.top(argc + 1)); // The func
		ENSURE_OBJECT(mStack.top(argc)); // The 'this'
		as_function *f = mStack.top(argc + 1).to_as_function();
		as_object *obj = mStack.top(argc).to_object().get();
		// We start with argc + 2 values related to this call
		// on the stack. We want to end with 1 value. We pass
		// argc values (the parameters), so we need to drop
		// one more than we pass and store the return just
		// below that one. Thus:
		// return is mStack.top(argc + 1)
		// bottom of arguments is argc deep
		// drop 1 more value than is passed, on return
		pushCall(f, obj, mStack.top(argc + 1), argc, -1);
		break;
	}
/// 0x42 ABC_ACTION_CONSTRUCT
/// Stream: V32 'arg_count'
/// Stack In:
///  argN ... arg1 -- the arg_count arguments to pass
///  function -- constructor for the object to be constructed
/// Stack Out:
///  value -- obj after it has been constructed as obj(arg1, ..., argN)
	case SWF::ABC_ACTION_CONSTRUCT:
	{
		boost::uint32_t argc = mStream->read_V32();
		as_function *f = mStack.top(argc).to_as_function();
		Property b(0, 0, f, NULL);
		pushCall(f, NULL, mStack.top(argc), argc, 0);
		break;
	}
/// 0x43 ABC_ACTION_CALLMETHOD
/// Stream: V32 'method_id + 1' | V32 'arg_count'
/// Stack In:
///  argN ... arg1 -- the arg_count arguments to pass
///  obj -- the object to be called
/// Stack Out:
///  value -- the value returned by obj::'method_id'(arg1, ..., argN)
	case SWF::ABC_ACTION_CALLMETHOD:
	{
		boost::uint32_t dispatch_id = mStream->read_V32() - 1;
		boost::uint32_t argc = mStream->read_V32();
		ENSURE_OBJECT(mStack.top(argc));
		as_object *obj = mStack.top(argc).to_object().get();
		const Property *f = obj->getByIndex(dispatch_id);
		as_function* func;
#if 0
		if (f->isGetterSetter())
		{
			// Likely an error, but try to handle it.
			func = f->getGetter();
		}
		else
#endif
		if (f->getValue(*obj).is_function())
			func = f->getValue(*obj).to_as_function();
		else
		{
			// Definitely an error, and not the kind we can handle.
			throw ASException();
		}
		pushCall(func, obj, mStack.top(argc), argc, 0);
		break;
	}
/// 0x44 ABC_ACTION_CALLSTATIC
/// Stream: V32 'method_id' | V32 'arg_count'
/// Stack In:
///  argN ... arg1 -- the arg_count arguments to pass
///  obj -- the object to act as a receiver for the static call
/// Stack Out:
///  value -- the value returned by obj->ABC::'method_id'(arg1, ..., argN)
	case SWF::ABC_ACTION_CALLSTATIC:
	{
		asMethod *m = pool_method(mStream->read_V32(), mPoolObject);
		boost::uint32_t argc = mStream->read_V32();
		as_function *func = m->getPrototype();
		ENSURE_OBJECT(mStack.top(argc));
		as_object *obj = mStack.top(argc).to_object().get();
		pushCall(func, obj, mStack.top(argc), argc, 0);
		break;
	}
/// 0x45 ABC_ACTION_CALLSUPER
/// 0x4E ABC_ACTION_CALLSUPERVOID
/// Stream: V32 'name_offset' | V32 'arg_count'
/// Stack In:
///  [ns [n]] -- Namespace stuff
///  argN ... arg1 -- the arg_count arguments to pass
///  obj -- the object whose super is to be called
/// Stack Out:
///  0x45: value -- the value returned by obj::(resolve)'name_offset'::super(arg1,
///   ..., argN)
///  0x4E: .
	case SWF::ABC_ACTION_CALLSUPER:
	case SWF::ABC_ACTION_CALLSUPERVOID:
	{
		asName a = pool_name(mStream->read_V32(), mPoolObject);
		boost::uint32_t argc = mStream->read_V32();
		int dropsize = completeName(a);
		ENSURE_OBJECT(mStack.top(argc + dropsize));
		mStack.drop(dropsize);
		as_object *super = mStack.top(argc).to_object()->get_super();
		if (!super)
			throw ASReferenceError();
		Property *b = super->findProperty(a.getABCName(), 
			a.getNamespace()->getURI());
		if (!b)
			throw ASReferenceError();
		as_function *f = // b->isGetterSetter() ? b->getGetter() :
			b->getValue(super).to_as_function();

		if (opcode == SWF::ABC_ACTION_CALLSUPER)
			pushCall(f, super, mStack.top(argc), argc, 0);
		else // Void call
			pushCall(f, super, mIgnoreReturn, argc, -1); // drop obj too.
		break;
	}
/// 0x46 ABC_ACTION_CALLPROPERTY
/// 0x4C ABC_ACTION_CALLPROPLEX
/// 0x4F ABC_ACTION_CALLPROPVOID
/// Stream: V32 'name_offset' | V32 'arg_count'
/// Stack In:
///  argN ... arg1 -- the arg_count arguments to pass
///  [ns [n]] -- Namespace stuff
///  obj -- The object whose property is to be accessed.
/// Stack Out:
///  value -- the value from obj::(resolve)'name_offset'(arg1, ..., argN)
///  (unless ABC_ACTION_CALL_PROPVOID, then: . )
/// NB: Calls getter/setter if they exist.
/// If the opcode is ABC_ACTION_CALLPROPLEX, obj is not altered by getter/setters
	case SWF::ABC_ACTION_CALLPROPERTY:
	case SWF::ABC_ACTION_CALLPROPLEX:
	case SWF::ABC_ACTION_CALLPROPVOID:
	{
		as_value result;
		asName a = pool_name(mStream->read_V32(), mPoolObject);
		boost::uint32_t argc = mStream->read_V32();
		std::auto_ptr< std::vector<as_value> > args = get_args(argc);
		//TODO: If multiname is runtime also pop namespace and/or name values.
        if ( a.isRuntime() )
        {
            log_unimpl("ABC_ACTION_CALL* with runtime multiname");
        }
		as_value object_val = pop_stack();

		as_object *object = object_val.to_object().get();
		if (!object) {
            IF_VERBOSE_ASCODING_ERRORS(
			log_aserror(_("Can't call a method of a value that doesn't cast to an object (%s)."),
                object_val);
            )
		}
		else{

			as_value property = object->getMember(a.getGlobalName(),0);
		
			if(!property.is_undefined() && !property.is_null()){
				LOG_DEBUG_AVM("Calling method %s on object %s",property.toDebugString(),object_val.toDebugString());
				as_environment env = as_environment(_vm);
				result = call_method(property,&env,object,args);

			}
			else{
                IF_VERBOSE_ASCODING_ERRORS(
				log_aserror(_("Property '%s' of object '%s' is '%s', cannot call as method"),
                    mPoolObject->mStringPool[a.getABCName()],
                    object_val, property);
                )
			}

		}
			if(opcode == SWF::ABC_ACTION_CALLPROPERTY){
				push_stack(result);
			}

/*		int shift = completeName(a, argc);
		ENSURE_OBJECT(mStack.top(shift + argc));
		as_object *obj = mStack.top(argc + shift).to_object().get();
		Property *b = obj->findProperty(a.getABCName(), 
			a.getNamespace()->getURI());
		if (!b)
			throw ASReferenceError();

		as_function *func;
		if (b->isGetterSetter())
		{
			if (lex_only)
			{
				mStack.top(argc + shift).set_undefined();
				mStack.drop(argc + shift);
				break;
			}
#if 0
			else
			{
				//func = b->getGetter();
				log_error("Can't do  ABC_ACTION_CALLPROPVOID or ABC_ACTION_CALLPROPERTY")
				break;
			}
#endif
		}
		//else
			func = b->getValue(obj).to_as_function();

		if (opcode == SWF::ABC_ACTION_CALLPROPVOID)
			pushCall(func, obj, mIgnoreReturn, argc, -shift - 1);
		else
			pushCall(func, obj, mStack.top(argc + shift), argc, -shift);*/
		break;
	}
/// 0x47 ABC_ACTION_RETURNVOID
/// Do: Return an undefined object up the callstack.
	case SWF::ABC_ACTION_RETURNVOID:
	{
		mStream->seekTo(0);
		if(mStateStack.size() == 0){
			return;
		}
		else{
			restoreState();
			if(mExitWithReturn){
				return;
			}
		}
		// Slot the return.
//		*mGlobalReturn = as_value();
		// And restore the previous state.
//		restoreState();
		break;
	}
/// 0x48 ABC_ACTION_RETURNVALUE
/// Stack In:
///  value -- value to be returned
/// Stack Out:
///  .
/// Do: Return value up the callstack.
	case SWF::ABC_ACTION_RETURNVALUE:
	{
		// Slot the return.
		mGlobalReturn = pop_stack();
		// And restore the previous state.
		restoreState();
		return;
	}
/// 0x49 ABC_ACTION_CONSTRUCTSUPER
/// Stream: V32 'arg_count'
/// Stack In:
///  argN ... arg1 -- the arg_count arguments
///  obj -- the object whose super's constructor should be invoked
/// Stack Out:
///  .
	case SWF::ABC_ACTION_CONSTRUCTSUPER:
	{
		boost::uint32_t argc = mStream->read_V32();
		LOG_DEBUG_AVM("There are %u arguments.",argc);
		get_args(argc);
//		ENSURE_OBJECT(mStack.top(argc));
		as_object *super = pop_stack().to_object().get()->get_super();
		//TODO: Actually construct the super.
//		as_object *super = mStack.top(argc).to_object()->get_super();
// 		if (!super)
// 		{
// 			throw ASException();
// 			break;
// 		}
// 		as_function *func = super->get_constructor();
		// 'obj' is the 'this' for the call, we ignore the return, there are
		// argc arguments, and we drop all of the arguments plus 'obj' from
		// the stack.
//		pushCall(func, obj, mIgnoreReturn, argc, -1);
		break;
	}
/// 0x4A ABC_ACTION_CONSTRUCTPROP
/// Stream: V32 'name_offset' | V32 'arg_count'
/// Stack In:
///  argN ... arg1 -- the arg_count arguments to pass
///  [ns [n]] -- Namespace stuff
///  obj -- the object whose property should be constructed
/// Stack Out:
///  value -- the newly constructed prop from obj::(resolve)
///   'name_offset'(arg1, ..., argN)
	case SWF::ABC_ACTION_CONSTRUCTPROP:
	{
		as_environment env = as_environment(_vm);
		asName a = pool_name(mStream->read_V32(), mPoolObject);
		boost::uint32_t argc = mStream->read_V32();
		std::auto_ptr< std::vector<as_value> > args = get_args(argc);
		as_object* object = pop_stack().to_object().get();
		if(!object){
			//TODO: Should this result in an exeception or an actionscript error?
			LOG_DEBUG_AVM("Can't constructor property on a null object.  Property not constructed.");
			push_stack(as_value());
			break;
		}
		std::string& classname = mPoolObject->mStringPool[a.getABCName()];
		
		as_value constructor_val = object->getMember(a.getGlobalName());
		boost::intrusive_ptr<as_function> constructor = constructor_val.to_as_function();
		if(constructor){
			boost::intrusive_ptr<as_object> newobj = constructor->constructInstance(env, args);
			push_stack(as_value(newobj));
		}
		//TODO: This else clause is needed to construct classes that aren't builtin into gnash.
		// I don't think this is correct, and I think the problem might be how AVM2 adds
		// new objects to the Global object.
		else{
 			LOG_DEBUG_AVM("Object %s is not a constructor",constructor_val.toDebugString());
			if(constructor_val.is_null() || constructor_val.is_undefined()){
				LOG_DEBUG_AVM("Constructor is undefined, will not construct property.");
				push_stack(as_value());
			}
			else{
				as_value val = constructor_val.to_object().get()->getMember(NSV::PROP_CONSTRUCTOR,0);
				as_value result = call_method(val,&env,constructor_val.to_object().get(),args);
				push_stack(result);
			}
		}
		
		break;
	}
/// 0x55 ABC_ACTION_NEWOBJECT
/// Stream: V32 'arg_count'
/// Stack In:
///  prop_value_1 -- a value object
///  prop_name_1 -- a string
///  .
///  . (arg_count value/name pairs in all)
///  .
///  prop_value_n -- a value object
///  prop_name_n -- a string
/// Stack Out:
///  obj -- A new object which contains all of the given properties.
/// NB: This builds an object from its properties, it's not a constructor.
	case SWF::ABC_ACTION_NEWOBJECT:
	{
		as_object *obj = new as_object(getObjectInterface());
		boost::uint32_t argc = mStream->read_V32();
		int i = argc;
		while (i--)
		{
			as_value val = pop_stack();
			as_value name = pop_stack();
			obj->init_member(name.to_string(),val,0,0);
		}
		push_stack(as_value(obj));
		break;
	}
/// 0x56 ABC_ACTION_NEWARRAY
/// Stream: V32 'array_size'
/// Stack In:
///  value_n -- a value
///  .
///  . (array_size of these)
///  .
///  value_1 -- a value
/// Stack Out:
///  array -- an array { value_1, value_2, ..., value_n }
	case SWF::ABC_ACTION_NEWARRAY:
	{
		boost::uint32_t asize = mStream->read_V32();
		LOG_DEBUG_AVM("Creating array of size %u",asize);
		Array_as *arr = new Array_as;
		arr->resize(asize);
		boost::uint32_t i = asize;
		while (i--){
			arr->set_indexed(i, pop_stack());
		}
		push_stack(as_value(arr));
		break;
	}
/// 0x57 ABC_ACTION_NEWACTIVATION
/// Stack Out:
///  vtable -- A new virtual table, which has the previous one as a parent.
	case SWF::ABC_ACTION_NEWACTIVATION:
	{
		// TODO:  The function contains traits that need to be included in the activation object.
		//For now we are using the function object as the activation object.  There is probably
		//a better way.
		push_stack(as_value(mCurrentFunction));
		break;
	}
/// 0x58 ABC_ACTION_NEWCLASS
/// Stream: V32 'class_id'
/// Stack In:
///  obj -- An object to be turned into a class. Its super is constructed.
/// Stack Out:
///  class -- The newly made class, made from obj and the information at
///   cinits_pool[class_id]
/// NB: This depends on scope and scope base (to determine lifetime(?))
	case SWF::ABC_ACTION_NEWCLASS:
	{
		boost::uint32_t cid = mStream->read_V32();
		asClass *c = pool_class(cid, mPoolObject);
		LOG_DEBUG_AVM("Creating new class id=%u name=%s",c->getName(),mPoolObject->mStringPool[c->getName()]);
		
		as_object* base_class = pop_stack().to_object().get();
		as_object* new_class = c->getPrototype();
		
		new_class->set_prototype(base_class);
		//Create the class.
		as_function* static_constructor = c->getStaticConstructor()->getPrototype();
		as_function* constructor = c->getConstructor()->getPrototype();
		new_class->init_member(NSV::PROP_uuCONSTRUCTORuu,as_value(static_constructor),0);
		new_class->init_member(NSV::PROP_CONSTRUCTOR,as_value(constructor),0);
		push_stack(as_value(new_class));

		//Call the class's static constructor.
		as_environment env = as_environment(_vm);
		as_value property = new_class->getMember(NSV::PROP_uuCONSTRUCTORuu,0);
		as_value value = call_method(property,&env,new_class,get_args(0));

		break;
	}
/// 0x59 ABC_ACTION_GETDESCENDANTS
/// Stream: V32 'name_id'
/// Stack In:
///  value -- Whose descendants to get
///  [ns [n]] -- Namespace stuff
/// Stack Out:
///  ?
/// NB: This op seems to always throw a TypeError in Tamarin, though I
/// assume that it ought to do something to yield a list of
/// descendants of a class.
	case SWF::ABC_ACTION_GETDESCENDANTS:
	{
		asName a = pool_name(mStream->read_V32(), mPoolObject);
		as_value &v = mStack.top(0);
		ENSURE_OBJECT(v);
		mStack.drop(1);
		mStack.drop(completeName(a));
		// TODO: Decide or discover what to do with this.
		break;
	}
/// 0x5A ABC_ACTION_NEWCATCH
/// Stream: V32 'catch_id'
/// Stack Out:
///  vtable -- vtable suitable to catch an exception of type in catch_id.
/// NB: Need more information on how exceptions are set up.
	case SWF::ABC_ACTION_NEWCATCH:
	{
		// TODO: Decide if we need this. (Might be a no-op.)
		break;
	}
/// 0x5D ABC_ACTION_FINDPROPSTRICT
/// 0x5E ABC_ACTION_FINDPROPERTY
/// Stream: V32 'name_id'
/// Stack In:
///  [ns [n]] -- Namespace stuff
/// Stack Out:
///  owner -- object which owns property given by looking up the name_id.
///  0x5D is the undefined object if not found
///  0x5E throws a ReferenceError if not found
	case SWF::ABC_ACTION_FINDPROPSTRICT:
	case SWF::ABC_ACTION_FINDPROPERTY:
	{
//		boost::uint32_t property_name = mStream->read_V32();
		asName a = pool_name(mStream->read_V32(), mPoolObject);
		find_prop_strict(a);
/*		mStack.drop(completeName(a));
		as_object *owner;
		Property *b = mCurrentScope->findProperty(a.getABCName(), 
			a.getNamespace()->getURI(), &owner);
		if (!b)
		{
			if (opcode == SWF::ABC_ACTION_FINDPROPSTRICT)
				throw ASReferenceError();
			else
				mStack.push(as_value());
		}
		else
		{
			mStack.push(owner);
		}*/
		break;
	}
/// 0x5F ABC_ACTION_FINDDEF
/// Stream: V32 'name_id' (no ns expansion)
/// Stack Out:
///  def -- The definition of the name at name_id.
	case SWF::ABC_ACTION_FINDDEF:
	{
		asName a = pool_name(mStream->read_V32(), mPoolObject);
		// The name is expected to be complete.
		// TODO
		break;
	}
/// 0x60 ABC_ACTION_GETLEX
/// Stream: V32 'name_id' (no ns expansion)
/// Stack Out:
///  property -- The result of 0x5D (ABC_ACTION_FINDPROPSTRICT)
///   + 0x66 (ABC_ACTION_GETPROPERTY)
	case SWF::ABC_ACTION_GETLEX:
	{
		asName a = pool_name(mStream->read_V32(), mPoolObject);
	
		as_value val = find_prop_strict(a);

		pop_stack();

		push_stack(val);

		break;
	}
///  ABC_ACTION_SETPROPERTY
/// Stream: V32 'name_id'
/// Stack In:
///  value -- The value to be used
///  [ns [n]] -- Namespace stuff
///      OR
///  [key] -- Key name for property. Will not have both Namespace and key.
///  obj -- The object whose property is to be set
/// Stack Out:
///  .
/// NB: If the name at name_id is completely qualified, neither a namespace
/// nor a key is needed.  If the name_id refers to a name with a runtime
/// namespace, then this will be used.  If neither of those is true and
/// obj is a dictionary and key is a name, then the name_id is discarded and
/// key/value is set in the dictionary obj instead.
	case SWF::ABC_ACTION_SETPROPERTY:
	{
		as_value value = pop_stack();
		string_table::key ns = 0;
		string_table::key name = 0;

		asName a = pool_name(mStream->read_V32(), mPoolObject);
		//TODO: If multiname is runtime we need to also pop namespace and name values of the stack.
		if(a.mFlags == asName::KIND_MultinameL){
			as_value nameValue = pop_stack();
			name = mST.find(nameValue.to_string());
		}
		else{
			name = a.getGlobalName();
		}
		as_object *object = pop_stack().to_object().get();
		object->set_member(name,value,ns,false);

		break;
	}
/// 0x62 ABC_ACTION_GETLOCAL
/// Stream: V32 'frame_index'
/// Frame: value at frame_index is needed
/// Stack Out:
///  value
	case SWF::ABC_ACTION_GETLOCAL:
	{
		boost::uint32_t index = mStream->read_V32();
		push_stack(get_register(index));
		break;
	}
/// 0x63 ABC_ACTION_SETLOCAL
/// Stream: V32 'frame_index'
/// Frame: obj at frame_index is set to value
/// Stack In:
///  value
/// Stack Out:
///  .
	case SWF::ABC_ACTION_SETLOCAL:
	{
		boost::uint32_t index = mStream->read_V32();
		LOG_DEBUG_AVM("Register index: %u",index);
		mRegisters[index] = pop_stack();
		break;
	}
/// 0x64 ABC_ACTION_GETGLOBALSCOPE
/// Stack Out:
///  global -- The global scope object
	case SWF::ABC_ACTION_GETGLOBALSCOPE:
	{
		//TODO: Use get_scope_stack here.
		push_stack(as_value(mScopeStack.value(0).get()));
//		print_stack();
		break;
	}
/// 0x65 ABC_ACTION_GETSCOPEOBJECT
/// Stream: S8 'depth'
/// Stack Out:
///  scope -- The scope object at depth
	case SWF::ABC_ACTION_GETSCOPEOBJECT:
	{
		boost::uint8_t depth = mStream->read_u8();
		push_stack(get_scope_stack(depth));
		print_scope_stack();
		break;
	}
/// 0x66 ABC_ACTION_GETPROPERTY
/// Stream: V32 'name_id'
/// Stack In:
///  [ns [n]] -- Namespace stuff
///      OR
///  [key] -- Key name for property. Will not have both Namespace and key.
///  obj -- The object whose property is to be retrieved
/// Stack Out:
///  prop -- The requested property.
/// NB: See 0x61 (ABC_ACTION_SETPROPETY) for the decision of ns/key.
	case SWF::ABC_ACTION_GETPROPERTY:
	{
		as_value prop;
		string_table::key ns = 0;
		string_table::key name = 0;
		asName a = pool_name(mStream->read_V32(), mPoolObject);
		//TODO: If multiname is runtime we need to also pop namespace and name values of the stack.
		if(a.mFlags == asName::KIND_MultinameL){
			as_value nameValue = pop_stack();
			name = mST.find(nameValue.to_string());
		}
		else{
			name = a.getGlobalName();
		}

		as_value object_val = pop_stack();

		as_object* object = object_val.to_object().get();
		if (!object) {
			IF_VERBOSE_ASCODING_ERRORS(
			log_aserror(_("Can't get a property of a value that doesn't cast to an object (%s)."),
				object_val);
			)
		}
		else{
			object->get_member(name, &prop, ns);
		}

		push_stack(prop);

		break;
	}
/// 0x68 ABC_ACTION_INITPROPERTY
/// Stream V32 'name_id'
/// Stack In:
///  value -- The value to be put into the property.
///  [ns [n]] -- Namespace stuff
///  obj -- The object whose property is to be initialized
/// Stack Out:
///  .
/// Do:
///  Set obj::(resolve)'name_id' to value, set bindings from the context.
	case SWF::ABC_ACTION_INITPROPERTY:
	{
		boost::uint32_t index = mStream->read_V32();
		asName a = pool_name(index, mPoolObject);
		as_value v = pop_stack();
		//TODO: If multiname is a runtime mutiname we need to also pop name and namespace values.
		as_value object_asval = pop_stack();
		if(object_asval.is_undefined() || object_asval.is_null()){
			LOG_DEBUG_AVM("Object is undefined will skip trying to initialize property.");
		}
		else{
			as_object& obj = *object_asval.to_object().get();
			!obj.set_member(a.getGlobalName(),v,false);
		}
		break;
	}
/// 0x6A ABC_ACTION_DELETEPROPERTY
/// Stream: V32 'name_id'
/// Stack In:
///  [ns [n]] -- Namespace stuff
///  obj -- The object whose property should be deleted.
/// Stack Out:
///  truth -- True if property was deleted or did not exist, else False.
	case SWF::ABC_ACTION_DELETEPROPERTY:
	{
		asName a = pool_name(mStream->read_V32(), mPoolObject);
		mStack.drop(completeName(a));
		//mStack.top(0) = mStack.top(0).deleteProperty(a);
		break;
	}
/// 0x6C ABC_ACTION_GETSLOT
/// Stream: V32 'slot_index + 1'
/// Stack In:
///  obj -- The object which owns the desired slot.
/// Stack Out:
///  slot -- obj.slots[slot_index]
	case SWF::ABC_ACTION_GETSLOT:
	{
		as_value val;
		boost::uint32_t sindex = mStream->read_V32();
		as_object* object = pop_stack().to_object().get();

		object->get_member_slot(sindex + 1, &val);

		LOG_DEBUG_AVM("object has value %s at real_slot=%u abc_slot=%u",val.toDebugString(),sindex + 1, sindex);
		push_stack(val);
		
		break;
	}
/// 0x6D ABC_ACTION_SETSLOT
/// Stream: V32 'slot_index + 1'
/// Stack In:
///  value -- The value intended for the slot.
///  obj -- The object whose slot should be set.
/// Stack Out:
///  .
/// Do: obj.slots[slot_index] = value
	case SWF::ABC_ACTION_SETSLOT:
	{
		boost::uint32_t sindex = mStream->read_V32();
		as_value value = pop_stack();
		as_value object = pop_stack();
		//We use sindex + 1, because currently as_object sets a property at a slot index
		//1 higher than the index the abc_block thinks the property is at.
		if(!object.to_object().get()->set_member_slot(sindex+1,value)){
			LOG_DEBUG_AVM("Failed to set property at real_slot=%u abc_slot=%u",sindex+1,sindex);
		}
		else{
			LOG_DEBUG_AVM("Set property at real_slot=%u abc_slot=%u",sindex+1,sindex);
		}
		//TODO: Actually set the object's value.
		break;
	}
/// 0x6E ABC_ACTION_GETGLOBALSLOT
/// Stream: V32 'slot_index + 1'
/// Stack In:
///  .
/// Stack Out:
///  slot -- globals.slots[slot_index]
/// NB: Deprecated
	case SWF::ABC_ACTION_GETGLOBALSLOT:
	{
		boost::uint32_t sindex = mStream->read_V32();
		if (!sindex)
			throw ASException();
		--sindex;
		mStack.grow(1);
		//TODO: mStack.top(0) = mGlobal.getSlot(sindex);
		break;
	}
/// 0x6F ABC_ACTION_SETGLOBALSLOT
/// Stream: V32 'slot_index + 1'
/// Stack In:
///  value -- The value to be placed into the slot.
/// Stack Out:
///  .
/// Do: globals[slot_index] = value
/// NB: Deprecated
	case SWF::ABC_ACTION_SETGLOBALSLOT:
	{
		boost::uint32_t sindex = mStream->read_V32();
		if (!sindex)
			throw ASException();
		--sindex;
		//TODO: mGlobal.setSlot(sindex, mStack.pop());
		break;
	}
/// 0x70 ABC_ACTION_CONVERT_S
/// Stack In:
///  value -- An object
/// Stack Out:
///  str_value -- value as a string
	case SWF::ABC_ACTION_CONVERT_S:
	{
		mStack.top(0) = mStack.top(0).to_string();
		break;
	}
/// 0x71 ABC_ACTION_ESC_XELEM
/// Stack In:
///  value -- An object to be escaped
/// Stack Out:
///  str_value -- value as a string, escaped suitably for an XML element.
	case SWF::ABC_ACTION_ESC_XELEM:
	{
		//TODO: mStack.top(0) = mStack.top(0).to_escaped_xml_element();
		break;
	}
/// 0x72 ABC_ACTION_ESC_XATTR
/// Stack In:
///  value -- An object to be escaped
/// Stack Out:
///  str_value -- value as a string, escaped suitably for an XML attribute.
	case SWF::ABC_ACTION_ESC_XATTR:
	{
		//TODO: mStack.top(0) = mStack.top(0).to_escaped_xml_attribute();
		break;
	}
/// 0x73 ABC_ACTION_CONVERT_I
/// 0x83 ABC_ACTION_COERCE_I (deprecated)
/// Stack In:
///  value -- An object to be converted to Integer
/// Stack Out:
///  int_value -- value as an integer object
	case SWF::ABC_ACTION_CONVERT_I:
	case SWF::ABC_ACTION_COERCE_I:
	{
		as_value val = pop_stack();
		val.to_int();
		push_stack(val);
		break;
	}
/// 0x74 ABC_ACTION_CONVERT_U
/// 0x88 ABC_ACTION_COERCE_U (deprecated)
/// Stack In:
///  value -- An object to be converted to unsigned integer
/// Stack Out:
///  int_value -- value as an unsigned integer object
	case SWF::ABC_ACTION_CONVERT_U:
	case SWF::ABC_ACTION_COERCE_U:
	{
		mStack.top(0) = mStack.top(0).to_number<unsigned int>();
		break;
	}
/// 0x75 ABC_ACTION_CONVERT_D
/// 0x84 ABC_ACTION_COERCE_D (deprecated)
/// Stack In:
///  value -- An object to be converted to a double
/// Stack Out:
///  double_value -- value as a double object
	case SWF::ABC_ACTION_CONVERT_D:
	case SWF::ABC_ACTION_COERCE_D:
	{
		push_stack(pop_stack().to_number());
		break;
	}
/// 0x76 ABC_ACTION_CONVERT_B
/// 0x81 ABC_ACTION_COERCE_B (deprecated)
/// Stack In:
///  value -- An object to be converted to a boolean
/// Stack Out:
///  bool_value -- value as a boolean object
	case SWF::ABC_ACTION_CONVERT_B:
	case SWF::ABC_ACTION_COERCE_B:
	{
		mStack.top(0).set_bool(mStack.top(0).to_bool());
		break;
	}
/// 0x77 ABC_ACTION_CONVERT_O
/// Stack In:
///  obj -- An object
/// Stack Out:
///  obj -- An object
/// Do: If obj is Undefined or Null, throw TypeError
	case SWF::ABC_ACTION_CONVERT_O:
	{
		mStack.top(0) = mStack.top(0).to_object().get();
		if (mStack.top(0).is_undefined() || mStack.top(0).is_null())
			throw ASTypeError();
		break;
	}
/// 0x78 ABC_ACTION_CHECKFILTER
/// Stack In:
///  obj -- An object
/// Stack Out:
///  obj -- An object
/// Do: If obj is not XML based, throw TypeError
	case SWF::ABC_ACTION_CHECKFILTER:
	{
		if (!mStack.top(0).is_object() || !mStack.top(0).to_object()->isXML())
			throw ASTypeError();
		break;
	}
/// 0x80 ABC_ACTION_COERCE
/// Stream: V32 'name_index'
/// Stack In:
///  [ns [n]] -- Possibly name/namespace stuff
///  obj -- An object to be converted
/// Stack Out:
///  coerced_obj -- The object as the desired (resolve)'name_index' type.
	case SWF::ABC_ACTION_COERCE:
	{
		asName a = pool_name(mStream->read_V32(), mPoolObject);
		as_value value = pop_stack();

		//TODO: Actually coerce the value.
//		if(value.is_null()){
//			as_value new_type = get_property_value(a);
//			value->
//			push_stack(new_type);
//		}
//		else{
			push_stack(value);
//		}
		break;
	}
/// 0x82 ABC_ACTION_COERCE_A
/// Stack In:
///  obj -- An object to be converted
/// Stack Out:
///  obj
/// Do: Nothing. (The 'a' is for atom, and it's unclear if anything is needed.)
	case SWF::ABC_ACTION_COERCE_A:
	{
		break;
	}
/// 0x85 ABC_ACTION_COERCE_S
/// Stack In:
///  obj -- An object to be converted
/// Stack Out:
///  str_obj -- obj as string. nullString object if obj is Null or Undefined
	case SWF::ABC_ACTION_COERCE_S:
	{
		if (mStack.top(0).is_undefined() || mStack.top(0).is_null())
			mStack.top(0) = "";
		else
			mStack.top(0) = mStack.top(0).to_string();
		break;
	}
/// 0x86 ABC_ACTION_ASTYPE
/// Stream: V32 'name_index'
/// Stack In:
///  [ns [n]] -- Possible namespace stuff
///  obj -- An object to be checked
/// Stack Out:
///  cobj -- obj if obj is of type (resolve)'name_index', otherwise Null
	case SWF::ABC_ACTION_ASTYPE:
	{
		asName a = pool_name(mStream->read_V32(), mPoolObject);
		as_value value = pop_stack();
		//TODO: Make sure the value is of the correct type;
		push_stack(value);
		break;
	}
/// 0x87 ABC_ACTION_ASTYPELATE
/// Stack In:
///  valid -- The object whose type is to be matched
///  obj -- An object to be checked
/// Stack Out:
///  cobj -- obj if type of obj conforms to valid, otherwise Null
	case SWF::ABC_ACTION_ASTYPELATE:
	{
		as_value type = pop_stack();
		as_value value = pop_stack();
		//TODO: If value is not the type defined by type, then push null.
		push_stack(value);
		break;
	}
/// 0x89 ABC_ACTION_COERCE_O
/// Stack In:
///  obj -- An object
/// Stack Out:
///  cobj -- obj if obj is not Undefined, otherwise Null
	case SWF::ABC_ACTION_COERCE_O:
	{
		if (mStack.top(0).is_undefined())
			mStack.top(0) = mStack.top(0).to_object().get();
		else
			mStack.top(0).set_undefined();
		break;
	}
/// 0x90 ABC_ACTION_NEGATE
/// Stack In:
///  obj -- An object
/// Stack Out:
///  negdouble -- -1.0 * (double) obj
	case SWF::ABC_ACTION_NEGATE:
	{
		mStack.top(0) = -mStack.top(0).to_number();
		break;
	}
/// 0x91 ABC_ACTION_INCREMENT
/// Stack In:
///  num -- A number, integer or double
/// Stack Out:
///  num + 1
	case SWF::ABC_ACTION_INCREMENT:
	{
		as_value val = pop_stack();
		push_stack(as_value(val.to_number() + 1));
		break;
	}
/// 0x92 ABC_ACTION_INCLOCAL
/// Stream: V32 'frame_addr'
/// Frame: Load i from frame_addr and increment it.
	case SWF::ABC_ACTION_INCLOCAL:
	{
		boost::uint32_t foff = mStream->read_V32();
		mRegisters[foff] = mRegisters[foff].to_number() + 1;
		break;
	}
/// 0x93 ABC_ACTION_DECREMENT
/// Stack In:
///  num -- A number, integer or double
/// Stack Out:
///  num - 1
	case SWF::ABC_ACTION_DECREMENT:
	{
		mStack.top(0) = mStack.top(0).to_number() - 1;
		break;
	}
/// 0x94 ABC_ACTION_DECLOCAL
/// Stream: V32 'frame_addr'
/// Frame: Load i from frame_addr and decrement it.
	case SWF::ABC_ACTION_DECLOCAL:
	{
		boost::uint32_t foff = mStream->read_V32();
		mRegisters[foff] = mRegisters[foff].to_number() - 1;
		break;
	}
/// 0x95 ABC_ACTION_ABC_TYPEOF
/// Stack In:
///  obj -- An object
/// Stack Out:
///  type -- typeof(obj) as a string
	case SWF::ABC_ACTION_ABC_TYPEOF:
	{	
		push_stack(pop_stack().typeOf());
		break;
	}
/// 0x96 ABC_ACTION_NOT
/// Stack In:
///  obj -- An object
/// Stack Out:
///  nobj -- A truth object with value !((Boolean) obj)
	case SWF::ABC_ACTION_NOT:
	{
		mStack.top(0).set_bool(!mStack.top(0).to_bool());
		break;
	}
/// 0x97 ABC_ACTION_BITNOT
/// Stack In:
///  obj -- An object
/// Stack Out:
///  nint -- ~((Int) obj)
	case SWF::ABC_ACTION_BITNOT:
	{
		mStack.top(0) = ~mStack.top(0).to_int();
		break;
	}
/// 0xA0 ABC_ACTION_ADD	
/// Stack In:
/// a
/// b
/// Stack Out:
/// a + b (double if numeric)
	case SWF::ABC_ACTION_ADD:
	{
		as_value b = pop_stack();
		as_value a = pop_stack();
		a.newAdd(b);
		push_stack(a);
		break;
	}
/// 0xA1 ABC_ACTION_SUBTRACT
/// Stack In:
///  b
///  a
/// Stack Out:
///  a - b (double)
	case SWF::ABC_ACTION_SUBTRACT:
	{
		as_value b = pop_stack();
		as_value a = pop_stack();
		a.subtract(b);
		push_stack(a);
		break;
	}
/// 0xA2 ABC_ACTION_MULTIPLY
/// Stack In:
///  a
///  b
/// Stack Out:
///  a * b (double)
	case SWF::ABC_ACTION_MULTIPLY:
	{
		mStack.top(1) = mStack.top(1).to_number() * mStack.top(0).to_number();
		mStack.drop(1);
		break;
	}
/// 0xA3 ABC_ACTION_DIVIDE
/// Stack In:
///  b
///  a
/// Stack Out:
///  a / b (double)
	case SWF::ABC_ACTION_DIVIDE:
	{
		mStack.top(1) = mStack.top(1).to_number() / mStack.top(0).to_number();
		mStack.drop(1);
		break;
	}
/// 0xA4 ABC_ACTION_MODULO
/// Stack In:
///  b
///  a
/// Stack Out:
///  a % b (not integer mod, but remainder)
	case SWF::ABC_ACTION_MODULO:
	{
		double result = mStack.top(1).to_number() / mStack.top(0).to_number();
		int trunc_result = static_cast<int> (result);
		mStack.top(1) = mStack.top(1).to_number<double>() - 
			(trunc_result * mStack.top(0).to_number<double>());
		mStack.drop(1);
		break;
	}
/// 0xA5 ABC_ACTION_LSHIFT
/// Stack In:
///  b
///  a
/// Stack Out:
///  a << b
	case SWF::ABC_ACTION_LSHIFT:
	{
		mStack.top(1) = mStack.top(1).to_int() << mStack.top(0).to_int();
		mStack.drop(1);
		break;
	}
/// 0xA6 ABC_ACTION_RSHIFT
/// Stack In:
///  a
///  b
/// Stack Out:
///  a >> b
	case SWF::ABC_ACTION_RSHIFT:
	{
		mStack.top(1) = mStack.top(1).to_int() >> mStack.top(0).to_int();
		mStack.drop(1);
		break;
	}
/// 0xA7 ABC_ACTION_URSHIFT
/// Stack In:
///  b
///  a
/// Stack Out:
///  ((unsigned) a) >> b
	case SWF::ABC_ACTION_URSHIFT:
	{
		mStack.top(1) = mStack.top(1).to_number<unsigned int>()
			>> mStack.top(0).to_int();
		mStack.drop(1);
		break;
	}
/// 0xA8 ABC_ACTION_BITAND
///  a
///  b
/// Stack Out:
///  a & b
	case SWF::ABC_ACTION_BITAND:
	{
		mStack.top(1) = mStack.top(1).to_int() & mStack.top(0).to_int();
		mStack.drop(1);
		break;
	}
/// 0xA9 ABC_ACTION_BITOR
/// Stack In:
///  b
///  a
/// Stack Out:
///  a | b
	case SWF::ABC_ACTION_BITOR:
	{
		mStack.top(1) = mStack.top(1).to_int() | mStack.top(0).to_int();
		mStack.drop(1);
		break;
	}
/// 0xAA ABC_ACTION_BITXOR
/// Stack In:
///  b
///  a
/// Stack Out:
///  a ^ b
	case SWF::ABC_ACTION_BITXOR:
	{
		mStack.top(1) = mStack.top(1).to_int() ^ mStack.top(0).to_int();
		mStack.drop(1);
		break;
	}
/// 0xAB ABC_ACTION_EQUALS
/// Stack In:
///  b
///  a
/// Stack Out:
///  truth -- Truth of (a == b) (weakly)
	case SWF::ABC_ACTION_EQUALS:
	{
		bool truth = abstractEquality(mStack.top(1), mStack.top(0), false);
		pop_stack();
		pop_stack();
		as_value result = as_value();
		result.set_bool(truth);
		push_stack(result);
		break;
	}
/// 0xAC ABC_ACTION_STRICTEQUALS
/// Stack In:
///  b
///  a
/// Stack Out:
///  truth -- Truth of (a == b) (strongly, as in 
///   0x19 (ABC_ACTION_IFSTRICTEQ))
	case SWF::ABC_ACTION_STRICTEQUALS:
	{
		bool truth = abstractEquality(mStack.top(1), mStack.top(0), true);
		mStack.drop(1);
		mStack.top(0).set_bool(truth);
		break;
	}
/// 0xAD ABC_ACTION_LESSTHAN
/// Stack In:
///  b
///  a
/// Stack Out:
///  truth -- Truth of (a < b)
	case SWF::ABC_ACTION_LESSTHAN:
	{
		bool truth;
		ABSTRACT_COMPARE(truth, mStack.top(1), mStack.top(0), false);
		mStack.drop(1);
		mStack.top(0).set_bool(truth); // truth is a < b
		break;
	}
/// 0xAE ABC_ACTION_LESSEQUALS
/// Stack In:
///  b
///  a
/// Stack Out:
///  truth -- Truth of (a <= b)
	case SWF::ABC_ACTION_LESSEQUALS:
	{
		bool truth;
		ABSTRACT_COMPARE(truth, mStack.top(0), mStack.top(1), true);
		mStack.drop(1);
		mStack.top(0).set_bool(!truth); // truth is b < a
		break;
	}
/// 0xAF ABC_ACTION_GREATERTHAN
/// Stack In:
///  b
///  a
/// Stack Out:
///  truth -- Truth of (a > b)
	case SWF::ABC_ACTION_GREATERTHAN:
	{
		bool truth;
		ABSTRACT_COMPARE(truth, mStack.top(0), mStack.top(1), false);
		mStack.drop(1);
		mStack.top(0).set_bool(truth); // truth is b < a
		break;
	}
/// 0xB0 ABC_ACTION_GREATEREQUALS
/// Stack In:
///  b
///  a
/// Stack Out:
///  truth -- Truth of (a >= b)
	case SWF::ABC_ACTION_GREATEREQUALS:
	{
		bool truth;
		ABSTRACT_COMPARE(truth, mStack.top(1), mStack.top(0), true);
		mStack.drop(1);
		mStack.top(0).set_bool(!truth); // truth is a < b
		break;
	}
/// 0xB1 ABC_ACTION_INSTANCEOF
/// Stack In:
///  super -- An object
///  val -- An object
/// Stack Out:
///  truth -- Truth of "val is an instance of super"
	case SWF::ABC_ACTION_INSTANCEOF:
	{
		bool truth;
		ABSTRACT_TYPELATE(truth, mStack.top(1), mStack.top(0));
		mStack.top(1).set_bool(truth);
		mStack.drop(1);
		break;
	}
/// 0xB2 ABC_ACTION_ISTYPE
/// Stream: V32 'name_id'
/// Stack In:
///  [ns] -- Namespace stuff
///  obj -- An object
/// Stack Out:
///  truth -- Truth of "obj is of the type given in (resolve)'name_id'"
	case SWF::ABC_ACTION_ISTYPE:
	{
		asName a = pool_name(mStream->read_V32(), mPoolObject);
		mStack.drop(completeName(a));
		// TODO: Namespace stuff?
		mStack.top(0).set_bool(mStack.top(0).conforms_to(a.getABCName()));
	}
/// 0xB3 ABC_ACTION_ISTYPELATE
/// Stack In:
///  type -- A type to match
///  obj -- An object
/// Stack Out:
///  truth -- Truth of "obj is of type"
	case SWF::ABC_ACTION_ISTYPELATE:
	{
		as_value type = pop_stack();
		as_value value = pop_stack();
		bool truth = value.to_object().get()->instanceOf(type.to_object().get());
		push_stack(as_value(truth));
		break;
	}
/// 0xB4 ABC_ACTION_IN
/// Stack In:
///  obj -- The object to search for it
///  name -- The name to find
/// Stack Out:
///  truth -- True if name is in current namespace or anywhere in object.
///   Don't look in the namespace if obj is a dictionary.
/// NB: Since there doesn't seem to be a way to make a dictionary, this
/// is not done. If there is, fix this lack.
	case SWF::ABC_ACTION_IN:
	{
		//TODO: mStack.top(1).set_bool(mStack.top(1).to_object().contains(mStack.top(0)));
		mStack.drop(1);
		break;
	}
/// 0xC0 ABC_ACTION_INCREMENT_I
/// See: 0x91 (ABC_ACTION_INCREMENT), but forces types to int, not double
	case SWF::ABC_ACTION_INCREMENT_I:
	{
		mStack.top(0) = mStack.top(0).to_int() + 1;
		break;
	}
/// 0xC1 ABC_ACTION_DECREMENT_I
/// See: 0x93 (ABC_ACTION_DECREMENT), but forces types to int, not double
	case SWF::ABC_ACTION_DECREMENT_I:
	{
		mStack.top(0) = mStack.top(0).to_int() - 1;
		break;
	}
/// 0xC2 ABC_ACTION_INCLOCAL_I
/// See: 0x92 (ABC_ACTION_INCLOCAL), but forces types to int, not double
	case SWF::ABC_ACTION_INCLOCAL_I:
	{
		boost::uint32_t foff = mStream->read_V32();
		mRegisters[foff] = mRegisters[foff].to_int() + 1;
		break;
	}
/// 0xC3 ABC_ACTION_DECLOCAL_I
/// See: 0x94 (ABC_ACTION_DECLOCAL), but forces types to int, not double
	case SWF::ABC_ACTION_DECLOCAL_I:
	{
		boost::uint32_t foff = mStream->read_V32();
		mRegisters[foff] = mRegisters[foff].to_int() - 1;
		break;
	}
/// 0xC4 ABC_ACTION_NEGATE_I
/// See: 0x90 (ABC_ACTION_NEGATE), but forces type to int, not double
	case SWF::ABC_ACTION_NEGATE_I:
	{
		mStack.top(0) = - mStack.top(0).to_int();
		break;
	}
/// 0xC5 ABC_ACTION_ADD_I
/// See: 0xA0 (ABC_ACTION_ADD), but forces type to int
	case SWF::ABC_ACTION_ADD_I:
	{
		mStack.top(1) = mStack.top(1).to_int() + mStack.top(0).to_int();
		mStack.drop(1);
		break;
	}
/// 0xC6 ABC_ACTION_SUBTRACT_I
/// See: 0xA1 (ABC_ACTION_SUBTRACT), but forces type to int
	case SWF::ABC_ACTION_SUBTRACT_I:
	{
		mStack.top(1) = mStack.top(1).to_int() - mStack.top(0).to_int();
		mStack.drop(1);
		break;
	}
/// 0xC7 ABC_ACTION_MULTIPLY_I
/// See: 0xA2 (ABC_ACTION_MULTIPLY), but forces type to int
	case SWF::ABC_ACTION_MULTIPLY_I:
	{
		mStack.top(1) = mStack.top(0).to_int() * mStack.top(1).to_int();
		mStack.drop(1);
		break;
	}
/// 0xD0 ABC_ACTION_GETLOCAL0
/// 0xD1 ABC_ACTION_GETLOCAL1
/// 0xD2 ABC_ACTION_GETLOCAL2
/// 0xD3 ABC_ACTION_GETLOCAL3
/// Frame: Load frame[#] as val
/// Stack Out:
///  val
	case SWF::ABC_ACTION_GETLOCAL0:
	case SWF::ABC_ACTION_GETLOCAL1:
	case SWF::ABC_ACTION_GETLOCAL2:
	case SWF::ABC_ACTION_GETLOCAL3:
	{
		//We shouldn't need to a call grow stack, because each function should now how big the stack will need to be and should allocate all the space, when it is loaded into the vm.
//		GROW_STACK();
//		mStack.grow(1);
//		mStack.push() instead?

		push_stack(get_register(opcode- SWF::ABC_ACTION_GETLOCAL0));
//		mStack.top(0) = mRegisters.value(opcode - SWF::ABC_ACTION_GETLOCAL0);
		break;
	}
/// 0xD4 ABC_ACTION_SETLOCAL0
/// 0xD5 ABC_ACTION_SETLOCAL1
/// 0xD6 ABC_ACTION_SETLOCAL2
/// 0xD7 ABC_ACTION_SETLOCAL3
/// Frame: Store val as frame[#]
/// Stack In:
///  val
/// Stack Out:
///  .
	case SWF::ABC_ACTION_SETLOCAL0:
	case SWF::ABC_ACTION_SETLOCAL1:
	case SWF::ABC_ACTION_SETLOCAL2:
	case SWF::ABC_ACTION_SETLOCAL3:
	{
		mRegisters[opcode - SWF::ABC_ACTION_SETLOCAL0] = pop_stack();
		break;
	}

/// 0xEF ABC_ACTION_DEBUG
/// Stream: 7 bytes of unknown stuff to be skipped
/// Do: skip ahead 7 bytes in stream
	case SWF::ABC_ACTION_DEBUG:
	{
		mStream->seekBy(7);
		break;
	}
/// 0xF0 ABC_ACTION_DEBUGLINE
/// Stream: V32 'line_number'
/// Do: Nothing, but line_number is for the debugger if wanted.
	case SWF::ABC_ACTION_DEBUGLINE:
	{
		mStream->skip_V32();
		break;
	}
/// 0xF1 ABC_ACTION_DEBUGFILE
/// Stream: V32 'name_offset'
/// Do: Nothing. 'name_offset' into string pool is the file name if wanted.
	case SWF::ABC_ACTION_DEBUGFILE:
	{
		mStream->skip_V32();
		break;
	}
/// 0xF2 ABC_ACTION_BKPTLINE
/// Stream: V32 'line_number'
/// Do: Enter debugger if present, line_number is the line number in source.
	case SWF::ABC_ACTION_BKPTLINE:
	{
		mStream->skip_V32();
		break;
	}
	} // end of switch statement
		LOG_DEBUG_AVM("* DONE *");
		IF_VERBOSE_ACTION(print_stack());
	} // end of AS3 conditional
	else // beginning of !AS3 (this code is AS2)
	{
	switch (opcode)
	{
	} // end of switch statement
	} // end of AS2 conditional (!AS3)
	} // end of main loop
} // end of execute function

void
Machine::getMember(asClass* pDefinition, asName& name,
	as_value& instance)
{
	if (!instance.is_object())
		throw ASTypeError();
#if 0
	if (!pBinding->isGetSet())
	{
		//TODO: mStack.push(pBinding->getFromInstance(instance));
		return;
	}

	// This is a getter, so we need to execute it. Even those
	// written in C++ get called like this, with pushCall handling.
	// And push the instance ('this')
	mStack.push(instance);
	pushCall(1, &mStack.top(0), pBinding); //TODO: pBinding->getGetter());
#else
UNUSED(pDefinition);
UNUSED(name);
#endif
}

void
Machine::setMember(asClass *pDefinition, asName& name, as_value& instance,
	as_value& newvalue)
{
	if (!instance.is_object())
		throw ASReferenceError();

	return;
	// TODO:
#if 0
	asBinding *pBinding = pDefinition->getBinding(name.getABCName());

	if (pBinding->isReadOnly())
		throw ASReferenceError();

	if (!pBinding->isGetSet())
	{
		//TODO: pBinding->setInInstance(instance, newvalue);
		return;
	}

	// Two parameters -- the target object, the value to set.
	mStack.push(instance);
	mStack.push(newvalue);
	pushCall(2, &mStack.top(1), pBinding); //TODO: pBinding->getSetter());
#else
UNUSED(pDefinition);
UNUSED(name);
UNUSED(newvalue);
#endif
}

int
Machine::completeName(asName &name, int offset)
{
	int size = 0;

	if (name.isRuntime())
	{
		as_value obj = mStack.top(offset);
		if (obj.is_object() && obj.to_object()->isQName())
			name.fill(obj.to_object().get());
		++size;

		if (name.isRtns())
			++size; // Ignore the Namespace.
	}
	else if (name.isRtns())
	{
		//TODO: This should be a namespace //name.setNamespace(mStack.top(offset));
		++size;
	}
	return size;
}

asClass *
Machine::findSuper(as_value &v, bool find_for_primitive)
{
	if (v.is_undefined() || v.is_null())
		return NULL;

	if (v.is_object())
	{
		asClass *pProto = NULL; // TODO: v.to_object()->getClass();
		return pProto ? pProto->getSuper() : NULL;
	}

	if (!find_for_primitive)
		return NULL;

	if (v.is_number())
	{
		return NULL; // TODO: mCH->getClass(NSV::CLASS_NUMBER);
	}

	// And so on...
	// TODO: Other primitives
	return NULL;
}

void
Machine::immediateFunction(const as_function *to_call, as_object *pThis,
	as_value& storage, unsigned char stack_in, short stack_out)
{
	// TODO: Set up the fn, or remove the need.
	fn_call fn(NULL, NULL, 0, 0);
	mStack.drop(stack_in - stack_out);
	saveState();
	mThis = pThis;
	mStack.grow(stack_in - stack_out);
	mStack.setDownstop(stack_in);
	storage = const_cast<as_function*>(to_call)->call(fn);
	restoreState();
}

void
Machine::pushGet(as_object *this_obj, as_value &return_slot, Property *prop)
{
	if (!prop)
		return;

	if (prop->isGetterSetter())
	{
		//TODO pushCall(prop->getGetter(), this_obj, return_slot, 0);
		return;
	}

	return_slot = prop->getValue(*this_obj);
}

void
Machine::pushSet(as_object *this_obj, as_value &value, Property *prop)
{
	if (!prop)
		return;

	if (prop->isGetterSetter())
	{
		mStack.push(value);
		//TODO pushCall(prop->getSetter(), this_obj, mIgnoreReturn, 1);
		return;
	}

	prop->setValue(*this_obj, value);
}

void
Machine::pushCall(as_function *func, as_object *pthis, as_value& return_slot,
	unsigned char stack_in, short stack_out)
{
	if (1 || func->isBuiltin())
	{
		immediateFunction(func, pthis, return_slot, stack_in, stack_out);
		return;
	}
	// TODO: Make this work for stackless.

	// Here is where the SafeStack shines:
	// We set the stack the way it should be on return.
	mStack.drop(stack_in - stack_out);
	// We save that state.
	saveState();
	// Set the 'this' for the new call
	mThis = pthis;
	// Retrieve the stack. (It wasn't lost)
	mStack.grow(stack_in - stack_out);
	// And then we set the downstop
	mStack.setDownstop(stack_in);

	// When control goes to the main loop of the interpreter, it will
	// automatically start executing the method.
}

void
Machine::restoreState()
{
	LOG_DEBUG_AVM("Restoring state.");
	State &s = mStateStack.top(0);
	s.to_debug_string();
//	mStack.setAllSizes(s.mStackTotalSize, s.mStackDepth);
//	mScopeStack.setAllSizes(s.mScopeTotalSize, s.mScopeStackDepth);
	mScopeStack.setAllSizes(s.mScopeTotalSize, s.mScopeStackDepth);
	mStream = s.mStream;
	mRegisters = s.mRegisters;
	mCurrentFunction = s.mFunction;
//	mExitWithReturn = s.mReturn;
//	mDefaultXMLNamespace = s.mDefaultXMLNamespace;
//	mCurrentScope = s.mCurrentScope;
//	mGlobalReturn = s.mGlobalReturn;
//	mThis = s.mThis;
//	mStateStack.drop(1);
	mStateStack.pop();
}

void
Machine::saveState()
{
	LOG_DEBUG_AVM("Saving state.");
	mStateStack.grow(1);
	State &s = mStateStack.top(0);
	s.mStackDepth = mStack.getDownstop();
	s.mStackTotalSize = mStack.totalSize();
	s.mScopeStackDepth = mScopeStack.getDownstop();
	s.mScopeTotalSize = mScopeStack.totalSize();
//	s.mScopeStackDepth = mScopeStack.getDownstop();
//	s.mScopeTotalSize = mScopeStack.totalSize();
	s.mStream = mStream;
	s.to_debug_string();
	s.mRegisters = mRegisters;
	s.mFunction = mCurrentFunction;
//	s.mReturn = mExitWithReturn;
//	s.mDefaultXMLNamespace = mDefaultXMLNamespace;
//	s.mCurrentScope = mCurrentScope;
//	s.mGlobalReturn = mGlobalReturn;
//	s.mThis = mThis;
}

void Machine::initMachine(abc_block* pool_block,as_object* global)
{
	mPoolObject = pool_block;
	log_debug("Getting entry script.");
	asClass* start_script = pool_block->mScripts.back();
	log_debug("Getting constructor.");
	asMethod* constructor = start_script->getConstructor();
	clearRegisters(constructor->getMaxRegisters());
	log_debug("Loding code stream.");
	mStream = constructor->getBody();
	mCurrentFunction = constructor->getPrototype();
	mRegisters[0] = as_value(global);
	mGlobalObject = global;
}

//This is called by abc_functions to execute their code stream.
//TODO: There is probably a better way to do this, once we understand what the VM is supposed
//todo, this should be fixed.
as_value Machine::executeFunction(asMethod* function, const fn_call& fn){
	
//TODO: Figure out a good way to use the State object to handle returning values.
	mCurrentFunction = function->getPrototype();
	bool prev_ext = mExitWithReturn;
	CodeStream *stream = function->getBody();
	load_function(stream, function->getMaxRegisters());
	mExitWithReturn = true;
	mRegisters[0] = as_value(fn.this_ptr);
	for(unsigned int i=0;i<fn.nargs;i++){
		mRegisters[i+1] = fn.arg(i);
	}
	//TODO:  There is probably a better way to do this.
	if(mCurrentFunction->mScopeStack){
		for(unsigned int i=0;i<mCurrentFunction->mScopeStack->size();++i){
			push_scope_stack(as_value(mCurrentFunction->mScopeStack->at(i)));
		}
	}
	execute();
	mExitWithReturn = prev_ext;
	stream->seekTo(0);

	return mGlobalReturn;
}

void Machine::executeCodeblock(CodeStream* stream){

	mStream = stream;
	execute();
//	restoreState();
}

void Machine::instantiateClass(std::string className, as_object* global){

	asClass* theClass = mPoolObject->locateClass(className);
	
	// TODO: what happens when it's not located?
	assert (theClass);
	clearRegisters(theClass->getConstructor()->getMaxRegisters());
	mCurrentFunction = theClass->getConstructor()->getPrototype();
	mStack.clear();
	mScopeStack.clear();
	mRegisters[0] = as_value(global);
	executeCodeblock(theClass->getConstructor()->getBody());
}

Machine::Machine(VM& vm):mST(vm.getStringTable()),mRegisters(),mExitWithReturn(false),_vm(vm)
{
	mCH = vm.getClassHierarchy();
	//Local registers should be initialized at the beginning of each function call, but
	//we don't currently parse the number of local registers for each function.
//	mRegisters.resize(16);
//	mST = new string_table();
//	mST = ST;
}

as_value Machine::find_prop_strict(asName multiname){
	
	as_value val;
	mScopeStack.push(mGlobalObject);
	for(int i=0;i<mScopeStack.size();i++){

		val = mScopeStack.top(i).get()->getMember(multiname.getGlobalName(),multiname.getNamespace()->getURI());

		if(!val.is_undefined()){
			push_stack(mScopeStack.top(i));
			mScopeStack.pop();
			return val;
		}
	}

	LOG_DEBUG_AVM("Cannot find property in scope stack.  Trying again using as_environment.");
	as_object *target = NULL;
	as_environment env = as_environment(_vm);
	std::string name = mPoolObject->mStringPool[multiname.getABCName()];
	std::string ns = mPoolObject->mStringPool[multiname.getNamespace()->getAbcURI()];
	std::string path = ns.size() == 0 ? name : ns + "." + name;
	val = env.get_variable(path,*getScopeStack(),&target);
	LOG_DEBUG_AVM("Got value.");
	push_stack(as_value(target));	
	mScopeStack.pop();
	return val;
}

as_value Machine::get_property_value(asName multiname){
	return get_property_value(NULL,multiname);
}

as_value Machine::get_property_value(boost::intrusive_ptr<as_object> obj, asName multiname){

	std::string ns = mPoolObject->mStringPool[multiname.getNamespace()->getAbcURI()];
	std::string name = mPoolObject->mStringPool[multiname.getABCName()];
	return get_property_value(obj,name,ns);
}

as_value Machine::get_property_value(boost::intrusive_ptr<as_object> obj, std::string name, std::string ns){

as_environment::ScopeStack stack;
	as_environment env = as_environment(_vm);
	if(obj == NULL){
		stack = *getScopeStack();
	}
	else{
		stack.push_back(obj);
	}
	std::string path;
	if(ns.size() == 0){
		path = name;
	}
	else{
		path = ns + "." + name;
	}

	return env.get_variable(path,stack,NULL);
}

void Machine::print_stack(){

	std::stringstream ss;
	ss << "Stack: ";
	for(unsigned int i=0;i<mStack.size();++i){
		if (i!=0) ss << " | ";
		ss << mStack.value(i).toDebugString();
	}
	LOG_DEBUG_AVM("%s", ss.str());
}

void Machine::print_scope_stack(){

	std::stringstream ss;
	ss << "ScopeStack: ";
	for(unsigned int i=0;i<mScopeStack.size();++i){
		ss << as_value(mScopeStack.top(i).get()).toDebugString();
	}
	LOG_DEBUG_AVM("%s", ss.str());
}	

std::auto_ptr< std::vector<as_value> > Machine::get_args(unsigned int argc){
	LOG_DEBUG_AVM("There are %u args",argc);
	std::auto_ptr< std::vector<as_value> > args = std::auto_ptr< std::vector<as_value> >(new std::vector<as_value>);
	args->resize(argc);
	for(unsigned int i=argc; i>0; --i){
		args->at(i-1) = pop_stack();
	}
	return args;
}

void Machine::load_function(CodeStream* stream,boost::uint32_t maxRegisters){
	saveState();
	//TODO: Maybe this call should be part of saveState(), it returns the old downstop.
	mScopeStack.fixDownstop();
	mStream = stream;
	clearRegisters(maxRegisters);
}

as_environment::ScopeStack* Machine::getScopeStack(){
	as_environment::ScopeStack *stack = new as_environment::ScopeStack();
	for(int i=0;i<mScopeStack.size();i++){
		stack->push_back(mScopeStack.top(i));
	}
	return stack;
}

void
Machine::clearRegisters(boost::uint32_t maxRegisters){
	mRegisters.clear();

	mRegisters.resize(maxRegisters);
}



} // end of namespace gnash
