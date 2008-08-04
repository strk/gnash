// amf.cpp:  AMF (Action Message Format) rpc marshalling, for Gnash.
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
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <string>
#include <vector>
#include <cmath>
#include <climits>

#include "buffer.h"
#include "log.h"
#include "amf.h"
#include "amfutf8.h"
#include "utility.h"
#include "flv.h"

#include <boost/cstdint.hpp> // for boost::?int??_t

using namespace std;
using namespace gnash;

namespace amf 
{


Flv::Flv() 
{
//    GNASH_REPORT_FUNCTION;
    memcpy(&_header.sig, "FLV", 3);
    _header.version = 1;
    _header.type = Flv::FLV_VIDEO | Flv::FLV_AUDIO;
    _header.head_size = 9;
    
    memset(&_tag, 0, sizeof(flv_tag_t));
    _tag.type = Flv::FLV_TAG_METADATA;
    _tag.bodysize[0] = 0x0f;
    _tag.bodysize[1] = 0x30;
    _tag.bodysize[2] = 0x00;
}

Flv::~Flv()
{
//    GNASH_REPORT_FUNCTION;
}

amf::Element *
Flv::findProperty(const std::string &name)
{
    if (_properties.size() > 0) {
	vector<amf::Element *>::iterator ait;
//	cerr << "# of Properties in object: " << _properties.size() << endl;
	for (ait = _properties.begin(); ait != _properties.end(); ait++) {
	    amf::Element *el = (*(ait));
	    if (el->getName() == name) {
		return el;
	    }
//	    el->dump();
	}
    }
    return 0;
}

void
Flv::dump()
{
//    GNASH_REPORT_FUNCTION;
    if (_properties.size() > 0) {
	vector<amf::Element *>::iterator ait;
	cerr << "# of Properties in object: " << _properties.size() << endl;
	for (ait = _properties.begin(); ait != _properties.end(); ait++) {
	    amf::Element *el = (*(ait));
            // an onMetaData packet of an FLV stream only contains number or
            // boolean bydefault
            if (el->getType() == Element::NUMBER_AMF0) {
                log_debug("FLV MetaData: %s: %s", el->getName(), el->to_number());
            } else if (el->getType() == Element::BOOLEAN_AMF0) {
                log_debug("FLV MetaData: %s: %s", el->getName(), (el->to_bool() ? "true" : "false") );
            } else {
                log_debug("FLV MetaData: %s: %s", el->getName(), el->to_string());
            }
	}
    }
}

} // end of amf namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
