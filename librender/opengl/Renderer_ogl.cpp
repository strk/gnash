// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011
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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "Renderer_ogl.h"

#include <boost/utility.hpp>
#include <boost/bind.hpp>
#include <list>
#include <cstring>
#include <cmath>
#include <boost/scoped_ptr.hpp>

#include "swf/ShapeRecord.h"
#include "GnashEnums.h"
#include "RGBA.h"
#include "GnashImage.h"
#include "GnashTexture.h"
#include "GnashNumeric.h"
#include "log.h"
#include "utility.h"
#include "Range2d.h"
#include "SWFCxForm.h"
#include "FillStyle.h"
#include "Transform.h"

#if defined(_WIN32) || defined(WIN32)
#  include <Windows.h>
#endif

#ifdef HAVE_VA_VA_GLX_H
#  include "GnashVaapiImage.h"
#  include "GnashVaapiTexture.h"
#endif

// Defined to 1 to disable (slow) anti-aliasing with the accumulation buffer
#define NO_ANTIALIASING 1

/// \file Renderer_ogl.cpp
/// \brief The OpenGL renderer and related code.
///
/// So how does this thing work?
///
/// 1. Flash graphics are fundamentally incompatible with OpenGL. Flash shapes
///    are defined by an arbitrary number of paths, which in turn are formed
///    from an arbitrary number of edges. An edge describes a quadratic Bezier
///    curve. A shape is defined by at least one path enclosing a space -- this
///    space is the shape. Every path may have a left and/or right fill style,
///    determining (if the path is thought of as a vector) which side(s) of
///    the path is to be filled.
///    OpenGL, on the other hand, understands only triangles, lines and points.
///    We must break Flash graphics down into primitives that OpenGL can
///    understand before we can render them.
///
/// 2. First, we must ensure that OpenGL receives only closed shapes with a
///    single fill style. Paths with two fill styles are duplicated. Then,
///    shapes with
///    a left fill style are reversed and the fill style is moved to the right.
///    The shapes must be closed, so the tesselator can parse them; this
///    involves a fun game of connect-the-dots. Fortunately, Flash guarantees
///    that shapes are always closed and that they're never self-intersecting.
///
/// 3. Now that we have a consistent set of shapes, we can interpolate the
///    Bezier curves of which each path is made of. OpenGL can do this for us,
///    using evaluators, but we currently do it ourselves.
///
/// 4. Being the proud owners of a brand new set of interpolated coordinates,
///    we can feed the coordinates into the GLU tesselator. The tesselator will
///    break our complex (but never self-intersecting) polygon into OpenGL-
///    grokkable primitives (say, a triangle fan or strip). We let the
///    tesselator worry about that part. When the tesselator is finished, all
///    we have to do is set up the fill style and draw the primitives given to
///    us. The GLU tesselator will take care of shapes having inner boundaries
///    (for example a donut shape). This makes life a LOT easier!

// TODO:
// - Profiling!
// - Optimize code:
// * Use display lists
// * Use better suited standard containers
// * convert to double at a later stage (oglVertex)
// * keep data for less time
// * implement hardware accelerated gradients. Most likely this will require
//   the use of fragment shader language.

// * The "Programming Tips" in the OpenGL "red book" discusses a coordinate system
// that would give "exact two-dimensional rasterization". AGG uses a similar
// system; consider the benefits and drawbacks of switching.

namespace gnash {

namespace renderer {

namespace opengl {

namespace {
    const CachedBitmap* createGradientBitmap(const GradientFill& gf,
            Renderer& renderer);

class bitmap_info_ogl : public CachedBitmap
{
public:
  
    /// Set line and fill styles for mesh & line_strip rendering.
    enum bitmap_wrap_mode
    {
      WRAP_REPEAT,
      WRAP_CLAMP
    };

    bitmap_info_ogl(std::auto_ptr<image::GnashImage> image, GLenum pixelformat,
                    bool ogl_accessible);

    ~bitmap_info_ogl();
 
    /// TODO: implement this meaningfully.
    virtual void dispose() {
        _disposed = true;
    }

    virtual bool disposed() const {
        return _disposed;
    }

    virtual image::GnashImage& image() {
        if (_cache.get()) return *_cache;
        switch (_pixel_format) {
            case GL_RGB:
                _cache.reset(new image::ImageRGB(_orig_width, _orig_height));
                break;
            case GL_RGBA:
                _cache.reset(new image::ImageRGBA(_orig_width, _orig_height));
                break;
            default:
                std::abort();
        }
        std::fill(_cache->begin(), _cache->end(), 0xff);
        return *_cache;
    }

    void apply(const gnash::SWFMatrix& bitmap_matrix,
               bitmap_wrap_mode wrap_mode) const;
private:
    inline bool ogl_accessible() const;
    void setup() const;    
    void upload(boost::uint8_t* data, size_t width, size_t height) const;
    
    mutable boost::scoped_ptr<image::GnashImage> _img;
    mutable boost::scoped_ptr<image::GnashImage> _cache;
    GLenum _pixel_format;
    GLenum _ogl_img_type;
    mutable bool _ogl_accessible;  
    mutable GLuint _texture_id;
    size_t _orig_width;
    size_t _orig_height;
    bool _disposed;
};

/// Style handler
//
/// Transfer FillStyles to the ogl renderer.
struct StyleHandler : boost::static_visitor<>
{
    StyleHandler(const SWFCxForm& c, Renderer& r)
        :
        _cx(c),
        _renderer(r)
    {}

    void operator()(const GradientFill& f) const {

        const bitmap_info_ogl* binfo = static_cast<const bitmap_info_ogl*>(
            createGradientBitmap(f, _renderer));  

        SWFMatrix m = f.matrix();
        binfo->apply(m, bitmap_info_ogl::WRAP_CLAMP); 
    }

    void operator()(const SolidFill& f) const {
        const rgba c = _cx.transform(f.color());
        glColor4ub(c.m_r, c.m_g, c.m_b, c.m_a);
    }

    void operator()(const BitmapFill& f) const {
        const bitmap_info_ogl* binfo = static_cast<const bitmap_info_ogl*>(
                   f.bitmap());
        binfo->apply(f.matrix(), f.type() == BitmapFill::TILED ?
                bitmap_info_ogl::WRAP_REPEAT : bitmap_info_ogl::WRAP_CLAMP);
    }

private:
    const SWFCxForm& _cx;
    Renderer& _renderer;
};  

}


#ifdef OSMESA_TESTING

class OSRenderMesa : public boost::noncopyable
{
public:
  OSRenderMesa(size_t width, size_t height)
    : _width(width),
      _height(height),
      _buffer(new boost::uint8_t[width * height * 3]), 
#if OSMESA_MAJOR_VERSION * 100 + OSMESA_MINOR_VERSION >= 305
      _context(OSMesaCreateContextExt(OSMESA_RGB, 0, 2, 0, NULL))
#else
      _context(OSMesaCreateContext(OSMESA_RGB, NULL))
#endif
  {
    if (!_context) {
      log_error("OSMesaCreateContext failed!");
      return; // FIXME: throw an exception?
    }

    if (!OSMesaMakeCurrent(_context, _buffer.get(), GL_UNSIGNED_BYTE, width,
                           height)) {
      log_error("OSMesaMakeCurrent failed!");
      return;
    }
   
    // FIXME: is there any reason to do this?
    OSMesaColorClamp(GL_TRUE);

    log_debug("OSMesa handle successfully created. with width %d"
            " and height %d.", width, height);  
  }
  
