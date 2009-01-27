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
#include "GnashFileUtilities.h"

#include <string>
#include <boost/tokenizer.hpp>
#include <cerrno>

namespace gnash {

/// Create a directory for a given filename.
//
/// Everything after the last '/' is assumed to be the filename.
bool
mkdirRecursive(const std::string& filename)
{
    // Not a directory, nothing to do.
    std::string::size_type pos = filename.rfind("/");
    if (pos == std::string::npos) {
        return true;
    }
    const std::string& target = filename.substr(0, pos);
    
    typedef boost::tokenizer<boost::char_separator<char> > Tok;
    boost::char_separator<char> sep("/");
    Tok t(target, sep);
    std::string newdir = "/";

    for (Tok::iterator tit = t.begin(); tit != t.end(); ++tit) {

        newdir += *tit;

        if (newdir.find("..") != std::string::npos) {
            return false;
        }

        int ret = mkdirUserPermissions(newdir);
        
        if ((errno != EEXIST) && (ret != 0)) {
            return false;
        }
        newdir.push_back('/');
    }
    return true;
}

} // namespace gnash

#endif
