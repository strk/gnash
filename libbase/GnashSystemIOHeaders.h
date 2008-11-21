// Compatibility system headers for Gnash
//
// Include this file for read() and write()

#ifndef GNASH_IO_HEADERS_H
#define GNASH_IO_HEADERS_H

/// Always include unistd.h unless compiling under MSVC++
//
/// This isn't always necessary on GNU/Linux under gcc, but it's still good
/// practice to include it.
//
/// @todo A configure 'test' (a #define in gnashconfig.h) might be a better
///       way of checking for compiler.

#include "gnashconfig.h"
#if !defined(_MSC_VER)
# include <unistd.h>
#else
#include <io.h>
#endif

#endif
