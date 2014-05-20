// amf.cpp:  AMF (Action Message Format) rpc marshalling, for Gnash.
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
//

#include <string>
#include <vector>
#include <cmath>
#include <climits>
#include <cstdint> // for boost::?int??_t

#include "buffer.h"
#include "log.h"
#include "amf.h"
#include "amfutf8.h"
#include "utility.h"
#include "element.h"
#include "GnashException.h"

using std::string;
using gnash::log_error;

/// \namespace cygnal
///
/// This namespace is for all the AMF specific classes in libamf.
namespace cygnal
{

/// \brief This is used to print more intelligent debug messages
const char *astype_str[] = {
    "Number",
    "Boolean",
    "String",
    "Object",
    "MovieClip",
    "Null",
    "Undefined",
    "Reference",
    "ECMAArray",
    "ObjectEnd",
    "StrictArray",
    "Date",
    "LongString",
    "Unsupported",
    "Recordset",
    "XMLObject",
    "TypedObject",
    "AMF3 Data"
};

/// \brief Create a new Element with no data type.
Element::Element()
    : _name(0),
      _type(NOTYPE)
{
//    GNASH_REPORT_FUNCTION;
}

/// \brief Delete this Element
///		This deallocates all the memory used to hold the data.
Element::~Element()
{
//    GNASH_REPORT_FUNCTION;
    delete[] _name;
}

/// \brief Construct an AMF Element from a double..
///
/// @param data The double to use as the value for this Element.
Element::Element(double indata)
    : _name(0),
      _type(NOTYPE)
{
//    GNASH_REPORT_FUNCTION;
    makeNumber(indata);
}

/// \brief Contruct a Property from a double.
///
/// @param name The name of the Property
///
/// @param num The double to use as the value of the property.
Element::Element(const string &name, double num)
    : _name(0),
      _type(NOTYPE)
{
//    GNASH_REPORT_FUNCTION;
    makeNumber(name, num);
}

/// \brief Contruct an AMF Element from an ASCII string.
///
/// @param data The ASCII string to use as the value of the property.
///
/// @remarks This assume the data string is already NULL terminated.
Element::Element(const string &indata)
    : _name(0),
      _type(NOTYPE)
{
//    GNASH_REPORT_FUNCTION;
    makeString(indata);
}

/// \overload Element(const std::string &data)
Element::Element(const char *indata)
    : _name(0),
      _type(NOTYPE)
{
//    GNASH_REPORT_FUNCTION;
    makeString(indata);
}

/// \brief Contruct a Property from an ASCII string.
///
/// @param name The name of the Property
///
/// @param data The ASCII string to use as the value of the property.
Element::Element(const string &name, const string &data)
    : _name(0),
      _type(NOTYPE)
{
//    GNASH_REPORT_FUNCTION;
    makeString(name, data);
}

/// \brief Contruct an AMF Element from a Boolean
///
/// @param data The boolean to use as the value for this Element.
Element::Element(bool data)
    : _name(0),
      _type(NOTYPE)
{
//    GNASH_REPORT_FUNCTION;
    makeBoolean(data);
}

/// \brief Contruct a Property from a Boolean
///
/// @param name The name of the Property
///
/// @param data The boolean to use as the value of the property.
Element::Element(const string &name, bool indata)
    : _name(0),
      _type(NOTYPE)
{
//    GNASH_REPORT_FUNCTION;
    makeBoolean(name, indata);
}

#if 0
// Create a function block for AMF
Element::Element(bool /* flag */, double /* unknown1 */, double /* unknown2 */,
		 const string &/* methodname */)
    : _name(0),
      _type(NOTYPE),
      _referenceid(0)
{
//    GNASH_REPORT_FUNCTION;
    log_unimpl(_("Can't create remote function calls yet"));
}

Element &
Element::init(bool flag, double unknown1, double unknown2,
	      const string &methodname)
{
//    GNASH_REPORT_FUNCTION;
//    _type = Element::FUNCTION_AMF0;
    if (methodname.size()) {
	setName(methodname);
    }

    // Build up the properties for the function block
    shared_ptr<cygnal::Element> el = new Element(flag);
    _properties.push_back(el);
    
    el = new Element(unknown1);
    _properties.push_back(el);
    
    el = new Element(unknown2);
    _properties.push_back(el);
    
    el = new Element(methodname);
    _properties.push_back(el);
    
    _buffer = new Buffer(3
	+ ((AMF_HEADER_SIZE + AMF0_NUMBER_SIZE) * 2)
	       + methodname.size() + AMF_HEADER_SIZE);
//     memcpy(_data, &indata, _length);
    return *this;
}
#endif

/// \brief Clear the contents of the buffer by setting all the bytes to
///		zeros.
///
/// @return nothing
void
Element::clear()
{
//    GNASH_REPORT_FUNCTION;
	delete[] _name;
	_name = 0;
	_buffer.reset();
}

/// \brief Get the size in bytes of the Element's data.
///	All data in an Element is stored in a Buffer class.
///
/// @return the size in bytes.
size_t
Element::getDataSize() const
{
//    GNASH_REPORT_FUNCTION;
    if (_buffer) {
	return _buffer->size();
    }
    return 0;
}

/// \brief Cast the data in this Element to a double value.
///
/// @return double value.
double
Element::to_number() const
{
//    GNASH_REPORT_FUNCTION;
    if (_buffer) {
	return *(reinterpret_cast<double *>(_buffer->reference()));
    }
//    return ::nan("NaN");
    return -1.0;
}

/// \brief Cast the data in this Element to a short value.
///
/// @return short value.
std::uint16_t
Element::to_short() const
{
//    GNASH_REPORT_FUNCTION;
    if (_buffer) {
	return *(reinterpret_cast<std::uint16_t *>(_buffer->reference()));
    }
//    return ::nan("NaN");
    return -1;
}

/// \brief Cast the data in this Element to a short value.
///
/// @return short value.
std::uint32_t
Element::to_integer() const
{
//    GNASH_REPORT_FUNCTION;
    if (_buffer) {
	return *(reinterpret_cast<std::uint32_t *>(_buffer->reference()));
    }
//    return ::nan("NaN");
    return -1;
}

/// \brief Cast the data in this Element to an ASCII string value.
///
/// @return A NULL terminated ASCII string.
const char *
Element::to_string() const
{
//    GNASH_REPORT_FUNCTION;
    if (_buffer) {
	if (_buffer->size() > 0) {
	    return reinterpret_cast<const char *>(_buffer->reference());
	}
	return "NULL";
    }
    return 0;
}

/// \brief Cast the data in this Element to a boolean value.
///
/// @return boolean value.
bool
Element::to_bool() const
{
//    GNASH_REPORT_FUNCTION;
    if (_buffer) {
	return *(reinterpret_cast<bool *>(_buffer->reference()));
    }
    return false;
}

/// \brief Cast the data in this Element to an real pointer to data.
///
/// @return A real pointer to the base address of the raw data in memory.
std::uint8_t *
Element::to_reference()
{
//    GNASH_REPORT_FUNCTION;
    if (_buffer) {
	return _buffer->reference();
    }
    return 0;
}

const std::uint8_t *
Element::to_reference() const
{
//    GNASH_REPORT_FUNCTION;
    if (_buffer) {
	return _buffer->reference();
    }
    return 0;
}

/// \brief Test equivalance against another Element.
///	This compares all the data and the data type in the
///	current Element with the supplied one, so it can be a
///	performance hit. This is primarily only used for
///	testing purposes.
///
/// @param buf A reference to an Element.
///
/// @return A boolean true if the Elements are indentical.
bool
Element::operator==(Element &el)
{
//    GNASH_REPORT_FUNCTION;
    int count = 0;

    // See if the names match
    if (_name) {
	if (strcmp(_name, el.getName()) == 0) {
	    count++;
	}
    } else {
	if (el.getNameSize() == 0) {
	    count++;
	}
    }

    // See if the types match
    if (_type == el.getType()) {
	count++;
    }

    if (_buffer && el.getDataSize()) {
	if (memcmp(_buffer->reference(), el.to_reference(), _buffer->size()) == 0) {
	    count++;
	}
    } else {
	count++;
    }
    
    // FIXME: make this test more exhaustive
    if (_properties.size() == el.propertySize()) {
	count++;
    }

    if (count == 4) {
	return true;
    }
    return false;;
}

/// \brief Test equivalance against a boolean value
///	This compares all the data and the data type in the
///	current Element to see if it is a Boolean and if the
///	values ard the same. This is primarily only used for
///	testing purposes.
///
/// @param buf A boolean value
///
/// @return A boolean true if the Elements are indentical.
bool
Element::operator==(bool x)
{
//    GNASH_REPORT_FUNCTION;
    if (_buffer) {
	*_buffer += x;
    }
    return false;
}

size_t
Element::calculateSize()
{
//    GNASH_REPORT_FUNCTION;
    return calculateSize();
}

size_t
Element::calculateSize(cygnal::Element &el) const
{
//    GNASH_REPORT_FUNCTION;    
    size_t outsize = 0;

    // Simple Elements have everything contained in just the class itself.
    // If thr name is set, it's a property, so the length is
    // prefixed to the name string.
    if (el.getNameSize()) {
	outsize += el.getNameSize() + sizeof(std::uint16_t);
    }
    // If there is any data, then the size of the data plus the header
    // of the type and the length is next.
    if (el.getDataSize()) {
	outsize += el.getDataSize() + AMF_HEADER_SIZE;
    }

    // If an array has no data, it's undefined, so has a length of zero.
    if (el.getType() == Element::STRICT_ARRAY_AMF0) {
	if (el.getDataSize() == 0) {
	    outsize = sizeof(std::uint32_t) + 1;
	}
    }
    
    // More complex messages have child elements, either properties or
    // the items in an array, If we have children, count up their size too.
    // Calculate the total size of the message
    std::vector<std::shared_ptr<cygnal::Element> > props = el.getProperties();
    for (size_t i=0; i<props.size(); i++) {
	outsize += props[i]->getDataSize();
	if (props[i]->getNameSize()) {
	    outsize += props[i]->getNameSize();
	    outsize += cygnal::AMF_PROP_HEADER_SIZE;
	} else {
	    outsize += cygnal::AMF_HEADER_SIZE;
	}
    }

    return outsize;
}

/// \brief Encode this Element (data type object).
///	This encodes this Element and all of it's associated
///	properties into raw binary data in big endoan format.
///
/// @return a smart pointer to a Buffer class.
std::shared_ptr<Buffer>
Element::encode()
{
//    GNASH_REPORT_FUNCTION;

    return encode(false);
}

std::shared_ptr<Buffer>
Element::encode(bool notobject)
{
//    GNASH_REPORT_FUNCTION;
    size_t size = 0;
    std::shared_ptr<Buffer> buf;

    if (_type == Element::OBJECT_AMF0) {
	// Calculate the total size of the output buffer
	// needed to hold the encoded properties
	if (_name) {
	    size = getNameSize() + AMF_HEADER_SIZE;
	}
	for (size_t i=0; i<_properties.size(); i++) {
	    size += _properties[i]->getDataSize();
	    size += _properties[i]->getNameSize();
	    size += AMF_PROP_HEADER_SIZE;
	}
	gnash::log_debug(_("Size of Element \"%s\" is: %d"), _name, size);
	buf.reset(new Buffer(size+AMF_PROP_HEADER_SIZE));
	if (!notobject) {
	    *buf = Element::OBJECT_AMF0;
	}
	if (_name > static_cast<char *>(0)) {
	    size_t length = getNameSize();
	    std::uint16_t enclength = length;
	    swapBytes(&enclength, 2);
	    *buf += enclength;
	    string str = _name;
	    *buf += str;
	    std::uint8_t byte = static_cast<std::uint8_t>(0x5);
	    *buf += byte;
	}

	for (size_t i=0; i<_properties.size(); i++) {
	    std::shared_ptr<Buffer> partial = AMF::encodeElement(_properties[i]);
//	    log_debug("Encoded partial size for is %d", partial->size());
// 	    _properties[i]->dump();
// 	    partial->dump();
	    if (partial) {
		*buf += partial;
		partial.reset();
	    } else {
		break;
	    }
	}
//	log_debug("FIXME: Terminating object");
	if (!notobject) {
	    std::uint8_t pad = 0;
	    *buf += pad;
	    *buf += pad;
	    *buf += TERMINATOR;
	}
	return buf;
    } else {
	    return AMF::encodeElement(*this);
    }
    
    return buf;
}

/// \brief Get the Element or Property at a specified location.
///
/// @param index The location as a numerical value of the item in
///		the array to get.
///
/// @return A smart pointer to the Element or property.
std::shared_ptr<Element>
Element::operator[](size_t index)
{
//    GNASH_REPORT_FUNCTION;
    if (index <= _properties.size()) {
	return _properties[index];
    }
    
    std::shared_ptr<Element> el;
    return el;
}

/// \brief Make this Element be the same as another Element.
///
/// @param el A reference to an Element class.
///
/// @return A reference to this Element.
Element &
Element::operator=(Element &el)
{
//    GNASH_REPORT_FUNCTION;
    return operator=(&el);
//    GNASH_REPORT_FUNCTION;
    _type = el.getType();
    if (el.getNameSize()) {
	setName(el.getName());
    }
    _buffer.reset(new Buffer(el.getDataSize()));
    _buffer->copy(el.to_reference(), el.getDataSize());
    return *this;
}

/// \brief Make this Element be the same as a double.
///		This sets both the data type and the value.
///
/// @param el A double value.
///
/// @return A reference to this Element.
Element &
Element::operator=(double num)
{
    return makeNumber(num);
}

/// \brief Make this Element be the same as an ASCII string.
///		This sets both the data type and the value.
///
/// @param el An ASCII string value.
///
/// @return A reference to this Element.
Element &
Element::operator=(const string &str)
{
    return makeString(str);
}

/// \brief Make this Element be the same as a boolean value.
///		This sets both the data type and the value.
///
/// @param el A boolean value.
///
/// @return A reference to this Element.
Element &
Element::operator=(bool flag)
{
    return makeBoolean(flag);
}

/// \note All Numbers are 64 bit, big-endian (network byte order)
///	entities. All strings are in multibyte format, which is to
///	say, probably normal ASCII. It may be that these need to be
///	converted to wide characters, but for now we just leave them
///	as standard multibyte characters.

/// \brief Make this Element with an ASCII string value.
///
/// @param data The ASCII string to use as the value.
///
/// @param size The number of bytes in the ASCII string.
///
/// @return A reference to this Element.
Element &
Element::makeString(std::uint8_t *data, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::STRING_AMF0;

    // If there is an existing string, 
    if (_buffer) {
	if (_buffer->size() < size) {
	    _buffer->resize(size+1); // add room for the NULL terminator
	}
    } else {
	// Make room for an additional NULL terminator
	try {
	    check_buffer(size+1);
	} catch (std::exception& e) {
	    log_error("%s", e.what());
	    return *this;
	}
    }
    _buffer->clear();		// FIXME: this could be a performance issue
    _buffer->copy(data, size);
    
    // Unlike other buffers, people like to print strings, so we must add
    // a NULL terminator to the string. When encoding, we are careful to
    // to adjust the byte count down by one, as the NULL terminator doesn't
    // get written.
//     *(_buffer->end() - 1) = 0;
    _buffer->setSize(size);
    return *this;
}

/// \brief Make this Element be a NULL String type.
///	A Null String is a string with a length of zero. The
///	data is only one byte, which always has the value of
///	zero of course.
///
/// @return A reference to this Element.
Element &
Element::makeNullString()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::STRING_AMF0;
    try {
	check_buffer(sizeof(std::uint8_t));
    } catch (std::exception& e) {
	log_error("%s", e.what());
	return *this;
    }
    
