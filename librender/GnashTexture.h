// GnashTexture.h: GnashImage class used for OpenGL rendering
// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
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
//

#ifndef GNASH_GNASHTEXTURE_H
#define GNASH_GNASHTEXTURE_H

#include "GnashImage.h"
#include <boost/shared_ptr.hpp>

namespace gnash {

/// Texture flags
enum {
    GNASH_TEXTURE_VAAPI = 1 << 0
};

/// OpenGL texture format
class DSOEXPORT GnashTextureFormat {
    unsigned int        _internal_format;
    unsigned int        _format;

public:
    GnashTextureFormat(image::ImageType type);

    /// Return GL internal format
    unsigned int internal_format() const
        { return _internal_format; }

    /// Return GL format
    unsigned int format() const
        { return _format; }
};

/// OpenGL texture abstraction
class DSOEXPORT GnashTexture {
    unsigned int        _width;
    unsigned int        _height;
    unsigned int        _texture;
    GnashTextureFormat  _format;

    /// OpenGL texture state
    struct TextureState {
        unsigned int    old_texture;
        unsigned int    was_enabled : 1;
        unsigned int    was_bound   : 1;
    }                   _texture_state;

protected:
    unsigned int        _flags;

private:
    bool init();

public:
    GnashTexture(unsigned int width, unsigned int height,
            image::ImageType type);
    virtual ~GnashTexture();

    /// Return texture flags
    unsigned int flags() const
        { return _flags; }

    /// Return texture width
    unsigned int width() const
        { return _width; }

    /// Return texture height
    unsigned int height() const
        { return _height; }

    /// Return GL texture
    unsigned int texture() const
        { return _texture; }

    /// Return GL internal format
    unsigned int internal_format() const
        { return _format.internal_format(); }

    /// Return GL format
    unsigned int format() const
        { return _format.format(); }

    /// Bind texture to a texturing target
    bool bind();

    /// Release texture
    void release();

    /// Copy texture data from a buffer.
    //
    /// Note that this buffer MUST have the same _pitch, or unexpected things
    /// will happen. In general, it is only safe to copy from another GnashImage
    /// (or derivative thereof) or unexpected things will happen. 
    ///
    /// @param data buffer to copy data from.
    void update(const boost::uint8_t *data);
};

} // gnash namespace

#endif /* GNASH_GNASHTEXTURE_H */
