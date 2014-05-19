// zlib_adapter.h    -- Thatcher Ulrich 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.


#ifndef ZLIB_ADAPTER_H
#define ZLIB_ADAPTER_H


#include "dsodefs.h"

#include <memory>

namespace gnash {

class IOChannel;


/// Code to wrap zlib compression/decompression around an IOChannel stream.
namespace zlib_adapter
{
    // NOTE: these functions will return NULL if
    // HAVE_ZLIB_H is not defined

    /// \brief
    /// Returns a read-only IOChannel stream that inflates the remaining
    /// content of the given input stream, as you read data from the
    /// new stream.
    //
    ///
    DSOEXPORT std::unique_ptr<IOChannel>
        make_inflater(std::unique_ptr<IOChannel> in);

} // namespace gnash.zlib_adapter
} // namespace gnash


#endif // ZLIB_ADAPTER_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
