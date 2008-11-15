// tu_file.cpp	-- Ignacio Castaño, Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A file class that can be customized with callbacks.

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
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

namespace gnash {


int
tu_file::read(void* dst, int bytes) 
// Return the number of bytes actually read.  EOF or an error would
// cause that to not be equal to "bytes".
{
//    GNASH_REPORT_FUNCTION;
    
    assert(dst);
    return fread( dst, 1, bytes, static_cast<FILE*>(m_data) );
}

int
tu_file::write(const void* src, int bytes)
// Return the number of bytes actually written.
{
    assert(src);
    return std::fwrite( src, 1, bytes, static_cast<FILE*>(m_data));
}

int
tu_file::seek(int pos)
{

    // TODO: optimize this by caching total stream size ?
    if (pos > size())
    {
	    return TU_FILE_SEEK_ERROR;
    }

    FILE* file = static_cast<FILE*>(m_data);

    clearerr(file); // make sure EOF flag is cleared.
    int	result = fseek(file, pos, SEEK_SET);
    if (result == EOF) {
	// @@ TODO should set m_error to something relevant based on errno.
	return TU_FILE_SEEK_ERROR;
    }

    assert ( ftell(file) == pos );

    return 0;
}

void
tu_file::go_to_end()
// Return 0 on success, TU_FILE_SEEK_ERROR on failure.
{

    int	result = fseek(static_cast<FILE*>(m_data), 0, SEEK_END);
    if (result == EOF) {
	    // Can't do anything here
    }

}

int
tu_file::tell() const
// Return the file position, or -1 on failure.
{
    FILE* f = static_cast<FILE*>(m_data);

    //if ( feof(f) )
    //assert ( ! feof(f) ); // I guess it's legal to call tell() while at eof.

    int ret = ftell(f);
    assert(ret <= size());
    return ret;
}

bool
tu_file::eof() const
// Return true if we're at EOF.
{
    if (feof((FILE*) m_data)) {
	return true;
    } else {
	return false;
    }
}

int
tu_file::get_error() const
// Return true if we're at EOF.
{
    if ( ! m_data ) return TU_FILE_OPEN_ERROR;
    return (ferror((FILE*) m_data));
}

int
tu_file::size() const
// Return -1 on failure, or the size
{
    assert(m_data);

    FILE* f = static_cast<FILE*>(m_data);

    struct stat statbuf;
    if ( -1 == fstat(fileno(f), &statbuf) )
    {
	    log_error("Could not fstat file");
	    return 0;
    }
    return statbuf.st_size;
}


void
tu_file::close()
// Return 0 on success, or TU_FILE_CLOSE_ERROR on failure.
{
    assert(m_data);
    int	result = std::fclose(static_cast<FILE*>(m_data));
    if (result == EOF) {
	    // @@ TODO should set m_error to something relevant based on errno.
    }
}


//// Create a file from a standard file pointer.
tu_file::tu_file(FILE* fp, bool autoclose=false) :
    m_data(fp),
    _autoclose(autoclose)
{
    //GNASH_REPORT_FUNCTION;
}


tu_file::~tu_file()
// Close this file when destroyed.
{
    if (_autoclose) close();
}


} // end namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