    *(_buffer->reference()) = 0;
    return *this;
}

/// \brief Make this Element with an ASCII string value.
///
/// @param str The ASCII string to use as the value.
///
/// @param size The number of bytes in the ASCII string.
///
/// @return A reference to this Element.
Element &
Element::makeString(const char *str, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::STRING_AMF0;
    std::uint8_t *ptr = reinterpret_cast<std::uint8_t *>(const_cast<char *>(str));
    return makeString(ptr, size);
}

/// \brief Make this Element with an ASCII string value.
///
/// @param str The ASCII string to use as the value.
///
/// @return A reference to this Element.
Element &
Element::makeString(const string &str)
{
//    GNASH_REPORT_FUNCTION;
    return makeString(str.c_str(), str.size());
}

/// \brief Make this Element a Property with an ASCII String value.
///
/// @param name The name of the Property
///
/// @param str The ASCII string to use as the value of the property.
///
/// @return A reference to this Element.
Element &
Element::makeString(const string &name, const string &str)
{
//    GNASH_REPORT_FUNCTION;
    if (name.size()) {
        setName(name);
    }    
    return makeString(str.c_str(), str.size());
}

/// \brief Make this Element with a double value.
///
/// @param buf A smart pointer to a Buffer class.
///
/// @return A reference to this Element.
Element &
Element::makeNumber(std::shared_ptr<cygnal::Buffer> buf)
{
//    GNASH_REPORT_FUNCTION;
    return makeNumber(buf->reference());
}

