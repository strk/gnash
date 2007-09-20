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
//

#ifndef GNASH_NAMESPACE_H
#define GNASH_NAMESPACE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "string_table.h"

namespace gnash {

class abc_block;

class Namespace
{
public:
	friend class abc_block; // Parser for abc_blocks.

	typedef enum
	{
		KIND_NORMAL = 0x08,
		KIND_PRIVATE = 0x05,
		KIND_PACKAGE = 0x16,
		KIND_PACKAGE_INTERNAL = 0x17,
		KIND_PROTECTED = 0x18,
		KIND_EXPLICIT = 0x19,
		KIND_STATIC_PROTECTED = 0x1A
	} kinds;

	string_table::key getUri() const { return mUri; }
	string_table::key getPrefix() const { return mPrefix; }

	Namespace(string_table::key uri, string_table::key prefix, kinds kind) :
		mUri(uri), mPrefix(prefix), mKind(kind)
	{/**/}

	Namespace() : mUri(0), mPrefix(0), mKind(KIND_NORMAL) {/**/}

	void Initialize(string_table::key uri, string_table::key prefix, kinds kind)
	{ mUri = uri; mPrefix = prefix; mKind = kind; }

private:
	string_table::key mUri;
	string_table::key mPrefix;
	kinds mKind;
};

}; /* namespace gnash */
#endif /* GNASH_NAMESPACE_H */
