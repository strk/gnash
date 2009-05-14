
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
