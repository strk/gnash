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

#ifndef GNASH_AS_NAME_H
#define GNASH_AS_NAME_H

#include <vector>

namespace gnash {

class as_object;

class asName
{
public:
	/// If true, the name needs a run-time string value to complete it.
	bool isRuntime() { return (mFlags & FLAG_RTNAME) != 0; }

	/// If true, the name needs a run-time namespace to complete it.
	bool isRtns() { return (mFlags & FLAG_RTNS) != 0; }

	bool isQName() { return (mFlags & FLAG_QNAME) != 0; }
	void setQName() { mFlags |= FLAG_QNAME; }

	void setNamespace(asNamespace *ns) { mNamespace = ns; }
	asNamespace* getNamespace() const { return mNamespace; }

	string_table::key getABCName() const { return mABCName; }
	void setABCName(string_table::key n) { mABCName = n;}

	string_table::key getGlobalName() const { return mGlobalName;}
	void setGlobalName(string_table::key n) {mGlobalName = n;}
	
	void setAttr() { mFlags |= FLAG_ATTR; }

	void fill(as_object*) {/*TODO*/}

	Property* findProperty();

	typedef enum
	{
		KIND_Qname = 0x07,
		KIND_QnameA = 0x0D,
		KIND_RTQname = 0x0F,
		KIND_RTQnameA = 0x10,
		KIND_RTQnameL = 0x11,
		KIND_RTQnameLA = 0x12,
		KIND_Multiname = 0x09,
		KIND_MultinameA = 0x0E,
		KIND_MultinameL = 0x1B,
		KIND_MultinameLA = 0x1C
	} kinds;
	typedef enum
	{
		FLAG_ATTR = 0x01,
		FLAG_QNAME = 0x02,
		FLAG_RTNS = 0x04,
		FLAG_RTNAME = 0x08,
		FLAG_NSSET = 0x10
	} flags;

	boost::uint8_t mFlags;
	std::vector<asNamespace*> *mNamespaceSet;

	asName() : mFlags(0), mNamespaceSet(NULL), mABCName(0), mGlobalName(0),
		mNamespace(NULL)
	{/**/}
private:
	string_table::key mABCName;
	string_table::key mGlobalName;
	asNamespace* mNamespace;
};

} // end of namespace gnash
#endif /* GNASH_AS_NAME_H */
