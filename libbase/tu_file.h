// tu_file.h	-- Ignacio Castaño, Thatcher Ulrich 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A very generic file class that can be customized with callbacks.


#ifndef TU_FILE_H
#define TU_FILE_H

#include <cstdio>
#include <memory>
#include "dsodefs.h"

namespace gnash {
    class IOChannel;
}

namespace gnash {

DSOEXPORT std::auto_ptr<IOChannel> makeFileChannel(FILE* fp, bool close);

} // namespace gnash
#endif 

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