/// \brief Make this Element with a double value.
///		The size isn't needed as a double is always the same size.
///
/// @param str The double to use as the value.
///
/// @return A reference to this Element.
Element &
Element::makeNumber(std::uint8_t *data)
{
//    GNASH_REPORT_FUNCTION;
    double num = *reinterpret_cast<const double*>(data);
    _type = Element::NUMBER_AMF0;
    try {
	check_buffer(AMF0_NUMBER_SIZE);
    } catch (std::exception& e) {
	log_error("%s", e.what());
	return *this;
    }
    
    *_buffer = num;
    
    return *this;
}

/// \brief Make this Element with a double value.
///
/// @param str The double to use as the value.
///
/// @return A reference to this Element.
Element &
Element::makeNumber(double num)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::NUMBER_AMF0;
    try {
	check_buffer(AMF0_NUMBER_SIZE);
    } catch (std::exception& e) {
	log_error("%s", e.what());
	return *this;
    }
    
    *_buffer = num;

    return *this;
}

/// \brief Make this Element a Property with a double value
///
/// @param name The name of the Property
///
/// @param num The double to use as the value of the property.
Element &
Element::makeNumber(const string &name, double num)
{
//    GNASH_REPORT_FUNCTION;
    if (name.size()) {
        setName(name);
    }
    return makeNumber(num);
}

