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

#ifndef _SOL_H_
#define _SOL_H_

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/cstdint.hpp>
#include <string>
#include <vector>
#include "element.h"
#include "amf.h"

// It comprises of a magic number, followed by the file length, a
// filetype, which appears to always be "TCSO", and what appears to be
// a marker at the end of the header block.
// After the SOL header, the rest is all AMF objects.

// Magic Number - 2 bytes (always 0x00bf)
// Length       - 4 bytes (the length of the file including the Marker bytes)
// Marker       - 10 bytes (always "TCSO0x000400000000")
// Object Name  - variable (the name of the object as an AMF encoded string)
// Padding      - 4 bytes
// After this is a series of AMF objects
namespace amf
{
class SOL {
public:
    SOL();
    ~SOL();
    size_t size() { return _amfobjs.size(); };
    
    // extract the header
    bool extractHeader(const std::vector<boost::uint8_t> &data);
    bool extractHeader(const std::string &filespec);

    // Create the header
    bool formatHeader(std::vector<boost::uint8_t> &data);
    bool formatHeader(std::string &name);
    bool formatHeader(std::string &name, int filesize);

    // write the data to disk as a .sol file
    bool writeFile();
    bool writeFile(std::string &filespec, std::string &objname);
    bool writeFile(std::string &filespec, const char *objname);
    bool writeFile(const char *filespec, const char *objname);
    
    // read the .sol file from disk
    bool readFile(std::string &filespec);
    
    std::vector<boost::uint8_t> getHeader() { return _header; };

    // Add the AMF objects that are the data of the file
    void addObj(amf::Element *x);
    std::vector<amf::Element *> getElements() { return _amfobjs; };
    Element *getElement(int x) { return _amfobjs[x]; };

    void dump();
//protected:
    void setFilespec(std::string &x) { _filespec = x; };
    std::string &getFilespec() { return _filespec; };

    void setObjectName(std::string &x) { _objname = x; };
    std::string &getObjectName() { return _objname; };
        
 private:
    std::vector<boost::uint8_t> _header;
    std::vector<boost::uint8_t> _data;
    std::string      _objname;
    std::string      _filespec;
    std::vector<amf::Element *> _amfobjs;
    int              _filesize;
  };

 
} // end of amf namespace

// end of _SOL_H_
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
