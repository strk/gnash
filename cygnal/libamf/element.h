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

#ifndef GNASH_ELEMENT_H
#define GNASH_ELEMENT_H

#include <vector>
#include <string>
#include <cstring>
#include <iostream> // for output operator
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

//#include "network.h"
#include "dsodefs.h" // DSOEXPORT

/// \namespace cygnal
///
/// This namespace is for all the AMF specific classes in libamf.
namespace cygnal
{

// Forward decleration
class Buffer;

/// \class Element
///
/// This class holds an deserialized AMF object or Property. The only
/// difference is that a Property sets the name field, while raw AMF
/// data does not.
class DSOEXPORT Element {
public:
    /// \enum Element::amf0_type_e
    ///		The following data types are defined within AMF0.
    typedef enum {
	NOTYPE            = -1,
        NUMBER_AMF0       = 0x00,
        BOOLEAN_AMF0      = 0x01,
        STRING_AMF0       = 0x02,
        OBJECT_AMF0       = 0x03,
        MOVIECLIP_AMF0    = 0x04,
        NULL_AMF0         = 0x05,
        UNDEFINED_AMF0    = 0x06,
        REFERENCE_AMF0    = 0x07,
        ECMA_ARRAY_AMF0   = 0x08,
        OBJECT_END_AMF0   = 0x09,
        STRICT_ARRAY_AMF0 = 0x0a,
        DATE_AMF0         = 0x0b,
        LONG_STRING_AMF0  = 0x0c,
        UNSUPPORTED_AMF0  = 0x0d,
        RECORD_SET_AMF0   = 0x0e,
        XML_OBJECT_AMF0   = 0x0f,
        TYPED_OBJECT_AMF0 = 0x10,
	AMF3_DATA         = 0x11,
// 	// these aren't part of the AMF spec, they're used internally
 	RTMP_HEADER       = 0x20
// 	FUNCTION=0x12
    } amf0_type_e;
    
    /// \enum Element::amf3_type_e
    ///		AMF3, was introduced with ActionScript 3 in SWF
    ///		version 9 to reduce duplicate, and allow for a more
    ///		compact layout of data to save bandwidth and improve
    ///		performance.
    typedef enum {
        UNDEFINED_AMF3 = 0x00,
        NULL_AMF3    = 0x01,
        FALSE_AMF3   = 0x02,
        TRUE_AMF3    = 0x03,
        INTEGER_AMF3 = 0x04,
        DOUBLE_AMF3  = 0x05,
        STRING_AMF3  = 0x06,
        XMLDOC_AMF3  = 0x07,
        DATE_AMF3    = 0x08,
        ARRAY_AMF3   = 0x09,
        OBJECT_AMF3  = 0x0a,
        XML_AMF3     = 0x0b,
        BYTES_AMF3   = 0x0c
    } amf3_type_e;

    /// \brief Create a new Element with no data type.
    Element();

    /// \brief Delete this Element
    ///		This deallocates all the memory used to hold the data.
    ~Element();

    /// \brief Construct an AMF Element from a double..
    ///
    /// @param data The double to use as the value for this Element.
    Element(double data);

    /// \brief Contruct a Property from a double.
    ///
    /// @param name The name of the Property
    ///
    /// @param num The double to use as the value of the property.
    Element(const std::string &name, double num);
    
    /// \brief Contruct an AMF Element from an ASCII string.
    ///
    /// @param data The ASCII string to use as the value of the property.
    ///
    /// @remarks This assume the data string is already NULL terminated.
    Element(const char *data);
    /// \overload Element(const std::string &data)
    Element(const std::string &data);
    
    /// \brief Contruct a Property from an ASCII string.
    ///
    /// @param name The name of the Property
    ///
    /// @param data The ASCII string to use as the value of the property.
    Element(const std::string &name, const std::string &data);
    
    /// \brief Contruct an AMF Element from a Boolean
    ///
    /// @param data The boolean to use as the value for this Element.
    Element(bool data);

    /// \brief Contruct a Property from a Boolean
    ///
    /// @param name The name of the Property
    ///
    /// @param data The boolean to use as the value of the property.
    Element(const std::string &name, bool data);
    
