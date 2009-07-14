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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <map>
#include <iostream>
#include <string>
#include <cstdio>
#include <boost/algorithm/string/case_conv.hpp>

#include <dirent.h> // used by scandir()
#include "GnashSystemIOHeaders.h" // used by unlink()
#include <fcntl.h>  // used by asyncmode()

//#include "VM.h"
#include "log.h"
#include "fn_call.h"
#include "as_object.h"
#include "builtin_function.h" // need builtin_function
#include "fileio.h"
#include "Array_as.h"  // used by scandir()

using namespace std;

namespace gnash
{

static const int BUFSIZE = 1024;

as_value fileio_fopen(const fn_call& fn);
as_value fileio_fread(const fn_call& fn);
as_value fileio_fgetc(const fn_call& fn);
as_value fileio_fgets(const fn_call& fn);
as_value fileio_gets(const fn_call& fn);
as_value fileio_fwrite(const fn_call& fn);
as_value fileio_fputc(const fn_call& fn);
as_value fileio_fputs(const fn_call& fn);
as_value fileio_puts(const fn_call& fn);
as_value fileio_fclose(const fn_call& fn);
as_value fileio_getchar(const fn_call& fn);
as_value fileio_putchar(const fn_call& fn);
as_value fileio_fflush(const fn_call& fn);
as_value fileio_ftell(const fn_call& fn);
as_value fileio_feof(const fn_call& fn);
as_value fileio_fseek(const fn_call& fn);
as_value fileio_unlink(const fn_call& fn);
as_value fileio_asyncmode(const fn_call& fn);

// <Udo> I needed a scandir() function and implemented it here for simplicity.
// Maybe this should be moved to a dedicated extension and a different class? 
// The scandir() syntax comes from PHP, since the C syntax is not quite 
// applicable in ActionScript.
// Same applies for unlink(). Maybe a class FileOP or sim. would be 
// appriopriate. 
as_value fileio_scandir(const fn_call& fn);

LogFile& dbglogfile = LogFile::getDefaultInstance();

static void
attachInterface(as_object& obj)
{
//    GNASH_REPORT_FUNCTION;

    obj.init_member("fopen", gl->createFunction(fileio_fopen));
    obj.init_member("fread", gl->createFunction(fileio_fread));
    obj.init_member("fgetc", gl->createFunction(fileio_fgetc));
    obj.init_member("fgets", gl->createFunction(fileio_fgets));
    obj.init_member("gets", gl->createFunction(fileio_fgets));
    obj.init_member("getchar", gl->createFunction(fileio_getchar));

    obj.init_member("fwrite", gl->createFunction(fileio_fwrite));
    obj.init_member("fputc", gl->createFunction(fileio_fputc));
    obj.init_member("fputs", gl->createFunction(fileio_fputs));
    obj.init_member("puts", gl->createFunction(fileio_puts));
    obj.init_member("putchar", gl->createFunction(fileio_putchar));
    
    obj.init_member("fflush", gl->createFunction(fileio_fflush));
    obj.init_member("fseek", gl->createFunction(fileio_fseek));
    obj.init_member("ftell", gl->createFunction(fileio_ftell));
    obj.init_member("asyncmode", gl->createFunction(fileio_asyncmode));
    obj.init_member("feof", gl->createFunction(fileio_feof));
    obj.init_member("fclose", gl->createFunction(fileio_fclose));
    
    obj.init_member("unlink", gl->createFunction(fileio_unlink));
    
    obj.init_member("scandir", gl->createFunction(fileio_scandir));
}

static as_object*
getInterface()
{
//    GNASH_REPORT_FUNCTION;
    static boost::intrusive_ptr<as_object> o;
    if (o == NULL) {
	o = new as_object();
	attachInterface(*o);
    }
    return o.get();
}

static as_value
fileio_ctor(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    Fileio * obj = new Fileio();

    if ( fn.nargs > 0 )
    {
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss; fn.dump_args(ss);
		log_aserror("new FileIO(%s): all arguments discarded", ss.str().c_str());
		);
    }

    return as_value(obj); // will keep alive
}


Fileio::Fileio()
    :
    as_object(getInterface()),
    _stream(0)
{
//    GNASH_REPORT_FUNCTION;
}

Fileio::~Fileio()
{
//    GNASH_REPORT_FUNCTION;
    fclose();
}

int
Fileio::fflush()
{
//    GNASH_REPORT_FUNCTION;
    if (_stream) {
        return ::fflush(_stream);
    }
    return -1;
}

void
Fileio::rewind()
{
//    GNASH_REPORT_FUNCTION;
    if (_stream) {
        ::fseek(_stream, 0L, SEEK_SET);
    }
}

