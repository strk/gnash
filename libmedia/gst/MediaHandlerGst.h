// MediaHandlerGst.h: GST media handler, for Gnash
// 
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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


#ifndef __MEDIAHANDLERGST_H__
#define __MEDIAHANDLERGST_H__

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "MediaHandler.h" // for inheritance

#include <memory>

namespace gnash {
namespace media {

class MediaHandlerGst : public MediaHandler
{
public:

	virtual std::auto_ptr<MediaParser> createMediaParser(std::auto_ptr<tu_file> stream);

	virtual std::auto_ptr<VideoDecoder> createVideoDecoder(videoCodecType format, int width, int height);
};


} // gnash.media namespace 
} // namespace gnash

#endif // __MEDIAHANDLERGST_H__
