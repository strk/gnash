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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <map>
#include <iostream>
#include <string>
#include <cstdio>
#include <boost/algorithm/string/case_conv.hpp>

#include "VM.h"
#include "log.h"
#include "fn_call.h"
#include "as_object.h"
#include "builtin_function.h" // need builtin_function
#include "fileio.h"


using namespace std;

namespace gnash
{

static const int BUFSIZE = 1024;

void fileio_fopen(const fn_call& fn);
void fileio_fread(const fn_call& fn);
void fileio_fgetc(const fn_call& fn);
void fileio_fgets(const fn_call& fn);
void fileio_gets(const fn_call& fn);
void fileio_fread(const fn_call& fn);
void fileio_fwrite(const fn_call& fn);
void fileio_fputc(const fn_call& fn);
void fileio_fputs(const fn_call& fn);
void fileio_puts(const fn_call& fn);
void fileio_fclose(const fn_call& fn);
void fileio_getchar(const fn_call& fn);
void fileio_putchar(const fn_call& fn);
void fileio_fflush(const fn_call& fn);
void fileio_ftell(const fn_call& fn);
void fileio_fseek(const fn_call& fn);

LogFile& dbglogfile = LogFile::getDefaultInstance();

static void
attachInterface(as_object *obj)
{
//    GNASH_REPORT_FUNCTION;

    obj->set_member("fopen", &fileio_fopen);
    obj->set_member("fread", &fileio_fread);
    obj->set_member("fgetc", &fileio_fgetc);
    obj->set_member("fgets", &fileio_fgets);
    obj->set_member("gets", &fileio_fgets);
    obj->set_member("getchar", &fileio_getchar);

    obj->set_member("fwrite", &fileio_fwrite);
    obj->set_member("fputc", &fileio_fputc);
    obj->set_member("fputs", &fileio_fputs);
    obj->set_member("puts", &fileio_puts);
    obj->set_member("putchar", &fileio_putchar);
    
    obj->set_member("fflush", &fileio_fflush);
    obj->set_member("fseek", &fileio_fseek);
    obj->set_member("ftell", &fileio_ftell);
    obj->set_member("fclose", &fileio_fclose);
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

static void
fileio_ctor(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    Fileio * obj = new Fileio();

    attachInterface(obj);
    fn.result->set_as_object(obj); // will keep alive
}


Fileio::Fileio()
    : _stream(0)
{
//    GNASH_REPORT_FUNCTION;
}

Fileio::~Fileio()
{
//    GNASH_REPORT_FUNCTION;
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
        ::fseek(_stream, offset, SEEK_SET);
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
Fileio::fopen(string &filespec, string &mode)
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
        ret = ::fread(buf, BUFSIZE, 1, _stream);
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
        str = ::fgets(buf, BUFSIZE, _stream);
        return str;
    }
    return str;
}

int
Fileio::fwrite(string &str)
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
Fileio::fputs(string &str)
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

void
fileio_fopen(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    Fileio *ptr = (Fileio *)fn.this_ptr;
    assert(ptr);
    
    if (fn.nargs > 0) {
	string filespec = fn.env->bottom(fn.first_arg_bottom_index).to_string();
	string mode = fn.env->bottom(fn.first_arg_bottom_index-1).to_string();
	fn.result->set_bool(ptr->fopen(filespec, mode));
    }
}

void
fileio_fclose(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    Fileio *ptr = (Fileio *)fn.this_ptr;
    assert(ptr);
    
    fn.result->set_bool(ptr->fclose());
}

void
fileio_fread(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    Fileio *ptr = (Fileio*)fn.this_ptr;
    assert(ptr);    
}

void
fileio_fgetc(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    Fileio *ptr = (Fileio*)fn.this_ptr;
    assert(ptr);
    int i = ptr->fgetc();
    char *c = reinterpret_cast<char *>(&i);
    fn.result->set_string(c);
}

void
fileio_fgets(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    Fileio *ptr = (Fileio*)fn.this_ptr;
    assert(ptr);
    string str = ptr->fgets(str);
    fn.result->set_string(str.c_str());
}

void
fileio_gets(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    Fileio *ptr = (Fileio*)fn.this_ptr;
    assert(ptr);    
    char buf[BUFSIZE];
    memset(buf, 0, BUFSIZE);
    string str = ::gets(buf);
    fn.result->set_string(buf);
}

// Read a single character from standard in
void
fileio_getchar(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    Fileio *ptr = (Fileio*)fn.this_ptr;
    assert(ptr);
    int i = ::getchar();
    char *c = reinterpret_cast<char *>(&i);
    fn.result->set_string(c);
}

void
fileio_fwrite(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    Fileio *ptr = (Fileio*)fn.this_ptr;
    assert(ptr);
    string str = fn.env->bottom(fn.first_arg_bottom_index).to_string();
    fn.result->set_int(ptr->fputs(str));
}

void
fileio_fputc(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    Fileio *ptr = (Fileio*)fn.this_ptr;
    assert(ptr);    
    int c = fn.env->bottom(fn.first_arg_bottom_index).to_number();
    fn.result->set_bool(ptr->fputc(c));
}

void
fileio_fputs(const fn_call& fn)
{
    //   GNASH_REPORT_FUNCTION;
    Fileio *ptr = (Fileio*)fn.this_ptr;
    assert(ptr);    
    string str = fn.env->bottom(fn.first_arg_bottom_index).to_string();
    fn.result->set_bool(ptr->fputs(str));
}

// print to standard put
void
fileio_puts(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    string str = fn.env->bottom(fn.first_arg_bottom_index).to_string();
    fn.result->set_int(::puts(str.c_str()));
}

void
fileio_putchar(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    Fileio *ptr = (Fileio*)fn.this_ptr;
    assert(ptr);    
    string x = fn.env->bottom(fn.first_arg_bottom_index).to_string();
    fn.result->set_int(::putchar(x[0]));
}

void
fileio_fflush(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    Fileio *ptr = (Fileio*)fn.this_ptr;
    assert(ptr);    
    fn.result->set_int(ptr->fflush());
}

void
fileio_fseek(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    Fileio *ptr = (Fileio*)fn.this_ptr;
    assert(ptr);    
    long c = fn.env->bottom(fn.first_arg_bottom_index).to_number();
    fn.result->set_int(ptr->fseek(c));
}

void
fileio_ftell(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    Fileio *ptr = (Fileio*)fn.this_ptr;
    assert(ptr);
    int i = ptr->ftell();
    fn.result->set_int(i);
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
	    cl = new builtin_function(&fileio_ctor, getInterface());
// 	    // replicate all interface to class, to be able to access
// 	    // all methods as static functions
 	    attachInterface(cl.get());
	}
	
	VM& vm = VM::get(); // cache this ?
	std::string name = "FileIO";
	if ( vm.getSWFVersion() < 7 ) {
	    boost::to_lower(name, vm.getLocale());
	}
	obj.init_member(name, cl.get());
    }
} // end of extern C


} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
