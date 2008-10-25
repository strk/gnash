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
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

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
//    Element(gnash::Network::byte_t *data);
    // Construct a Number
    Element(double data);
    Element(const std::string &name, double data);
    // Construct a String
    Element(const char *data);
    Element(const std::string &data);
    Element(const std::string &name, const std::string &data);
    // Construct a Boolean
    Element(bool data);
    Element(const std::string &name, bool data);

    //.Construct a strict array, where each item is the same type
    Element(std::vector<double> &data);
    Element(std::vector<std::string> &data);

    // Construct an ECMA array, which can contain any type
    Element(std::vector<Element > &data);
    
    // Create a function block for AMF
    Element(bool, double, double, const std::string &str);
    
    ~Element();
    void clear();
#if 0
    // Create a function block for AMF
    Element &init(bool, double, double, const std::string &str);
#endif
    
    // Make ourselves be the same as another Element.
    Element &operator=(Element &);
//    Element &operator=(boost::shared_ptr<Element>);

    // Make ourselves be the same as other commonly used data types. Note
    // that this can;t be used for properties unless you call setName() later.
    Element &operator=(double num);
    Element &operator=(const std::string &str);
    Element &operator=(bool flag);
    
    // A Null String is a string with a length of zero
    Element &makeNullString();

    // Make a string element
    Element &makeString(const char *str, size_t size);
    Element &makeString(gnash::Network::byte_t *data, size_t size);
    Element &makeString(const std::string &data);
    Element &makeString(const std::string &name, const std::string &data);

    // Make a number element
    Element &makeNumber(double num); 
    Element &makeNumber(boost::shared_ptr<amf::Buffer> buf); 
    Element &makeNumber(gnash::Network::byte_t *data);
    Element &makeNumber(const std::string &name, double num) ;
    Element &makeNumber(const std::string &name, gnash::Network::byte_t *data);

    // Make a boolean element
    Element &makeBoolean(gnash::Network::byte_t *data);
    Element &makeBoolean(bool data);
    Element &makeBoolean(const std::string &name, bool data);

    // Make an undefined element
    Element &makeUndefined();
    Element &makeUndefined(const std::string &name);

    // Make a Null element, which is often used as a placeholder
    Element &makeNull();
    Element &makeNull(const std::string &name);

    // Terminate an object
    Element &makeObjectEnd();

    // Make an AMF Object. This is an AMF type that support having properties.
    Element &makeObject();
    Element &makeObject(const std::string &name);
    Element &makeObject(std::vector<boost::shared_ptr<amf::Element> > &data);
    Element &makeObject(const std::string &name, std::vector<boost::shared_ptr<amf::Element> > &data);
    
    // Make an XML Object. This is like a string object, but the type is different.
    Element &makeXMLObject();
    Element &makeXMLObject(const std::string &name);
    Element &makeXMLObject(const std::string &name, const std::string &data);

    // Make an ECMA array. This is a mixed array of any AMF types. These are stored
    // the same as an object, but with a different type.
    Element &makeECMAArray();
    Element &makeECMAArray(const std::string &name);
    Element &makeECMAArray(std::vector<boost::shared_ptr<amf::Element> > &data);
    Element &makeECMAArray(const std::string &name, std::vector<boost::shared_ptr<amf::Element> > &data);

    // Make a Strict array. This is an array of a single AMF type. These are stored
    // the same as an object, but with a different type.
    Element &makeStrictArray();
    Element &makeStrictArray(const std::string &name);
    Element &makeStrictArray(std::vector<boost::shared_ptr<amf::Element> > &data);
    Element &makeStrictArray(const std::string &name, std::vector<boost::shared_ptr<amf::Element> > &data);

    Element &makeTypedObject(const std::string &name);
    Element &makeTypedObject(gnash::Network::byte_t *data, size_t size);
    
    Element &makeReference();
    Element &makeReference(gnash::Network::byte_t *data, size_t size);
    
    Element &makeMovieClip();
    Element &makeMovieClip(gnash::Network::byte_t *data, size_t size);

    Element &makeLongString();
    Element &makeLongString(gnash::Network::byte_t *data, size_t size);
    
    Element &makeRecordSet();
    Element &makeRecordSet(gnash::Network::byte_t *data, size_t size);
    
    Element &makeDate();
    Element &makeDate(gnash::Network::byte_t *data);
    
    Element &makeUnsupported();
    Element &makeUnsupported(gnash::Network::byte_t *data, size_t size);
    
//    Element &makeArray();

//    Element &makeConnect();
    
    // Test to see if Elements are the same
    bool operator==(Element &);
    bool operator==(boost::shared_ptr<amf::Element> );
    bool operator==(bool x);

    boost::shared_ptr<amf::Element> operator[](size_t x);

//    gnash::Network::byte_t *getData();
    size_t getDataSize() const;
    
    // These are all accessors for the various output formats
    amf0_type_e getType() const { return _type; };
    void setType(amf0_type_e x) { _type = x; };
//    void setData(Buffer *buf) { _buffer = buf; };

    // These accessors convert the raw data to a standard data type
    // we can use.
    bool to_bool() const;
    double to_number() const;
    const char *to_string() const;
    gnash::Network::byte_t *to_reference();

    // Manipulate the name of a property
    size_t getNameSize();
    char *getName() const { return _name; };
    void setName(const std::string &name);
    void setName(gnash::Network::byte_t *name, size_t x);
    void setName(const char *name, size_t x);

    // Manipulate the children Elements of an object
    boost::shared_ptr<Element> findProperty(const std::string &name);
    boost::shared_ptr<Element> getProperty(size_t x) const { return _properties[x]; };
    
    void addProperty(boost::shared_ptr<Element> el) { _properties.push_back(el); };
    boost::shared_ptr<Element> popProperty()        { return _properties.front(); };
    size_t propertySize() const   { return _properties.size(); };
    boost::shared_ptr<Buffer> encode();
    std::vector<boost::shared_ptr<Element> > getProperties() { return _properties; };
    void dump() const { dump(std::cerr); }
    void dump(std::ostream& os) const;
private:
    void check_buffer(size_t size);
    char		*_name;
    boost::shared_ptr<Buffer> _buffer;
    amf0_type_e		_type;
    std::vector<boost::shared_ptr<Element> > _properties;
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
