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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
//

#include "as_value.h"
#include "fn_call.h"
#include "mysql_table.h"

namespace mysqldb
{
	using namespace gnash;

	void	size_method(const fn_call& fn)
	{
		assert(fn.this_ptr);	assert(fn.env);
		table* tbl = (table*) (as_object*) fn.this_ptr;
		*fn.result = tbl->size();
	}

	table::table(MYSQL_RES* result)
	{
		as_object::set_member("size", &size_method);

	  // retrieve data
		MYSQL_FIELD* fld = mysql_fetch_fields(result);
		int num_fields = mysql_num_fields(result);
		int num_rows =  mysql_num_rows(result);
		
		m_data.resize(num_rows);
		for (int i = 0; i < num_rows; i++)
		{
			MYSQL_ROW row = mysql_fetch_row(result);

			m_data[i] = new as_object();
			m_data[i]->add_ref();

			for (int j = 0; j < num_fields; j++)
			{
				as_value val;
				if (row[j] == NULL)
				{
					val.set_null();
				}
				else
				{
					switch (fld[j].type)
					{
						case MYSQL_TYPE_TINY:
						case MYSQL_TYPE_SHORT:
						case MYSQL_TYPE_INT24:
							val.set_int(atoi(row[j]));
							break;

						case MYSQL_TYPE_DECIMAL:
						case MYSQL_TYPE_LONG:
						case MYSQL_TYPE_FLOAT:
						case MYSQL_TYPE_DOUBLE:
						case MYSQL_TYPE_LONGLONG:
							val.set_double(atof(row[j]));
							break;

						case MYSQL_TYPE_NULL:
						case MYSQL_TYPE_TIMESTAMP:
						case MYSQL_TYPE_DATE:
						case MYSQL_TYPE_TIME:
						case MYSQL_TYPE_DATETIME:
						case MYSQL_TYPE_YEAR:
						case MYSQL_TYPE_NEWDATE:
						case MYSQL_TYPE_VARCHAR:
						case MYSQL_TYPE_BIT:
						case MYSQL_TYPE_NEWDECIMAL:
						case MYSQL_TYPE_ENUM:
						case MYSQL_TYPE_SET:
						case MYSQL_TYPE_TINY_BLOB:
						case MYSQL_TYPE_MEDIUM_BLOB:
						case MYSQL_TYPE_LONG_BLOB:
						case MYSQL_TYPE_BLOB:
						case MYSQL_TYPE_VAR_STRING:
						case MYSQL_TYPE_STRING:
						case MYSQL_TYPE_GEOMETRY:
							val.set_string(row[j]);
							break;
					}
				}
				m_data[i]->set_member(fld[j].name, val);
			}
		}
	}

	table::~table()
	{
		for (int i = 0; i < size(); i++)
		{
			m_data[i]->drop_ref();
		}
	}

	bool	table::get_member(const tu_stringi& name, as_value* val)
	{
		// check table methods
		if (as_object::get_member(name, val) == false)
		{
			// hack
			int idx = atoi(name.c_str());
			if (idx >=0 && idx < size())
			{
				*val = m_data[idx];
			}
			else
			{
				val->set_undefined();
			}
		}
		return true;
	}

//	void	table::set_member(const tu_stringi& name, const as_value& val)
//	{
//		as_object::set_member(name,val);
//	}


	int table::size()
	{
		return m_data.size();
	}

} 	//	end of namespace mysqldb