    /// \brief Contruct an AMF Element from an array of doubles.
    ///		This becomes a StrictArray by default, as all the data
    ///		types are the same.
    ///
    /// @param data The data to use as the values for this Element.
    Element(std::vector<double> &data);

    /// \brief Contruct an AMF Element from an array of ASCII strings
    ///		This becomes a StrictArray by default, as all the data
    ///		types are the same.
    ///
    /// @param data The data to use as the values for this Element.
    Element(std::vector<std::string> &data);

    /// \brief Contruct an AMF Element from an array of other Elements.
    ///		This becomes an EcmaArray by default, as all the data
    ///		types are different.
    ///
    /// @param data The data to use as the values for this Element.
    Element(std::vector<Element > &data);

    /// \brief Clear the contents of the buffer by setting all the bytes to
    ///		zeros.
    ///
    /// @return nothing
    void clear();
    
#if 0
    // Create a function block for AMF
    Element(bool, double, double, const std::string &str);

    // Create a function block for AMF
    Element &init(bool, double, double, const std::string &str);
#endif
    
    /// \brief Make this Element be the same as another Element.
    ///
    /// @param el A reference to an Element class.
    ///
    /// @return A reference to this Element.
    Element &operator=(Element &el);
//    Element &operator=(std::shared_ptr<Element>);

    /// \brief Make this Element be the same as a double.
    ///		This sets both the data type and the value.
    ///
    /// @param el A double value.
    ///
    /// @return A reference to this Element.
    Element &operator=(double num);
    
    /// \brief Make this Element be the same as an ASCII string.
    ///		This sets both the data type and the value.
    ///
    /// @param el An ASCII string value.
    ///
    /// @return A reference to this Element.
    Element &operator=(const std::string &str);

    /// \brief Make this Element be the same as a boolean value.
    ///		This sets both the data type and the value.
    ///
    /// @param el A boolean value.
    ///
    /// @return A reference to this Element.
    Element &operator=(bool flag);

    /// \brief Make this Element be a NULL String type.
    ///		A Null String is a string with a length of zero
    ///
    /// @return A reference to this Element.
    Element &makeNullString();

    /// \brief Make this Element with an ASCII string value.
    ///
    /// @param str The ASCII string to use as the value.
    ///
    /// @param size The number of bytes in the ASCII string.
    ///
    /// @return A reference to this Element.
    Element &makeString(const char *str, size_t size);
    /// \overload Element::makeString(boost::uint8_t *data, size_t size)
    Element &makeString(boost::uint8_t *data, size_t size);

    /// \brief Make this Element with an ASCII string value.
    ///
    /// @param str The ASCII string to use as the value.
    ///
    /// @return A reference to this Element.
    Element &makeString(const std::string &data);
    
    /// \brief Make this Element a Property with an ASCII String value.
    ///
    /// @param name The name of the Property
    ///
    /// @param str The ASCII string to use as the value of the
    ///		property.
    ///
    /// @return A reference to this Element.
    Element &makeString(const std::string &name, const std::string &str);

    /// \brief Make this Element with a double value.
    ///
    /// @param str The double to use as the value.
    ///
    /// @return A reference to this Element.
    Element &makeNumber(double num); 

    /// \brief Make this Element with a double value.
    ///
    /// @param buf A smart pointer to a Buffer class.
    ///
    /// @return A reference to this Element.
    Element &makeNumber(std::shared_ptr<cygnal::Buffer> buf);

    /// \brief Make this Element with a double value.
    ///		The size isn't needed as a double is always the same size.
    ///
    /// @param str The double to use as the value.
    ///
    /// @return A reference to this Element.
    Element &makeNumber(boost::uint8_t *data);
    
    /// \brief Make this Element a Property with a double value
    ///
    /// @param name The name of the Property
    ///
    /// @param num The double to use as the value of the property.
    Element &makeNumber(const std::string &name, double num) ;
    /// \overload Element::makeNumber(const std::string &name, boost::uint8_t *data);
    ///		The size isn't needed as a double is always the same size.
    Element &makeNumber(const std::string &name, boost::uint8_t *data);

