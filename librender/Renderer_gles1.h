// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

#ifndef GNASH_RENDER_HANDLER_GLES_H
#define GNASH_RENDER_HANDLER_GLES_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

// gles-1.0c for Linux
#ifdef HAVE_GLES_GL_H
# include <GLES/gl.h>
#endif
#ifdef HAVE_GLES_EGL_H
#include <GLES/egl.h>
#endif

#if 0
// Mali Developer Tools for ARM 1.x
#ifdef HAVE_EGL_EGL_H
# include <EGL/egl.h>
# include <EGL/eglext.h>
#endif
// Mali Developer Tools for ARM 2.x and Android 2.1
#ifdef HAVE_GLES2_GL2_H
# include <GLES2/gl2.h>
# include <GLES2/gl2ext.h>
#endif
#endif

#include <vector>

#include "Renderer.h"
#include "Geometry.h"

#include <map>

namespace gnash {

typedef std::vector<const Path*> PathRefs;

struct oglVertex {
    oglVertex(GLfloat x, GLfloat y, GLfloat z = 0.0)
        : _x(x), _y(y), _z(z)
        {
        }
    
    oglVertex(const point& p)
        : _x(p.x), _y(p.y), _z(0.0)
        {
        }
    
    GLfloat _x;
    GLfloat _y;
    GLfloat _z;
};

typedef std::map<const Path*, std::vector<oglVertex> > PathPointMap;

class WholeShape
{
public:
    void newPath(const Path& new_path)
    {
        PathRefs refs;
        refs.push_back(&new_path); 
        shape.push_back(refs);
    }
  
    void addPath(const Path& add_path)
    {
        PathRefs& refs = shape.back();
        refs.push_back(&add_path);
    }
  
    void addPathRefs(const PathRefs& pathrefs)
    {
        PathRefs new_refs(pathrefs.begin(), pathrefs.end());
        shape.push_back(new_refs);
    }
  
    const std::vector<PathRefs>& get() const
    {
        return shape;
    }
private:
  std::vector<PathRefs> shape;
};

    class bitmap_info_ogl //: public BitmapInfo
{
public:
    
    /// Set line and fill styles for mesh & line_strip rendering.
    enum bitmap_wrap_mode
    {
        WRAP_REPEAT,
        WRAP_CLAMP
    };
    
    bitmap_info_ogl(GnashImage* image, GLenum pixelformat,
                    bool ogl_accessible);
    ~bitmap_info_ogl();
    
    void apply(const gnash::SWFMatrix& bitmap_matrix,
               bitmap_wrap_mode wrap_mode) const;
private:
    inline bool ogl_accessible() const;
    void setup() const;    
    void upload(boost::uint8_t* data, size_t width, size_t height) const;
    
    mutable std::auto_ptr<GnashImage> _img;
    GLenum _pixel_format;
    GLenum _ogl_img_type;
    mutable bool _ogl_accessible;  
    mutable GLuint _texture_id;
    size_t _orig_width;
    size_t _orig_height;
};

DSOEXPORT Renderer* create_Renderer_gles(bool init = true);

} // namespace gnash

#endif // __RENDER_HANDLER_GLES_H__

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
