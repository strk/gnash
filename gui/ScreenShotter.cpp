// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include "ScreenShotter.h"

#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <cstdio>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>

#include "log.h"
#include "GnashEnums.h"
#include "IOChannel.h"
#include "Renderer.h"
#include "tu_file.h"

namespace gnash {

/// Guess an appropriate file type from the filename.
//
/// If none can be guessed, we use png.
FileType
typeFromFileName(const std::string& filename)
{
    struct { const char* ext; FileType type; } matches[] =
        {
            { ".png", GNASH_FILETYPE_PNG },
            { ".jpg", GNASH_FILETYPE_JPEG },
            { ".jpeg", GNASH_FILETYPE_JPEG }
        };
    
    for (size_t i = 0; i < 3; ++i) {
        const char* ext = matches[i].ext;
        const std::string::size_type pos = filename.rfind(ext);
        if (pos != std::string::npos &&
                pos + std::strlen(ext) == filename.size()) {
            return matches[i].type;
        }
    }
    return GNASH_FILETYPE_PNG;
}

ScreenShotter::ScreenShotter(const std::string& fileName, int quality)
    :
    _immediate(false),
    _fileName(fileName),
    _last(false),
    _type(typeFromFileName(fileName)),
    _quality(quality)
{
}

ScreenShotter::ScreenShotter(const std::string& fileName, FileType type,
        int quality)
    :
    _immediate(false),
    _fileName(fileName),
    _last(false),
    _type(type),
    _quality(quality)
{
}

ScreenShotter::~ScreenShotter()
{
}

void
ScreenShotter::saveImage(const Renderer& r, const std::string& id) const
{
    // Replace all "%f" in the filename with the frameAdvance.
    std::string outfile(_fileName);
    boost::replace_all(outfile, "%f", id);
    
    FILE* f = std::fopen(outfile.c_str(), "wb");
    if (f) {
        std::unique_ptr<IOChannel> t(makeFileChannel(f, true));
        r.renderToImage(std::move(t), _type, _quality);
    }
    else {
        log_error(_("Failed to open screenshot file \"%s\"!"), outfile);
    }
}

void
ScreenShotter::setFrames(const FrameList& frames)
{
    _frames = frames;
    std::sort(_frames.begin(), _frames.end());
}

}

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
