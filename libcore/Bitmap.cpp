// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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


#include "Bitmap.h"
#include "flash/display/BitmapData_as.h"
#include "GnashImage.h"
#include "FillStyle.h"
#include "DynamicShape.h"
#include "SWFRect.h"
#include "Renderer.h"
#include "VM.h"
#include "movie_root.h"
#include "RunResources.h"
#include "Transform.h"

namespace gnash {

Bitmap::Bitmap(movie_root& mr, as_object* object, BitmapData_as* bd,
        DisplayObject* parent)
    :
    DisplayObject(mr, object, parent),
    _bitmapData(bd),
    _width(_bitmapData->width()),
    _height(_bitmapData->height())
{
    _shape.setBounds(SWFRect(0, 0,
                pixelsToTwips(_width), pixelsToTwips(_height)));
    assert(bd);
    assert(!bd->disposed());
}

Bitmap::Bitmap(movie_root& mr, as_object* object,
        const BitmapMovieDefinition* def, DisplayObject* parent)
    :
    DisplayObject(mr, object, parent),
    _def(def),
    _bitmapData(0),
    _width(def->get_width_pixels()),
    _height(def->get_height_pixels())
{
    _shape.setBounds(def->get_frame_size());
}

Bitmap::~Bitmap()
{
}

const CachedBitmap*
Bitmap::bitmap() const
{
    if (_def) return _def->bitmap();
    if (_bitmapData) return _bitmapData->bitmapInfo();
    return 0;
}

void
Bitmap::construct(as_object* /*init*/)
{
    if (_bitmapData) _bitmapData->attach(this);

    if (!_def && !_bitmapData) return;

    // Width and height are a maximum of 2880, so there is no risk of 
    // overflow 
    const int w = pixelsToTwips(_width);
    const int h = pixelsToTwips(_height);

    SWFMatrix mat;
    mat.set_scale(1.0 / 20, 1.0 / 20);

    // Can this be tiled? And smoothing?
    FillStyle fill = BitmapFill(BitmapFill::CLIPPED, bitmap(), mat,
            BitmapFill::SMOOTHING_UNSPECIFIED);

    const size_t fillLeft = _shape.addFillStyle(fill);

    Path bmpath(w, h, fillLeft, 0, 0, false);
    bmpath.drawLineTo(w, 0);
    bmpath.drawLineTo(0, 0);
    bmpath.drawLineTo(0, h);
    bmpath.drawLineTo(w, h);

    _shape.add_path(bmpath);
    _shape.finalize();

    set_invalidated();
}

bool
Bitmap::pointInShape(boost::int32_t  x, boost::int32_t  y) const
{
    return pointInBounds(x, y);
}

void
Bitmap::display(Renderer& renderer, const Transform& base)
{
    /// Don't display cleared Bitmaps.
    if (!_def && !_bitmapData) return;
    
    const Transform xform = base * transform();

    _shape.display(renderer, xform);
    clear_invalidated();
}

void
Bitmap::add_invalidated_bounds(InvalidatedRanges& ranges, bool force)
{
    if (!force && !invalidated()) return;

    ranges.add(m_old_invalidated_ranges);

    SWFRect bounds;
    bounds.expand_to_transformed_rect(getWorldMatrix(*this), getBounds()); 
    ranges.add(bounds.getRange());

}

SWFRect
Bitmap::getBounds() const
{
    return _shape.getBounds();
}

void
Bitmap::update()
{
    /// Nothing to do for disposed bitmaps.
    if (!_bitmapData) return;
    
    set_invalidated();

    if (_bitmapData->disposed()) {
        _bitmapData = 0;
        _shape.clear();
    }
}

}
