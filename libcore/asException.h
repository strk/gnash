
#ifndef GNASH_AS_EXCEPTION_H
#define GNASH_AS_EXCEPTION_H

namespace gnash {

class asException
{
public:
	void setStart(boost::uint32_t i) { _start = i; }
	void setEnd(boost::uint32_t i) { mEnd = i; }
	void setCatch(boost::uint32_t i) { mCatch = i; }
	void catchAny() { mCatchAny = true; }
	void setCatchType(asClass* p) { mCatchType = p; }
	void setNamespace(asNamespace* n) { _namespace = n; }
	void setName(string_table::key name) { _name = name; }

private:
	boost::uint32_t _start;
	boost::uint32_t mEnd;
	boost::uint32_t mCatch;
	bool mCatchAny;
	asClass *mCatchType;
	asNamespace *_namespace;
	string_table::key _name;
};

} // namespace gnash
#endif
