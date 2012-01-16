// GnashFileUtilities.h     File handling for Gnash
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

/// This file should also be included for the following system functions:
//
/// stat()
/// fstat()
/// lstat()
/// dup()
/// readdir()
#ifndef GNASH_FILE_UTILITIES_H
#define GNASH_FILE_UTILITIES_H

#include "dsodefs.h"

#if !defined(_MSC_VER)
# include <unistd.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <dirent.h>
# include <cerrno>
#else
#include <io.h>
#define dup _dup
#endif

#include <string>

namespace gnash {

    /// Create a directory, granting owner rwx permissions.
    //
    /// On non-POSIX systems, just create the directory.
    /// 
    /// @param dirname
    ///        Directory name, may be absolute or relative to CWD
    ///
    inline int mkdirUserPermissions(const std::string& dirname)
    {
#if !defined(_WIN32) && !defined(_MSC_VER) && !defined(__amigaos4__)
        return mkdir(dirname.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
#elif defined(__amigaos4__)
      // on AmigaOS4 if you try to create a directory that is an assign or a drive
      // you will receive an EINVAL or an ENOTDIR instead of EEXIST and so will force it
      int ret = 0;
      ret =  mkdir(dirname.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
      if (errno == EINVAL || errno == ENOTDIR)
          errno = EEXIST;
      return ret;
#else
        return mkdir(dirname.c_str());
#endif
    }

    /// Create a directory for a given filename.
    //
    /// Everything after the last '/' is assumed to be the filename.
    ///
    /// @param filename
    ///        Full file path, may be absolute or relative to CWD
    ///
    DSOEXPORT bool mkdirRecursive(const std::string& filename);

} // namespace gnash

#endif
