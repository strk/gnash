// 
//   Copyright (C) 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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


#ifndef GNASH_BLURFILTER_H
#define GNASH_BLURFILTER_H

#include "BitmapFilter.h"

#include <boost/cstdint.hpp> // for boost::uint8_t

namespace gnash {

// A blur effect filter.
class BlurFilter : public BitmapFilter
{
public:
    // Fill from a SWFStream. See parser/filter_factory.cpp for the implementations.
    virtual bool read(SWFStream& in);

    virtual ~BlurFilter() { return; }

    BlurFilter() : 
        m_blurX(0.0f), m_blurY(0.0f), m_quality(0)
    { return; }

    BlurFilter(float blurX, float blurY, boost::uint8_t quality) :
        m_blurX(blurX), m_blurY(blurY), m_quality(quality)
    { return; }

    float m_blurX; // How much horizontal blur.
    float m_blurY; // How much vertical blur.
    boost::uint8_t m_quality; // How many passes to take.
};

} // Namespace gnash

#endif // GNASH_BLURFILTER_H
