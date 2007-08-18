// jpeg.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2002

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Wrapper for jpeg file operations.  The actual work is done by the
// IJG jpeg lib.


#include "utility.h"
#include "jpeg.h"
#include "tu_file.h"
#include "log.h"
#include "GnashException.h"

#include <cstdio>

extern "C" {
#undef HAVE_STDLIB_H
#include <jpeglib.h>
}


namespace jpeg
{
	// jpeglib data source constructors, for using tu_file* instead
	// of stdio for jpeg IO.
	void	setup_rw_source(jpeg_decompress_struct* cinfo, tu_file* instream);
	void	setup_rw_dest(jpeg_compress_struct* cinfo, tu_file* outstream);


	// Helper object for reading jpeg image data.  Basically a thin
	static const int	IO_BUF_SIZE = 4096;

	// A jpeglib source manager that reads from a tu_file.  Paraphrased
	// from IJG jpeglib jdatasrc.c.
	class rw_source
	{
	public:
		struct jpeg_source_mgr	m_pub;		/* public fields */

		rw_source(tu_file* in)
			:
			m_in_stream(in),
			m_start_of_file(true)
		// Constructor.  The caller is responsible for closing the input stream
		// after it's done using us.
		{
			// fill in function pointers...
			m_pub.init_source = init_source;
			m_pub.fill_input_buffer = fill_input_buffer;
			m_pub.skip_input_data = skip_input_data;
			m_pub.resync_to_restart = jpeg_resync_to_restart;	// use default method
			m_pub.term_source = term_source;
			m_pub.bytes_in_buffer = 0;
			m_pub.next_input_byte = NULL;
		}

		static void init_source(j_decompress_ptr cinfo)
		{
			rw_source*	src = (rw_source*) cinfo->src;
			src->m_start_of_file = true;
		}

		// Read data into our input buffer.  Client calls this
		// when it needs more data from the file.
		static boolean fill_input_buffer(j_decompress_ptr cinfo)
		{
			rw_source*	src = (rw_source*) cinfo->src;

			size_t	bytes_read = src->m_in_stream->read_bytes(src->m_buffer, IO_BUF_SIZE);

			if (bytes_read <= 0) {
				// Is the file completely empty?
				if (src->m_start_of_file) {
					// Treat this as a fatal error.
					gnash::log_error("empty jpeg source stream.");
					return FALSE;
				}
				// warn("jpeg end-of-stream");

				// Insert a fake EOI marker.
				src->m_buffer[0] = (JOCTET) 0xFF;
				src->m_buffer[1] = (JOCTET) JPEG_EOI;
				bytes_read = 2;
			}

			// Hack to work around SWF bug: sometimes data
			// starts with FFD9FFD8, when it should be
			// FFD8FFD9!
			if (src->m_start_of_file && bytes_read >= 4)
			{
				if (src->m_buffer[0] == 0xFF
				    && src->m_buffer[1] == 0xD9 
				    && src->m_buffer[2] == 0xFF
				    && src->m_buffer[3] == 0xD8)
				{
					src->m_buffer[1] = 0xD8;
					src->m_buffer[3] = 0xD9;
				}
			}

			// Expose buffer state to clients.
			src->m_pub.next_input_byte = src->m_buffer;
			src->m_pub.bytes_in_buffer = bytes_read;
			src->m_start_of_file = false;

			return TRUE;
		}

		// Called by client when it wants to advance past some
		// uninteresting data.
		static void	skip_input_data(j_decompress_ptr cinfo, long num_bytes)
		{
			rw_source*	src = (rw_source*) cinfo->src;

			// According to jpeg docs, large skips are
			// infrequent.  So let's just do it the simple
			// way.
			if (num_bytes > 0) {
				while (num_bytes > (long) src->m_pub.bytes_in_buffer) {
					num_bytes -= (long) src->m_pub.bytes_in_buffer;
					fill_input_buffer(cinfo);
				}
				// Handle remainder.
				src->m_pub.next_input_byte += (size_t) num_bytes;
				src->m_pub.bytes_in_buffer -= (size_t) num_bytes;
			}
		}

		static void term_source(j_decompress_ptr /* cinfo */)
		// Terminate the source.  Make sure we get deleted.
		{
			/*rw_source*	src = (rw_source*) cinfo->src;
			assert(src);

			// @@ it's kind of bogus to be deleting here
			// -- term_source happens at the end of
			// reading an image, but we're probably going
			// to want to init a source and use it to read
			// many images, without reallocating our
			// buffer.
			delete src;
			cinfo->src = NULL;*/
		}