  ~OSRenderMesa()
  {
    if (!_context) {
      return;
    }
    OSMesaDestroyContext(_context);
  }
  
  bool getPixel(rgba& color_out, int x, int y) const
  {  
    glFinish(); // Force pending commands out (and wait until they're done).    

    if (x > _width || y > _height) {
      return false;
    }
    
    ptrdiff_t offset = (_height - y) * (_width * 3) + x * 3;
    color_out = rgba(_buffer[offset], _buffer[offset+1], _buffer[offset+2],
            255);

    return true;
  }
  
  unsigned int getBitsPerPixel() const
  {
    return 24;
  }

private:
  size_t _width;
  size_t _height;
  boost::scoped_array<boost::uint8_t> _buffer;
  OSMesaContext _context;  
};

#endif // OSMESA_TESTING


typedef std::vector<Path> PathVec;

class oglScopeEnable : public boost::noncopyable
{
public:
  oglScopeEnable(GLenum capability)
    :_cap(capability)
  {
    glEnable(_cap);
  }
  
  ~oglScopeEnable()
  {
    glDisable(_cap);
  }
private:
  GLenum _cap;
};

class oglScopeMatrix : public boost::noncopyable
{
public:
  oglScopeMatrix(const SWFMatrix& m)
  {
    glPushMatrix();

    // Multiply (AKA "append") the new SWFMatrix with the current OpenGL one.
    float mat[16];
    memset(&mat[0], 0, sizeof(mat));
    mat[0] = m.a() / 65536.0f;
    mat[1] = m.b() / 65536.0f;
    mat[4] = m.c() / 65536.0f;
    mat[5] = m.d() / 65536.0f;
    mat[10] = 1;
    mat[12] = m.tx();
    mat[13] = m.ty();
    mat[15] = 1;
    glMultMatrixf(mat);
  }
  
  ~oglScopeMatrix()
  {
    glPopMatrix();
  }
};

static void
check_error()
{
  GLenum error = glGetError();
  
  if (error == GL_NO_ERROR) {
    return;
  }
  
  log_error("OpenGL: %s", gluErrorString(error));
}

/// @ret A point in the middle of points a and b, that is, the middle of a line
///      drawn from a to b.
point middle(const point& a, const point& b)
{
  return point(0.5 * (a.x + b.x), 0.5 * (a.y + b.y));
}

namespace {

class
PointSerializer
{
public:
    PointSerializer(std::vector<boost::int16_t>& dest)
        :
        _dest(dest)
    {}
    
    void operator()(const point& p)
    {
        _dest.push_back(p.x);
        _dest.push_back(p.y);
    }
private:
    std::vector<boost::int16_t>& _dest;
};

}

// Unfortunately, we can't use OpenGL as-is to interpolate the curve for us. It
// is legal for Flash coordinates to be outside of the viewport, which will
// be ignored by OpenGL's feedback mode. Feedback mode
// will simply not return those coordinates, which will destroy a shape which
// is partly off-screen.

// So, if we transform the coordinates to be always positive, it should be
// possible to use evaluators. This then presents another problem: what if
// one coordinate is negative and the other is not, and what if both of
// those are outside of the viewport?

// one solution would be to use feedback mode unless one of the coordinates
// is outside of the viewport.
void trace_curve(const point& startP, const point& controlP,
                  const point& endP, std::vector<oglVertex>& coords)
{
  // Midpoint on line between two endpoints.
  point mid = middle(startP, endP);

  // Midpoint on the curve.
  point q = middle(mid, controlP);

  if (mid.distance(q) < 0.1 /*error tolerance*/) {
    coords.push_back(oglVertex(endP));
  } else {
    // Error is too large; subdivide.
    trace_curve(startP, middle(startP, controlP), q, coords);

    trace_curve(q, middle(controlP, endP), endP, coords);
  }
}

std::vector<oglVertex> interpolate(const std::vector<Edge>& edges,
        const float& anchor_x, const float& anchor_y)
{
  point anchor(anchor_x, anchor_y);
  
  std::vector<oglVertex> shape_points;
  shape_points.push_back(oglVertex(anchor));
  
  for (std::vector<Edge>::const_iterator it = edges.begin(), end = edges.end();
        it != end; ++it) {
      const Edge& the_edge = *it;
      
      point target(the_edge.ap.x, the_edge.ap.y);

      if (the_edge.straight()) {
        shape_points.push_back(oglVertex(target));
      } else {
        point control(the_edge.cp.x, the_edge.cp.y);
        
        trace_curve(anchor, control, target, shape_points);
      }
      anchor = target;
  }
  
  return shape_points;  
}

// FIXME: OSX doesn't like void (*)().
Tesselator::Tesselator()
: _tessobj(gluNewTess())
{
  gluTessCallback(_tessobj, GLU_TESS_ERROR, 
                  reinterpret_cast<GLUCALLBACKTYPE>(Tesselator::error));
  gluTessCallback(_tessobj, GLU_TESS_COMBINE_DATA,
                  reinterpret_cast<GLUCALLBACKTYPE>(Tesselator::combine));
  
  gluTessCallback(_tessobj, GLU_TESS_BEGIN,
                  reinterpret_cast<GLUCALLBACKTYPE>(glBegin));
  gluTessCallback(_tessobj, GLU_TESS_END,
                  reinterpret_cast<GLUCALLBACKTYPE>(glEnd));
                  
  gluTessCallback(_tessobj, GLU_TESS_VERTEX,
                  reinterpret_cast<GLUCALLBACKTYPE>(glVertex3dv)); 
  
#if 0        
  // for testing, draw only the outside of shapes.          
  gluTessProperty(_tessobj, GLU_TESS_BOUNDARY_ONLY, GL_TRUE);
#endif
                    
  // all coordinates lie in the x-y plane
  // this speeds up tesselation 
  gluTessNormal(_tessobj, 0.0, 0.0, 1.0);
}
  
Tesselator::~Tesselator()
{
  gluDeleteTess(_tessobj);  
}
  
void
Tesselator::beginPolygon()
{
  gluTessBeginPolygon(_tessobj, this);
}

void Tesselator::beginContour()
{
  gluTessBeginContour(_tessobj);
}

void
Tesselator::feed(std::vector<oglVertex>& vertices)
{
  for (std::vector<oglVertex>::const_iterator it = vertices.begin(), end = vertices.end();
        it != end; ++it) {
    GLdouble* vertex = const_cast<GLdouble*>(&(*it)._x);
    gluTessVertex(_tessobj, vertex, vertex);
  }
}

void
Tesselator::endContour()
{
  gluTessEndContour(_tessobj);  
}
  
void
Tesselator::tesselate()
{
  gluTessEndPolygon(_tessobj);

  for (std::vector<GLdouble*>::iterator it = _vertices.begin(),
       end = _vertices.end(); it != end; ++it) {
    delete [] *it;
  }

  _vertices.clear();
}

void
Tesselator::rememberVertex(GLdouble* v)
{
  _vertices.push_back(v);
}

// static
void
Tesselator::error(GLenum error)
{  
  log_error("GLU: %s", gluErrorString(error));
}

// static
void
Tesselator::combine(GLdouble coords [3], void **/*vertex_data[4] */,
		    GLfloat * /* weight[4] */, void **outData, void* userdata)
{
  Tesselator* tess = (Tesselator*)userdata;
  assert(tess);

  GLdouble* v = new GLdouble[3];
  v[0] = coords[0];
  v[1] = coords[1];
  v[2] = coords[2];
  
  *outData = v;
  
  tess->rememberVertex(v);
}

bool isEven(const size_t& n)
{
  return n % 2 == 0;
}

bitmap_info_ogl::bitmap_info_ogl(std::auto_ptr<image::GnashImage> image,
        GLenum pixelformat, bool ogl_accessible)
:
  _img(image.release()),
  _pixel_format(pixelformat),
  _ogl_img_type(_img->height() == 1 ? GL_TEXTURE_1D : GL_TEXTURE_2D),
  _ogl_accessible(ogl_accessible),
  _texture_id(0),
  _orig_width(_img->width()),
  _orig_height(_img->height()),
  _disposed(false)
{   
  if (!_ogl_accessible) {
    return;      
  }
  
  setup();
}   
    
bitmap_info_ogl::~bitmap_info_ogl()
{
  glDeleteTextures(1, &_texture_id);
  glDisable(_ogl_img_type);
}

void
bitmap_info_ogl::setup() const
{      
    oglScopeEnable enabler(_ogl_img_type);
  
    glGenTextures(1, &_texture_id);
    glBindTexture(_ogl_img_type, _texture_id);
  
    bool resize = false;
    if (_img->height() == 1) {
        if (!isEven(_img->width())) {
            resize = true;
        }
    } 
    else {
        if (!isEven(_img->width()) || !isEven(_img->height())) {
            resize = true;
        }     
    }
  
    if (!resize) {
        upload(_img->begin(), _img->width(), _img->height());
    } 
    else {     
    
        size_t w = 1; while (w < _img->width()) { w <<= 1; }
        size_t h = 1;
        if (_img->height() != 1) {
            while (h < _img->height()) { h <<= 1; }
        }
    
        boost::scoped_array<boost::uint8_t> resized_data(
                new boost::uint8_t[w * h * _img->channels()]);
        // Q: Would mipmapping these textures aid in performance?
    
        GLint rv = gluScaleImage(_pixel_format, _img->width(),
                _img->height(), GL_UNSIGNED_BYTE, _img->begin(), w, h,
                GL_UNSIGNED_BYTE, resized_data.get());
        if (rv != 0) {
            Tesselator::error(rv);
            assert(0);
        }
    
        upload(resized_data.get(), w, h);
    }
  
    _img.reset();
}

void
bitmap_info_ogl::upload(boost::uint8_t* data, size_t width, size_t height) const
{
  glTexParameteri(_ogl_img_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  
  // FIXME: confirm that OpenGL can handle this image
  
  if (_ogl_img_type == GL_TEXTURE_1D) {
    glTexImage1D(GL_TEXTURE_1D, 0, _pixel_format, width,
                  0, _pixel_format, GL_UNSIGNED_BYTE, data);
  
  } else {      
    glTexImage2D(_ogl_img_type, 0, _pixel_format, width, height,
                  0, _pixel_format, GL_UNSIGNED_BYTE, data);

  }
}

void
bitmap_info_ogl::apply(const gnash::SWFMatrix& bitmap_matrix,
                       bitmap_wrap_mode wrap_mode) const
{
    // TODO: use the altered data. Currently it doesn't display anything,
    // so I don't feel obliged to implement this!

  glEnable(_ogl_img_type);

  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);
  
  if (!_ogl_accessible) {
    // renderer context wasn't available when this class was instantiated.
    _ogl_accessible=true;
    setup();
  }
  
  glEnable(_ogl_img_type);
  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);    
  
