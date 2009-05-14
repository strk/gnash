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

#ifndef GNASH_AS_BOUND_VALUES_H
#define GNASH_AS_BOUND_VALUES_H

namespace gnash {

class asBoundValue;

class asBoundAccessor
{
public:
	bool setGetter(asMethod *p) { mGetter = p; return true; }
	bool setSetter(asMethod *p) { _setter = p; return true; }
	bool setValue(asBoundValue *p) { mValue = p; return true; }

	asBoundValue* getValue() { return mValue; }
	asMethod *getGetter() { return mGetter; }
	asMethod *getSetter() { return _setter; }

private:
	asMethod *mGetter;
	asMethod *_setter;
	asBoundValue *mValue;
};

class asBoundValue 
{
public:
	asBoundValue() : mConst(false), mValue()
	{ mValue.set_undefined(); }
	void setValue(as_value &v) { mValue = v; }
	as_value getCurrentValue() { return mValue; }

	void setType(asClass *t) { mType = t; }
	asClass *getType() { return mType; }

private:
	bool mConst;
	asClass *mType;
	as_value mValue;
};

} // namespace gnash

#endif
