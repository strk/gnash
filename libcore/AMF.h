

#ifndef GNASH_AMF_H
#define GNASH_AMF_H

namespace gnash {

namespace amf {

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

}
}


#endif
