// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifndef GNASH_VIDEO_STREAM_INSTANCE_H
#define GNASH_VIDEO_STREAM_INSTANCE_H

#include "character.h" // for inheritance

// Forward declarations
namespace gnash {
	class NetStream_as;
    class GnashImage;
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
class Video : public character
{

public:

	boost::intrusive_ptr<SWF::DefineVideoStreamTag> m_def;
	
	Video(SWF::DefineVideoStreamTag* def, character* parent,
            int id);

	~Video();

	virtual bool pointInShape(boost::int32_t x, boost::int32_t y) const
	{
		// video character shape is always a rectangle..
		return pointInBounds(x, y);
	}

	rect getBounds() const;

	/// We use the call to ::advance to properly set invalidated status
	virtual void advance();

	/// Register this video instance as a live character
	virtual void stagePlacementCallback(as_object* initObj = 0);

	void display();

	// For sure isActionScriptReferenceable...
	bool wantsInstanceName() const
	{
		return true; // text fields can be referenced 
	}	

	void add_invalidated_bounds(InvalidatedRanges& ranges, bool force);

	/// Set the input stream for this video
	void setStream(boost::intrusive_ptr<NetStream_as> ns);

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

#ifdef GNASH_USE_GC
	/// Mark video-specific reachable resources and invoke
	/// the parent's class version (markCharacterReachable)
	//
	/// video-specific reachable resources are:
	///	- Associated NetStream if any (_ns) 
	///
	virtual void markReachableResources() const;
#endif // GNASH_USE_GC

private:

	/// Initialize decoder for embedded video 
	//
	/// Call only if given a non-null video definition.
	///
	void initializeDecoder();

	/// Get video frame to be displayed
	GnashImage* getVideoFrame();

	// Who owns this ? Should it be an intrusive ptr ?
	boost::intrusive_ptr<NetStream_as> _ns;

	/// Playing an embbeded video stream ?
	bool _embeddedStream;

	/// Last decoded frame number
	boost::int32_t _lastDecodedVideoFrameNum;

	/// Last decoded frame 
	std::auto_ptr<GnashImage> _lastDecodedVideoFrame;

	/// The decoder used to decode the video frames
	std::auto_ptr<media::VideoDecoder> _decoder;

    /// Whether to request smoothing when the video is scaled
    bool _smoothing;
};

void video_class_init(as_object& global);

}	// end namespace gnash


#endif // GNASH_VIDEO_STREAM_INSTANCE_H
