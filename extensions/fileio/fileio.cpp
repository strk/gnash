// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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
#include "Global_as.h"
#include "fileio.h"
#include "Array_as.h"  // used by scandir()
#include "as_function.h"

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

// TODO: Document this class !!
class FileIO : public Relay
{
public:
    FileIO();
    ~FileIO();

    bool fopen(const std::string &filespec, const std::string &mode);

    int fread(std::string &str);
    int fgetc();
    std::string &fgets(std::string &str);
    
    int fwrite(const std::string &str);
    bool fputc(int c);
    bool fputs(const std::string &str);
    int fclose();
    int fflush();
    void rewind();
    int fseek(long offset);
    int fseek(long offset, int whence);
    long ftell();
    bool asyncmode(bool async); 
    bool feof();
    bool unlink(const std::string &filespec);
private:
    FILE        *_stream;
    std::string _filespec;
};

static void
attachInterface(as_object& obj)
{
    Global_as& gl = getGlobal(obj);
    
    obj.init_member("fopen", gl.createFunction(fileio_fopen));
    obj.init_member("fread", gl.createFunction(fileio_fread));
    obj.init_member("fgetc", gl.createFunction(fileio_fgetc));
    obj.init_member("fgets", gl.createFunction(fileio_fgets));
    obj.init_member("gets", gl.createFunction(fileio_fgets));
    obj.init_member("getchar", gl.createFunction(fileio_getchar));

    obj.init_member("fwrite", gl.createFunction(fileio_fwrite));
    obj.init_member("fputc", gl.createFunction(fileio_fputc));
    obj.init_member("fputs", gl.createFunction(fileio_fputs));
    obj.init_member("puts", gl.createFunction(fileio_puts));
    obj.init_member("putchar", gl.createFunction(fileio_putchar));
    
    obj.init_member("fflush", gl.createFunction(fileio_fflush));
    obj.init_member("fseek", gl.createFunction(fileio_fseek));
    obj.init_member("ftell", gl.createFunction(fileio_ftell));
    obj.init_member("asyncmode", gl.createFunction(fileio_asyncmode));
    obj.init_member("feof", gl.createFunction(fileio_feof));
    obj.init_member("fclose", gl.createFunction(fileio_fclose));
    
    obj.init_member("unlink", gl.createFunction(fileio_unlink));
    
    obj.init_member("scandir", gl.createFunction(fileio_scandir));
}

static as_value
fileio_ctor(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);
    obj->setRelay(new FileIO());

    if (fn.nargs > 0) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::stringstream ss; fn.dump_args(ss);
            log_aserror("new FileIO(%s): all arguments discarded",
                        ss.str().c_str());
            );
    }

    return as_value();
}


FileIO::FileIO()
    :
    _stream(0)
{
}

FileIO::~FileIO()
{
//    GNASH_REPORT_FUNCTION;
    fclose();
}

int
FileIO::fflush()
{
//    GNASH_REPORT_FUNCTION;
    if (_stream) {
        return ::fflush(_stream);
    }
    return -1;
}

void
FileIO::rewind()
{
//    GNASH_REPORT_FUNCTION;
    if (_stream) {
        ::fseek(_stream, 0L, SEEK_SET);
    }
}

int
FileIO::fseek(long offset)
{
//    GNASH_REPORT_FUNCTION;
    if (_stream) {
        return ::fseek(_stream, offset, SEEK_SET);
    }
    return -1;
}

int
FileIO::fseek(long offset, int whence)
{
//    GNASH_REPORT_FUNCTION;
    if (_stream) {
        return ::fseek(_stream, offset, whence);
    }
    return -1;
}

long
FileIO::ftell()
{
//    GNASH_REPORT_FUNCTION;
    if (_stream) {
        return ::ftell(_stream);
    }
    return -1;
}

bool
FileIO::asyncmode(bool async)
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
FileIO::feof()
{
//    GNASH_REPORT_FUNCTION;
    if (_stream) {
        return ::feof(_stream);
    }
    return -1;
}

bool
FileIO::fopen(const string &filespec, const string &mode)
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
FileIO::fread(string &str)
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
FileIO::fgetc()
{
//    GNASH_REPORT_FUNCTION;
    if (_stream) {
        return ::fgetc(_stream);
    }
    return -1;
}

string &
FileIO::fgets(std::string &str)
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
FileIO::fwrite(const string &str)
{
//    GNASH_REPORT_FUNCTION;
    return ::fwrite(str.c_str(), str.size(), 1, _stream);
}


bool
FileIO::fputc(int c)
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
FileIO::fputs(const string &str)
{
//    GNASH_REPORT_FUNCTION;
    if (_stream) {
        if (::fputs(str.c_str(), _stream) != EOF) {
            return true;
        }
    }    
    return false;
}

