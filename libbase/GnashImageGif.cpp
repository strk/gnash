// GnashImageGif.cpp: gif_lib wrapper for Gnash.
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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#include "utility.h"
#include "GnashImageGif.h"
#include "log.h"
#include "GnashException.h"
#include "IOChannel.h"

#include <sstream>
#include <cstring> // std::memcpy
#include <boost/scoped_array.hpp>

extern "C" {
#include <gif_lib.h>
}

namespace gnash {

static int
readData(GifFileType* ft, GifByteType* data, int length)
{
    // Do not read until opened.
    assert(ft);
    IOChannel* in = reinterpret_cast<IOChannel*>(ft->UserData);
    return in->read(reinterpret_cast<char*>(data), length);
}


GifImageInput::GifImageInput(boost::shared_ptr<IOChannel> in) :
    ImageInput(in),
    _gif(NULL),
    _currentRow(0)
{
}

GifImageInput::~GifImageInput()
{
    // Clean up allocated data.
    DGifCloseFile(_gif);
}

size_t
GifImageInput::getHeight() const
{
    assert (_gif);
    return _gif->SHeight;
}

size_t
GifImageInput::getWidth() const
{
    assert (_gif);
    return _gif->SWidth;
}

void
GifImageInput::readScanline(unsigned char* rgbData)
{

    ColorMapObject* colormap = (_gif->Image.ColorMap) ?
                            _gif->Image.ColorMap :
                            _gif->SColorMap;

    assert(colormap);

    unsigned char* ptr = rgbData;

    for (size_t i = 0, e = getWidth(); i < e; ++i)
    {
        GifColorType* mapentry = &colormap->Colors[_gifData[_currentRow][i]];
        *ptr++ = mapentry->Red;
        *ptr++ = mapentry->Green;
        *ptr++ = mapentry->Blue;
    }
    
    _currentRow++;

}

void
GifImageInput::read()
{
    _gif = DGifOpen(_inStream.get(), &readData); 

    GifRecordType record;

    // Parse the (first?) image into memory.
    // Is there a multi-dimensional smart array? It's silly to
    // have to allocate each row separately and can mean a lot
    // of reallocation.
    do {

        if (DGifGetRecordType(_gif, &record) != GIF_OK) {
            throw ParserException(_("GIF: Error retrieving record type"));
        }
        
        switch (record) {

            case IMAGE_DESC_RECORD_TYPE:
            {
                // Fill the _gif->Image fields 
                if (DGifGetImageDesc(_gif) != GIF_OK) {
                    throw ParserException(_("GIF: Error retrieving image "
                                "description"));
                }
                const int backgroundColor = _gif->SBackGroundColor;

                // Set the height dimension of the array
                _gifData.reset(new PixelRow[getHeight()]);
                
                // The GIF 'screen' width and height
                const size_t screenWidth = getWidth();
                const size_t screenHeight = getHeight();

                // Set all the pixels to the background colour.
                for (size_t i = 0; i < screenHeight; ++i) {
                    // Set the width dimension of the array
                    _gifData[i].reset(new GifPixelType[screenWidth]);
                    // Fill all the pixels with the background color.
                    std::memset(_gifData[i].get(), backgroundColor,
                            screenWidth);
                }
                
                // The position of the image on the GIF 'screen'
                const size_t imageHeight = _gif->Image.Height;
                const size_t imageWidth = _gif->Image.Width;
                const size_t imageTop = _gif->Image.Top;
                const size_t imageLeft = _gif->Image.Left;
                
                if (imageHeight + imageTop > screenHeight ||
                    imageWidth + imageLeft > screenWidth) {
                    throw ParserException(_("GIF: invalid image data "
                                "(bounds outside GIF screen)"));
                }

                // Handle interlaced data in four passes.
                if (_gif->Image.Interlace) {
                    log_debug(_("Found interlaced GIF (%d x %d)"),
                            screenWidth, screenHeight);

                    // The order of interlaced GIFs.
                    static const int interlacedOffsets[] =
                                            { 0, 4, 2, 1 };
                    static const int interlacedJumps[] =
                                            { 8, 8, 4, 2 };

                    for (size_t i = 0; i < 4; ++i) {

                        for (size_t j = imageTop + interlacedOffsets[i];
                                    j < imageTop + imageHeight;
                                    j += interlacedJumps[i]) {

                            if (DGifGetLine(_gif, &_gifData[j][imageLeft],
                                        imageWidth) != GIF_OK) {

                                throw ParserException(_("GIF: failed reading "
                                            "pixel data"));

                            }
                        }
                    }
                    break;
                }

                // Non-interlaced data.
                log_debug(_("Found non-interlaced GIF (%d x %d)"),
                        screenWidth, screenHeight);

                for (size_t i = imageTop; i < imageHeight; ++i) {
                    // Read the gif data into the gif array.
                    if (DGifGetLine(_gif, &_gifData[i][imageLeft], imageWidth)
                            != GIF_OK) {
                        throw ParserException(_("GIF: failed reading "
                                    "pixel data"));
                    }                    
                }
                break;
            }
            case EXTENSION_RECORD_TYPE:
                // Skip all extension records.
                GifByteType* extension;
                int extCode;
                DGifGetExtension(_gif, &extCode, &extension);
                while (extension) {
                    if (DGifGetExtensionNext(_gif, &extension) == GIF_ERROR) {
                        break;
                    }
                }
                break;         
            default:
                break;
        }
    } while (record != TERMINATE_RECORD_TYPE);

    // Set the type to RGB
    // TODO: implement RGBA!
    _type = GNASH_IMAGE_RGB;

}

} // namespace gnash

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
