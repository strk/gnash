// Machine.cpp A machine to run AS3 code, with AS2 code in the future
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
//

#include "Machine.h"
#include "as_object.h"
#include "ClassHierarchy.h"
#include "namedStrings.h"
#include "AbcBlock.h"
#include "MultiName.h"
#include "fn_call.h"
#include "abc_function.h"
#include "VM.h"
#include "Globals.h"
#include "Global_as.h"
#include "Class.h"
#include "CodeStream.h"
#include "SWF.h"

namespace gnash {
namespace abc {

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
	{}
};

/// Functions for getting pool constants.
//
/// TODO: it's quite possible for a malformed SWF to ask for out-of-bounds
/// pool access, although at the moment it's mainly Gnash bugs causing this.
/// Throwing an exception is good here, but it's not clear which one.
namespace {

inline const std::string&
pool_string(boost::uint32_t index, AbcBlock *pool)
{
	if (!pool) throw ASException();
    try {
        return pool->stringPoolAt(index);
    }
    catch (std::range_error& e) {
        throw ASException();
    }
}

inline int
pool_int(boost::uint32_t index, AbcBlock *pool)
{
	if (!pool) throw ASException();
    try {
        return pool->integerPoolAt(index);
    }
    catch (std::range_error& e) {
        throw ASException();
    }
}

inline unsigned int
pool_uint(boost::uint32_t index, AbcBlock *pool)
{
	if (!pool) throw ASException();
    try {
        return pool->uIntegerPoolAt(index);
    }
    catch (std::range_error& e) {
        throw ASException();
    }
}

inline double
pool_double(boost::uint32_t index, AbcBlock *pool)
{
	if (!pool) throw ASException();
    try {
        return pool->doublePoolAt(index);
    }
    catch (std::range_error& e) {
        throw ASException();
    }
}

inline Namespace*
pool_namespace(boost::uint32_t index, AbcBlock *pool)
{
	if (!pool) throw ASException();
    try {
        return pool->namespacePoolAt(index);
    }
    catch (std::range_error& e) {
        throw ASException();
    }

}

inline Method*
pool_method(boost::uint32_t index, AbcBlock* pool)
{
	if (!pool) throw ASException();
    try {
        return pool->methodPoolAt(index);
    }
    catch (std::range_error& e) {
        throw ASException();
    }
}

inline Class*
pool_script(boost::uint32_t index, AbcBlock* pool)
{
	if (!pool) throw ASException();
    try {
        return pool->classPoolAt(index);
    }
    catch (std::range_error& e) {
        throw ASException();
    }
}

// Don't make this a reference or you'll taint the pool.
inline MultiName
pool_name(boost::uint32_t index, AbcBlock* pool)
{
	if (!pool) throw ASException();
	try {
        MultiName multiname = pool->multinamePoolAt(index);
        return multiname;
    }
    catch (std::range_error& e) {
        throw ASException();
    }
}

} // anonymous namespace

/// ENSURE_NUMBER makes sure that the given argument is a number,
/// calling the valueOf method if necessary -- it's a macro so that
/// the valueOf method may be pushed if needed, and then whatever
/// opcode asked for this will be re-entered.
#define ENSURE_NUMBER(vte)													\
{																			\
	as_value *e = &vte;														\
	if (e->is_object())														\
	{																		\
		Property *b = e->to_object(*_global)->findProperty(NSV::PROP_VALUE_OF, 0);	\
		if (b)																\
		{																	\
			mStream->seekTo(opStart);										\
			pushGet(e->to_object(*_global), *e, b);							\
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
		Property *d = c->to_object(*_global)->findProperty(NSV::PROP_TO_STRING, 0);	\
		if (d)																\
		{																	\
			mStream->seekTo(opStart);										\
			pushGet(c->to_object(*_global), *c, d);							\
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
		else if (isInf(ad) && ad > 0)	 									\
			store = false; 													\
		else if (isInf(bd) && bd > 0)	 									\
			store = true; 													\
		else if (isInf(bd) && bd < 0)	 									\
			store = false; 													\
		else if (isInf(ad) && ad < 0)										\
			store = true;													\
		else 																\
			store = ad < bd; 												\
	} 																		\
}												/* end of ABSTRACT_COMPARE */

inline bool abstractEquality(const as_value& a, const as_value& b,
       bool strictness_on)
{
    // TODO: this is a very quick hack to fix some tests without touching
    // as_value. Tamarin has a detailed algorithm for working out equality,
    // which can be implemented as a separate member function of as_value.
    if (a.is_object() && !b.is_object()) {
        return a.to_string() == b.to_string();
    }
    if ( strictness_on ) return a.strictly_equals(b);
    else return a.equals(b);
}								

/// NB: the stubbed but unimplemented as_value::conforms_to no longer exists,
/// but the code is left here for later reference.
#define ABSTRACT_TYPELATE(st, checkval, matchval)							\
{																			\
	bool *store = &st;														\
	/*as_value &a = checkval;  Don't call checkval multiple times */		\
	as_value &b = matchval; /* Don't call matchval multiple times */		\
	*store = true;															\
	if (b.is_object())														\
	{																		\
		as_value v;															\
		b.to_object(*_global)->get_member(NSV::INTERNAL_TYPE, &v);					\
		if (true) /*(!a.conforms_to(mST.find(v.to_string()))) */	\
			*store = false;													\
	}																		\
	else if (b.is_string())													\
	{																		\
		if (true) /*(!a.conforms_to(mST.find(b.to_string())))	*/   	    \
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

namespace {

/// Switch the execution context to AVM2, and make sure it's
/// switched back again to what it was before even when there's an exception.
class AVM2Switcher
{
public:
    AVM2Switcher(VM& vm)
        :
        _vm(vm),
        _ver(vm.getAVMVersion())
    {
        _vm.setAVMVersion(VM::AVM2);
    }

    ~AVM2Switcher()
    {
        _vm.setAVMVersion(_ver);
    }

private:
    VM& _vm;
    VM::AVMVersion _ver;
};

}

Machine::Machine(VM& vm)
    :
    _stack(),
    _registers(),
    _scopeStack(),
    mStream(0),
    mST(vm.getStringTable()),
    mDefaultXMLNamespace(0),
    mCurrentScope(0),
    mGlobalScope(0),
    mDefaultThis(0),
    mThis(0),
    _global(0),
    mGlobalReturn(),
    mIgnoreReturn(),
    mExitWithReturn(false),
    mPoolObject(0),
    mCurrentFunction(0),
    _vm(vm)
{
               
}

void
Machine::init()
{

    // TODO: The Global constructor needs the Machine and VM to be more or less
    // fully constructed, so we might think how to do this better.
    AVM2Global* g = new AVM2Global(*this, _vm);
    _global = g;
    
    AVM2Switcher switcher(_vm);
    g->registerClasses();

}

Global_as*
Machine::global()
{
    return _global;
}

void
Machine::push_scope_stack(as_value object)
{
    as_object* scopeObj = object.to_object(*_global);
    assert(scopeObj);
    log_abc("Pushing value %s onto scope stack.", object);
    _scopeStack.push(scopeObj);
    print_scope_stack();
}

void
Machine::execute()
{

    // This automatically switches back again when we leave this scope.
    AVM2Switcher avm2(_vm);

    assert(mStream);

	for (;;) {
		std::size_t opStart = mStream->tellg();
        
        try {

            SWF::abc_action_type opcode = static_cast<SWF::abc_action_type>(
                    mStream->read_as3op());
            
            log_abc("** Executing opcode: %s (%d) **", opcode, (int)opcode);
            
            switch (opcode) 
            {
                default:
                    throw ASException();
                
                /// This is not actually an opcode -- it occurs when the
                /// stream is empty. We may need to return from a function,
                /// or we may be done.
                case SWF::ABC_ACTION_END:
                    return;

                /// 0x01 ABC_ACTION_BKPT
                /// Do: Enter the debugger if one has been invoked.
                /// This is a no-op. Enable it if desired.
                /// 0x02 ABC_ACTION_NOP
                /// Do: Nothing.
                /// 0xF3 ABC_ACTION_TIMESTAMP
                case SWF::ABC_ACTION_NOP:
                case SWF::ABC_ACTION_BKPT:
                case SWF::ABC_ACTION_TIMESTAMP:
                    break;

                /// 0x03 ABC_ACTION_THROW
                /// Stack In:
                ///  obj -- an object
                /// Stack Out:
                ///  .
                /// Do: Throw obj as an exception
                /// Equivalent: ACTIONTHROW
                case SWF::ABC_ACTION_THROW:
                {
                    throw ASException(_stack.pop());
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
                ///  May be the same as the value of obj.name (E.g. inherited
                /// variables)
                case SWF::ABC_ACTION_GETSUPER:
                {
                    // Get the name.
                    MultiName a = pool_name(mStream->read_V32(), mPoolObject);
                    // Finish it, if necessary.
                    _stack.drop(completeName(a));
                    // Get the target object.
                    ENSURE_OBJECT(_stack.top(0));
                    
                    as_object *super = _stack.top(0).to_object(*_global)->
                        get_prototype();
                    
                    // If we don't have a super, throw.
                    if (!super) throw ASReferenceError();
                    const ObjectURI uri(a.getGlobalName(),
                            a.getNamespace()->getURI());
                    Property *b = super->findProperty(uri);
                    // The object is on the top already.
                    pushGet(super, _stack.top(0), b);
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
                    MultiName a = pool_name(mStream->read_V32(), mPoolObject);
                    as_value vobj = _stack.pop(); // The value

                    _stack.drop(completeName(a));

                    ENSURE_OBJECT(_stack.top(0));
                    
                    // This is all wrong. It needs fixing once supers are
                    // correctly implemented.
                    as_object* obj = _stack.pop().to_object(*_global);
                    if (!obj) throw ASReferenceError();

                    as_object* super = obj->get_prototype();
                    if (!super) throw ASReferenceError();

                    const ObjectURI uri(a.getGlobalName(),
                            a.getNamespace()->getURI());
                    Property* b = super->findProperty(uri);

                    _stack.push(vobj);
                    pushSet(super, vobj, b);
                    break;
                }

                /// 0x06 ABC_ACTION_DXNS
                /// Default XML Namespace
                /// Stream: UV32 index to string pool 'nsname'
                /// Do: Create a new public namespace with name nsname, and make
                /// this the default XML namespace. 
                case SWF::ABC_ACTION_DXNS:
                {
                    boost::uint32_t soffset = mStream->read_V32();
                    const std::string& uri = pool_string(soffset, mPoolObject);

                    ClassHierarchy& ch = _global->classHierarchy();
                    mDefaultXMLNamespace = ch.anonNamespace(mST.find(uri));
                    break;
                }

                /// 0x07 ABC_ACTION_DXNSLATE
                /// Stack In:
                ///  nsname -- a string object
                /// Stack Out:
                ///  .
                /// Do: Same as ABC_ACTION_DXNS, but the uri is in the stack,
                /// not the stream.
                case SWF::ABC_ACTION_DXNSLATE:
                {
                    ENSURE_STRING(_stack.top(0));
                    const std::string& uri = _stack.top(0).to_string();
                    
                    ClassHierarchy& ch = _global->classHierarchy();
                    mDefaultXMLNamespace = ch.anonNamespace(mST.find(uri));
                    _stack.drop(1);
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
                    setRegister(regNum, as_value());
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
                /// Do: If !(a < b) move by jump in stream, as ABC_ACTION_JUMP
                /// does.
                case SWF::ABC_ACTION_IFNLT:
                {
                    bool truth;
                    ABSTRACT_COMPARE(truth, _stack.top(1), _stack.top(0),
                            false);
                    _stack.drop(2);
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
                /// Do: If !(a <= b) move by jump in stream, as ABC_ACTION_JUMP
                /// does.
                case SWF::ABC_ACTION_IFNLE:
                {
                    bool truth;
                    ABSTRACT_COMPARE(truth, _stack.top(0), _stack.top(1), true);
                    _stack.drop(2);
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
                /// Do: If !(a > b) move by jump in stream, as ABC_ACTION_JUMP
                /// does.
                case SWF::ABC_ACTION_IFNGT:
                {
                    bool truth;
                    ABSTRACT_COMPARE(truth, _stack.top(0), _stack.top(1),
                            false);
                    _stack.drop(2);
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
                /// Do: If !(a >= b) move by jump in stream, as ABC_ACTION_JUMP
                /// does.
                case SWF::ABC_ACTION_IFNGE:
                {
                    bool truth;
                    ABSTRACT_COMPARE(truth, _stack.top(1), _stack.top(0), true);
                    _stack.drop(2);
                    JUMPIF(truth); // truth is: a < b
                    break;
                }

                /// 0x10 ABC_ACTION_JUMP
                /// Stream: S24 jump offset 'jump'
                /// Do: If jump is negative, check for interrupts. Move by
                /// jump in stream.
                /// Equivalent: ACTION_BRANCHALWAYS
                case SWF::ABC_ACTION_JUMP:
                {
                    const boost::int32_t bytes = mStream->read_S24();
                    log_abc("ABC_ACTION_JUMP: Jumping %d bytes.",bytes);
                    mStream->seekBy(bytes);
                    break;
                }

                /// 0x11 ABC_ACTION_IFTRUE
                /// Stream: S24 jump offset 'jump'
                /// Stack In:
                ///  a -- an object
                /// Stack Out:
                ///  .
                /// Do: If a is 'true', move by jump in stream, as
                /// ABC_ACTION_JUMP does.
                /// Equivalent: ACTION_BRANCHIFTRUE
                case SWF::ABC_ACTION_IFTRUE:
                {
                    const boost::int32_t bytes = mStream->read_S24();
                    if (pop_stack().to_bool()) {
                        log_abc("ABC_ACTION_IFTRUE: Jumping %d bytes.",bytes);
                        mStream->seekBy(bytes);
                    }
                    else {
                        log_abc("ABC_ACTION_IFTRUE: Would have jumped %d "
                                "bytes.", bytes);
                    }
                    break;
                }

                /// 0x12 ABC_ACTION_IFFALSE
                /// Stream: S24 jump offset 'jump'
                /// Stack In:
                ///  a -- an object
                /// Stack Out:
                ///  .
                /// Do: If a is 'false', move by jump in stream, as
                /// ABC_ACTION_JUMP does.
                case SWF::ABC_ACTION_IFFALSE:
                {
                    const boost::int32_t bytes = mStream->read_S24();
                    const bool truth = pop_stack().to_bool();
                    if (!truth) {
                        log_abc("ABC_ACTION_IFFALSE: Jumping %d bytes.", bytes);
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
                /// Do: If a == b (weakly), move by jump in stream, as
                /// ABC_ACTION_JUMP does.
                case SWF::ABC_ACTION_IFEQ:
                {
                    const boost::int32_t bytes = mStream->read_S24();
                    const as_value b = pop_stack();
                    const as_value a = pop_stack();
                    if (a.equals(b)) {
                        log_abc("Jumping %d bytes.", bytes);
                        mStream->seekBy(bytes);
                    }
                    else{
                        log_abc("Would have jumped %d bytes", bytes);
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
                /// Do: If a != b (weakly), move by jump in stream, as
                /// ABC_ACTION_JUMP does.
                case SWF::ABC_ACTION_IFNE:
                {
                    as_value a = pop_stack();
                    as_value b = pop_stack();
                    const boost::int32_t bytes = mStream->read_S24();
                    if (!a.equals(b)) {
                        log_abc("Jumping... %d bytes.", bytes);
                        mStream->seekBy(bytes);
                    }
                    else {
                        log_abc("Would have jumped %d bytes", bytes);
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
                /// Do: If a < b move by jump in stream, as ABC_ACTION_JUMP
                /// does.
                case SWF::ABC_ACTION_IFLT:
                {
                    as_value b = pop_stack();
                    as_value a = pop_stack();
                    const boost::int32_t bytes = mStream->read_S24();
                    const bool jump = newLessThan(a, b, _vm).to_bool();
                    if (jump) {
                        log_abc("Jumping... %d bytes.", bytes);
                        mStream->seekBy(bytes);
                    }
                    else {
                        log_abc("Would have jumped %d bytes", bytes);
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
                /// Do: If a <= b move by jump in stream, as ABC_ACTION_JUMP
                /// does.
                case SWF::ABC_ACTION_IFLE:
                {
                    bool truth;
                    ABSTRACT_COMPARE(truth, _stack.top(0), _stack.top(1), true);
                    _stack.drop(2);
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
                    ABSTRACT_COMPARE(truth, _stack.top(0), _stack.top(1), false);
                    _stack.drop(2);
                    if (truth) {
                        log_abc("Jumping %d bytes.",bytes);
                        mStream->seekBy(bytes);
                    }
                    else{
                        log_abc("Would have jumped %d bytes.",bytes);
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
                /// Do: If a >= b move by jump in stream, as ABC_ACTION_JUMP
                /// does.
                case SWF::ABC_ACTION_IFGE:
                {
                    bool truth;
                    ABSTRACT_COMPARE(truth, _stack.top(0), _stack.top(1), true);
                    _stack.drop(2);
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
                /// Do: If a == b (strictly), move by jump in stream, as
                /// ABC_ACTION_JUMP
                case SWF::ABC_ACTION_IFSTRICTEQ:
                {
                    bool truth = abstractEquality(_stack.top(1), _stack.top(0),
                            true);
                    _stack.drop(2);
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
                /// Do: If a != b (strongly), move by jump in stream, as
                /// ABC_ACTION_JUMP
                case SWF::ABC_ACTION_IFSTRICTNE:
                {
                    const bool truth = abstractEquality(_stack.top(1),
                            _stack.top(0), true);
                    _stack.drop(2);
                    JUMPIF(!truth);
                    break;
                }

                /// 0x18 ABC_ACTION_LOOKUPSWITCH
                /// Stream: 3 bytes | V32 count as 'case_count - 1' |
                /// case_count of S24 as 'cases'
                /// Stack In:
                ///  index -- an integer object
                /// Stack Out:
                ///  .
                /// Do: If index >= case_count, reset stream to position on
                /// op entry. Otherwise, move by cases[index] - 1 from stream
                /// position on op entry.
                case SWF::ABC_ACTION_LOOKUPSWITCH:
                {
                    std::size_t npos = mStream->tellg();
                    if (!_stack.top(0).is_number()) throw ASException();

                    boost::uint32_t index =
                        toNumber(_stack.top(0), getVM(fn));
                    _stack.drop(1);

                    mStream->seekBy(3); // Skip the intial offset.
                    boost::uint32_t cases = mStream->read_V32();
                    // Read from our original position and use it to skip
                    // if the case is out of range.
                    if (index > cases) {
                        mStream->seekTo(npos);
                        mStream->seekTo(npos + mStream->read_S24());
                    }
                    else {
                        mStream->seekTo(npos + 3 * (index + 1));
                        boost::uint32_t newpos = mStream->read_S24();
                        mStream->seekTo(npos - 1 + newpos);
                    }
                    break;
                }

                /// 0x30 ABC_ACTION_PUSHSCOPE
                case SWF::ABC_ACTION_PUSHSCOPE:
                {
                    as_value scope_value = pop_stack();
                    if (!scope_value.to_object(*_global)) {
                        // Should throw an exception.
                        IF_VERBOSE_ASCODING_ERRORS(
                        log_aserror(_("Can't push a null value onto the "
                                "scope stack (%s)."), scope_value);
                        );
                        break;
                    }	
                    push_scope_stack(scope_value);
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
                    log_unimpl("ABC_ACTION_PUSHWITH");
                    // A scope object is just a regular object.
            // 		ENSURE_OBJECT(_stack.top(0));
            // 		as_object *a = _stack.top(0).to_object(*_global);
            // 
            // 		if (!_scopeStack.empty())
            // 			a->set_prototype(_scopeStack.top(0).mScope);
            // 		else
            // 			a->set_prototype(NULL);
            // 
            // 		if (opcode == SWF::ABC_ACTION_PUSHWITH &&
            // 				_scopeStack.totalSize() == _scopeStack.size())
            // 		{
            // 			_scopeStack.push(Scope(0, a));
            // 		}
            // 		else
            // 		{
            // 			_scopeStack.push(Scope(_scopeStack.size(), a));
            // 		}
            // 		mCurrentScope = a;
            // 		_stack.drop(1);
                    break;
                }

                /// 0x1D ABC_ACTION_POPSCOPE
                /// Do: exit current scope. Clear the base if the depth is now 
                ///  shallower than the base's depth.
                case SWF::ABC_ACTION_POPSCOPE:
                {
                    pop_scope_stack();
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
                    ENSURE_NUMBER(_stack.top(0));
                    ENSURE_OBJECT(_stack.top(1));
                    as_object *obj = _stack.top(1).to_object(*_global);
                    const boost::uint32_t index =
                        toNumber(_stack.top(0), getVM(fn));
                    
                    if (!obj) {
                        // TODO: check what to do here.
                        log_debug("ABC_ACTION_NEXTNAME: expecting object on "
                                "stack, got %s", _stack.top(1));
                        _stack.drop(2);
                        break;
                    }
                    
                    _stack.drop(1);
                    const Property *b = obj->getByIndex(index);
                    if (b) _stack.top(0) = mST.value(getName(b->uri()));
                    else _stack.top(0) = "";
                    break;
                }

                /// 0x1F ABC_ACTION_HASNEXT
                /// Stack In:
                ///  index -- an integer object
                ///  obj -- an object
                /// Stack Out:
                ///  next_index -- next index after index in obj, or 0 if none.
                /// Do: If the index is 0, return the first logical property.
                /// We'll do this by name, since the name id can be used for
                /// this directly.
                case SWF::ABC_ACTION_HASNEXT:
                {
                    ENSURE_NUMBER(_stack.top(0));
                    ENSURE_OBJECT(_stack.top(1));
                    as_object *obj = _stack.top(1).to_object(*_global);
                    boost::uint32_t index =
                        toNumber(_stack.top(0), getVM(fn));
                    _stack.drop(1);
                    assert(obj);
                    _stack.top(0) = obj->nextIndex(index);
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
                    _stack.grow(1);
                    _stack.top(0).set_undefined();
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
                    ENSURE_NUMBER(_stack.top(0));
                    ENSURE_OBJECT(_stack.top(1));
                    as_object *obj = _stack.top(1).to_object(*_global);
                    const boost::uint32_t index =
                        toNumber(_stack.top(0), getVM(fn));
                    const Property *b = obj->getByIndex(index);
                    _stack.drop(1);
                    if (!b) _stack.top(0).set_undefined();
                    else {
                        _stack.drop(1);
                        pushGet(obj, _stack.top(0), const_cast<Property*>(b));
                    }
                    break;
                }

                /// 0x24 ABC_ACTION_PUSHBYTE
                /// Stream: S8 as 'byte'
                /// Stack Out:
                ///  byte -- as a raw byte
                case SWF::ABC_ACTION_PUSHBYTE:
                {
                    const boost::int8_t b = mStream->read_s8();
                    push_stack(b);
                    break;
                }

                /// 0x25 ABC_ACTION_PUSHSHORT
                /// Stream: V32 as 'value'
                /// Stack Out:
                ///  value -- as a raw integer
                case SWF::ABC_ACTION_PUSHSHORT:
                {
                    const boost::int16_t s =
                        static_cast<boost::int16_t>(mStream->read_V32());
                    push_stack(s);
                    break;
                }

                /// 0x26 ABC_ACTION_PUSHTRUE
                /// Stack Out:
                ///  true -- the True object
                case SWF::ABC_ACTION_PUSHTRUE:
                    _stack.grow(1);
                    _stack.top(0).set_bool(true);
                    break;

                /// 0x27 ABC_ACTION_PUSHFALSE
                /// Stack Out:
                ///  false -- the False object
                case SWF::ABC_ACTION_PUSHFALSE:
                    push_stack(false);
                    break;

                /// 0x28 ABC_ACTION_PUSHNAN
                /// Stack Out:
                ///  NaN -- the NaN object
                case SWF::ABC_ACTION_PUSHNAN:
                    _stack.grow(1);
                    setNaN(_stack.top(0));
                    break;
            
                /// 0x29 ABC_ACTION_POP
                /// Stack In:
                ///  a -- anything
                /// Stack Out:
                ///  .
                case SWF::ABC_ACTION_POP:
                    pop_stack();
                    break;
            
                /// 0x2A ABC_ACTION_DUP
                /// Stack In:
                ///  a -- anything
                /// Stack Out:
                ///  a
                ///  a
                case SWF::ABC_ACTION_DUP:
                    _stack.grow(1);
                    _stack.top(0) = _stack.top(1);
                    break;

                /// 0x2B ABC_ACTION_SWAP
                /// Stack In:
                ///  a -- anything
                ///  b -- anything
                /// Stack Out:
                ///  b
                ///  a
                case SWF::ABC_ACTION_SWAP:
                {
                    as_value inter = _stack.top(0);
                    _stack.top(0) = _stack.top(1);
                    _stack.top(1) = inter;
                    break;
                }

                /// 0x2C ABC_ACTION_PUSHSTRING
                /// Stream: V32 string pool index 'index'
                /// Stack Out:
                ///  value -- String object from string_pool[index]
                case SWF::ABC_ACTION_PUSHSTRING:
                    push_stack(pool_string(mStream->read_V32(), mPoolObject));
                    break;

                /// 0x2D ABC_ACTION_PUSHINT
                /// Stream: V32 int pool index 'index'
                /// Stack Out:
                ///  value -- Integer object from integer_pool[index]
                case SWF::ABC_ACTION_PUSHINT:
                    push_stack(pool_int(mStream->read_V32(), mPoolObject));
                    break;

                /// 0x2E ABC_ACTION_PUSHUINT
                /// Stream: V32 uint pool index 'index'
                /// Stack Out:
                ///  value -- Unsigned Integer object from unsigned_integer_pool[index]
                case SWF::ABC_ACTION_PUSHUINT:
                {
                    _stack.grow(1);
                    _stack.top(0) = pool_uint(mStream->read_V32(), mPoolObject);
                    break;
                }

                /// 0x2F ABC_ACTION_PUSHDOUBLE
                /// Stream: V32 double pool index 'index'
                /// Stack Out:
                ///  value -- Double object from double_pool[index]
                case SWF::ABC_ACTION_PUSHDOUBLE:
                {
                    push_stack(as_value(pool_double(mStream->read_V32(), mPoolObject)));
                    break;
                }

                /// 0x31 ABC_ACTION_PUSHNAMESPACE
                /// Stream: V32 namespace pool index 'index'
                /// Stack Out:
                ///  ns -- Namespace object from namespace_pool[index]
                case SWF::ABC_ACTION_PUSHNAMESPACE:
                {
#if 0
                    Namespace *ns = pool_namespace(mStream->read_V32(),
                            mPoolObject);
                    _stack.grow(1);

                    // Here we should probably construct a Namespace object,
                    // but there is no need to do it using as_value's
                    // constructor.

                    _stack.top(0) = *ns;
                    break;
#endif
                }

                /// 0x32 ABC_ACTION_HASNEXT2
                /// Stream: V32 frame location 'objloc' | V32 frame location
                /// 'indexloc'
                /// Stack Out:
                ///  truth -- True if frame[objloc] has key/val pair after
                ///   frame[indexloc], following delegates (__proto__) objects
                ///   if needed. False, otherwise.
                /// Frame:
                ///  Change at objloc to object which possessed next value.
                ///  Change at indexloc to index (as object) of the next value.
                /// N.B.: A value of '0' for indexloc initializes to the
                /// first logical property.
                case SWF::ABC_ACTION_HASNEXT2:
                {
                    const boost::int32_t oindex = mStream->read_V32();
                    const boost::int32_t iindex = mStream->read_V32();

                    const as_value& objv = getRegister(oindex);
                    const as_value& indexv = getRegister(iindex);
                    
                    log_abc("HASNEXT2: Object is %s, index is %d",
                            objv, indexv);

                    as_object *obj = objv.to_object(*_global);
                    if (!obj) {
                        // TODO: Check what to do here.
                        log_error("ABC_ACTION_HASNEXT2: expecting object in "
                                "register %d, got %s", oindex, objv);
                        // Stack is unchanged, so just break? Or push false?
                        // Or throw?
                        break;
                    }
                    
                    boost::uint32_t index = toInt(indexv);

                    as_object *owner = 0;
                    int next = obj->nextIndex(index, &owner);
                    log_abc("HASNEXT2: Next index is %d", next);

                    if (next) {
                        push_stack(true);
                        if (owner) setRegister(oindex, owner);
                        else {
                            as_value null;
                            null.set_null();
                            setRegister(oindex, null);
                        }
                        setRegister(iindex, next);
                    }
                    else {
                        push_stack(false);
                        as_value null;
                        null.set_null();
                        setRegister(oindex, null);
                        setRegister(iindex, 0.0);
                    }
                    break;
                }

                /// 0x40 ABC_ACTION_NEWFUNCTION
                /// Stream: V32 'index'
                /// Stack Out:
                ///  func -- the function object
                /// Do: Information about function is in the pool at index. Construct
                /// the function from this information and bind the current scope.
                case SWF::ABC_ACTION_NEWFUNCTION:
                {
                    boost::int32_t method_index = mStream->read_V32();
                    log_abc("Creating new abc_function: method index=%u",method_index);
                    Method *m = pool_method(method_index, mPoolObject);
                    abc_function* new_function = m->getPrototype();
                    
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
                    ENSURE_OBJECT(_stack.top(argc + 1)); // The func
                    ENSURE_OBJECT(_stack.top(argc)); // The 'this'
                    as_function *f = _stack.top(argc + 1).to_function();
                    as_object *obj = _stack.top(argc).to_object(*_global);
                    // We start with argc + 2 values related to this call
                    // on the stack. We want to end with 1 value. We pass
                    // argc values (the parameters), so we need to drop
                    // one more than we pass and store the return just
                    // below that one. Thus:
                    // return is _stack.top(argc + 1)
                    // bottom of arguments is argc deep
                    // drop 1 more value than is passed, on return
                    pushCall(f, obj, _stack.top(argc + 1), argc, -1);
                    break;
                }

                /// 0x42 ABC_ACTION_CONSTRUCT
                /// Stream: V32 'arg_count'
                /// Stack In:
                ///  argN ... arg1 -- the arg_count arguments to pass
                ///  function -- constructor for the object to be constructed
                /// Stack Out:
                ///  value -- obj after it has been constructed as
                ///  obj(arg1, ..., argN)
                case SWF::ABC_ACTION_CONSTRUCT:
                {
                    boost::uint32_t argc = mStream->read_V32();
                    as_function *f = _stack.top(argc).to_function();
                    if (!f) {
                        log_abc("CONSTRUCT: No function on stack!");
                        break;
                    }
                    Property b(0, 0, f, NULL);
                    pushCall(f, NULL, _stack.top(argc), argc, 0);
                    break;
                }

                /// 0x43 ABC_ACTION_CALLMETHOD
                /// Stream: V32 'method_id + 1' | V32 'arg_count'
                /// Stack In:
                ///  argN ... arg1 -- the arg_count arguments to pass
                ///  obj -- the object to be called
                /// Stack Out:
                ///  value -- the value returned by obj::'method_id'(arg1,
                ///         ..., argN)
                case SWF::ABC_ACTION_CALLMETHOD:
                {
                    boost::uint32_t dispatch_id = mStream->read_V32() - 1;
                    boost::uint32_t argc = mStream->read_V32();
                    ENSURE_OBJECT(_stack.top(argc));
                    as_object *obj = _stack.top(argc).to_object(*_global);
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
                        func = f->getValue(*obj).to_function();
                    else
                    {
                        // Definitely an error, and not the kind we can handle.
                        throw ASException();
                    }
                    pushCall(func, obj, _stack.top(argc), argc, 0);
                    break;
                }

                /// 0x44 ABC_ACTION_CALLSTATIC
                /// Stream: V32 'method_id' | V32 'arg_count'
                /// Stack In:
                ///  argN ... arg1 -- the arg_count arguments to pass
                ///  obj -- the object to act as a receiver for the static call
                /// Stack Out:
                ///  value -- the value returned by obj->ABC::'method_id' (arg1,
                ///  ..., argN)
                case SWF::ABC_ACTION_CALLSTATIC:
                {
                    Method *m = pool_method(mStream->read_V32(), mPoolObject);
                    boost::uint32_t argc = mStream->read_V32();
                    as_function *func = m->getPrototype();
                    ENSURE_OBJECT(_stack.top(argc));
                    as_object *obj = _stack.top(argc).to_object(*_global);
                    pushCall(func, obj, _stack.top(argc), argc, 0);
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
                ///  0x45: value -- the value returned by obj::(resolve)'name_offset'::
                ///        super(arg1, ..., argN)
                ///  0x4E: .
                case SWF::ABC_ACTION_CALLSUPER:
                case SWF::ABC_ACTION_CALLSUPERVOID:
                {
                    MultiName a = pool_name(mStream->read_V32(), mPoolObject);
                    boost::uint32_t argc = mStream->read_V32();
                    int dropsize = completeName(a);
                    ENSURE_OBJECT(_stack.top(argc + dropsize));
                    _stack.drop(dropsize);

                    // get_super is wrong!
                    as_object* super =
                        _stack.top(argc).to_object(*_global)->get_super();

                    if (!super) throw ASReferenceError();
                    
                    const ObjectURI uri(a.getGlobalName(),
                            a.getNamespace()->getURI());
                    Property* b = super->findProperty(uri);
                    
                    if (!b) throw ASReferenceError();
                    
                    as_function *f = // b->isGetterSetter() ? b->getGetter() :
                        b->getValue(*super).to_function();

                    if (opcode == SWF::ABC_ACTION_CALLSUPER) {
                        pushCall(f, super, _stack.top(argc), argc, 0);
                    }
                    else {
                        // Void call
                        pushCall(f, super, mIgnoreReturn, argc, -1); // drop obj too.
                    }
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
                ///  value -- the value from obj::(resolve)'name_offset'
                ///      (arg1, ..., argN)
                ///  (unless ABC_ACTION_CALL_PROPVOID, then: . )
                /// NB: Calls getter/setter if they exist.
                /// If the opcode is ABC_ACTION_CALLPROPLEX, obj is
                /// not altered by getter/setters
                case SWF::ABC_ACTION_CALLPROPERTY:
                case SWF::ABC_ACTION_CALLPROPLEX:
                case SWF::ABC_ACTION_CALLPROPVOID:
                {
                    as_value result;
                    MultiName a = pool_name(mStream->read_V32(), mPoolObject);
                    boost::uint32_t argc = mStream->read_V32();

                    fn_call::Args args;
                    get_args(argc, args);

                    if (a.isRuntime()) {
                        _stack.drop(completeName(a));
                    }
                    
                    log_abc("CALL_PROP*: calling property %s of object %s",
                            mST.value(a.getGlobalName()), _stack.top(0));

                    as_value object_val = pop_stack();

                    as_object *object = object_val.to_object(*_global);
                    if (!object) {
                        log_abc(_("CALLPROP: Can't call a method of a value "
                                "that doesn't cast to an object (%s)."),
                                object_val);
                    }
                    else {

                        as_value property =
                            getMember(*object, a.getGlobalName());
                    
                        if (!property.is_undefined() && !property.is_null()) {
                            log_abc("Calling method %s on object %s",
                                    property, object_val);
                            as_environment env = as_environment(_vm);
                            result = invoke(property,env,object,args);

                        }
                        else {
                            log_abc(_("CALLPROP: Property '%s' of object '%s' "
                                    "is '%s', cannot call as method"),
                                    mPoolObject->stringPoolAt(a.getABCName()),
                                    object_val, property);
                        }

                    }
                    
                    if (opcode == SWF::ABC_ACTION_CALLPROPERTY) {
                        push_stack(result);
                    }

            /*		int shift = completeName(a, argc);
                    ENSURE_OBJECT(_stack.top(shift + argc));
                    as_object *obj = _stack.top(argc + shift).to_object(*_global);
                    Property *b = obj->findProperty(a.getABCName(), 
                        a.getNamespace()->getURI());
                    if (!b)
                        throw ASReferenceError();

                    as_function *func;
                    if (b->isGetterSetter())
                    {
                        if (lex_only)
                        {
                            _stack.top(argc + shift).set_undefined();
                            _stack.drop(argc + shift);
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
                        func = b->getValue(obj).to_function();

                    if (opcode == SWF::ABC_ACTION_CALLPROPVOID)
                        pushCall(func, obj, mIgnoreReturn, argc, -shift - 1);
                    else
                        pushCall(func, obj, _stack.top(argc + shift), argc, -shift);*/
                    break;
                }
                
                /// 0x47 ABC_ACTION_RETURNVOID
                /// Do: Return an undefined object up the callstack.
                case SWF::ABC_ACTION_RETURNVOID:
                    mStream->seekTo(0);
                    if (!mStateStack.size()) return;
                    
                    mGlobalReturn = as_value();
                    restoreState();
                    
                    if (mExitWithReturn) return;
                    break;
                
                /// 0x48 ABC_ACTION_RETURNVALUE
                /// Stack In:
                ///  value -- value to be returned
                /// Stack Out:
                ///  .
                /// Do: Return value up the callstack.
                case SWF::ABC_ACTION_RETURNVALUE:
                    mStream->seekTo(0);
                    
                    // Slot the return.
                    mGlobalReturn = pop_stack();
                    // And restore the previous state.
                    restoreState();
                    return;
                
                /// 0x49 ABC_ACTION_CONSTRUCTSUPER
                /// Stream: V32 'arg_count'
                /// Stack In:
                ///  argN ... arg1 -- the arg_count arguments
                ///  obj -- the object whose super's constructor should be
                ///         invoked
                /// Stack Out:
                ///  .
                case SWF::ABC_ACTION_CONSTRUCTSUPER:
                {
                    boost::uint32_t argc = mStream->read_V32();
                    fn_call::Args args;
                    get_args(argc, args);
                    
                    as_object* obj = _stack.top(argc).to_object(*_global);

                    // Is get_prototype what we want?
                    as_object* super = obj ? obj->get_prototype() : 0;
                    log_abc("CONSTRUCTSUPER: object %s, super %s, args %s",
                            _stack.top(argc), super, argc);

                    if (!super) {
                        log_error("ABC_ACTION_CONSTRUCTSUPER: No super found");
                        throw ASException();
                    }

                    as_value c = getMember(*super, NSV::PROP_CONSTRUCTOR);
                    pushCall(c.to_function(), super, mIgnoreReturn,
                            argc, -1);
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
                    print_stack();
                    as_environment env = as_environment(_vm);
                    MultiName a = pool_name(mStream->read_V32(), mPoolObject);
                    
                    boost::uint32_t argc = mStream->read_V32();
                    fn_call::Args args;
                    get_args(argc, args);
                    
                    log_abc("CONSTRUCT_PROP: will try to construct property "
                            "%s on object %s", mST.value(a.getGlobalName()),
                            _stack.top(0));

                    as_object* object = pop_stack().to_object(*_global);

                    if (!object) {
                        //TODO: Should this result in an exeception or an 
                        // actionscript error?
                        log_abc("Can't construct property on a null object. "
                                "Property not constructed.");
                        push_stack(as_value());
                        break;
                    }
                    
                    string_table::key ns = a.getNamespace() ?
                        a.getNamespace()->getURI() : 0;

                    as_value c = getMember(*object, 
                            ObjectURI(a.getGlobalName(), ns));

                    // TODO: don't do this. Scriptes should not be functions;
                    // we should always use the constructor member, most
                    // likely.
                    as_function* ctor = c.to_function();
                    
                    if (ctor) {
                        as_object* newobj = constructInstance(*ctor, env, args);
                        push_stack(as_value(newobj));
                    }

                    // TODO: This is more or less how everything should be done.
                    else {
                        log_abc("The property we found (%s) is not a "
                                "constructor", c);

                        if (c.is_null() || c.is_undefined()) {

                            log_abc("Constructor is undefined, will not "
                                    "construct property.");
                            push_stack(as_value());
                        }
                        else {
                            as_value val = c.to_object(*_global)->getMember(
                                    NSV::PROP_CONSTRUCTOR);

                            invoke(val, env, c.to_object(*_global), args);

                            // Push the constructed property
                            push_stack(c);
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
                ///  obj -- A new object which contains all of the given
                ///  properties.
                /// NB: This builds an object from its properties, it's not
                ///  a constructor.
                case SWF::ABC_ACTION_NEWOBJECT:
                {
                    as_object *obj = _global->createObject();
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
                    boost::uint32_t i = asize;

                    as_object* arr = _global->createArray();
                    while (i--) {
                        callMethod(arr, NSV::PROP_PUSH, pop_stack());
                    }
                    push_stack(as_value(arr));
                    break;
                }

                /// 0x57 ABC_ACTION_NEWACTIVATION
                /// Stack Out:
                ///  vtable -- A new virtual table, which has the
                ///  previous one as a parent.
                case SWF::ABC_ACTION_NEWACTIVATION:
                {
                    // TODO:  The function contains traits that need to be
                    // included in the activation object.
                    // For now we are using the function object as the
                    // activation object.  There is probably
                    // a better way.
                    //
                    if (!mCurrentFunction->needsActivation()) {
                        log_abc("NEWACTIVATION: called for a function without"
                                "the NEED_ACTIVATION flag");
                    }
                    push_stack(as_value(mCurrentFunction));
                    break;
                }

                /// 0x58 ABC_ACTION_NEWCLASS
                /// Stream: V32 'class_id'
                /// Stack In:
                ///  obj -- An object to be turned into a class. Its super
                ///     is constructed.
                /// Stack Out:
                ///  class -- The newly made class, made from obj and the
                ///     information at cinits_pool[class_id]
                /// NB: This depends on scope and scope base (to determine
                ///     lifetime(?))
                case SWF::ABC_ACTION_NEWCLASS:
                {
                    boost::uint32_t cid = mStream->read_V32();
                    log_abc("Class index: %s", cid);
                    Class* c = pool_script(cid, mPoolObject);
                    log_abc("Creating new class id=%u name=%s", c->getName(),
                            mST.value(c->getName()));
                    
                    // This may be 0, and that's fine.
                    as_object* base_class = pop_stack().to_object(*_global);
                    as_object* new_class = c->getPrototype();
                    
                    new_class->set_prototype(base_class);
                    
                    //Create the class.
                    Method* scmethod = c->getStaticConstructor();
                    // What if there isn't one?
                    assert(scmethod);
                    
                    as_function* ctor = c->getConstructor()->getPrototype();
                    new_class->init_member(NSV::PROP_CONSTRUCTOR, ctor, 0);

                    push_stack(new_class);

                    // Call the class's static constructor (which may be
                    // undefined).
                    as_environment env = as_environment(_vm);

                    /// This can be null.
                    as_function* staticCtor = scmethod->getPrototype();
                    
                    // We don't care about the return.
                    fn_call::Args args;
                    invoke(staticCtor, env, new_class, args);

                    log_abc("NEWCLASS(%1%) finished.",
                            mST.value(c->getName()));

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
                    MultiName a = pool_name(mStream->read_V32(), mPoolObject);
                    //as_value &v = _stack.top(0);
                    ENSURE_OBJECT(v);
                    _stack.drop(1);
                    _stack.drop(completeName(a));
                    // TODO: Decide or discover what to do with this.
                    LOG_ONCE( log_unimpl("ABC_ACTION_GETDESCENDANTS") );
                    break;
                }
            /// 0x5A ABC_ACTION_NEWCATCH
            /// Stream: V32 'catch_id'
            /// Stack Out:
            ///  vtable -- vtable suitable to catch an exception of type in
            ///         catch_id.
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
                ///  owner -- object which owns property given by looking
                ///  up the name_id.
                ///  0x5D is the undefined object if not found
                ///  0x5E throws a ReferenceError if not found
                case SWF::ABC_ACTION_FINDPROPSTRICT:
                case SWF::ABC_ACTION_FINDPROPERTY:
                {
                    MultiName a = pool_name(mStream->read_V32(), mPoolObject);
                    if (a.isRuntime()) {
                        _stack.drop(completeName(a));
                    }

                    as_value ret = find_prop_strict(a);


            /*		_stack.drop(completeName(a));
                    as_object *owner;
                    Property *b = mCurrentScope->findProperty(a.getABCName(), 
                        a.getNamespace()->getURI(), &owner);
                    if (!b)
                    {
                        if (opcode == SWF::ABC_ACTION_FINDPROPSTRICT)
                            throw ASReferenceError();
                        else
                            _stack.push(as_value());
                    }
                    else
                    {
                        _stack.push(owner);
                    }*/
                    break;
                }
            /// 0x5F ABC_ACTION_FINDDEF
            /// Stream: V32 'name_id' (no ns expansion)
            /// Stack Out:
            ///  def -- The definition of the name at name_id.
                case SWF::ABC_ACTION_FINDDEF:
                {
                    MultiName a = pool_name(mStream->read_V32(), mPoolObject);
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
                    MultiName a = pool_name(mStream->read_V32(), mPoolObject);
                
                    as_value val = find_prop_strict(a);

                    log_abc("GETLEX: found value %s", val);
                    _stack.top(0) = val;

                    break;
                }
                ///  ABC_ACTION_SETPROPERTY
                /// Stream: V32 'name_id'
                /// Stack In:
                ///  value -- The value to be used
                ///  [ns [n]] -- Namespace stuff
                ///      OR
                ///  [key] -- Key name for property. Will not have both
                ///          Namespace and key.
                ///  obj -- The object whose property is to be set
                /// Stack Out:
                ///  .
                /// NB: If the name at name_id is completely qualified,
                ///     neither a namespace nor a key is needed.  If the
                ///     name_id refers to a name with a runtime
                ///     namespace, then this will be used.  If neither of
                ///     those is true and obj is a dictionary and key is
                ///     a name, then the name_id is discarded and key/value
                ///     is set in the dictionary obj instead.
                case SWF::ABC_ACTION_SETPROPERTY:
                {
                    as_value value = pop_stack();
                    string_table::key ns = 0;
                    string_table::key name = 0;

                    MultiName a = pool_name(mStream->read_V32(), mPoolObject);
                    // TODO: If multiname is runtime we need to also pop
                    // namespace and name values off the stack.
                    if (a.flags() == MultiName::KIND_MultinameL) {
                        log_abc("SETPROPERTY: name is a late runtime "
                                "multiname");
                        as_value nameValue = pop_stack();
                        name = mST.find(nameValue.to_string());
                    }
                    else name = a.getGlobalName();

                    as_value val = pop_stack();
                    as_object *object = val.to_object(*_global);

                    if (!object) {
                        log_error("ABC_ACTION_SETPROPERTY: expecting object "
                                "on stack, got %s!", val);
                        break;
                    }

                    object->set_member(ObjectURI(name, ns), value, false);
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
                    push_stack(getRegister(index));
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
                    log_abc("Register index: %u",index);
                    setRegister(index, pop_stack());
                    break;
                }

                /// 0x64 ABC_ACTION_GETGLOBALSCOPE
                /// Stack Out:
                ///  global -- The global scope object
                case SWF::ABC_ACTION_GETGLOBALSCOPE:
                {
                    push_stack(_scopeStack.value(0));
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
                ///  [key] -- Key name for property. Will not have both
                ///           Namespace and key.
                ///  obj -- The object whose property is to be retrieved
                /// Stack Out:
                ///  prop -- The requested property.
                /// NB: See 0x61 (ABC_ACTION_SETPROPETY) for the decision of
                ///     ns/key.
                case SWF::ABC_ACTION_GETPROPERTY:
                {
                    string_table::key ns = 0;
                    string_table::key name = 0;
                    MultiName a = pool_name(mStream->read_V32(), mPoolObject);
                    // TODO: If multiname is runtime we need to also pop
                    // namespace and name values of the stack.
                    if (a.flags() == MultiName::KIND_MultinameL) {
                        as_value nameValue = pop_stack();
                        name = mST.find(nameValue.to_string());
                    }
                    else name = a.getGlobalName();

                    as_value object_val = pop_stack();
                    as_object* object = object_val.to_object(*_global);
                    
                    log_abc(_("GETPROPERTY: Looking for property "
                            "%s of object %s"), mST.value(name), object_val);

                    if (!object) {
                        log_abc(_("GETPROPERTY: expecting object on "
                                    "stack, got %s."), object_val);
                        push_stack(as_value());
                        break;
                    }
                    
                    as_value prop;
                    
                    const ObjectURI uri(name, ns);
                    const bool found = object->get_member(uri, &prop);
                    if (!found) {
                        log_abc("GETPROPERTY: property %s not found",
                                mST.value(name));
                    }
                    else {
                        log_abc("GETPROPERTY: property %s is %s",
                                mST.value(name), prop);
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
                ///  Set obj::(resolve)'name_id' to value, set bindings
                /// from the context.
                case SWF::ABC_ACTION_INITPROPERTY:
                {
                    boost::uint32_t index = mStream->read_V32();
                    MultiName a = pool_name(index, mPoolObject);
                    as_value v = pop_stack();
                    // TODO: If multiname is a runtime mutiname we need to also
                    // pop name and namespace values.
                    as_value object_val = pop_stack();

                    as_object* object = object_val.to_object(*_global);
                    if (!object) {
                        log_abc("INITPROPERTY: expecting object on stack, "
                                "got %s", object_val);
                    }
                    else {
                        object->set_member(a.getGlobalName(), v, false);
                    }
                    break;
                }

                /// 0x6A ABC_ACTION_DELETEPROPERTY
                /// Stream: V32 'name_id'
                /// Stack In:
                ///  [ns [n]] -- Namespace stuff
                ///  obj -- The object whose property should be deleted.
                /// Stack Out:
                ///  truth -- True if property was deleted or did not exist,
                ///           else False.
                case SWF::ABC_ACTION_DELETEPROPERTY:
                {
                    MultiName a = pool_name(mStream->read_V32(), mPoolObject);
                    _stack.drop(completeName(a));
                    as_object* obj = _stack.top(0).to_object(*_global);

                    if (!obj) {
                        // TODO: what here?
                        log_abc("DELETEPROPERTY: expecting object on stack, "
                                "got %s", _stack.top(0));
                        break;
                    }

                    // Look in the global namespace if there is none specified.
                    Namespace* n = a.getNamespace();
                    const string_table::key ns = n ? n->getURI() : 0;
                    const string_table::key prop = a.getGlobalName();

                    const bool deleted = obj->delProperty(
                            ObjectURI(prop, ns)).second;
                    _stack.top(0) = deleted;
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
                    as_object* object = pop_stack().to_object(*_global);
                    if (!object) {
                        log_abc("GETSLOT: Did not find expected object on "
                                "stack");
                        break;
                    } 

                    object->get_member_slot(sindex + 1, &val);

                    log_abc("object has value %s at real_slot=%u abc_slot=%u",
                            val, sindex + 1, sindex);
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
                //
                /// Slot index must be greater than 0 and less than or
                /// equal to the number of slots (so one-based index?).
                case SWF::ABC_ACTION_SETSLOT:
                {
                    boost::uint32_t sindex = mStream->read_V32();
                    as_value value = pop_stack();
                    as_value object = pop_stack();
                    log_abc("SETSLOT object: %s, value: %s, index: %s",
                            object, value, sindex);

                    as_object* obj = object.to_object(*_global);
                    if ( ! obj )
                    {
                        IF_VERBOSE_ASCODING_ERRORS(
                        log_aserror(_("ABC_ACTION_SETSLOT: "
                            "unexpected non-object stack value %s"), object);
                        );
                        break;
                    }

                    // We use sindex + 1, because currently as_object sets
                    // a property at a slot index 1 higher than the
                    // index the AbcBlock thinks the property is at.
                    if ( ! obj->set_member_slot(sindex+1, value) )
                    {
                        log_abc("Failed to set property at "
                                "real_slot=%u abc_slot=%u", sindex+1, sindex);
                    }
                    else
                    {
                        log_abc("Set property at real_slot=%u abc_slot=%u",
                                sindex+1, sindex);
                    }

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
                    _stack.grow(1);
                    //TODO: _stack.top(0) = mGlobal.getSlot(sindex);
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
                    //TODO: mGlobal.setSlot(sindex, _stack.pop());
                    break;
                }

                /// 0x70 ABC_ACTION_CONVERT_S
                /// Stack In:
                ///  value -- An object
                /// Stack Out:
                ///  str_value -- value as a string
                case SWF::ABC_ACTION_CONVERT_S:
                    _stack.top(0) = _stack.top(0).to_string();
                    break;

                /// 0x71 ABC_ACTION_ESC_XELEM
                /// Stack In:
                ///  value -- An object to be escaped
                /// Stack Out:
                ///  str_value -- value as a string, escaped suitably for
                ///         an XML element.
                case SWF::ABC_ACTION_ESC_XELEM:
                    log_unimpl("ABC_ACTION_ESC_XELEM");
                    //TODO: set _stack.top(0) to an escaped string.
                    break;

                /// 0x72 ABC_ACTION_ESC_XATTR
                /// Stack In:
                ///  value -- An object to be escaped
                /// Stack Out:
                ///  str_value -- value as a string, escaped suitably for an
                ///     XML attribute.
                case SWF::ABC_ACTION_ESC_XATTR:
                    log_unimpl("ABC_ACTION_ESC_XATTR");
                    //TODO: set _stack.top(0) to an escaped string.
                    break;

                /// 0x73 ABC_ACTION_CONVERT_I
                /// 0x83 ABC_ACTION_COERCE_I (deprecated)
                /// Stack In:
                ///  value -- An object to be converted to Integer
                /// Stack Out:
                ///  int_value -- value as an integer object
                case SWF::ABC_ACTION_CONVERT_I:
                case SWF::ABC_ACTION_COERCE_I:
                    _stack.top(0) = toInt(_stack.top(0));
                    break;

                /// 0x74 ABC_ACTION_CONVERT_U
                /// 0x88 ABC_ACTION_COERCE_U (deprecated)
                /// Stack In:
                ///  value -- An object to be converted to unsigned integer
                /// Stack Out:
                ///  int_value -- value as an unsigned integer object
                case SWF::ABC_ACTION_CONVERT_U:
                case SWF::ABC_ACTION_COERCE_U:
                    _stack.top(0) = static_cast<boost::uint32_t>(
                            toNumber(_stack.top(0), getVM(fn)));
                    break;

                /// 0x75 ABC_ACTION_CONVERT_D
                /// 0x84 ABC_ACTION_COERCE_D (deprecated)
                /// Stack In:
                ///  value -- An object to be converted to a double
                /// Stack Out:
                ///  double_value -- value as a double object
                case SWF::ABC_ACTION_CONVERT_D:
                case SWF::ABC_ACTION_COERCE_D:
                    _stack.top(0) = toNumber(_stack.top(0), getVM(fn));
                    break;

                /// 0x76 ABC_ACTION_CONVERT_B
                /// 0x81 ABC_ACTION_COERCE_B (deprecated)
                /// Stack In:
                ///  value -- An object to be converted to a boolean
                /// Stack Out:
                ///  bool_value -- value as a boolean object
                case SWF::ABC_ACTION_CONVERT_B:
                case SWF::ABC_ACTION_COERCE_B:
                    _stack.top(0) = _stack.top(0).to_bool();
                    break;

                /// 0x77 ABC_ACTION_CONVERT_O
                /// Stack In:
                ///  obj -- An object
                /// Stack Out:
                ///  obj -- An object
                /// Do: If obj is Undefined or Null, throw TypeError
                case SWF::ABC_ACTION_CONVERT_O:
                {
                    _stack.top(0) = _stack.top(0).to_object(*_global);
                    if (_stack.top(0).is_undefined() || _stack.top(0).is_null())
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
                    if (!_stack.top(0).is_object()) { 
                        // TODO: check whether it is an XML object.
                        throw ASTypeError();
                    }
                    break;
                }

                /// 0x80 ABC_ACTION_COERCE
                /// Stream: V32 'name_index'
                /// Stack In:
                ///  [ns [n]] -- Possibly name/namespace stuff
                ///  obj -- An object to be converted
                /// Stack Out:
                ///  coerced_obj -- The object as the desired (resolve)
                //      'name_index' type.
                case SWF::ABC_ACTION_COERCE:
                {
                    // TODO: handle runtime names?
                    MultiName a = pool_name(mStream->read_V32(), mPoolObject);

                    as_value value = _stack.top(0);
                    log_abc("COERCE: object for conversion is %s, "
                            "desired type %s", value,
                            mST.value(a.getGlobalName()));

                    // Examples of desired type: "Sprite" "Button",
                    // "GlobalListener", "Object".
                    // Tamarin seems to look up the traits of the
                    // target type. If it's a builtin type (boolean, number,
                    // string, in, uint, object, "any") it succeeds. 
                    // Otherwise check null or undefined and do something.
                    // Otherwise look at the type traits of the original. If
                    // these traits contain the expected interface, return the
                    // original value. Otherwise throw error.
                    break;
                }
                /// 0x82 ABC_ACTION_COERCE_A
                /// Stack In:
                ///  obj -- An object to be converted
                /// Stack Out:
                ///  obj
                /// Do: Nothing. (The 'a' is for atom, and it's unclear
                /// if anything is needed.)
                case SWF::ABC_ACTION_COERCE_A:
                {
                    break;
                }

                /// 0x85 ABC_ACTION_COERCE_S
                /// Stack In:
                ///  obj -- An object to be converted
                /// Stack Out:
                ///  str_obj -- obj as string. nullString object if obj is
                ///  Null or Undefined
                case SWF::ABC_ACTION_COERCE_S:
                {
                    if (_stack.top(0).is_undefined() ||
                            _stack.top(0).is_null()) {
                        _stack.top(0) = "";
                    }
                    else _stack.top(0) = _stack.top(0).to_string();
                    break;
                }

                /// 0x86 ABC_ACTION_ASTYPE
                /// Stream: V32 'name_index'
                /// Stack In:
                ///  [ns [n]] -- Possible namespace stuff
                ///  obj -- An object to be checked
                /// Stack Out:
                ///  cobj -- obj if obj is of type (resolve)'name_index',
                ///     otherwise Null
                case SWF::ABC_ACTION_ASTYPE:
                {
                    MultiName a = pool_name(mStream->read_V32(), mPoolObject);
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
                ///  cobj -- obj if type of obj conforms to valid, otherwise
                ///     Null
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
                    if (_stack.top(0).is_undefined())
                        _stack.top(0) = _stack.top(0).to_object(*_global);
                    else
                        _stack.top(0).set_undefined();
                    break;
                }
            /// 0x90 ABC_ACTION_NEGATE
            /// Stack In:
            ///  obj -- An object
            /// Stack Out:
            ///  negdouble -- -1.0 * (double) obj
                case SWF::ABC_ACTION_NEGATE:
                {
                    _stack.top(0) = -toNumber(_stack.top(0), getVM(fn));
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
                    setRegister(foff, toNumber(getRegister(foff), getVM(fn)) + 1);
                    break;
                }

                /// 0x93 ABC_ACTION_DECREMENT
                /// Stack In:
                ///  num -- A number, integer or double
                /// Stack Out:
                ///  num - 1
                case SWF::ABC_ACTION_DECREMENT:
                {
                    _stack.top(0) = toNumber(_stack.top(0), getVM(fn)) - 1;
                    break;
                }

                /// 0x94 ABC_ACTION_DECLOCAL
                /// Stream: V32 'frame_addr'
                /// Frame: Load i from frame_addr and decrement it.
                case SWF::ABC_ACTION_DECLOCAL:
                {
                    const boost::uint32_t foff = mStream->read_V32();
                    setRegister(foff, toNumber(getRegister(foff), getVM(fn)) - 1);
                    break;
                }

                /// 0x95 ABC_ACTION_ABC_TYPEOF
                /// Stack In:
                ///  obj -- An object
                /// Stack Out:
                ///  type -- typeof(obj) as a string
                case SWF::ABC_ACTION_ABC_TYPEOF:
                    _stack.top(0) = _stack.top(0).typeOf();
                    break;

                /// 0x96 ABC_ACTION_NOT
                /// Stack In:
                ///  obj -- An object
                /// Stack Out:
                ///  nobj -- A truth object with value !((Boolean) obj)
                case SWF::ABC_ACTION_NOT:
                    _stack.top(0).set_bool(!_stack.top(0).to_bool());
                    break;

                /// 0x97 ABC_ACTION_BITNOT
                /// Stack In:
                ///  obj -- An object
                /// Stack Out:
                ///  nint -- ~((Int) obj)
                case SWF::ABC_ACTION_BITNOT:
                    _stack.top(0) = ~toInt(_stack.top(0));
                    break;

                /// 0xA0 ABC_ACTION_ADD	
                /// Stack In:
                /// a
                /// b
                /// Stack Out:
                /// a + b (double if numeric)
                case SWF::ABC_ACTION_ADD:
                    newAdd(_stack.top(1), _stack.top(0), _vm);
                    _stack.drop(1);
                    break;
                
                /// 0xA1 ABC_ACTION_SUBTRACT
                /// Stack In:
                ///  b
                ///  a
                /// Stack Out:
                ///  a - b (double)
                case SWF::ABC_ACTION_SUBTRACT:
                    subtract(_stack.top(1), _stack.top(0), _vm);
                    _stack.drop(1);
                    break;

                /// 0xA2 ABC_ACTION_MULTIPLY
                /// Stack In:
                ///  a
                ///  b
                /// Stack Out:
                ///  a * b (double)
                case SWF::ABC_ACTION_MULTIPLY:
                    _stack.top(1) = toNumber(_stack.top(1), getVM(fn)) * toNumber(_stack.top(0), getVM(fn));
                    _stack.drop(1);
                    break;

                /// 0xA3 ABC_ACTION_DIVIDE
                /// Stack In:
                ///  b
                ///  a
                /// Stack Out:
                ///  a / b (double)
                case SWF::ABC_ACTION_DIVIDE:
                    _stack.top(1) = toNumber(_stack.top(1), getVM(fn)) / toNumber(_stack.top(0), getVM(fn));
                    _stack.drop(1);
                    break;

                /// 0xA4 ABC_ACTION_MODULO
                /// Stack In:
                ///  b
                ///  a
                /// Stack Out:
                ///  a % b (not integer mod, but remainder)
                case SWF::ABC_ACTION_MODULO:
                {
                    // TODO: test this properly and fix the UB (overflow).
                    double result = toNumber(_stack.top(1), getVM(fn)) / toNumber(_stack.top(0), getVM(fn));
                    int trunc_result = static_cast<int> (result);
                    _stack.top(1) = toNumber(_stack.top(1), getVM(fn)) - 
                        (trunc_result * toNumber(_stack.top(0), getVM(fn)));
                    _stack.drop(1);
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
                    _stack.top(1) = toInt(_stack.top(1)) << toInt(_stack.top(0));
                    _stack.drop(1);
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
                    _stack.top(1) = toInt(_stack.top(1)) >> toInt(_stack.top(0));
                    _stack.drop(1);
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
                    _stack.top(1) =
                        static_cast<boost::uint32_t>(toNumber(_stack.top(1), getVM(fn)))
                        >> toInt(_stack.top(0));
                    _stack.drop(1);
                    break;
                }

                /// 0xA8 ABC_ACTION_BITAND
                ///  a
                ///  b
                /// Stack Out:
                ///  a & b
                case SWF::ABC_ACTION_BITAND:
                    _stack.top(1) = toInt(_stack.top(1)) & toInt(_stack.top(0));
                    _stack.drop(1);
                    break;

                /// 0xA9 ABC_ACTION_BITOR
                /// Stack In:
                ///  b
                ///  a
                /// Stack Out:
                ///  a | b
                case SWF::ABC_ACTION_BITOR:
                    _stack.top(1) = toInt(_stack.top(1)) | toInt(_stack.top(0));
                    _stack.drop(1);
                    break;

                /// 0xAA ABC_ACTION_BITXOR
                /// Stack In:
                ///  b
                ///  a
                /// Stack Out:
                ///  a ^ b
                case SWF::ABC_ACTION_BITXOR:
                {
                    _stack.top(1) = toInt(_stack.top(1)) ^ toInt(_stack.top(0));
                    _stack.drop(1);
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
                    bool truth = abstractEquality(_stack.top(1), _stack.top(0), false);
                    _stack.drop(1);
                    _stack.top(0).set_bool(truth);
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
                    bool truth = abstractEquality(_stack.top(1), _stack.top(0), true);
                    _stack.drop(1);
                    _stack.top(0).set_bool(truth);
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
                    ABSTRACT_COMPARE(truth, _stack.top(1), _stack.top(0), false);
                    _stack.drop(1);
                    _stack.top(0).set_bool(truth); // truth is a < b
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
                    ABSTRACT_COMPARE(truth, _stack.top(0), _stack.top(1), true);
                    _stack.drop(1);
                    _stack.top(0).set_bool(!truth); // truth is b < a
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
                    ABSTRACT_COMPARE(truth, _stack.top(0), _stack.top(1), false);
                    _stack.drop(1);
                    _stack.top(0).set_bool(truth); // truth is b < a
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
                    ABSTRACT_COMPARE(truth, _stack.top(1), _stack.top(0), true);
                    _stack.drop(1);
                    _stack.top(0).set_bool(!truth); // truth is a < b
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
                    ABSTRACT_TYPELATE(truth, _stack.top(1), _stack.top(0));
                    _stack.top(1).set_bool(truth);
                    _stack.drop(1);
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
                    MultiName a = pool_name(mStream->read_V32(), mPoolObject);
                    _stack.drop(completeName(a));
                    // TODO: Implement it.
                    //_stack.top(0).set_bool(_stack.top(0).conforms_to(a.getABCName()));
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
                    as_object* const valueObject = value.to_object(*_global);
                    as_object* const typeObject = type.to_object(*_global);

                    if (!valueObject || !typeObject) {
                        // TODO: what here!?
                        // This should eventually be a malformed SWF error.
                        log_error("ACTION_ISTYPELATE: require two objects "
                                "on stack, got obj: %s, type: %s.",
                                value, type);
                        break;
                    }
                    const bool truth = valueObject->instanceOf(typeObject);
                    push_stack(truth);
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
                    log_unimpl("ABC_ACTION_IN");
                    //TODO: _stack.top(1).set_bool(_stack.top(1).to_object(*_global).contains(_stack.top(0)));
                    _stack.drop(1);
                    break;
                }

                /// 0xC0 ABC_ACTION_INCREMENT_I
                /// See: 0x91 (ABC_ACTION_INCREMENT), but forces types to int, not double
                case SWF::ABC_ACTION_INCREMENT_I:
                {
                    _stack.top(0) = toInt(_stack.top(0)) + 1;
                    break;
                }

                /// 0xC1 ABC_ACTION_DECREMENT_I
                /// See: 0x93 (ABC_ACTION_DECREMENT), but forces types to int, not double
                case SWF::ABC_ACTION_DECREMENT_I:
                {
                    _stack.top(0) = toInt(_stack.top(0)) - 1;
                    break;
                }

                /// 0xC2 ABC_ACTION_INCLOCAL_I
                /// See: 0x92 (ABC_ACTION_INCLOCAL), but forces types to int
                /// not double
                case SWF::ABC_ACTION_INCLOCAL_I:
                {
                    const boost::uint32_t foff = mStream->read_V32();
                    setRegister(foff,  toInt(getRegister(foff)) + 1);
                    break;
                }

                /// 0xC3 ABC_ACTION_DECLOCAL_I
                /// See: 0x94 (ABC_ACTION_DECLOCAL), but forces types to int,
                /// not double
                case SWF::ABC_ACTION_DECLOCAL_I:
                {
                    const boost::uint32_t foff = mStream->read_V32();
                    setRegister(foff, toInt(getRegister(foff)) - 1);
                    break;
                }

                /// 0xC4 ABC_ACTION_NEGATE_I
                /// See: 0x90 (ABC_ACTION_NEGATE), but forces type to int,
                /// not double
                case SWF::ABC_ACTION_NEGATE_I:
                {
                    _stack.top(0) = - toInt(_stack.top(0));
                    break;
                }

                /// 0xC5 ABC_ACTION_ADD_I
                /// See: 0xA0 (ABC_ACTION_ADD), but forces type to int
                case SWF::ABC_ACTION_ADD_I:
                {
                    _stack.top(1) = toInt(_stack.top(1)) +
                        toInt(_stack.top(0));
                    _stack.drop(1);
                    break;
                }

                /// 0xC6 ABC_ACTION_SUBTRACT_I
                /// See: 0xA1 (ABC_ACTION_SUBTRACT), but forces type to int
                case SWF::ABC_ACTION_SUBTRACT_I:
                {
                    _stack.top(1) = toInt(_stack.top(1)) -
                        toInt(_stack.top(0));
                    _stack.drop(1);
                    break;
                }

                /// 0xC7 ABC_ACTION_MULTIPLY_I
                /// See: 0xA2 (ABC_ACTION_MULTIPLY), but forces type to int
                case SWF::ABC_ACTION_MULTIPLY_I:
                {
                    _stack.top(1) = toInt(_stack.top(0)) * toInt(_stack.top(1));
                    _stack.drop(1);
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
            //		_stack.grow(1);
            //		_stack.push() instead?

                    push_stack(getRegister(opcode- SWF::ABC_ACTION_GETLOCAL0));
            //		_stack.top(0) = _registers.value(opcode - SWF::ABC_ACTION_GETLOCAL0);
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
                    setRegister(opcode - SWF::ABC_ACTION_SETLOCAL0,
                            pop_stack());
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
                /// Do: Enter debugger if present, line_number is the
                /// line number in source.
                case SWF::ABC_ACTION_BKPTLINE:
                    mStream->skip_V32();
                    break;
            }

            log_abc("* DONE *");
            IF_VERBOSE_ACTION(print_stack());
        } 
        catch (const StackException& s) {
            log_error("Threw a stack exception during AVM2 execution.");
        }
        catch (ASException& s) {
            // TODO: The exception should be pushed back into AS execution
            // somehow.
            log_unimpl("Caught an AS exception from AVM2 execution.");
        }

	} 
} 

void
Machine::getMember(Class* pDefinition, MultiName& name,
	as_value& instance)
{
	if (!instance.is_object())
		throw ASTypeError();
#if 0
	if (!pBinding->isGetSet())
	{
		//TODO: _stack.push(pBinding->getFromInstance(instance));
		return;
	}

	// This is a getter, so we need to execute it. Even those
	// written in C++ get called like this, with pushCall handling.
	// And push the instance ('this')
	_stack.push(instance);
	pushCall(1, &_stack.top(0), pBinding); //TODO: pBinding->getGetter());
#else
UNUSED(pDefinition);
UNUSED(name);
#endif
}

void
Machine::setMember(Class *pDefinition, MultiName& name, as_value& instance,
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
	_stack.push(instance);
	_stack.push(newvalue);
	pushCall(2, &_stack.top(1), pBinding); //TODO: pBinding->getSetter());
#else
UNUSED(pDefinition);
UNUSED(name);
UNUSED(newvalue);
#endif
}

int
Machine::completeName(MultiName& name, int offset)
{
    
    // TODO: implement this properly.
    // Should this really be called when there's nothing on the stack?
    if (_stack.empty()) return 0;

	int size = 0;

	if (name.isRuntime())
	{
		as_value obj = _stack.top(offset);

		if (name.isRtns()) {
			++size; // Ignore the Namespace.
        }
	}
	else if (name.isRtns())
	{
		//TODO: This should be a namespace //name.setNamespace(_stack.top(offset));
		++size;
	}
	return size;
}

Class*
Machine::findSuper(as_value &v, bool find_for_primitive)
{
	if (v.is_undefined() || v.is_null()) return NULL;

	if (v.is_object()) {
		Class *pProto = NULL; // TODO: v.to_object(*_global)->getScript();
		return pProto ? pProto->getSuper() : NULL;
	}

	if (!find_for_primitive) return 0;

	if (v.is_number()) {
		return NULL; // TODO: _classes->getScript(NSV::CLASS_NUMBER);
	}

	// And so on...
	// TODO: Other primitives
	return 0;
}

void
Machine::immediateFunction(const as_function* func, as_object* thisptr,
        as_value& storage, unsigned char stack_in, short stack_out)
{
    assert(func);

	// TODO: Set up the fn to use the stack
    fn_call::Args args;
    size_t st = 0;
    while (st < stack_in) {
        args += _stack.top(st);
        ++st;
    }

	fn_call fn(thisptr, as_environment(_vm), args);
    _stack.drop(stack_in - stack_out);
	saveState();
    _stack.grow(stack_in - stack_out);
    _stack.setDownstop(stack_in);
	mThis = thisptr;
	storage = const_cast<as_function*>(func)->call(fn);
	restoreState();
}

void
Machine::pushGet(as_object *this_obj, as_value &return_slot, Property *prop)
{
	if (!prop) return;

	if (prop->isGetterSetter()) {
		//TODO pushCall(prop->getGetter(), this_obj, return_slot, 0);
		return;
	}

	return_slot = prop->getValue(*this_obj);
}

void
Machine::pushSet(as_object *this_obj, as_value &value, Property *prop)
{
	if (!prop) return;

	if (prop->isGetterSetter()) {
		_stack.push(value);
		//TODO pushCall(prop->getSetter(), this_obj, mIgnoreReturn, 1);
		return;
	}

	prop->setValue(*this_obj, value);
}

void
Machine::pushCall(as_function *func, as_object *pthis, as_value& return_slot,
	unsigned char stack_in, short stack_out)
{

    log_abc("Pushing function call for function %s", func);

	if (1 || func->isBuiltin())
	{
		immediateFunction(func, pthis, return_slot, stack_in, stack_out);
		return;
	}
	// TODO: Make this work for stackless.

	// Here is where the SafeStack shines:
	// We set the stack the way it should be on return.
	_stack.drop(stack_in - stack_out);
	// We save that state.
	saveState();
	// Set the 'this' for the new call
	mThis = pthis;
	// Retrieve the stack. (It wasn't lost)
	_stack.grow(stack_in - stack_out);
	// And then we set the downstop
	_stack.setDownstop(stack_in);

	// When control goes to the main loop of the interpreter, it will
	// automatically start executing the method.
}

void
Machine::restoreState()
{
	log_abc("Restoring state.");
	State &s = mStateStack.top(0);
	s.to_debug_string();
//	_stack.setAllSizes(s._stackTotalSize, s._stackDepth);
	_scopeStack.setAllSizes(s.mScopeTotalSize, s._scopeStackDepth);
	mStream = s.mStream;
	_registers = s._registers;
	mCurrentFunction = s.mFunction;
//	mExitWithReturn = s.mReturn;
//	mDefaultXMLNamespace = s.mDefaultXMLNamespace;
//	mCurrentScope = s.mCurrentScope;
//	mGlobalReturn = s.mGlobalReturn;
//	mThis = s.mThis;
	mStateStack.pop();
}

void
Machine::saveState()
{
	log_abc("Saving state.");
	mStateStack.grow(1);
	State &s = mStateStack.top(0);
	s._stackDepth = _stack.getDownstop();
	s._stackTotalSize = _stack.totalSize();
	s._scopeStackDepth = _scopeStack.getDownstop();
	s.mScopeTotalSize = _scopeStack.totalSize();
	s.mStream = mStream;
	s.to_debug_string();
	s._registers = _registers;
	s.mFunction = mCurrentFunction;
//	s.mReturn = mExitWithReturn;
//	s.mDefaultXMLNamespace = mDefaultXMLNamespace;
//	s.mCurrentScope = mCurrentScope;
//	s.mGlobalReturn = mGlobalReturn;
//	s.mThis = mThis;
}

void
Machine::initMachine(AbcBlock* pool_block)
{
	mPoolObject = pool_block;
	log_debug("Getting entry script.");
	Class* start_script = pool_block->scripts().back();
	log_debug("Getting constructor.");
	Method* constructor = start_script->getConstructor();
	clearRegisters(constructor->getMaxRegisters());
	log_debug("Loading code stream.");
	mStream = constructor->getBody();
	mCurrentFunction = constructor->getPrototype();
	setRegister(0, _global);
}

//This is called by abc_functions to execute their code stream.
//TODO: There is probably a better way to do this, once we understand what the VM is supposed
//todo, this should be fixed.
as_value
Machine::executeFunction(Method* method, const fn_call& fn)
{
	
    //TODO: Figure out a good way to use the State object to handle
    //returning values.
	mCurrentFunction = method->getPrototype();
	bool prev_ext = mExitWithReturn;
	CodeStream *stream = method->getBody();
    
    // Protect the current stack from alteration
    // TODO: use saveState only, but not before checking other effects.
    size_t stackdepth = _stack.fixDownstop();
    size_t stacksize = _stack.totalSize();
    size_t scopedepth = _scopeStack.fixDownstop();
    size_t scopesize = _scopeStack.totalSize();
	
    saveState();
	mStream = stream;
	clearRegisters(method->getMaxRegisters());
	
    log_abc("Executing function: max registers %s, scope depth %s, "
            "max scope %s, max stack: %s", method->getMaxRegisters(),
            method->scopeDepth(), method->maxScope(), method->maxStack());
	mExitWithReturn = true;
	setRegister(0, fn.this_ptr);
	for (unsigned int i=0;i<fn.nargs;i++) {
		setRegister(i + 1, fn.arg(i));
	}
	
    execute();
	mExitWithReturn = prev_ext;
	
    _stack.setAllSizes(stacksize, stackdepth);
    _scopeStack.setAllSizes(scopesize, scopedepth);

	return mGlobalReturn;
}

void
Machine::executeCodeblock(CodeStream* stream)
{
	mStream = stream;
	execute();
}

void
Machine::markReachableResources() const
{
    _global->setReachable();
}

void
Machine::instantiateClass(std::string className, as_object* /*global*/)
{

    if (!mPoolObject) {
        log_debug("No ABC block! Can't instantiate class!");
        return;
    }

    log_debug("instantiateClass: class name %s", className);

	Class* cl = mPoolObject->locateClass(className);
    if (!cl)
    {
        /// This seems like a big error.
        log_error("Could not locate class '%s' for instantiation", className);
        return;
    }
	
    Method* ctor = cl->getConstructor();

    if (!ctor) {
        log_error("Class found has no constructor, can't instantiate "
                "class");
        return;
    }

	clearRegisters(ctor->getMaxRegisters());
	mCurrentFunction = ctor->getPrototype();

    // Protect the current stack from alteration
    // TODO: use saveState
    size_t stackdepth = _stack.fixDownstop();
    size_t stacksize = _stack.totalSize();
    size_t scopedepth = _scopeStack.fixDownstop();
    size_t scopesize = _scopeStack.totalSize();

    // The value at _registers[0] is generally pushed to the stack for
    // CONSTRUCTSUPER, which apparently expects the object whose super
    // is to be constructed. The pp's stack names the class for instantiation
    // at register 0 when the constructor body is executed, which must
    // correspond to the class prototype.
	setRegister(0, cl->getPrototype());
	executeCodeblock(ctor->getBody());
    log_debug("Finished instantiating class %s", className);

    _stack.setAllSizes(stacksize, stackdepth);
    _scopeStack.setAllSizes(scopesize, scopedepth);

}

as_value
Machine::find_prop_strict(MultiName multiname)
{
	
    log_abc("Looking for property %2% in namespace %1%",
            mST.value(multiname.getNamespace()->getURI()),
            mST.value(multiname.getGlobalName()));

    // We should not push anything onto the scope stack here; whatever is
    // needed should already be pushed. The pp will not call FINDPROP*
    // when the scope stack is empty.
    //
    // However, the complete scope stack, including elements that are
    // 'invisible' to this scope, is available
	as_value val;
    print_scope_stack();
    const string_table::key var = multiname.getGlobalName();
    const string_table::key ns = multiname.getNamespace()->getURI();

	for (size_t i = 0; i < _scopeStack.totalSize(); ++i)
    {
		as_object* scope_object = _scopeStack.at(i);
		if (!scope_object) {
			log_abc("Scope object is NULL.");
			continue;
		}
        
        if (scope_object->get_member(ObjectURI(var, ns), &val)) {
            push_stack(_scopeStack.at(i));
			return val;
		}
	}

    // TODO: find out what to do here.
    log_abc("Failed to find property in scope stack.");
	as_object* null = 0;
    push_stack(null);
    return val;
}

void
Machine::print_stack()
{

	std::stringstream ss;
	ss << "Stack: ";
	for (unsigned int i = 0; i < _stack.totalSize(); ++i) {
		if (i!=0) ss << " | ";
		ss << _stack.at(i);
	}
	log_abc("%s", ss.str());
}

void
Machine::print_scope_stack()
{

	std::stringstream ss;
	ss << "ScopeStack: ";

    size_t totalSize = _scopeStack.totalSize();

    for (unsigned int i = 0; i < totalSize; ++i) {
		ss << as_value(_scopeStack.at(i)).toDebugString();
	}
	log_abc("%s", ss.str());
}	

void
Machine::get_args(size_t argc, fn_call::Args& args)
{
    std::vector<as_value> v(argc);
	for (size_t i = argc; i > 0; --i) {
		v.at(i-1) = pop_stack();
	}
    args.swap(v);
}

void
Machine::clearRegisters(boost::uint32_t maxRegisters)
{
	_registers.clear();
	_registers.resize(maxRegisters);
}


} // namespace abc
} // namespace gnash