int
Fileio::fseek(long offset)
{
//    GNASH_REPORT_FUNCTION;
    if (_stream) {
        return ::fseek(_stream, offset, SEEK_SET);
    }
    return -1;
}

int
Fileio::fseek(long offset, int whence)
{
//    GNASH_REPORT_FUNCTION;
    if (_stream) {
        return ::fseek(_stream, offset, whence);
    }
    return -1;
}

long
Fileio::ftell()
{
//    GNASH_REPORT_FUNCTION;
    if (_stream) {
        return ::ftell(_stream);
    }
    return -1;
}

bool
Fileio::asyncmode(bool async)
{
//    GNASH_REPORT_FUNCTION;
    if (_stream) {
    
        int fd = fileno(_stream);
        
        long flags = fcntl(fd, F_GETFL);
        
        int res;
    
        if (async)
          res = fcntl(fd, F_SETFL, flags|O_NONBLOCK);
        else
          res = fcntl(fd, F_SETFL, flags&(~O_NONBLOCK));
          
        return res>=0;
    }
    return false;
}

bool
Fileio::feof()
{
//    GNASH_REPORT_FUNCTION;
    if (_stream) {
        return ::feof(_stream);
    }
    return -1;
}

bool
Fileio::fopen(const string &filespec, const string &mode)
{
//    GNASH_REPORT_FUNCTION;
    _stream = ::fopen(filespec.c_str(), mode.c_str());
    if (_stream) {
        return true;
    } else {
        return false;
    }
}


int
Fileio::fread(string &str)
{
//    GNASH_REPORT_FUNCTION;
    int ret = -1;
    if (_stream) {
        char buf[BUFSIZE];
        memset(buf, 0, BUFSIZE);    
        ret = ::fread(buf, 1, BUFSIZE, _stream);
        if (ret) {
            str = buf;
        }
    }
    return ret;
}

int
Fileio::fgetc()
{
//    GNASH_REPORT_FUNCTION;
    if (_stream) {
        return ::fgetc(_stream);
    }
    return -1;
}

string &
Fileio::fgets(std::string &str)
{
//    GNASH_REPORT_FUNCTION;
    if (_stream) {
        char buf[BUFSIZE];
        memset(buf, 0, BUFSIZE);
        char* res = ::fgets(buf, BUFSIZE, _stream);
        if (res) 
          str = res;
        else
          str = "";  // we might want to return NULL to the VM ?
        return str;
    }
    return str;
}

int
Fileio::fwrite(const string &str)
{
//    GNASH_REPORT_FUNCTION;
    return ::fwrite(str.c_str(), str.size(), 1, _stream);
}


bool
Fileio::fputc(int c)
{
//    GNASH_REPORT_FUNCTION;
    if (_stream) {
        if (::fputc(c, _stream)) {
            return true;
        }
    }
    return false;
}

bool
Fileio::fputs(const string &str)
{
//    GNASH_REPORT_FUNCTION;
    if (_stream) {
        if (::fputs(str.c_str(), _stream)) {
            return true;
        }
    }    
    return false;
}

int
Fileio::fclose()
{
//    GNASH_REPORT_FUNCTION;
    if (_stream) {
        int ret = ::fclose(_stream);
        _stream = 0;
        return ret;
    }
    return -1;
}

bool
Fileio::unlink(const std::string &filespec)
{
//    GNASH_REPORT_FUNCTION;
		return ::unlink(filespec.c_str()) >= 0;		
}

void
Fileio::scandir(const std::string& dir, as_value* result) 
{

	struct dirent **namelist;
	
	int n = ::scandir(dir.c_str(), &namelist, 0, alphasort);
	
	if (n<0) {
		result->set_bool(false);
		return;
	}
	
	Array_as* array = new Array_as();	
	as_value item;
	
	//array->resize(n);
	// TODO: Looks like I can't set an array item by index since
	// array::at() returns not a reference.
	
	for (int idx=0; idx<n; idx++) {
		item.set_string(namelist[idx]->d_name);
		array->push(item);
		free(namelist[idx]);
	}
	free(namelist);
	
	result->set_as_object(array);
}

as_value
fileio_fopen(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<Fileio> ptr = ensureType<Fileio>(fn.this_ptr);
    assert(ptr);
    
    if (fn.nargs < 2)
    {
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss; fn.dump_args(ss);
		log_aserror("FileIO.fopen(%s): need two arguments", ss.str().c_str());
		);
		return as_value(false);
    }

    string filespec = fn.arg(0).to_string();
    string mode = fn.arg(1).to_string();
    return as_value(ptr->fopen(filespec, mode));

}

as_value
fileio_fclose(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<Fileio> ptr = ensureType<Fileio>(fn.this_ptr);
    assert(ptr);
    
    return as_value(ptr->fclose());
}

