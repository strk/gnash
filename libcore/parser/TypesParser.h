// TypesParser.h: read SWF types from a stream
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

#ifndef GNASH_TYPESPARSER_H
#define GNASH_TYPESPARSER_H

#include <boost/optional.hpp>

#include "SWF.h"

namespace gnash {
    class SWFStream;
    class SWFMatrix;
    class SWFRect;
    class SWFCxForm;
    class rgba;
    class movie_definition;
    class FillStyle;
}

namespace gnash {

/// Read a SWFMatrix from input stream
SWFMatrix readSWFMatrix(SWFStream& in);

/// Read an RGBA rgba from input stream
rgba readRGBA(SWFStream& in);

/// Read an RGB rgba from input stream
rgba readRGB(SWFStream& in);
    
/// Read a bit-packed rectangle from an SWF stream
SWFRect readRect(SWFStream& in);

/// Either a single or a morph-pair FillStyle.
typedef std::pair<FillStyle, boost::optional<FillStyle> > OptionalFillPair;

/// Read FillStyles from a stream
//
/// Read either single or morph-pair fill styles from a stream. 
OptionalFillPair readFills(SWFStream& in, SWF::TagType t, movie_definition& m,
        bool readMorph);

/// Read a RGB CxForm from the input stream
SWFCxForm readCxFormRGB(SWFStream& in);

/// Read a CxForm with alpha from the input stream
SWFCxForm readCxFormRGBA(SWFStream& in);

} // namespace gnash

#endif