/// \overload Element::makeNumber(const std::string &name, std::uint8_t *data);
///		The size isn't needed as a double is always the same size.
Element &
Element::makeNumber(const std::string &name, std::uint8_t *data)
{
//    GNASH_REPORT_FUNCTION;
    if (name.size()) {
        setName(name);
    }
    _type = Element::NUMBER_AMF0;
    try {
	check_buffer(AMF0_NUMBER_SIZE);
    } catch (std::exception& e) {
	log_error("%s", e.what());
	return *this;
    }
    
    *_buffer = data;
    return *this;
}

/// \brief Make this Element with a boolean value.
///
/// @param data A boolean to use as the value.
///
/// @return A reference to this Element.
Element &
Element::makeBoolean(bool flag)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::BOOLEAN_AMF0;
    try {
	check_buffer(1);
    } catch (std::exception& e) {
	log_error("%s", e.what());
	return *this;
    }
    
    *_buffer = flag;

    return *this;
}

/// \brief Make this Element a Property with a boolean value
///
/// @param name The name of the Property
///
/// @param data The boolean to use as the value of the property.
///
/// @return A reference to this Element.
Element &
Element::makeBoolean(const string &name, bool flag)
{
//    GNASH_REPORT_FUNCTION;
    if (name.size()) {
        setName(name);
    }
    return makeBoolean(flag);
}

