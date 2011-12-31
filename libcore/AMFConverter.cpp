// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include "AMFConverter.h"

#include <map>

#include "SimpleBuffer.h"
#include "AMF.h"
#include "namedStrings.h"
#include "as_value.h"
#include "as_object.h"
#include "ObjectURI.h"
#include "VM.h"
#include "Date_as.h"
#include "XML_as.h"
#include "Array_as.h"
#include "Global_as.h"
#include "fn_call.h"
#include "as_function.h"
#include "PropertyList.h"

// Define this macro to make AMF parsing verbose
//#define GNASH_DEBUG_AMF_DESERIALIZE 1

// Define this macto to make AMF writing verbose
// #define GNASH_DEBUG_AMF_SERIALIZE 1

namespace gnash {
namespace amf {

namespace {

/// Class used to serialize properties of an object to a buffer
class ObjectSerializer : public PropertyVisitor
{

public:
    ObjectSerializer(Writer& w, VM& vm)
        :
        _writer(w),
        _st(vm.getStringTable()),
        _error(false)
    {}
    
    bool success() const { return !_error; }

    virtual bool accept(const ObjectURI& uri, const as_value& val) {

        if (_error) return true;

        // Tested with SharedObject and AMFPHP
        if (val.is_function()) {
            log_debug("AMF0: skip serialization of FUNCTION property");
            return true;
        }

        const string_table::key key = getName(uri);

        // Test conducted with AMFPHP:
        // '__proto__' and 'constructor' members
        // of an object don't get back from an 'echo-service'.
        // Dunno if they are not serialized or just not sent back.
        // A '__constructor__' member gets back, but only if 
        // not a function. Actually no function gets back.
        if (key == NSV::PROP_uuPROTOuu || key == NSV::PROP_CONSTRUCTOR)
        {
#ifdef GNASH_DEBUG_AMF_SERIALIZE
            log_debug(" skip serialization of specially-named property %s",
                    _st.value(key));
#endif
            return true;
        }

        // write property name
        const std::string& name = _st.value(key);

#ifdef GNASH_DEBUG_AMF_SERIALIZE
        log_debug(" serializing property %s", name);
#endif
        _writer.writePropertyName(name);
        if (!val.writeAMF0(_writer)) {
            log_error(_("Problems serializing an object's member"));
            _error = true;
        }
        return true;
    }

private:

