// GnashImagePng.h: libpng wrapper for Gnash.
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
//

#ifndef GNASH_IMAGE_PNG_H
#define GNASH_IMAGE_PNG_H

#include <memory>
#include <boost/shared_ptr.hpp>

// Forward declarations
namespace gnash {
    class IOChannel;
    namespace image {
        class Input;
        class Output;
    }
}

namespace gnash {
namespace image {

/// Create a PngInput and transfer ownership to the caller.
//
/// @param in   The IOChannel to read PNG data from.
std::unique_ptr<Input> createPngInput(std::shared_ptr<IOChannel> in);

std::unique_ptr<Output> createPngOutput(std::shared_ptr<IOChannel> out,
        size_t width, size_t height, int quality);

} // namespace image
} // namespace gnash



#endif
