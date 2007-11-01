// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

#ifndef __RENDER_HANDLER_OGL_H__
#define __RENDER_HANDLER_OGL_H__


#if defined(NOT_SGI_GL) || defined(__APPLE_CC__)
#include <AGL/agl.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#else
# include <GL/gl.h>
# ifdef WIN32
#  define GL_CLAMP_TO_EDGE 0x812F
# else
# include <GL/glx.h>
# endif
# include <GL/glu.h>
# ifndef APIENTRY
#  define APIENTRY
# endif
#endif




namespace gnash {




typedef std::vector<const path*> PathRefs;



struct oglVertex {
  oglVertex(double x, double y, double z = 0.0)
    : _x(x), _y(y), _z(z)
  {
  }
  
  oglVertex(const point& p)
    : _x(p.m_x), _y(p.m_y), _z(0.0)
  {
  }

  GLdouble _x;
  GLdouble _y;
  GLdouble _z;
};

typedef std::map< const path*, std::vector<oglVertex> > PathPointMap;

class Tesselator
{
public:
  Tesselator();  
  ~Tesselator();
  
  void beginPolygon();
  
  void feed(std::vector<oglVertex>& vertices);
  
  void tesselate();
  
  void beginContour();
  void endContour();
  
  void rememberVertex(GLdouble* v);
  
  static void
  error(GLenum error);

  static void combine(GLdouble coords [3], void *vertex_data[4],
                      GLfloat weight[4], void **outData, void* userdata);
  

  
private:
  std::vector<GLdouble*> _vertices;
  GLUtesselator* _tessobj;
};

class WholeShape
{
public:
  void newPath(const path& new_path)
  {
    PathRefs refs;
    refs.push_back(&new_path);
    
    shape.push_back(refs);
  }
  
  void addPath(const path& add_path)
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






} // namespace gnash


#endif // __RENDER_HANDLER_OGL_H__

