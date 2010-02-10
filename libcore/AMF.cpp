// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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


#include <map>

#include "SimpleBuffer.h"
#include "AMF.h"
#include "namedStrings.h"
#include "as_value.h"
#include "as_object.h"
#include "ObjectURI.h"
#include "VM.h"
#include "Date_as.h"
#include "Array_as.h"

namespace gnash {

namespace AMF {

namespace {

/// Class used to serialize properties of an object to a buffer
class PropsBufSerializer : public AbstractPropertyVisitor
{

public:
    PropsBufSerializer(Writer& w, VM& vm)
        :
        _writer(w),
        _st(vm.getStringTable()),
        _error(false)
    {}
    
    bool success() const { return !_error; }

    bool accept(const ObjectURI& uri, const as_value& val) {
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
            log_error("Problems serializing an object's member");
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
    boost::uint16_t namelen = name.size();
    _buf.appendNetworkShort(namelen);
    _buf.append(name.c_str(), namelen);
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
        log_debug(_("amf: serializing object (or function) "
                    "as reference to %d"), idx);
#endif
        _buf.appendByte(REFERENCE_AMF0);
        _buf.appendNetworkShort(idx);
        return true;
    }
     
    // 1 for the first, etc...
    const size_t idx = _offsets.size() + 1;
    _offsets[obj] = idx;

    // Dates are handled specially. 
    Date_as* date;
    if (isNativeType(obj, date))
    {
        double d = date->getTimeValue(); 
#ifdef GNASH_DEBUG_AMF_SERIALIZE
        log_debug(_("amf: serializing date object "
                    "with index %d and value %g"), idx, d);
#endif
        _buf.appendByte(DATE_AMF0);

        // This actually only swaps on little-endian machines
        swapBytes(&d, 8);
        _buf.append(&d, 8);

        // This should be timezone
        boost::uint16_t tz = 0; 
        _buf.appendNetworkShort(tz);

        return true;
    }

    VM& vm = getVM(*obj);

    // Arrays are handled specially.
    if (obj->array()) {

        string_table& st = vm.getStringTable();
        const size_t len = arrayLength(*obj);
        if (_strictArray) {
            IsStrictArray s(st);
            // Check if any non-hidden properties are non-numeric.
            obj->visitProperties<IsEnumerable>(s);

            if (s.strict()) {

#ifdef GNASH_DEBUG_AMF_SERIALIZE
                log_debug(_("amf: serializing array of %d "
                            "elements as STRICT_ARRAY (index %d)"),
                            len, idx);
#endif
                _buf.appendByte(STRICT_ARRAY_AMF0);
                _buf.appendNetworkLong(len);

                as_value elem;
                for (size_t i = 0; i < len; ++i) {
                    elem = obj->getMember(arrayKey(st, i));
                    if (!elem.writeAMF0(*this)) {
                        log_error("Problems serializing strict array "
                                "member %d=%s", i, elem);
                        return false;
                    }
                }
                return true;
            }
        }

        // A normal array.
#ifdef GNASH_DEBUG_AMF_SERIALIZE
        log_debug(_("amf: serializing array of %d "
                    "elements as ECMA_ARRAY (index %d) "
                    "[allowStrict:%d, isStrict:%d]"),
                    len, idx, allowStrict, isStrict);
#endif
        _buf.appendByte(ECMA_ARRAY_AMF0);
        _buf.appendNetworkLong(len);
    }
    else {
        // It's a simple object
#ifdef GNASH_DEBUG_AMF_SERIALIZE
        log_debug(_("amf: serializing object (or function) "
                    "with index %d"), idx);
#endif
        _buf.appendByte(OBJECT_AMF0);
    }

    PropsBufSerializer props(*this, vm);
    obj->visitProperties<IsEnumerable>(props);
    if (!props.success()) {
        log_error("Could not serialize object");
        return false;
    }
    _buf.appendNetworkShort(0);
    _buf.appendByte(OBJECT_END_AMF0);
    return true;
}

bool
Writer::writeString(const std::string& str)
{
    const size_t strlen = str.size();
    if (strlen <= 65535) {
#ifdef GNASH_DEBUG_AMF_SERIALIZE
        log_debug(_("amf: serializing string '%s'"), str);
#endif
        _buf.appendByte(STRING_AMF0);
        _buf.appendNetworkShort(strlen);
        _buf.append(str.c_str(), strlen);
        return true;
    }

#ifdef GNASH_DEBUG_AMF_SERIALIZE
    log_debug(_("amf: serializing long string '%s'"), str);
#endif
    _buf.appendByte(LONG_STRING_AMF0);
    _buf.appendNetworkLong(strlen);
    _buf.append(str.c_str(), strlen);
    return true;
}

bool
Writer::writeNumber(double d)
{
#ifdef GNASH_DEBUG_AMF_SERIALIZE
    log_debug(_("amf: serializing number '%g'"), d);
#endif
    _buf.appendByte(NUMBER_AMF0);
    swapBytes(&d, 8);
    _buf.append(&d, 8);
    return true;
}

bool
Writer::writeUndefined()
{
#ifdef GNASH_DEBUG_AMF_SERIALIZE
    log_debug(_("amf: serializing undefined"));
#endif
    _buf.appendByte(UNDEFINED_AMF0);
    return true;
}

bool
Writer::writeNull()
{
#ifdef GNASH_DEBUG_AMF_SERIALIZE
    log_debug(_("amf: serializing null"));
#endif
    _buf.appendByte(NULL_AMF0);
    return true;
}

void
Writer::writeData(boost::uint8_t* data, size_t length)
{
    _buf.append(data, length);
}

bool
Writer::writeBoolean(bool b)
{
#ifdef GNASH_DEBUG_AMF_SERIALIZE
    log_debug(_("amf: serializing boolean '%s'"), b);
#endif
    _buf.appendByte(BOOLEAN_AMF0);
    
    if (b) _buf.appendByte(1);
    else _buf.appendByte(0);

    return true;
}

void*
swapBytes(void *word, size_t size)
{
    union {
    boost::uint16_t s;
    struct {
        boost::uint8_t c0;
        boost::uint8_t c1;
    } c;
    } u;
       
    u.s = 1;
        if (u.c.c0 == 0) {
        // Big-endian machine: do nothing
        return word;
    }

    // Little-endian machine: byte-swap the word
    // A conveniently-typed pointer to the source data
    boost::uint8_t *x = static_cast<boost::uint8_t *>(word);

    /// Handle odd as well as even counts of bytes
    std::reverse(x, x + size);
    
    return word;
}

} // namespace AMF
} // namespace gnash
