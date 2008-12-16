


#include "Bitmap.h"
#include "flash/display/BitmapData_as.h"
#include "GnashImage.h"
#include "DynamicShape.h"
#include "rect.h"

namespace gnash {

Bitmap::Bitmap(boost::intrusive_ptr<BitmapData_as> bd, character* parent,
        int id)
    :
    character(parent, id),
    _bitmapData(bd),
    _bitmapInfo(0),
    _shapeDef(0)
{
    _bitmapData->registerBitmap(this);
    update();
}

Bitmap::~Bitmap()
{
    _bitmapData->unregisterBitmap(this);
}

void
Bitmap::display()
{
    assert(_shapeDef);

    _shapeDef->display(this);

    clear_invalidated();
}

void
Bitmap::add_invalidated_bounds(InvalidatedRanges& ranges, bool force)
{
    if (!force && !m_invalidated) return;

    rect bounds;
    bounds.expand_to_transformed_rect(getWorldMatrix(), getBounds()); 
    ranges.add(bounds.getRange());

    log_debug("ranges now: %s", ranges);

}

rect
Bitmap::getBounds() const
{
    return _shapeDef->get_bound();
}

void
Bitmap::update()
{

    set_invalidated();

    const BitmapData_as::BitmapArray& data = _bitmapData->getBitmapData();

    if (data.empty()) {
        _shapeDef->set_bound(rect());
        return;
    }

    std::auto_ptr<GnashImage> im(new ImageRGBA(_bitmapData->getWidth(), 
                _bitmapData->getHeight()));

    for (size_t i = 0; i < _bitmapData->getHeight(); ++i) {

        boost::uint8_t* row = im->scanline(i);

        for (size_t j = 0; j < _bitmapData->getWidth(); ++j) {
            const BitmapData_as::BitmapArray::value_type pixel =
                data[i * _bitmapData->getWidth() + j];
            row[j * 4] = (pixel & 0x00ff0000) >> 16;
            row[j * 4 + 1] = (pixel & 0x0000ff00) >> 8;
            row[j * 4 + 2] = (pixel & 0x000000ff);
            row[j * 4 + 3] = (pixel & 0xff000000) >> 24;
        }
    }

    _bitmapInfo = render::createBitmapInfo(im);

    if (!_shapeDef) {
        _shapeDef = new DynamicShape;
        _shapeDef->set_bound(rect(0, 0,
                 _bitmapData->getWidth() * 20, _bitmapData->getHeight() * 20));

        SWFMatrix mat;
        mat.set_scale(1.0 / 20, 1.0 / 20);
        fill_style fill(_bitmapInfo.get(), mat);
        const size_t fillLeft = _shapeDef->add_fill_style(fill);

        boost::int32_t w = _bitmapData->getWidth() * 20;
        boost::int32_t h = _bitmapData->getHeight() * 20;

        path bmpath(w, h, fillLeft, 0, 0, false);
        bmpath.drawLineTo(w, 0);
        bmpath.drawLineTo(0, 0);
        bmpath.drawLineTo(0, h);
        bmpath.drawLineTo(w, h);

        _shapeDef->add_path(bmpath);
    }

    _shapeDef->finalize();

}

}