    /// \brief Make this Element with a boolean value.
    ///		The size isn't needed as a boolean is always the same size.
    ///
    /// @param data A real pointer to the boolean use as the value.
    ///
    /// @return A reference to this Element.
    Element &makeBoolean(boost::uint8_t *data);

    /// \brief Make this Element with a boolean value.
    ///
    /// @param data A boolean to use as the value.
    ///
    /// @return A reference to this Element.
    Element &makeBoolean(bool data);

    /// \brief Make this Element a Property with a boolean value
    ///
    /// @param name The name of the Property
    ///
    /// @param data The boolean to use as the value of the property.
    ///
    /// @return A reference to this Element.
    Element &makeBoolean(const std::string &name, bool data);
    
    /// \brief Make this Element an Undefined data type
    ///
    /// @return A reference to this Element.
    Element &makeUndefined();

    /// \brief Make this Element a Property with an Undefined data type.
    ///
    /// @param name The name of the Property
    ///
    /// @return A reference to this Element.
    Element &makeUndefined(const std::string &name);

    /// \brief Make this Element an NULL Object data type
    ///
    /// @return A reference to this Element.
    Element &makeNull();
    
    /// \brief Make this Element a Property with an NULL Object data type.
    ///
    /// @param name The name of the Property
    ///
    /// @return A reference to this Element.
    Element &makeNull(const std::string &name);

    /// \brief Make this Element an Object End data type
    ///
    /// @return A reference to this Element.
    Element &makeObjectEnd();

    /// \brief Make this Element as a Object data type.
    ///		This is AMF data type that supports complex objects
    ///		with properties. A Reference refers to a previously
    ///		sent ActionScript object to save on bandwidth.
    ///
    /// @param data A smart pointer to an Element to use as the value.
    ///
    /// @return A reference to this Element.
    Element &makeObject();

    /// \brief Make this Element as an Object data type.
    ///
    /// @param name The name of this object. This is not the same as
    ///		the name of a property.
    ///
    /// @return A reference to this Element.
    Element &makeObject(const std::string &name);

    /// \brief Make this Element as an Object data type.
    ///
    /// @param data A smart pointer to an Element to use as the value.
    ///
    /// @return A reference to this Element.
    Element &makeObject(std::vector<std::shared_ptr<cygnal::Element> > &data);
    
    /// \brief Make this Element a Property with an Object as the value.
    ///
    /// @param name The name of the Property
    ///
    /// @param data A smart pointer to an Element to use as the value.
    ///
    /// @return A reference to this Element.
    Element &makeObject(const std::string &name, std::vector<std::shared_ptr<cygnal::Element> > &data);
    
    /// \brief Make this Element as an XML Object data type.
    ///		This is like a string object, but the type is different.
    ///
    /// @return A reference to this Element.
    Element &makeXMLObject();
    
    /// \brief Make this Element a Property with an XML Object as the value.
    ///
    /// @param name The name of the Property
    ///
    /// @param data The boolean to use as the value of the property.
    ///
    /// @return A reference to this Element.
    Element &makeXMLObject(const std::string &name);

    /// \brief Make this Element a Property with an XML Object as the value.
    ///
    /// @param name The name of the Property
    ///
    /// @param data A smart pointer to an Element to use as the value.
    ///
    /// @return A reference to this Element.
    Element &makeXMLObject(const std::string &name, const std::string &data);
    Element &makeXMLObject(boost::uint8_t *data);

    /// \brief Make this Element a Property with an ECMA Array as the value.
    ///		This is a mixed array of any AMF types. These are stored
    ///		the same as an object, but with a different type.
    ///
    /// @return A reference to this Element.
    Element &makeECMAArray();

    /// \brief Make this Element a Property with an ECMA Array as the value.
    ///
    /// @param name The name of the Property
    ///
    /// @return A reference to this Element.
    Element &makeECMAArray(const std::string &name);

    /// \brief Make this Element a Property with an ECMA Array as the value.
    ///		This is a mixed array of any AMF types. These are stored
    ///		the same as an object, but with a different type.
    ///
    /// @param data A smart pointer to a vector of Elements to use as the vaule.
    ///
    /// @return A reference to this Element.
    Element &makeECMAArray(std::vector<std::shared_ptr<cygnal::Element> > &data);

