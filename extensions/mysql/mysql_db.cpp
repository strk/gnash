// mysql_db.cpp:  MySQL database interface ActionScript objects, for Gnash.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "mysql_db.h"

#include "namedStrings.h"
#include <errmsg.h>
#include <mysql.h>
#include <vector>

#include "log.h"
#include "Array_as.h"
#include "as_value.h"
#include "fn_call.h"
#include "Global_as.h"
#include "as_function.h"

namespace gnash {

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

static void
attachInterface(as_object& obj)
{
    Global_as& gl = getGlobal(obj);

    obj.init_member("connect", gl.createFunction(mysql_connect));
    obj.init_member("qetData", gl.createFunction(mysql_qetData));
    obj.init_member("disconnect", gl.createFunction(mysql_disconnect));
    obj.init_member("query", gl.createFunction(mysql_query));
    obj.init_member("fetch_row", gl.createFunction(mysql_fetch));
    obj.init_member("num_fields", gl.createFunction(mysql_fields));
    obj.init_member("free_result", gl.createFunction(mysql_free));
    obj.init_member("store_results", gl.createFunction(mysql_store));
}

class MySQL : public Relay
{
public:
    typedef std::vector< std::vector<const char *> > query_t;
    MySQL();
    ~MySQL();
    bool connect(const char *host, const char *dbname, const char *user, const char *passwd);
    int getData(const char *sql, query_t &result);
    bool disconnect();

    // These are wrappers for the regular MySQL API
    bool guery(MYSQL *db, const char *sql);
    bool guery(const char *sql);
    int num_fields();
    int num_fields(MYSQL_RES *result);
    MYSQL_ROW fetch_row();
    MYSQL_ROW fetch_row(MYSQL_RES *result);
    void free_result();
    void free_result(MYSQL_RES *result);
    MYSQL_RES *store_result();
    MYSQL_RES *store_result(MYSQL *db);
private:    
    MYSQL *_db;
    MYSQL_RES *_result;
    MYSQL_ROW _row;
};

static as_value
mysql_ctor(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);
    obj->setRelay(new MySQL());
    return as_value(); 
}


MySQL::MySQL() :
    _db(NULL),
    _result(NULL),
    _row(NULL)
{
}

MySQL::~MySQL()
{
    disconnect();
}

int
MySQL::num_fields()
{
    if (_result) {
        return num_fields(_result);
    }
    return -1;
}

int
MySQL::num_fields(MYSQL_RES *result)
{
    return mysql_num_fields(result);
}

MYSQL_ROW
MySQL::fetch_row()
{
    if (_result) {
        return fetch_row(_result);
    }
    return NULL;
}

MYSQL_ROW
MySQL::fetch_row(MYSQL_RES *result)
{
    return mysql_fetch_row(result);
}

void
MySQL::free_result()
{
    if (_result) {
        free_result(_result);
    }
}

void
MySQL::free_result(MYSQL_RES *result)
{
    mysql_free_result(result);
}

MYSQL_RES *
MySQL::store_result()
{
    if (_db) {
        return store_result(_db);
    }
    return NULL;
}

MYSQL_RES *
MySQL::store_result(MYSQL *db)
{
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
	std::vector<const char *> row_vec;
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

    MySQL* ptr = ensure<ThisIsNative<MySQL> >(fn);

    if (fn.nargs == 4) {
        std::string host = fn.arg(0).to_string();
        std::string db = fn.arg(1).to_string();
        std::string user = fn.arg(2).to_string();
        std::string passwd = fn.arg(3).to_string();	
        return as_value(ptr->connect(host.c_str(), db.c_str(),
                         user.c_str(), passwd.c_str()));
    } 

    return as_value(false);
}

as_value
mysql_qetData(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;

    if (fn.nargs > 0) {
        std::string sql = fn.arg(0).to_string();
	    as_object* arr = toObject(fn.arg(1), getVM(fn));

        MySQL::query_t qresult;

        for (size_t i=0; i<qresult.size(); i++) {
            std::vector<const char *> row;
            row = qresult[i];
            for (size_t j=0; j< row.size(); j++) {
                as_value entry = row[j];
                callMethod(arr, NSV::PROP_PUSH, entry);
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
    MySQL* ptr = ensure<ThisIsNative<MySQL> >(fn);
    ptr->free_result();
    return as_value(true);
}

as_value
mysql_fields(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    MySQL* ptr = ensure<ThisIsNative<MySQL> >(fn);
    return as_value(ptr->num_fields());
}

as_value
mysql_fetch(const fn_call& fn)
{
    MySQL* ptr = ensure<ThisIsNative<MySQL> >(fn);
    if (fn.nargs > 0) {
        MYSQL_ROW res = ptr->fetch_row();
        as_value aaa = *res;       
        Global_as& gl = getGlobal(fn);
        as_object* arr = gl.createArray();
        callMethod(arr, NSV::PROP_PUSH, aaa);
        return as_value(arr);
    }
    log_aserror("Mysql.fetch(): missing arguments");
    return as_value();
}

as_value
mysql_store(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    MySQL* ptr = ensure<ThisIsNative<MySQL> >(fn);
    as_object* obj = reinterpret_cast<as_object*>(ptr->store_result());
    return as_value(obj);
}

as_value
mysql_query(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    MySQL* ptr = ensure<ThisIsNative<MySQL> >(fn);
    if (fn.nargs > 0) {
        std::string sql = fn.arg(0).to_string();
        return as_value(ptr->guery(sql.c_str()));
    }
    log_aserror("Missing arguments to MySQL.query");
    return as_value();
}

as_value
mysql_disconnect(const fn_call& fn)
{
    MySQL* ptr = ensure<ThisIsNative<MySQL> >(fn);
    return as_value(ptr->disconnect());
}

extern "C" {

void mysql_class_init(as_object &obj)
{
    Global_as& gl = getGlobal(obj);
    as_object* proto = createObject(gl);
	as_object *cl = gl.createClass(&mysql_ctor, proto);
    attachInterface(*proto);
	obj.init_member("MySQL", cl);
} 

}

} // end of gnash namespace


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
