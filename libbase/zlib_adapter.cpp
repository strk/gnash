// zlib_adapter.cpp    -- Thatcher Ulrich 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Code to wrap zlib compression/decompression around a IOChannel
// stream.


#include "zlib_adapter.h"

#include <algorithm>
#include <sstream>
#include <memory>

#include "IOChannel.h" // for inheritance
#include "log.h"
#include "GnashException.h"

namespace gnash {

#ifndef HAVE_ZLIB_H


// Stubs, in case client doesn't want to link to zlib.
namespace zlib_adapter
{
    std::auto_ptr<IOChannel> make_inflater(std::auto_ptr<IOChannel> /*in*/) {
        std::abort(); 
    }
}

#else // HAVE_ZLIB_H

extern "C" {
# include <zlib.h>
}

namespace zlib_adapter {

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
    virtual bool seek(std::streampos pos);

    // See dox in IOChannel
    virtual std::streamsize read(void* dst, std::streamsize bytes) {
        if (m_error) return 0;
        return inflate_from_stream(dst, bytes);
    }

    // See dox in IOChannel
    virtual void go_to_end();

    // See dox in IOChannel
    virtual std::streampos tell() const {
        return m_logical_stream_pos;
    }

    // See dox in IOChannel
    virtual bool eof() const {
        return m_at_eof;
    }

    // See dox in IOChannel
    virtual bool bad() const {
        return m_error;
    }

private:

    static const int ZBUF_SIZE = 4096;

    std::auto_ptr<IOChannel> m_in;

    // position of the input stream where we started inflating.
    std::streampos m_initial_stream_pos;
    
    unsigned char m_rawdata[ZBUF_SIZE];
    
    z_stream m_zstream;

    // current stream position of uncompressed data.
    std::streampos m_logical_stream_pos;

    bool m_at_eof;
    bool m_error;

    /// Discard current results and rewind to the beginning.
    //
    //
    /// Necessary in order to seek backwards.
    ///
    /// might throw a ParserException if unable to reset the uderlying
    /// stream to original position.
    ///
    void reset();

    std::streamsize inflate_from_stream(void* dst, std::streamsize bytes);

    // If we have unused bytes in our input buffer, rewind
    // to before they started.
    void rewind_unused_bytes();

};

const int InflaterIOChannel::ZBUF_SIZE;

void
InflaterIOChannel::rewind_unused_bytes()
{
    if (m_zstream.avail_in > 0) {
        const int pos = m_in->tell();
        const int rewound_pos = pos - m_zstream.avail_in;
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
    const int err = inflateReset(&m_zstream);
    if (err != Z_OK) {
	    log_error(_("inflater_impl::reset() inflateReset() returned %d"),
		      err);
        m_error = 1;
        return;
    }

    m_zstream.next_in = 0;
    m_zstream.avail_in = 0;

    m_zstream.next_out = 0;
    m_zstream.avail_out = 0;

    // Rewind the underlying stream.
    if (!m_in->seek(m_initial_stream_pos))
    {
        std::stringstream ss;
        ss << "inflater_impl::reset: unable to seek underlying "
            "stream to position " <<  m_initial_stream_pos;
        throw ParserException(ss.str());
    }

    m_logical_stream_pos = m_initial_stream_pos;
}

std::streamsize
InflaterIOChannel::inflate_from_stream(void* dst, std::streamsize bytes)
{

    assert(bytes);

    if (m_error) return 0;

    m_zstream.next_out = static_cast<unsigned char*>(dst);
    m_zstream.avail_out = bytes;

    for (;;) {
        if (m_zstream.avail_in == 0) {
            // Get more raw data.
            const int new_bytes = m_in->read(m_rawdata, ZBUF_SIZE);
            if (new_bytes == 0) {
                // The cupboard is bare!  We have nothing to feed to inflate().
                break;
            }
            else {
                m_zstream.next_in = m_rawdata;
                m_zstream.avail_in = new_bytes;
            }
        }

        const int err = inflate(&m_zstream, Z_SYNC_FLUSH);
        if (err == Z_STREAM_END) {
            m_at_eof = true;
            break;
        }
        if (err == Z_BUF_ERROR) {
            std::ostringstream ss;
            ss << __FILE__ << ":" << __LINE__ << ": " << m_zstream.msg;
            log_error("%s", ss.str());
            break;
        }
        if (err == Z_DATA_ERROR) {
            std::ostringstream ss;
            ss << __FILE__ << ":" << __LINE__ << ": " << m_zstream.msg;
            throw ParserException(ss.str());
            break;
        }
        if (err == Z_MEM_ERROR) {
            std::ostringstream ss;
            ss << __FILE__ << ":" << __LINE__ << ": " << m_zstream.msg;
            throw ParserException(ss.str());
            break;
        }
        if (err != Z_OK) {
            // something's wrong.
            std::ostringstream ss;
            ss << __FILE__ << ":" << __LINE__ << ": " << m_zstream.msg;
            throw ParserException(ss.str());
            break;
        }

        if (m_zstream.avail_out == 0) {
            break;
        }
    }

    if (m_error) return 0;

    const int bytes_read = bytes - m_zstream.avail_out;
    m_logical_stream_pos += bytes_read;

    return bytes_read;
}

void
InflaterIOChannel::go_to_end()
{
    if (m_error) {
        throw IOException("InflaterIOChannel is in error condition, "
                "can't seek to end");
    }

    // Keep reading until we can't read any more.

    unsigned char temp[ZBUF_SIZE];

    // Seek forwards.
    for (;;) {
        const std::streamsize bytes_read = inflate_from_stream(temp, ZBUF_SIZE);
        if (!bytes_read) {
            // We've seeked as far as we can.
            break;
        }
    }
}

bool
InflaterIOChannel::seek(std::streampos pos)
{
    if (m_error) {
	    log_debug(_("Inflater is in error condition"));
        return false;
    }

    // If we're seeking backwards, then restart from the beginning.
    if (pos < m_logical_stream_pos) {
	    log_debug(_("inflater reset due to seek back from %d to %d"),
                m_logical_stream_pos, pos );
        reset();
    }

    unsigned char temp[ZBUF_SIZE];

    // Now seek forwards, by just reading data in blocks.
    while (m_logical_stream_pos < pos) {
        std::streamsize to_read = pos - m_logical_stream_pos;
        assert(to_read > 0);

        std::streamsize readNow = std::min<std::streamsize>(to_read, ZBUF_SIZE);
        assert(readNow > 0);

        std::streamsize bytes_read = inflate_from_stream(temp, readNow);
        assert(bytes_read <= readNow);
        if (bytes_read == 0) {
		log_debug(_("Trouble: can't seek any further.. "));
            return false;
        }
    }

    assert(m_logical_stream_pos == pos);

    return true; 
}

InflaterIOChannel::InflaterIOChannel(std::auto_ptr<IOChannel> in)
    :
    m_in(in),
    m_initial_stream_pos(m_in->tell()),
    m_zstream(),
    m_logical_stream_pos(m_initial_stream_pos),
    m_at_eof(false),
    m_error(0)
{
    assert(m_in.get());

    const int err = inflateInit(&m_zstream);
    if (err != Z_OK) {
	    log_error(_("inflateInit() returned %d"), err);
        m_error = 1;
        return;
    }
}

std::auto_ptr<IOChannel> make_inflater(std::auto_ptr<IOChannel> in)
{
    assert(in.get());
    return std::auto_ptr<IOChannel>(new InflaterIOChannel(in));
}

}

#endif // HAVE_ZLIB_H

} // namespace gnash 

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