as_value
fileio_fread(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<Fileio> ptr = ensureType<Fileio>(fn.this_ptr);
    assert(ptr);
		
		string str;
		int count = ptr->fread(str);

		if (count<0)
			return as_value(false);
		else
			return as_value(str.c_str());    
}

as_value
fileio_fgetc(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<Fileio> ptr = ensureType<Fileio>(fn.this_ptr);
    assert(ptr);
    int i = ptr->fgetc();
    
    if ((i==EOF) || (i<0)) 
    {
      return as_value(false);  // possible in async mode
    } else {
      char c[2]="x"; // set to 1 char to get the zero byte!
      c[0] = i;
      return as_value(c);
    }
}

as_value
fileio_fgets(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<Fileio> ptr = ensureType<Fileio>(fn.this_ptr);
    assert(ptr);
    string str; 
    str = ptr->fgets(str);
    return as_value(str.c_str());
}

as_value
fileio_gets(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<Fileio> ptr = ensureType<Fileio>(fn.this_ptr);
    assert(ptr);    
    char buf[BUFSIZE];
    memset(buf, 0, BUFSIZE);
    string str = ::gets(buf);
    return as_value(buf);
}

// Read a single character from standard in
as_value
fileio_getchar(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<Fileio> ptr = ensureType<Fileio>(fn.this_ptr);
    assert(ptr);
    int i = ::getchar();
    char *c = reinterpret_cast<char *>(&i);
    return as_value(c);
}

as_value
fileio_fwrite(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<Fileio> ptr = ensureType<Fileio>(fn.this_ptr);
    assert(ptr);
    string str = fn.arg(0).to_string();
    return as_value(ptr->fputs(str));
}

as_value
fileio_fputc(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<Fileio> ptr = ensureType<Fileio>(fn.this_ptr);
    assert(ptr);    
    int c = (int) fn.arg(0).to_number();
    return as_value(ptr->fputc(c));
}

as_value
fileio_fputs(const fn_call& fn)
{
    //   GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<Fileio> ptr = ensureType<Fileio>(fn.this_ptr);

    string str = fn.arg(0).to_string();
    return as_value(ptr->fputs(str));
}

// print to standard put
as_value
fileio_puts(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    string str = fn.arg(0).to_string();
    return as_value(::puts(str.c_str()));
}

as_value
fileio_putchar(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<Fileio> ptr = ensureType<Fileio>(fn.this_ptr);
    assert(ptr);    
    string x = fn.arg(0).to_string();
    return as_value(::putchar(x[0]));
}

as_value
fileio_fflush(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<Fileio> ptr = ensureType<Fileio>(fn.this_ptr);
    assert(ptr);    
    return as_value(ptr->fflush());
}

as_value
fileio_fseek(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<Fileio> ptr = ensureType<Fileio>(fn.this_ptr);
    assert(ptr);    
    long c = (long) fn.arg(0).to_number();
    return as_value(ptr->fseek(c));
}

as_value
fileio_ftell(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<Fileio> ptr = ensureType<Fileio>(fn.this_ptr);
    assert(ptr);
    int i = ptr->ftell();
    return as_value(i);
}

as_value 
fileio_asyncmode(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<Fileio> ptr = ensureType<Fileio>(fn.this_ptr);
    assert(ptr);
    bool b = (bool) fn.arg(0).to_bool();
    return as_value(ptr->asyncmode(b));
}

as_value
fileio_feof(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<Fileio> ptr = ensureType<Fileio>(fn.this_ptr);
    assert(ptr);
    bool b = ptr->feof();
    return as_value(b);
}

as_value
fileio_unlink(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<Fileio> ptr = ensureType<Fileio>(fn.this_ptr);
    assert(ptr);
    string str = fn.arg(0).to_string();
    return as_value(ptr->unlink(str));
}

as_value
fileio_scandir(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;

		// TODO: Check optional second parameter and sort array if it's true
		// or missing.
    boost::intrusive_ptr<Fileio> ptr = ensureType<Fileio>(fn.this_ptr);

    assert(ptr);    
    string str = fn.arg(0).to_string();
    as_value val;
    ptr->scandir(str, &val);
    return val;
}

std::auto_ptr<as_object>
init_fileio_instance()
{
    return std::auto_ptr<as_object>(new Fileio());
}

extern "C" {
    void
    fileio_class_init(as_object &obj)
    {
//	GNASH_REPORT_FUNCTION;
	// This is going to be the global "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;
	if (cl == NULL) {
        Global_as* gl = getGlobal(global);
        cl = gl->createClass(&fileio_ctor, getInterface());
// 	    // replicate all interface to class, to be able to access
// 	    // all methods as static functions
 	    //attachInterface(*cl);
	}
	
	obj.init_member("FileIO", cl.get());
    }
} // end of extern C


} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
