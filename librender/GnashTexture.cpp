// GnashTexture.cpp: GnashImage class used for OpenGL rendering
// 
// Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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

#include "GnashTexture.h"
#include <GL/gl.h>

#define GL_DEBUG 0

#if GL_DEBUG
#  define D(x) x
#else
#  define D(x)
#endif
#define bug printf

namespace gnash {

// Returns a string representation of an OpenGL error
static const char *gl_get_error_string(GLenum error)
{
    static const struct {
        GLenum val;
        const char *str;
    }
    gl_errors[] = {
        { GL_NO_ERROR,          "no error" },
        { GL_INVALID_ENUM,      "invalid enumerant" },
        { GL_INVALID_VALUE,     "invalid value" },
        { GL_INVALID_OPERATION, "invalid operation" },
        { GL_STACK_OVERFLOW,    "stack overflow" },
        { GL_STACK_UNDERFLOW,   "stack underflow" },
        { GL_OUT_OF_MEMORY,     "out of memory" },
#ifdef GL_INVALID_FRAMEBUFFER_OPERATION_EXT
        { GL_INVALID_FRAMEBUFFER_OPERATION_EXT, "invalid framebuffer operation" },
#endif
        { ~0, NULL }
    };

    int i;
    for (i = 0; gl_errors[i].str; i++) {
        if (gl_errors[i].val == error)
            return gl_errors[i].str;
    }
    return "unknown";
}

static inline bool gl_do_check_error(int report)
{
    GLenum error;
    bool is_error = false;
    while ((error = glGetError()) != GL_NO_ERROR) {
        if (report)
            log_error(_("glError: %s caught\n"), gl_get_error_string(error));
        is_error = true;
    }
    return is_error;
}

static inline void gl_purge_errors(void)
{
    gl_do_check_error(0);
}

static inline bool gl_check_error(void)
{
    return gl_do_check_error(1);
}

// glGetIntegerv() wrapper
static bool gl_get_param(GLenum param, unsigned int *pval)
{
    GLint val;

    gl_purge_errors();
    glGetIntegerv(param, &val);
    if (gl_check_error())
        return false;
    if (pval)
        *pval = val;
    return true;
}

// Check for GLX extensions (TFP, FBO)
static bool check_extension(const char *name, const char *ext)
{
    const char *end;
    int name_len, n;

    if (name == NULL || ext == NULL)
        return false;

    end = ext + strlen(ext);
    name_len = strlen(name);
    while (ext < end) {
        n = strcspn(ext, " ");
        if (n == name_len && strncmp(name, ext, n) == 0)
            return true;
        ext += (n + 1);
    }
    return false;
}

GnashTextureFormat::GnashTextureFormat(image::ImageType type)
{
    switch (type) {
        case image::TYPE_RGB:
            _internal_format = GL_RGB;
            _format = GL_RGB;
            break;
        case image::TYPE_RGBA:
            _internal_format = GL_RGBA;
            _format = GL_BGRA;
            break;
        default:
            assert(0);
            break;
    }
}

GnashTexture::GnashTexture(unsigned int width, unsigned int height,
        image::ImageType type)
    : 
    _width(width),
    _height(height),
    _texture(0),
    _format(type),
    _flags(0)
{
    D(bug("GnashTexture::GnashTexture()\n"));

    init();
}

GnashTexture::~GnashTexture()
{
    D(bug("GnashTexture::~GnashTexture()\n"));

    if (_texture) {
        glDeleteTextures(1, &_texture);
        _texture = 0;
    }
}

bool GnashTexture::init()
{
    // XXX: we only support NPOT textures
    const char *gl_extensions = (const char *)glGetString(GL_EXTENSIONS);
    if (!check_extension("GL_ARB_texture_non_power_of_two", gl_extensions))
        return false;

    assert(_width > 0);
    assert(_height > 0);
    if (_width == 0 || _height == 0)
        return false;

    glGenTextures(1, &_texture);
    if (!_texture)
        return false;

    if (!bind()) {
        glDeleteTextures(1, &_texture);
        return false;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, internal_format() == GL_RGBA ? 4 : 1);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format(), _width, _height, 0,
                 format(), GL_UNSIGNED_BYTE, NULL);
    release();
    return true;
}

// Bind texture, preserve previous texture state
bool GnashTexture::bind()
{
    TextureState * const ts = &_texture_state;
    ts->old_texture = 0;
    ts->was_bound   = 0;
    ts->was_enabled = glIsEnabled(GL_TEXTURE_2D);

    if (!ts->was_enabled)
        glEnable(GL_TEXTURE_2D);
    else if (gl_get_param(GL_TEXTURE_BINDING_2D, &ts->old_texture))
        ts->was_bound = _texture == ts->old_texture;
    else
        return false;

    if (!ts->was_bound) {
        gl_purge_errors();
        glBindTexture(GL_TEXTURE_2D, _texture);
        if (gl_check_error())
            return false;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return true;
}

// Release texture, restore previous texture state
void GnashTexture::release()
{
    TextureState * const ts = &_texture_state;
    if (!ts->was_bound && ts->old_texture)
        glBindTexture(GL_TEXTURE_2D, ts->old_texture);
    if (!ts->was_enabled)
        glDisable(GL_TEXTURE_2D);
    gl_check_error();
}

// Update texture with data
void GnashTexture::update(const boost::uint8_t *data)
{
    D(bug("GnashTexture::update(): data %p, size %dx%d\n", data, _width, _height));

    bind();
    glTexSubImage2D(GL_TEXTURE_2D, 0,
                    0, 0, _width, _height,
                    format(), GL_UNSIGNED_BYTE, data);
    release();
}

} // gnash namespace
