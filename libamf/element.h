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

#include <vector>
#include <string>
#include <cstring>
#include <boost/cstdint.hpp>

//#include "buffer.h"
#include "network.h"
#include "dsodefs.h" // DSOEXPORT

namespace amf
{

class Buffer;

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
    } amf_type_e;
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
    Element &makeNumber(gnash::Network::byte_t *data); 
    Element &makeNumber(const std::string &name, double num);
    
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

    // Test to see if Elements are the same
    bool operator==(Element &);
    bool operator==(Element *);
    bool operator==(bool x);

    // Make ourselves be the same as another Element.
    Element &operator=(Element &);
    Element &operator=(Element *);
    
    Element *operator[](int x);

    gnash::Network::byte_t *getData();
    size_t getLength();
    Buffer *getBuffer() { return _buffer; };
    
    // These are all accessors for the various output formats
    amf_type_e getType() { return _type; };
    void setType(amf_type_e x) { _type = x; };
//    void setData(Buffer *buf) { _buffer = buf; };

    // These accessors convert the raw data to a standard data type we can use.
    double to_number();
    const char *to_string();
    bool to_bool();
    void *to_reference();
    
    char *getName() const { return _name; };
    size_t getNameSize();
    void setName(const std::string &name);
    void setName(gnash::Network::byte_t *name, size_t x);

    // Manipulate the children Elements of an object
    void addChild(Element &el) { _children.push_back(&el); };
    void addChild(Element *el) { _children.push_back(el); };
    Element *popChild() { return _children.front(); };
    size_t childrenSize() { return _children.size(); };
//    std::vector<Element	*> &getChildren() { return _children; };

    amf::Buffer *encode();
    
    void dump();
private:
    void check_buffer(size_t size);
    char		*_name;
    Buffer		*_buffer;
    amf_type_e		_type;
    std::vector<Element	*> _children;
};                              // end of class definition


} // end of amf namespace

// end of _ELEMENT_H_
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