/// \brief Make this Element with a boolean value.
///	The size isn't needed as a boolean is always the same size.
///
/// @param data A real pointer to the boolean use as the value.
///
/// @return A reference to this Element.
Element &
Element::makeBoolean(std::uint8_t *data)
{
//    GNASH_REPORT_FUNCTION;
    bool flag = *reinterpret_cast<const bool*>(data);
    
    return makeBoolean(flag);
}

/// \brief Make this Element an Undefined data type
///
/// @return A reference to this Element.
Element &
Element::makeUndefined()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::UNDEFINED_AMF0;

    return *this;
}

/// \brief Make this Element a Property with an Undefined data type.
///
/// @param name The name of the Property
///
/// @return A reference to this Element.
Element &
Element::makeUndefined(const std::string &name)
{
//    GNASH_REPORT_FUNCTION;
    if (name.size()) {
        setName(name);
    }
    return makeUndefined();
}

/// \brief Make this Element an NULL Object data type
///	A NULL AMF0 Object consists of a single byte, which is the type
///
/// @return A reference to this Element.
Element &
Element::makeNull()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::NULL_AMF0;
    return *this;
}

/// \brief Make this Element a Property with an NULL Object data type.
///
/// @param name The name of the Property
///
/// @return A reference to this Element.
Element &
Element::makeNull(const std::string &name)
{
//    GNASH_REPORT_FUNCTION;    
    if (name.size()) {
        setName(name);
    }
    return makeNull();
}

/// \brief Make this Element as a Object data type.
///	This is AMF data type that supports complex objects
///	with properties. A Reference refers to a previously
///	sent ActionScript object to save on bandwidth.
///
/// @param data A smart pointer to an Element to use as the value.
///
/// @return A reference to this Element.
Element &
Element::makeObject()
{
//    GNASH_REPORT_FUNCTION;
    _type = OBJECT_AMF0;
    return *this;
}

/// \brief Make this Element as an Object data type.
///
/// @param name The name of this object. This is not the same as
///		the name of a property.
///
/// @return A reference to this Element.
Element &
Element::makeObject(const std::string &name)
{
//    GNASH_REPORT_FUNCTION;
    if (name.size()) {
        setName(name);
    }
    return makeObject();
}

/// \brief Make this Element a Property with an Object as the value.
///
/// @param name The name of the Property
///
/// @param data A smart pointer to an Element to use as the value.
///
/// @return A reference to this Element.
Element &
Element::makeObject(const std::string &name, std::vector<std::shared_ptr<Element> > &data)
{
//    GNASH_REPORT_FUNCTION;
    _type = OBJECT_AMF0;
    if (name.size()) {
        setName(name);
    }
    return makeObject(data);
}

/// \brief Make this Element as an Object data type.
///
/// @param data A smart pointer to an Element to use as the value.
///
/// @return A reference to this Element.
Element &
Element::makeObject(std::vector<std::shared_ptr<Element> > &data)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::OBJECT_AMF0;
    std::vector<std::shared_ptr<Element> >::const_iterator ait;
    for (ait = data.begin(); ait != data.end(); ++ait) {
	std::shared_ptr<Element> el = (*(ait));
	addProperty(el);
//	el->dump(os);
    }
    return *this;
}

/// \brief Make this Element as an XML Object data type.
///	This is like a string object, but the type is different.
///
/// @return A reference to this Element.
Element &
Element::makeXMLObject()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::XML_OBJECT_AMF0;
    return *this;
}
Element &
Element::makeXMLObject(std::uint8_t * /*data*/)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::XML_OBJECT_AMF0;
    return *this;
}

/// \brief Make this Element a Property with an XML Object as the value.
///
/// @param name The name of the Property
///
/// @param data The boolean to use as the value of the property.
///
/// @return A reference to this Element.
Element &
Element::makeXMLObject(const string &data)
{
//    GNASH_REPORT_FUNCTION;
    makeString(data);
    _type = Element::XML_OBJECT_AMF0;
    return *this;
}

/// \brief Make this Element a Property with an XML Object as the value.
///
/// @param name The name of the Property
///
/// @param data A smart pointer to an Element to use as the value.
///
/// @return A reference to this Element.
Element &
Element::makeXMLObject(const string &name, const string &data)
{
//    GNASH_REPORT_FUNCTION;
    makeXMLObject(name, data);
    _type = Element::XML_OBJECT_AMF0;
    return *this;
}

