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

#ifndef GNASH_VIDEO_H
#define GNASH_VIDEO_H

#include <boost/intrusive_ptr.hpp>
#include "DisplayObject.h"

// Forward declarations
namespace gnash {
	class NetStream_as;
    class as_object;
    namespace image {
        class GnashImage;
    }
    struct ObjectURI;
    namespace SWF {
        class DefineVideoStreamTag;
    }
    namespace media {
        class VideoDecoder;
    }
}

namespace gnash {

/// VideoStream ActionScript object
//
/// A VideoStream provides audio/video frames either
/// embedded into the SWF itself or loaded from the
/// network using an associated NetStream object.
///
class Video : public DisplayObject
{
public:
	
	Video(as_object* object, const SWF::DefineVideoStreamTag* def,
            DisplayObject* parent);

	~Video();

	virtual bool pointInShape(std::int32_t x, std::int32_t y) const
	{
		// video DisplayObject shape is always a rectangle..
		return pointInBounds(x, y);
	}

	virtual SWFRect getBounds() const;

	/// Register this video instance as a live DisplayObject
	virtual void construct(as_object* init = nullptr);

	virtual void display(Renderer& renderer, const Transform& xform);

	void add_invalidated_bounds(InvalidatedRanges& ranges, bool force);

	/// Set the input stream for this video
	void setStream(NetStream_as* ns);

    void clear();

    /// Get the height of the video.
    //
    /// The method depends on whether it is an embedded or a live
    /// stream. This returns 0 until the height is known, which for
    /// FLV streams is only after decoding. The value may possibly
    /// vary during playback.
    int height() const;

    /// Get the width of the video.
    //
    /// The method depends on whether it is an embedded or a live
    /// stream. This returns 0 until the height is known, which for
    /// FLV streams is only after decoding. The value may possibly
    /// vary during playback.
    int width() const;

    /// Whether this Video object should request smoothing when scaled.
    bool smoothing() const { return _smoothing; }

    /// Set whether smoothing is required.
    void setSmoothing(bool b) { _smoothing = b; }

protected:
    
	/// Mark video-specific reachable resources.
	//
	/// video-specific reachable resources are:
	///	- Associated NetStream if any (_ns) 
	///
	virtual void markOwnResources() const;

private:

	/// Get video frame to be displayed
    image::GnashImage* getVideoFrame();

	const boost::intrusive_ptr<const SWF::DefineVideoStreamTag> m_def;

    // Who owns this ? Should it be an intrusive ptr ?
	NetStream_as* _ns;

	/// Playing an embbeded video stream ?
	bool _embeddedStream;

	/// Last decoded frame number
	std::int32_t _lastDecodedVideoFrameNum;

	/// Last decoded frame 
	std::unique_ptr<image::GnashImage> _lastDecodedVideoFrame;

    /// The decoder used to decode the video frames for embedded streams
    //
    /// For dynamically loaded videos NetStream takes care of decoding
    /// see the _ns property
    ///
    std::unique_ptr<media::VideoDecoder> _decoder;

    /// Whether to request smoothing when the video is scaled
    bool _smoothing;
};

} // namespace gnash

#endif 