		void	discard_partial_buffer()
		{
			// Discard existing bytes in our buffer.
			m_pub.bytes_in_buffer = 0;
			m_pub.next_input_byte = NULL;
		}
	private:
		tu_file*	m_in_stream;		/* source stream */
		bool	m_start_of_file;		/* have we gotten any data yet? */
		JOCTET	m_buffer[IO_BUF_SIZE];		/* start of buffer */
	};

	
	void	setup_rw_source(jpeg_decompress_struct* cinfo, tu_file* instream)
	// Set up the given decompress object to read from the given
	// stream.
	{
		// assert(cinfo->src == NULL);
		cinfo->src = (jpeg_source_mgr*) (new rw_source(instream));
	}


	// A jpeglib destination manager that writes to a tu_file.
	// Paraphrased from IJG jpeglib jdatadst.c.
	class rw_dest
	{
	public:
		struct jpeg_destination_mgr	m_pub;	/* public fields */

		/// Constructor. 
		//
		/// The caller is responsible for closing
		/// the output stream after it's done using us.
		///
		/// @param out
		///	The output stream, externally owned.
		///
		rw_dest(tu_file* out)
			:
			m_out_stream(out)
		{
			// fill in function pointers...
			m_pub.init_destination = init_destination;
			m_pub.empty_output_buffer = empty_output_buffer;
			m_pub.term_destination = term_destination;

			m_pub.next_output_byte = m_buffer;
			m_pub.free_in_buffer = IO_BUF_SIZE;
		}

		static void init_destination(j_compress_ptr cinfo)
		{
			rw_dest*	dest = (rw_dest*) cinfo->dest;
			assert(dest);

			dest->m_pub.next_output_byte = dest->m_buffer;
			dest->m_pub.free_in_buffer = IO_BUF_SIZE;
		}

		/// Write the output buffer into the stream.
		static boolean	empty_output_buffer(j_compress_ptr cinfo)
		{
			rw_dest*	dest = (rw_dest*) cinfo->dest;
			assert(dest);

			if (dest->m_out_stream->write_bytes(dest->m_buffer, IO_BUF_SIZE) != IO_BUF_SIZE)
			{
				// Error.
				// @@ bah, exceptions suck.  TODO consider alternatives.
				gnash::log_error("jpeg::rw_dest couldn't write data.");
				return false;
			}

			dest->m_pub.next_output_byte = dest->m_buffer;
			dest->m_pub.free_in_buffer = IO_BUF_SIZE;

			return true;
		}

		/// Terminate the destination. 
		//
		/// Flush any leftover data, and make sure we get deleted.
		///
		static void term_destination(j_compress_ptr cinfo)
		{
			rw_dest*	dest = (rw_dest*) cinfo->dest;
			assert(dest);

			// Write any remaining data.
			int	datacount = IO_BUF_SIZE - dest->m_pub.free_in_buffer;
			if (datacount > 0) {
				if (dest->m_out_stream->write_bytes(dest->m_buffer, datacount) != datacount)
				{
					// Error.
					gnash::log_error("jpeg::rw_dest::term_destination couldn't write data.");
				}
			}

			// Clean ourselves up.
			delete dest;
			cinfo->dest = NULL;
		}

	private:	

		// source stream, externally owned
		tu_file*	m_out_stream;	

		JOCTET	m_buffer[IO_BUF_SIZE];		/* start of buffer */
	};


	// Set up the given compress object to write to the given
	// output stream.
	void	setup_rw_dest(j_compress_ptr cinfo, tu_file* outstream)
	{
		cinfo->dest = (jpeg_destination_mgr*) (new rw_dest(outstream));
	}


	//
	// Error handler
	//


	// Called when jpeglib has a fatal error.
	static void	jpeg_error_exit(j_common_ptr cinfo);

	// Set up some error handlers for the jpeg lib.
	static void	setup_jpeg_err(jpeg_error_mgr* jerr)
	{
		// Set up defaults.
		jpeg_std_error(jerr);

		jerr->error_exit = jpeg_error_exit;
	}


	//
	// wrappers
	//


	/// Bascially this is a thin wrapper around jpeg_decompress object.
	class input_impl : public input
	{
	public:
		// State needed for input.
		struct jpeg_decompress_struct	m_cinfo;
		struct jpeg_error_mgr	m_jerr;

		bool	m_compressor_opened;

		/// This flag will be set to true by the error callback
		/// invoked by jpeg lib. Will be later used to throw
		/// a ParserException.
		///
		bool errorOccurred;

