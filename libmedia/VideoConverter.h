//
//   Copyright (C) 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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


#ifndef GNASH_VIDEOCONVERTER_H
#define GNASH_VIDEOCONVERTER_H

#include <boost/noncopyable.hpp>
#include <cstdint>
#include <boost/array.hpp>
#include <memory>

namespace gnash {
namespace media {



/// Image buffer wrapper
//
/// Unfortunately, the GnashImage buffer class currently insists on owning
/// its buffer. Hacking around this results in things like gnashGstBuffer,
/// which is less than desirable. Furthermore, it only supports a handful of
/// pixel and image formats. Something more elaborate is needed to support the
/// various YUV formats and different bit depths for RGB. But in the mean time:
/// here's a simple image class for use in VideoConverter, at least until we
/// merge the image classes.

struct ImgBuf : public boost::noncopyable
{
    typedef std::uint32_t Type4CC;
    typedef void (*FreeFunc)(void*);

    ImgBuf(Type4CC t, std::uint8_t* dataptr, size_t datasize, size_t w,
           size_t h)
    : type(t),
      data(dataptr),
      size(datasize),
      width(w),
      height(h),
      dealloc(array_delete)
    {}
    
    ~ImgBuf()
    {
        dealloc(data);
    }
    
    static void array_delete(void* voidptr)
    {
        std::uint8_t* ptr = static_cast<std::uint8_t*>(voidptr);
        delete [] ptr;
    }
    
    static void noop(void* /*voidptr*/)
    {
    }

    Type4CC type;
    std::uint8_t* data;

    size_t size; // in bytes
    size_t width; // in pixels
    size_t height; // in pixels
    
    boost::array<size_t, 4> stride;
    
    FreeFunc dealloc;
};


/// Abstract base class for video image space conversion.

class VideoConverter : public boost::noncopyable {

public:
    VideoConverter(ImgBuf::Type4CC srcFormat, ImgBuf::Type4CC dstFormat)
     : _src_fmt(srcFormat),
       _dst_fmt(dstFormat) 
    {
    }
    
    virtual ~VideoConverter()
    {
    }
  
    /// Convert a (video) image from one colorspace to another.
    //
    /// @param src the image to convert
    /// @return the converted image or a NULL unique_ptr if an error occurred.
    virtual std::unique_ptr<ImgBuf> convert(const ImgBuf& src) = 0;
  
protected:
    ImgBuf::Type4CC  _src_fmt;
    ImgBuf::Type4CC  _dst_fmt;
};

	
} // gnash.media namespace 
} // gnash namespace

#endif // __VIDEOCONVERTER_H__
