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
	const GetterSetter* a = boost::get<const GetterSetter>(&mBound);

	as_environment env(this_ptr.getVM());
	fn_call fn(const_cast<as_object*>(&this_ptr), env);
	if (mDestructive)
	{
		as_value ret = a->get(fn);
		// The getter might have called the setter, and we should not override.
		if (mDestructive)
		{
			mBound = ret;
			mDestructive = false;
		}
		return ret;
	}
	return a->get(fn);

}

void
Property::setDelayedValue(as_object& this_ptr, const as_value& value)
{
	GetterSetter* a = boost::get<GetterSetter>(&mBound);

	as_environment env(this_ptr.getVM());

	std::auto_ptr< std::vector<as_value> > args ( new std::vector<as_value> );
	args->push_back(value);

	fn_call fn(&this_ptr, env, args);

	a->set(fn);
	a->setCache(value);
}

void
Property::setSetter(as_function* func)
{
	if (isGetterSetter())
	{
		GetterSetter* a = boost::get<GetterSetter>(&mBound);
		a->setSetter(func);
	}
	else
		mBound = GetterSetter(NULL, func);
}

void
Property::setGetter(as_function* func)
{
	if (isGetterSetter())
	{
		GetterSetter* a = boost::get<GetterSetter>(&mBound);
		a->setGetter(func);
	}
	else mBound = GetterSetter(func, 0);
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
		    const GetterSetter& a = boost::get<GetterSetter>(mBound);
		    a.markReachableResources();
		    break;
	    }
	    default:
	    	abort(); // Not here.
		    break;
	}
}

void
GetterSetter::UserDefinedGetterSetter::markReachableResources() const
{
	if (mGetter) mGetter->setReachable();
	if (mSetter) mSetter->setReachable();
	underlyingValue.setReachable();
}

as_value
GetterSetter::UserDefinedGetterSetter::get(fn_call& fn) const
{
	ScopedLock lock(*this);
	if ( ! lock.obtained() )
	{
		return underlyingValue;
	}

	if ( mGetter ) return (*mGetter)(fn);
	else return as_value(); // should we return underlyingValue here ?
}

void
GetterSetter::UserDefinedGetterSetter::set(fn_call& fn)
{
	ScopedLock lock(*this);
	if ( ! lock.obtained() || ! mSetter )
	{
		underlyingValue = fn.arg(0);
		return;
	}

	(*mSetter)(fn);
}

as_value
Property::getValue(const as_object& this_ptr) const
{
	switch (mBound.which())
	{
	case 0: // blank, nothing to do.
		return as_value();
	case 1: // Bound value
		return boost::get<as_value>(mBound);
	case 2: // Getter/setter
		return getDelayedValue(this_ptr);
	} // end of switch
	return as_value(); // Not reached.
}

const as_value&
Property::getCache() const
{
	static as_value undefVal;

	switch (mBound.which())
	{
	case 0: // blank, nothing to do.
		return undefVal;
	case 1: // Bound value
		return boost::get<as_value&>(mBound);
	case 2: // Getter/setter
		return boost::get<GetterSetter&>(mBound).getCache();
	} // end of switch
	return undefVal; // not reached
}

void
Property::setValue(as_object& this_ptr, const as_value &value)
{
	switch (mBound.which())
	{
	case 0: // As yet unbound, so make it a simple
	case 1: // Bound value, set. Trust our callers to check read-only.
		mBound = value;
		return;
	case 2: // Getter/setter
		// Destructive are always overwritten.
		if (mDestructive)
		{
			mDestructive = false;
			mBound = value;
		}
		else setDelayedValue(this_ptr, value);
		return;
	}
}

void
Property::setCache(const as_value &value)
{
	switch (mBound.which())
	{
	case 0: // As yet unbound, so make it a simple
	case 1: // Bound value, set. Trust our callers to check read-only.
		mBound = value;
		return;
	case 2: // Getter/setter
		boost::get<GetterSetter&>(mBound).setCache(value);
		return;
	}
}

} // namespace gnash