/// \brief Make this Element an Object End data type
///
/// @return A reference to this Element.
Element &
Element::makeObjectEnd()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::OBJECT_END_AMF0;
    return *this;
}

/// \brief Make this Element a Property with an Typed Object as the value.
///
/// @param name The name of the Property
///
/// @return A reference to this Element.    
Element &
Element::makeTypedObject(const std::string &name)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::TYPED_OBJECT_AMF0;  
    if (name.size()) {
        setName(name);
    }
    return *this;
}

Element &
Element::makeTypedObject()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::TYPED_OBJECT_AMF0;  

    return *this;
}

/// \brief Make this Element a Property with an Typed Object as the value.
///
/// @param data A real pointer to the raw data to use as the value.
///
/// @param size The number of bytes to use as the value.
///
/// @return A reference to this Element.
Element &
Element::makeTypedObject(std::uint8_t */*data*/)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::TYPED_OBJECT_AMF0;
//     check_buffer(size);
//     _buffer->copy(data, size);
    return *this;
}

/// \brief Make this Element a Property with an Object Reference as the value.
///
/// @return A reference to this Element.
Element &
Element::makeReference()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::REFERENCE_AMF0;
    return *this;
}

Element &
Element::makeReference(std::uint16_t index)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::REFERENCE_AMF0;
    std::uint8_t *ptr = reinterpret_cast<std::uint8_t *>(&index);
    return makeReference(ptr, sizeof(std::uint16_t));
    
    return *this;
}

/// \brief Make this Element a Property with an Object Reference as the value.
///
/// @param data A real pointer to the raw data to use as the value.
///
/// @param size The number of bytes to use as the value.
///
/// @return A reference to this Element.
Element &
Element::makeReference(std::uint8_t *indata, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::REFERENCE_AMF0;
    try {
	check_buffer(size);
    } catch (std::exception& e) {
	log_error("%s", e.what());
	return *this;
    }
    
    _buffer->copy(indata, size);
    return *this;
}

/// \brief Make this Element a Property with a Movie Clip (SWF data) as the value.
///
/// @return A reference to this Element.
Element &
Element::makeMovieClip()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::MOVIECLIP_AMF0;
    return *this;
}

/// \brief Make this Element a Property with a Movie Clip (SWF data) as the value.
///
/// @param data A real pointer to the raw data to use as the value.
///
/// @param size The number of bytes to use as the value.
///
/// @return A reference to this Element.
Element &
Element::makeMovieClip(std::uint8_t *indata, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::MOVIECLIP_AMF0;
    check_buffer(size);
    _buffer->copy(indata, size);
    return *this;    
}

/// \brief Make this Element a Property with an ECMA Array as the value.
///		This is a mixed array of any AMF types. These are stored
///		the same as an object, but with a different type.
///
/// @return A reference to this Element.
Element &
Element::makeECMAArray()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::ECMA_ARRAY_AMF0;
    return *this;
}

/// \brief Make this Element a Property with an ECMA Array as the value.
///
/// @param name The name of the Property
///
/// @return A reference to this Element.
Element &
Element::makeECMAArray(const std::string &name)
{
//    GNASH_REPORT_FUNCTION;
    if (name.size()) {
        setName(name);
    }
    return makeECMAArray();
}

/// \brief Make this Element a Property with an ECMA Array as the value.
///
/// @param name The name of the Property
///
/// @param data A smart pointer to a vector of Elements to use as the vaule.
///
/// @return A reference to this Element.
Element &
Element::makeECMAArray(const std::string &name, std::vector<std::shared_ptr<cygnal::Element> > &data)
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::ECMA_ARRAY_AMF0;
    makeObject(name, data);
    _type = Element::ECMA_ARRAY_AMF0;
    return *this;
}

/// \brief Make this Element a Property with an ECMA Array as the value.
///	This is a mixed array of any AMF types. These are stored
///	the same as an object, but with a different type.
///
/// @param data A smart pointer to a vector of Elements to use as the vaule.
///
/// @return A reference to this Element.
Element &
Element::makeECMAArray(std::vector<std::shared_ptr<cygnal::Element> > &data)
{
//    GNASH_REPORT_FUNCTION;
    makeObject(data);
    _type = Element::ECMA_ARRAY_AMF0;
    return *this;
}

/// \brief Make this Element a Property with an Strict Array as the value.
///	This is an array of a single AMF type. These are stored
///	the same as an object, but with a different type.
///
/// @return A reference to this Element.
Element &
Element::makeStrictArray()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::STRICT_ARRAY_AMF0;
    return *this;
}

/// \brief Make this Element a Property with an Strict Array as the value.
///		This is an array of a single AMF type. These are stored
///		the same as an object, but with a different type.
///
/// @param name The name of the Property
///
/// @return A reference to this Element.
Element &
Element::makeStrictArray(const std::string &name)
{
//    GNASH_REPORT_FUNCTION;
    if (name.size()) {
        setName(name);
    }
    return makeStrictArray();
}

