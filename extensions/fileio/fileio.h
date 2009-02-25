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

#ifndef __FILEIO_PLUGIN_H__
#define __FILEIO_PLUGIN_H__

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <memory> // for auto_ptr
#include "as_object.h"

#include <cstdio>
#include <string>

namespace gnash
{

// TODO: Document this class !!
class Fileio : public as_object {
public:
    Fileio();
    ~Fileio();

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
    void scandir(const std::string& dir, as_value* result);    
private:
    FILE        *_stream;
    std::string _filespec;
};

extern "C" {
    void fileio_class_init(as_object &obj);  
    /// Return an  instance
}

std::auto_ptr<as_object> init_fileio_instance();

} // end of gnash namespace

// __FILEIO_PLUGIN_H__
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