  glBindTexture(_ogl_img_type, _texture_id);
  
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  
  if (wrap_mode == WRAP_CLAMP) {  
    glTexParameteri(_ogl_img_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(_ogl_img_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  } else {
    glTexParameteri(_ogl_img_type, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(_ogl_img_type, GL_TEXTURE_WRAP_T, GL_REPEAT);
  }
    
  // Set up the bitmap SWFMatrix for texgen.
    
  float inv_width = 1.0f / _orig_width;
  float inv_height = 1.0f / _orig_height;
    
  const gnash::SWFMatrix& m = bitmap_matrix;
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  float p[4] = { 0, 0, 0, 0 };
  p[0] = m.a() / 65536.0f * inv_width;
  p[1] = m.c() / 65536.0f * inv_width;
  p[3] = m.tx() * inv_width;
  glTexGenfv(GL_S, GL_OBJECT_PLANE, p);

  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  p[0] = m.b() / 65536.0f * inv_height;
  p[1] = m.c() / 65536.0f * inv_height;
  p[3] = m.ty() * inv_height;
  glTexGenfv(GL_T, GL_OBJECT_PLANE, p);
  
}

template<typename C, typename T, typename R, typename A>
void 
for_each(C& container, R (T::*pmf)(const A&),const A& arg)
{
    std::for_each(container.begin(), container.end(),
                  boost::bind(pmf, _1, boost::ref(arg)));
}



class DSOEXPORT Renderer_ogl : public Renderer
{
public: 
  Renderer_ogl()
    : _xscale(1.0),
      _yscale(1.0),
      _drawing_mask(false)
  {
  }

  std::string description() const { return "OpenGL"; }
  
  void init()
  {
    // Turn on alpha blending.
    // FIXME: do this when it's actually used?
    glEnable(GL_BLEND);
    // This blend operation is best for rendering antialiased points and lines in 
    // no particular order.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Turn on line smoothing.  Antialiased lines can be used to
    // smooth the outsides of shapes.
    glEnable(GL_LINE_SMOOTH);

    // Use fastest line smoothing since additional anti-aliasing will happen later.
    glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST); // GL_NICEST, GL_FASTEST, GL_DONT_CARE

    glMatrixMode(GL_PROJECTION);
    
    float oversize = 1.0;

    // Flip the image, since (0,0) by default in OpenGL is the bottom left.
    gluOrtho2D(-oversize, oversize, oversize, -oversize);
    // Restore the SWFMatrix mode to the default.
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glShadeModel(GL_FLAT);
  
  }
  
  ~Renderer_ogl()
  {
  }
  
  inline bool
  ogl_accessible() const
  {
#if defined(_WIN32) || defined(WIN32)
    return wglGetCurrentContext();
#elif defined(__APPLE_CC__)
    return aglGetCurrentContext();
#else
# ifdef OSMESA_TESTING
    if (_offscreen.get()) {
      return OSMesaGetCurrentContext();
    }
# endif
    return glXGetCurrentContext();
#endif
  }    

  virtual CachedBitmap* createCachedBitmap(std::auto_ptr<image::GnashImage> im)
  {
      switch (im->type()) {
          case image::TYPE_RGB:
          {
              std::auto_ptr<image::GnashImage> rgba(
                      new image::ImageRGBA(im->width(), im->height()));

              image::GnashImage::iterator it = rgba->begin();
              for (size_t i = 0; i < im->size(); ++i) {
                  *it++ = *(im->begin() + i);
                  if (!(i % 3)) *it++ = 0xff;
              }
              im = rgba;
          }
          case image::TYPE_RGBA:
                return new bitmap_info_ogl(im, GL_RGBA, ogl_accessible());
          default:
                std::abort();
      }
  }

  boost::shared_ptr<GnashTexture> getCachedTexture(image::GnashImage *frame)
  {
      boost::shared_ptr<GnashTexture> texture;
      GnashTextureFormat frameFormat(frame->type());
      unsigned int frameFlags;

      switch (frame->location()) {
      case image::GNASH_IMAGE_CPU:
          frameFlags = 0;
          break;
#ifdef HAVE_VA_VA_GLX_H
      case image::GNASH_IMAGE_GPU:
          frameFlags = GNASH_TEXTURE_VAAPI;
          break;
#endif
      default:
          assert(0);
          return texture;
      }

      // Look for a texture with the same dimensions and type
      std::list< boost::shared_ptr<GnashTexture> >::iterator it;
      for (it = _cached_textures.begin(); it != _cached_textures.end(); it++) {
          if ((*it)->width() == frame->width() &&
              (*it)->height() == frame->height() &&
              (*it)->internal_format() == frameFormat.internal_format() &&
              (*it)->format() == frameFormat.format() &&
              (*it)->flags() == frameFlags)
              break;
      }

      // Found texture and remove it from cache. It will be pushed
      // back into the cache when rendering is done, in end_display()
      if (it != _cached_textures.end()) {
          texture = *it;
          _cached_textures.erase(it);
      }

      // Otherwise, create one and empty cache because they may no
      // longer be referenced
      else {
          _cached_textures.clear();

          switch (frame->location()) {
          case image::GNASH_IMAGE_CPU:
              texture.reset(new GnashTexture(frame->width(),
                                             frame->height(),
                                             frame->type()));
              break;
          case image::GNASH_IMAGE_GPU:
              // This case should never be reached if vaapi is not
              // enabled; but has to be handled to keep the compiler
              // happy.
#ifdef HAVE_VA_VA_GLX_H
              texture.reset(new GnashVaapiTexture(frame->width(),
                                                  frame->height(),
                                                  frame->type()));
#endif
              break;
          }
      }

      assert(texture->width() == frame->width());
      assert(texture->height() == frame->height());
      assert(texture->internal_format() == frameFormat.internal_format());
      assert(texture->format() == frameFormat.format());
      assert(texture->flags() == frameFlags);
      return texture;
  }

  // Since we store drawing operations in display lists, we take special care
  // to store video frame operations in their own display list, lest they be
  // anti-aliased with the rest of the drawing. Since display lists cannot be
  // concatenated this means we'll add up with several display lists for normal
  // drawing operations.
  virtual void drawVideoFrame(image::GnashImage* frame, const Transform& xform,
          const SWFRect* bounds, bool /*smooth*/)
  {
    GLint index;

    glGetIntegerv(GL_LIST_INDEX, &index);

    if (index >= 255) {
      log_error("An insane number of video frames have been requested to be "
                "drawn. Further video frames will be ignored.");
      return;
    }

    glEndList();

    boost::shared_ptr<GnashTexture> texture = getCachedTexture(frame);
    if (!texture.get())
        return;

    switch (frame->location()) {
    case image::GNASH_IMAGE_CPU:
        texture->update(frame->begin());
        break;
#ifdef HAVE_VA_VA_GLX_H
    case image::GNASH_IMAGE_GPU:
        dynamic_cast<GnashVaapiTexture *>(texture.get())->update(dynamic_cast<GnashVaapiImage *>(frame)->surface());
        break;
#endif
    default:
        assert(0);
        return;
    }
    _render_textures.push_back(texture);

    glGenLists(2);

    ++index;

    glNewList(index, GL_COMPILE);
    _render_indices.push_back(index);

    reallyDrawVideoFrame(texture, &xform.matrix, bounds);

    glEndList();

    ++index;

    glNewList(index, GL_COMPILE);
    _render_indices.push_back(index);
  }

private:  
  void reallyDrawVideoFrame(boost::shared_ptr<GnashTexture> texture, const SWFMatrix* m, const SWFRect* bounds)
  {
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
    glPushMatrix();

    gnash::point l, u;
    m->transform(&l, point(bounds->get_x_min(), bounds->get_y_min()));
    m->transform(&u, point(bounds->get_x_max(), bounds->get_y_max()));
    const unsigned int w = u.x - l.x;
    const unsigned int h = u.y - l.y;

    texture->bind();
    glTranslatef(l.x, l.y, 0.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    {
        glTexCoord2f(0.0f, 0.0f); glVertex2i(0, 0);
        glTexCoord2f(0.0f, 1.0f); glVertex2i(0, h);
        glTexCoord2f(1.0f, 1.0f); glVertex2i(w, h);
        glTexCoord2f(1.0f, 0.0f); glVertex2i(w, 0);
    }
    glEnd();
    texture->release();

    glPopMatrix();
    glPopAttrib();
  }

  // FIXME
  geometry::Range2d<int>
  world_to_pixel(const SWFRect& worldbounds) const
  {
    // TODO: verify this is correct
    geometry::Range2d<int> ret(worldbounds.getRange());
    ret.scale(1.0/20.0); // twips to pixels
    return ret;
  }

  // FIXME
  point 
  pixel_to_world(int x, int y) const
  {
    // TODO: verify this is correct
    return point(pixelsToTwips(x), pixelsToTwips(y));
  }

public:
      
    virtual Renderer* startInternalRender(image::GnashImage& /*im*/) {
        return 0;
    }

    virtual void endInternalRender() {}

  virtual void  begin_display(
    const rgba& bg_color,
    int viewport_width, int viewport_height,
    float x0, float x1, float y0, float y1)
  {
    glViewport(0, 0, viewport_width, viewport_height);
    glLoadIdentity();

    gluOrtho2D(x0, x1, y0, y1);
    
    _width  = std::fabs(x1 - x0);
    _height = std::fabs(y1 - y0);

    glScalef(static_cast<float>(twipsToPixels(_width)) /
    static_cast<float>(viewport_width),
    static_cast<float>(twipsToPixels(_height)) / 
    static_cast<float>(viewport_height), 1.0f);

    // Setup the clear color. The actual clearing will happen in end_display.
    if (bg_color.m_a) {
      glClearColor(bg_color.m_r / 255.0, bg_color.m_g / 255.0, bg_color.m_b / 255.0, 
                   bg_color.m_a / 255.0);
    } else {
      glClearColor(1.0, 1.0, 1.0, 1.0);
    }

    glGenLists(1);
    
    // Start a new display list which will contain almost everything we draw.
    glNewList(1, GL_COMPILE);
    _render_indices.push_back(1);
    
  }
  
  virtual void
  end_display()
  {
    glEndList();    
    
#if NO_ANTIALIASING
    // Don't use accumulation buffer based anti-aliasing
    glClear(GL_COLOR_BUFFER_BIT);
    glCallLists(_render_indices.size(), GL_UNSIGNED_BYTE,
                &_render_indices.front());
#else
    // This is a table of randomly generated numbers between -0.5 and 0.5.
    struct {
      GLfloat x;
      GLfloat y;
    } points [] = {
      {      	-0.448823,  	 0.078771 },
      {     	-0.430852, 	0.240592 },
      {     	-0.208887, 	-0.492535 },
      {     	-0.061232, 	-1.109432 },
      {     	-0.034984, 	-0.247317 },
      {     	-0.367119, 	-0.440909 },
      {     	0.047688, 	0.315757 },
      {     	0.434600, 	-0.204068 }
    };

    int numPoints = 8;    
    
    GLint viewport[4];
    glGetIntegerv (GL_VIEWPORT, viewport);
    const GLint& viewport_width = viewport[2],
                 viewport_height = viewport[3];

    glClearAccum(0.0, 0.0, 0.0, 0.0);

    glClear(GL_ACCUM_BUFFER_BIT);

    for (int i = 0; i < numPoints; ++i) {
    
      glClear(GL_COLOR_BUFFER_BIT);
      glPushMatrix ();

      glTranslatef (points[i].x * _width / viewport_width,
                    points[i].y * _height / viewport_height, 0.0);

      glCallLists(_render_indices.size(), GL_UNSIGNED_BYTE,
                  &_render_indices.front());
      
      glPopMatrix ();
      
      glAccum(GL_ACCUM, 1.0/numPoints);
    }
    
    glAccum (GL_RETURN, 1.0);
#endif
  
  #if 0
    GLint box[4];
    glGetIntegerv(GL_SCISSOR_BOX, box);
    
    int x = pixelsToTwips(box[0]),
        y = pixelsToTwips(box[1]),
        w = pixelsToTwips(box[2]),
        h = pixelsToTwips(box[3]);

    glRectd(x, y - h, x + w, y + h);
  #endif

    glDeleteLists(1, _render_indices.size());
    _render_indices.clear();

    for (size_t i = 0; i < _render_textures.size(); i++)
        _cached_textures.push_front(_render_textures[i]);
    _render_textures.clear();
  
    check_error();

    glFlush(); // Make OpenGL execute all commands in the buffer.
  }
  
  /// Draw a line-strip directly, using a thin, solid line. 
  //
  /// Can be used to draw empty boxes and cursors.
  virtual void drawLine(const std::vector<point>& coords, const rgba& color,
                  const SWFMatrix& mat)
  {
    oglScopeMatrix scope_mat(mat);

    const size_t numPoints = coords.size();

    glColor3ub(color.m_r, color.m_g, color.m_b);

    std::vector<boost::int16_t> pointList;
    pointList.reserve(numPoints * 2);
    std::for_each(coords.begin(), coords.end(), PointSerializer(pointList));

    // Send the line-strip to OpenGL
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_SHORT, 0 /* tight packing */, &pointList.front());
    glDrawArrays(GL_LINE_STRIP, 0, numPoints);

    // Draw a dot on the beginning and end coordinates to round lines.
    // glVertexPointer: skip all but the first and last coordinates in the line.
    glVertexPointer(2, GL_SHORT, (sizeof(boost::int16_t) * 2) *
            (numPoints - 1), &pointList.front());
    glEnable(GL_POINT_SMOOTH); // Draw a round (antialiased) point.
    glDrawArrays(GL_POINTS, 0, 2);
    glDisable(GL_POINT_SMOOTH);
    glPointSize(1); // return to default

    glDisableClientState(GL_VERTEX_ARRAY);
  }

  // NOTE: this implementation can't handle complex polygons (such as concave
  // polygons.
  virtual void draw_poly(const std::vector<point>& corners, 
    const rgba& fill, const rgba& outline, const SWFMatrix& mat, bool /* masked */)
  {
    if (corners.empty()) return;

    oglScopeMatrix scope_mat(mat);

    glColor4ub(fill.m_r, fill.m_g, fill.m_b, fill.m_a);

    glEnableClientState(GL_VERTEX_ARRAY);

    // Draw simple polygon
    glVertexPointer(2, GL_FLOAT, 0 /* tight packing */, &corners.front());
    glDrawArrays(GL_POLYGON, 0, corners.size());

    // Draw outline
    glLineWidth(1.0);
    glColor4ub(outline.m_r, outline.m_g, outline.m_b, outline.m_a);
    glVertexPointer(2, GL_FLOAT, 0 /* tight packing */, &corners.front());
    glDrawArrays(GL_LINE_LOOP, 0, corners.size());

    glDisableClientState(GL_VERTEX_ARRAY);

    glPopMatrix();
  }

  virtual void  set_antialiased(bool /* enable */ )
  {
    log_unimpl("set_antialiased");
  }
    
  virtual void begin_submit_mask()
  {
    PathVec mask;
    _masks.push_back(mask);
    
    _drawing_mask = true;
  }
  
  virtual void end_submit_mask()
  {
    _drawing_mask = false;
    
    apply_mask();
  }

  /// Apply the current mask; nesting is supported.
  ///
  /// This method marks the stencil buffer by incrementing every stencil pixel
  /// by one every time a solid from one of the current masks is drawn. When
  /// all the mask solids are drawn, we change the stencil operation to permit
  /// only drawing where all masks have drawn, in other words, where all masks
  /// intersect, or in even other words, where the stencil pixel buffer equals
  /// the number of masks.
  void apply_mask()
  {
    if (_masks.empty()) return;
    
    glEnable(GL_STENCIL_TEST);

    glClearStencil(0x0); // FIXME: default is zero, methinks
    glClear(GL_STENCIL_BUFFER_BIT);

    // GL_NEVER means the stencil test will never succeed, so OpenGL wil never
    // continue to the drawing stage.
    glStencilFunc (GL_NEVER, 0x1, 0x1);

    glStencilOp (GL_INCR /* Stencil test fails */,
                 GL_KEEP /* ignored */,
                 GL_KEEP /* Stencil test passes; never happens */);

    // Call add_paths for each mask.
    std::for_each(_masks.begin(), _masks.end(),
      boost::bind(&Renderer_ogl::add_paths, this, _1));    
          
    glStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilFunc(GL_EQUAL, _masks.size(), _masks.size());
  }
  
  void
  add_paths(const PathVec& path_vec)
  {
    SWFCxForm dummy_cx;
    std::vector<FillStyle> dummy_fs;
    
    FillStyle coloring = FillStyle(SolidFill(rgba(0, 0, 0, 0)));
    
    dummy_fs.push_back(coloring);
    
    std::vector<LineStyle> dummy_ls;
    
    draw_subshape(path_vec, SWFMatrix(), dummy_cx, dummy_fs, dummy_ls);
  }
  
  virtual void disable_mask()
  {
    _masks.pop_back();
    
    if (_masks.empty()) {
      glDisable(GL_STENCIL_TEST);
    } else {
      apply_mask();
    }
  }

#if 0
  void print_path(const Path& path)
  {
    std::cout << "Origin: ("
              << path.ap.x
              << ", "
              << path.ap.y
              << ") fill0: "
              << path.m_fill0
              << " fill1: "
              << path.m_fill1
              << " line: "
              << path.m_line
              << " new shape: "
              << path.m_new_shape              
              << " number of edges: "
              << path.m_edges.size()
              << " edge endpoint: ("
              << path.m_edges.back().ap.x
              << ", "
              << path.m_edges.back().ap.y
              << " ) points:";

    for (std::vector<edge>::const_iterator it = path.m_edges.begin(), end = path.m_edges.end();
         it != end; ++it) {
      const edge& cur_edge = *it;
      std::cout << "( " << cur_edge.ap.x << ", " << cur_edge.ap.y << ") ";
    }
    std::cout << std::endl;             
  }
#endif
  
  
  Path reverse_path(const Path& cur_path)
  {
    const Edge& cur_end = cur_path.m_edges.back();    
        
    float prev_cx = cur_end.cp.x;
    float prev_cy = cur_end.cp.y;        
                
    Path newpath(cur_end.ap.x, cur_end.ap.y, cur_path.m_fill1, cur_path.m_fill0, cur_path.m_line, cur_path.m_new_shape);
    
    float prev_ax = cur_end.ap.x;
    float prev_ay = cur_end.ap.y; 

    for (std::vector<Edge>::const_reverse_iterator it = cur_path.m_edges.rbegin()+1, end = cur_path.m_edges.rend();
         it != end; ++it) {
      const Edge& cur_edge = *it;

      if (prev_ax == prev_cx && prev_ay == prev_cy) {
        prev_cx = cur_edge.ap.x;
        prev_cy = cur_edge.ap.y;      
      }

      Edge newedge(prev_cx, prev_cy, cur_edge.ap.x, cur_edge.ap.y); 
          
      newpath.m_edges.push_back(newedge);
          
      prev_cx = cur_edge.cp.x;
      prev_cy = cur_edge.cp.y;
      prev_ax = cur_edge.ap.x;
      prev_ay = cur_edge.ap.y;
           
    }
        
    Edge newlastedge(prev_cx, prev_cy, cur_path.ap.x, cur_path.ap.y);    
    newpath.m_edges.push_back(newlastedge);
        
    return newpath;
  }
  
  const Path* find_connecting_path(const Path& to_connect,
                                   std::list<const Path*> path_refs)
  {
        
    float target_x = to_connect.m_edges.back().ap.x;
    float target_y = to_connect.m_edges.back().ap.y;

    if (target_x == to_connect.ap.x &&
        target_y == to_connect.ap.y) {
      return NULL;
    }
  
    for (std::list<const Path*>::const_iterator it = path_refs.begin(), end = path_refs.end();
         it != end; ++it) {
      const Path* cur_path = *it;
      
      if (cur_path == &to_connect) {
      
        continue;
      
      }
      
            
      if (cur_path->ap.x == target_x && cur_path->ap.y == target_y) {
 
        if (cur_path->m_fill1 != to_connect.m_fill1) {
          continue;
        }
        return cur_path;
      }
    }
  
  
    return NULL;  
  }
  
  PathVec normalize_paths(const PathVec &paths)
  {
    PathVec normalized;
  
    for (PathVec::const_iterator it = paths.begin(), end = paths.end();
         it != end; ++it) {
      const Path& cur_path = *it;
      
      if (cur_path.m_edges.empty()) {
        continue;
      
      } else if (cur_path.m_fill0 && cur_path.m_fill1) {     
        
        // Two fill styles; duplicate and then reverse the left-filled one.
        normalized.push_back(cur_path);
        normalized.back().m_fill0 = 0; 
     
        Path newpath = reverse_path(cur_path);
        newpath.m_fill0 = 0;        
           
        normalized.push_back(newpath);       

      } else if (cur_path.m_fill0) {
        // Left fill style.
        Path newpath = reverse_path(cur_path);
        newpath.m_fill0 = 0;
           
        normalized.push_back(newpath);
      } else if (cur_path.m_fill1) {
        // Right fill style.

        normalized.push_back(cur_path);
      } else {
        // No fill styles; copy without modifying.
        normalized.push_back(cur_path);
      }

    }
    
    return normalized;
  }
  
  
  
  
  
  
  /// Analyzes a set of paths to detect real presence of fills and/or outlines
  /// TODO: This should be something the character tells us and should be 
  /// cached. 
  void analyze_paths(const PathVec &paths, bool& have_shape,
    bool& have_outline) {
    //normalize_paths(paths);
    have_shape=false;
    have_outline=false;
    
    int pcount = paths.size();
    
    for (int pno=0; pno<pcount; pno++) {
    
      const Path &the_path = paths[pno];
    
      if ((the_path.m_fill0>0) || (the_path.m_fill1>0)) {
        have_shape=true;
        if (have_outline) return; // have both
      }
    
      if (the_path.m_line>0) {
        have_outline=true;
        if (have_shape) return; // have both
      }
    
    }    
  }

  void apply_FillStyle(const FillStyle& style, const SWFMatrix& /* mat */, const SWFCxForm& cx)
  {
      const StyleHandler st(cx, *this);
      boost::apply_visitor(st, style.fill);
  }
  
  bool apply_line_style(const LineStyle& style, const SWFCxForm& cx, const SWFMatrix& mat)
  {
  //  GNASH_REPORT_FUNCTION;
     
    // In case GL_TEXTURE_2D was enabled by apply_FillStyle(), disable it now.
    // FIXME: this sucks
    glDisable(GL_TEXTURE_2D);
    
    
    bool rv = true;
    
    float width = style.getThickness();
    //    float pointSize;
    
    if (!width)
    {
      glLineWidth(1.0f);
      rv = false; // Don't draw rounded lines.
    }
    else if ( (!style.scaleThicknessVertically()) && (!style.scaleThicknessHorizontally()) )
    {
      float pxThickness = twipsToPixels(width);
      glLineWidth(pxThickness);
      glPointSize(pxThickness);
    }
    else
    {
      if ( (!style.scaleThicknessVertically()) || (!style.scaleThicknessHorizontally()) )
      {
         LOG_ONCE( log_unimpl(_("Unidirectionally scaled strokes in OGL renderer")) );
      }
      
      float stroke_scale = std::fabs(mat.get_x_scale()) + std::fabs(mat.get_y_scale());
      stroke_scale /= 2.0f;
      stroke_scale *= (std::fabs(_xscale) + std::fabs(_yscale)) / 2.0f;
      width *= stroke_scale;
      width = twipsToPixels(width);

      GLfloat width_info[2];
      
      glGetFloatv(GL_LINE_WIDTH_RANGE, width_info);          
      
      if (width > width_info[1]) {
        LOG_ONCE( log_unimpl("Your OpenGL implementation does not support the line width" \
                  " requested. Lines will be drawn with reduced width.") );
        width = width_info[1];
      }
      
      
      glLineWidth(width);
      glPointSize(width);
#if 0
      if (width >= 1.5) {
        glPointSize(width-1);
      } else {
        // Don't draw rounded lines.
        rv = false;
      }
#endif
      
      
    }

    rgba c = cx.transform(style.get_color());

    glColor4ub(c.m_r, c.m_g, c.m_b, c.m_a);
    
    return rv;
  }
 
  PathPointMap getPathPoints(const PathVec& path_vec)
  {
  
    PathPointMap pathpoints;
    
    for (PathVec::const_iterator it = path_vec.begin(), end = path_vec.end();
         it != end; ++it) {
      const Path& cur_path = *it;

      if (!cur_path.m_edges.size()) {
        continue;
      }
        
      pathpoints[&cur_path] = interpolate(cur_path.m_edges, cur_path.ap.x,
                                                            cur_path.ap.y);

    }
    
    return pathpoints;
  } 
  
  typedef std::vector<const Path*> PathPtrVec;
  
  static void
  draw_point(const Edge& point_edge)
  {  
    glVertex2d(point_edge.ap.x, point_edge.ap.y);  
  }
    
  void
  draw_outlines(const PathVec& path_vec, const PathPointMap& pathpoints,
		const SWFMatrix& mat, const SWFCxForm& cx,
		const std::vector<FillStyle>& /* FillStyles */,
                const std::vector<LineStyle>& line_styles)
  {
  
    for (PathVec::const_iterator it = path_vec.begin(), end = path_vec.end();
         it != end; ++it) {
      const Path& cur_path = *it;
      
      if (!cur_path.m_line) {
        continue;
      }
      
      bool draw_points = apply_line_style(line_styles[cur_path.m_line-1], cx, mat);
      
      assert(pathpoints.find(&cur_path) != pathpoints.end());
      
      const std::vector<oglVertex>& shape_points = (*pathpoints.find(&cur_path)).second;

      // Draw outlines.
      glEnableClientState(GL_VERTEX_ARRAY);
      glVertexPointer(3, GL_DOUBLE, 0 /* tight packing */, &shape_points.front());
      glDrawArrays(GL_LINE_STRIP, 0, shape_points.size());
      glDisableClientState(GL_VERTEX_ARRAY);

      if (!draw_points) {
        continue;
      }
      
      // Drawing points on the edges will allow us to simulate rounded lines.

      glEnable(GL_POINT_SMOOTH); // Draw round points.      

      glBegin(GL_POINTS);
      {
        glVertex2d(cur_path.ap.x, cur_path.ap.y);
        std::for_each(cur_path.m_edges.begin(), cur_path.m_edges.end(),
                      draw_point);
      }
      glEnd();
    }
  }

  std::list<PathPtrVec> get_contours(const PathPtrVec &paths)
  {
    std::list<const Path*> path_refs;
    std::list<PathPtrVec> contours;
    
    
    for (PathPtrVec::const_iterator it = paths.begin(), end = paths.end();
         it != end; ++it) {
      const Path* cur_path = *it;
      path_refs.push_back(cur_path);
    }
        
    for (std::list<const Path*>::const_iterator it = path_refs.begin(), end = path_refs.end();
         it != end; ++it) {
      const Path* cur_path = *it;
      
      if (cur_path->m_edges.empty()) {
        continue;
      }
      
      if (!cur_path->m_fill0 && !cur_path->m_fill1) {
        continue;
      }
      
      PathPtrVec contour;
            
      contour.push_back(cur_path);
        
      const Path* connector = find_connecting_path(*cur_path, path_refs);

      while (connector) {       
        contour.push_back(connector);

        const Path* tmp = connector;
        connector = find_connecting_path(*connector, std::list<const Path*>(boost::next(it), end));
  
        // make sure we don't iterate over the connecting path in the for loop.
        path_refs.remove(tmp);
        
      } 
      
      contours.push_back(contour);   
    }
    
    return contours;
  }

  
  void draw_mask(const PathVec& path_vec)
  {    
    for (PathVec::const_iterator it = path_vec.begin(), end = path_vec.end();
         it != end; ++it) {
      const Path& cur_path = *it;
      
      if (cur_path.m_fill0 || cur_path.m_fill1) {
        _masks.back().push_back(cur_path);
        _masks.back().back().m_line = 0;    
      }
    }  
  }
  
  PathPtrVec
  paths_by_style(const PathVec& path_vec, unsigned int style)
  {
    PathPtrVec paths;
    for (PathVec::const_iterator it = path_vec.begin(), end = path_vec.end();
         it != end; ++it) {
      const Path& cur_path = *it;
      
      if (cur_path.m_fill0 == style) {
        paths.push_back(&cur_path);
      }
      
      if (cur_path.m_fill1 == style) {
        paths.push_back(&cur_path);
      }
      
    }
    return paths;
  }
  
  
  std::vector<PathVec::const_iterator>
  find_subshapes(const PathVec& path_vec)
  {
    std::vector<PathVec::const_iterator> subshapes;
    
    PathVec::const_iterator it = path_vec.begin(),
                            end = path_vec.end();
    
    subshapes.push_back(it);
    ++it;

    for (;it != end; ++it) {
      const Path& cur_path = *it;
    
      if (cur_path.m_new_shape) {
        subshapes.push_back(it); 
      } 
    }

    if (subshapes.back() != end) {
      subshapes.push_back(end);
    }
    
    return subshapes;
  }
  
  /// Takes a path and translates it using the given SWFMatrix.
  void
  apply_matrix_to_paths(std::vector<Path>& paths, const SWFMatrix& mat)
  {  
    std::for_each(paths.begin(), paths.end(),
                  boost::bind(&Path::transform, _1, boost::ref(mat)));
                  
    //for_each(paths, &path::transform, mat);
  }  

  void
  draw_subshape(const PathVec& path_vec,
    const SWFMatrix& mat,
    const SWFCxForm& cx,
    const std::vector<FillStyle>& FillStyles,
    const std::vector<LineStyle>& line_styles)
  {
    PathVec normalized = normalize_paths(path_vec);
    PathPointMap pathpoints = getPathPoints(normalized);
    
    for (size_t i = 0; i < FillStyles.size(); ++i) {
      PathPtrVec paths = paths_by_style(normalized, i+1);
      
      if (!paths.size()) {
        continue;
      }
      
      std::list<PathPtrVec> contours = get_contours(paths);

      _tesselator.beginPolygon();
      
      for (std::list<PathPtrVec>::const_iterator iter = contours.begin(),
           final = contours.end(); iter != final; ++iter) {      
        const PathPtrVec& refs = *iter;
        
        _tesselator.beginContour();
                   
        for (PathPtrVec::const_iterator it = refs.begin(), end = refs.end();
             it != end; ++it) {
          const Path& cur_path = *(*it);          
          
          assert(pathpoints.find(&cur_path) != pathpoints.end());

          _tesselator.feed(pathpoints[&cur_path]);
          
        }
        
        _tesselator.endContour();
      }
      
      apply_FillStyle(FillStyles[i], mat, cx);

      // This is terrible, but since the renderer is half dead I don't care.
      try {
          boost::get<SolidFill>(FillStyles[i].fill);
      }
      catch (const boost::bad_get&) {
          // For non solid fills...
          glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
      }
      
      _tesselator.tesselate();
      
      try {
          boost::get<SolidFill>(FillStyles[i].fill);
      }
      catch (const boost::bad_get&) {
          // Restore to original.
          glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      }
      
      glDisable(GL_TEXTURE_GEN_S);
      glDisable(GL_TEXTURE_GEN_T);    
      glDisable(GL_TEXTURE_1D);
      glDisable(GL_TEXTURE_2D);      
    }
    
    draw_outlines(normalized, pathpoints, mat, cx, FillStyles, line_styles);
  }
  
// Drawing procedure:
// 1. Separate paths by subshape.
// 2. Separate subshapes by fill style.
// 3. For every subshape/fill style combo:
//  a. Separate contours: find closed shapes by connecting ends.
//  b. Apply fill style.
//  c. Feed the contours in the tesselator. (Render.)
//  d. Draw outlines for every path in the subshape with a line style.
//
// 4. ...
// 5. Profit!

  virtual void drawShape(const SWF::ShapeRecord& shape, const Transform& xform)
  {
  
    const PathVec& path_vec = shape.paths();

    if (!path_vec.size()) {
      // No paths. Nothing to draw...
      return;
    }
    
    if (_drawing_mask) {
      PathVec scaled_path_vec = path_vec;
      
      apply_matrix_to_paths(scaled_path_vec, xform.matrix);
      draw_mask(scaled_path_vec); 
      return;
    }    
    
    bool have_shape, have_outline;
    
    analyze_paths(path_vec, have_shape, have_outline);
    
    if (!have_shape && !have_outline) {
      return; // invisible character
    }    
    
    oglScopeMatrix scope_mat(xform.matrix);

    std::vector<PathVec::const_iterator> subshapes = find_subshapes(path_vec);
    
    const std::vector<FillStyle>& FillStyles = shape.fillStyles();
    const std::vector<LineStyle>& line_styles = shape.lineStyles();
    
    for (size_t i = 0; i < subshapes.size()-1; ++i) {
      PathVec subshape_paths;
      
      if (subshapes[i] != subshapes[i+1]) {
        subshape_paths = PathVec(subshapes[i], subshapes[i+1]);
      } else {
        subshape_paths.push_back(*subshapes[i]);
      }
      
      draw_subshape(subshape_paths, xform.matrix, xform.colorTransform,
              FillStyles, line_styles);
    }
  }

  virtual void drawGlyph(const SWF::ShapeRecord& rec, const rgba& c,
         const SWFMatrix& mat)
  {
    if (_drawing_mask) abort();
    SWFCxForm dummy_cx;
    std::vector<FillStyle> glyph_fs;
    
    FillStyle coloring = FillStyle(SolidFill(c));
    
    glyph_fs.push_back(coloring);
    
    std::vector<LineStyle> dummy_ls;
    
    oglScopeMatrix scope_mat(mat);
    
    draw_subshape(rec.paths(), mat, dummy_cx, glyph_fs, dummy_ls);
  }

  virtual void set_scale(float xscale, float yscale) {
    _xscale = xscale;
    _yscale = yscale;
  }

  virtual void set_invalidated_regions(const InvalidatedRanges& /* ranges */)
  {
#if 0
    if (ranges.isWorld() || ranges.isNull()) {
      glDisable(GL_SCISSOR_TEST);
      return;
    }    
    
    glEnable(GL_SCISSOR_TEST);
    
    geometry::Range2d<float> area = ranges.getFullArea;
    
    glScissor( (int)twipsToPixels(area.getMinX()), window_height-(int)twipsToPixels(area.getMaxY()),
               (int)twipsToPixels(area.width()), (int)twipsToPixels(area.height()));
#endif
  }

#ifdef OSMESA_TESTING
  bool getPixel(rgba& color_out, int x, int y) const
  {  
    return _offscreen->getPixel(color_out, x, y);
  }
  
  bool initTestBuffer(unsigned width, unsigned height)
  {
    GNASH_REPORT_FUNCTION;

    _offscreen.reset(new OSRenderMesa(width, height));

    init();
    
    return true;
  }

  unsigned int getBitsPerPixel() const
  {
    return _offscreen->getBitsPerPixel();
  }
#endif // OSMESA_TESTING
    

private:
  
  Tesselator _tesselator;
  float _xscale;
  float _yscale;
  float _width; // Width of the movie, in world coordinates.
  float _height;
  
  std::vector<PathVec> _masks;
  bool _drawing_mask;
  
  std::vector<boost::uint8_t> _render_indices;
  std::vector< boost::shared_ptr<GnashTexture> > _render_textures;
  std::list< boost::shared_ptr<GnashTexture> > _cached_textures;
  
#ifdef OSMESA_TESTING
  std::auto_ptr<OSRenderMesa> _offscreen;
#endif
}; // class Renderer_ogl
  
Renderer* create_handler(bool init)
// Factory.
{
  Renderer_ogl* renderer = new Renderer_ogl;
  if (init) {
    renderer->init();
  }
  return renderer;
}
 
namespace {

// TODO: this function is rubbish and shouldn't survive a rewritten OGL
// renderer.
rgba
sampleGradient(const GradientFill& fill, boost::uint8_t ratio)
{

    // By specs, first gradient should *always* be 0, 
    // anyway a malformed SWF could break this,
    // so we cannot rely on that information...
    if (ratio < fill.record(0).ratio) {
        return fill.record(0).color;
    }

    if (ratio >= fill.record(fill.recordCount() - 1).ratio) {
        return fill.record(fill.recordCount() - 1).color;
    }
        
    for (size_t i = 1, n = fill.recordCount(); i < n; ++i) {

        const GradientRecord& gr1 = fill.record(i);
        if (gr1.ratio < ratio) continue;

        const GradientRecord& gr0 = fill.record(i - 1);
        if (gr0.ratio > ratio) continue;

        float f = 0.0f;

        if (gr0.ratio != gr1.ratio) {
            f = (ratio - gr0.ratio) / float(gr1.ratio - gr0.ratio);
        }
        else {
            // Ratios are equal IFF first and second GradientRecord
            // have the same ratio. This would be a malformed SWF.
            IF_VERBOSE_MALFORMED_SWF(
                log_swferror(_("two gradients in a FillStyle "
                    "have the same position/ratio: %d"),
                    gr0.ratio);
            );
        }

        return lerp(gr0.color, gr1.color, f);
    }

    // Assuming gradients are ordered by ratio? see start comment
    return fill.record(fill.recordCount() - 1).color;
}

const CachedBitmap*
createGradientBitmap(const GradientFill& gf, Renderer& renderer)
{
    std::auto_ptr<image::ImageRGBA> im;

    switch (gf.type())
    {
        case GradientFill::LINEAR:
            // Linear gradient.
            im.reset(new image::ImageRGBA(256, 1));

            for (size_t i = 0; i < im->width(); i++) {

                rgba sample = sampleGradient(gf, i);
                im->setPixel(i, 0, sample.m_r, sample.m_g,
                        sample.m_b, sample.m_a);
            }
            break;

        case GradientFill::RADIAL:
            // Focal gradient.
            im.reset(new image::ImageRGBA(64, 64));

            for (size_t j = 0; j < im->height(); j++)
            {
                for (size_t i = 0; i < im->width(); i++)
                {
                    float radiusy = (im->height() - 1) / 2.0f;
                    float radiusx = radiusy + std::abs(radiusy * gf.focalPoint());
                    float y = (j - radiusy) / radiusy;
                    float x = (i - radiusx) / radiusx;
                    int ratio = std::floor(255.5f * std::sqrt(x*x + y*y));
                    
                    if (ratio > 255) ratio = 255;

                    rgba sample = sampleGradient(gf, ratio);
                    im->setPixel(i, j, sample.m_r, sample.m_g,
                            sample.m_b, sample.m_a);
                }
            }
            break;
        default:
            break;
    }

    const CachedBitmap* bi = renderer.createCachedBitmap(
                    static_cast<std::auto_ptr<image::GnashImage> >(im));

    return bi;
}

} 
  
} // namespace gnash::renderer::opengl
} // namespace gnash::renderer
} // namespace gnash

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
