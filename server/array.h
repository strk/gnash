// 
//   Copyright (C) 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifndef GNASH_ARRAY_H
#define GNASH_ARRAY_H

#include "action.h"
#include <vector>

namespace gnash {

	struct as_array_object;

	void array_init(as_array_object *array);

	struct as_array_object : public as_object
	{
		std::vector<as_value> elements;
		as_array_object();

		const int size() const;

		// this function is used internally by set_member and get_member
		// it takes a string that is the member name of the array and returns -1
		// if the string does not refer to an index, or an appropriate int if the string does refer to an index
		int index_requested(const tu_stringi& name);

		virtual void set_member(const tu_stringi& name, const as_value& val );

		virtual bool get_member(const tu_stringi& name, as_value *val);
	};

	void	as_global_array_ctor(const fn_call& fn);
};

#endif