/// \brief Make this Element a Property with an Strict Array as the value.
///
/// @param name The name of the Property
///
/// @param data A smart pointer to a vector of Elements to use as the vaule.
///
/// @return A reference to this Element.
Element &
Element::makeStrictArray(const std::string &name, std::vector<std::shared_ptr<cygnal::Element> > &data)
{
//    GNASH_REPORT_FUNCTION;
    makeObject(name, data);
    _type = Element::STRICT_ARRAY_AMF0;
    return *this;
}

/// \brief Make this Element a Property with an ECMA Array as the value.
///	This is an array of a single AMF type. These are stored
///	the same as an object, but with a different type.
///
/// @param data A smart pointer to a vector of Elements to use as the vaule.
///
/// @return A reference to this Element.
Element &
Element::makeStrictArray(std::vector<std::shared_ptr<cygnal::Element> > &data)
{
//    GNASH_REPORT_FUNCTION;
    makeObject(data);
    _type = Element::STRICT_ARRAY_AMF0;
    return *this;
}

/// \brief Make this Element a Property with an Unsupported value.
///
/// @return A reference to this Element.
Element &
Element::makeUnsupported()
{
//    GNASH_REPORT_FUNCTION;    
    _type = Element::UNSUPPORTED_AMF0;
    return *this;
}

/// \brief Make this Element a Property with an Unsupported value.
///
/// @param data A real pointer to the raw data to use as the value.
///
/// @param size The number of bytes to use as the value.
///
/// @return A reference to this Element.
Element &
Element::makeUnsupported(std::uint8_t *data)
{
    UNUSED(data);
    _type = Element::UNSUPPORTED_AMF0;
    return *this;
}

/// \brief Make this Element a Property with a UTF8 String as the value.
///
/// @return A reference to this Element.
Element &
Element::makeLongString()
{
//    GNASH_REPORT_FUNCTION;    
    _type = Element::LONG_STRING_AMF0;
    return *this;
}

/// \brief Make this Element a Property with a UTF8 String as the value.
///
/// @param data A real pointer to the raw data to use as the value.
///
/// @param size The number of bytes to use as the value.
///
/// @return A reference to this Element.
Element &
Element::makeLongString(std::uint8_t *indata)
{
    UNUSED(indata);
    _type = Element::LONG_STRING_AMF0;
//     check_buffer(size);
//     _buffer->copy(indata, size);
    return *this;
}

/// \brief Make this Element a Property with a Record Set as the value.
///
/// @return A reference to this Element.
Element &
Element::makeRecordSet()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::RECORD_SET_AMF0;
    return *this;
}
Element &
Element::makeRecordSet(std::uint8_t *data)
{
    UNUSED(data);
    _type = Element::RECORD_SET_AMF0;
    return *this;
}

/// \brief Make this Element a Property with a Date as the value.
///
/// @param data A real pointer to the raw data to use as the value.
///
/// @return A reference to this Element.
Element &
Element::makeDate()
{
//    GNASH_REPORT_FUNCTION;
    _type = Element::DATE_AMF0;

    return *this;
}

Element &
Element::makeDate(std::uint8_t *date)
{
//    GNASH_REPORT_FUNCTION;

    makeNumber(date);
    _type = Element::DATE_AMF0;

    return *this;
}

Element &
Element::makeDate(double date)
{
//    GNASH_REPORT_FUNCTION;
    //std::uint8_t *ptr = reinterpret_cast<std::uint8_t *>(&date);
    _type = Element::DATE_AMF0;
    try {
	check_buffer(AMF0_NUMBER_SIZE);
    } catch (std::exception& e) {
	log_error("%s", e.what());
	return *this;
    }
    
    *_buffer = date;

    return *this;
}

/// \brief Get the number of bytes in the name of this Element.
///	Only top level Objects or properties have a name.
///
/// @return The size of the name string.
size_t
Element::getNameSize() const
{
//    GNASH_REPORT_FUNCTION;
    if (_name) {
	return strlen(_name);
    }
    return 0;
}

/// \brief Set the name of this Element or property.
///		Only top level Objects or properties have a name.
///
/// @param str the name to use for this Element.
/// 
/// @return nothing.
void
Element::setName(const string &str)
{
//    GNASH_REPORT_FUNCTION;
    _name = new char[str.size() + 1];
    std::copy(str.begin(), str.end(), _name);
    *(_name + str.size()) = 0;
}

/// \brief Set the name of this Element or property.
///		Only top level Objects or properties have a name.
///
/// @param name A real pointer to the raw bytes to use as the name for this Element.
///
/// @param size The number of bytes to use for the name.
///
/// @return nothing.
///
/// @remarks This adds a NULL string terminator so the name can be printed.
void
Element::setName(const char *name, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    std::uint8_t *ptr = reinterpret_cast<std::uint8_t *>(const_cast<char *>(name));
    return setName(ptr, size);
}