    /// \brief Make this Element a Property with an ECMA Array as the value.
    ///
    /// @param name The name of the Property
    ///
    /// @param data A smart pointer to a vector of Elements to use as the vaule.
    ///
    /// @return A reference to this Element.
    Element &makeECMAArray(const std::string &name, std::vector<std::shared_ptr<cygnal::Element> > &data);

    /// \brief Make this Element a Property with an Strict Array as the value.
    ///		This is an array of a single AMF type. These are stored
    ///		the same as an object, but with a different type.
    ///
    /// @return A reference to this Element.
    Element &makeStrictArray();

    /// \brief Make this Element a Property with an Strict Array as the value.
    ///		This is an array of a single AMF type. These are stored
    ///		the same as an object, but with a different type.
    ///
    /// @param name The name of the Property
    ///
    /// @return A reference to this Element.
    Element &makeStrictArray(const std::string &name);

    /// \brief Make this Element a Property with an ECMA Array as the value.
    ///		This is an array of a single AMF type. These are stored
    ///		the same as an object, but with a different type.
    ///
    /// @param data A smart pointer to a vector of Elements to use as the vaule.
    ///
    /// @return A reference to this Element.
    Element &makeStrictArray(std::vector<std::shared_ptr<cygnal::Element> > &data);

    /// \brief Make this Element a Property with an Strict Array as the value.
    ///
    /// @param name The name of the Property
    ///
    /// @param data A smart pointer to a vector of Elements to use as the vaule.
    ///
    /// @return A reference to this Element.
    Element &makeStrictArray(const std::string &name, std::vector<std::shared_ptr<cygnal::Element> > &data);

    /// \brief Make this Element a Property with an Typed Object as the value.
    ///
    /// @param name The name of the Property
    ///
    /// @return A reference to this Element.    
    Element &makeTypedObject();
    Element &makeTypedObject(const std::string &name);

    /// \brief Make this Element a Property with an Typed Object as the value.
    ///
    /// @param data A real pointer to the raw data to use as the value.
    ///
    /// @param size The number of bytes to use as the value.
    ///
    /// @return A reference to this Element.
    Element &makeTypedObject(boost::uint8_t *data);
    
    /// \brief Make this Element a Property with an Object Reference as the value.
    ///
    /// @return A reference to this Element.
    Element &makeReference();
    Element &makeReference(boost::uint16_t index);

    /// \brief Make this Element a Property with an Object Reference as the value.
    ///
    /// @param data A real pointer to the raw data to use as the value.
    ///
    /// @param size The number of bytes to use as the value.
    ///
    /// @return A reference to this Element.
    Element &makeReference(boost::uint8_t *data, size_t size);
    
    /// \brief Make this Element a Property with a Movie Clip (SWF data) as the value.
    ///
    /// @return A reference to this Element.
    Element &makeMovieClip();

    /// \brief Make this Element a Property with a Movie Clip (SWF data) as the value.
    ///
    /// @param data A real pointer to the raw data to use as the value.
    ///
    /// @param size The number of bytes to use as the value.
    ///
    /// @return A reference to this Element.
    Element &makeMovieClip(boost::uint8_t *data, size_t size);

    /// \brief Make this Element a Property with a UTF8 String as the value.
    ///
    /// @return A reference to this Element.
    Element &makeLongString();
    
    /// \brief Make this Element a Property with a UTF8 String as the value.
    ///
    /// @param data A real pointer to the raw data to use as the value.
    ///
    /// @param size The number of bytes to use as the value.
    ///
    /// @return A reference to this Element.
    Element &makeLongString(boost::uint8_t *data);
    
    /// \brief Make this Element a Property with a Record Set as the value.
    ///
    /// @return A reference to this Element.
    Element &makeRecordSet();

    /// \brief Make this Element a Property with a Record Set as the value.
    ///
    /// @param data A real pointer to the raw data to use as the value.
    ///
    /// @param size The number of bytes to use as the value.
    ///
    /// @return A reference to this Element.
    Element &makeRecordSet(boost::uint8_t *data);
    