int
FileIO::fclose()
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
FileIO::unlink(const std::string &filespec)
{
//    GNASH_REPORT_FUNCTION;
		return ::unlink(filespec.c_str()) >= 0;		
}


as_value
fileio_fopen(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    FileIO* ptr = ensure<ThisIsNative<FileIO> >(fn);
    assert(ptr);
    
    if (fn.nargs < 2) {
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
    FileIO* ptr = ensure<ThisIsNative<FileIO> >(fn);
    assert(ptr);
    
    return as_value(ptr->fclose());
}

as_value
fileio_fread(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    FileIO* ptr = ensure<ThisIsNative<FileIO> >(fn);
    assert(ptr);
		
    string str;
    int count = ptr->fread(str);
    
    if (count<0) {
	return as_value(false);
    } else {
	return as_value(str.c_str());
    }
}

as_value
fileio_fgetc(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    FileIO* ptr = ensure<ThisIsNative<FileIO> >(fn);
    assert(ptr);
    int i = ptr->fgetc();
    
    if ((i==EOF) || (i<0)) {
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
    FileIO* ptr = ensure<ThisIsNative<FileIO> >(fn);
    assert(ptr);
    string str; 
    str = ptr->fgets(str);
    return as_value(str.c_str());
}

as_value
fileio_gets(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    FileIO* ptr = ensure<ThisIsNative<FileIO> >(fn);
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
    FileIO* ptr = ensure<ThisIsNative<FileIO> >(fn);
    assert(ptr);
    int i = ::getchar();
    char *c = reinterpret_cast<char *>(&i);
    return as_value(c);
}

as_value
fileio_fwrite(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    FileIO* ptr = ensure<ThisIsNative<FileIO> >(fn);
    assert(ptr);
    string str = fn.arg(0).to_string();
    return as_value(ptr->fputs(str));
}

as_value
fileio_fputc(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    FileIO* ptr = ensure<ThisIsNative<FileIO> >(fn);
    assert(ptr);    
    int c = (int) toNumber(fn.arg(0), getVM(fn));
    return as_value(ptr->fputc(c));
}

as_value
fileio_fputs(const fn_call& fn)
{
    //   GNASH_REPORT_FUNCTION;
    FileIO* ptr = ensure<ThisIsNative<FileIO> >(fn);

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
    FileIO* ptr = ensure<ThisIsNative<FileIO> >(fn);
    assert(ptr);    
    string x = fn.arg(0).to_string();
    return as_value(::putchar(x[0]));
}

as_value
fileio_fflush(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    FileIO* ptr = ensure<ThisIsNative<FileIO> >(fn);
    assert(ptr);    
    return as_value(ptr->fflush());
}

as_value
fileio_fseek(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    FileIO* ptr = ensure<ThisIsNative<FileIO> >(fn);
    assert(ptr);    
    long c = static_cast<long>(toNumber(fn.arg(0), getVM(fn)));
    return as_value(ptr->fseek(c));
}

as_value
fileio_ftell(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    FileIO* ptr = ensure<ThisIsNative<FileIO> >(fn);
    assert(ptr);
    int i = ptr->ftell();
    return as_value(i);
}

as_value 
fileio_asyncmode(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    FileIO* ptr = ensure<ThisIsNative<FileIO> >(fn);
    assert(ptr);
    bool b = toBool(fn.arg(0), getVM(fn));
    return as_value(ptr->asyncmode(b));
}

as_value
fileio_feof(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    FileIO* ptr = ensure<ThisIsNative<FileIO> >(fn);
    assert(ptr);
    bool b = ptr->feof();
    return as_value(b);
}

as_value
fileio_unlink(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    FileIO* ptr = ensure<ThisIsNative<FileIO> >(fn);
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

    if (!fn.nargs) return as_value(false);

    const std::string& dir = fn.arg(0).to_string();
	
    struct dirent **namelist;
	
	const int n = ::scandir(dir.c_str(), &namelist, 0, alphasort);
	
	if (n < 0) {
	    return as_value(false);
	}
    
    Global_as& gl = getGlobal(fn);
    VM& vm = getVM(fn);
	as_object* array = gl.createArray();	
	
	for (int idx = 0; idx < n; ++idx) {
		array->set_member(arrayKey(vm, idx), namelist[idx]->d_name);
		free(namelist[idx]);
	}
	free(namelist);

    return as_value(array);
}

extern "C" {

void
fileio_class_init(as_object& where, const ObjectURI& /* uri */)
{
    //	GNASH_REPORT_FUNCTION;
    Global_as& gl = getGlobal(where);

    as_object* proto = createObject(gl);
    attachInterface(*proto);
    as_object* cl = gl.createClass(&fileio_ctor, proto);
    
    where.init_member("FileIO", cl);
}
} // end of extern C


} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
