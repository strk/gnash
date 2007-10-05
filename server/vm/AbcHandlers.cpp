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
//

void
ActionMachine::execute_as3()
{
	for ( ; ; )
	{
	switch ((opcode = mStream.read_as3op())) // Assignment intentional
	{
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
		asName a = mStream.read_V32();
		// Finish it, if necessary.
		mStack.drop(completeName(a));
		// Get the target object.
		as_value vobj& = mStack.top(0);
		asClass *pClass = findSuper(vobj, true);
		// If we don't have a class yet, throw.
		if (!pClass)
			throw ASReferenceError();
		// getMember will throw any necessary exceptions and do the push.
		getMember(pClass, a, vobj);
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
		asName a = mStream.read_V32();
		as_value vobj = mStack.pop(); // The value

		mStack.drop(completeName(a));

		as_value target = mStack.pop();
		asClass *pClass = findSuper(target, true);
		if (!pClass)
			throw ASReferenceError();

		setMember(pClass, a, target, vobj);
		break;
	}
/// 0x06 ABC_ACTION_DXNS
/// Default XML Namespace
/// Stream: UV32 index to string pool 'nsname'
/// Do: Create a new public namespace with name nsname, and make this the
///  default XML namespace. 
	case SWF::ABC_ACTION_DXNS:
	{
		uint32_t soffset = mStream.read_V32();
		std::string& uri = pool_string(soffset);
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
		uint32_t regNum = mStream.read_V32();
		mFrame.value(regnum).set_undefined();
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
		bool truth = mStack.top(1).less_than(mStack.top(0));
		mStack.drop(2);
		jumpIf(!truth);
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
		bool truth = mStack.top(1).less_equal(mStack.top(0));
		mStack.drop(2);
		jumpIf(!truth);
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
		bool truth = mStack.top(1).greater_than(mStack.top(0));
		mStack.drop(2);
		jumpIf(!truth);
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
		bool truth = mStack.top(1).greater_equal(mStack.top(0));
		mStack.drop(2);
		jumpIf(!truth);
		break;
	}
/// 0x10 ABC_ACTION_JUMP
/// Stream: S24 jump offset 'jump'
/// Do: If jump is negative, check for interrupts. Move by jump in stream.
/// Equivalent: ACTION_BRANCHALWAYS
	case SWF::ABC_ACTION_JUMP:
	{
		jumpIf(true);
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
		bool truth = mStack.top(0).to_bool();
		mStack.drop(1);
		jumpIf(truth);
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
		bool truth = mStack.top(0).to_bool();
		mStack.drop(1);
		jumpIf(!truth);
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
		bool truth = mStack.top(1).weak_equals(mStack.top(0));
		mStack.drop(2);
		jumpIf(truth);
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
		bool truth = mStack.top(1).weak_nequals(mStack.top(0));
		mStack.drop(2);
		jumpIf(truth);
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
		bool truth = mStack.top(1).less_than(mStack.top(0));
		mStack.drop(2);
		jumpIf(truth);
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
		bool truth = mStack.top(1).less_equal(mStack.top(0));
		mStack.drop(2);
		jumpIf(truth);
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
		bool truth = mStack.top(1).greater_than(mStack.top(0));
		mStack.drop(2);
		jumpIf(truth);
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
		bool truth = mStack.top(1).greater_equal(mStack.top(0));
		mStack.drop(2);
		jumpIf(truth);
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
		bool truth = mStack.top(1).strict_equals(mStack.top(0));
		mStack.drop(2);
		jumpIf(truth);
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
		bool truth = mStack.top(1).strict_nequals(mStack.top(0));
		mStack.drop(2);
		jumpIf(truth);
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
		AbcHandlers::stream_position npos = mStream.tell();
		uint32_t index = mStack.top(0).to_number<uint32_t>();
		mStack.drop(1);

		mStream.seekBy(3); // Skip the intial offset.
		uint32_t cases = mStream.read_V32();
		// Read from our original position and use it to skip if the case
		// is out of range.
		if (index > cases)
		{
			mStream.seekTo(npos);
			mStream.seekTo(npos + mStream.read_S24());
		}
		else
		{
			mStream.seekTo(npos + 3 * (index + 1));
			uint_32t newpos = mStream.read_S24();
			mStream.seekTo(npos - 1 + newpos);
		}
		break;
	}
/// 0x1C ABC_ACTION_PUSHWITH
/// Stack In:
///  scope -- a scope
/// Stack Out:
///  .
/// Do: Enter scope with previous scope as its base, unless it already had
///  a base, in which case leave that alone.
	case SWF::ABC_ACTION_PUSHWITH:
	{
		asScope = mStack.top(0).to_scope();
		mStack.drop(1);
		pushScope(asScope);
		// If there wasn't a base scope, then this becomes it.
		if (!mBaseScope)
			mBaseScope = mCurrentScope;
		break;
	}
/// 0x1D ABC_ACTION_POPSCOPE
/// Do: exit current scope. Clear the base if the depth is now 
///  shallower than the base's depth.
	case SWF::ABC_ACTION_POPSCOPE:
	{
		if (mCurrentScope == mBaseScope)
			mBaseScope = NULL;
		popScope();
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
		as_object *obj = mStack.top(1).to_object();
		uint32_t index = mStack.top(0).to_number<uint32_t>();
		mStack.drop(1);
		asBinding *b = obj->binding_at_index(index);
		if (next)
			mStack.top(0) = b->getNameString();
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
/// Do: If there is a key/val pair after index, make next_index as it.
///  Otherwise, make next_index 0.
	case SWF::ABC_ACTION_HASNEXT:
	{
		as_object *obj = mStack.top(1).to_object();
		uint32_t index = mStack.top(0).to_number<uint32_t>();
		mStack.drop(1);
		asBinding *next = obj->binding_after_index(index);
		if (next)
			mStack.top(0) = next->getDispatch();
		else
			mStack.top(0) = 0;
		break;
	}
/// 0x20 ABC_ACTION_PUSHNULL
/// Stack Out:
///  n -- a Null object.
	case SWF::ABC_ACTION_PUSHNULL:
	{
		mStack.grow(1);
		mStack.top(0).set_null();
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
		as_object *obj = mStack.top(1).to_object();
		uint32_t index = mStack.top(0).to_number<uint32_t>();
		mStack.drop(1);
		asBinding *b = obj->binding_at_index(index);
		if (next)
			mStack.top(0) = b->getValue();
		else
			mStack.top(0).set_undefined();
		break;
	}
/// 0x24 ABC_ACTION_PUSHBYTE
/// Stream: S8 as 'byte'
/// Stack Out:
///  byte -- as a raw byte
	case SWF::ABC_ACTION_PUSHBYTE:
	{
		int8_t *b = mStream.read_s8();
		mStack.grow(1);
		mStack.top(0) = b;
		break;
	}
/// 0x25 ABC_ACTION_PUSHSHORT
/// Stream: V32 as 'value'
/// Stack Out:
///  value -- as a raw integer
	case SWF::ABC_ACTION_PUSHSHORT:
	{
		signed short s = static_cast<signed short>(mStream.read_V32());
		mStack.grow(1);
		mStack.top(0) = s;
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
		mStack.grow(1);
		mStack.top(0).set_bool(false);
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
		mStack.drop(1);
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
		mStack.grow(1);
		mStack.top(0) = pool_string(mStream.read_V32());
		break;
	}
/// 0x2D ABC_ACTION_PUSHINT
/// Stream: V32 int pool index 'index'
/// Stack Out:
///  value -- Integer object from integer_pool[index]
	case SWF::ABC_ACTION_PUSHINT:
	{
		mStack.grow(1);
		mStack.top(0) = pool_int(mStream.read_V32());
		break;
	}
/// 0x2E ABC_ACTION_PUSHUINT
/// Stream: V32 uint pool index 'index'
/// Stack Out:
///  value -- Unsigned Integer object from unsigned_integer_pool[index]
	case SWF::ABC_ACTION_PUSHUINT:
	{
		mStack.grow(1);
		mStack.top(0) = pool_uint(mStream.read_V32());
		break;
	}
/// 0x2F ABC_ACTION_PUSHDOUBLE
/// Stream: V32 double pool index 'index'
/// Stack Out:
///  value -- Double object from double_pool[index]
	case SWF::ABC_ACTION_PUSHDOUBLE:
	{
		mStack.grow(1);
		mStack.top(0) = pool_double(mStream.read_V32());
		break;
	}
/// 0x30 ABC_ACTION_PUSHSCOPE
/// Stack In:
///  scope -- a scope
/// Stack Out:
///  .
/// Do: Enter scope without altering base.
	case SWF::ABC_ACTION_PUSHSCOPE:
	{
		asScope = mStack.top(0).to_scope();
		mStack.drop(1);
		pushScope(asScope);
		break;
	}
/// 0x31 ABC_ACTION_PUSHNAMESPACE
/// Stream: V32 namespace pool index 'index'
/// Stack Out:
///  ns -- Namespace object from namespace_pool[index]
	case SWF::ABC_ACTION_PUSHNAMESPACE:
	{
		asNamespace *ns = pool_namespace(mStream.read_V32());
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
	case ABC_ACTION_HASNEXT2:
	{
		int32_t oindex = mStream.read_V32();
		int32_t iindex = mStream.read_V32();
		as_object *obj = mFrame.value(oindex).to_object();
		int32_t index = mFrame.value(iindex).to_number<int32_t>();
		asBinding *next = obj->next_after_index(index);
		mStack.grow(1);
		if (next)
		{
			mStack.top(0) = true;
			mFrame.value(oindex) = next->getOwner();
			mFrame.value(iindex) = next->getName();
		}
		else
		{
			mStack.top(0) = false;
			mFrame.value(oindex).set_null();
			mFrame.value(iindex) = 0;
		}
		break;
	}
/// 0x40 ABC_ACTION_NEWFUNCTION
/// Stream: V32 'index'
/// Stack Out:
///  func -- the function object
/// Do: Information about function is in the pool at index. Construct the
///  function from this information, current scope, and base of the scope.
	case SWF::ABC_ACTION_NEWFUNCTION:
	{
		mStack.grow(1);
		asMethod *m = pool_method(mStream.read_V32());
		mStack.top(0) = m->construct(mCurrentScope, mBaseScope);
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
		uint32_t argc = mStream.read_V32();
		as_function *f = mStack.top(argc + 1).to_function();
		as_object *obj = mStack.top(argc).to_object();
		mStack.top(argc + 1) = f.call(obj, mStack);
		mStack.drop(argc + 1);
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
		uint32_t argc = mStream.read_V32();
		as_function *f = mStack.top(argc).to_function();
		mStack.top(argc) = f.construct(mStack);
		mStack.drop(argc);
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
		uint32_t dispatch_id = mStream.read_V32() - 1;
		uint32_t argc = mStream.read_V32();
		as_object *obj = mStack.top(argc).to_object();
		mStack.top(argc) = obj.call_dispatch(dispatch_id);
		mStack.drop(argc);
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
		asMethod *m = pool_method(mStream.read_V32());
		uint32_t argc = mStream.read_V32();
		mStack.top(argc) = m->call_static(mStack.top(argc), mStack.top(argc - 1));
		mStack.drop(argc);
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
		asName a = mStream.read_V32();
		mStack.drop(completeName(a));
	
		mStack.top(argc) = mStack.top(argc).get_named_super(a)
			.call_function(mStack.top(argc - 1));
		mStack.drop(opcode == SWF::ABC_ACTION_CALL_SUPER ? argc : argc + 1);
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
		bool lex_only = (opcode == SWF::ABC_ACTION_CALLPROPLEX);
		asName a = mStream.read_V32();
		uint32_t argc = mStream.read_V32();
		int shift = completeName(a, argc);
		// Ugly setup. Any name stuff is between the args and the object.
		as_object *obj = mStack.top(argc + shift).to_object();
		mStack.top(shift + argc) = obj->call_property(a, mStack.top(argc));
		mStack.drop(shift + argc);
		if (opcode == SWF::ABC_ACTION_CALLPROPVOID)
			mStack.drop(1);
		break;
	}
/// 0x47 ABC_ACTION_RETURNVOID
/// Do: Return an undefined object up the callstack.
	case SWF::ABC_ACTION_RETURNVOID:
	{
		mReturnStack.grow(1);
		mReturnStack.top(0).set_undefined();
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
		mReturnStack.push(mStack.pop());
		break;
	}
/// 0x49 ABC_ACTION_CONSTRUCTSUPER
/// Stream: V32 'arg_count'
/// Stack In:
///  obj -- the object whose super's constructor should be invoked
///  arg1 ... argN -- the arg_count arguments
/// Stack Out:
///  .
///
	case SWF::ABC_ACTION_CONSTRUCTSUPER:
	{
		uint32_t argc = mStream.read_V32();
		as_object *obj = mStack.top(argc).to_object();
		obj->super_construct(mStack.top(argc - 1));
		mStack.drop(argc + 1);
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
		asName a = mStream.read_V32();
		uint32_t argc = mStream.read_V32();
		int shift = completeName(a, argc);
		as_object *obj = mStack.top(argc + shift).to_object();
		mStack.top(argc + shift) = obj->construct_property(a, mStack.top(argc));
		mStack.drop(argc + shift);
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
		as_object *obj = mCH->newOfType(NSV::CLASS_OBJECT);
		uint32_t argc = mStream.read_V32();
		int i = argc;
		while (i--)
		{
			obj->add_property(mStack.top(i * 2 + 1), mStack.top(i * 2));
		}
		mStack.drop(argc * 2 - 1);
		mStack.top(0) = obj;
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
		uint32_t asize = mStream.read_V32();
		as_object *obj = mCH->newOfType(NSV::CLASS_ARRAY);
		as_array *array = dynamic_cast<as_array*> (obj);
		array->load(mStack.top(asize - 1));
		mStack.drop(asize - 1);
		mStack.top(0) = array;
		break;
	}
/// 0x57 ABC_ACTION_NEWACTIVATION
/// Stack Out:
///  vtable -- A new virtual table, which has the previous one as a parent.
	case SWF::ABC_ACTION_NEWACTIVATION:
	{
		// TODO
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
		uint32_t cid = mStream.read_V32();
		mStack.top(0) = mCH->newOfId(cid, mStack.top(0));
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
		asName a = mStream.read_V32();
		mStack.drop(completeName(a));
		as_value v = findProperty(a);
		if ((opcode == SWF::ABC_ACTION_FINDPROPSTRICT) && v.is_undefined())
			throw ASReferenceException();
		break;
	}
/// 0x5F ABC_ACTION_FINDDEF
/// Stream: V32 'name_id' (no ns expansion)
/// Stack Out:
///  def -- The definition of the name at name_id.
	case SWF::ABC_ACTION_FINDDEF:
	{
		asName a = mStream.read_V32();
		// TODO
		break;
	}
/// 0x60 ABC_ACTION_GETLEX
/// Stream: V32 'name_id' (no ns expansion)
/// Stack Out:
///  property -- The result of 0x5D (ABC_ACTION_FINDPROPSTRICT)
///   + 0x66 (ABC_ACTION_GETPROPERTY)
	case SWF::ABC_ACTION_GETLEX
	{
		asName a = mStream.read_V32();
		as_value v = findProperty(a);
		if (v.is_undefined())
			throw ASReferenceException();
		mStack.grow(1);
		mStack.top(0) = v.getProperty(a);
		break;
	}
/// 0x61 ABC_ACTION_SETPROPERTY
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
		asName a = mStream.read_V32();
		as_value &v = mStack.pop();
		if (!a.isRuntime())
		{
			mStack.top(0).setProperty(a, v);
			mStack.drop(1);
		}
		else
		{
			if (a.isRtns() || !(mStack.top(0).is_object()
				&& mStack.top(1).is_dictionary()))
			{
				mStack.drop(completeName(a));
				mStack.top(0).setProperty(a, v);
				mStack.drop(1);
			}
			else
			{
				mStack.top(1).setDictProperty(mStack.top(0), v);
				mStack.drop(2);
			}
		}
		break;
	}
/// 0x62 ABC_ACTION_GETLOCAL
/// Stream: V32 'frame_index'
/// Frame: value at frame_index is needed
/// Stack Out:
///  value
	case SWF::ABC_ACTION_GETLOCAL:
	{
		mStack.grow(1);
		mStack.top(0) = mFrame.value(mStream.read_V32());
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
		mFrame.value(mStream.read_V32()) = mStack.top(0);
		mStack.drop(1);
		break;
	}
/// 0x64 ABC_ACTION_GETGLOBALSCOPE
/// Stack Out:
///  global -- The global scope object
	case SWF::ABC_ACTION_GETGLOBALSCOPE:
	{
		mStack.grow(1);
		mStack.top(0) = mGlobalScope;
		break;
	}
/// 0x65 ABC_ACTION_GETSCOPEOBJECT
/// Stream: S8 'depth'
/// Stack Out:
///  scope -- The scope object at depth
	case SWF::ABC_ACTION_GETSCOPEOBJECT
	{
		uint8_t depth = mStream.read_u8();
		mStack.grow(1);
		mStack.top(0) = mScopeStack(depth);
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
		asName a = mStream.read_V32();
		if (!a.isRuntime())
		{
			mStack.top(0) = mStack.top(0).getProperty(a, v);
		}
		else
		{
			if (a.isRtns() || !(mStack.top(0).is_object()
				&& mStack.top(1).is_dictionary()))
			{
				mStack.drop(completeName(a));
				mStack.top(0) = mStack.top(0).getProperty(a);
			}
			else
			{
				mStack.top(1) = mStack.top(1).getDictProperty(mStack.top(0));
				mStack.drop(1);
			}
		}
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
		asName a = mStream.read_V32();
		as_value& v = mStack.pop();
		mStack.drop(completeName(a));
		mStack.pop().to_object().setProperty(a, v, true); // true for init
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
		asName a = mStream.read_V32();
		mStack.drop(completeName(a));
		mStack.top(0) = mStack.top(0).deleteProperty(a);
		break;
	}
/// 0x6C ABC_ACTION_GETSLOT
/// Stream: V32 'slot_index + 1'
/// Stack In:
///  obj -- The object which owns the desired slot.
/// Stack Out:
///  slot -- obj.slots[slot_index]
	case SWF::ABC_ACTION_GETSLOT
	{
		uint32_t sindex = mStream.read_V32();
		if (!sindex)
			throw ASException();
		--sindex;
		mStack.top(0) = mStack.top(0).getSlot(sindex);
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
		uint32_t sindex = mStream.read_V32();
		if (!sindex)
			throw ASException();
		--sindex;
		mStack.top(0).setSlot(sindex, mStack.top(1));
		mStack.drop(2);
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
		uint32_t sindex = mStream.read_V32();
		if (!sindex)
			throw ASException();
		--sindex;
		mStack.grow(1);
		mStack.top(0) = mGlobal.getSlot(sindex);
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
		uint32_t sindex = mStream.read_V32();
		if (!sindex)
			throw ASException();
		--sindex;
		mGlobal.setSlot(sindex, mStack.pop());
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
		mStack.top(0) = mStack.top(0).to_escaped_xml_element();
		break;
	}
/// 0x72 ABC_ACTION_ESC_XATTR
/// Stack In:
///  value -- An object to be escaped
/// Stack Out:
///  str_value -- value as a string, escaped suitably for an XML attribute.
	case SWF::ABC_ACTION_ESC_XATTR:
	{
		mStack.top(0) = mStack.top(0).to_escaped_xml_attribute();
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
		mStack.top(0) = mStack.top(0).to_number<int>();
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
		mStack.top(0) = mStack.top(0).to_double();
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
		mStack.top(0) = mStack.top(0).to_bool();
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
		mStack.top(0) = mStack.top(0).to_object();
		if (mStack.top(0).is_undefined() || mStack.top(0).is_null())
			throw ASTypeException();
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
		if (!mStack.top(0).is_xml())
			throw ASTypeException();
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
		asName a = mStream.read_V32();
		mStack.drop(completeName(a));
		mStack.top(0) = mStack.top(0).coerce(a);
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
		asName a = mStream.read_V32();
		mStack.drop(completeName(a));
		if (!mStack.top(0).conforms_to(a)
			mStack.top(0).set_null();
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
		if (!mStack.top(1).conforms_to(mStack.top(0).to_object()))
			mStack.top(1).set_null();
		mStack.drop(1);
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
			mStack.top(0) = mStack.top(0).to_object();
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
		mStack.top(0) = mStack.top(0).to_number() + 1;
		break;
	}
/// 0x92 ABC_ACTION_INCLOCAL
/// Stream: V32 'frame_addr'
/// Frame: Load i from frame_addr and increment it.
	case SWF::ABC_ACTION_INCLOCAL:
	{
		uint32_t foff = mStream.read_V32();
		mFrame.value(foff) = mFrame.value(foff).to_number() + 1;
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
		uint32_t foff = mStream.read_V32();
		mFrame.value(foff) = mFrame.value(foff).to_number() - 1;
		break;
	}
/// 0x95 ABC_ACTION_ABC_TYPEOF
/// Stack In:
///  obj -- An object
/// Stack Out:
///  type -- typeof(obj) as a string
	case SWF::ABC_ACTION_ABC_TYPEOF:
	{
		mStack.top(0) = mStack.top(0).typeOf();
		break;
	}
/// 0x96 ABC_ACTION_NOT
/// Stack In:
///  obj -- An object
/// Stack Out:
///  nobj -- A truth object with value !((Boolean) obj)
	case SWF::ABC_ACTION_NOT:
	{
		mStack.top(0).set_bool(!mStack.top(0).to_bool();)
		break;
	}
/// 0x97 ABC_ACTION_BITNOT
/// Stack In:
///  obj -- An object
/// Stack Out:
///  nint -- ~((Int) obj)
	case SWF::ABC_ACTION_BITNOT:
	{
		mStack.top(0) = ~mStack.top(0).to_number<int>();
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
		mStack.top(1) = mStack.top(1).add(mStack.top(0));
		mStack.drop(1);
		break;
	}
/// 0xA1 ABC_ACTION_SUBTRACT
/// Stack In:
///  a
///  b
/// Stack Out:
///  a - b (double)
	case SWF::ABC_ACTION_SUBTRACT:
	{
		mStack.top(1) = mStack.top(1).to_number() - mStack.top(1).to_number();
		mStack.drop(1);
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
			>> mStack.top(0).to_number<int>();
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
		mStack.top(1) = mStack.top(1).weak_equals(mStack.top(0));
		mStack.drop(1);
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
		mStack.top(1) = mStack.top(1).strict_equals(mStack.top(0));
		mStack.drop(1);
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
		mStack.top(1) = mStack.top(1).less_than(mStack.top(0));
		mStack.drop(1);
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
		mStack.top(1) = mStack.top(1).less_equal(mStack.top(0));
		mStack.drop(1);
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
		mStack.top(1) = mStack.top(1).greater_than(mStack.top(0));
		mStack.drop(1);
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
		mStack.top(1) = mStack.top(1).greater_equal(mStack.top(0));
		mStack.drop(1);
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
		if (mStack.top(1).is_null())
			mStack.top(1) = false;
		else // Calling to_object intentionally causes an exception if super is not an object.
			mStack.top(1) = mStack.top(1).conforms_to(mStack.top(0).to_object());
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
		asName a = mStream.read_V32();
		mStack.drop(completeName(a));
		mStack.top(0).set_bool(mStack.top(0).conforms_to(a));
	}
/// 0xB3 ABC_ACTION_ISTYPELATE
/// Stack In:
///  type -- A type to match
///  obj -- An object
/// Stack Out:
///  truth -- Truth of "obj is of type"
	case SWF::ABC_ACTION_ISTYPELATE:
	{
		mStack.top(1).set_bool(mStack.top(1).conforms_to(mStack.top(0)));
		mStack.drop(1);
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
		mStack.top(1).set_bool(mStack.top(1).to_object().contains(mStack.top(0)));
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
		mStack.top(0) = mStack.top(0).to_number<int>() - 1;
		break;
	}
/// 0xC2 ABC_ACTION_INCLOCAL_I
/// See: 0x92 (ABC_ACTION_INCLOCAL), but forces types to int, not double
	case SWF::ABC_ACTION_INCLOCAL_I:
	{
		uint32_t foff = mStream.read_V32();
		mFrame.value(foff) = mFrame.value(foff).to_number<int>() + 1;
		break;
	}
/// 0xC3 ABC_ACTION_DECLOCAL_I
/// See: 0x94 (ABC_ACTION_DECLOCAL), but forces types to int, not double
	case SWF::ABC_ACTION_DECLOCAL_I:
	{
		uint32_t foff = mStream.read_V32();
		mFrame.value(foff) = mFrame.value(foff).to_number<int>() - 1;
		break;
	}
/// 0xC4 ABC_ACTION_NEGATE_I
/// See: 0x90 (ABC_ACTION_NEGATE), but forces type to int, not double
	case SWF::ABC_ACTION_NEGATE_I:
	{
		mStack.top(0) = - mStack.top(0).to_number<int>();
		break;
	}
/// 0xC5 ABC_ACTION_ADD_I
/// See: 0xA0 (ABC_ACTION_ADD), but forces type to int
	case SWF::ABC_ACTION_ADD_I:
	{
		mStack.top(1) = mStack.top(1).to_number<int>() + mStack.top(0).to_number<int>();
		mStack.drop(1);
		break;
	}
/// 0xC6 ABC_ACTION_SUBTRACT_I
/// See: 0xA1 (ABC_ACTION_SUBTRACT), but forces type to int
	case SWF::ABC_ACTION_SUBTRACT_I:
	{
		mStack.top(1) = mStack.top(1).to_number<int>() - mStack.top(0).to_number<int>();
		mStack.drop(1);
		break;
	}
/// 0xC7 ABC_ACTION_MULTIPLY_I
/// See: 0xA2 (ABC_ACTION_MULTIPLY), but forces type to int
	case SWF::ABC_ACTION_MULTIPLY_I:
	{
		mStack.top(1) = mStack.top(0).to_number<int>() * mStack.top(1).to_number<int>();
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
		mStack.grow(1);
		mStack.top(0) = mFrame.value(opcode - SWF::ABC_ACTION_GETLOCAL0);
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
		mFrame.value(opcode - SWF::ABC_ACTION_SETLOCAL0) = mStack.top(0);
		mStack.drop(1);
		break;
	}

/// 0xEF ABC_ACTION_DEBUG
/// Stream: 7 bytes of unknown stuff to be skipped
/// Do: skip ahead 7 bytes in stream
	case SWF::ABC_ACTION_DEBUG:
	{
		seekBy(7);
		break;
	}
/// 0xF0 ABC_ACTION_DEBUGLINE
/// Stream: V32 'line_number'
/// Do: Nothing, but line_number is for the debugger if wanted.
	case SWF::ABC_ACTION_DEBUGLINE:
	{
		mStream.skip_V32();
		break;
	}
/// 0xF1 ABC_ACTION_DEBUGFILE
/// Stream: V32 'name_offset'
/// Do: Nothing. 'name_offset' into string pool is the file name if wanted.
	case SWF::ABC_ACTION_DEBUGFILE:
	{
		mStream.skip_V32();
		break;
	}
/// 0xF2 ABC_ACTION_BKPTLINE
/// Stream: V32 'line_number'
/// Do: Enter debugger if present, line_number is the line number in source.
	case SWF::ABC_ACTION_BKPTLINE:
	{
		mStream.skip_V32();
		break;
	}
	} // end of switch statement
	
	} // end of main loop
} // end of execute function

void
ActionMachine::getMember(asClass* pDefinition, asBoundName& name,
	as_value& instance)
{
	if (!instance.is_object())
		throw ASTypeError();

	asBinding *pBinding = pDefinition->findMember(name);
	if (pBinding->isWriteOnly())
		throw ASReferenceError();

	if (!pBinding->isGetSet())
	{
		mStack.push(pBinding->getFromInstance(instance));
		return;
	}

	// This is a getter, so we need to execute it. Even those
	// written in C++ get called like this, with pushCall handling.
	// 1 parameter (the source), 1 value expected back.
	mStack.push(instance);
	pushCall(1, 1, pBinding->getGetter());
}

void
ActionMachine::setMember(asClass *pDefinition, asBoundName& name,
	as_value& instance, as_value& newvalue)
{
	if (!instance.is_object())
		throw ReferenceError();

	asBinding *pBinding = pDefinition->findMember(name);
	if (pBinding->isReadOnly())
		throw ASReferenceError();

	if (!pBinding->isGetSet())
	{
		pBinding->setInInstance(instance, newvalue);
		return;
	}

	// Two parameters -- the target object, the value to set.
	mStack.push(instance);
	mStack.push(newvalue);
	pushCall(2, 0, pBinding->getSetter());
}

int
ActionMachine::completeName(asBoundName &name, int offset)
{
	int size = 0;

	asUnboundName* uname = mNamePool.at(name.id);
	if (uname->isRuntime())
	{
		as_value obj = mStack.top(offset);
		if (obj.is_object() && obj.to_object()->isQName())
			name.fill(obj.to_object());
		++size;

		if (uname->isRuntimeNamespace())
			++size; // Ignore the Namespace.
	}
	else if (uname->isRuntimeNamespace())
	{
		uname->setNamespace(mStack.top(offset);
		++size;
	}
	return size;
}

asClass *
ActionMachine::findSuper(as_value &v, bool find_for_primitive)
{
	if (v.is_undefined() || v.is_null())
		return NULL;

	if (v.is_object())
	{
		asClass *pProto = v.to_object()->getClass();
		return pProto ? pProto->getSuper() : NULL;
	}

	if (v.is_number())
	{
		return mCH->getClass(NSV::CLASS_NUMBER);
	}

	// And so on...
	// TODO: Other primitives
	return NULL;
}

