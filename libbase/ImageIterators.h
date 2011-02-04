// ImageIterators.h: Specialized iterators for image data.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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


#ifndef GNASH_IMAGE_ITERATORS_H
#define GNASH_IMAGE_ITERATORS_H

#include <boost/iterator/iterator_facade.hpp>
#include <iterator>
#include <algorithm>

#include "GnashImage.h"

namespace gnash {
namespace image {

/// Adapt a pixel_iterator to use 32-bit values in ARGB byte order.
class ARGB
{
public:

    typedef GnashImage::iterator iterator;

    /// Construct an ARGB pixel helper
    ARGB(iterator& i, ImageType t)
        :
        _it(i),
        _t(t)
    {}

    /// Standard assignment just copies bytes.
    //
    /// Underlying bytes are really in RGBA order, so we use that.
    const ARGB& operator=(const ARGB& other) const {
        switch (_t) {
            case TYPE_RGBA:
                // RGBA to RGBA
                if (other._t == TYPE_RGBA) {
                    std::copy(other._it, other._it + 4, _it);
                    break;
                }

                // RGB to RGBA
                std::copy(other._it, other._it + 3, _it);
                *(_it + 3) = 0xff;
                break;

            case TYPE_RGB:
                // It doesn't matter what the other image is.
                std::copy(other._it, other._it + 3, _it);

            default:
                break;
        }
        return *this;
    }
    
    /// Writes a 32-bit unsigned value in ARGB byte order to the image
    //
    /// Take note of the different byte order!
    const ARGB& operator=(boost::uint32_t pixel) const {
        switch (_t) {
            case TYPE_RGBA:
                // alpha
                *(_it + 3) = (pixel & 0xff000000) >> 24;
            case TYPE_RGB:
                *_it = (pixel & 0x00ff0000) >> 16;
                *(_it + 1) = (pixel & 0x0000ff00) >> 8;
                *(_it + 2) = (pixel & 0x000000ff);
            default:
                break;
        }
        return *this;
    }
    
    /// Convert to uint32_t in ARGB order
    operator boost::uint32_t() const {
        boost::uint32_t ret = 0xff000000;
        switch (_t) {
            case TYPE_RGBA:
                // alpha
                ret = *(_it + 3) << 24;
            case TYPE_RGB:
                ret |= (*_it << 16 | *(_it + 1) << 8 | *(_it + 2));
            default:
                break;
        }
        return ret;
    }

private:
    iterator& _it;
    const ImageType _t;
};


/// The pixel_iterator class is a pixel-level adaptor for a GnashImage
//
/// Instead of iterating byte-by-byte, this iterator provides access at a
/// whole-pixel level. This makes it possible to assign custom colour values.
//
/// @tparam Pixel       A class that determines the byte order of the colour
///                     value.
template<typename Pixel>
struct pixel_iterator : public boost::iterator_facade<
                            pixel_iterator<Pixel>,
                            const Pixel,
                            std::random_access_iterator_tag>
{

    typedef std::ptrdiff_t difference_type;
    typedef typename Pixel::iterator iterator;

    /// Construct a pixel_iterator
    pixel_iterator(iterator it, ImageType t)
        :
        _it(it),
        _t(t),
        _p(_it, _t)
    {}
    
    /// Copy a pixel_iterator
    pixel_iterator(const pixel_iterator& other)
        :
        _it(other._it),
        _t(other._t),
        _p(_it, _t)
    {}
    
    /// Assign to a pixel_iterator
    pixel_iterator& operator=(const pixel_iterator& other)
    {
        _it = other._it;
        _t = other._t;
        _p = Pixel(_it, _t);
        return *this;
    }
 
private:

    friend class boost::iterator_core_access;

    const Pixel& dereference() const {
        return _p;
    }

    void increment() {
        _it += numChannels(_t);
    }
    
    void decrement() {
        _it -= numChannels(_t);
    }

    bool equal(const pixel_iterator& o) const {
        return o._it == _it;
    }

    difference_type distance_to(const pixel_iterator& o) const {
        return (o._it - _it) / static_cast<int>(numChannels(_t));
    }

    void advance(difference_type n) {
        _it += n * numChannels(_t);
    }

    iterator _it;
    ImageType _t;
    Pixel _p;
};

template<typename T>
pixel_iterator<T>
begin(GnashImage& im)
{
    return pixel_iterator<T>(im.begin(), im.type());
}

template<typename T>
pixel_iterator<T>
end(GnashImage& im)
{
    return pixel_iterator<T>(im.end(), im.type());
}

} // namespace image
} // namespace gnash

#endif
