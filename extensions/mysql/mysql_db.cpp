// mysql_db.cpp:  MySQL database interface ActionScript objects, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <cstdarg>

#include <mysql/errmsg.h>
#include <mysql/mysql.h>
#include <iostream>
#include <vector>

#include "log.h"
#include "Array_as.h"
#include "as_value.h"
#include "fn_call.h"
#include "mysql_db.h"
#include "builtin_function.h" // need builtin_function

using namespace std;

namespace gnash
{

as_value mysql_connect(const fn_call& fn);
as_value mysql_qetData(const fn_call& fn);
as_value mysql_disconnect(const fn_call& fn);

as_value mysql_query(const fn_call& fn);
as_value mysql_row(const fn_call& fn);
as_value mysql_fields(const fn_call& fn);
as_value mysql_fetch(const fn_call& fn);
as_value mysql_store(const fn_call& fn);
as_value mysql_free(const fn_call& fn);

LogFile& dbglogfile = LogFile::getDefaultInstance();

class mysql_as_object : public as_object
{
public:
    MySQL obj;
};

static void
attachInterface(as_object *obj)
{
//    GNASH_REPORT_FUNCTION;

    obj->init_member("connect", new builtin_function(mysql_connect));
    obj->init_member("qetData", new builtin_function(mysql_qetData));
    obj->init_member("disconnect", new builtin_function(mysql_disconnect));
    obj->init_member("query", new builtin_function(mysql_query));
    obj->init_member("fetch_row", new builtin_function(mysql_fetch));
    obj->init_member("num_fields", new builtin_function(mysql_fields));
    obj->init_member("free_result", new builtin_function(mysql_free));
    obj->init_member("store_results", new builtin_function(mysql_store));
}

static as_object*
getInterface()
{
//    GNASH_REPORT_FUNCTION;

    static boost::intrusive_ptr<as_object> o;
    if (o == NULL) {
	o = new as_object();
    }
    return o.get();
}

static as_value
mysql_ctor(const fn_call& /*fn*/)
{
//    GNASH_REPORT_FUNCTION;

    mysql_as_object* obj = new mysql_as_object();

    attachInterface(obj);
    return as_value(obj); // will keep alive
//    printf ("Hello World from %s !!!\n", __PRETTY_FUNCTION__);
}


MySQL::MySQL(): _db(NULL), _result(NULL), _row(NULL)
{
//    GNASH_REPORT_FUNCTION;
}

MySQL::~MySQL()
{
//    GNASH_REPORT_FUNCTION;
    disconnect();
}

int
MySQL::num_fields()
{
//    GNASH_REPORT_FUNCTION;
    if (_result) {
	return num_fields(_result);
    }
    return -1;
}

int
MySQL::num_fields(MYSQL_RES *result)
{
//    GNASH_REPORT_FUNCTION;
    return mysql_num_fields(result);
}

MYSQL_ROW
MySQL::fetch_row()
{
//    GNASH_REPORT_FUNCTION;
    if (_result) {
	return fetch_row(_result);
    }
    return NULL;
}

MYSQL_ROW
MySQL::fetch_row(MYSQL_RES *result)
{
//    GNASH_REPORT_FUNCTION;
    return mysql_fetch_row(result);
}

void
MySQL::free_result()
{
//    GNASH_REPORT_FUNCTION;
    if (_result) {
	free_result(_result);
    }
}

void
MySQL::free_result(MYSQL_RES *result)
{
//    GNASH_REPORT_FUNCTION;
    mysql_free_result(result);
}

MYSQL_RES *
MySQL::store_result()
{
//    GNASH_REPORT_FUNCTION;
    if (_db) {
	return store_result(_db);
    }
    return NULL;
}

MYSQL_RES *
MySQL::store_result(MYSQL *db)
{
//    GNASH_REPORT_FUNCTION;
    _result = mysql_store_result(db);
    return _result;
}

bool
MySQL::connect(const char* host, const char* dbname, const char* user, const char* passwd)
{
//    GNASH_REPORT_FUNCTION;

    // Closes a previously opened connection &
    // also deallocates the connection handle
    disconnect();
    
    if ((_db = mysql_init(NULL)) == NULL ) {
	log_error(_("Couldn't initialize database"));
	return false;
    }
    
    if (mysql_real_connect(_db, host, user, passwd, dbname, 0, NULL, 0) == NULL) {
	log_error(_("Couldn't connect to database"));
	return false;
    }
    
    return true;
}

bool
MySQL::guery(const char *sql)
{
//    GNASH_REPORT_FUNCTION;
    if (_db) {
	return guery(_db, sql);
    }
    return -1;
}

bool
MySQL::guery(MYSQL *db, const char *sql)
{
//    GNASH_REPORT_FUNCTION;
    int res = mysql_real_query(db, sql, strlen(sql));
    switch (res) {
      case CR_SERVER_LOST:
      case CR_COMMANDS_OUT_OF_SYNC:
      case CR_SERVER_GONE_ERROR:
	  log_error (_("MySQL connection error: %s"), mysql_error(_db));
	  // Try to reconnect to the database
// 	  closeDB();
// 	  openDB();
	  break;
      case -1:
      case CR_UNKNOWN_ERROR:
	  log_error (_("MySQL error on query for:\n\t%s\nQuery was: %s"),
		     mysql_error(_db), sql); 
	  return false;
	  break;            
       default:
 	  return true;
    } 
    return false;
}

int
MySQL::getData(const char *sql, query_t &qresult)
{
//    GNASH_REPORT_FUNCTION;

    bool qstatus = false;
    int res = mysql_real_query(_db, sql, strlen(sql));
    switch (res) {
      case CR_SERVER_LOST:
      case CR_COMMANDS_OUT_OF_SYNC:
      case CR_SERVER_GONE_ERROR:
	  log_error(_("MySQL connection error: %s"), mysql_error(_db));
	  // Try to reconnect to the database
// 	  closeDB();
// 	  openDB();
	  break;
      case -1:
      case CR_UNKNOWN_ERROR:
	  log_error (_("MySQL error on query for:\n\t%s\nQuery was: %s"),
		     mysql_error(_db), sql); 
//	  return false;
	  break;            
//       default:
// 	  return true;
    }    

    _result = mysql_store_result(_db);
//    int nrows = mysql_num_rows(result);

#if 0
    for (size_t i=0; i<mysql_num_fields(_result); i++) {
	MYSQL_FIELD *fields = mysql_fetch_fields(_result);
	log_debug(_("Field name is: %s: "), fields->name);
    }
#endif
    
    while((_row = mysql_fetch_row(_result))) {
	vector<const char *> row_vec;
	for (size_t i=0; i<mysql_num_fields(_result); i++) {
//	    log_debug("Column[%d] is: \"%s\"", i, row[i]);
	    row_vec.push_back(_row[i]);
        }
	qresult.push_back(row_vec);
	qstatus = true;
    }

    mysql_free_result(_result);
    return(qstatus);
}

bool
MySQL::disconnect()
{
//    GNASH_REPORT_FUNCTION;
    if (_db != NULL) {
        mysql_close(_db);    
        _db = NULL;
    }
    return true;
}


// Entry points for ActionScript methods
as_value
mysql_connect(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;

    boost::intrusive_ptr<mysql_as_object> ptr = ensureType<mysql_as_object>(fn.this_ptr);

    if (fn.nargs == 4) {
	string host = fn.arg(0).to_string();
	string db = fn.arg(1).to_string();
	string user = fn.arg(2).to_string();
	string passwd = fn.arg(3).to_string();	
	return as_value(ptr->obj.connect(host.c_str(), db.c_str(),
					 user.c_str(), passwd.c_str()));
    } else {
	return as_value(false);
    }
}

as_value
mysql_qetData(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;

    boost::intrusive_ptr<mysql_as_object> ptr = ensureType<mysql_as_object>(fn.this_ptr);

    if (fn.nargs > 0) {
	string sql = fn.arg(0).to_string();
	Array_as *arr = (Array_as *)fn.arg(1).to_object().get();
//	std::vector< std::vector<const char *> >
	MySQL::query_t qresult;
#if 0
	// This clearly makes no sense...
	return as_value(ptr->obj.getData(sql, qresult));
#endif
	for (size_t i=0; i<qresult.size(); i++) {
	    vector<const char *> row;
	    row = qresult[i];
	    for (size_t j=0; j< row.size(); j++) {
//		cerr << "ARR: " << i << ":" << j << " " << row[j] << endl;
		as_value entry = row[j];
		arr->push(entry);
	    }
	}
 	return as_value(true);
    }
    log_aserror("Mysql.getData(): missing arguments");
 	return as_value(false);
}

as_value
mysql_free(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<mysql_as_object> ptr = ensureType<mysql_as_object>(fn.this_ptr);
    ptr->obj.free_result();
    return as_value(true);
}

as_value
mysql_fields(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<mysql_as_object> ptr = ensureType<mysql_as_object>(fn.this_ptr);
    return as_value(ptr->obj.num_fields());
}

as_value
mysql_fetch(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    if (fn.nargs > 0) {
	boost::intrusive_ptr<mysql_as_object> ptr = ensureType<mysql_as_object>(fn.this_ptr);
	assert(ptr);
	MYSQL_ROW res = ptr->obj.fetch_row();
	as_value aaa = *res;       
	Array_as *arr = new Array_as;
	arr->push(aaa);
	return as_value(arr);
    }
    log_aserror("Mysql.fetch(): missing arguments");
    return as_value();
}

as_value
mysql_store(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<mysql_as_object> ptr = ensureType<mysql_as_object>(fn.this_ptr);
    return as_value(ptr->obj.store_result());
}

as_value
mysql_query(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<mysql_as_object> ptr = ensureType<mysql_as_object>(fn.this_ptr);
    if (fn.nargs > 0) {
	string sql = fn.arg(0).to_string();
	return as_value(ptr->obj.guery(sql.c_str()));
    }
    log_aserror("Missing arguments to MySQL.query");
    return as_value();
}

as_value
mysql_disconnect(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;

    boost::intrusive_ptr<mysql_as_object> ptr = ensureType<mysql_as_object>(fn.this_ptr);
    return as_value(ptr->obj.disconnect());
}

extern "C" {
    void
    mysql_class_init(as_object &obj)
    {
//	GNASH_REPORT_FUNCTION;
	// This is going to be the global "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;
	if (cl == NULL) {
	    cl = new builtin_function(&mysql_ctor, getInterface());
// 	    // replicate all interface to class, to be able to access
// 	    // all methods as static functions
 	    attachInterface(cl.get());
	}	
	obj.init_member("MySQL", cl.get());
    }
    
} // end of extern C

} // end of gnash namespace


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
