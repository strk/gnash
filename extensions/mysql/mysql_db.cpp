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

#include <cstdarg>

#include "as_value.h"
#include "fn_call.h"
#include "mysql_db.h"

#ifdef _WIN32
#	define snprintf _snprintf
#endif

namespace mysqldb
{

	void	connect_method(const fn_call& fn)
	{
		assert(fn.this_ptr);	assert(fn.env);
		db* mydb = (db*) (as_object*) fn.this_ptr;

		if (fn.nargs < 4)
		{
			mydb->set_err("'connect' needs 4 arguments\n");
			fn.result->set_bool(false);
			return;
		}

		fn.result->set_bool(mydb->connect(fn.arg(0).to_string(), fn.arg(1).to_string(), fn.arg(2).to_string(), fn.arg(3).to_string()));
	}

	void	run_method(const fn_call& fn)
	{
		assert(fn.this_ptr);	assert(fn.env);
		db* mydb = (db*) (as_object*) fn.this_ptr;

		if (fn.nargs < 1)
		{
			mydb->set_err("'run' needs one argument");
			fn.result->set_int(-1);
			return;
		}

		fn.result->set_int(mydb->run(fn.arg(0).to_tu_string()));
	}

	void	open_method(const fn_call& fn)
	{
		assert(fn.this_ptr);	assert(fn.env);
		db* mydb = (db*) (as_object*) fn.this_ptr;

		if (fn.nargs < 1)
		{
			mydb->set_err("'open' needs one argument");
			fn.result->set_null();
			return;
		}

		as_object* tbl = mydb->open(fn.arg(0).to_string());
		if (tbl == NULL)
		{
			fn.result->set_null();
			return;
		}

		fn.result->set_as_object(tbl);
	}

	void	constructor(const fn_call& fn)
	{
		*fn.result = new db();
	}

	db::db(): m_db(NULL)
	{
		as_object::set_member("connect", &connect_method);
		as_object::set_member("run", &run_method);
		as_object::set_member("open", &open_method);
	}
	
	db::~db()
	{
		disconnect();
	}

	void db::disconnect()
	{
		if (m_db != NULL)
		{
		  mysql_close(m_db);    
			m_db = NULL;
	  }
	}

	bool db::connect(const char* host, const char* dbname, const char* user, const char* pwd)
	{
		// Closes a previously opened connection &
		// also deallocates the connection handle
		disconnect();

		m_db = mysql_init(NULL);

		if ( m_db == NULL )
		{
			set_err("no memory");  
			return false;
	  }

		if (mysql_real_connect(m_db, host, user, pwd, dbname,	0, NULL, CLIENT_MULTI_STATEMENTS) == NULL)
		{
			set_err("%s", mysql_error(m_db));
			return false;
  	}

		set_err("");
		return true;
	}

	int db::run(const tu_string& sql)
	{
		if (m_db == NULL)
		{
			set_err("missing connection");
			return -1;
		}

		if (mysql_query(m_db, sql))
		{
		  set_err("%s", mysql_error(m_db));
			return -1;
		}

		set_err("");
		return mysql_affected_rows(m_db);
	}

	table* db::open(const tu_string& sql)
	{
		if (m_db == NULL)
		{
			set_err("missing connection");
			return NULL;
		}

		if (mysql_query(m_db, sql.c_str()))
		{
		  set_err("%s", mysql_error(m_db));
			return NULL;
		}

		// query succeeded, process any data returned by it
		MYSQL_RES* result = mysql_store_result(m_db);
		if (result)
		{
			table* tbl = new table(result);
			mysql_free_result(result);
			set_err("");
			return tbl;
		}

		set_err("query does not return data");
		return NULL;
	}

//	bool	db::get_member(const tu_stringi& name, as_value* val)
//	{
//		return as_object::get_member(name, val);
//	}

//	void	db::set_member(const tu_stringi& name, const as_value& val)
//	{
//		as_object::set_member(name,val);
//	}

	void db::set_err(const char *fmt, ...)
	{
		char msg[BUFSIZE];
		va_list ap;
		va_start(ap, fmt);
		vsnprintf(msg, BUFSIZE, fmt, ap);
		va_end(ap);

		as_object::set_member("err", msg);
	}


} 	//	end of namespace mysqldb
