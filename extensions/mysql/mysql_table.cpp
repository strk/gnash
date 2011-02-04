// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include "as_value.h"
#include "fn_call.h"
#include "mysql_table.h"

namespace mysqldb
{
	using namespace gnash;

	as_value	size_method(const fn_call& fn)
	{
		assert(fn.this_ptr);	assert(fn.env);
		table* tbl = (table*) (as_object*) fn.this_ptr;
		return as_value(tbl->size());
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
							val.set_int(stroul(row[j], NULL, 0));
							break;

						case MYSQL_TYPE_DECIMAL:
						case MYSQL_TYPE_LONG:
						case MYSQL_TYPE_FLOAT:
						case MYSQL_TYPE_DOUBLE:
						case MYSQL_TYPE_LONGLONG:
							val.set_double(strtod(row[j], NULL));
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

	bool	table::get_member(const std::string& name, as_value* val)
	{
		// check table methods
		if ( as_object::get_member(name, val) == false )
		{
			// hack
			int idx = strtoul(name.c_str(), NULL, 0);
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

	int table::size()
	{
		return m_data.size();
	}

} 	//	end of namespace mysqldb
