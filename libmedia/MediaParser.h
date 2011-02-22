// MediaParser.h: Base class for media parsers
// 
//   Copyright (C) 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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

#ifndef GNASH_MEDIAPARSER_H
#define GNASH_MEDIAPARSER_H

#include "IOChannel.h" // for inlines
#include "dsodefs.h" // DSOEXPORT

#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/barrier.hpp>
#include <memory>
#include <deque>
#include <map>
#include <vector>
#include <iosfwd> // for output operator forward declarations

// Undefine this to load/parse media files in main thread
#define LOAD_MEDIA_IN_A_SEPARATE_THREAD 1

namespace gnash {
    class SimpleBuffer;
}

namespace gnash {
namespace media {


/// Video frame types
enum videoFrameType
{
	/// Key frames
	KEY_FRAME = 1,

	/// Interlaced frames
	INTER_FRAME = 2,

	/// Disposable interlaced frames
	DIS_INTER_FRAME = 3
};

/// The type of the codec id passed in the AudioInfo or VideoInfo class
enum codecType
{
	/// The internal flash codec ids
	CODEC_TYPE_FLASH,

	/// Custom codecs ids
	CODEC_TYPE_CUSTOM
};

/// Video codec ids as defined in flash
enum videoCodecType
{
	/// H263/SVQ3 video codec
	VIDEO_CODEC_H263 = 2,

	/// Screenvideo codec
	VIDEO_CODEC_SCREENVIDEO = 3,

	/// On2 VP6 video codec
	VIDEO_CODEC_VP6 = 4,

	/// On2 VP6 Alpha video codec
	VIDEO_CODEC_VP6A = 5,

	/// Screenvideo2 codec
	VIDEO_CODEC_SCREENVIDEO2 = 6,

	/// MPEG-4 Part 10, or Advanced Video Coding
	VIDEO_CODEC_H264 = 7

	// NOTE: if you add more elements here remember to
	//       also add them to the output operator!
};

DSOEXPORT std::ostream& operator<< (std::ostream& os, const videoCodecType& t);

/// Audio codec ids as defined in flash
//
/// For some encodings, audio data is organized
/// in logical frames. The structure of such frames
/// (header/payload) is codec dependent.
/// The actual size of each frame may not be known
/// w/out parsing the encoded stream, as it
/// might be specified in the header of each frame.
///
/// Other encodings are loosier on frames. For these
/// you can define a frame any way you want, as long
/// as a frame doesn't contain partial samples.
///
/// For FFMPEG, you can NOT construct a parser for the
/// loosy-framed codecs.
///
/// Parser-needing codecs will be documented as such.
///
enum audioCodecType
{
	/// Signed Linear PCM, unspecified byte order 
   	//
   	/// Use of this codec is deprecated (but still supported) due to
   	/// the unspecified byte order (you can only play >8bit samples
   	/// in a sane way when the endiannes of encoding and decoding
   	/// hosts match).
   	///
   	/// 90% of the times the actual encoder did run on windows, so
   	/// it is a good bet to guess for little-endian.
   	/// SampleSize may be 8 or 16 bits.
    ///
	AUDIO_CODEC_RAW = 0,	

	/// ADPCM format
    //
	/// SWF support 2, 3, 4, and 5 bits / sample.
	/// ADPCM "frames" consits of 4096 ADPCM codes per channel.
	/// 
	/// For streaming there is no concept of "seekSamples" like
	/// MP3 streaming implements. Thus ADPCM ist suboptimal for
	/// streaming as it is difficult to match sound frames with
	/// movie frames.
   	/// Uncompressed SampleSize is always 16 bit.
    ///
	AUDIO_CODEC_ADPCM = 1,

	/// MP3 format
   	//
   	/// MP3 is supported for SWF4 and later. 
   	/// MP3 sound is structured in frames consiting of  a fixed sized 
   	/// header (32Bit) and compressed sound data. Compressed sound
   	/// data always contains a fixed number of sound samples (576 or 1152).
   	/// For streaming sound an additional field is necessary (seekSamples)
   	/// to keep track of samples exceeding movie frame border.
   	///
   	/// MP3 header contains all necessary information to decode a single
   	/// frame. From this information one can derive the number of samples 
   	/// and the frame's size.
   	/// Uncompressed SampleSize is always 16 bit.
    ///
	AUDIO_CODEC_MP3 = 2,

	/// Linear PCM, strictly little-endian
	AUDIO_CODEC_UNCOMPRESSED = 3,