		enum SWF_DEFINE_BITS_JPEG2 { SWF_JPEG2 };
		enum SWF_DEFINE_BITS_JPEG2_HEADER_ONLY { SWF_JPEG2_HEADER_ONLY };

		// Constructor.  Read the header data from in, and
		// prepare to read data.
		input_impl(tu_file* in)
			:
			m_compressor_opened(false),
			errorOccurred(false)
		{
			setup_jpeg_err(&m_jerr);
			m_cinfo.err = &m_jerr;
			m_cinfo.client_data = this;

			// Initialize decompression object.
			jpeg_create_decompress(&m_cinfo);

			setup_rw_source(&m_cinfo, in);

			start_image();
		}


		// The SWF file format stores JPEG images with the
		// encoding tables separate from the image data.  This
		// constructor reads the encoding table only and keeps
		// them in this object.  You need to call
		// start_image() and finish_image() around any calls
		// to get_width/height/components and read_scanline.
		//
		input_impl(SWF_DEFINE_BITS_JPEG2_HEADER_ONLY /* e */, tu_file* in)
			:
			m_compressor_opened(false),
			errorOccurred(false)
		{
			setup_jpeg_err(&m_jerr);
			m_cinfo.err = &m_jerr;
			m_cinfo.client_data = this;

			// Initialize decompression object.
			jpeg_create_decompress(&m_cinfo);

			setup_rw_source(&m_cinfo, in);

			// Read the encoding tables.
			int ret = jpeg_read_header(&m_cinfo, FALSE);
			switch (ret)
			{
				case JPEG_SUSPENDED: // suspended due to lack of data
					throw gnash::ParserException("lack of data during JPEG header parsing");
					//log_debug("jpeg_read_header returned JPEG_SUSPENDED");
					break;
				case JPEG_HEADER_OK: // Found valid image datastream
					//gnash::log_debug("unexpected: jpeg_read_header returned JPEG_HEADER_OK [%s:%d]", __FILE__, __LINE__);
					break;
				case JPEG_HEADER_TABLES_ONLY: // Found valid table-specs-only datastream
					//log_debug("jpeg_read_header returned JPEG_HEADER_TABLES_ONLY");
					break;
				default:
					gnash::log_debug("unexpected: jpeg_read_header returned %d [%s:%d]", ret, __FILE__, __LINE__);
					break;
			}

			if ( errorOccurred )
			{
				throw gnash::ParserException("errors during JPEG header parsing");
			}

			// Don't start reading any image data!
			// App does that manually using start_image.
		}

		// Destructor.  Clean up our jpeg reader state.
		~input_impl()
		{
			finish_image();

			rw_source* src = (rw_source*) m_cinfo.src;
			delete src;
			m_cinfo.src = NULL;


			jpeg_destroy_decompress(&m_cinfo);
		}


		// Discard any data sitting in our input buffer.  Use
		// this before/after reading headers or partial image
		// data, to avoid screwing up future reads.
		void	discard_partial_buffer()
		{
			rw_source* src = (rw_source*) m_cinfo.src;

			// We only have to discard the input buffer after reading the tables.
			if (src)
			{
				src->discard_partial_buffer();
			}
		}


		// This is something you can do with "abbreviated"
		// streams; i.e. if you constructed this inputter
		// using (SWF_JPEG2_HEADER_ONLY) to just load the
		// tables, or if you called finish_image() and want to
		// load another image using the existing tables.
		// 
		void	start_image()
		{
			assert(m_compressor_opened == false);

			// hack, FIXME
			static const int stateReady = 202;	/* found SOS, ready for start_decompress */
			while (m_cinfo.global_state != stateReady)
			{
				int ret = jpeg_read_header(&m_cinfo, FALSE);
				switch (ret)
				{
					case JPEG_SUSPENDED: // suspended due to lack of data
						throw gnash::ParserException("lack of data during JPEG header parsing");
						//log_debug("jpeg_read_header returned JPEG_SUSPENDED");
						break;
					case JPEG_HEADER_OK: // Found valid image datastream
						//log_debug("jpeg_read_header returned JPEG_HEADER_OK");
						break;
					case JPEG_HEADER_TABLES_ONLY: // Found valid table-specs-only datastream
						//gnash::log_debug("unexpected: jpeg_read_header returned JPEG_HEADER_TABLES_ONLY [%s:%d]", __FILE__, __LINE__);
						break;
					default:
						gnash::log_debug("unexpected: jpeg_read_header returned %d [%s:%d]", ret, __FILE__, __LINE__);
						break;
				}
			}

			if ( errorOccurred )
			{
				throw gnash::ParserException("errors during JPEG header parsing");
			}

			jpeg_start_decompress(&m_cinfo);

			if ( errorOccurred )
			{
				throw gnash::ParserException("errors during JPEG decompression");
			}

			m_compressor_opened = true;
		}

