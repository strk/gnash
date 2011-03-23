//   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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
    class Property;
    namespace abc {
        class Namespace;
    }
}

namespace gnash {
namespace abc {

/// This type should always be used for the index of AbcBlocks' names.
//
/// It serves to distinguish them from global names, which are identified
/// by a string_table::key. This identifies global names' position in the
/// string_table. AbcBlock resources have nothing to do with the string_table
/// and their URI does not correspond to a string_table entry, so it
/// makes no sense whatsoever to use string_table::key to index them.   
typedef size_t URI;

/// An MultiName represents a single ABC multiname.
//
/// All MultiNames are internal to a single AbcBlock. Most are created during
/// parsing, though the Machine can also create them for the lifetime of
/// a single script run (which corresponds to an AbcBlock).
//
/// This means it is possible to store internal ABC URI of multiname here.
class MultiName
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

	MultiName()
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

	void setNamespace(Namespace *ns) { _namespace = ns; }
	Namespace* getNamespace() const { return _namespace; }

    abc::URI getABCName() const { return _abcName; }
	void setABCName(abc::URI n) { _abcName = n;}

	string_table::key getGlobalName() const { return _globalName;}
	void setGlobalName(string_table::key n) { _globalName = n;}
	
	void setAttr() { _flags |= FLAG_ATTR; }

	void fill(as_object*) {}

	Property* findProperty();
    
    void namespaceSet(std::vector<Namespace*>* v) {
        _namespaceSet = v;
    }

    const std::vector<Namespace*>* namespaceSet() const {
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

    std::vector<Namespace*>* _namespaceSet;

    abc::URI _abcName;

	string_table::key _globalName;

	Namespace* _namespace;

};

} // namespace abc
} // namespace gnash
#endif 
