// ScreenShotter.cpp:  Handles screen dumps
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#include "ScreenShotter.h"

#include <cstdio>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>

#include "tu_file.h" 
#include "IOChannel.h"

namespace gnash {

void
ScreenShotter::saveImage(const std::string& id) const
{
    // Replace all "%f" in the filename with the frameAdvance.
    std::string outfile(_fileName);
    boost::replace_all(outfile, "%f", id);
    
    FILE* f = std::fopen(outfile.c_str(), "wb");
    if (f) {
        boost::shared_ptr<IOChannel> t(makeFileChannel(f, true).release());
        _renderer->renderToImage(t, GNASH_FILETYPE_PNG);
    } else {
        log_error("Failed to open screenshot file \"%s\"!", outfile);
    }
}

void
ScreenShotter::screenShot(size_t frameAdvance)
{
    // Save an image if an spontaneous screenshot was requested or the
    // frame is in the list of requested frames.
    if (_immediate || std::binary_search(_frames.begin(), _frames.end(),
                frameAdvance)) {
        saveImage(boost::lexical_cast<std::string>(frameAdvance));
        _immediate = false;
    }
}

void
ScreenShotter::last() const
{
    if (_last) {
	saveImage("last");
    }
}

void
ScreenShotter::setFrames(const FrameList& frames)
{
    _frames = frames;
    std::sort(_frames.begin(), _frames.end());
}

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: nil
