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

#ifndef _ELEMENT_H_
#define _ELEMENT_H_

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <vector>
#include <string>
#include <cstring>

#include <boost/cstdint.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "network.h"
#include "amfutf8.h"
#include "amf.h"
#include "dsodefs.h" // DSOEXPORT

namespace amf 
{

class DSOEXPORT Element {
public:
    // The following elements are defined within AMF:
    typedef enum {
	NOTYPE=-1,
        NUMBER=0x00,
        BOOLEAN=0x01,
        STRING=0x02,
        OBJECT=0x03,
        MOVIECLIP=0x04,
        NULL_VALUE=0x05,
        UNDEFINED=0x06,
        REFERENCE=0x07,
        ECMA_ARRAY=0x08,
        OBJECT_END=0x09,
        STRICT_ARRAY=0x0a,
        DATE=0x0b,
        LONG_STRING=0x0c,
        UNSUPPORTED=0x0d,
        RECORD_SET=0x0e,
        XML_OBJECT=0x0f,
        TYPED_OBJECT=0x10,
	// these aren't part of the AMF spec, they're used internally
	VARIABLE=0x11,
	FUNCTION=0x12
    } astype_e;
    Element();
    Element(gnash::Network::byte_t *data);
    Element(double data);
    Element(std::vector<double> &data);
    Element(const std::string &data);
    Element(const std::string &name, const std::string &data);
    Element(bool data);
    Element(const std::string &name, bool data);
    // Create a function block for AMF
    Element(bool, double, double, const std::string &str);
    ~Element();
    void clear();
    gnash::Network::byte_t *init(gnash::Network::byte_t *data);
    Element &init(const std::string &name, double data);
    Element &init(double data);
    Element &init(std::vector<double> &data);
    Element &init(const std::string &name, const std::string &data);
    Element &init(const std::string &data);
    Element & init(const std::string &name, bool data);
    Element &init(bool data);
    // Create a function block for AMF
    Element &init(bool, double, double, const std::string &str);

    // These create the other "special" AMF types.
    Element &makeString(gnash::Network::byte_t *data, int size); 
    Element &makeString(const std::string &data); 
    Element &makeString(const std::string &name, const std::string &data);
    
    Element &makeNumber(gnash::Network::byte_t *data); 
    Element &makeNumber(const std::string &name, double);
    
    Element &makeBoolean(gnash::Network::byte_t *data); 
    Element &makeBoolean(bool data); 
    Element &makeBoolean(const std::string &name, bool data);
    
    Element &makeUndefined();
    Element &makeUndefined(const std::string &name);
    Element &makeNull();
    Element &makeNull(const std::string &name);
    Element &makeObjectEnd();
    Element &makeObject(gnash::Network::byte_t *data, int size);
    Element &makeXMLObject(gnash::Network::byte_t *data, int size);
    Element &makeTypedObject(gnash::Network::byte_t *data, int size);
    Element &makeReference(gnash::Network::byte_t *data, int size);
    Element &makeMovieClip(gnash::Network::byte_t *data, int size);
    Element &makeECMAArray(gnash::Network::byte_t *data, int size);
    Element &makeLongString(gnash::Network::byte_t *data, int size);
    Element &makeRecordSet(gnash::Network::byte_t *data, int size);
    Element &makeDate(gnash::Network::byte_t *data);
    Element &makeStrictArray(gnash::Network::byte_t *data, int size);
//    Element &makeArray();
    
    Element &operator=(Element &);

    bool operator==(bool x) { if (_data) return _data[0] == x; return false; };
    uint8_t operator[](int x) { if (_data) return _data[x]; return 0; };
    
    // These are all accessors for the various output formats
    astype_e getType() { return _type; };
    void setType(astype_e x) { _type = x; };
    gnash::Network::byte_t *getData() { return _data; };
    void setData(gnash::Network::byte_t *x) { _data = x; };

    // These accessors convert the raw data to a standard data type we can use.
    double to_number();
    const char *to_string();
    bool to_bool();
    void *to_reference();
    
    boost::uint16_t getLength() { return _length; };
    void setLength(boost::uint16_t x) { _length = x; };
    const std::string &getName() const { return _name; };
    void setName(const std::string &name) { _name = name; };
    void setName(gnash::Network::byte_t *name, int x);
//    boost::posix_time::ptime to_date();
    void dump();
    
private:
    astype_e	_type;
    boost::int16_t _length;
    std::string    _name;
    gnash::Network::byte_t *_data;
    std::vector<Element	*> _children;
};                              // end of class definition


} // end of amf namespace

// end of _ELEMENT_H_
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