	/// Proprietary simple format. Always 5Khz mono ?
    //
	/// SWF6 and later.
	/// Data is organized in frames of 256 samples.
    ///
	AUDIO_CODEC_NELLYMOSER_8HZ_MONO = 5,

	/// Proprietary simple format
    //
	/// SWF6 and later.
	/// Data is organized in frames of 256 samples.
    ///
	AUDIO_CODEC_NELLYMOSER = 6,

	/// Advanced Audio Coding
	AUDIO_CODEC_AAC = 10,

	/// Always 16kHz mono
	AUDIO_CODEC_SPEEX = 11

	// NOTE: if you add more elements here remember to
	//       also add them to the output operator!
};

DSOEXPORT std::ostream& operator<< (std::ostream& os, const audioCodecType& t);

/// Information about an audio stream 
//
/// The information stored is codec-id,
/// samplerate, samplesize, stereo, duration and codec-type.
///
/// Additionally, an abstract ExtraInfo can be hold.
///
class AudioInfo
{

public:

    /// Construct an AudioInfo object
    //
    /// @param codeci
    ///     Audio codec id.
    ///     To be interpreted as a media::audioCodecType if the typei
    ///     parameter is CODEC_TYPE_FLASH; otherwise it's an opaque number to use
    ///     for codec information transfer between a MediaParser and a
    ///     AudioDecoder from the same %media handler module.
    ///
    /// @param sampleRatei
    ///     Nominal sample rate.
    ///     @todo document units.
    ///
    /// @param sampleSizei
    ///     Sample size, in bytes.
    ///
    /// @param stereoi
    ///     Sample type (stereo if true, mono otherwise).
    ///     @todo document if and how intepretation of sampleSizei changes
    ///
    /// @param durationi
    ///     Nominal audio stream duration, in milliseconds.
    ///
    /// @param typei
    ///     Changes interpretation of the codeci parameter.
    ///
	AudioInfo(int codeci, boost::uint16_t sampleRatei,
            boost::uint16_t sampleSizei, bool stereoi,
            boost::uint64_t durationi, codecType typei)
		:
        codec(codeci),
		sampleRate(sampleRatei),
		sampleSize(sampleSizei),
		stereo(stereoi),
		duration(durationi),
		type(typei)
		{
		}

	/// Codec identifier
	//
	/// This has to be interpreted as audioCodecType if codecType type is CODEC_TYPE_FLASH
	/// or interpretation is opaque and we rely on the assumption that the AudioInfo
	/// creator and the AudioInfo user have a way to get a shared interpretation
	///
	int codec;

	boost::uint16_t sampleRate;

	/// Size of each sample, in bytes
	boost::uint16_t sampleSize;

	bool stereo;

	boost::uint64_t duration;

	codecType type;

	/// Extra info about an audio stream
    //
	/// Abstract class to hold any additional info
	/// when required for proper decoder initialization.
    ///
	class ExtraInfo {
	public:
		virtual ~ExtraInfo() {}
	};

	/// Extra info about audio stream, if when needed
    //
    /// Could be ExtraVideoInfoFlv or a media-handler specific info
    ///
	std::auto_ptr<ExtraInfo> extra;
};

/// Information about a video stream 
//
/// The information stored is codec-id, width, height, framerate and duration.
///
/// Additionally, an abstract ExtraInfo can be hold.
///
class VideoInfo
{
public:

    /// Construct a VideoInfo object
    //
    /// @param codeci
    ///     Video codec id.
    ///     To be interpreted as a media::videoCodecType if the typei
    ///     parameter is CODEC_TYPE_FLASH; otherwise it's an opaque number to use
    ///     for codec information transfer between a MediaParser and a
    ///     VideoDecoder from the same %media handler module.
    ///
    /// @param widthi
    ///     Video frame width.
    ///     @todo check if still needed.
    ///
    /// @param heighti
    ///     Video frame height.
    ///     @todo check if still needed.
    ///
    /// @param frameRatei
    ///     Nominal video frame rate.
    ///     @todo document units.
    ///
    /// @param durationi
    ///     Nominal video duration.
    ///     @todo check if still needed, if so document units!
    ///
    /// @param typei
    ///     Changes interpretation of the codeci parameter.
    ///     
	VideoInfo(int codeci, boost::uint16_t widthi, boost::uint16_t heighti,
            boost::uint16_t frameRatei, boost::uint64_t durationi,
            codecType typei)
		:
        codec(codeci),
		width(widthi),
		height(heighti),
		frameRate(frameRatei),
		duration(durationi),
		type(typei)
	{
	}

