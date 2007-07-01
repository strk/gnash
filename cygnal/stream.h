// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

#ifndef __STREAM_H__
#define __STREAM_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "statistics.h"

namespace cygnal {

class Stream {
public:
    typedef enum {
        NO_STATE,
        OPEN,
        PLAY,
        PREVIEW,
        THUMBNAIL,
        PAUSE,
        SEEK,
        UPLOAD,
        MULTICAST,
        DONE
    } state_e;  
    Stream();
    ~Stream();
    bool open(const char *filespec);
    bool open(const char *filespec, int netfd);
    bool open(const char *filespec, int netfd, Statistics  *statistics);
    // Stream the movie
    bool play();
    bool play(int netfd);
    // Stream a preview, instead of the full movie.
    bool preview(const char *filespec, int frames);
    // Stream a series of thumbnails
    bool thumbnail(const char *filespec, int quantity);
    // Pause the stream
    bool pause(int frame);
    // Seek within the stream
    bool seek(int frame);
    // Upload a stream into a sandbox
    bool upload(const char *filespec);
    // Stream a single "real-time" source.
    bool multicast(const char *filespec);

private:
    state_e     _state;
    int         _bytes;
    int         _filefd;
    int         _netfd;
    char        *_filespec;
    Statistics  *_statistics;
    unsigned char *_dataptr;
    unsigned char *_seekptr;
    int          _filesize;
};
 
} // end of cygnal namespace


#endif // __STREAM_H__

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
