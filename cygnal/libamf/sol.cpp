// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#include "GnashSystemNetHeaders.h"
#include "GnashFileUtilities.h"
#include "element.h"
#include "amf.h"
#include "buffer.h"
#include "sol.h"
#include "log.h"
#include "GnashException.h"

#include <cstdint>
#include <cerrno>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cassert>

using std::vector;
using gnash::ParserException;
using gnash::log_debug;
using gnash::log_error;

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
const std::uint16_t SOL_MAGIC = 0x00bf;	// is in big-endian format, this is the first
				// two bytes. of the .sol file.
//char *SOL_FILETYPE = "TCSO";
const std::uint16_t SOL_BLOCK_MARK = 0x0004;

/// \define ENSUREBYTES
///
/// @param from The base address to check.
///
/// @param tooFar The ending address that is one byte too many.
///
/// @param size The number of bytes to check for: from to tooFar.
///
/// @remarks May throw an Exception
#define ENSUREBYTES(from, toofar, size) { \
	if ( from+size >= toofar ) \
		throw ParserException("Premature end of AMF stream"); \
}

/// \namespace cygnal
///
/// This namespace is for all the AMF specific classes in libamf.
namespace cygnal
{

SOL::SOL() 
    : _filesize(0)
{
//    GNASH_REPORT_FUNCTION;
}

SOL::~SOL()
{
//    GNASH_REPORT_FUNCTION;
}

/// \brief Extract the header from the file.
///
/// @param filespec The name and path of the .sol file to parse.
///
/// @return true if this succeeded. false if it doesn't.
bool
SOL::extractHeader(const std::string & /*filespec*/)
{
//    GNASH_REPORT_FUNCTION;
      return false;
}

/// \brief Extract the header from the file.
///
/// @param data a reference to a vector of bytes that contains the
///	.sol file data.
///
/// @return true if this succeeded. false if it doesn't.
bool
SOL::extractHeader(const vector<unsigned char> & /*data*/)
{
//    GNASH_REPORT_FUNCTION;
      return false;
}

/// \brief Add the AMF objects that are the data of the file
//
/// @param el A smart pointer to the Element to add to the .sol file.
///
/// @return nothing.
void
SOL::addObj(std::shared_ptr<cygnal::Element> el)
{
//    GNASH_REPORT_FUNCTION;
    _amfobjs.push_back(el);
//    _filesize += el->getName().size() + el->getDataSize() + 5;
}

/// \brief Create the file header.
///
/// @param data a reference to a vector of bytes that contains the
///		.sol file data.
///
/// @return true if this succeeded. false if it doesn't.
bool
SOL::formatHeader(const vector<unsigned char> & /*data*/)
{
//    GNASH_REPORT_FUNCTION;
      return false;
}

/// \brief Create the file header.
///
/// @param name The name of the SharedObject for this file.
///
/// @return true if this succeeded. false if it doesn't.
bool
SOL::formatHeader(const std::string &name)
{
    return formatHeader(name, _filesize);
}

template <typename T>
void
appendSwapped(std::vector<std::uint8_t>& container, T val)
{
    std::uint8_t *ptr =
        static_cast<std::uint8_t*>(swapBytes(&val, sizeof(val)));
    container.insert(container.end(), ptr, ptr+sizeof(val));
}

/// \brief Create the file header.
///
/// @param name The name of the SharedObject for this file.
///
/// @param filesize The size of the file.
///
/// @return true if this succeeded. false if it doesn't.
bool
SOL::formatHeader(const std::string &name, int filesize)
{
//    GNASH_REPORT_FUNCTION;

    // First we add the magic number. All SOL data is in big-endian format,
    // so we swap it first.
    appendSwapped(_header, SOL_MAGIC);

    // Next is the file size to be created. We adjust it as the filesize
    // includes the padding in the header, the mystery bytes, and the
    // padding, plus the length of the name itself.
    filesize += name.size() + 16;
    std::uint32_t len = filesize;
    appendSwapped(_header, len);

    // Then the mystery block, but as the value never seems to every change,
    // we just built it the same way it always is.
    // first is the TCSO, we have no idea what this stands for.
    const char magic[] = "TCSO";
    _header.insert(_header.end(), boost::begin(magic), boost::end(magic));

    // then the 0x0004 bytes, also a mystery
    appendSwapped(_header, SOL_BLOCK_MARK);
    // finally a bunch of zeros to pad things for this field
    _header.insert(_header.end(), '\0', sizeof(std::uint32_t));

    // Encode the name. This is not a string object, which has a type field
    // one byte field precedding the length as a file type of AMF::STRING.
    //  First the length in two bytes
    std::uint16_t size = name.size();
    appendSwapped(_header, size);

    // then the string itself
    _header.insert(_header.end(), name.begin(), name.end());
    
    // finally a bunch of zeros to pad things at the end of the header
    _header.insert(_header.end(), '\0', sizeof(std::uint32_t));

#if 0
    unsigned char *hexint;
    hexint = new unsigned char[(_header.size() + 3) *3];
    
    hexify(hexint, (unsigned char *)_header, _header.size(), true);
    log_debug (_("%s: SOL file header is: \n%s"), __FUNCTION__, (char *)hexint);
    delete hexint;
#endif    
    
    return true;
}    

/// \brief Write the data to disk as a .sol file
///
/// @param filespec The name and path of the .sol file to parse.
///
/// @param name The name of the SharedObject for this file.
///
/// @return true if this succeeded. false if it doesn't.
bool
SOL::writeFile(const std::string &filespec, const std::string &name)
{
//    GNASH_REPORT_FUNCTION;
    std::ofstream ofs(filespec.c_str(), std::ios::binary);
    if ( ! ofs ) {
        log_error(_("Failed opening file '%s' in binary mode"), filespec.c_str());
        return false;
    }
    
    vector<std::uint8_t>::iterator it;
    vector<std::shared_ptr<cygnal::Element> >::iterator ita;
    AMF amf_obj;
    char *ptr;
    int size = 0;
    
    if (filespec.empty()) {
	return false;
    }

    for (ita = _amfobjs.begin(); ita != _amfobjs.end(); ++ita) {
        std::shared_ptr<cygnal::Element> el = (*(ita));
	size += el->getNameSize() + el->getDataSize() + 7;
    }
    _filesize = size;
    
    std::unique_ptr<char[]> body ( new char[size + 20] );
    std::fill_n(body.get(), size, 0);
    ptr = body.get();
    char* endPtr = ptr+size+20; // that's the amount we allocated..

    for (ita = _amfobjs.begin(); ita != _amfobjs.end(); ++ita) {
        std::shared_ptr<Element> el = (*(ita));
        std::shared_ptr<cygnal::Buffer> var = amf_obj.encodeProperty(el);
        //  std::uint8_t *var = amf_obj.encodeProperty(el, outsize);
        if (!var) {
            continue;
        }
        size_t outsize = 0;
        switch (el->getType()) {
	  case Element::BOOLEAN_AMF0:
	      outsize = el->getNameSize() + 4;
	      memcpy(ptr, var->reference(), outsize); 
	      ptr += outsize;
	      break;
	  case Element::OBJECT_AMF0:
	      outsize = el->getNameSize() + 5;
              assert(ptr+outsize < endPtr);
//	      outsize = el->getNameSize() + 5;
	      memcpy(ptr, var->reference(), outsize);
	      ptr += outsize;
	      *ptr++ = Element::OBJECT_END_AMF0;
//	      *ptr++ = 0;	// objects are terminated too!
	      break;
	  case Element::NUMBER_AMF0:
	      outsize = el->getNameSize() + AMF0_NUMBER_SIZE + 3;
              assert(ptr+outsize < endPtr);
	      memcpy(ptr, var->reference(), outsize);
	      ptr += outsize;
// 	      *ptr++ = 0;	// doubles are terminated too!
// 	      *ptr++ = 0;	// doubles are terminated too!
	      break;
	  case Element::STRING_AMF0:
	      if (el->getDataSize() == 0) {
              	  assert(ptr+outsize+1 < endPtr);
		  memcpy(ptr, var->reference(), outsize+1);
		  ptr += outsize+1;
	      } else {		// null terminate the string
                  assert(ptr+outsize < endPtr);
		  memcpy(ptr, var->reference(), outsize);
		  ptr += outsize;
		  *ptr++ = 0;
	      }
	      break;
	  default:
              assert(ptr+outsize < endPtr);
	      memcpy(ptr, var->reference(), outsize);
	      ptr += outsize;
	}
    }
    
    _filesize = ptr - body.get();
	int len = name.size() + sizeof(std::uint16_t) + 16;
    std::unique_ptr<char[]> head ( new char[len + 4] );
    memset(head.get(), 0, len);
    ptr = head.get();
    formatHeader(name);
    for (it = _header.begin(); it != _header.end(); ++it) {
        *ptr++ = (*(it));
    }
    
    if ( ofs.write(head.get(), _header.size()).fail() )
    {
        log_error(_("Error writing %d bytes of header to output file %s"),
                _header.size(), filespec);
        return false;
    }

    if ( ofs.write(body.get(), _filesize).fail() )
    {
        log_error(_("Error writing %d bytes of body to output file %s"),
                _filesize, filespec);
        return false;
    }
    ofs.close();

    return true;
}

/// \brief Read a .sol file from disk
///
/// @param filespec The name and path of the .sol file to parse.
///
/// @return true if this succeeded. false if it doesn't.
bool
SOL::readFile(const std::string &filespec)
{
//    GNASH_REPORT_FUNCTION;
    struct stat st;
    std::uint16_t size;
    size_t bodysize;

    // Make sure it's an SOL file
    if (stat(filespec.c_str(), &st) != 0)  return false;

	try {

        std::uint8_t *ptr = 0;

	    std::ifstream ifs(filespec.c_str(), std::ios::binary);

        _filesize = st.st_size;
        std::unique_ptr<std::uint8_t[]> buf(
                new std::uint8_t[_filesize + sizeof(int)]);

	    ptr = buf.get();
	    std::uint8_t* tooFar = buf.get() + _filesize;
	    
	    bodysize = st.st_size - 6;
	    _filespec = filespec;
	    ifs.read(reinterpret_cast<char *>(ptr), _filesize);
	    
#ifndef GNASH_TRUST_AMF
	    ENSUREBYTES(ptr, tooFar, 2+4+10); // magic number, file size, file marker
#endif
	    
	    // skip the magic number (will check later)
	    ptr += 2;
	    
	    // extract the file size
	    std::uint32_t length = *(reinterpret_cast<std::uint32_t *>(ptr));
	    length = ntohl(length);
	    ptr += 4;
	    
	    // skip the file marker field
	    ptr += 10;
	    
	    // consistency check
	    if ((buf[0] == 0) && (buf[1] == 0xbf)) {
            if (bodysize == length) {
                log_debug(_("%s is an SOL file"), filespec);
            }
            else {
		    log_error(_("%s looks like an SOL file, but the length is wrong. "
				"Should be %d, got %d"),
                    filespec, (_filesize - 6), length);
            }
        }
        else {
            log_error(_("%s isn't an SOL file"), filespec);
	    }
	    
#ifndef GNASH_TRUST_AMF
	    ENSUREBYTES(ptr, tooFar, 2); 
#endif
	    
	    // 2 bytes for the length of the object name, but it's also 
        // null terminated
	    size = *(reinterpret_cast<std::uint16_t *>(ptr));
	    size = ntohs(size);
	    ptr += 2;
	    
#ifndef GNASH_TRUST_AMF
	    ENSUREBYTES(ptr, tooFar, size+4);  // 4 is the padding below
#endif
	    
	    // TODO: make sure there's the null-termination, or
	    //       force if if it's there by definition
	    _objname = reinterpret_cast<const char *>(ptr);
	    ptr += size;
	    
	    // Go past the padding
	    ptr += 4;
	    
	    AMF amf_obj;
	    std::shared_ptr<cygnal::Element> el;
	    while ( ptr < tooFar) {
            if (ptr) {
                el = amf_obj.extractProperty(ptr, tooFar);
                if (el != 0) {
                    ptr += amf_obj.totalsize() + 1;
                    _amfobjs.push_back(el);
                }
                else break;
            } else break;
	    }
	    
	    ifs.close();
	    return true;
	}
    catch (std::exception& e) {
	log_error(_("Reading SharedObject %s: %s"), filespec, e.what());
	    return false;
	}
    
}

bool 
SOL::updateSO(std::shared_ptr<cygnal::Element> &newel)
{
//    GNASH_REPORT_FUNCTION;
    vector<std::shared_ptr<cygnal::Element> >::iterator ita;
    for (ita = _amfobjs.begin(); ita != _amfobjs.end(); ++ita) {
        std::shared_ptr<cygnal::Element> oldel = (*(ita));
	if (oldel == newel) {
	    oldel = newel;
	}
   }
   return true;
}

bool
SOL::updateSO(int index, std::shared_ptr<cygnal::Element> &el)
{
//    GNASH_REPORT_FUNCTION;
    _amfobjs[index] = el;    
    return true;
}

///  \brief Dump the internal data of this class in a human readable form.
///
/// @remarks This should only be used for debugging purposes.
void
SOL::dump()
{
    using namespace std;
    vector<std::shared_ptr<cygnal::Element> >::iterator it;

    cerr << "Dumping SOL file" << endl;
    cerr << "The file name is: " << _filespec << endl;
    cerr << "The size of the file is: " << _filesize << endl;
    cerr << "The name of the object is: " << _objname << endl;
    for (it = _amfobjs.begin(); it != _amfobjs.end(); ++it) {
	std::shared_ptr<cygnal::Element> el = (*(it));
        cerr << el->getName() << ": ";
        if (el->getType() == Element::STRING_AMF0) {
            if (el->getDataSize() != 0) {
                cerr << el->to_string();
            } else {
                cerr << "null";
            }
        }
        if (el->getType() == Element::NUMBER_AMF0) {
            double ddd = el->to_number();
	    swapBytes(&ddd, sizeof(double));
	    cerr << ddd << endl;
//            cerr << "( " << hexify(el->to_(), 8, false) << ")";
        }
        if (el->getType() == Element::BOOLEAN_AMF0) {
            if (el->to_bool() == true) {
                cerr << "true";
            }
            if (el->to_bool() == false) {
                cerr << "false";
            }
        }
        if (el->getType() == Element::OBJECT_AMF0) {
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
