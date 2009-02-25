//  Declaration of functions and data types used for MD5 sum computing
//  library functions.
//  Copyright (C) 1995-1997,1999,2000,2001,2004,2005, 2009
//  Free Software Foundation, Inc.
//  This file is part of the GNU C Library.
//  The GNU C Library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.

//  The GNU C Library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.

//  You should have received a copy of the GNU Lesser General Public
//  License along with the GNU C Library; if not, write to the Free
//  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
//  02111-1307 USA.

// Written by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.
// Trimmed by Rob Savoye for Gnash, and converted to C++ <rob@welcomehome.org>, 2007.

#ifndef _MD5_H
#define _MD5_H 1

#include <cstdio>
#include <string>
#include <boost/cstdint.hpp>

// #if defined HAVE_LIMITS_H
// # include <limits.h>
// #endif

#define MD5_DIGEST_SIZE 16
#define MD5_BLOCK_SIZE 64

typedef unsigned long md5_uintptr;

// Structure to save state of computation between the single steps. 
struct md5_ctx
{
  uint32_t A;
  uint32_t B;
  uint32_t C;
  uint32_t D;

  uint32_t total[2];
  uint32_t buflen;
  char buffer[128] __attribute__ ((__aligned__ (__alignof__ (uint32_t))));
};

// The following three functions are build up the low level used in
// the functions `md5_stream' and `md5_buffer'.

// Initialize structure containing state of computation.
// (RFC 1321, 3.3: Step 3)
void md5_init_ctx (struct md5_ctx *ctx);

// Starting with the result of former calls of this function (or the
// initialization function update the context for the next LEN bytes
// starting at BUFFER.
// It is necessary that LEN is a multiple of 64!!!
void md5_process_block (const void *buffer, size_t len,
				 struct md5_ctx *ctx);

// Starting with the result of former calls of this function (or the
// initialization function update the context for the next LEN bytes
// starting at BUFFER.
// It is NOT required that LEN is a multiple of 64.
void md5_process_bytes (const void *buffer, size_t len,
				 struct md5_ctx *ctx);

// Process the remaining bytes in the buffer and put result from CTX
// in first 16 bytes following RESBUF.  The result is always in little
// endian byte order, so that a byte-wise output yields to the wanted
// ASCII representation of the message digest.

// IMPORTANT: On some systems it is required that RESBUF is correctly
// aligned for a 32 bits value.
void *md5_finish_ctx (struct md5_ctx *ctx, void *resbuf);


// Put result from CTX in first 16 bytes following RESBUF.  The result is
// always in little endian byte order, so that a byte-wise output yields
// to the wanted ASCII representation of the message digest.

// IMPORTANT: On some systems it is required that RESBUF is correctly
// aligned for a 32 bits value.
void *md5_read_ctx (const struct md5_ctx *ctx, void *resbuf);

// Compute MD5 message digest for bytes read from STREAM.  The
// resulting message digest number will be written into the 16 bytes
// beginning at RESBLOCK.
int md5_stream (FILE *stream, void *resblock);

// Compute MD5 message digest for bytes read from filespec. The
// resulting message digest number will be written into the 16 bytes
// beginning at RESBLOCK.
bool md5_filespec(std::string &filespec, void *resblock);

// Compute MD5 message digest for bytes read from filespec. The result
// is compared to the ascii version of the MD5 (as produced by the
// shell utility md5sum).
bool md5_filespec_check(std::string &filespec, std::string &md5);

// Compute MD5 message digest for LEN bytes beginning at BUFFER.  The
// result is always in little endian byte order, so that a byte-wise
// output yields to the wanted ASCII representation of the message
// digest.
void *md5_buffer (const char *buffer, size_t len,
			   void *resblock);

#endif /* md5.h */
// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
