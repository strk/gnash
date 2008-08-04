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

#ifndef GNASH_FLV_H
#define GNASH_FLV_H

#include <vector>
#include <string>
#include <cstring>
#include <boost/cstdint.hpp>    // for boost::?int??_t

//#include "buffer.h"
#include "element.h"
#include "dsodefs.h" // DSOEXPORT

namespace amf
{

class DSOEXPORT Flv {
  public:
    typedef enum {
        FLV_VIDEO = 0x1,
        FLV_AUDIO = 0x4
    } flv_type_e;
    typedef enum {
        FLV_TAG_VIDEO = 0x8,
        FLV_TAG_AUDIO = 0x9,
        FLV_TAG_METADATA = 0x12
    } flv_tag_type_e;
    typedef struct {
        boost::uint8_t  sig[3];      // always "FLV"
        boost::uint8_t  version;     // version, always seems to be 1
        boost::uint8_t  type;        // Bitmask: 0x4 for audio, 0x1 for video
        boost::uint32_t head_size;   // size of header, always seems to be 9
    } flv_header_t;
    typedef struct {
        boost::uint8_t  type;         // the type. audio, video, or meta
        boost::uint8_t  bodysize[3];  // body size (tag size - sizeof(flv_tag_t))
        boost::uint8_t  timestamp[3]; // timestamp in milliseconds
        boost::uint8_t  extended;     // extended timestamp
        boost::uint8_t  streamid[3];  // always 0
    } flv_tag_t;
    Flv();
    ~Flv();
    
    amf::Element *findProperty(const std::string &name);
    void setProperties(std::vector<amf::Element *> x) { _properties = x; };
    
    void dump();
  private:
    flv_header_t                _header;
//    boost::uint32_t             _previous_tag_size;
    flv_tag_t                   _tag;
    std::vector<amf::Element *> _properties;
}; // end of class definition


} // end of amf namespace

// end of _FLV_H_
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
