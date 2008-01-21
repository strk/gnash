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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/cstdint.hpp>
#include <cerrno>
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
const short SOL_MAGIC = 0x00bf;	// this is in big-endian format already
//char *SOL_FILETYPE = "TCSO";
const short SOL_BLOCK_MARK = 0x0004;

namespace amf
{

SOL::SOL() 
    : _filesize(0)
{
//    GNASH_REPORT_FUNCTION;
}

SOL::~SOL()
{
//    GNASH_REPORT_FUNCTION;

    vector<amf::Element *>::iterator it;
    for (it = _amfobjs.begin(); it != _amfobjs.end(); it++) {
	//amf::Element *el = (*(it));
//	delete el;
    }
}

bool
SOL::extractHeader(const std::string & /*filespec*/)
{
//    GNASH_REPORT_FUNCTION;
      return false;
}

bool
SOL::extractHeader(const vector<unsigned char> & /*data*/)
{
//    GNASH_REPORT_FUNCTION;
      return false;
}

void
SOL::addObj(amf::Element *el)
{
//    GNASH_REPORT_FUNCTION;
    _amfobjs.push_back(el);
//    _filesize += el->getName().size() + el->getLength() + 5;
}

bool
SOL::formatHeader(vector<unsigned char> & /*data*/)
{
//    GNASH_REPORT_FUNCTION;
      return false;
}

// name is the object name
bool
SOL::formatHeader(std::string &name)
{
    return formatHeader(name, _filesize);
}

bool
SOL::formatHeader(std::string &name, int filesize)
{
//    GNASH_REPORT_FUNCTION;
    boost::uint32_t i;

    // First we add the magic number. All SOL data is in big-endian format,
    // so we swap it first.
    boost::uint16_t swapped = SOL_MAGIC;
//    swapped = ntohs(swapped);
    boost::uint8_t *ptr = reinterpret_cast<boost::uint8_t *>(&swapped);
    for (i=0; i<sizeof(boost::uint16_t); i++) {
        _header.push_back(ptr[i]);
    }

    // Next is the file size to be created. We adjust it as the filesize
    // includes the padding in the header, the mystery bytes, and the
    // padding, plus the length of the name itself.
    filesize += name.size() + 16;
    boost::uint32_t len = filesize;
    len = htonl(len);
    ptr = reinterpret_cast<boost::uint8_t *>(&len);
    for (i=0; i<sizeof(boost::uint32_t); i++) {
        _header.push_back(ptr[i]);
    }

    // Then the mystery block, but as the value never seems to every change,
    // we just built it the same way it always is.
    // first is the TCSO, we have no idea what this stands for.
//    ptr = reinterpret_cast<uint8_t *>(const_cast<uint8_t *>("TCSO");
    ptr = (uint8_t *)"TCSO";
    for (i=0; i<sizeof(boost::uint32_t); i++) {
        _header.push_back(ptr[i]);
    }
    // then the 0x0004 bytes, also a mystery
    swapped = SOL_BLOCK_MARK;
    swapped = htons(swapped);
    ptr = reinterpret_cast<boost::uint8_t *>(&swapped);
    for (i=0; i<sizeof(boost::uint16_t); i++) {
        _header.push_back(ptr[i]);
    }
    // finally a bunch of zeros to pad things for this field
    for (i=0; i<sizeof(boost::uint32_t); i++) {
        _header.push_back('\0');
    }

    // Encode the name. This is not a string object, which has a type field
    // one byte field precedding the length as a file type of AMF::STRING.
    //  First the length in two bytes
    swapped = name.size();
    swapped = htons(swapped);
    ptr = reinterpret_cast<boost::uint8_t *>(&swapped);
    for (i=0; i<sizeof(boost::uint16_t); i++) {
        _header.push_back(ptr[i]);
    }
    // then the string itself
    ptr = (boost::uint8_t *)name.c_str();
    for (i=0; i<name.size(); i++) {
        _header.push_back(ptr[i]);
    }
    
    // finally a bunch of zeros to pad things at the end of the header
    for (i=0; i<sizeof(boost::uint32_t); i++) {
        _header.push_back('\0');
    }

#if 0
    unsigned char *hexint;
    hexint = new unsigned char[(_header.size() + 3) *3];
    
    hexify(hexint, (unsigned char *)_header, _header.size(), true);
    log_msg (_("%s: SOL file header is: \n%s"), __FUNCTION__, (char *)hexint);
    delete hexint;
#endif    
    
    return true;
}    

// write the data to disk as a .sol file

bool
SOL::writeFile(string &filespec, const char *name)
{
//    GNASH_REPORT_FUNCTION;
    string str = name;
    return writeFile(filespec, str);
}

bool
SOL::writeFile(const char *filespec, const char *name)
{
//    GNASH_REPORT_FUNCTION;
    string str1 = filespec;
    string str2 = name;
    return writeFile(str1, str2);
}

bool
SOL::writeFile(string &filespec, string &name)
{
//    GNASH_REPORT_FUNCTION;
    ofstream ofs(filespec.c_str(), ios::binary);
    vector<boost::uint8_t>::iterator it;
    vector<amf::Element *>::iterator ita; 
    AMF amf_obj;
    char *ptr;
    int size = 0;
    
    if (filespec.size() == 0) {
	return false;
    }

    for (ita = _amfobjs.begin(); ita != _amfobjs.end(); ita++) {
        amf::Element *el = (*(ita));
	size += el->getName().size() + el->getLength() + 5;
    }
    _filesize = size;
    
    char *body = new char[size + 20];
    memset(body, 0, size);
    ptr = body;

    for (ita = _amfobjs.begin(); ita != _amfobjs.end(); ita++) {
        amf::Element *el = (*(ita));
        int outsize = el->getName().size() + el->getLength() + 5;
        boost::uint8_t *foo = amf_obj.encodeVariable(el); 
        switch (el->getType()) {
	  case Element::BOOLEAN:
	      outsize = el->getName().size() + 5;
	      memcpy(ptr, foo, outsize);
	      ptr += outsize;
	      break;
	  case Element::OBJECT:
	      outsize = el->getName().size() + 5;
	      memcpy(ptr, foo, outsize);
	      ptr += outsize;
	      *ptr++ = Element::OBJECT_END;
	      *ptr++ = 0;	// objects are terminated too!
	      break;
	  case Element::NUMBER:
	      outsize = el->getName().size() + AMF_NUMBER_SIZE + 2;
	      memcpy(ptr, foo, outsize);
	      ptr += outsize;
	      *ptr++ = 0;	// doubles are terminated too!
	      *ptr++ = 0;	// doubles are terminated too!
	      break;
	  case Element::STRING:
	      if (el->getLength() == 0) {
		  memcpy(ptr, foo, outsize+1);
		  ptr += outsize+1;
	      } else {		// null terminate the string
		  memcpy(ptr, foo, outsize);
		  ptr += outsize;
		  *ptr++ = 0;
	      }
	      break;
	  default:
	      memcpy(ptr, foo, outsize);
	      ptr += outsize;
	}
	if (foo) {
	    delete[] foo;
	}
    }
    
    _filesize = ptr - body;
    int len = name.size() + sizeof(uint16_t) + 16;
    char *head = new char[len + 4];
    memset(head, 0, len);
    ptr = head;
    formatHeader(name);
    for (it = _header.begin(); it != _header.end(); it++) {
        *ptr++ = (*(it));
    }
    
    ofs.write(head, _header.size());
//    ofs.write(body, (ptr - body));
    ofs.write(body, _filesize);
    ofs.close();

    return true;
}

// read the .sol file from disk
bool
SOL::readFile(std::string &filespec)
{
//    GNASH_REPORT_FUNCTION;
    struct stat st;
    boost::uint16_t size;
    boost::uint8_t *buf, *ptr;
    int bodysize;

    // Make sure it's an SOL file
    if (stat(filespec.c_str(), &st) == 0) {
        ifstream ifs(filespec.c_str(), ios::binary);
        _filesize = st.st_size;
	bodysize = st.st_size - 6;
        _filespec = filespec;
        ptr = buf = new boost::uint8_t[_filesize+1];
        ifs.read(reinterpret_cast<char *>(buf), _filesize);

        // skip the magic number (will check later)
        ptr += 2;

        // extract the file size
        int length = *(reinterpret_cast<boost::uint32_t *>(ptr));
        length = ntohl(length);
        ptr += 4;

        // skip the file marker field
        ptr += 10;

	// consistency check
        if ((buf[0] == 0) && (buf[1] == 0xbf)) {
            if (bodysize == length) {
                log_debug("%s is an SOL file", filespec.c_str());
            } else {
                log_error("%s looks like an SOL file, but the length is wrong. Should be %d, got %d",
                          filespec.c_str(), (_filesize - 6), length);
            }
            
        } else {
            log_error("%s isn't an SOL file", filespec.c_str());
        }

        // 2 bytes for the length of the object name, but it's also null terminated
        size = *(reinterpret_cast<boost::uint16_t *>(ptr));
        size = ntohs(size);
        ptr += 2;
        _objname = reinterpret_cast<const char *>(ptr);

        ptr += size;
        // Go past the padding
        ptr += 4;

        AMF amf_obj;
        while ((ptr - buf) < bodysize) {
	    amf::Element *el = new amf::Element;
	    ptr = amf_obj.extractVariable(el, ptr);
            if (ptr != 0) {
		ptr += 1;    
		addObj(el);
	    } else {
		break;
	    }
        }
        
        ifs.close();
        return true;
    }
    
//    log_error("Couldn't open file: %s", strerror(errno));
    return false;
}

void
SOL::dump()
{
    uint8_t *hexint;
    vector<amf::Element *>::iterator it;

    cerr << "Dumping SOL file" << endl;
    cerr << "The file name is: " << _filespec << endl;
    cerr << "The size of the file is: " << _filesize << endl;
    cerr << "The name of the object is: " << _objname << endl;
    for (it = _amfobjs.begin(); it != _amfobjs.end(); it++) {
	amf::Element *el = (*(it));
        cerr << el->getName() << ": ";
        if (el->getType() == Element::STRING) {
            if (el->getLength() != 0) {
                cerr << el->getData();
            } else {
                cerr << "null";
            }
        }
        if (el->getType() == Element::NUMBER) {
            double ddd = *((double *)el->getData());
             cerr << ddd << " ";
            hexint = new uint8_t[(sizeof(double) *3) + 3];
            hexify(hexint, el->getData(), 8, false);
            cerr << "( " << hexint << ")";
        }
        if ((*(it))->getType() == Element::BOOLEAN) {
            if (el[0] == true) {
                cerr << "true";
            }
            if (el[0] == false) {
                cerr << "false";
            }
        }
        if (el->getType() == Element::OBJECT) {
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
