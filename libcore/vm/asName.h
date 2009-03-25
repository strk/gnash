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
	
	enum Kind
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
	};

	asName()
        :
        _flags(0),
        _namespaceSet(0),
        _abcName(0),
        _globalName(0),
		_namespace(0)
	{}

    void setFlags(Kind kind) {
        _flags = kind;
    }

    boost::uint8_t flags() const {
        return _flags;
    }

    /// If true, the name needs a run-time string value to complete it.
	bool isRuntime() { return _flags & FLAG_RTNAME; }

	/// If true, the name needs a run-time namespace to complete it.
	bool isRtns() { return _flags & FLAG_RTNS; }

	bool isQName() { return _flags & FLAG_QNAME; }
	void setQName() { _flags |= FLAG_QNAME; }

	void setNamespace(asNamespace *ns) { _namespace = ns; }
	asNamespace* getNamespace() const { return _namespace; }

	string_table::key getABCName() const { return _abcName; }
	void setABCName(string_table::key n) { _abcName = n;}

	string_table::key getGlobalName() const { return _globalName;}
	void setGlobalName(string_table::key n) { _globalName = n;}
	
	void setAttr() { _flags |= FLAG_ATTR; }

	void fill(as_object*) {}

	Property* findProperty();
    
    void namespaceSet(std::vector<asNamespace*>* v) {
        _namespaceSet = v;
    }

    const std::vector<asNamespace*>* namespaceSet() const {
        return _namespaceSet;
    }

private:

	enum Flag
	{
		FLAG_ATTR = 0x01,
		FLAG_QNAME = 0x02,
		FLAG_RTNS = 0x04,
		FLAG_RTNAME = 0x08,
		FLAG_NSSET = 0x10
	};

    boost::uint8_t _flags;

    std::vector<asNamespace*>* _namespaceSet;

	string_table::key _abcName;
	string_table::key _globalName;
	asNamespace* _namespace;

};

} // namespace gnash
#endif 
