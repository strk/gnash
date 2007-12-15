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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <boost/cstdint.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "amf.h"
#include "sol.h"
#include "log.h"

#if defined(HAVE_WINSOCK_H) && !defined(__OS2__)
# include <winsock2.h>
# include <windows.h>
# include <sys/stat.h>
# include <io.h>
# include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif
using namespace std;
using namespace gnash;

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
const short SOL_MAGIC = 0xbf00;
const char *SOL_FILETYPE = "TCSO";
const short SOL_BLOCK_MARK = 0x0004;

namespace amf
{
SOL::SOL()
{
    GNASH_REPORT_FUNCTION;
}

SOL::~SOL()
{
    GNASH_REPORT_FUNCTION;
}

bool
SOL::extractHeader(std::string &filespec)
{
    GNASH_REPORT_FUNCTION;
}

bool
SOL::extractHeader(vector<unsigned char> &data)
{
}

bool
SOL::formatHeader(vector<unsigned char> &data)
{
    GNASH_REPORT_FUNCTION;
}

bool
SOL::formatHeader(std::string &name)
{
    GNASH_REPORT_FUNCTION;
}    

// write the data to disk as a .sol file
bool
SOL::writeFile(std::string &filespec)
{
    GNASH_REPORT_FUNCTION;
    ofstream ofs("filename", ios::binary);
}

// read the .sol file from disk
bool
SOL::readFile(std::string &filespec)
{
    GNASH_REPORT_FUNCTION;
    struct stat st;
    uint16_t magic, size;
    char *buf, *ptr;

    // Make sure it's an SOL file
    if (stat(filespec.c_str(), &st) == 0) {
        ifstream ifs(filespec.c_str(), ios::binary);
        _filesize = st.st_size;
        _filespec = filespec;
        ptr = buf = new char[_filesize+1];
        ifs.read(buf, _filesize);

        // extract the magic number
        magic = *(reinterpret_cast<uint16_t *>(ptr));
        magic = ntohs(magic);
        ptr += 2;

        // extract the file size
        int length = *(reinterpret_cast<uint32_t *>(ptr));
        length = ntohl(length);
        ptr += 4;

        // extract the file marker field
//        char *marker = ptr;
        ptr += 10;
        
        if ((memcmp(buf, &SOL_MAGIC, 2) == 0) && (_filesize - 6 == length)) {
            log_debug("%s is an SOL file", filespec.c_str());
        } else {
            log_error("%s isn't an SOL file", filespec.c_str());
        }

        // 2 bytes for the length of the object name, but it's also null terminated
        size = *(reinterpret_cast<uint16_t *>(ptr));
        size = ntohs(size);
        ptr += 2;
        _objname = ptr;

        ptr += size;
        // Go past the padding
        ptr += 4;

        AMF amf_obj;
        AMF::amf_element_t el;
        while ((buf - ptr) <= _filesize) {
            ptr = (char *)amf_obj.extractVariable(&el, reinterpret_cast<uint8_t *>(ptr));
            if (ptr == 0) {
                return true;
            }
            ptr += 1;            
            addObj(el);
        }
        
        ifs.close();
        return true;
    }
    
    return false;
}

void
SOL::dump()
{
    uint8_t *hexint;
    vector<AMF::amf_element_t>::iterator it;

    cerr << "Dumping SOL file" << endl;
    cerr << "The file name is: " << _filespec << endl;
    cerr << "The size of the file is: " << _filesize << endl;
    cerr << "The name of the object is: " << _objname << endl;
    for (it = _amfobjs.begin(); it != _amfobjs.end(); it++) {
        AMF::amf_element_t *el = &(*(it));
        cerr << el->name << ": ";
        if (el->type == AMF::STRING) {
            if (el->length != 0) {
                cerr << el->data;
            } else {
                cerr << "null";
            }
        }
        if (el->type == AMF::NUMBER) {
            double *dd = (double *)el->data;
            cerr << *dd << "    ";
            hexint = new uint8_t[(sizeof(double) *3) + 3];
            hexify(hexint, el->data, 8, false);
            cerr << "( " << hexint << ")";
        }
        if ((*(it)).type == AMF::BOOLEAN) {
            if (el->data[0] == true) {
                cerr << "true";
            }
            if (el->data[0] == false) {
                cerr << "false";
            }
        }
        if (el->type == AMF::OBJECT) {
            cerr << "is an object";
        }
        cerr << endl;
    }
}


} // end of amf namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
