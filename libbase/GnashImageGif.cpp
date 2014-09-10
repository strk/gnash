// GnashImageGif.cpp: gif_lib wrapper for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#include "GnashImageGif.h"

#include <sstream>
#include <algorithm>
#include <boost/scoped_array.hpp>

extern "C" {
#include <gif_lib.h>
}

#include "GnashImage.h"
#include "utility.h"
#include "log.h"
#include "GnashException.h"
#include "IOChannel.h"

namespace gnash {
namespace image {

namespace {

int
readData(GifFileType* ft, GifByteType* data, int length)
{
    // Do not read until opened.
    assert(ft);
    IOChannel* in = reinterpret_cast<IOChannel*>(ft->UserData);
    return in->read(reinterpret_cast<char*>(data), length);
}

class GifInput : public Input
{

public:

    /// Construct a GifInput object to read from an IOChannel.
    //
    /// @param in   The stream to read GIF data from. Ownership is shared
    ///             between caller and GifInput, so it is freed
    ///             automatically when the last owner is destroyed.
    GifInput(boost::shared_ptr<IOChannel> in);
    
    ~GifInput();

    /// Begin processing the image data.
    void read();

    /// Get the image's height in pixels.
    //
    /// @return     The height of the image in pixels.
    size_t getHeight() const;

    /// Get the image's width in pixels.
    //
    /// @return     The width of the image in pixels.
    size_t getWidth() const;

    /// Get number of components (channels)
    //
    /// @return     The number of components, e.g. 3 for RGB
    size_t getComponents() const { return 3; }

    /// Read a scanline's worth of image data into the given buffer.
    //
    /// The amount of data read is getWidth() * getComponents().
    ///
    /// @param rgbData  The buffer for writing raw RGB data to.
    void readScanline(unsigned char* rgb_data);

private:
    
    /// Initialize gif_lib
    void init();

    /// Process a single image record
    //
    /// @return     false if no image was parsed, true if we have an image.
    bool processRecord(GifRecordType record);

    // State needed for input.
    GifFileType* _gif;
    
    // A counter for keeping track of the last row copied.
    size_t _currentRow;
    
    typedef boost::scoped_array<GifPixelType> PixelRow;

    // A 2-dimensional scoped array holding the unpacked pixel data.
    boost::scoped_array<PixelRow> _gifData;
};


GifInput::GifInput(boost::shared_ptr<IOChannel> in)
    :
    Input(in),
    _gif(0),
    _currentRow(0)
{
}


GifInput::~GifInput()
{
    // Clean up allocated data.
#if GIFLIB_MAJOR==5 && GIFLIB_MINOR==1    
	DGifCloseFile(_gif, 0);
#else
	DGifCloseFile(_gif);
#endif
}

size_t
GifInput::getHeight() const
{
    assert (_gif);
    return _gif->SHeight;
}

size_t
GifInput::getWidth() const
{
    assert (_gif);
    return _gif->SWidth;
}

void
GifInput::readScanline(unsigned char* rgbData)
{

    const ColorMapObject* const colormap = (_gif->Image.ColorMap) ?
                            _gif->Image.ColorMap : _gif->SColorMap;

    assert(colormap);

    unsigned char* ptr = rgbData;

    for (size_t i = 0, e = getWidth(); i < e; ++i) {

        const GifColorType* const mapentry =
            &colormap->Colors[_gifData[_currentRow][i]];

        *ptr++ = mapentry->Red;
        *ptr++ = mapentry->Green;
        *ptr++ = mapentry->Blue;
    }
    
    _currentRow++;

}

bool
GifInput::processRecord(GifRecordType record)
{
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
                std::fill_n(_gifData[i].get(), screenWidth,
                        backgroundColor);
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
                log_debug("Found interlaced GIF (%d x %d)",
                        screenWidth, screenHeight);

                // The order of interlaced GIFs.
                const int interlacedOffsets[] = { 0, 4, 2, 1 };
                const int interlacedJumps[] = { 8, 8, 4, 2 };

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
                // One record is enough.
                return true;
            }

            // Non-interlaced data.
            log_debug("Found non-interlaced GIF (%d x %d)",
                    screenWidth, screenHeight);

            for (size_t i = imageTop; i < imageHeight; ++i) {
                // Read the gif data into the gif array.
                if (DGifGetLine(_gif, &_gifData[i][imageLeft], imageWidth)
                        != GIF_OK) {
                    throw ParserException(_("GIF: failed reading "
                                "pixel data"));
                }                    
            }
            // One record is enough.
            return true;
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
    return false;
}

void
GifInput::read()
{
#if GIFLIB_MAJOR >= 5
    int errorCode;
    _gif = DGifOpen(_inStream.get(), &readData, &errorCode); 
#else
    _gif = DGifOpen(_inStream.get(), &readData); 
#endif

    if ( ! _gif ) {
        // TODO: decode errorCode if available
        throw ParserException("Could not open input GIF stream");
    }

    GifRecordType record;

    // Parse the (first?) image into memory.
    // Is there a multi-dimensional smart array? It's silly to
    // have to allocate each row separately and can mean a lot
    // of reallocation.
    do {

        if (DGifGetRecordType(_gif, &record) != GIF_OK) {
            throw ParserException(_("GIF: Error retrieving record type"));
        }
        if (processRecord(record)) break;
            
    } while (record != TERMINATE_RECORD_TYPE);

    // Set the type to RGB
    // TODO: implement RGBA!
    _type = TYPE_RGB;

}

} // unnamed namespace

std::auto_ptr<Input>
createGifInput(boost::shared_ptr<IOChannel> in)
{
    std::auto_ptr<Input> ret(new GifInput(in));
    ret->read();
    return ret;
}

} // namespace image
} // namespace gnash

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
