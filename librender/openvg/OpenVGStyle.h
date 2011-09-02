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

#ifndef GNASH_OPENVG_STYLE_H
#define GNASH_OPENVG_STYLE_H

#include "CachedBitmap.h"
#include "GnashImage.h"
#include "Renderer.h"
#include "FillStyle.h"
#include "SWFCxForm.h"
#include "SWFMatrix.h"
#include "openvg/OpenVGBitmap.h"

namespace gnash {

// Forward declarations.
class SolidFill;
class GradientFill;
class BitmapFill;
class rgba; 
class StyleHandler;

namespace renderer {

namespace openvg {

/// @note These helper functions are used by the boost::variant used
/// for fill styles. A variant is a C++ style version of the C union.
/// Before accessing any of the data of the variant, we have to use
/// boost::apply_visitor() to bind one of these classes to the style
/// to extract the data.

/// Transfer FillStyles to OpenVG styles.
struct StyleHandler : boost::static_visitor<>
{
    StyleHandler(const SWFCxForm& cx,
                 const VGPaint &p, float x, float y)
        : _cxform(cx),
          _vgpaint(p),
          _x(x),
          _y(y)
        {
            // GNASH_REPORT_FUNCTION;
        }
                   
    void operator()(const GradientFill& g) const {
        GNASH_REPORT_FUNCTION;
        SWFMatrix mat = g.matrix();
        Renderer_ovg::printVGMatrix(mat);
        //      from OpenVG specification PDF
        //
        //          dx(x - x0) + dy((y - y0)
        // g(x,y) = ------------------------
        //                dx^2 + dy^2
        // where dx = x1 - x0, dy = y1 - y0
        //
        int width = 800;
        int height = 480;
        const GradientFill::Type fill_type = g.type();
        OpenVGBitmap* binfo = new OpenVGBitmap(_vgpaint);
        if (fill_type ==  GradientFill::LINEAR) {
            const std::vector<gnash::GradientRecord> &records = g.getRecords();
            log_debug("Fill Style Type: Linear Gradient, %d records", records.size());
            // Use the display size for the extent of the shape, 
            // as it'll get clipped by OpenVG at the end of the
            // shape that is being filled with the gradient.
            binfo->createLinearBitmap(_x, _y, width, height, _cxform, records,  _vgpaint);
        }
        if (fill_type == GradientFill::RADIAL) {
            float focalpt = g.focalPoint();
            const std::vector<gnash::GradientRecord> &records = g.getRecords();
            log_debug("Fill Style Type: Radial Gradient: focal is: %d, %d:%d",
                      focalpt, _x, _y);
            binfo->createRadialBitmap(_x, _y, width, height, focalpt,
                                      _cxform, records, _vgpaint);
        }
    }

    void operator()(const SolidFill& f) const {
        // GNASH_REPORT_FUNCTION;
        const rgba incolor = f.color();
        rgba c = _cxform.transform(incolor);
        VGfloat color[] = {
            c.m_r / 255.0f,
            c.m_g / 255.0f,
            c.m_b / 255.0f,
            c.m_a / 255.0f
        };
        
        vgSetParameteri (_vgpaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
        vgSetParameterfv (_vgpaint, VG_PAINT_COLOR, 4, color);
    }

    void operator()(const BitmapFill& b) const {
        GNASH_REPORT_FUNCTION;
        SWFMatrix mat = b.matrix();
        const bool type = b.type();
        CachedBitmap *cb = const_cast<CachedBitmap *>(b.bitmap());
        OpenVGBitmap* binfo = new OpenVGBitmap(_vgpaint);
        if (!cb) {
            // See misc-swfmill.all/missing_bitmap.swf
            // _sh.add_color(agg::rgba8_pre(255,0,0,255));
        } else if ( cb->disposed() ) {
            // See misc-ming.all/BeginBitmapFill.swf
            // _sh.add_color(agg::rgba8_pre(0,0,0,0));
        } else {
            if (type == BitmapFill::TILED) {
                binfo->applyPatternBitmap(mat, OpenVGBitmap::WRAP_REPEAT,
                                          cb, _vgpaint);
            } else if (type == BitmapFill::CLIPPED) {
                binfo->applyPatternBitmap(mat, OpenVGBitmap::WRAP_PAD,
                                          cb, _vgpaint);
            }
        }
    }
    
private:
    const SWFCxForm& _cxform;
    const VGPaint&   _vgpaint;
    float            _x;
    float            _y;
};

} // namespace gnash::renderer::openvg
} // namespace gnash::renderer
} // namespace gnash

#endif // __RENDER_OPENVG_STYLE_H__

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
