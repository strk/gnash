// sound/InputStream.h -- Audio input stream interface
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


#ifndef SOUND_INPUTSTREAM_H
#define SOUND_INPUTSTREAM_H

#include <cstdint> // For C99 int types

namespace gnash {
namespace sound {

/// A %sound input stream
//
/// Instance of this class are sounds you can
/// pull samples from.
///
/// The format of the samples you pull is expected to
/// be PCM samples as signed 16bit values
/// in system-endian format.
///
/// It is expected that consecutive samples fetched 
/// are one for left and one for right stereo channel,
/// in that order (even indexes for left channel, odd
/// indexes for right channel).
///
/// Instances of this class would be the input 
/// for the gnash Mixer (currently sound_handler)
/// instance.
///
class InputStream {

public:

    /// Fetch the given amount of samples, non-blocking and thread-safe.
    //
    /// @param to
    ///     Output buffer, must be at least nSamples*bytes.
    ///     (or nSamples items sized, being a container of 16bit values).
    ///
    /// @param nSamples
    ///     Number of samples to fetch.
    ///     It is expected that the fetcher fetches samples in multiples
    ///     of 2, being each couple composed by a sample for the left
    ///     channel and a sample for the right channel, in that order.
    /// 
    /// @return number of samples actually written to the output buffer.
    ///         If < nSamples this InputStream ran out of data, either
    ///         temporarly or permanently. Use eof() to tell.
    ///
    /// @throws a SoundException (to be better defined a set of them)
    ///     if unable to process this and further requests due to internal
    ///     errors (not if it just happens to complete its source)
    ///
    virtual unsigned int fetchSamples(std::int16_t* to, unsigned int nSamples)=0;

    /// Return number of samples fetched from this stream
    //
    /// It is expected for the return to be always a multiple
    /// of 2, being each stereo sample unit composed by a sample
    /// for the left channel and a sample for the right channel,
    /// in that order.
    ///
    virtual unsigned int samplesFetched() const=0;

    /// Return true if there'll be no more data to fetch.
    virtual bool eof() const=0;

    virtual ~InputStream() {}

};
	

} // gnash.sound namespace 
} // namespace gnash

#endif // SOUND_INPUTSTREAM_H
