// MediaParserGst.h: gstreamer media parsers, for Gnash
// 
//   Copyright (C) 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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

#ifndef GNASH_MEDIAPARSER_GST_H
#define GNASH_MEDIAPARSER_GST_H

#include <deque>
#include <memory>
#include <queue>
#include <gst/gst.h>
#include <boost/optional.hpp>

#include "MediaParser.h" // for inheritance
#include "ClockTime.h"
#include "Id3Info.h"

// Forward declaration
namespace gnash {
	class IOChannel;
}

namespace gnash {
namespace media {
namespace gst {

/// Class to hold extra info found in any stream by the parser.
struct ExtraInfoGst : public AudioInfo::ExtraInfo, VideoInfo::ExtraInfo,
	boost::noncopyable
{
    ExtraInfoGst(GstCaps* gstcaps)
    :
        caps(gstcaps)
    {
        gst_caps_ref(caps);
    }

    ~ExtraInfoGst()
    {
        gst_caps_unref(caps);
    }

    GstCaps* caps;
};

/// Class to hold GstBuffer. Takes ownership.
struct EncodedExtraGstData : public EncodedExtraData, boost::noncopyable
{
    EncodedExtraGstData(GstBuffer* buf)
    : buffer(buf)
    {
        gst_buffer_ref(buffer);
    }
    ~EncodedExtraGstData()
    {
        gst_buffer_unref(buffer);
    }

    GstBuffer* buffer;
};


/// Simple timer used for probe timeout (deprecated)
//
/// @deprecated probe-on-construction requirement for MediaParser was drop,
///             so no use for this class anymore
///
class SimpleTimer : public boost::noncopyable
{
public:
    SimpleTimer()
        : _start_time(clocktime::getTicks())
    {
    }
    
    bool expired() const
    {
        return (clocktime::getTicks() - _start_time) > 1000;
    }

private:
    std::uint64_t _start_time;
};



/// Gstreamer based MediaParser
class MediaParserGst: public MediaParser
{
public:

    /// Construct a Gstreamer-based media parser for given stream
    //
    /// Can throw a MediaException if input format couldn't be detected
    ///
    MediaParserGst(std::unique_ptr<IOChannel> stream);

    ~MediaParserGst();

    // See dox in MediaParser.h
    bool seek(std::uint32_t&);

    // See dox in MediaParser.h
    bool parseNextChunk();

    // See dox in MediaParser.h
    virtual std::uint64_t getBytesLoaded() const;

    virtual boost::optional<Id3Info> getId3Info() const;

    void rememberAudioFrame(EncodedAudioFrame* frame);
    void rememberVideoFrame(EncodedVideoFrame* frame);

private:
    void link_to_fakesink(GstPad* pad);

    static void cb_typefound (GstElement *typefind, guint probability,
                              GstCaps *caps, gpointer data);
                              
    static void cb_pad_added(GstElement* element,
        GstPad* new_pad, gpointer user_data);
    static void cb_no_more_pads (GstElement* element, gpointer data);

    static GstFlowReturn cb_chain_func_audio (GstPad *pad, GstBuffer *buffer);
    static GstFlowReturn cb_chain_func_video (GstPad *pad, GstBuffer *buffer);

    bool pushGstBuffer();
    bool emitEncodedFrames();


    GstElement* _bin;
    GstPad* _srcpad;
    GstPad* _audiosink;
    GstPad* _videosink;
    
    bool _demux_probe_ended;

    std::deque<EncodedAudioFrame*> _enc_audio_frames;
    std::deque<EncodedVideoFrame*> _enc_video_frames;
};


} // gnash.media.gst namespace
} // gnash.media namespace 
} // namespace gnash

#endif // __MEDIAPARSER_GST_H__
