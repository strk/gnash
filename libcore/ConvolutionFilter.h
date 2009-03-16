// 
//   Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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
    // Fill from a SWFStream. See parser/filter_factory.cpp for the implementations.
    virtual bool read(SWFStream& in);

    virtual ~ConvolutionFilter() { return; }

    ConvolutionFilter() : 
        m_matrixX(), m_matrixY(), m_matrix(), m_divisor(), m_bias(),
        m_preserveAlpha(false), m_clamp(false), m_color(), m_alpha()
    { return; }

    ConvolutionFilter(boost::uint8_t matrixX, boost::uint8_t matrixY, 
        std::vector<float> a_matrix,
        float divisor, float bias, bool preserveAlpha, bool clamp, boost::uint32_t color,
        boost::uint8_t alpha) :
        m_matrixX(matrixX), m_matrixY(matrixY), m_matrix(a_matrix),
        m_divisor(divisor), m_bias(bias), m_preserveAlpha(preserveAlpha),
        m_clamp(clamp), m_color(color), m_alpha(alpha)
    { return; }

protected:
    boost::uint8_t m_matrixX; // Number of columns
    boost::uint8_t m_matrixY; // Number of rows
    std::vector<float> m_matrix; // The convolution matrix
    float m_divisor;
    float m_bias;
    bool m_preserveAlpha; // If true, don't convolute the alpha channel
    bool m_clamp; // Whether or not to clamp
    boost::uint32_t m_color; // For off-image pixels
    boost::uint8_t m_alpha; // For off-image pixels
};

} // Namespace gnash

#endif // GNASH_CONVOLUTIONFILTER_H
