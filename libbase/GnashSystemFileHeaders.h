// Compatibility system filesystem headers for Gnash
//
// Include this file for stat(), fstat(), lstat(), dup()

#ifndef GNASH_FILE_HEADERS_H
#define GNASH_FILE_HEADERS_H

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
# include <sys/stat.h>
# include <sys/types.h>
#else
#include <io.h>
#define dup _dup
#endif

#endif
