// AuxStream.h - external sound input stream
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

#ifndef SOUND_AUXSTREAM_H
#define SOUND_AUXSTREAM_H

#include "InputStream.h" // for inheritance

#include <boost/cstdint.hpp> // For C99 int types

namespace gnash {
namespace sound {

/// @see sound_handler::attach_aux_streamer
typedef unsigned int (*aux_streamer_ptr)(void *udata,
        boost::int16_t* samples, unsigned int nSamples, bool& eof);

class AuxStream : public InputStream {
public:
    AuxStream(aux_streamer_ptr cb, void* arg)
        :
        _samplesFetched(0),
        _eof(false),
        _cb(cb),
        _cbArg(arg)
    {}

    // See dox in InputStream.h
    unsigned int fetchSamples(boost::int16_t* to, unsigned int nSamples)
    {
        unsigned int wrote = _cb(_cbArg, to, nSamples, _eof);
        _samplesFetched += wrote;
        return wrote;
    }

    // See dox in InputStream.h
    unsigned int samplesFetched() const
    {
        return _samplesFetched;
    }

    // See dox in InputStream.h
    bool eof() const 
    {
        return _eof;
    }

private:
    unsigned int _samplesFetched;
    bool _eof;
    aux_streamer_ptr _cb;
    void* _cbArg;
};

} // gnash.sound namespace 
} // namespace gnash

#endif // SOUND_AUXSTREAM_H
