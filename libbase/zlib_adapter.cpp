// zlib_adapter.cpp	-- Thatcher Ulrich 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Code to wrap zlib compression/decompression around a tu_file
// stream.


#include "zlib_adapter.h"
#include "tu_file.h"
#include "utility.h"
#include "log.h"
#include "GnashException.h"

#include <memory>

using namespace gnash;

#ifndef HAVE_ZLIB_H


// Stubs, in case client doesn't want to link to zlib.
namespace zlib_adapter
{
	std::auto_ptr<tu_file> make_inflater(std::auto_ptr<tu_file> /*in*/)
	{
		abort(); // callers should check this themselves
		return std::auto_ptr<tu_file>(NULL);
	}
	tu_file* make_deflater(tu_file* /*out*/)
	{
		abort(); // callers should check this themselves
		return NULL;
	}
}


#else // HAVE_ZLIB_H


#include <zlib.h>


namespace zlib_adapter
{
	const int	ZBUF_SIZE = 4096;

	class inflater_impl
	{
	private:
		std::auto_ptr<tu_file>	m_in;
		int		m_initial_stream_pos;	// position of the input stream where we started inflating.
		unsigned char	m_rawdata[ZBUF_SIZE];

	public:

		z_stream	m_zstream;

		// current stream position of uncompressed data.
		int		m_logical_stream_pos;

		bool		m_at_eof;
		int		m_error;
		
		inflater_impl(std::auto_ptr<tu_file> in)
		// Constructor.
			:
			m_in(in),
			m_initial_stream_pos(m_in->get_position()),
			m_logical_stream_pos(m_initial_stream_pos),
			m_at_eof(false),
			m_error(0)
		{
			assert(m_in.get());

			m_zstream.zalloc = (alloc_func)0;
			m_zstream.zfree = (free_func)0;
			m_zstream.opaque = (voidpf)0;

			m_zstream.next_in  = 0;
			m_zstream.avail_in = 0;

			m_zstream.next_out = 0;
			m_zstream.avail_out = 0;

			int	err = inflateInit(&m_zstream);
			if (err != Z_OK) {
				gnash::log_error("inflater_impl::ctor() inflateInit() returned %d\n", err);
				m_error = 1;
				return;
			}

			// Ready to go!
		}


		void	reset()
		// Discard current results and rewind to the beginning.
		// Necessary in order to seek backwards.
		{
			m_error = 0;
			m_at_eof = 0;
			int	err = inflateReset(&m_zstream);
			if (err != Z_OK) {
				gnash::log_error("inflater_impl::reset() inflateReset() returned %d\n", err);
				m_error = 1;
				return;
			}

			m_zstream.next_in = 0;
			m_zstream.avail_in = 0;

			m_zstream.next_out = 0;
			m_zstream.avail_out = 0;

			// Rewind the underlying stream.
			m_in->set_position(m_initial_stream_pos);

			m_logical_stream_pos = m_initial_stream_pos;
		}


		int	inflate_from_stream(void* dst, int bytes)
		{
			using gnash::ParserException;

			if (m_error)
			{
				return 0;
			}

			m_zstream.next_out = (unsigned char*) dst;
			m_zstream.avail_out = bytes;

			for (;;)
			{
				if (m_zstream.avail_in == 0)
				{
					// Get more raw data.
					int	new_bytes = m_in->read_bytes(m_rawdata, ZBUF_SIZE);
					if (new_bytes == 0)
					{
						// The cupboard is bare!  We have nothing to feed to inflate().
						break;
					}
					else
					{
						m_zstream.next_in = m_rawdata;
						m_zstream.avail_in = new_bytes;
					}
				}

				int	err = inflate(&m_zstream, Z_SYNC_FLUSH);
				if (err == Z_STREAM_END)
				{
					m_at_eof = true;
					break;
				}
				if (err == Z_BUF_ERROR)
				{
					//gnash::log_error("inflater_impl::inflate_from_stream() inflate() returned Z_BUF_ERROR");
					// we should call inflate again... giving more input or output space !
					//m_error = 1;
					break;
				}
				if (err == Z_DATA_ERROR)
				{
					throw ParserException("Data error inflating input");
					break;
				}
				if (err == Z_MEM_ERROR)
				{
					throw ParserException("Memory error inflating input");
					break;
				}
				if (err != Z_OK)
				{
					// something's wrong.
					std::stringstream ss;
					ss << "inflater_impl::inflate_from_stream() inflate() returned " << err;
					throw ParserException(ss.str());
					//m_error = 1;
					break;
				}

				if (m_zstream.avail_out == 0)
				{
					break;
				}
			}

			if (m_error)
			{
				return 0;
			}

			int	bytes_read = bytes - m_zstream.avail_out;
			m_logical_stream_pos += bytes_read;

			return bytes_read;
		}

