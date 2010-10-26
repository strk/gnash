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

#ifndef GNASH_GUI_SCREENSHOTTER_H
#define GNASH_GUI_SCREENSHOTTER_H

#include "Renderer.h" 

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

namespace gnash {

/// Handles screen dumps.
class ScreenShotter
{
public:

    typedef std::vector<size_t> FrameList;

    /// Create a ScreenShotter with renderer and output name.
    ScreenShotter(boost::shared_ptr<Renderer> r, const std::string& fileName)
        :
        _renderer(r),
        _immediate(false),
        _fileName(fileName),
        _last(false)
    {}

    /// Take a screenshot at the next possible moment.
    void now() {
        _immediate = true;
    }

    /// Take a screenshot when the last frame is reached.
    void lastFrame() {
        _last = true;
    }

    /// Called on the last frame before exit.
    //
    /// Which frame is last depends on the execution path of the SWF, whether
    /// the SWF loops, whether a timeout was requested or a maximum number of
    /// advances set. Those conditions are not knowable in advance, so
    /// the last frame is a special case.
    void last() const;

    /// Takes a screenshot if required.
    //
    /// Called on each advance.
    //
    /// @param frameAdvance     used to check whether a screenshot is required
    ///                         as well as to construct the filename.
    void screenShot(size_t frameAdvance);

    /// Request a list of frames to be rendered to image files.
    void setFrames(const FrameList& frames);

private:

    /// Take the screenshot.
    void saveImage(const std::string& filename) const;

    boost::shared_ptr<Renderer> _renderer;

    /// If true, the next call to screenshot will take a screenshot
    bool _immediate;

    /// Name used to generate output file.
    const std::string _fileName;

    /// Whether to take a screenshot on the last frame.
    bool _last;

    FrameList _frames;

};

} // end of gnash namespace

// end of GNASH_GUI_SCREENSHOTTER_H
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
