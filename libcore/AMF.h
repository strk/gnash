

#ifndef GNASH_AMF_H
#define GNASH_AMF_H

#include <map>
#include <string>

namespace gnash {
    class as_object;
    class SimpleBuffer;
}

namespace gnash {

namespace AMF {

    enum Type {
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
    };

    class Writer
    {
    public:

        typedef std::map<as_object*, size_t> OffsetTable;

        Writer(SimpleBuffer& buf, bool strictArray = false)
            :
            _buf(buf),
            _strictArray(strictArray)
        {}

        /// Write any simple Object type: not DisplayObjects.
        //
        /// Handles functions, dates, and arrays. The object may not be null.
        bool writeObject(as_object* obj);

        /// Write a string.
        //
        /// Handles long and short strings.
        bool writeString(const std::string& str);

        /// Write a null value.
        bool writeNull();

        /// Write an undefined value.
        bool writeUndefined();

        /// Write a double.
        bool writeNumber(double d);

        bool writeBoolean(bool b);

        bool writePropertyName(const std::string& name);

        void writeData(boost::uint8_t* data, size_t length);

    private:
        OffsetTable _offsets;
        SimpleBuffer& _buf;
        bool _strictArray;
    };

} // namespace amf
} // namespace gnash

#endif
