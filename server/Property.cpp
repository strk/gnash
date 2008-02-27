// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#include "Property.h"
#include "VM.h"
#include "as_function.h"
#include "Machine.h"

namespace gnash {

as_value
Property::getDelayedValue(const as_object& this_ptr) const
{
	const as_accessors* a = boost::get<const as_accessors>(&mBound);

	// Don't recursively invoke a getter
	//
	// NOTE: it seems recursion protection is handled differently
	//       when the SWF is targetted at player 7 or higher.
	//       See actionscript.all/Object.as
	//
	as_accessors::ScopedLock lock(*a);
	if ( ! lock.obtained() )
	{
		return a->underlyingValue;
	}

	as_environment env;
	fn_call fn(const_cast<as_object*>(&this_ptr), &env, 0, 0);
	if (mDestructive)
	{
		as_function *f = a->mGetter;
		as_value ret = (*f)(fn);
		// The getter might have called the setter, and we should not override.
		if (mDestructive)
		{
			mBound = ret;
			mDestructive = false;
		}
		return ret;
	}
	return (*a->mGetter)(fn);

	as_value v;	
	VM::get().getMachine()->immediateFunction(a->mGetter, 
		const_cast<as_object*>(&this_ptr),	v, 0, 0);
	if (mDestructive)
	{
		((boundType) mBound) = v;
		mDestructive = false;
	}
	return v;
}

void
Property::setDelayedValue(as_object& this_ptr, const as_value& value)
{
	log_debug("setDelayedValue: %s", value.to_debug_string().c_str());
	as_accessors* a = boost::get<as_accessors>(&mBound);

	// Don't recursively invoke a setter
	//
	// NOTE: it seems recursion protection is handled differently
	//       when the SWF is targetted at player 7 or higher.
	//       In particular, it is not avoided unless the setter
	//       calls self.. See actionscript.all/Object.as
	//
	as_accessors::ScopedLock lock(*a);
	if ( ! lock.obtained() )
	{
		a->underlyingValue = value;
		return;
	}


	as_environment env;
	env.push(value);
	fn_call fn(&this_ptr, &env, 1, 0);
	(*a->mSetter)(fn);

	return;

	// TODO: Push value
	VM::get().getMachine()->immediateProcedure(a->mSetter,
		const_cast<as_object*>(&this_ptr), 1, 0);
}

void
Property::setSetter(as_function* func)
{
	if (isGetterSetter())
	{
		boost::get<as_accessors>(&mBound)->mSetter = func;
	}
	else
		mBound = as_accessors(NULL, func);
}

void
Property::setGetter(as_function* func)
{
	if (isGetterSetter())
	{
		boost::get<as_accessors>(&mBound)->mGetter = func;
	}
	else
		mBound = as_accessors(func, NULL);
}

void
Property::setReachable() const
{
	switch (mBound.which())
	{
	case 0: // Blank, nothing to do.
		break;
	case 1: // Simple property, value
	{
		boost::get<as_value>(mBound).setReachable();
		break;
	}
	case 2: // Getter/setter
	{
		const as_accessors& a = boost::get<as_accessors>(mBound);
		a.markReachableResources();
		break;
	}
	default:
		abort(); // Not here.
		break;
	}
}

void
as_accessors::markReachableResources() const
{
	if (mGetter) mGetter->setReachable();
	if (mSetter) mSetter->setReachable();
	underlyingValue.setReachable();
}

} // namespace gnash
