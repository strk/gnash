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

namespace gnash {

Bitmap::Bitmap(boost::intrusive_ptr<BitmapData_as> bd, DisplayObject* parent,
        int id)
    :
    DisplayObject(parent, id),
    _bitmapData(bd),
    _bitmapInfo(0),
    _shapeDef(new DynamicShape),
    _width(_bitmapData->getWidth()),
    _height(_bitmapData->getHeight())
{
    _shapeDef->set_bound(rect(0, 0, _width * 20, _height * 20));
}


Bitmap::~Bitmap()
{
}


void
Bitmap::stagePlacementCallback(as_object* initObj)
{
    assert(!initObj);

    _bitmapData->registerBitmap(this);
    update();
}

bool
Bitmap::pointInShape(boost::int32_t  x, boost::int32_t  y) const
{
    return pointInBounds(x, y);
}

void
Bitmap::display()
{
    assert(_shapeDef);

    _shapeDef->display(*this);

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
    return _shapeDef->get_bound();
}

void
Bitmap::drawBitmap()
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

    _bitmapInfo = render::createBitmapInfo(im);

}


void
Bitmap::finalize()
{

    if (!_bitmapData) return;

    const BitmapData_as::BitmapArray& data = _bitmapData->getBitmapData();

    /// In this case, dispose() was called. It seems like a good idea to
    /// set _bitmapData to 0 to avoid any further interaction.
    if (data.empty()) {
        _bitmapData = 0;
        _shapeDef->set_bound(rect());
        return;
    }

    drawBitmap();

    // Width and height are a maximum of 2880, so there is no risk of 
    // overflow 
    const int w = _width * 20;
    const int h = _height * 20;

    if (!_shapeDef) {
    }

    SWFMatrix mat;
    mat.set_scale(1.0 / 20, 1.0 / 20);
    fill_style fill(_bitmapInfo.get(), mat);
    const size_t fillLeft = _shapeDef->add_fill_style(fill);


    path bmpath(w, h, fillLeft, 0, 0, false);
    bmpath.drawLineTo(w, 0);
    bmpath.drawLineTo(0, 0);
    bmpath.drawLineTo(0, h);
    bmpath.drawLineTo(w, h);

    _shapeDef->add_path(bmpath);

    _shapeDef->finalize();

}

void
Bitmap::update()
{

    set_invalidated();

    finalize();

}

}
