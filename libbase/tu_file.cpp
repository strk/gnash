// tu_file.cpp	-- Ignacio Castaño, Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A file class that can be customized with callbacks.

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#ifdef HAVE_PTHREADS
#include <pthread.h>
#endif

#include "tu_file.h"
#include "utility.h"
#include "log.h"

#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

//
// tu_file functions using FILE
//

using namespace gnash;

namespace gnash {

static int std_read_func(void* dst, int bytes, void* appdata);
static int std_write_func(const void* src, int bytes, void* appdata);
static int std_seek_func(int pos, void *appdata);
static int std_seek_to_end_func(void *appdata);
static int std_tell_func(void *appdata);
static bool std_get_eof_func(void *appdata);
static int std_get_err_func(void *appdata);
static long std_get_stream_size_func(void *appdata);

static int
std_read_func(void* dst, int bytes, void* appdata) 
// Return the number of bytes actually read.  EOF or an error would
// cause that to not be equal to "bytes".
{
//    GNASH_REPORT_FUNCTION;
    
    assert(appdata);
    assert(dst);
    return fread( dst, 1, bytes, static_cast<FILE*>(appdata) );
}

static int
std_write_func(const void* src, int bytes, void* appdata)
// Return the number of bytes actually written.
{
    assert(appdata);
    assert(src);
    return std::fwrite( src, 1, bytes, static_cast<FILE*>(appdata));
}

static int
std_seek_func(int pos, void *appdata)
{
    assert(appdata);

    // TODO: optimize this by caching total stream size ?
    if (pos > std_get_stream_size_func(appdata))
    {
	    return TU_FILE_SEEK_ERROR;
    }

    FILE* file = static_cast<FILE*>(appdata);

    clearerr(file); // make sure EOF flag is cleared.
    int	result = fseek(file, pos, SEEK_SET);
    if (result == EOF) {
	// @@ TODO should set m_error to something relevant based on errno.
	return TU_FILE_SEEK_ERROR;
    }

    assert ( ftell(file) == pos );

    return 0;
}

static int
std_seek_to_end_func(void *appdata)
// Return 0 on success, TU_FILE_SEEK_ERROR on failure.
{
    assert(appdata);
    int	result = fseek(static_cast<FILE*>(appdata), 0, SEEK_END);
    if (result == EOF) {
	// @@ TODO should set m_error to something relevant based on errno.
	return TU_FILE_SEEK_ERROR;
    }
    return 0;
}

static int
std_tell_func(void *appdata)
// Return the file position, or -1 on failure.
{
    assert(appdata);
    FILE* f = static_cast<FILE*>(appdata);

    //if ( feof(f) )
    //assert ( ! feof(f) ); // I guess it's legal to call tell() while at eof.

    int ret = ftell(f);
    assert(ret <= std_get_stream_size_func(appdata));
    return ret;
}

static bool
std_get_eof_func(void *appdata)
// Return true if we're at EOF.
{
    assert(appdata);
    if (feof((FILE*) appdata)) {
	return true;
    } else {
	return false;
    }
}

static int
std_get_err_func(void *appdata)
// Return true if we're at EOF.
{
    if ( ! appdata ) return TU_FILE_OPEN_ERROR;
    return (ferror((FILE*) appdata));
}

static long
std_get_stream_size_func(void *appdata)
// Return -1 on failure, or the size
{
    assert(appdata);

    FILE* f = static_cast<FILE*>(appdata);

    struct stat statbuf;
    if ( -1 == fstat(fileno(f), &statbuf) )
    {
	    log_error("Could not fstat file");
	    return 0;
    }
    return statbuf.st_size;
}


static int
std_close_func(void *appdata)
// Return 0 on success, or TU_FILE_CLOSE_ERROR on failure.
{
    assert(appdata);
    int	result = std::fclose(static_cast<FILE*>(appdata));
    if (result == EOF) {
	// @@ TODO should set m_error to something relevant based on errno.
	return TU_FILE_CLOSE_ERROR;
    }
    return 0;
}


} // end namespace gnash

//// Create a file from a standard file pointer.
tu_file::tu_file(FILE* fp, bool autoclose=false)
{
    //GNASH_REPORT_FUNCTION;

    m_data = static_cast<void*>(fp);
    m_read = std_read_func;
    m_write = std_write_func;
    m_seek = std_seek_func;
    m_seek_to_end = std_seek_to_end_func;
    m_tell = std_tell_func;
    m_get_eof = std_get_eof_func;
    m_get_err = std_get_err_func;
    m_get_stream_size = std_get_stream_size_func;
    m_close = autoclose ? std_close_func : NULL;
}

// Create a file from the given name and the given mode.
tu_file::tu_file(const char * name, const char * mode)
{
	GNASH_REPORT_RETURN;

	m_data = fopen(name, mode);
    
	m_read = std_read_func;
	m_write = std_write_func;
	m_seek = std_seek_func;
	m_seek_to_end = std_seek_to_end_func;
	m_tell = std_tell_func;
	m_get_eof = std_get_eof_func;
	m_get_err = std_get_err_func;
	m_get_stream_size = std_get_stream_size_func;
	m_close = std_close_func;
}

tu_file::~tu_file()
// Close this file when destroyed.
{
    close();
}

void
tu_file::close() 
// Close this file.
{ 
    if (m_close && m_data) {
	m_close(m_data);
    }
    m_data = NULL; 
    m_read = NULL; 
    m_write = NULL; 
    m_seek = NULL; 
    m_tell = NULL; 
    m_close = NULL; 
}


void
tu_file::copy_from(tu_file* src)
// Copy remaining contents of *src into *this.
{
    // @@ bah, should buffer this!
    while (src->eof() == false) {
	boost::uint8_t	b = src->read8();
	if (src->get_error()) {
	    break;
	}
	
	write8(b);
    }
}


int
tu_file::copy_bytes(tu_file* src, int byte_count)
// Copy a fixed number of bytes from *src into *this.  Return the
// number of bytes copied.
{
    static const int	BUFSIZE = 4096;
    char	buffer[BUFSIZE];
    
    int	bytes_left = byte_count;
    while (bytes_left) {
	int	to_copy = std::min<int>(bytes_left, BUFSIZE);
	
	int	read_count = src->read(buffer, to_copy);
	int	write_count = write(buffer, read_count);
	
	assert(write_count <= read_count);
	assert(read_count <= to_copy);
	assert(to_copy <= bytes_left);
	
	bytes_left -= write_count;
	if (write_count < to_copy) {
	    // Some kind of error; abort.
	    return byte_count - bytes_left;
	}
    }
    
    assert(bytes_left == 0);
    
    return byte_count;
}

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
