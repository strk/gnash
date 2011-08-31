// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011
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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <sys/time.h>
#include <cstring>
#include <cmath>
#include <iostream>
#include <boost/utility.hpp>
#include <boost/bind.hpp>

#include "log.h"
#include "RGBA.h"
#include "GnashImage.h"
#include "GnashNumeric.h"
#include "FillStyle.h"
#include "LineStyle.h"
#include "Transform.h"
#include "log.h"
#include "utility.h"
#include "Range2d.h"
#include "SWFCxForm.h"
#include "openvg/OpenVGRenderer.h"
#include "openvg/OpenVGBitmap.h"
#include "openvg/OpenVGStyle.h"
#include "SWFMatrix.h"
#include "swf/ShapeRecord.h"
#include "CachedBitmap.h"

#include <VG/vgu.h>
#ifdef HAVE_VG_EXT_H
# include <VG/ext.h>
#else
# ifdef HAVE_VG_VGEXT_H
#  include <VG/vgext.h>
# endif
#endif
#include <VG/openvg.h>
#define GNASH_IMAGE_QUALITY     VG_IMAGE_QUALITY_FASTER
#define GNASH_RENDER_QUALITY    VG_RENDERING_QUALITY_FASTER

#define  MAX_POINTS (4096)

/// \file Renderer_ovg.cpp
/// \brief The OpenVG renderer and related code.
///

static const int TwipsPerInch = 1440;

namespace gnash {

typedef std::vector<Path> PathVec;
typedef std::vector<geometry::Range2d<int> > ClipBounds;

namespace renderer {

namespace openvg {

/// Transforms the current OpenVG SWFMatrix using the given SWFMatrix.
/// When it goes out of scope, the SWFMatrix will be reset to what it
/// was before the new SWFMatrix was applied.
class eglScopeMatrix : public boost::noncopyable
{
public:
    eglScopeMatrix(const SWFMatrix& m)
        {
            // GNASH_REPORT_FUNCTION;            

            vgGetMatrix(_orig_mat);
            // Renderer_ovg::printVGMatrix(_orig_mat);
            
            float mat[9];
            memset(mat, 0, sizeof(mat));
            mat[0] = m.a() / 65536.0f;
            mat[1] = m.b() / 65536.0f;
            mat[3] = m.c() / 65536.0f;
            mat[4] = m.d() / 65536.0f;
            mat[6] = m.tx();
            mat[7] = m.ty();
            vgMultMatrix(mat);
            // Renderer_ovg::printVGMatrix(mat);
        }
  
