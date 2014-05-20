// WAVWriter.h: .wav audio writer 
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

#ifndef GNASH_SOUND_WAVWRITER_H
#define GNASH_SOUND_WAVWRITER_H

#include <fstream> // for composition (file_stream)
#include <cstdint>

#include "dsodefs.h"

namespace gnash {
namespace sound {

/// WAV writer class
class DSOEXPORT WAVWriter {

public:

    WAVWriter(const std::string& outfile);

    ~WAVWriter();

    /// Write samples to file
    //
    /// @param from
    ///     The buffer to read samples from.
    ///     Buffer must be big enough to hold nSamples samples.
    ///
    /// @param nSamples
    ///     The amount of samples to read.
    ///     NOTE: this number currently refers to "mono" samples
    ///     due to some bad design decision. It is so expected
    ///     that the user fetches 44100 * 2 samples which has to
    ///     be interpreted as series of left,right channel couples.
    ///     TODO: use actual number of samples so that it's expected
    ///           to fetch 44100 per second and let expose a function
    ///           to give interpretation of what comes back (how many
    ///           bytes per channel, which format).
    ///
    void pushSamples(std::int16_t* from, unsigned int nSamples);

private:

    /// File stream for dump file
    //
    /// TODO: move to base class ?
    ///
    std::ofstream file_stream;

    // write a .WAV file header
    void write_wave_header(std::ofstream& outfile);

};

} // gnash.sound namespace 
} // namespace gnash

#endif // GNASH_SOUND_WAVWRITER_H
