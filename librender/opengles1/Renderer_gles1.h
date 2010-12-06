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

#ifndef GNASH_RENDER_HANDLER_GLES1_H
#define GNASH_RENDER_HANDLER_GLES1_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <vector>

// gles-1.0c for Linux
#ifdef HAVE_GLES1_GL_H
# include <GLES/gl.h>
#endif
#ifdef HAVE_GLES1_EGL_H
#include <GLES/egl.h>
#endif

#include "Renderer.h"
#include "Geometry.h"
#include "egl/eglDevice.h"

#include <map>

namespace gnash {

class GnashImage;
class SWFCxForm;

namespace renderer {

namespace gles1 {

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

class  DSOEXPORT Renderer_gles1: public Renderer, public EGLDevice
{
public:
    std::string description() const { return "OpenGLES1"; }
    Renderer_gles1();
    Renderer_gles1(GnashDevice::dtype_t dtype);
    
    ~Renderer_gles1();
        
    void init(float x, float y);
    CachedBitmap *createCachedBitmap(std::auto_ptr<image::GnashImage> im);

    void world_to_pixel(int& x, int& y, float world_x, float world_y);
    gnash::geometry::Range2d<int> world_to_pixel(const gnash::SWFRect& wb);
    geometry::Range2d<int> world_to_pixel(const geometry::Range2d<float>& wb);
    gnash::point pixel_to_world(int, int);

    void begin_display(const gnash::rgba&, int, int, float,
                                        float, float, float);
    // // This is from the patch
    // void begin_display(const rgba& bg_color, int viewport_x0,
    //                    int viewport_y0, int viewport_width,
    //                    int viewport_height, float x0, float x1,
    //                    float y0, float y1);
    void end_display();
    void drawLine(const std::vector<point>& coords, const rgba& fill,
                  const SWFMatrix& mat);
    void drawVideoFrame(gnash::image::GnashImage *frame, const gnash::Transform& tx,
                        const gnash::SWFRect *bounds, bool smooth);
    void drawPoly(const point* corners, size_t corner_count, 
                  const rgba& fill, const rgba& outline,
                  const SWFMatrix& mat, bool masked);
    void drawShape(const gnash::SWF::ShapeRecord&, const gnash::Transform&);
    void drawGlyph(const SWF::ShapeRecord& rec, const rgba& c,
                   const SWFMatrix& mat);

    void set_antialiased(bool enable);
    void begin_submit_mask();
    void end_submit_mask();
    void apply_mask();
    void disable_mask();
        
    void set_scale(float xscale, float yscale);
    void set_invalidated_regions(const InvalidatedRanges &ranges);

    // These weren't in the patch
    Renderer *startInternalRender(gnash::image::GnashImage&);
    void endInternalRender();

    unsigned int getBitsPerPixel();
    bool initTestBuffer(unsigned width, unsigned height);

    // These methods are only for debugging and development
    void printVGParams();
    void printVGHardware();
    void printVGPath();
  private:
    unsigned char *_testBuffer; // buffer used by initTestBuffer() only
};    

DSOEXPORT Renderer* create_handler(const char *pixelformat);

} // namespace gnash::renderer::gles1
} // namespace gnash::renderer
} // namespace gnash

#endif // __RENDER_HANDLER_GLES1_H__

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