    /// \brief Make this Element a Property with a Date as the value.
    ///
    /// @return A reference to this Element.
    Element &makeDate();

    /// \brief Make this Element a Property with a Date as the value.
    ///
    /// @param data A real pointer to the raw data to use as the value.
    ///
    /// @return A reference to this Element.
    Element &makeDate(boost::uint8_t *data);
    Element &makeDate(double data);
    
    /// \brief Make this Element a Property with an Unsupported value.
    ///
    /// @return A reference to this Element.
    Element &makeUnsupported();

    /// \brief Make this Element a Property with an Unsupported value.
    ///
    /// @param data A real pointer to the raw data to use as the value.
    ///
    /// @param size The number of bytes to use as the value.
    ///
    /// @return A reference to this Element.
    Element &makeUnsupported(boost::uint8_t *data);
    
    /// \brief Test equivalance against another Element.
    ///		This compares all the data and the data type in the
    ///		current Element with the supplied one, so it can be a
    ///		performance hit. This is primarily only used for
    ///		testing purposes.
    ///
    /// @param buf A reference to an Element.
    ///
    /// @return A boolean true if the Elements are indentical.
    bool operator==(Element &);
    
    /// \brief Test equivalance against another Element.
    ///		This compares all the data and the data type in the
    ///		current Element with the supplied one, so it can be a
    ///		performance hit. This is primarily only used for
    ///		testing purposes.
    ///
    /// @param buf A smart pointer to an Element.
    ///
    /// @return A boolean true if the Elements are indentical.
    bool operator==(std::shared_ptr<cygnal::Element> );
    
    /// \brief Test equivalance against a boolean value
    ///		This compares all the data and the data type in the
    ///		current Element to see if it is a Boolean and if the
    ///		values ard the same. This is primarily only used for
    ///		testing purposes.
    ///
    /// @param buf A boolean value
    ///
    /// @return A boolean true if the Elements are indentical.
    bool operator==(bool x);

    /// \brief Get the Element or Property at a specified location.
    ///
    /// @param index The location as a numerical value of the item in
    ///		the array to get.
    ///
    /// @return A smart pointer to the Element or property.
    std::shared_ptr<cygnal::Element> operator[](size_t index);

    /// \brief Get the size in bytes of the Element's data.
    ///		All data in an Element is stored in a Buffer class.
    ///
    /// @return the size in bytes.
    size_t getDataSize() const;
    
    /// \brief Get the data type of this Element
    ///
    /// @return The data type.
    amf0_type_e getType() const { return _type; };
    
    /// \brief Get the data type of this Element
    ///
    /// @param type The data type to use as the value.
    ///
    /// @return nothing.
    void setType(amf0_type_e type) { _type = type; };
//    void setData(Buffer *buf) { _buffer = buf; };

    /// \brief Cast the data in this Element to a boolean value.
    ///
    /// @return boolean value.
    bool to_bool() const;
    
    /// \brief Cast the data in this Element to a double value.
    ///
    /// @return double value.
    double to_number() const;

    /// \brief Cast the data in this Element to a short (2 bytes) value.
    ///
    /// @return short (2 bytes) value.
    boost::uint16_t to_short() const;

    /// \brief Cast the data in this Element to an integer (4 bytes) value.
    ///
    /// @return integer (4 bytes) value.
    boost::uint32_t to_integer() const;

    /// \brief Cast the data in this Element to an ASCII string value.
    ///
    /// @return A NULL terminated ASCII string.
    const char *to_string() const;

    /// \brief Cast the data in this Element to an real pointer to data.
    ///
    /// @return A real pointer to the base address of the raw data in memory.
    boost::uint8_t *to_reference();
    const boost::uint8_t *to_reference() const;

    // Manipulate the name of a property

    /// \brief Get the number of bytes in the name of this Element.
    ///		Only top level Objects or properties have a name.
    ///
    /// @return The size of the name string.
    size_t getNameSize() const;
    
    /// \brief Get the name of this Element.
    ///		Only top level Objects or properties have a name.
    ///
    /// @return The size of the name string.
    char *getName() const { return _name; };