    Writer& _writer;
    string_table& _st;
    mutable bool _error;

};

}

bool
Writer::writePropertyName(const std::string& name)
{
    writePlainString(_buf, name, STRING_AMF0);
    return true;
}

bool
Writer::writeObject(as_object* obj)
{
    assert(obj);

    // This probably shouldn't happen.
    if (obj->to_function()) return false;

    OffsetTable::iterator it = _offsets.find(obj);

    // Handle references first.
    if (it != _offsets.end()) {
        const size_t idx = it->second;
#ifdef GNASH_DEBUG_AMF_SERIALIZE
        log_debug("amf: serializing object (or function) "
                    "as reference to %d", idx);
#endif
        _buf.appendByte(REFERENCE_AMF0);
        _buf.appendNetworkShort(idx);
        return true;
    }
     
    // 1 for the first, etc...
    const size_t idx = _offsets.size() + 1;
    _offsets[obj] = idx;

    /// Native objects are handled specially.
    if (obj->relay()) {
        
        Date_as* date;
        if (isNativeType(obj, date)) {

            double d = date->getTimeValue(); 
#ifdef GNASH_DEBUG_AMF_SERIALIZE
            log_debug("amf: serializing date object "
                        "with index %d and value %g", idx, d);
#endif
            _buf.appendByte(DATE_AMF0);
            writePlainNumber(_buf, d);

            // This should be timezone
            boost::uint16_t tz = 0; 
            _buf.appendNetworkShort(tz);

            return true;
        }

        /// XML is written like a long string (but with an XML marker).
        XML_as* xml;
        if (isNativeType(obj, xml)) {
            _buf.appendByte(XML_OBJECT_AMF0);
            std::ostringstream s;
            xml->toString(s, true);

            const std::string& xmlstr = s.str();
            writePlainString(_buf, xmlstr, LONG_STRING_AMF0);

            return true;
        }

        // Any native objects not explicitly handled are unsupported (this
        // is just a guess).
        _buf.appendByte(UNSUPPORTED_AMF0);
        return true;
    }

    VM& vm = getVM(*obj);

    // Arrays are handled specially.
    if (obj->array()) {

        const size_t len = arrayLength(*obj);
        if (_strictArray) {
            IsStrictArray s(vm);
            // Check if any non-hidden properties are non-numeric.
            obj->visitProperties<IsEnumerable>(s);

            if (s.strict()) {

#ifdef GNASH_DEBUG_AMF_SERIALIZE
                log_debug("amf: serializing array of %d "
                            "elements as STRICT_ARRAY (index %d)",
                            len, idx);
#endif
                _buf.appendByte(STRICT_ARRAY_AMF0);
                _buf.appendNetworkLong(len);

                as_value elem;
                for (size_t i = 0; i < len; ++i) {
                    elem = getMember(*obj,arrayKey(vm, i));
                    if (!elem.writeAMF0(*this)) {
                        log_error(_("Problems serializing strict array "
                                    "member %d=%s"), i, elem);
                        return false;
                    }
                }
                return true;
            }
        }

        // A normal array.
#ifdef GNASH_DEBUG_AMF_SERIALIZE
        log_debug("amf: serializing array of %d "
                    "elements as ECMA_ARRAY (index %d) ",
                    len, idx);
#endif
        _buf.appendByte(ECMA_ARRAY_AMF0);
        _buf.appendNetworkLong(len);
    }
    else {
        // It's a simple object
#ifdef GNASH_DEBUG_AMF_SERIALIZE
        log_debug("amf: serializing object (or function) "
                    "with index %d", idx);
#endif
        _buf.appendByte(OBJECT_AMF0);
    }

    ObjectSerializer props(*this, vm);
    obj->visitProperties<IsEnumerable>(props);
    if (!props.success()) {
        log_error(_("Could not serialize object"));
        return false;
    }
    _buf.appendNetworkShort(0);
    _buf.appendByte(OBJECT_END_AMF0);
    return true;
}

bool
Writer::writeString(const std::string& str)
{
    write(_buf, str);
    return true;
}

bool
Writer::writeNumber(double d)
{
    write(_buf, d);
    return true;
}

bool
Writer::writeBoolean(bool b)
{
    write(_buf, b);
    return true;
}


bool
Writer::writeUndefined()
{
#ifdef GNASH_DEBUG_AMF_SERIALIZE
    log_debug("amf: serializing undefined");
#endif
    _buf.appendByte(UNDEFINED_AMF0);
    return true;
}

bool
Writer::writeNull()
{
#ifdef GNASH_DEBUG_AMF_SERIALIZE
    log_debug("amf: serializing null");
#endif
    _buf.appendByte(NULL_AMF0);
    return true;
}

void
Writer::writeData(const boost::uint8_t* data, size_t length)
{
    _buf.append(data, length);
}

bool
Reader::operator()(as_value& val, Type t)
{

    // No more reads possible.
    if (_pos == _end) {
        return false;
    }

    // This may leave the read position at the _end of the buffer, but
    // some types are complete with the type byte (null, undefined).
    if (t == NOTYPE) {
        t = static_cast<Type>(*_pos);
        ++_pos;
    }
    
    try {

        switch (t) {
            
            default:
                log_error(_("Unknown AMF type %s! Cannot proceed"), t);
                // A fatal error, since we don't know how much to parse
                return false;

            // Simple types.
            case BOOLEAN_AMF0:
                val = readBoolean(_pos, _end);
                return true;
            
            case STRING_AMF0:
                val = readString(_pos, _end);
                return true;
            
            case LONG_STRING_AMF0:
                 val = readLongString(_pos, _end);
                 return true;
            
            case NUMBER_AMF0:
                val = readNumber(_pos, _end);
                return true;
            
            case UNSUPPORTED_AMF0:
            case UNDEFINED_AMF0:
                val = as_value();
                return true;
            
            case NULL_AMF0:
                val = static_cast<as_object*>(0);
                return true;
            
            // Object types need access to Global_as to create objects.
            case REFERENCE_AMF0:
                val = readReference();
                return true;
            
            case OBJECT_AMF0:
                val = readObject();
                return true;
            
            case ECMA_ARRAY_AMF0:
                val = readArray();
                return true;
            
            case STRICT_ARRAY_AMF0:
                val = readStrictArray();
                return true;
            
            case DATE_AMF0:
                val = readDate();
                return true;

            case XML_OBJECT_AMF0:
                val = readXML();
                return true;
        }
    }
    catch (const AMFException& e) {
        log_error(_("AMF parsing error: %s"), e.what());
        return false;
    }

}

/// Construct an XML object.
//
/// Note that the pp seems not to call the constructor or parseXML, but
/// rather to create it magically. It could do this by calling an ASNative
/// function.
as_value
Reader::readXML()
{
    as_value str = readLongString(_pos, _end);
    as_function* ctor = getMember(_global, NSV::CLASS_XML).to_function();
    
    as_value xml;
    if (ctor) {
        fn_call::Args args;
        args += str;
        VM& vm = getVM(_global);
        xml = constructInstance(*ctor, as_environment(vm), args);
    }
    return xml;
}

as_value
Reader::readStrictArray()
{
    if (_end - _pos < 4) {
        throw AMFException(_("Read past _end of buffer for strict array length"));
    }

    const boost::uint32_t li = readNetworkLong(_pos);
    _pos += 4;

#ifdef GNASH_DEBUG_AMF_DESERIALIZE
    log_debug("amf0 starting read of STRICT_ARRAY with %i elements", li);
#endif
    
    as_object* array = _global.createArray();
    _objectRefs.push_back(array);

    as_value arrayElement;
    for (size_t i = 0; i < li; ++i) {

        // Recurse.
        if (!operator()(arrayElement)) {
            throw AMFException(_("Unable to read array elements"));
        }

        callMethod(array, NSV::PROP_PUSH, arrayElement);
    }

    return as_value(array);
}

// TODO: this function is inconsistent about when it interrupts parsing
// if the AMF is truncated. If it doesn't interrupt, the next read will
// fail.
as_value
Reader::readArray()
{

    if (_end - _pos < 4) {
        throw AMFException(_("Read past _end of buffer for array length"));
    }

    const boost::uint32_t li = readNetworkLong(_pos);
    _pos += 4;
    
    as_object* array = _global.createArray();
    _objectRefs.push_back(array);

    // the count specifies array size, so to have that even if none
    // of the members are indexed
    // if short, will be incremented everytime an indexed member is
    // found
    array->set_member(NSV::PROP_LENGTH, li);

#ifdef GNASH_DEBUG_AMF_DESERIALIZE
    log_debug("amf0 starting read of ECMA_ARRAY with %i elements", li);
#endif

    as_value objectElement;
    VM& vm = getVM(_global);
    for (;;) {

        // It seems we don't mind about this situation, although it means
        // the next read will fail.
        if (_end - _pos < 2) {
            log_error(_("MALFORMED AMF: premature _end of ECMA_ARRAY "
                        "block"));
            break;
        }
        const boost::uint16_t strlen = readNetworkShort(_pos);
        _pos += 2; 

        // _end of ECMA_ARRAY is signalled by an empty string
        // followed by an OBJECT_END_AMF0 (0x09) byte
        if (!strlen) {
            // expect an object terminator here
            if (*_pos != OBJECT_END_AMF0) {
                log_error(_("MALFORMED AMF: empty member name not "
                            "followed by OBJECT_END_AMF0 byte"));
            }
            ++_pos;
            break;
        }
        
        // Throw exception instead?
        if (_end - _pos < strlen) {
            log_error(_("MALFORMED AMF: premature _end of ECMA_ARRAY "
                      "block"));
            break;
        }

        const std::string name(reinterpret_cast<const char*>(_pos), strlen);

#ifdef GNASH_DEBUG_AMF_DESERIALIZE
        log_debug("amf0 ECMA_ARRAY prop name is %s", name);
#endif

        _pos += strlen;

        // Recurse to read element.
        if (!operator()(objectElement)) {
            throw AMFException(_("Unable to read array element"));
        }
        array->set_member(getURI(vm, name), objectElement);
    }
    return as_value(array);
}

as_value
Reader::readObject()
{
    VM& vm = getVM(_global);
    as_object* obj = createObject(_global); 

#ifdef GNASH_DEBUG_AMF_DESERIALIZE
    log_debug("amf0 starting read of OBJECT");
#endif

    _objectRefs.push_back(obj);

    as_value tmp;
    std::string keyString;
    for (;;) {

        if (!operator()(tmp, STRING_AMF0)) {
            throw AMFException(_("Could not read object property name"));
        }
        keyString = tmp.to_string();

        if (keyString.empty()) {
            if (_pos < _end) {
                // AMF0 has a redundant "object _end" byte
                ++_pos; 
            }
            else {
                // What is the point?
                log_error(_("AMF buffer terminated just before "
                            "object _end byte. continuing anyway."));
            }
            return as_value(obj);
        }

        if (!operator()(tmp)) {
            throw AMFException("Unable to read object member");
        }
        obj->set_member(getURI(vm, keyString), tmp);
    }
}

as_value
Reader::readReference()
{

    if (_end - _pos < 2) {
        throw AMFException("Read past _end of buffer for reference index");
    }
    const boost::uint16_t si = readNetworkShort(_pos);
    _pos += 2;

#ifdef GNASH_DEBUG_AMF_DESERIALIZE
    log_debug("readAMF0: reference #%d", si);
#endif
    if (si < 1 || si > _objectRefs.size()) {
        log_error(_("readAMF0: invalid reference to object %d (%d known "
                  "objects)"), si, _objectRefs.size());
        throw AMFException("Reference to invalid object reference");
    }
    return as_value(_objectRefs[si - 1]);
}

as_value
Reader::readDate()
{
    const double d = readNumber(_pos, _end);

#ifdef GNASH_DEBUG_AMF_DESERIALIZE
    log_debug("amf0 read date: %e", dub);
#endif

    as_function* ctor = getMember(_global, NSV::CLASS_DATE).to_function();
    VM& vm = getVM(_global);

    as_value date;
    if (ctor) {
        fn_call::Args args;
        args += d;
        date = constructInstance(*ctor, as_environment(vm), args);

        if (_end - _pos < 2) {
            throw AMFException("premature _end of input reading "
                        "timezone from Date type");
        }
        const boost::uint16_t tz = readNetworkShort(_pos);
        if (tz != 0) {
            log_error(_("Date type encoded timezone info %1%, even though "
                "this field should not be used."), tz);
        }
        _pos += 2;
    }
    return date;
}

} // namespace amf
} // namespace gnash
