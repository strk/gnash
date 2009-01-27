// Compatibility system filesystem headers for Gnash
//
// Include this file for stat(), fstat(), lstat(), dup()

#ifndef GNASH_FILE_UTILITIES_H
#define GNASH_FILE_UTILITIES_H

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

#include <string>

namespace gnash {

    /// Create a directory, granting owner rwx permissions.
    //
    /// On non-POSIX systems, just create the directory.
    inline int mkdirUserPermissions(const std::string& dirname)
    {
#if !defined(_WIN32) && !defined(_MSC_VER)
        return mkdir(dirname.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
#else
        return mkdir(dirname.c_str());
#endif
    }

    /// Create a directory for a given filename.
    //
    /// Everything after the last '/' is assumed to be the filename.
    bool mkdirRecursive(const std::string& filename);

} // namespace gnash

#endif
