// jpeg.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2002

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Wrapper for jpeg file operations.  The actual work is done by the
// IJG jpeg lib.


extern "C" {
#include <png.h>
}

#include "utility.h"
#include "GnashPNG.h"
#include "tu_file.h"
#include "log.h"
#include "GnashException.h"

#include <sstream>
#include <csetjmp>


namespace gnash {
namespace png {

/// input/output wrappers for IOChannel
namespace IOChannel_wrappers
{

	// Helper object for reading png image data.  Basically a thin
	static const int IO_BUF_SIZE = 4096;

	// A jpeglib source manager that reads from a IOChannel.  Paraphrased
	// from IJG jpeglib jdatasrc.c.
	class rw_source_IOChannel
	{
	public:

		// Constructor.  The caller is responsible for closing the input stream
		// after it's done using us.
		//
		rw_source_IOChannel(gnash::IOChannel* in)
			:
			m_in_stream(in)
		{
			init();
		}

		~rw_source_IOChannel()
		{
		}


		void	discard_partial_buffer()
		{
			// Discard existing bytes in our buffer.
		}

        void init() {}

	private:

		gnash::IOChannel*	m_in_stream;		/* source stream */

	};


	/// Basically this is a thin wrapper
	class input_IOChannel : public input
	{
	private:
   		// State needed for input.
        png_structp _pngPtr;
        png_infop _infoPtr;
       
        // A reference to the stream containing the PNG data.
        IOChannel& _inStream;
        
        // A counter for keeping track of the last row copied.
        size_t _currentRow;

	public:

		/// Constructor.  
		//
		/// @param in
		/// 	The stream to read from.
		input_IOChannel(gnash::IOChannel& in) :
		    _pngPtr(0),
		    _infoPtr(0),
		    _inStream(in),
		    _currentRow(0)
		{
		    init();
    	}

        static void error(png_struct*, const char* err)
        {
        }
        
        static void warning(png_struct*, const char* warning)
        {
        }

        // Read function using a gnash::IOChannel
        static void readData(png_structp pngptr,
                    png_bytep data, png_size_t length)
        {
            // Do not call unless the PNG exists.
            assert (pngptr);
            IOChannel* in = reinterpret_cast<IOChannel*>(png_get_io_ptr(pngptr));
            in->read(reinterpret_cast<char*>(data), length);
        }

        void init()
        {
            // Initialize png library.
            _pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                                NULL, &error, &warning);
            if (!_pngPtr) return;

            _infoPtr = png_create_info_struct(_pngPtr);
            if (!_infoPtr)
            {
                png_destroy_read_struct(&_pngPtr, (png_infopp)NULL, (png_infopp)NULL);
                return;
            }
        }

        void read()
        {
            // Set our user-defined reader function
            png_set_read_fn(_pngPtr, &_inStream, &readData);
            
            // read
            // TODO: sort out transform options.
            png_read_png(_pngPtr, _infoPtr, PNG_TRANSFORM_STRIP_ALPHA, NULL);
        }

		// Destructor. Free libpng-allocated memory.
		~input_IOChannel()
		{
            png_destroy_read_struct(&_pngPtr, &_infoPtr, (png_infopp)NULL);
		}

		// Return the height of the image.
		size_t getHeight() const
		{
		    assert (_pngPtr && _infoPtr);
            return png_get_image_height(_pngPtr, _infoPtr);
        }

		// Return the width of the image.
		size_t getWidth() const
		{
		    assert (_pngPtr && _infoPtr);
            return png_get_image_width(_pngPtr, _infoPtr);
		}

		// Return number of components (i.e. == 3 for RGB
		// data).  The size of the data for a scanline is
		// get_width() * get_components().
		//
		int	getComponents() const
		{
            return 3;
		}

		// Read a scanline's worth of image data into the
		// given buffer.  The amount of data read is
		// get_width() * get_components().
		//
		void readScanline(unsigned char* rgb_data)
		{
            assert (_currentRow < getHeight());
		    png_bytepp row_pointers = png_get_rows(_pngPtr, _infoPtr);
		    std::memcpy(rgb_data, row_pointers[_currentRow], getWidth() * getComponents());
            ++_currentRow;
		}
    };

} // namespace IOChannel_wrappers


void
input::errorOccurred(const char* msg)
{
	gnash::log_debug("Long jump: banzaaaaaai!");
	_errorOccurred = msg;
	std::longjmp(_jmpBuf, 1);
}


std::auto_ptr<input>
input::create(gnash::IOChannel& in)
{
	using IOChannel_wrappers::input_IOChannel;
	std::auto_ptr<input> ret ( new input_IOChannel(in) );
	if ( ret.get() ) ret->read(); // might throw an exception (I guess)
	return ret;
}

/*static*/
//input*
//input::create_swf_jpeg2_header_only(gnash::IOChannel* in, unsigned int maxHeaderBytes, bool takeOwnership)
//{
//	using IOChannel_wrappers::input_IOChannel;
//	std::auto_ptr<input_IOChannel> ret ( new input_IOChannel(in, takeOwnership) );
//	if ( ret.get() ) ret->readHeader(maxHeaderBytes); // might throw an exception
//	return ret.release();
//}


} // namespace png
} // namespace gnash

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
