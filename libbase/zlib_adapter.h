// zlib_adapter.h	-- Thatcher Ulrich 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.


#ifndef ZLIB_ADAPTER_H
#define ZLIB_ADAPTER_H


#include "dsodefs.h"

#include <memory>

class tu_file;


/// Code to wrap zlib compression/decompression around a tu_file stream.
namespace zlib_adapter
{
	// NOTE: these functions will return NULL if
	// HAVE_ZLIB_H is not defined

	/// \brief
	/// Returns a read-only tu_file stream that inflates the remaining
	/// content of the given input stream, as you read data from the
	/// new stream.
	//
	///
	DSOEXPORT std::auto_ptr<tu_file> make_inflater(std::auto_ptr<tu_file> in);

	/// \brief
	/// Returns a write-only tu_file stream that deflates the remaining
	/// content of the given input stream.
	//
	/// TODO: take and return by auto_ptr
	///
	DSOEXPORT tu_file*	make_deflater(tu_file* out);
}


#endif // ZLIB_ADAPTER_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