		void	rewind_unused_bytes()
		// If we have unused bytes in our input buffer, rewind
		// to before they started.
		{
			if (m_zstream.avail_in > 0)
			{
				int	pos = m_in->get_position();
				int	rewound_pos = pos - m_zstream.avail_in;
				assert(pos >= 0);
				assert(pos >= m_initial_stream_pos);
				assert(rewound_pos >= 0);
				assert(rewound_pos >= m_initial_stream_pos);

				m_in->set_position(rewound_pos);
			}
		}
	};


	inline int inflate_read(void* dst, int bytes, void* appdata)
	// Return number of bytes actually read.
	{
		inflater_impl*	inf = (inflater_impl*) appdata;
		if (inf->m_error)
		{
			return 0;
		}

		return inf->inflate_from_stream(dst, bytes);
	}


       inline int inflate_write(const void* /* src */, int /* bytes */, void* /* appdata */)
	// Return number of bytes actually written.
	{
		// *In*flaters can't write!!!
		abort();
		return 0;
	}


	int inflate_seek(int pos, void* appdata)
	// Try to go to pos.  Return 0 on success or TU_FILE_SEEK_ERROR on error.
	{
		inflater_impl*	inf = (inflater_impl*) appdata;
		if (inf->m_error)
		{
			gnash::log_debug("Inflater is in error condition");
			return TU_FILE_SEEK_ERROR;
			//return inf->m_logical_stream_pos;
		}

		// If we're seeking backwards, then restart from the beginning.
		if (pos < inf->m_logical_stream_pos)
		{
			inf->reset();
		}

		unsigned char	temp[ZBUF_SIZE];

		// Now seek forwards, by just reading data in blocks.
		while (inf->m_logical_stream_pos < pos)
		{
			int	to_read = pos - inf->m_logical_stream_pos;
			int	to_read_this_time = imin(to_read, ZBUF_SIZE);
			assert(to_read_this_time > 0);

			int	bytes_read = inf->inflate_from_stream(temp, to_read_this_time);
			assert(bytes_read <= to_read_this_time);
			if (bytes_read == 0)
			{
				// Trouble; can't seek any further.
				gnash::log_debug("Trouble: can't seek any further.. ");
				return TU_FILE_SEEK_ERROR;
				break;
			}
		}

		//assert(inf->m_logical_stream_pos <= pos);
		assert(inf->m_logical_stream_pos == pos);

		return 0; // inf->m_logical_stream_pos;
	}


	int	inflate_seek_to_end(void* appdata)
	{
		GNASH_REPORT_FUNCTION;
		inflater_impl*	inf = (inflater_impl*) appdata;
		if (inf->m_error)
		{
			return inf->m_logical_stream_pos;
		}

		// Keep reading until we can't read any more.

		unsigned char	temp[ZBUF_SIZE];

		// Seek forwards.
		for (;;)
		{
			int	bytes_read = inf->inflate_from_stream(temp, ZBUF_SIZE);
			if (bytes_read == 0)
			{
				// We've seeked as far as we can.
				break;
			}
		}

		return inf->m_logical_stream_pos;
	}

	inline int inflate_tell(void* appdata)
	{
		inflater_impl*	inf = (inflater_impl*) appdata;

		return inf->m_logical_stream_pos;
	}

	inline bool inflate_get_eof(void* appdata)
	{
		inflater_impl*	inf = (inflater_impl*) appdata;

		return inf->m_at_eof;
	}

	inline int inflate_get_err(void* appdata)
	{
		inflater_impl*	inf = (inflater_impl*) appdata;
		return inf->m_error;
	}

	inline int inflate_close(void* appdata)
	{
		inflater_impl*	inf = (inflater_impl*) appdata;

		inf->rewind_unused_bytes();
		int	err = inflateEnd(&(inf->m_zstream));

		delete inf;

		if (err != Z_OK)
		{
			return TU_FILE_CLOSE_ERROR;
		}

		return 0;
	}


	std::auto_ptr<tu_file> make_inflater(std::auto_ptr<tu_file> in)
	{
		assert(in.get());

		inflater_impl*	inflater = new inflater_impl(in);
		return std::auto_ptr<tu_file> (
			new tu_file(
				inflater,
				inflate_read,
				inflate_write,
				inflate_seek,
				inflate_seek_to_end,
				inflate_tell,
				inflate_get_eof,
				inflate_get_err,
				NULL, // get stream size
				inflate_close)
			);
	}


	// @@ TODO
	// tu_file*	make_deflater(tu_file* out) { ... }
}

#endif // HAVE_ZLIB_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