/// \brief Set the name of this Element or property.
///		Only top level Objects or properties have a name.
///
/// @param name A real pointer to the raw bytes to use as the name for this Element.
///
/// @param size The number of bytes to use for the name.
///
/// @return nothing.
///
/// @remarks This adds a NULL string terminator so the name can be printed.
void
Element::setName(std::uint8_t *name, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    if ((size > 0) && (name != 0)) {
	_name = new char[size+1];
	std::copy(name, name+size, _name);
	*(_name + size) = 0;
    }
}

/// \brief Make sure the Buffer used for storing data is big enough.
///		This will force an exception if the Buffer used to
//		store the data isn't big enough to hold	the new size.
///
/// @param size The minimum size the buffer needs to be.
///
/// @return nothing
void
Element::check_buffer(size_t size)
{
//    GNASH_REPORT_FUNCTION;
    if (_buffer == 0) {
	_buffer.reset(new Buffer(size));
    } else {
	if (_buffer->size() < size) {
	    throw gnash::ParserException("Buffer not big enough, try resizing!");
	}
	if (_buffer->size() == 0) {
	    throw gnash::ParserException("Buffer has zero size, not initialized!");
	}
    }
}

///  \brief Dump the internal data of this class in a human readable form.
/// @remarks This should only be used for debugging purposes.
void
Element::dump(std::ostream& os) const
{
//    GNASH_REPORT_FUNCTION;
    
    os << astype_str[_type] << ": ";
    if (_name) {
 	os << " property name is: \"" << _name << "\", ";
    } else {
 	os << "(no name), ";
    }
    os << "data length is " << getDataSize() << std::endl;


    switch (_type) {
      case Element::NUMBER_AMF0:
	  os << to_number() << std::endl;
	  break;
      case Element::BOOLEAN_AMF0:
	  os << (to_bool() ? "true" : "false") << std::endl;
	  break;
      case Element::STRING_AMF0:
	  os << "(" << getDataSize() << " bytes): ";
	  if (getDataSize()) {
	      os << "\t\"" << to_string() << "\"";
	  }
	  std::cerr << std::endl;
	  break;
      case Element::OBJECT_AMF0:
	  break;
      case Element::MOVIECLIP_AMF0:
      case Element::NULL_AMF0: 
      case Element::UNDEFINED_AMF0:
      case Element::REFERENCE_AMF0:
      case Element::ECMA_ARRAY_AMF0:
      case Element::OBJECT_END_AMF0:
      case Element::STRICT_ARRAY_AMF0:
      case Element::DATE_AMF0:
      case Element::LONG_STRING_AMF0:
      case Element::UNSUPPORTED_AMF0:
      case Element::RECORD_SET_AMF0:
      case Element::XML_OBJECT_AMF0:
      case Element::TYPED_OBJECT_AMF0:
	  std::cerr << std::endl;
	  break;
      case Element::AMF3_DATA:
	  if (getDataSize() != 0) {
	      gnash::log_debug(_("FIXME: got AMF3 data!"));
	  }
//	  cerr << "AMF3 data is: 0x" << hexify(_data, _length, false) << endl;
	  break;
//       case Element::VARIABLE:
//       case Element::FUNCTION:
//  	  os << "# of properties in object: " << properties.size() << endl;
// 	  for (size_t i=0; i< properties.size(); i++) {
// 	      properties[i]->dump();
// 	  }
// 	  break;
      default:
//	  log_unimpl("%s: type %d", __PRETTY_FUNCTION__, (int)_type);
	  break;
    }

    if (_type != Element::BOOLEAN_AMF0) {
	if (_buffer) {
	    _buffer->dump();
	}
    }

    if (_properties.size() > 0) {
	std::vector<std::shared_ptr<Element> >::const_iterator ait;
	os << "# of Properties in object: " << _properties.size() << std::endl;
	for (ait = _properties.begin(); ait != _properties.end(); ++ait) {
	    const std::shared_ptr<Element> el = (*(ait));
	    el->dump(os);
	}
    }
}

/// \brief Find the named property for this Object.
///
/// @param name An ASCII string that is the name of the property to
///	search for.
///
/// @return A smart pointer to the Element for this property.
std::shared_ptr<cygnal::Element>
Element::findProperty(const std::string &name)
{
    if (_properties.size() > 0) {
	std::vector<std::shared_ptr<Element> >::iterator ait;
//	cerr << "# of Properties in object: " << _properties.size() << endl;
	for (ait = _properties.begin(); ait != _properties.end(); ++ait) {
	    std::shared_ptr<Element> el = (*(ait));
	    if (el->getName() == name) {
		return el;
	    }
//	    el->dump();
	}
    }
    std::shared_ptr<Element> el;
    return el;
}

} // end of amf namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
