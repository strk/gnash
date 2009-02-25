// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifndef __MYSQL_DB_H__
#define __MYSQL_DB_H__

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <string>
#include <vector>
#include <mysql/errmsg.h>
#include <mysql/mysql.h>

#include "as_value.h"
#include "as_object.h"
#include "extension.h"

namespace gnash
{

class MySQL
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

extern "C" {
    void mysql_class_init(as_object &obj);  
}

/// Return an  instance
std::auto_ptr<as_object> init_mysql_instance();

}

// __MYSQL_DB_H__
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