		void	finish_image()
		{
			if (m_compressor_opened)
			{
				jpeg_finish_decompress(&m_cinfo);
				m_compressor_opened = false;
			}
		}

		// Return the height of the image.  Take the data from our m_cinfo struct.
		int	get_height() const
		{
			assert(m_compressor_opened);
			return m_cinfo.output_height;
		}

		// Return the width of the image.  Take the data from our m_cinfo struct.
		int	get_width() const
		{
			assert(m_compressor_opened);
			return m_cinfo.output_width;
		}

		// Return number of components (i.e. == 3 for RGB
		// data).  The size of the data for a scanline is
		// get_width() * get_components().
		//
		int	get_components() const
		{
			assert(m_compressor_opened);
			return m_cinfo.output_components;
		}


		// Read a scanline's worth of image data into the
		// given buffer.  The amount of data read is
		// get_width() * get_components().
		//
		void	read_scanline(unsigned char* rgb_data)
		{
			assert(m_compressor_opened);
			assert(m_cinfo.output_scanline < m_cinfo.output_height);
			int	lines_read = jpeg_read_scanlines(&m_cinfo, &rgb_data, 1);
			assert(lines_read == 1);
			lines_read = lines_read;	// avoid warning in NDEBUG
			// Expand grayscale to RGB
			if (m_cinfo.out_color_space == JCS_GRAYSCALE)
			{
				int w = get_width();
				unsigned char* src = rgb_data + w - 1;
				unsigned char* dst = rgb_data + (w * 3) - 1;
				for (;  w;  w--, src--)
				{
					*dst-- = *src;
					*dst-- = *src;
					*dst-- = *src;
				}
			}	
		}
	};

	static void	jpeg_error_exit(j_common_ptr cinfo)
	{
        IF_VERBOSE_MALFORMED_SWF(
		gnash::log_swferror("Internal jpeg error: %s", cinfo->err->jpeg_message_table[cinfo->err->msg_code]);
        );

		// Set a flag to stop parsing 
		input_impl* impl = static_cast<input_impl*>(cinfo->client_data);
		impl->errorOccurred = true;
	}



	/*static*/
	input*
	input::create(tu_file* in)
	{
		input* ret = new input_impl(in);
		return ret;
	}

	/*static*/
	input*
	input::create_swf_jpeg2_header_only(tu_file* in)
	{
		input* ret = new input_impl(input_impl::SWF_JPEG2_HEADER_ONLY, in);
		return ret;
	}


	// Default destructor.
	input::~input() {}


	class output_impl : public output
	// Basically this is a thin wrapper around jpeg_compress
	// object.
	{
	public:
		// State needed for output.
		struct jpeg_compress_struct	m_cinfo;
		struct jpeg_error_mgr m_jerr;

		/// Constructor. 
		//
		/// Read the header data from in, and
		///  prepare to read data.
		output_impl(tu_file* out, int width, int height, int quality)
		{
			m_cinfo.err = jpeg_std_error(&m_jerr);

			// Initialize decompression object.
			jpeg_create_compress(&m_cinfo);

			setup_rw_dest(&m_cinfo, out);
			m_cinfo.image_width = width;
			m_cinfo.image_height = height;
			m_cinfo.input_components = 3;
			m_cinfo.in_color_space = JCS_RGB;
			jpeg_set_defaults(&m_cinfo);
			jpeg_set_quality(&m_cinfo, quality, TRUE);

			jpeg_start_compress(&m_cinfo, TRUE);
		}


		~output_impl()
		// Destructor.  Clean up our jpeg reader state.
		{
			jpeg_finish_compress(&m_cinfo);
/*
			rw_dest* src = (rw_source*) m_cinfo.dest;
			delete dest;
			m_cinfo.dest = NULL;
*/
			jpeg_destroy_compress(&m_cinfo);
		}


		void	write_scanline(unsigned char* rgb_data)
		// Write out a single scanline.
		{
			jpeg_write_scanlines(&m_cinfo, &rgb_data, 1);
		}
	};


	/*static*/ output*	output::create(tu_file* in, int width, int height, int quality)
	// Create and return a jpeg-input object that will read from the
	// given input stream.
	{
		return new output_impl(in, width, height, quality);
	}


	// Default constructor.
	output::~output() {}
}


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
