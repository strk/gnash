// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

//

#ifndef TABLE_H
#define TABLE_H

#include <mysql.h>
#include <deque>

#include "as_value.h"
#include "as_object.h"


namespace mysqldb
{
	using namespace gnash;

	class table: public as_object
	{
	public:
		table(MYSQL_RES* result);
		~table();

		virtual bool	get_member(const tu_stringi& name, as_value* val);
//		virtual void	set_member(const tu_stringi& name, const as_value& val);
		int size();

		std::vector< as_object* > m_data;	// [columns][rows]

	};



}

#endif // TABLE_H
