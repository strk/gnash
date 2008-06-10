// zlib_adapter.cpp	-- Thatcher Ulrich 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Code to wrap zlib compression/decompression around a IOChannel
// stream.


#include "zlib_adapter.h"
#include "IOChannel.h" // for inheritance
#include "log.h"
#include "GnashException.h"
#include <algorithm> // std::min

#include <sstream>
#include <memory>

namespace gnash {


#ifndef HAVE_ZLIB_H


// Stubs, in case client doesn't want to link to zlib.
namespace zlib_adapter
{
	std::auto_ptr<IOChannel> make_inflater(std::auto_ptr<IOChannel> /*in*/)
	{
		abort(); // callers should check this themselves
		return std::auto_ptr<IOChannel>(NULL);
	}

	IOChannel* make_deflater(IOChannel* /*out*/)
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

class InflaterIOChannel : public IOChannel 
{
public:

	/// Constructor.
	InflaterIOChannel(std::auto_ptr<IOChannel> in);

	~InflaterIOChannel() {
		rewind_unused_bytes();
		inflateEnd(&(m_zstream));
	}

	// See dox in IOChannel
	virtual int seek(int pos);

	// See dox in IOChannel
	virtual int read(void* dst, int bytes)
	{
		if (m_error) return 0;
		return inflate_from_stream(dst, bytes);
	}

	// See dox in IOChannel
	virtual void go_to_end();

	// See dox in IOChannel
	virtual int tell() const
	{
		return m_logical_stream_pos;
	}

	// See dox in IOChannel
	virtual bool eof() const
	{
		return m_at_eof;
	}

	// See dox in IOChannel
	virtual int get_error() const
	{
		return m_error;
	}

private:

	std::auto_ptr<IOChannel>	m_in;
	int		m_initial_stream_pos;	// position of the input stream where we started inflating.
	unsigned char	m_rawdata[ZBUF_SIZE];
	z_stream	m_zstream;

	// current stream position of uncompressed data.
	int		m_logical_stream_pos;

	bool		m_at_eof;
	int		m_error;

	/// Discard current results and rewind to the beginning.
	//
	//
	/// Necessary in order to seek backwards.
	///
	/// might throw a ParserException if unable to reset the uderlying
	/// stream to original position.
	///
	void	reset();

	int	inflate_from_stream(void* dst, int bytes);

	// If we have unused bytes in our input buffer, rewind
	// to before they started.
	void	rewind_unused_bytes();


};

void
InflaterIOChannel::rewind_unused_bytes()
{
	if (m_zstream.avail_in > 0)
	{
		int	pos = m_in->tell();
		int	rewound_pos = pos - m_zstream.avail_in;
		assert(pos >= 0);
		assert(pos >= m_initial_stream_pos);
		assert(rewound_pos >= 0);
		assert(rewound_pos >= m_initial_stream_pos);

		m_in->seek(rewound_pos);
	}
}

void
InflaterIOChannel::reset()
{
	m_error = 0;
	m_at_eof = 0;
	int	err = inflateReset(&m_zstream);
	if (err != Z_OK) {
		gnash::log_error("inflater_impl::reset() inflateReset() returned %d", err);
		m_error = 1;
		return;
	}

	m_zstream.next_in = 0;
	m_zstream.avail_in = 0;

	m_zstream.next_out = 0;
	m_zstream.avail_out = 0;

	// Rewind the underlying stream.
	if ( m_in->seek(m_initial_stream_pos) == TU_FILE_SEEK_ERROR )
	{
		std::stringstream ss;
		ss << "inflater_impl::reset: unable to seek underlying stream to position " <<  m_initial_stream_pos;
		throw gnash::ParserException(ss.str());
	}

	m_logical_stream_pos = m_initial_stream_pos;
}

int
InflaterIOChannel::inflate_from_stream(void* dst, int bytes)
{
	using gnash::ParserException;

	assert(bytes);

	if (m_error) return 0;

	m_zstream.next_out = (unsigned char*) dst;
	m_zstream.avail_out = bytes;

	for (;;)
	{
		if (m_zstream.avail_in == 0)
		{
			// Get more raw data.
			int	new_bytes = m_in->read(m_rawdata, ZBUF_SIZE);
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
			std::ostringstream ss;
			ss << __FILE__ << ":" << __LINE__ << ": " << m_zstream.msg;
			// we should call inflate again... giving more input or output space !
			gnash::log_error("%s", ss.str());
			break;
		}
		if (err == Z_DATA_ERROR)
		{
			std::ostringstream ss;
			ss << __FILE__ << ":" << __LINE__ << ": " << m_zstream.msg;
			throw ParserException(ss.str());
			break;
		}
		if (err == Z_MEM_ERROR)
		{
			std::ostringstream ss;
			ss << __FILE__ << ":" << __LINE__ << ": " << m_zstream.msg;
			throw ParserException(ss.str());
			break;
		}
		if (err != Z_OK)
		{
			// something's wrong.
			std::ostringstream ss;
			ss << __FILE__ << ":" << __LINE__ << ": " << m_zstream.msg;
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

void
InflaterIOChannel::go_to_end()
{
	if (m_error)
	{
		throw IOException("InflaterIOChannel is in error condition, can't seek to end");
	}

	// Keep reading until we can't read any more.

	unsigned char	temp[ZBUF_SIZE];

	// Seek forwards.
	for (;;)
	{
		int	bytes_read = inflate_from_stream(temp, ZBUF_SIZE);
		if (bytes_read == 0)
		{
			// We've seeked as far as we can.
			break;
		}
	}
}

int
InflaterIOChannel::seek(int pos)
{
	if (m_error)
	{
		gnash::log_debug("Inflater is in error condition");
		return TU_FILE_SEEK_ERROR;
		//return inf->m_logical_stream_pos;
	}

	// If we're seeking backwards, then restart from the beginning.
	if (pos < m_logical_stream_pos)
	{
		log_debug("inflater reset due to seek back from %d to %d", m_logical_stream_pos, pos );
		reset();
	}

	unsigned char	temp[ZBUF_SIZE];

	// Now seek forwards, by just reading data in blocks.
	while (m_logical_stream_pos < pos)
	{
		int	to_read = pos - m_logical_stream_pos;
		assert(to_read > 0);
		int	to_read_this_time = std::min<int>(to_read, ZBUF_SIZE);
		assert(to_read_this_time > 0);

		int	bytes_read = inflate_from_stream(temp, to_read_this_time);
		assert(bytes_read <= to_read_this_time);
		if (bytes_read == 0)
		{
			// Trouble; can't seek any further.
			gnash::log_debug("Trouble: can't seek any further.. ");
			return TU_FILE_SEEK_ERROR;
			break;
		}
	}

	assert(m_logical_stream_pos == pos);

	return 0; // m_logical_stream_pos;
}

InflaterIOChannel::InflaterIOChannel(std::auto_ptr<IOChannel> in)
	:
	m_in(in),
	m_initial_stream_pos(m_in->tell()),
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
		gnash::log_error("inflater_impl::ctor() inflateInit() returned %d", err);
		m_error = 1;
		return;
	}

	// Ready to go!
}



std::auto_ptr<IOChannel> make_inflater(std::auto_ptr<IOChannel> in)
{
	assert(in.get());
	return std::auto_ptr<IOChannel> (new InflaterIOChannel(in));
}


// @@ TODO
// IOChannel*	make_deflater(IOChannel* out) { ... }

}

#endif // HAVE_ZLIB_H

} // namespace gnash 

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
