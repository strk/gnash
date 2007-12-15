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

#ifndef _SOL_H_
#define _SOL_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>
#include <vector>
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
    bool extractHeader(std::vector<unsigned char> &data);
    bool extractHeader(std::string &filespec);

    // Create the header
    bool formatHeader(std::vector<unsigned char> &data);
    bool formatHeader(std::string &name);

    // write the data to disk as a .sol file
    bool writeFile(std::string &filespec);
    
    // read the .sol file from disk
    bool readFile(std::string &filespec);

    // Add the AMF objects that are the data of the file
    void addObj(AMF::amf_element_t &x) { _amfobjs.push_back(x); };
    std::vector<AMF::amf_element_t> getElements() { return _amfobjs; };
    AMF::amf_element_t getElement(int x) { return _amfobjs[x]; };

    void dump();
 private:
    std::vector<unsigned char> _data;
    std::string      _objname;
    std::string      _filespec;
    std::vector<AMF::amf_element_t> _amfobjs;
    int              _filesize;
  };

 
} // end of amf namespace

// end of _SOL_H_
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
