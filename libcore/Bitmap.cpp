// 
//   Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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
#include "DynamicShape.h"
#include "rect.h"
#include "Renderer.h"
#include "VM.h"
#include "movie_root.h"

namespace gnash {

Bitmap::Bitmap(boost::intrusive_ptr<BitmapData_as> bd, DisplayObject* parent,
        int id)
    :
    DisplayObject(parent, id),
    _bitmapData(bd),
    _bitmapInfo(0),
    _width(_bitmapData->getWidth()),
    _height(_bitmapData->getHeight())
{
    _shape.setBounds(rect(0, 0, pixelsToTwips(_width), pixelsToTwips(_height)));
}

Bitmap::Bitmap(const BitmapMovieDefinition* const def, DisplayObject* parent,
        int id)
    :
    DisplayObject(parent, id),
    _def(def),
    _bitmapInfo(0),
    _width(twipsToPixels(def->get_frame_size().width())),
    _height(twipsToPixels(def->get_frame_size().height()))
{
    _shape.setBounds(def->get_frame_size());
}

Bitmap::~Bitmap()
{
}

const BitmapInfo*
Bitmap::bitmap() const
{
    if (_def) return _def->bitmap();
    return _bitmapInfo.get();
}

void
Bitmap::stagePlacementCallback(as_object* initObj)
{
    assert(!initObj);
    if (_bitmapData) _bitmapData->registerBitmap(this);
    update();
}

bool
Bitmap::pointInShape(boost::int32_t  x, boost::int32_t  y) const
{
    return pointInBounds(x, y);
}

void
Bitmap::display(Renderer& renderer)
{
    /// Don't display cleared Bitmaps.
    if (!_def && !_bitmapData) return;

    _shape.display(renderer, *this);
    clear_invalidated();
}

void
Bitmap::add_invalidated_bounds(InvalidatedRanges& ranges, bool force)
{
    if (!force && !m_invalidated) return;

    ranges.add(m_old_invalidated_ranges);

    rect bounds;
    bounds.expand_to_transformed_rect(getWorldMatrix(), getBounds()); 
    ranges.add(bounds.getRange());

}

rect
Bitmap::getBounds() const
{
    return _shape.getBounds();
}

void
Bitmap::makeBitmap()
{

    const BitmapData_as::BitmapArray& data = _bitmapData->getBitmapData();

    std::auto_ptr<GnashImage> im(new ImageRGBA(_width, _height)); 

    for (size_t i = 0; i < _height; ++i) {

        boost::uint8_t* row = im->scanline(i);

        for (size_t j = 0; j < _width; ++j) {
            const BitmapData_as::BitmapArray::value_type pixel =
                data[i * _width + j];
            row[j * 4] = (pixel & 0x00ff0000) >> 16;
            row[j * 4 + 1] = (pixel & 0x0000ff00) >> 8;
            row[j * 4 + 2] = (pixel & 0x000000ff);
            row[j * 4 + 3] = (pixel & 0xff000000) >> 24;
        }
    }

    Renderer* renderer = _vm.getRoot().runInfo().renderer();
    if (renderer) _bitmapInfo = renderer->createBitmapInfo(im);

}


void
Bitmap::checkBitmapData()
{

    /// Nothing to do for disposed bitmaps.
    if (_def && !_bitmapData) return;

    const BitmapData_as::BitmapArray& data = _bitmapData->getBitmapData();

    /// In this case, dispose() was called. It seems like a good idea to
    /// set _bitmapData to 0 to avoid any further interaction.
    if (data.empty()) {
        _bitmapData = 0;
        _shape.clear();
        return;
    }
}

void
Bitmap::makeBitmapShape()
{

    if (!_def && !_bitmapData) return;

    if (_bitmapData) makeBitmap();

    // Width and height are a maximum of 2880, so there is no risk of 
    // overflow 
    const int w = pixelsToTwips(_width);
    const int h = pixelsToTwips(_height);

    SWFMatrix mat;
    mat.set_scale(1.0 / 20, 1.0 / 20);
    fill_style fill(bitmap(), mat);
    const size_t fillLeft = _shape.add_fill_style(fill);

    Path bmpath(w, h, fillLeft, 0, 0, false);
    bmpath.drawLineTo(w, 0);
    bmpath.drawLineTo(0, 0);
    bmpath.drawLineTo(0, h);
    bmpath.drawLineTo(w, h);

    _shape.add_path(bmpath);

    _shape.finalize();

}

void
Bitmap::update()
{
    set_invalidated();
    checkBitmapData();
    makeBitmapShape();
}

}
