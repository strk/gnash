// VaapiGlobalContext.h: VA API global context
// 
//   Copyright (C) 2009 Splitted-Desktop Systems
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

#ifndef GNASH_VAAPIGLOBALCONTEXT_H
#define GNASH_VAAPIGLOBALCONTEXT_H

#include "vaapi_common.h"
#include <vector>
#include "VaapiDisplay.h"

namespace gnash {

/// VA API global context
class DSOEXPORT VaapiGlobalContext {
    std::auto_ptr<VaapiDisplay>	_display;
    std::vector<VAProfile>	_profiles;
    std::vector<VAImageFormat>	_image_formats;

    bool init();

public:
    VaapiGlobalContext(std::auto_ptr<VaapiDisplay> display);
    ~VaapiGlobalContext();

    /// Get the unique global VA context
    //
    /// @return     The global VA context
    static VaapiGlobalContext *get();

    /// Check VA profile is supported
    bool hasProfile(VAProfile profile) const;

    /// Get the VA image format matching FOURCC
    //
    /// @return     The VA image format
    const VAImageFormat *getImageFormat(boost::uint32_t fourcc) const;

    /// Get the VA display
    //
    /// @return     The VA display
    VADisplay display() const
	{ return _display->get(); }
};

} // gnash namespace

#endif /* GNASH_VAAPIGLOBALCONTEXT_H */
