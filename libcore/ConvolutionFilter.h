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


#ifndef GNASH_CONVOLUTIONFILTER_H
#define GNASH_CONVOLUTIONFILTER_H

#include "BitmapFilter.h"

#include <vector>
#include <boost/cstdint.hpp> // for XintXX_t

namespace gnash {

// A convolution effect filter.
class ConvolutionFilter : public BitmapFilter
{
public:
    // Fill from a SWFStream. See parser/filter_factory.cpp for
    // the implementations.
    virtual bool read(SWFStream& in);

    virtual ~ConvolutionFilter() {}

    ConvolutionFilter()
        :
        _matrixX(),
        _matrixY(),
        _matrix(),
        _divisor(),
        _bias(),
        _preserveAlpha(false),
        _clamp(false),
        _color(),
        _alpha()
    {}

    ConvolutionFilter(boost::uint8_t matrixX, boost::uint8_t matrixY, 
        const std::vector<float>& _matrix, float divisor, float bias,
        bool preserveAlpha, bool clamp, boost::uint32_t color,
        boost::uint8_t alpha)
        :
        _matrixX(matrixX),
        _matrixY(matrixY),
        _matrix(_matrix),
        _divisor(divisor),
        _bias(bias),
        _preserveAlpha(preserveAlpha),
        _clamp(clamp),
        _color(color),
        _alpha(alpha)
    {}

protected:
    boost::uint8_t _matrixX; // Number of columns
    boost::uint8_t _matrixY; // Number of rows
    std::vector<float> _matrix; // The convolution matrix
    float _divisor;
    float _bias;
    bool _preserveAlpha; // If true, don't convolute the alpha channel
    bool _clamp; // Whether or not to clamp
    boost::uint32_t _color; // For off-image pixels
    boost::uint8_t _alpha; // For off-image pixels
};

} // Namespace gnash

#endif // GNASH_CONVOLUTIONFILTER_H
