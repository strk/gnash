// png.h: libpng wrapper for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#ifndef GNASH_IMAGE_PNG_H
#define GNASH_IMAGE_PNG_H

#include "dsodefs.h"
#include <csetjmp> // for jmp_buf
#include <memory>

// Forward declarations
namespace gnash { class IOChannel; }

namespace gnash {
namespace png  {

	class input {

	public:

		input()
		{}

		virtual ~input() {}


		/// \brief
		/// Create and return a jpeg-input object that will read from the
		/// given input stream.
		//
		/// The created input reads the jpeg header
		///
		/// @param in
		///	The stream to read from. Ownership specified by last arg.
		///
		/// @param takeOwnership
		///	If false, ownership of the stream 
		///	is left to caller, otherwise we take it.
		///	NOTE: In case the caller retains ownership, it must
		///	make sure the stream is alive and not modified
		///	for the whole lifetime of the returned instance.
		///
		/// @return NULL on error
		///
		DSOEXPORT static std::auto_ptr<input> create(gnash::IOChannel& in);

		/// Read SWF JPEG2-style header. 
		//
		/// App needs to call start_image() before loading any
		/// image data.  Multiple images can be loaded by
		/// bracketing within start_image()/finish_image() pairs.
		///
		/// @param in
		///	The gnash::IOChannel to use for input. Ownership specified
		///	by last arg.
		///
		/// @param maxHeaderBytes
		///	Max number of bytes to read from input for header.
		///
		/// @param takeOwnership
		///	If false, ownership of the stream 
		///	is left to caller, otherwise we take it.
		///	NOTE: In case the caller retains ownership, it must
		///	make sure the stream is alive and not modified
		///	for the whole lifetime of the returned instance.
		///
		/// @return NULL on error
		///
//		DSOEXPORT std::auto_ptr<input> create_swf_jpeg2_header_only(gnash::IOChannel* in,
//				unsigned int maxHeaderBytes, bool takeOwnership=false);

		/// Discard existing bytes in our buffer.

        virtual void read() = 0;

		virtual size_t getHeight() const = 0;
		virtual size_t getWidth() const = 0;
		virtual void readScanline(unsigned char* rgb_data) = 0;

	};

} // namespace png
} // namespace gnash



#endif