	int codec;
	boost::uint16_t width;
	boost::uint16_t height;
	boost::uint16_t frameRate;
	boost::uint64_t duration;
	codecType type;

	/// Extra info about a video stream
    //
	/// Abstract class to hold any additional info
	/// when required for proper decoder initialization
    ///
	class ExtraInfo {
	public:
		virtual ~ExtraInfo() {}
	};

	/// Extra info about video stream, if when needed
    //
    /// Could be ExtraAudioInfoFlv or a media-handler specific info
    ///
	std::auto_ptr<ExtraInfo> extra;
};

DSOEXPORT std::ostream& operator << (std::ostream& os, const VideoInfo& vi);


class EncodedExtraData {

public:
	virtual ~EncodedExtraData() {}

};

/// An encoded video frame
class EncodedVideoFrame
{
public:

	/// Create an encoded video frame
	//
	/// @param data
	///     Data buffer, ownership transferred
	///
	/// @param size
	///     Size of the data buffer
	///
	/// @param frameNum
	///     Frame number.
	///
	/// @param timestamp
	///     Presentation timestamp, in milliseconds.
	///
	EncodedVideoFrame(boost::uint8_t* data, boost::uint32_t size,
			unsigned int frameNum,
			boost::uint64_t timestamp=0)
		:
		_size(size),
		_data(data),
		_frameNum(frameNum),
		_timestamp(timestamp)
	{}

	/// Return pointer to actual data. Ownership retained by this class.
	const boost::uint8_t* data() const { return _data.get(); }

	/// Return size of data buffer.
	boost::uint32_t dataSize() const { return _size; }

	/// Return video frame presentation timestamp
	boost::uint64_t timestamp() const { return _timestamp; }

	/// Return video frame number
	unsigned frameNum() const { return _frameNum; }

	// FIXME: should have better encapsulation for this sort of stuff.
	std::auto_ptr<EncodedExtraData> extradata;
private:

	boost::uint32_t _size;
	boost::scoped_array<boost::uint8_t> _data;
	unsigned int _frameNum;
	boost::uint64_t _timestamp;
};

/// An encoded audio frame
class EncodedAudioFrame
{
public:
	boost::uint32_t dataSize;
	boost::scoped_array<boost::uint8_t> data;
	boost::uint64_t timestamp;

	// FIXME: should have better encapsulation for this sort of stuff.
	std::auto_ptr<EncodedExtraData> extradata;
};

/// The MediaParser class provides cursor-based access to encoded %media frames 
//
/// Cursor-based access allow seeking as close as possible to a specified time
/// and fetching frames from there on, sequentially.
/// See seek(), nextVideoFrame(), nextAudioFrame() 
///
/// Input is received from a IOChannel object.
///
class MediaParser
{
public:

    /// A container for executable MetaTags contained in media streams.
    //
    /// Presently only known in FLV.
    typedef std::multimap<boost::uint64_t, boost::shared_ptr<SimpleBuffer> >
        MetaTags;
    
    typedef std::vector<MetaTags::mapped_type> OrderedMetaTags;
        MediaParser(std::auto_ptr<IOChannel> stream);

	// Classes with virtual methods (virtual classes)
	// must have a virtual destructor, or the destructors
	// of subclasses will never be invoked, tipically resulting
	// in memory leaks..
	//
	virtual ~MediaParser();

	/// \brief
	/// Seeks to the closest possible position the given position,
	/// and returns the new position.
	//
	///
	/// @param time input/output parameter, input requests a time, output
	///        return the actual time seeked to.
	/// 
	/// @return true if the seek was valid, false otherwise.
	///
	virtual bool seek(boost::uint32_t& time)=0;

	/// Returns mininum length of available buffers in milliseconds
	//
	/// TODO: FIXME: NOTE: this is currently used by NetStream.bufferLength
	/// but is bogus as it doesn't take the *current* playhead cursor time
	/// into account. A proper way would be having a  getLastBufferTime ()
	/// interface here, returning minimun timestamp of last available 
	/// frames and let NetSTream::bufferLength() use that with playhead
	/// time to find out...
	///
	DSOEXPORT boost::uint64_t getBufferLength() const;

	/// Return the time we want the parser thread to maintain in the buffer
	DSOEXPORT boost::uint64_t getBufferTime() const
	{
		boost::mutex::scoped_lock lock(_bufferTimeMutex);
		return _bufferTime;
	}

	/// Set the time we want the parser thread to maintain in the buffer
	//
	/// @param t
	///	Number of milliseconds to keep in the buffers.
	///
	DSOEXPORT void setBufferTime(boost::uint64_t t)
	{
		boost::mutex::scoped_lock lock(_bufferTimeMutex);
		_bufferTime=t;
	}

	/// Get timestamp of the next frame available, if any
	//
	/// @param ts will be set to timestamp of next available frame
	/// @return false if no frame is available yet
	///
	DSOEXPORT bool nextFrameTimestamp(boost::uint64_t& ts) const;

	/// Get timestamp of the video frame which would be returned on nextVideoFrame
	//
	/// @return false if there no video frame left
	///         (either none or no more)
	///
	DSOEXPORT bool nextVideoFrameTimestamp(boost::uint64_t& ts) const;

	/// Returns the next video frame in the parsed buffer, advancing video cursor.
	//
	/// If no frame has been played before the first frame is returned.
	/// If there is no more frames in the parsed buffer NULL is returned.
	/// you can check with parsingCompleted() to know wheter this is due to 
	/// EOF reached.
	///
	DSOEXPORT std::auto_ptr<EncodedVideoFrame> nextVideoFrame();

	/// Get timestamp of the audio frame which would be returned on nextAudioFrame
	//
	/// @return false if there no video frame left
	///         (either none or no more)
	///
	DSOEXPORT bool nextAudioFrameTimestamp(boost::uint64_t& ts) const;

	/// Returns the next audio frame in the parsed buffer, advancing audio cursor.
	//
	/// If no frame has been played before the first frame is returned.
	/// If there is no more frames in the parsed buffer NULL is returned.
	/// you can check with parsingCompleted() to know wheter this is due to 
	/// EOF reached.
	///
	DSOEXPORT std::auto_ptr<EncodedAudioFrame> nextAudioFrame();

	/// Returns a VideoInfo class about the videostream
	//
	/// @return a VideoInfo class about the videostream,
	///         or zero if unknown (no video or not enough data parsed yet).
	///
	VideoInfo* getVideoInfo() { return _videoInfo.get(); }

	/// Returns a AudioInfo class about the audiostream
	//
	/// @return a AudioInfo class about the audiostream,
	///         or zero if unknown (no audio or not enough data parsed yet).
	///
	AudioInfo* getAudioInfo() { return _audioInfo.get(); }

	/// Return true of parsing is completed
	//
	/// If this function returns true, any call to nextVideoFrame()
	/// or nextAudioFrame() will always return NULL
	///
	/// TODO: make thread-safe
	///
	bool parsingCompleted() const { return _parsingComplete; }

	/// Return true of indexing is completed
	//
	/// If this function returns false, parseNextChunk will
	/// be called even when buffers are full. Parsers
	/// supporting indexing separated from parsing should 
	/// override this method and have parseNextChunk figure
	/// if they only need to index or to parse based on bufferFull.
	///
	virtual bool indexingCompleted() const { return true; }

	/// Return number of bytes parsed so far
	virtual boost::uint64_t getBytesLoaded() const { return 0; }

	/// Return total number of bytes in input
	boost::uint64_t getBytesTotal() const
	{
		return _stream->size();
	}

	/// Parse next chunk of input
	//
	/// The implementations are required to parse a small chunk
	/// of input, so to avoid blocking too much if parsing conditions
	/// change (ie: seek or destruction requested)
	///
	/// When LOAD_MEDIA_IN_A_SEPARATE_THREAD is defined, this should
	/// never be called by users (consider protected).
	///
	virtual bool parseNextChunk()=0;

    /// Retrieve any parsed metadata tags up to a specified timestamp.
    //
    /// @param ts   The latest timestamp to retrieve metadata for.
    /// @param tags This is filled with shared pointers to metatags in
    ///             timestamp order. Ownership of the data is shared. It
    ///             is destroyed automatically along with the last owner.
    //
    /// Metadata is currently only parsed from FLV streams. The default
    /// is a no-op.
    virtual void fetchMetaTags(OrderedMetaTags& tags, boost::uint64_t ts);

protected:

	/// Subclasses *must* set the following variables: @{ 

	/// Info about the video stream (if any)
	std::auto_ptr<VideoInfo> _videoInfo;

	/// Info about the audio stream (if any)
	std::auto_ptr<AudioInfo> _audioInfo;

	/// Whether the parsing is complete or not
	bool _parsingComplete;

	/// Number of bytes loaded
	boost::uint64_t _bytesLoaded;

	/// }@

	/// Start the parser thread
	void startParserThread();

	/// Stop the parser thread
	//
	/// This method should be always called
	/// by destructors of subclasses to ensure
	/// the parser thread won't attempt to access
	/// destroyed structures.
	///
	void stopParserThread();

	/// Clear the a/v buffers
	void clearBuffers();

	/// Push an encoded audio frame to buffer.
	//
	/// Will wait on a condition if buffer is full or parsing was completed
	///
	void pushEncodedAudioFrame(std::auto_ptr<EncodedAudioFrame> frame);

	/// Push an encoded video frame to buffer.
	//
	/// Will wait on a condition if buffer is full or parsing was completed
	///
	void pushEncodedVideoFrame(std::auto_ptr<EncodedVideoFrame> frame);

	/// Return pointer to next encoded video frame in buffer
	//
	/// If no video is present, or queue is empty, 0 is returned
	///
	const EncodedVideoFrame* peekNextVideoFrame() const;

	/// Return pointer to next encoded audio frame in buffer
	//
	/// If no video is present, or queue is empty, 0 is returned
	///
	const EncodedAudioFrame* peekNextAudioFrame() const;

	/// The stream used to access the file
	std::auto_ptr<IOChannel> _stream;
	mutable boost::mutex _streamMutex;

	static void parserLoopStarter(MediaParser* mp)
	{
		mp->parserLoop();
	}

	/// The parser loop runs in a separate thread
	/// and calls parseNextChunk until killed.
	///
	/// parseNextChunk is expected to push encoded frames
	/// on the queue, which may trigger the thread to be
	/// put to sleep when queues are full or parsing
	/// was completed.
	///
	void parserLoop();

	bool parserThreadKillRequested() const
	{
		boost::mutex::scoped_lock lock(_parserThreadKillRequestMutex);
		return _parserThreadKillRequested;
	}

	boost::uint64_t _bufferTime;
	mutable boost::mutex _bufferTimeMutex;

	std::auto_ptr<boost::thread> _parserThread;
	boost::barrier _parserThreadStartBarrier;
	mutable boost::mutex _parserThreadKillRequestMutex;
	bool _parserThreadKillRequested;
	boost::condition _parserThreadWakeup;

	/// Wait on the _parserThreadWakeup condition if buffer is full
	/// or parsing was completed.
	/// 
	/// Callers *must* pass a locked lock on _qMutex
	///
	void waitIfNeeded(boost::mutex::scoped_lock& qMutexLock);

	void wakeupParserThread();

	/// mutex protecting access to the a/v encoded frames queues
	mutable boost::mutex _qMutex;


	/// Mutex protecting _bytesLoaded (read by main, set by parser)
	mutable boost::mutex _bytesLoadedMutex;

	/// Method to check if buffer is full w/out locking the _qMutex
	//
	///
	/// This is intended for being called by waitIfNeeded, which 
	/// is passed a locked lock on _qMutex, and by parseNextChunk
	/// to determine whether to index-only or also push on queue.
	///
	bool bufferFull() const;

	/// On seek, this flag will be set, while holding a lock on _streamMutex.
	/// The parser, when obtained a lock on _streamMutex, will check this
	/// flag, if found to be true will clear the buffers and reset to false.
	bool _seekRequest;
private:

	typedef std::deque<EncodedVideoFrame*> VideoFrames;
	typedef std::deque<EncodedAudioFrame*> AudioFrames;

	/// Queue of video frames (the video buffer)
	//
	/// Elements owned by this class.
	///
	VideoFrames _videoFrames;

	/// Queue of audio frames (the audio buffer)
	//
	/// Elements owned by this class.
	///
	AudioFrames _audioFrames;

	void requestParserThreadKill()
	{
		boost::mutex::scoped_lock lock(_parserThreadKillRequestMutex);
		_parserThreadKillRequested=true;
		_parserThreadWakeup.notify_all();
	}

	/// Return diff between timestamp of last and first audio frame
	boost::uint64_t audioBufferLength() const;

	/// Return diff between timestamp of last and first video frame
	boost::uint64_t videoBufferLength() const;

	/// A getBufferLength method not locking the _qMutex (expected to be locked by caller already).
	boost::uint64_t getBufferLengthNoLock() const;
	
};


} // gnash.media namespace 
} // namespace gnash

#endif // __MEDIAPARSER_H__
