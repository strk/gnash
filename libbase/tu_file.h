// tu_file.h	-- Ignacio Castaño, Thatcher Ulrich 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A very generic file class that can be customized with callbacks.


#ifndef TU_FILE_H
#define TU_FILE_H

#include <cstdio>
#include <memory>
#include "dsodefs.h"

namespace gnash {
    class IOChannel;
}

namespace gnash {

/// \brief 
/// Creates an IOChannel wrapper around a C stream.
//
/// @param fp A C stream
///
/// @param close Whether the C stream should be automatically closed.
DSOEXPORT std::auto_ptr<IOChannel> makeFileChannel(FILE* fp, bool close);

/// \brief
/// Creates an IOChannel by opening the given file in the given mode.
//
/// @param filepath A path to a file in the local filesystem.
///
/// @param mode The mode the file should be opened in, such as "rb" "w+b" (see
/// std::fopen)
///
/// @return An IOChannel or NULL if the file could not be opened.
DSOEXPORT std::auto_ptr<IOChannel> makeFileChannel(const char* filepath, const char* mode);

} // namespace gnash
#endif 

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
