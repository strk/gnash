// SoundEnvelope.h - sound envelopes
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#ifndef SOUND_SOUNDENVELOPE_H
#define SOUND_SOUNDENVELOPE_H

#include <boost/cstdint.hpp> // For C99 int types
#include <vector> // for SoundEnvelopes typedef


namespace gnash {
namespace sound {

/// A %sound envelope
//
/// Used to control volume for event sounds. It basically tells that from
/// sample X the volume for left out is Y and for right out is Z. Several
/// envelopes can be assigned to a sound to make a fade out or similar stuff.
//
/// See http://www.m2osw.com/en/swf_alexref.html#swf_envelope
///
class SoundEnvelope
{
public:

    /// Byte offset this envelope applies to
    //
    /// The offset is always given as if the sample data
    /// was defined with a rate of 44100 bytes per seconds.
    /// For instance, the sample number 1 in a sound effect
    /// with a sample rate of 5.5K is given as position 8 in
    /// the envelope. 
    ///
    boost::uint32_t m_mark44;

    /// Volume for the left channel (0..32768)
    boost::uint16_t m_level0;

    /// Volume for the right channel (0..32768)
    boost::uint16_t m_level1;
};

/// A vector of SoundEnvelope objects
typedef std::vector<SoundEnvelope> SoundEnvelopes;



} // gnash.sound namespace 
} // namespace gnash

#endif // SOUND_SOUNDENVELOPE_H