    /// \brief Set the name of this Element or property.
    ///		Only top level Objects or properties have a name.
    ///
    /// @param str the name to use for this Element.
    /// 
    /// @return nothing.
    void setName(const std::string &name);
    /// \overload setName(const char *name, size_t x)
    void setName(const char *name, size_t x);

    /// \brief Set the name of this Element or property.
    ///		Only top level Objects or properties have a name.
    ///
    /// @param name A real pointer to the raw bytes to use as the name for this Element.
    ///
    /// @param size The number of bytes to use for the name.
    ///
    /// @return nothing.
    ///
    /// @remarks This add a NULL string terminator so the name can be printed.
    void setName(boost::uint8_t *name, size_t size);

    // Manipulate the children Elements of an object

    /// \brief Find the named property for this Object.
    ///
    /// @param name An ASCII string that is the name of the property to
    ///		search for.
    ///
    /// @return A smart pointer to the Element for this property.
    std::shared_ptr<Element> findProperty(const std::string &name);

    /// \brief Find the property at this index for this Object.
    ///
    /// @param index The index of the property in the array of data.
    ///
    /// @return A smart pointer to the Element for this property.
    std::shared_ptr<Element> getProperty(size_t index) const { return _properties[index]; };

    /// \brief Add a Property to the array of properties for this object.
    ///
    /// @param el A smart pointer to the Element for this Property.
    ///
    /// @return nothing.
    void addProperty(std::shared_ptr<Element> el) { _properties.push_back(el); };

    void clearProperties() { return _properties.clear(); };

    /// \brief Get a smart pointer to the Element for this Property.
    ///
    /// @return A smart pointer to the Element for this Property
    ///
    /// @remarks This does not remove the Element from array of
    ///		properties.
    std::shared_ptr<Element> popProperty()
			{ return _properties.front(); };

    /// \brief Get the count of properties for this Element.
    ///
    /// @return The number of Properties associated with this object.
    size_t propertySize() const
			{ return _properties.size(); };

    /// \brief Encode this Element (data type object).
    ///		This encodes this Element and all of it's associated
    ///		properties into raw binary data in big endoan format.
    ///
    /// @param notobject Flag to not encode the element as an object,
    ///		instead it's just a list of properties. This is used when
    ///		formatting onStatus response packets.
    ///
    /// @return a smart pointer to a Buffer class.
    std::shared_ptr<Buffer> encode();
    std::shared_ptr<Buffer> encode(bool notobject);

    /// \brief Get the array of properties for this Element.
    ///
    /// @return A smart pointer to a vector of Elements.
    ///
    /// @remarks This is only intended to be used for testing and
    ///		debugging purposes.
    std::vector<std::shared_ptr<Element> > getProperties() const
			{ return _properties; };

    size_t calculateSize();
    size_t calculateSize(cygnal::Element &el) const;

    ///  \brief Dump the internal data of this class in a human readable form.
    /// @remarks This should only be used for debugging purposes.
    void dump() const { dump(std::cerr); }
    /// \overload dump(std::ostream& os) const
    void dump(std::ostream& os) const;
    
private:
    /// \brief Make sure the Buffer used for storing data is big enough.
    ///		This will force a Buffer::resize() is the existing
    ///		Buffer used to store the data isn't big enough to hold
    ///		the new size.
    ///
    /// @param size The minimum size the buffer needs to be.
    ///
    /// @return nothing
    void check_buffer(size_t size);

    /// \var _name
    ///		Only two types of Elements have names, Properties, or
    ///		the top level object that contains properties.
    char		*_name;
    
    /// \var _buffer
    ///		A smart pointer to the Buffer used to hold the data
    ///		for this Element.
    std::shared_ptr<cygnal::Buffer> _buffer;

    /// \var _type
    ///		The AMF0 data type of this Element.
    amf0_type_e		_type;

    /// \var _properties
    ///		The vector of properties stored for this Element if
    ///		it's a top level object.
    std::vector<std::shared_ptr<Element> > _properties;
};                              // end of class definition


/// \brief Dump to the specified output stream.
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
// indent-tabs-mode: nil
// End:
