// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
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

#ifndef GNASH_LIBMEDIA_SOUNDINFO_H
#define GNASH_LIBMEDIA_SOUNDINFO_H

#include "MediaParser.h" // for audioCodecType enum and AudioInfo

namespace gnash {
namespace media {

/// Class containing information about an embedded sound definition
//
/// Is created by the parser while
/// parsing, and ownership is then transfered to EmbeddedSound. When the parser is
/// parsing streams, it will ask the soundhandler for this to know what properties
/// the stream has.
///
class SoundInfo {
public:
	/// Constructor
	//
	/// @param format
	///     The encoding/format of this sound. 
	///
	/// @param stereo
	///     Defines whether the sound is in stereo.
	///
	/// @param sampleRate
	///     The sample rate of the sound.
	///
	/// @param sampleCount
	///     The sample count in the sound.
    ///     In soundstreams this is an average for each frame.
	///
	/// @param is16bit
	///     If true, the sound is in 16bit format (samplesize == 2)
	///     else it is 8bit (samplesize == 1).
	///     Used for streams when decoding adpcm.
	///
	/// @param delaySeek
	///     Number of samples to seek forward or delay.
	///     If this value is positive, the player seeks this
	///     number of samples into the sound block before the
	///     sound is played.
	///     If this value is negative the player plays this
	///     number of silent samples before playing the sound block
	///     NOTE that this value refers to input samples, so must
	///     be multiplied by OUTPUT_SAMPLE_RATE/getSampleRate()
	///     and by 2 (two channels) to find number of output
	///     samples to skip or fill.
	///
	SoundInfo(audioCodecType format, bool stereo, boost::uint32_t sampleRate,
            boost::uint32_t sampleCount, bool is16bit,
            boost::int16_t delaySeek=0)
	    :
        _format(format),
		_stereo(stereo),
		_sampleRate(sampleRate),
		_sampleCount(sampleCount),
		_delaySeek(delaySeek),
		_is16bit(is16bit)
	{
	}

	/// Returns the format of the sound
	//
	/// @return the format of the sound
	audioCodecType getFormat() const { return _format; }

	/// Returns the stereo status of the sound
	//
	/// @return the stereo status of the sound
	bool isStereo() const { return _stereo; }

	/// Returns the samplerate of the sound
	//
	/// @return the samplerate of the sound
	unsigned long getSampleRate() const { return _sampleRate; }

	/// Returns the samplecount of the sound
	//
	/// This is the amount of samples you'd get after
	/// successfully decoding the sound.
	///
	/// @return the samplecount of the sound
	///
	unsigned long getSampleCount() const { return _sampleCount; }

	/// Return the number of samples to seek forward or delay
	// 
	/// The number is to be considered in pre-resampling units.
	///
	boost::int16_t getDelaySeek() const { return _delaySeek; }

	/// Returns the 16bit status of the sound
	//
	/// @return the 16bit status of the sound
	bool is16bit() const { return _is16bit; }

private:
	/// Current format of the sound (MP3, raw, etc).
	audioCodecType _format;

	/// Stereo or not
	bool _stereo;

	/// Sample rate, one of 5512, 11025, 22050, 44100
	boost::uint32_t _sampleRate;

	/// Number of samples
	boost::uint32_t _sampleCount;

	/// Number of samples to seek forward or delay.
	boost::int16_t _delaySeek;

	/// Is the audio in 16bit format (samplesize == 2)? else it 
	/// is 8bit (samplesize == 1). Used for streams when decoding adpcm.
	bool _is16bit;
};

} // gnash.media namespace 
} // namespace gnash

#endif // __SOUNDINFO_H__
