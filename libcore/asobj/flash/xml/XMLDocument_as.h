// XMLDocument_as.h:  ActionScript 3 "XMLDocument" class, for Gnash.
//
//   Copyright (C) 2009, 2010 Free Software Foundation, Inc.
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

#ifndef GNASH_ASOBJ3_XMLDOCUMENT_H
#define GNASH_ASOBJ3_XMLDOCUMENT_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "LoadableObject.h"
#include "xml/XMLNode_as.h"
#include "log.h"
#include "dsodefs.h"
#include "StringPredicates.h"

#include <map>
#include <string>


namespace gnash {

// Forward declarations
class fn_call;
class URL;


/// Escape using XML entities.
//
/// Note this is not the same as a URL escape.
void escapeXML(std::string& text);

/// Register the XML class.
void xml_class_init(as_object& where, const ObjectURI& uri);

/// Register XML native functions.
void registerXMLNative(as_object& where);

}	// namespace gnash
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

