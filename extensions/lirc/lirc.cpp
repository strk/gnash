// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include <string>
#include "log.h"
#include "lirc.h"
#include "lirc/lirc_client.h"
#include "fn_call.h"
#include "as_object.h"
#include "builtin_function.h" // need builtin_function

using namespace std;

namespace gnash
{
  struct lirc_config *config;

  as_value lirc_ext_init(const fn_call& fn);
  as_value lirc_ext_deinit(const fn_call& fn);
  as_value lirc_ext_readconfig(const fn_call& fn);
  as_value lirc_ext_freeconfig(const fn_call& fn);
  as_value lirc_ext_nextcode(const fn_call& fn);
  as_value lirc_ext_code2char(const fn_call& fn);
  as_value lirc_ext_readconfig_only(const fn_call& fn);
  as_value lirc_ext_code2charprog(const fn_call& fn);
  as_value lirc_ext_getsocketname(const fn_call& fn);
  as_value lirc_ext_getmode(const fn_call& fn);
  as_value lirc_ext_setmode(const fn_call& fn);

class lirc_as_object : public as_object
{
public:
    Lirc obj;
};

static void
attachInterface(as_object *obj)
{
    GNASH_REPORT_FUNCTION;
    obj->init_member("lirc_init", new builtin_function(lirc_ext_init));
    obj->init_member("lirc_deinit", new builtin_function(lirc_ext_deinit));
    obj->init_member("lirc_readconfig", new builtin_function(lirc_ext_readconfig));
    obj->init_member("lirc_freeconfig", new builtin_function(lirc_ext_freeconfig));
    obj->init_member("lirc_nextcode", new builtin_function(lirc_ext_nextcode));
    obj->init_member("lirc_code2char", new builtin_function(lirc_ext_code2char));
    obj->init_member("lirc_readconfig_only", new builtin_function(lirc_ext_readconfig_only));
    obj->init_member("lirc_code2charprog", new builtin_function(lirc_ext_code2charprog));
    obj->init_member("lirc_getsocketname", new builtin_function(lirc_ext_getsocketname));
    obj->init_member("lirc_getmode", new builtin_function(lirc_ext_getmode));
    obj->init_member("lirc_setmode", new builtin_function(lirc_ext_setmode));
}

static as_object*
getInterface()
{
    GNASH_REPORT_FUNCTION;
    static boost::intrusive_ptr<as_object> o;
    if (o == NULL) {
	o = new as_object();
    }
    return o.get();
}

static as_value
lirc_ctor(const fn_call& /* fn */)
{
    GNASH_REPORT_FUNCTION;
    lirc_as_object* obj = new lirc_as_object();

    attachInterface(obj);
    return as_value(obj); // will keep alive
//    printf ("Hello World from %s !!!\n", __PRETTY_FUNCTION__);
}


Lirc::Lirc() 
{
    GNASH_REPORT_FUNCTION;
}

Lirc::~Lirc()
{
    GNASH_REPORT_FUNCTION;
}

//  int lirc_init(char *prog,int verbose);
as_value
lirc_ext_init(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<lirc_as_object> ptr = ensureType<lirc_as_object>(fn.this_ptr);
    
    if (fn.nargs > 0) {
	string text = fn.arg(0).to_string();
	int num = fn.arg(1).to_number<int>();
 	return as_value(lirc_init(const_cast<char *>(text.c_str()), num));
    }
    return as_value(false);
}

// int lirc_deinit(void);
as_value
lirc_ext_deinit(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<lirc_as_object> ptr = ensureType<lirc_as_object>(fn.this_ptr);
    
    if (fn.nargs > 0) {
	string text = fn.arg(0).to_string();
	return as_value(lirc_deinit());
    }
    return as_value(false);
}

// int lirc_readconfig(char *file,struct lirc_config **config, int (check)(char *s));
as_value
lirc_ext_readconfig(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<lirc_as_object> ptr = ensureType<lirc_as_object>(fn.this_ptr);
    
    if (fn.nargs > 0) {
      string text = fn.arg(0).to_string();
      config = dynamic_cast<lirc_config *>(fn.arg(1).to_object().get());
      char *code = const_cast<char *>(text.c_str());
      //      char *check = const_cast<char *>(fn.arg(2).to_string().c_str());
      //      return as_value(lirc_readconfig(const_cast<char *>(text.c_str(), &config, check)));
    }
    return as_value(false);
}

// void lirc_freeconfig(struct lirc_config *config);
as_value lirc_ext_freeconfig(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<lirc_as_object> ptr = ensureType<lirc_as_object>(fn.this_ptr);
    
    if (fn.nargs > 0) {
      config = dynamic_cast<lirc_config *>(fn.arg(0).to_object().get());
      lirc_freeconfig(config);
      return as_value(true);
    }
    return as_value(false);
}

// int lirc_nextcode(char **code);
as_value lirc_ext_nextcode(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<lirc_as_object> ptr = ensureType<lirc_as_object>(fn.this_ptr);
    
    if (fn.nargs > 0) {
	string text = fn.arg(0).to_string();
	char *code = const_cast<char *>(text.c_str());
	return as_value(lirc_nextcode(&code));
    }
    return as_value(false);
}

// int lirc_code2char(struct lirc_config *config,char *code,char **string);
as_value lirc_ext_code2char(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<lirc_as_object> ptr = ensureType<lirc_as_object>(fn.this_ptr);
    
    if (fn.nargs > 0) {
      config = dynamic_cast<lirc_config *>(fn.arg(0).to_object().get());
      char *code = const_cast<char *>(fn.arg(1).to_string().c_str());
      char *str = const_cast<char *>(fn.arg(2).to_string().c_str());
      return as_value(lirc_code2char(config, code, &str));
    }
    return as_value(false);
}

// int lirc_readconfig_only(char *file,struct lirc_config **config,
//                          int (check)(char *s));
as_value
lirc_ext_readconfig_only(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<lirc_as_object> ptr = ensureType<lirc_as_object>(fn.this_ptr);
    
    if (fn.nargs > 0) {
      char *file = const_cast<char *>(fn.arg(0).to_string().c_str());
      config = dynamic_cast<lirc_config *>(fn.arg(1).to_object().get());
      char *check = const_cast<char *>(fn.arg(2).to_string().c_str());
      //      return as_value(lirc_readconfig_only(file, &config, check));
    }
    return as_value(false);
}

// int lirc_code2charprog(struct lirc_config *config,char *code,char **string,
//                        char **prog);
as_value
lirc_ext_code2charprog(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<lirc_as_object> ptr = ensureType<lirc_as_object>(fn.this_ptr);
    
    if (fn.nargs > 0) {
	config = dynamic_cast<lirc_config *>(fn.arg(0).to_object().get());
	char *code = const_cast<char *>(fn.arg(1).to_string().c_str());
	char *str = const_cast<char *>(fn.arg(2).to_string().c_str());
	char *prog = const_cast<char *>(fn.arg(3).to_string().c_str());
	return as_value(lirc_code2charprog(config, code, &str, &prog));
    }
    return as_value(false);
}

// size_t lirc_getsocketname(const char *filename, char *buf, size_t size);
as_value
lirc_ext_getsocketname(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<lirc_as_object> ptr = ensureType<lirc_as_object>(fn.this_ptr);
    
    if (fn.nargs > 0) {
      const char *filename = fn.arg(0).to_string().c_str();
      char *buf = const_cast<char *>(fn.arg(1).to_string().c_str());
      size_t size = fn.arg(2).to_number<int>();
      lirc_getsocketname(filename, buf, size);
    }
    return as_value(false);
}

// const char *lirc_getmode(struct lirc_config *config);
as_value
lirc_ext_getmode(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<lirc_as_object> ptr = ensureType<lirc_as_object>(fn.this_ptr);
    
    if (fn.nargs > 0) {
	config = dynamic_cast<lirc_config *>(fn.arg(0).to_object().get());
	return as_value(lirc_getmode(config));
    }
    return as_value(false);
}

// const char *lirc_setmode(struct lirc_config *config, const char *mode);
as_value
lirc_ext_setmode(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<lirc_as_object> ptr = ensureType<lirc_as_object>(fn.this_ptr);
    
    if (fn.nargs > 0) {
	config = dynamic_cast<lirc_config *>(fn.arg(0).to_object().get());
	char *mode = const_cast<char *>(fn.arg(1).to_string().c_str());
	return as_value(lirc_setmode(config, mode));
    }
    return as_value(false);
}

std::auto_ptr<as_object>
init_lirc_instance()
{
    return std::auto_ptr<as_object>(new lirc_as_object());
}

// const char *lirc_setmode(struct lirc_config *config, const char *mode);
extern "C" {
    void
    lirc_class_init(as_object &obj)
    {
//	GNASH_REPORT_FUNCTION;
	// This is going to be the global "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;
	if (cl == NULL) {
	    cl = new builtin_function(&lirc_ctor, getInterface());
// 	    // replicate all interface to class, to be able to access
// 	    // all methods as static functions
 	    attachInterface(cl.get());
	}
	
	obj.init_member("Lirc", cl.get());
    }
} // end of extern C


} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
