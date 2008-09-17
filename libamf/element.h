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

#ifndef GNASH_ELEMENT_H
#define GNASH_ELEMENT_H

#include <vector>
#include <string>
#include <cstring>
#include <iostream> // for output operator
#include <boost/cstdint.hpp>

//#include "buffer.h"
#include "network.h"
#include "dsodefs.h" // DSOEXPORT

namespace amf
{

class Buffer;

class DSOEXPORT Element {
public:
    // The following elements are defined within AMF0, and are used
    // up through SWF version 8.
    typedef enum {
	NOTYPE=-1,
        NUMBER_AMF0=0x00,
        BOOLEAN_AMF0=0x01,
        STRING_AMF0=0x02,
        OBJECT_AMF0=0x03,
        MOVIECLIP_AMF0=0x04,
        NULL_AMF0=0x05,
        UNDEFINED_AMF0=0x06,
        REFERENCE_AMF0=0x07,
        ECMA_ARRAY_AMF0=0x08,
        OBJECT_END_AMF0=0x09,
        STRICT_ARRAY_AMF0=0x0a,
        DATE_AMF0=0x0b,
        LONG_STRING_AMF0=0x0c,
        UNSUPPORTED_AMF0=0x0d,
        RECORD_SET_AMF0=0x0e,
        XML_OBJECT_AMF0=0x0f,
        TYPED_OBJECT_AMF0=0x10,
	AMF3_DATA=0x11,
// 	// these aren't part of the AMF spec, they're used internally
 	RTMP_HEADER=0x20,
// 	FUNCTION=0x12
    } amf0_type_e;
    // AMF3, was introduced with ActionScript 3 in SWF version 9
    // to reduce duplicate, and allow for a more compact layout of
    // data to save bandwidth and improve performance.
    typedef enum {
        UNDEFINED_AMF3=0x00,
        NULL_AMF3=0x01,
        FALSE_AMF3=0x02,
        TRUE_AMF3=0x03,
        INTEGER_AMF3=0x04,
        DOUBLE_AMF3=0x05,
        STRING_AMF3=0x06,
        XMLDOC_AMF3=0x07,
        DATE_AMF3=0x08,
        ARRAY_AMF3=0x09,
        OBJECT_AMF3=0x0a,
        XML_AMF3=0x0b,
        BYTES_AMF3=0x0c,
    } amf3_type_e;
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
    Element &init(const std::string &name, double data);
    Element &init(double data);
    Element &init(std::vector<double> &data);
    Element &init(const std::string &name, const std::string &data);
    Element &init(const std::string &data);
    Element &init(const std::string &name, bool data);
    Element &init(bool data);
    // Create a function block for AMF
    Element &init(bool, double, double, const std::string &str);

    // These create the other "special" AMF types.
    Element &makeNullString(); 
    Element &makeString(const char *str, size_t size); 
    Element &makeString(gnash::Network::byte_t *data, size_t size); 
    Element &makeString(const std::string &data); 
    Element &makeString(const std::string &name, const std::string &data);
    
    Element &makeNumber(double num); 
    Element &makeNumber(amf::Buffer *buf); 
    Element &makeNumber(gnash::Network::byte_t *data); 
    Element &makeNumber(const std::string &name, double num);
    Element &makeNumber(const std::string &name, gnash::Network::byte_t *data); 
    
    Element &makeBoolean(gnash::Network::byte_t *data); 
    Element &makeBoolean(bool data); 
    Element &makeBoolean(const std::string &name, bool data);
    
    Element &makeUndefined();
    Element &makeUndefined(const std::string &name);
    
    Element &makeNull();
    Element &makeNull(const std::string &name);

    Element &makeObjectEnd();

    Element &makeObject();
    Element &makeObject(const std::string &name);
    Element &makeObject(gnash::Network::byte_t *data, size_t size);
    
    Element &makeXMLObject(const std::string &name);
    Element &makeXMLObject(gnash::Network::byte_t *data, size_t size);
    
    Element &makeTypedObject(const std::string &name);
    Element &makeTypedObject(gnash::Network::byte_t *data, size_t size);
    
    Element &makeReference();
    Element &makeReference(gnash::Network::byte_t *data, size_t size);
    
    Element &makeMovieClip();
    Element &makeMovieClip(gnash::Network::byte_t *data, size_t size);

    Element &makeECMAArray();
    Element &makeECMAArray(gnash::Network::byte_t *data, size_t size);
    
    Element &makeLongString();
    Element &makeLongString(gnash::Network::byte_t *data, size_t size);
    
    Element &makeRecordSet();
    Element &makeRecordSet(gnash::Network::byte_t *data, size_t size);
    
    Element &makeDate();
    Element &makeDate(gnash::Network::byte_t *data);
    
    Element &makeUnsupported();
    Element &makeUnsupported(gnash::Network::byte_t *data, size_t size);
    
    Element &makeStrictArray();
    Element &makeStrictArray(gnash::Network::byte_t *data, size_t size);
//    Element &makeArray();

//    Element &makeConnect();
    
    // Test to see if Elements are the same
    bool operator==(Element &);
    bool operator==(Element *);
    bool operator==(bool x);

    // Make ourselves be the same as another Element.
    Element &operator=(Element &);
    Element &operator=(Element *);
    
    Element *operator[](size_t x);

    gnash::Network::byte_t *getData() const;
    size_t getLength() const;
    Buffer *getBuffer() { return _buffer; };
    
    // These are all accessors for the various output formats
    amf0_type_e getType() const { return _type; };
    void setType(amf0_type_e x) { _type = x; };
//    void setData(Buffer *buf) { _buffer = buf; };

    // These accessors convert the raw data to a standard data type we can use.
    double to_number() const;
    const char *to_string() const;
    bool to_bool() const;
    void *to_reference();
    
    char *getName() const { return _name; };
    size_t getNameSize();
    void setName(const std::string &name);
    void setName(gnash::Network::byte_t *name, size_t x);
    void setName(const char *name, size_t x);

    // Manipulate the children Elements of an object
    Element *findProperty(const std::string &name);
    Element *getProperty(size_t x) const { return _properties[x]; };
    
    void addProperty(Element &el) { _properties.push_back(&el); };
    void addProperty(Element *el) { _properties.push_back(el); };
    Element *popProperty()        { return _properties.front(); };
    size_t propertySize() const   { return _properties.size(); };
    amf::Buffer *encode();
    std::vector<Element *> getProperties() { return _properties; };
    void dump() const { dump(std::cerr); }
    void dump(std::ostream& os) const;
private:
    void check_buffer(size_t size);
    char		*_name;
    Buffer		*_buffer;
    amf0_type_e		_type;
    std::vector<Element	*> _properties;
};                              // end of class definition

inline std::ostream& operator << (std::ostream& os, const Element& el)
{
	el.dump(os);
	return os;
}


} // end of amf namespace

// end of _ELEMENT_H_
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
