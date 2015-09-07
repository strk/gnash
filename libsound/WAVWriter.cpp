// WAVWriter.cpp: .wav audio writer 
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

#include "WAVWriter.h"

#include <cstdint>
#include <fstream> // for composition (file_stream)
#include <iostream> 

#include "GnashException.h" // for SoundException
#include "log.h" // will import boost::format too

namespace gnash {
namespace sound {

namespace { // anonymous

// Header of a wave file
// http://ftp.iptel.org/pub/sems/doc/full/current/wav__hdr_8c-source.html
struct WAV_HDR {
     char rID[4];            // 'RIFF'
     std::uint32_t rLen;
     char wID[4];            // 'WAVE'
     char fId[4];            // 'fmt '
     std::uint32_t pcm_header_len;   // varies...
     std::int16_t wFormatTag;
     std::int16_t nChannels;      // 1,2 for stereo data is (l,r) pairs
     std::uint32_t nSamplesPerSec;
     std::uint32_t nAvgBytesPerSec;
     std::int16_t nBlockAlign;
     std::int16_t nBitsPerSample;
};

// Chunk of wave file
// http://ftp.iptel.org/pub/sems/doc/full/current/wav__hdr_8c-source.html
struct CHUNK_HDR{
    char dId[4];            // 'data' or 'fact'
    std::uint32_t dLen;
};

} // end of anonymous namespace

/* public */
WAVWriter::WAVWriter(const std::string& wavefile)
{
        file_stream.open(wavefile.c_str());
        if (file_stream.fail()) {
            boost::format fmt = boost::format(_("Unable to write file %1%"))
                % wavefile;
            throw SoundException(fmt.str());
        } 
        else {
            data_size = 0;
            write_wave_header(file_stream);
            std::cout << "# Created 44100 16Mhz stereo wave file:\n" <<
                    "AUDIOFILE=" << wavefile << std::endl;
        }
}

/* public */
WAVWriter::~WAVWriter()
{
    if (file_stream) {
        // attempt to flush metadata
        file_stream.seekp(0);
        if (file_stream.fail()) {
            log_error("WAVWriter: Failed to flush audio dump metadata, resulting file would be incomplete");
        }
        else {
            write_wave_header(file_stream);
        }

        // close the stream
        file_stream.close();
    }
}

/* public */
void
WAVWriter::pushSamples(std::int16_t* from, unsigned int nSamples)
{
    // NOTE: if muted, the samples will be silent already
        std::uint8_t* stream = reinterpret_cast<std::uint8_t*>(from);
        unsigned int len = nSamples*2;
        file_stream.write((char*) stream, len);
        data_size += len;
}

/* private */
void
WAVWriter::write_wave_header(std::ofstream& outfile)
{
 
  // allocate wav header
  WAV_HDR wav;
  CHUNK_HDR chk;
 
  // setup wav header
  // CID 1149094
  std::memcpy(wav.rID, "RIFF", 4);
  std::memcpy(wav.wID, "WAVE", 4);
  std::memcpy(wav.fId, "fmt ", 4);
 
  wav.wFormatTag = 1;
  wav.nBitsPerSample = 16;
  wav.nSamplesPerSec = 44100;
  wav.nAvgBytesPerSec = 44100;
  wav.nAvgBytesPerSec *= wav.nBitsPerSample / 8;
  wav.nAvgBytesPerSec *= 2;
  wav.nChannels = 2;
  wav.nBlockAlign = 2 * wav.nBitsPerSample / 8;

  // setup data chunk header
  std::memcpy(chk.dId, "data", 4);
  chk.dLen = data_size;
 
  // setup wav header's size field
  wav.pcm_header_len = 16;
  wav.rLen = sizeof(WAV_HDR) - 8 + sizeof(CHUNK_HDR) + chk.dLen;

  /* write riff/wav header */
  outfile.write((char *)&wav, sizeof(WAV_HDR));
 
  /* write chunk header */
  outfile.write((char *)&chk, sizeof(CHUNK_HDR));
 
}


} // gnash.sound namespace 
} // namespace gnash