    ~eglScopeMatrix()
        {
            vgLoadMatrix(_orig_mat);
        }
private:
    VGfloat _orig_mat[9];
};

/// @note
/// A VGpath is constructed from a series of appended path
/// segments. When drawing shapes from flash, we start each path by
/// moving to a known location. Then segments are appended, and then
/// the path is closed. This is also used for fills.

#define MAX_SEG  (256)

/// Start a VGPath by moving to a specified location
///
/// @param path The VGPath to start
/// @returns nothing
inline void
startpath(VGPath path, const int x, const int y)
{
    VGubyte     gseg[1];
    VGfloat     gdata[2];

    gseg[0] = VG_MOVE_TO;
    gdata[0] = x;
    gdata[1] = y;
    vgAppendPathData (path, 1, gseg, gdata);
}

/// Close the VGPath started by startpath()
///
/// @param path The VGPath to close
/// @returns nothing
inline void
closepath(VGPath path)
{
    VGubyte     gseg[1];
    VGfloat     gdata[2];
  
    gseg[0] = VG_CLOSE_PATH;
    vgAppendPathData (path, 1, gseg, gdata);
}

/// Add a series of edges to the existing path created by startpath()
///
/// @param path The VGPath to append segments to
/// @param edges The segments to append to the path
/// @param anchor_x The X coordinate to start from
/// @param anchor_y The Y coordinate to start from
/// @returns nothing
inline void
preparepath(VGPath path, const std::vector<Edge>& edges,
                        const float& anchor_x, const float& anchor_y)
{
    VGubyte     gseg[MAX_SEG];
    VGfloat     gdata[MAX_SEG*3*2];
    int         scount = 0;
    int         dcount = 0; 

    point start(anchor_x, anchor_y);
    point anchor(anchor_x, anchor_y);
  
    for (std::vector<Edge>::const_iterator it = edges.begin(), end = edges.end();
         it != end; ++it) {
        const Edge& the_edge = *it;
      
        point target(the_edge.ap.x, the_edge.ap.y);

        if (the_edge.straight()) {
            gseg[scount++]  = VG_LINE_TO;
            gdata[dcount++] = target.x;
            gdata[dcount++] = target.y;
        } else {
            gseg[scount++]  = VG_QUAD_TO;
            gdata[dcount++] = the_edge.cp.x;
            gdata[dcount++] = the_edge.cp.y;
            gdata[dcount++] = target.x;
            gdata[dcount++] = target.y; 
        }
        if (scount >= MAX_SEG-2) {
            vgAppendPathData(path, scount, gseg, gdata);
            scount = 0;
            dcount = 0;
        }
        anchor = target;
    }
  
    if (scount > 0)
        vgAppendPathData (path, scount, gseg, gdata);
}

template<typename C, typename T, typename R, typename A>
void 
for_each(C& container, R (T::*pmf)(const A&),const A& arg)
{
    std::for_each(container.begin(), container.end(),
                  boost::bind(pmf, _1, boost::ref(arg)));
}

Renderer_ovg::Renderer_ovg()
    : _display_width(0.0),
      _display_height(0.0),
      _drawing_mask(false),
#ifdef OPENVG_VERSION_1_1    
      _mask_layer(VG_INVALID_HANDLE),
#endif
      _fillpaint(VG_INVALID_HANDLE),
      _strokepaint(VG_INVALID_HANDLE),
      _aspect_ratio(0.75)       // 4:3 aspect ratio
{
    // GNASH_REPORT_FUNCTION;
}

Renderer_ovg::Renderer_ovg(renderer::GnashDevice::dtype_t /* dtype */)
    : _display_width(0.0),
      _display_height(0.0),
      _drawing_mask(false),
#ifdef OPENVG_VERSION_1_1    
      _mask_layer(VG_INVALID_HANDLE),
#endif
      _fillpaint(VG_INVALID_HANDLE),
      _strokepaint(VG_INVALID_HANDLE),
      _aspect_ratio(0.75)       // 4:3 aspect ratio
{
    // GNASH_REPORT_FUNCTION;

    set_scale(1.0f, 1.0f);

#if 0
    _fillpaint = vgCreatePaint();
    
    _strokepaint = vgCreatePaint();
    
    // this paint object is used for solid, gradient, and pattern fills.
    vgSetPaint (_fillpaint,   VG_FILL_PATH);

    // this pain object is used for paths
    vgSetPaint (_strokepaint, VG_STROKE_PATH);
#endif
}

void
Renderer_ovg::init(float x, float y)
{
    // GNASH_REPORT_FUNCTION;

    _display_width = x;
    _display_height = y;
    
    // this paint object is used for solid, gradient, and pattern fills.
    _fillpaint = vgCreatePaint();
    vgSetPaint (_fillpaint, VG_FILL_PATH);

    // this pain object is used for paths
    _strokepaint = vgCreatePaint();
    vgSetPaint (_strokepaint, VG_STROKE_PATH);

    // Turn on alpha blending.
    vgSeti (VG_BLEND_MODE, VG_BLEND_SRC_OVER);
    
    vgSeti(VG_RENDERING_QUALITY, GNASH_RENDER_QUALITY);
    vgSetf(VG_STROKE_LINE_WIDTH, 20.0f);
    
    VGfloat clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    vgSetfv( VG_CLEAR_COLOR, 4, clearColor );
    
#ifdef OPENVG_VERSION_1_1    
    _mask_layer = vgCreateMaskLayer(x, y);
#endif

    log_debug("VG Vendor is %s, VG Version is %s, VG Renderer is %s",
              vgGetString(VG_VENDOR), vgGetString(VG_VERSION),
              vgGetString(VG_RENDERER));
    log_debug("VG Extensions are: ", vgGetString(VG_EXTENSIONS));
    printVGParams();
    
    // vgSeti(VG_SCISSORING, VG_FALSE);
    vgClear(0, 0, x*2, y*2);
}

Renderer_ovg::~Renderer_ovg()
{
    // GNASH_REPORT_FUNCTION;

    vgDestroyPaint(_fillpaint);
    vgDestroyPaint(_strokepaint);
#ifdef OPENVG_VERSION_1_1    
    vgDestroyMaskLayer(_mask_layer);
#endif
}

// Given an image, returns a pointer to a CachedBitmap class
// that can later be passed to fill_styleX_bitmap(), to set a
// bitmap fill style. We only cache the GnashImage here, as a
// VGImage can't be created yet until the renderer is initialized.
CachedBitmap *
Renderer_ovg::createCachedBitmap(std::auto_ptr<image::GnashImage> im)
{
    // GNASH_REPORT_FUNCTION;

    CachedBitmap *cbinfo = reinterpret_cast<CachedBitmap *>(new OpenVGBitmap(im.release(),
                                                                             _fillpaint));
    return cbinfo;
}

// Since we store drawing operations in display lists, we take special care
// to store video frame operations in their own display list, lest they be
// anti-aliased with the rest of the drawing. Since display lists cannot be
// concatenated this means we'll add up with several display lists for normal
// drawing operations.
void
Renderer_ovg::drawVideoFrame(image::GnashImage* /* frame */, const SWFMatrix* /* m */,
                             const SWFRect* /* bounds */, bool /*smooth*/)
{
    log_unimpl("drawVideoFrame");  
}

void
Renderer_ovg::world_to_pixel(int& x, int& y, float world_x, float world_y) const
{
//    GNASH_REPORT_FUNCTION;

    // negative pixels seems ok here... we don't
    // clip to valid range, use world_to_pixel(rect&)
    // and Intersect() against valid range instead.
    point p(world_x, world_y);
    stage_matrix.transform(p);
    x = (int)p.x;
    y = (int)p.y;
}

geometry::Range2d<int>
Renderer_ovg::world_to_pixel(const SWFRect& wb) const
{
//    GNASH_REPORT_FUNCTION;

    using namespace gnash::geometry;
    
    if ( wb.is_null() ) return Range2d<int>(nullRange);
    if ( wb.is_world() ) return Range2d<int>(worldRange);
    
    int xmin, ymin, xmax, ymax;

    world_to_pixel(xmin, ymin, wb.get_x_min(), wb.get_y_min());
    world_to_pixel(xmax, ymax, wb.get_x_max(), wb.get_y_max());
    
    return Range2d<int>(xmin, ymin, xmax, ymax);
}

geometry::Range2d<int>
Renderer_ovg::world_to_pixel(const geometry::Range2d<float>& wb) const
{
    // GNASH_REPORT_FUNCTION;

    if (wb.isNull() || wb.isWorld()) return wb;
    
    int xmin, ymin, xmax, ymax;
    
    world_to_pixel(xmin, ymin, wb.getMinX(), wb.getMinY());
    world_to_pixel(xmax, ymax, wb.getMaxX(), wb.getMaxY());
    
    return geometry::Range2d<int>(xmin, ymin, xmax, ymax);
}

point
Renderer_ovg::pixel_to_world(int x, int y) const
{
    // GNASH_REPORT_FUNCTION;

    point p(x, y);
    SWFMatrix mat = stage_matrix;
    mat.invert().transform(p);
    return p;
};

/// Setup the renderer to display by setting the Matrix for scaling,
/// shearing, and transformations.
///
/// @param width - stage width
/// @param height - stage height
/// @param x0 - minimum frame size in X dimension in twips
/// @param x1 - maximum frame size in X dimension in twips
/// @param y0 - minimum frame size in Y dimension in twips
/// @param y1 - maximum frame size in Y dimension in twips
void
Renderer_ovg::begin_display(gnash::rgba const&, int width, int height,
                            float x0, float x1, float y0, float y1)
{
    // GNASH_REPORT_FUNCTION;

    // Disable masking
    vgSeti (VG_MASKING, VG_FALSE);

    VGfloat mat[9];
    memset(mat, 0, sizeof(mat));
    // sx and sy define scaling in the x and y directions, respectively;
    // shx and shy define shearing in the x and y directions, respectively;
    // tx and ty define translation in the x and y directions, respectively.
    
    // Flash internally calculates anything that uses pixels with
    // twips (or 1/20 of a pixel). Sprites, movie clips and any other
    // object on the stage are positioned with twips. As a result, the
    // coordinates of (for example) sprites are always multiples of
    // 0.05 (i.e. 1/20).
    mat[0] = (VGfloat)width / VGfloat(x1 - x0);  // scale sx = 0.05
    mat[1] = 0; // shx
    mat[3] = 0; // shy
    mat[4] = -((VGfloat)height / VGfloat(y1 - y0)); // scale sy = -0.05
    mat[6] = 0;   // shift tx in pixels
    mat[7] = height;   // shift ty in pixels
    
    vgSeti (VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
    // The default values after vgLoadIdentity() are:
    //          [ 1 0 0 ]
    //       M =| 0 1 0 |
    //          [ 0 0 1 ]
    vgLoadIdentity();

    // An affine transformation maps a point (x, y) (represented using
    // homogeneous coordinates as the column vector [x, y, 1]T) into the
    // point (x*sx + y*shx + tx, x*shy + y*sy + ty) using matrix multiplication:
    // [ sx shx tx ] [ x ]   [ x∗sx + y∗shx + tx ]
    // | shy sy ty |.[ y | = | x∗shy + y∗sy + ty |
    // [   0  0  1 ] [ 1 ]   [            1      ]
    //
    // If not VG_MATRIX_IMAGE_USER_TO_SURFACE, w0, w1, and w2 are ignored.
    vgLoadMatrix (mat);
    
    // vgSeti(VG_SCISSORING, VG_FALSE);
    vgClear(0, 0, _display_width, _display_height);
}

void
Renderer_ovg::end_display()
{
    // GNASH_REPORT_FUNCTION;
}

/// Draw a line-strip directly, using a thin, solid line. 
//
/// Can be used to draw empty boxes and cursors.
void
Renderer_ovg::drawLine(const std::vector<point>& coords, const rgba& fill,
                       const SWFMatrix& mat)
{
    // GNASH_REPORT_FUNCTION;
    
    VGubyte     gseg[MAX_SEG];
    VGfloat     gdata[MAX_SEG*3*2];
    int         scount = 0;
    int         dcount = 0;

    if (coords.empty()) return;
    
    eglScopeMatrix scope_mat(mat);
    
    VGfloat color[] = {
        fill.m_r / 255.0f,
        fill.m_g / 255.0f,
        fill.m_b / 255.0f,
        1.0f
    };
    VGPath      vg_path;
    vg_path = vgCreatePath (VG_PATH_FORMAT_STANDARD,
                            VG_PATH_DATATYPE_F,
                            1, 0, 0, 0,
                            VG_PATH_CAPABILITY_ALL);
    vgSetf (VG_FILL_RULE, VG_EVEN_ODD );
    vgSetParameteri (_strokepaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
    vgSetParameterfv (_strokepaint, VG_PAINT_COLOR, 4, color);
    
    std::vector<point>::const_iterator  it = coords.begin();
    std::vector<point>::const_iterator end = coords.end();
    
    gseg[scount++] = VG_MOVE_TO;
    gdata[dcount++] = (float)(*it).x;
    gdata[dcount++] = (float)(*it).y;
    ++it;            
    
    for (; it != end; ++it) {
        gseg[scount++] = VG_LINE_TO;
        gdata[dcount++] = (float)(*it).x;
        gdata[dcount++] = (float)(*it).y;
        if (scount >= MAX_SEG-1) {
            vgAppendPathData(vg_path, scount, gseg, gdata);
            scount = 0;
            dcount = 0;
        }
    }
    
    if (scount > 0) {
        vgAppendPathData(vg_path, scount, gseg, gdata);
        
    }
    
    vgSetf (VG_STROKE_JOIN_STYLE, VG_JOIN_ROUND);
    vgSetf (VG_STROKE_CAP_STYLE, VG_CAP_ROUND);
    vgSetf (VG_STROKE_LINE_WIDTH, 20.0f);
    
    vgDrawPath (vg_path, VG_STROKE_PATH);
    vgDestroyPath(vg_path);
}

void
Renderer_ovg::draw_poly(const std::vector<point>& corners,
                        const rgba& fill, const rgba& /* outline */,
                        const SWFMatrix& mat, bool /* masked */)
{
    // GNASH_REPORT_FUNCTION;

    VGubyte     gseg[MAX_SEG];
    VGfloat     gdata[MAX_SEG*3*2];
    int         scount = 0;
    int         dcount = 0;

    if (corners.empty()) {
        return;
    }
    
    eglScopeMatrix scope_mat(mat);
    
    VGfloat color[] = {
        fill.m_r / 255.0f,
        fill.m_g / 255.0f,
        fill.m_b / 255.0f,
        fill.m_a / 255.0f
    };
    VGPath      vg_path;
    vg_path = vgCreatePath (VG_PATH_FORMAT_STANDARD,
                            VG_PATH_DATATYPE_F,
                            1, 0, 0, 0,
                            VG_PATH_CAPABILITY_ALL);
    vgSetf (VG_FILL_RULE, VG_NON_ZERO );
    
    vgSetParameteri (_fillpaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
    vgSetParameterfv (_fillpaint, VG_PAINT_COLOR, 4, color);
    
    const point *ptr = &corners.front();
    gseg[scount++] = VG_MOVE_TO;
    gdata[dcount++] = (float)ptr->x;
    gdata[dcount++] = (float)ptr->y;
    ptr++;
    
    for (size_t i = 1; i < corners.size(); i++) {
        gseg[scount++] = VG_LINE_TO;
        gdata[dcount++] = (float)ptr->x;
        gdata[dcount++] = (float)ptr->y;
        ptr++;
        if (scount >= MAX_SEG-2) {
            vgAppendPathData(vg_path, scount, gseg, gdata);
            scount = 0;
            dcount = 0;
        }
    }
    gseg[scount++] = VG_CLOSE_PATH;
    vgAppendPathData (vg_path, scount, gseg, gdata);
    
    vgDrawPath (vg_path, VG_FILL_PATH);
    vgDestroyPath(vg_path);
}

void
Renderer_ovg::set_antialiased(bool /* enable */)
{
    log_unimpl("set_antialiased");
}

void
Renderer_ovg::begin_submit_mask()
{
    // GNASH_REPORT_FUNCTION;

    PathVec mask;
    _masks.push_back(mask);
    _drawing_mask = true;
}

void
Renderer_ovg::end_submit_mask()
{
    // GNASH_REPORT_FUNCTION;

    // If masking is disabled, rhen we can't use it
    if (_drawing_mask == true) {
        _drawing_mask = false;    
        apply_mask();
    }
}

/// Apply the current mask; nesting is supported.
///
/// This method marks the stencil buffer by incrementing every stencil pixel
/// by one every time a solid from one of the current masks is drawn. When
/// all the mask solids are drawn, we change the stencil operation to permit
/// only drawing where all masks have drawn, in other words, where all masks
/// intersect, or in even other words, where the stencil pixel buffer equals
/// the number of masks.
void
Renderer_ovg::apply_mask()
{  
    // GNASH_REPORT_FUNCTION;

    if (_masks.empty()) {
        return;
    }
    
    float mat[9];
    float omat[9];
    
    vgSeti (VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
    vgLoadIdentity();
    vgGetMatrix (omat);         // get the current matrix
    
    memset(mat, 0, sizeof(mat));
    mat[0] =  stage_matrix.get_x_scale(); // scale sx
    mat[1] =  0.0f; // shx
    mat[3] =  0.0f; // shy
    mat[4] =  -stage_matrix.get_x_scale(); // scale sy
    mat[6] =  0;    // shift tx
    mat[7] =  _display_height;   // shift ty
    vgLoadMatrix(mat);
    // Renderer_ovg::printVGMatrix(mat);
    
#ifdef OPENVG_VERSION_1_1
    vgMask(_mask_layer, VG_FILL_MASK, 0, 0, _display_width, _display_height);
#endif
// Call add_paths for each mask.
    std::for_each(_masks.begin(), _masks.end(),
                  boost::bind(&Renderer_ovg::add_paths, this, _1));
    vgSeti(VG_MASKING, VG_TRUE);
    
    vgLoadMatrix (omat);        // restore the current matrix
}

void
Renderer_ovg::disable_mask()
{
    // GNASH_REPORT_FUNCTION;
    
    // if (vgGeti(VG_MASKING) == VG_TRUE) {
        _masks.pop_back();
        
        if (_masks.empty()) {
            vgSeti (VG_MASKING, VG_FALSE);
        } else {
            apply_mask();
        }
    // }
}

void
Renderer_ovg::add_paths(const PathVec& path_vec)
{
    // GNASH_REPORT_FUNCTION;

    SWFCxForm dummy_cx;
    
    FillStyle coloring = FillStyle(SolidFill(rgba(0, 255, 0, 255)));

    draw_submask(path_vec, SWFMatrix(), dummy_cx, coloring);
}

Path
Renderer_ovg::reverse_path(const Path& cur_path)
{
    // GNASH_REPORT_FUNCTION;

    const Edge& cur_end = cur_path.m_edges.back();    
    
    float prev_cx = cur_end.cp.x;
    float prev_cy = cur_end.cp.y;        
    
    Path newpath(cur_end.ap.x, cur_end.ap.y, cur_path.m_fill1, cur_path.m_fill0, cur_path.m_line, cur_path.m_new_shape);
    
    float prev_ax = cur_end.ap.x;
    float prev_ay = cur_end.ap.y; 
    
    for (std::vector<Edge>::const_reverse_iterator it = cur_path.m_edges.rbegin()+1, end = cur_path.m_edges.rend(); it != end; ++it) {
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

const Path *
Renderer_ovg::find_connecting_path(const Path& to_connect,
                                   std::list<const Path*> path_refs)
{        
    // GNASH_REPORT_FUNCTION;

    float target_x = to_connect.m_edges.back().ap.x;
    float target_y = to_connect.m_edges.back().ap.y;
    
    if (target_x == to_connect.ap.x &&
        target_y == to_connect.ap.y) {
        return NULL;
    }
    
    for (std::list<const Path*>::const_iterator it = path_refs.begin(),
             end = path_refs.end(); it != end; ++it) {
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

PathVec
Renderer_ovg::normalize_paths(const PathVec &paths)
{
    // GNASH_REPORT_FUNCTION;

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
void
Renderer_ovg::analyze_paths(const PathVec &paths, bool& have_shape,
                            bool& have_outline) 
{
    // GNASH_REPORT_FUNCTION;

    have_shape = false;
    have_outline = false;
    
    int pcount = paths.size();
    
    for (int pno= 0; pno<pcount; pno++) {
        
        const Path &the_path = paths[pno];
        
        // If a left or right fill is set, then this is an outline
        if ((the_path.m_fill0 > 0) || (the_path.m_fill1 > 0)) {
            have_shape = true;
            if (have_outline) return; // have both
        }
        
        // If a line is set, then it's a shape. A path can be both
        if (the_path.m_line > 0) {
            have_outline = true;
            if (have_shape) return; // have both
        }
    }    
}

bool
Renderer_ovg::apply_line_style(const LineStyle& style, const SWFCxForm& cx, 
                               const SWFMatrix& mat)
{
    // GNASH_REPORT_FUNCTION;
    
    bool rv = true;
    
    switch(style.joinStyle()) {
      case JOIN_ROUND:
          vgSetf (VG_STROKE_JOIN_STYLE, VG_JOIN_ROUND);
          break;
      case JOIN_BEVEL:
          vgSetf (VG_STROKE_JOIN_STYLE, VG_JOIN_BEVEL);
          break;
      case JOIN_MITER:
          vgSetf (VG_STROKE_JOIN_STYLE, VG_JOIN_MITER);
          break;
      default:
          log_unimpl("join style");
    }
    
    switch(style.startCapStyle()) {
      case CAP_ROUND:
          vgSetf (VG_STROKE_CAP_STYLE, VG_CAP_ROUND);
          break;
      case CAP_NONE:
          vgSetf (VG_STROKE_CAP_STYLE, VG_CAP_BUTT);
          break;
      case CAP_SQUARE:
          vgSetf (VG_STROKE_CAP_STYLE, VG_CAP_SQUARE);
          break;
      default:
          log_unimpl("cap style");
    }
    
    vgSetf (VG_STROKE_MITER_LIMIT, style.miterLimitFactor());
    
    float width = style.getThickness();
    
    if (!width) {
        vgSetf(VG_STROKE_LINE_WIDTH, 20.0f);
        rv = false; // Don't draw rounded lines.
    } else if ( (!style.scaleThicknessVertically()) && (!style.scaleThicknessHorizontally()) ) {
        vgSetf(VG_STROKE_LINE_WIDTH, width);
    } else {
        if ( (!style.scaleThicknessVertically()) || (!style.scaleThicknessHorizontally()) ) {
            LOG_ONCE( log_unimpl(_("Unidirectionally scaled strokes in OGL renderer")) );
        }
        
        float stroke_scale = fabsf(mat.get_x_scale()) + fabsf(mat.get_y_scale());
        stroke_scale /= 2.0f;
        stroke_scale *= (stage_matrix.get_x_scale() + stage_matrix.get_y_scale()) / 2.0f;
        width *= stroke_scale;
        
        if (width < 20.0f) {
            width = 20.0f;
        }
        
        vgSetf(VG_STROKE_LINE_WIDTH, width);
    }
    
    rgba c = cx.transform(style.get_color());
    VGfloat color[] = {
        c.m_r / 255.0f,
        c.m_g / 255.0f,
        c.m_b / 255.0f,
        c.m_a / 255.0f
    };
    
    
    vgSetParameteri (_strokepaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
    vgSetParameterfv (_strokepaint, VG_PAINT_COLOR, 4, color);
    
    return rv;
}

typedef std::vector<const Path*> PathPtrVec;
  
void
Renderer_ovg::draw_outlines(const PathVec& path_vec, const SWFMatrix& mat,
                            const SWFCxForm& cx, const std::vector<LineStyle>& line_styles)
{
    // GNASH_REPORT_FUNCTION;

    for (PathVec::const_iterator it = path_vec.begin(), end = path_vec.end();
         it != end; ++it) {
        
        const Path& cur_path = *it;
        
        if (!cur_path.m_line) {
            continue;
        }
        
        VGPath      vpath;
        vpath = vgCreatePath (VG_PATH_FORMAT_STANDARD,
                              VG_PATH_DATATYPE_F,
                              1, 0, 0, 0,
                              VG_PATH_CAPABILITY_ALL);
        vgSetf (VG_FILL_RULE, VG_EVEN_ODD );
        startpath(vpath, cur_path.ap.x, cur_path.ap.y);
        preparepath(vpath, cur_path.m_edges, cur_path.ap.x,
                    cur_path.ap.y);
        
        apply_line_style(line_styles[cur_path.m_line-1], cx, mat);
        vgDrawPath (vpath, VG_STROKE_PATH);
        vgDestroyPath(vpath);
    }
}

std::list<PathPtrVec>
Renderer_ovg::get_contours(const PathPtrVec &paths)
{
    // GNASH_REPORT_FUNCTION;

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

void
Renderer_ovg::draw_mask(const PathVec& path_vec)
{ 
    // GNASH_REPORT_FUNCTION;
   
    if (_drawing_mask == true) {
        for (PathVec::const_iterator it = path_vec.begin(), end = path_vec.end();
             it != end; ++it) {
            const Path& cur_path = *it;
            
            if (cur_path.m_fill0 || cur_path.m_fill1) {
                _masks.back().push_back(cur_path);
                _masks.back().back().m_line = 0;    
            }
        }  
    }
}

PathPtrVec
Renderer_ovg::paths_by_style(const PathVec& path_vec, unsigned int style)
{
    // GNASH_REPORT_FUNCTION;

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
Renderer_ovg::find_subshapes(const PathVec& path_vec)
{
    // GNASH_REPORT_FUNCTION;

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
Renderer_ovg::apply_matrix_to_paths(std::vector<Path>& paths, const SWFMatrix& mat)
{  
    // GNASH_REPORT_FUNCTION;

    std::for_each(paths.begin(), paths.end(),
                  boost::bind(&Path::transform, _1, boost::ref(mat)));
}  

void
Renderer_ovg::draw_subshape(const PathVec& path_vec,
                            const SWFMatrix& mat,
                            const SWFCxForm& cx,
                            const std::vector<FillStyle>& fill_styles,
                            const std::vector<LineStyle>& line_styles)
{
    // GNASH_REPORT_FUNCTION;

    PathVec normalized = normalize_paths(path_vec);
    
    for (size_t i = 0; i < fill_styles.size(); ++i) {
        PathPtrVec paths = paths_by_style(normalized, i+1);
        
        if (!paths.size()) {
            continue;
        }
        
        std::list<PathPtrVec> contours = get_contours(paths);
        
        VGPath      vg_path;
        vg_path = vgCreatePath (VG_PATH_FORMAT_STANDARD,
                                VG_PATH_DATATYPE_F,
                                1, 0, 0, 0,
                                VG_PATH_CAPABILITY_ALL);              
        vgSetf (VG_FILL_RULE, VG_EVEN_ODD );                                  
        
        for (std::list<PathPtrVec>::const_iterator iter = contours.begin(),
                 final = contours.end(); iter != final; ++iter) {
            const PathPtrVec& refs = *iter;
            
            startpath(vg_path, (*(refs[0])).ap.x, (*(refs[0])).ap.y);

            // Create a Linear or Radial gradient
            // All positions are specified in twips, which are 20 to the
            // pixel.
            const StyleHandler st(cx, _fillpaint,
                                  (*(refs[0])).ap.x/20, (*(refs[0])).ap.y/20);
            boost::apply_visitor(st, fill_styles[i].fill);

            for (PathPtrVec::const_iterator it = refs.begin(), end = refs.end();
                 it != end; ++it) {
                const Path& cur_path = *(*it);
                if (!cur_path.m_fill0 && !cur_path.m_fill1)
                    continue;
                preparepath(vg_path, cur_path.m_edges, cur_path.ap.x,
                            cur_path.ap.y);
            }
            closepath(vg_path);
        }
        vgDrawPath (vg_path, VG_FILL_PATH);
        vgDestroyPath(vg_path);
    }
    
    draw_outlines(normalized, mat, cx, line_styles);
}

void
Renderer_ovg::draw_submask(const PathVec& path_vec,
                           const SWFMatrix& /* mat */,
                           const SWFCxForm& /* cx */,
                            const FillStyle& /* f_style */)
{
    // GNASH_REPORT_FUNCTION;
    
    PathVec normalized = normalize_paths(path_vec);
    
    PathPtrVec paths = paths_by_style(normalized, 1);
    
    std::list<PathPtrVec> contours = get_contours(paths);
    
    VGfloat color[] = {1.0f, 1.0f, 1.0f, 1.0f};
    
    vgSetParameteri (_fillpaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
    vgSetParameterfv (_fillpaint, VG_PAINT_COLOR, 4, color);
    
    
    VGPath      vg_path;
    vg_path = vgCreatePath (VG_PATH_FORMAT_STANDARD,
                            VG_PATH_DATATYPE_F,
                            1, 0, 0, 0,
                            VG_PATH_CAPABILITY_ALL);
    vgSetf (VG_FILL_RULE, VG_EVEN_ODD );
    
    for (std::list<PathPtrVec>::const_iterator iter = contours.begin(),
             final = contours.end(); iter != final; ++iter) {
        const PathPtrVec& refs = *iter;
        startpath(vg_path, (*(refs[0])).ap.x, (*(refs[0])).ap.y);
        for (PathPtrVec::const_iterator it = refs.begin(), end = refs.end();
             it != end; ++it) {
            const Path& cur_path = *(*it);
            preparepath(vg_path, cur_path.m_edges, cur_path.ap.x,
                        cur_path.ap.y);
        }
        closepath(vg_path);
    }
    
#ifdef OPENVG_VERSION_1_1
    vgRenderToMask(vg_path, VG_FILL_PATH, VG_INTERSECT_MASK);
#endif
    vgDestroyPath(vg_path);
}

// Drawing procedure:
// 1. Separate paths by subshape.
// 2. Separate subshapes by fill style.
// 3. For every subshape/fill style combo:
//  a. Separate contours: find closed shapes by connecting ends.
//  b. Apply fill style.
//  c. Feed the contours in the tesselator. (Render.)
//  d. Draw outlines for every path in the subshape with a line style.
void
Renderer_ovg::drawShape(gnash::SWF::ShapeRecord const &shape, 
                        gnash::Transform const& xform)
{
    // GNASH_REPORT_FUNCTION;

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
        return;
    }    
    
    eglScopeMatrix scope_mat(xform.matrix);
    
    std::vector<PathVec::const_iterator> subshapes = find_subshapes(path_vec);
    
    const std::vector<FillStyle>& fill_styles = shape.fillStyles();
    const std::vector<LineStyle>& line_styles = shape.lineStyles();
    
    for (size_t i = 0; i < subshapes.size()-1; ++i) {
        PathVec subshape_paths;
        
        if (subshapes[i] != subshapes[i+1]) {
            subshape_paths = PathVec(subshapes[i], subshapes[i+1]);
        } else {
            subshape_paths.push_back(*subshapes[i]);
        }
        
        draw_subshape(subshape_paths, xform.matrix, xform.colorTransform,
                      fill_styles, line_styles);
    }
}

void
Renderer_ovg::drawGlyph(const SWF::ShapeRecord& rec, const rgba& c,
                        const SWFMatrix& mat)
{
    // GNASH_REPORT_FUNCTION;

    if (_drawing_mask) {
        abort();
    }
    
    if (rec.getBounds().is_null()) {
        return;
    }
    
    SWFCxForm dummy_cx;
    std::vector<FillStyle> glyph_fs;
    
    FillStyle coloring = FillStyle(SolidFill(c));

    glyph_fs.push_back(coloring);

    std::vector<LineStyle> dummy_ls;
    
    eglScopeMatrix scope_mat(mat);
    
    draw_subshape(rec.paths(), mat, dummy_cx, glyph_fs, dummy_ls);
    
}

void
Renderer_ovg::set_scale(float xscale, float yscale)
{
    // GNASH_REPORT_FUNCTION;

    _xscale = xscale;
    _yscale = yscale;
    stage_matrix.set_identity();
    stage_matrix.set_scale(xscale/20.0f, yscale/20.0f);
}

void
Renderer_ovg::set_invalidated_regions(const InvalidatedRanges& /* ranges */)
{
    // GNASH_REPORT_FUNCTION;

    // do nothing obviously. This method is required by the base class though,
    // so something has to be here.
}
  
DSOEXPORT Renderer *
create_handler(const char */* pixelformat */)
{
    // GNASH_REPORT_FUNCTION;

    Renderer_ovg *renderer = new Renderer_ovg;
    return renderer;
}


// These methods are only for debugging and development
void
Renderer_ovg::printVGParams()
{
    // vgGetVectorSize();

    std::string str;
    switch(vgGeti(VG_MATRIX_MODE)) {
      case VG_MATRIX_PATH_USER_TO_SURFACE:
          str = "VG_MATRIX_PATH_USER_TO_SURFACE";
          break;
      case VG_MATRIX_IMAGE_USER_TO_SURFACE:
          str = "VG_MATRIX_IMAGE_USER_TO_SURFACE";
          break;
      case VG_MATRIX_FILL_PAINT_TO_USER:
          str = "VG_MATRIX_FILL_PAINT_TO_USER";
          break;
      case VG_MATRIX_STROKE_PAINT_TO_USER:
          str = "VG_MATRIX_STROKE_PAINT_TO_USER";
          break;
#ifdef VG_MATRIX_MODE_FORCE_SIZE
      case VG_MATRIX_MODE_FORCE_SIZE:
          str = "VG_MATRIX_MODE_FORCE_SIZE";
          break;
#endif
      default:
          log_error("unsupported VG_MATRIX_MODE!");
    }
    log_debug("VG_MATRIX_MODE is %s", str);
    str.clear();

    switch(vgGeti(VG_FILL_RULE)) {
      case VG_EVEN_ODD:
          str = "VG_EVEN_ODD";
          break;
      case VG_NON_ZERO:
          str = "VG_NON_ZERO";
          break;
      default:
          log_error("unsupported VG_FILL_RULE!");          
    }
    log_debug("VG_FILL_RULE is %s", str);
    str.clear();

    switch(vgGeti(VG_IMAGE_QUALITY)) {
      case VG_IMAGE_QUALITY_NONANTIALIASED:
          str = "VG_IMAGE_QUALITY_NONANTIALIASED";
          break;
      case VG_IMAGE_QUALITY_FASTER:
          str = "VG_IMAGE_QUALITY_FASTER";
          break;
      case VG_IMAGE_QUALITY_BETTER:
          str = "VG_IMAGE_QUALITY_BETTER";
          break;
#ifdef VG_MATRIX_MODE_FORCE_SIZE
      case VG_IMAGE_QUALITY_FORCE_SIZE:
          str = "VG_IMAGE_QUALITY_FORCE_SIZE";
          break;
#endif
      default:
          log_error("unsupported VG_IMAGE_QUALITY!");
    }
    log_debug("VG_IMAGE_QUALITY is %s", str);
    str.clear();
    
    switch(vgGeti(VG_RENDERING_QUALITY)) {
      case VG_RENDERING_QUALITY_NONANTIALIASED:
          str = "VG_RENDERING_QUALITY_NONANTIALIASED";
          break;
      case VG_RENDERING_QUALITY_FASTER:
          str = "VG_RENDERING_QUALITY_FASTER";
          break;
      case VG_RENDERING_QUALITY_BETTER:
          str = "VG_RENDERING_QUALITY_BETTER";
          break;
#ifdef VG_MATRIX_MODE_FORCE_SIZE
      case VG_RENDERING_QUALITY_FORCE_SIZE:
          str = "VG_RENDERING_QUALITY_FORCE_SIZE";
          break;
#endif
      default:
          log_error("unsupported VG_RENDERING_QUALITY!");
    }
    log_debug("VG_RENDERING_QUALITY is %s", str);
    str.clear();
    
    switch(vgGeti(VG_BLEND_MODE)) {
      case VG_BLEND_SRC:
          str = "VG_BLEND_SRC";
          break;
      case VG_BLEND_SRC_OVER:
          str = "VG_BLEND_SRC_OVER";
          break;
      case VG_BLEND_DST_OVER:
          str = "VG_BLEND_DST_OVER";
          break;
      case VG_BLEND_SRC_IN:
          str = "VG_BLEND_SRC_IN";
          break;
      case VG_BLEND_DST_IN:
          str = "VG_BLEND_DST_IN";
          break;
      case VG_BLEND_MULTIPLY: 
          str = "VG_BLEND_MULTIPLY";
          break;
      case VG_BLEND_SCREEN:
          str = "VG_BLEND_SCREEN";
          break;
      case VG_BLEND_DARKEN:
          str = "VG_BLEND_DARKEN";
          break;
      case VG_BLEND_LIGHTEN:
          str = "VG_BLEND_LIGHTEN";
          break;
      case VG_BLEND_ADDITIVE:
          str = "VG_BLEND_ADDITIVE";
          break;
      default:
          log_error("unsupported VG_BLEND_MODE!");
    }
    log_debug("VG_BLEND_MODE is %s", str);    
    str.clear();
    
    switch(vgGeti(VG_IMAGE_MODE)) {
      case VG_DRAW_IMAGE_NORMAL:
          str = "VG_DRAW_IMAGE_MULTIPLY";
          break;
      case VG_DRAW_IMAGE_MULTIPLY:
          str = "VG_DRAW_IMAGE_MULTIPLY";
          break;
      case VG_DRAW_IMAGE_STENCIL:
          str = "VG_DRAW_IMAGE_STENCIL";
          break;
#ifdef VG_MATRIX_MODE_FORCE_SIZE
      case VG_IMAGE_MODE_FORCE_SIZE:
          str = "VG_IMAGE_MODE_FORCE_SIZE";
          break;
#endif
      default:
          log_error("unsupported VG_IMAGE_MODE!");
    }
    log_debug("VG_IMAGE_MODE is %s", str);
    str.clear();
    
    log_debug("VG_STROKE_LINE_WIDTH is %d", vgGeti(VG_STROKE_LINE_WIDTH));    
    str.clear();
    
    switch(vgGeti(VG_STROKE_CAP_STYLE)) {
      case VG_CAP_BUTT:
          str = "VG_CAP_BUTT";
          break;
      case VG_CAP_ROUND:
          str = "VG_CAP_ROUND";
          break;
      case VG_CAP_SQUARE:
          str = "VG_CAP_SQUARE";
          break;
#ifdef VG_MATRIX_MODE_FORCE_SIZE
      case VG_CAP_STYLE_FORCE_SIZE:
          str = "VG_CAP_STYLE_FORCE_SIZE";
          break;
#endif
      default:
          log_error("unsupported VG_STROKE_CAP_STYLE!");
    }
    log_debug("VG_STROKE_CAP_STYLE is %s", str);
    str.clear();
    
    switch(vgGeti(VG_STROKE_JOIN_STYLE)) {
      case VG_JOIN_MITER:
          str = "VG_JOIN_MITER";
          break;
      case VG_JOIN_ROUND:
          str = "VG_JOIN_ROUND";
          break;
      case VG_JOIN_BEVEL:
          str = "VG_JOIN_BEVEL";
          break;
#ifdef VG_MATRIX_MODE_FORCE_SIZE
      case VG_JOIN_STYLE_FORCE_SIZE:
          str = "VG_JOIN_STYLE_FORCE_SIZE";
          break;
#endif
      default:
          log_error("unsupported VG_STROKE_JOIN_STYLE!");
    }
    log_debug("VG_STROKE_JOIN_STYLE is %s", str);
    str.clear();
    
    log_debug("VG_STROKE_MITER_LIMIT is %d", vgGeti(VG_STROKE_MITER_LIMIT));
    log_debug("VG_MASKING is %d", vgGeti(VG_MASKING));
    log_debug("VG_SCISSORING is %d", vgGeti(VG_SCISSORING));    
    str.clear();
    
    switch(vgGeti(VG_PIXEL_LAYOUT)) {
      case VG_PIXEL_LAYOUT_UNKNOWN:
          str = "VG_PIXEL_LAYOUT_UNKNOWN";
          break;
      case VG_PIXEL_LAYOUT_RGB_VERTICAL:
          str = "VG_PIXEL_LAYOUT_RGB_VERTICAL";
          break;
      case VG_PIXEL_LAYOUT_BGR_VERTICAL:
          str = "VG_PIXEL_LAYOUT_BGR_VERTICAL";
          break;
      case VG_PIXEL_LAYOUT_RGB_HORIZONTAL:
          str = "VG_PIXEL_LAYOUT_RGB_HORIZONTAL";
          break;
      case VG_PIXEL_LAYOUT_BGR_HORIZONTAL:
          str = "VG_PIXEL_LAYOUT_BGR_HORIZONTAL";
          break;
#ifdef VG_MATRIX_MODE_FORCE_SIZE
      case VG_PIXEL_LAYOUT_FORCE_SIZE:
          str = "VG_PIXEL_LAYOUT_FORCE_SIZE";
          break;
#endif
      default:
          log_error("unsupported VG_PIXEL_LAYOUT!");
    }
    log_debug("VG_PIXEL_LAYOUT is %s", str);
    
    log_debug("VG_STROKE_DASH_PHASE_RESET is %s",
              (vgGeti(VG_STROKE_DASH_PHASE_RESET) == true) ? "true" : "false");
    log_debug("VG_FILTER_FORMAT_LINEAR is %s",
              (vgGeti(VG_FILTER_FORMAT_LINEAR) == true) ? "true" : "false");
    log_debug("VG_FILTER_FORMAT_PREMULTIPLIED is %s",
              (vgGeti(VG_FILTER_FORMAT_PREMULTIPLIED) == true) ? "true" : "false");
    str.clear();
    
    VGint value = vgGeti(VG_FILTER_CHANNEL_MASK);
    if (value & VG_RED) {
        str += " VG_RED";
    }
    if (value & VG_GREEN) {
        str += " VG_GREEN";
    }
    if (value & VG_BLUE) {
        str += " VG_BLUE";
    }
    if (value & VG_ALPHA) {
        str += " VG_ALPHA";
    }
    log_debug("VG_FILTER_CHANNEL_MASK is %s", str);
    
    log_debug("VG_MAX_IMAGE_WIDTH is %d", vgGeti(VG_MAX_IMAGE_WIDTH));
    log_debug("VG_MAX_IMAGE_HEIGHT is %d", vgGeti(VG_MAX_IMAGE_HEIGHT));
    log_debug("VG_MAX_IMAGE_PIXELS is %d", vgGeti(VG_MAX_IMAGE_PIXELS));
    log_debug("VG_MAX_IMAGE_BYTES is %d", vgGeti(VG_MAX_IMAGE_BYTES));

}

void
Renderer_ovg::printVGPath(VGPath path)
{
    log_debug("VG_PATH_FORMAT is %d", vgGetParameteri(path, VG_PATH_FORMAT));
    log_debug("VG_PATH_DATATYPE is %d", vgGetParameteri(path, VG_PATH_DATATYPE));
    log_debug("VG_PATH_CAPABILITY_APPEND_TO is %d", vgGetParameteri(path, VG_PATH_CAPABILITY_APPEND_TO));
    log_debug("VG_PATH_SCALE is %g", vgGetParameteri(path, VG_PATH_SCALE));
    log_debug("VG_PATH_BIA is %g", vgGetParameteri(path, VG_PATH_BIAS));

    log_debug("VG_PATH_NUM_SEGMENTS is %d", vgGetParameteri(path, VG_PATH_NUM_SEGMENTS));
    log_debug("VG_PATH_NUM_COORDS is %d", vgGetParameteri(path, VG_PATH_NUM_COORDS));
}

// Print an OpenVG matric, which is 3 x 3. Elements 2 and 5 are
// ignored, as they are the w0 and w1 paramaters.
// It looks like this: { sx, shy, w0, shx, sy, w1, tx, ty, w2 }
void
Renderer_ovg::printVGMatrix(VGfloat *mat)
{
    std::cerr << "sx, shx, tx: " << mat[0] << ", " << mat[1]<< ", "
              << std::fixed << mat[3] << std::endl;
    std::cerr << "sy, shy, ty: " << mat[4]<< ", " << mat[6] << ", "
              << std::scientific << mat[7] << std::endl;
}

void
Renderer_ovg::printVGMatrix(const SWFMatrix &mat)
{
    std::cerr << "a, shx, tx: " << mat.a() << ", " << mat.b() << ", " << mat.tx() << std::endl;
    std::cerr << "sy, shy, ty: " << mat.c() << ", " << mat.d() << ", " << mat.ty() << std::endl;
}

Renderer *
Renderer_ovg::startInternalRender(gnash::image::GnashImage&)
{
    // GNASH_REPORT_FUNCTION;

    return 0;
}

void
Renderer_ovg::endInternalRender()
{
    // GNASH_REPORT_FUNCTION;    
}

void
Renderer_ovg::drawVideoFrame(gnash::image::GnashImage*, gnash::Transform const&, gnash::SWFRect const*, bool)
{
    GNASH_REPORT_FUNCTION;
}

unsigned int
Renderer_ovg::getBitsPerPixel()
{
    return 0;
}

const char *
Renderer_ovg::getErrorString(VGErrorCode error)
{
    switch (error) {
    case VG_NO_ERROR:
        return "No Error";
        break;
    case VG_BAD_HANDLE_ERROR:
        return "Bad Handle";
        break;
    case VG_ILLEGAL_ARGUMENT_ERROR:
        return "Illegal Argument";
        break;
    case VG_OUT_OF_MEMORY_ERROR:
        return "Our Of Memory";
        break;
    case VG_PATH_CAPABILITY_ERROR:
        return "Path Capability";
        break;
    case VG_UNSUPPORTED_IMAGE_FORMAT_ERROR:
        return "Unsupported Image Format";
        break;
    case VG_UNSUPPORTED_PATH_FORMAT_ERROR:
        return "Unsupported Path Format";
        break;
    case VG_IMAGE_IN_USE_ERROR: 
        return "VG Image In Use";
       break;
    case VG_NO_CONTEXT_ERROR:
        return "No COntext";
        break;
#ifdef VG_ERROR_CODE_FORCE_SIZE
    case VG_ERROR_CODE_FORCE_SIZE:
        return "Code Force Size";
        break;
#endif
    default:
        return "Unknown error";
        break;
    }
}

} // namespace gnash::renderer::gles1
} // namespace gnash::renderer
} // namespace gnash

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
