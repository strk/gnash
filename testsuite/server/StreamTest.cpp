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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#undef HAVE_DEJAGNU_H

#ifdef HAVE_DEJAGNU_H
#include "dejagnu.h"
#else
#include "check.h"
#endif

#include "tu_file.h"
#include "stream.h"

#include <cstdio>
#include <iostream>
#include <cassert>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sstream>

using namespace std;
using namespace gnash;

#define CHUNK_SIZE 4

TestState runtest;

struct ByteReader
{
	unsigned char b;

	ByteReader(unsigned char by)
		:
		b(by)
	{}

	static int readFunc(void* dst, int bytes, void* appdata) 
	{
	    assert(bytes == 1);
	    ByteReader* br = (ByteReader*) appdata;
	    
	    memcpy(dst, &(br->b), sizeof(unsigned char));

		return 1;
	}
};

int
main(int /*argc*/, char** /*argv*/)
{
	ByteReader br(0xAA);

	tu_file fakeIn(
		&br,
		ByteReader::readFunc,
		0, // write_func wf,
		0, //seek_func sf,
		0, //seek_to_end_func ef,
		0, //tell_func tf,
		0, //get_eof_func gef,
		0, //get_err_func ger
		0, // get_stream_size_func gss,
		0 // close_func cf
	);

	stream s(&fakeIn);

	int ret;

	/// bits: 10101010 (0xAA)

	s.align();
	ret = s.read_uint(1); check_equals(ret, 1);
	ret = s.read_uint(1); check_equals(ret, 0);
	ret = s.read_uint(1); check_equals(ret, 1);
	ret = s.read_uint(1); check_equals(ret, 0);

	/// bits: 10101010 (0xAA)

	s.align();
	ret = s.read_uint(1); check_equals(ret, 1);
	ret = s.read_uint(1); check_equals(ret, 0);
	ret = s.read_uint(1); check_equals(ret, 1);
	ret = s.read_uint(1); check_equals(ret, 0);

	/// bits: 10101010 (0xAA)

	s.align();
	ret = s.read_uint(2); check_equals(ret, 2);
	ret = s.read_uint(2); check_equals(ret, 2);
	ret = s.read_uint(2); check_equals(ret, 2);
	ret = s.read_uint(2); check_equals(ret, 2);

	/// bits: 10101010 (0xAA)

	s.align();
	ret = s.read_uint(3); check_equals(ret, 5);
	ret = s.read_uint(3); check_equals(ret, 2);
	ret = s.read_uint(2); check_equals(ret, 2);

	/// bits: 10101010 (0xAA)

	s.align();
	ret = s.read_uint(3); check_equals(ret, 5);
	ret = s.read_uint(2); check_equals(ret, 1);
	ret = s.read_uint(3); check_equals(ret, 2);

	/// bits: 10101010 (0xAA)

	s.align();
	ret = s.read_uint(2); check_equals(ret, 2);
	ret = s.read_uint(3); check_equals(ret, 5);
	ret = s.read_uint(3); check_equals(ret, 2);

	/// bits: 10101010 (0xAA)

	s.align();
	ret = s.read_uint(4); check_equals(ret, 10);
	ret = s.read_uint(4); check_equals(ret, 10);

	/// bits: 10101010 (0xAA)

	s.align();
	ret = s.read_uint(5); check_equals(ret, 21);
	ret = s.read_uint(3); check_equals(ret, 2);

	/// bits: 10101010 (0xAA)

	s.align();
	ret = s.read_uint(3); check_equals(ret, 5);
	ret = s.read_uint(5); check_equals(ret, 10);

	/// bits: 10101010 (0xAA)

	s.align();
	ret = s.read_uint(6); check_equals(ret, 42);
	ret = s.read_uint(2); check_equals(ret, 2);

	/// bits: 10101010 (0xAA)

	s.align();
	ret = s.read_uint(2); check_equals(ret, 2);
	ret = s.read_uint(6); check_equals(ret, 42);

	/// bits: 10101010 (0xAA)

	s.align();
	ret = s.read_uint(7); check_equals(ret, 85);
	ret = s.read_uint(1); check_equals(ret, 0);

	/// bits: 10101010 (0xAA)

	s.align();
	ret = s.read_uint(1); check_equals(ret, 1);
	ret = s.read_uint(7); check_equals(ret, 42);

	/// bits: 10101010 (0xAA)

	s.align();
	ret = s.read_uint(8); check_equals(ret, 170);

	/// bits: 10101 01010 10101 0 (0xAAAA)

	s.align();
	ret = s.read_uint(5); check_equals(ret, 21);
	ret = s.read_uint(5); check_equals(ret, 10);
	ret = s.read_uint(5); check_equals(ret, 21);

	/// bits: 101010 101 0101010 (0xAAAA)

	s.align();
	ret = s.read_uint(6); check_equals(ret, 42);
	ret = s.read_uint(3); check_equals(ret, 5);
	ret = s.read_uint(7); check_equals(ret, 42);

	/// bits: 1010101010101010 (0xAAAA)

	s.align();
	ret = s.read_uint(16); check_equals(ret, 43690);

	return 0;
}

