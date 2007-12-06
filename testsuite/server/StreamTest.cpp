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
#include "log.h"

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
	unsigned int pos;

	ByteReader(unsigned char by)
		:
		b(by),
		pos(0)
	{}

	void setByte(unsigned char by)
	{
		b=by;
	}

	static int readFunc(void* dst, int bytes, void* appdata) 
	{
		ByteReader* br = (ByteReader*) appdata;

		unsigned char* ptr = static_cast<unsigned char*>(dst);
		for (int i=0; i<bytes; ++i)
		{
			memcpy(ptr+i, &(br->b), sizeof(unsigned char));
		}

		br->pos += bytes;
		return bytes;
	}

	static int tellFunc(void* appdata)
	{
		ByteReader* br = (ByteReader*) appdata;
		return br->pos;
	}

	static int seekFunc(int newPos, void* appdata)
	{
		ByteReader* br = (ByteReader*) appdata;
		br->pos=newPos;
		return 0; // ok, no error (urgh)
	}
};

int
main(int /*argc*/, char** /*argv*/)
{
	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile.setVerbosity(1);

	ByteReader br(0xAA);

	tu_file fakeIn(
		&br,
		ByteReader::readFunc,
		0, // write_func wf,
		ByteReader::seekFunc, // seek_func sf,
		0, //seek_to_end_func ef,
		ByteReader::tellFunc, // tell_func tf,
		0, //get_eof_func gef,
		0, //get_err_func ger
		0, // get_stream_size_func gss,
		0 // close_func cf
	);

	int ret;

	{
	/// bits: 10101010 (0xAA)
	br.setByte(0xAA);
	stream s(&fakeIn);


	check_equals(s.get_position(), 0);
	s.align();
	check_equals(s.get_position(), 0);
	ret = s.read_uint(1); check_equals(ret, 1);
	check_equals(s.get_position(), 1);
	ret = s.read_uint(1); check_equals(ret, 0);
	check_equals(s.get_position(), 1);
	ret = s.read_uint(1); check_equals(ret, 1);
	check_equals(s.get_position(), 1);
	ret = s.read_uint(1); check_equals(ret, 0);
	check_equals(s.get_position(), 1);
	ret = s.read_uint(1); check_equals(ret, 1);
	check_equals(s.get_position(), 1);
	ret = s.read_uint(1); check_equals(ret, 0);
	check_equals(s.get_position(), 1);
	ret = s.read_uint(1); check_equals(ret, 1);
	check_equals(s.get_position(), 1);
	ret = s.read_uint(1); check_equals(ret, 0);
	check_equals(s.get_position(), 1);

	/// bits: 10101010 (0xAA)

	s.align();
	// align just marks all bits in current byte as used, but doesn't read more
	check_equals(s.get_position(), 1);
	ret = s.read_uint(2); check_equals(ret, 2);
	check_equals(s.get_position(), 2);
	ret = s.read_uint(2); check_equals(ret, 2);
	check_equals(s.get_position(), 2);
	ret = s.read_uint(2); check_equals(ret, 2);
	check_equals(s.get_position(), 2);
	ret = s.read_uint(2); check_equals(ret, 2);
	check_equals(s.get_position(), 2);

	/// bits: 10101010 (0xAA)

	s.align();
	// align just marks all bits in current byte as used, but doesn't read more
	check_equals(s.get_position(), 2);
	ret = s.read_uint(3); check_equals(ret, 5);
	check_equals(s.get_position(), 3);
	ret = s.read_uint(3); check_equals(ret, 2);
	check_equals(s.get_position(), 3);
	ret = s.read_uint(2); check_equals(ret, 2);
	check_equals(s.get_position(), 3);

	/// bits: 10101010 (0xAA)

	s.align();
	check_equals(s.get_position(), 3);
	ret = s.read_uint(3); check_equals(ret, 5);
	check_equals(s.get_position(), 4);
	ret = s.read_uint(2); check_equals(ret, 1);
	check_equals(s.get_position(), 4);
	ret = s.read_uint(3); check_equals(ret, 2);
	check_equals(s.get_position(), 4);

	/// bits: 10101010 (0xAA)

	s.align();
	check_equals(s.get_position(), 4);
	ret = s.read_uint(2); check_equals(ret, 2);
	check_equals(s.get_position(), 5);
	ret = s.read_uint(3); check_equals(ret, 5);
	check_equals(s.get_position(), 5);
	ret = s.read_uint(3); check_equals(ret, 2);
	check_equals(s.get_position(), 5);

	/// bits: 10101010 (0xAA)

	s.align();
	check_equals(s.get_position(), 5);
	ret = s.read_uint(4); check_equals(ret, 10);
	check_equals(s.get_position(), 6);
	ret = s.read_uint(4); check_equals(ret, 10);
	check_equals(s.get_position(), 6);

	/// bits: 10101010 (0xAA)

	s.align();
	check_equals(s.get_position(), 6);
	ret = s.read_uint(5); check_equals(ret, 21);
	check_equals(s.get_position(), 7);
	ret = s.read_uint(3); check_equals(ret, 2);
	check_equals(s.get_position(), 7);

	/// bits: 10101010 (0xAA)

	s.align();
	check_equals(s.get_position(), 7);
	ret = s.read_uint(3); check_equals(ret, 5);
	check_equals(s.get_position(), 8);
	ret = s.read_uint(5); check_equals(ret, 10);
	check_equals(s.get_position(), 8);

	/// bits: 10101010 (0xAA)

	s.align();
	check_equals(s.get_position(), 8);
	ret = s.read_uint(6); check_equals(ret, 42);
	check_equals(s.get_position(), 9);
	ret = s.read_uint(2); check_equals(ret, 2);
	check_equals(s.get_position(), 9);

	/// bits: 10101010 (0xAA)

	s.align();
	check_equals(s.get_position(), 9);
	ret = s.read_uint(2); check_equals(ret, 2);
	check_equals(s.get_position(), 10);
	ret = s.read_uint(6); check_equals(ret, 42);
	check_equals(s.get_position(), 10);

	/// bits: 10101010 (0xAA)

	s.align();
	check_equals(s.get_position(), 10);
	ret = s.read_uint(7); check_equals(ret, 85);
	check_equals(s.get_position(), 11);
	ret = s.read_uint(1); check_equals(ret, 0);
	check_equals(s.get_position(), 11);

	/// bits: 10101010 (0xAA)

	s.align();
	check_equals(s.get_position(), 11);
	ret = s.read_uint(1); check_equals(ret, 1);
	check_equals(s.get_position(), 12);
	ret = s.read_uint(7); check_equals(ret, 42);
	check_equals(s.get_position(), 12);

	/// bits: 10101010 (0xAA)

	s.align();
	check_equals(s.get_position(), 12);
	ret = s.read_uint(8); check_equals(ret, 170);
	check_equals(s.get_position(), 13);

	/// bits: 10101 01010 10101 0 (0xAAAA)

	s.align();
	check_equals(s.get_position(), 13);
	ret = s.read_uint(5); check_equals(ret, 21);
	check_equals(s.get_position(), 14);
	ret = s.read_uint(5); check_equals(ret, 10);
	check_equals(s.get_position(), 15);
	ret = s.read_uint(5); check_equals(ret, 21);
	check_equals(s.get_position(), 15);

	/// bits: 101010 101 0101010 (0xAAAA)

	s.align();
	check_equals(s.get_position(), 15);
	ret = s.read_uint(6); check_equals(ret, 42);
	check_equals(s.get_position(), 16);
	ret = s.read_uint(3); check_equals(ret, 5);
	check_equals(s.get_position(), 17);
	ret = s.read_uint(7); check_equals(ret, 42);
	check_equals(s.get_position(), 17);

	/// bits: 1010101010101010 (0xAAAA)

	s.align();
	check_equals(s.get_position(), 17);
	ret = s.read_uint(16); check_equals(ret, 43690);
	check_equals(s.get_position(), 19);

	/// bits: 101010 10101010101010 1010 (0xAAAAAA)

	s.align();
	check_equals(s.get_position(), 19);
	ret = s.read_uint(6); check_equals(ret, 42);
	check_equals(s.get_position(), 20);
	ret = s.read_uint(14); check_equals(ret, 10922);
	check_equals(s.get_position(), 22);
	ret = s.read_uint(4); check_equals(ret, 10);
	check_equals(s.get_position(), 22);

	/// bits: 101010101010101010101010 (0xAAAAAA)

	s.align();
	check_equals(s.get_position(), 22);
	ret = s.read_uint(24); check_equals(ret, 11184810);
	check_equals(s.get_position(), 25);

	/// bits: 1010101010101010 (0xAAAA)

	s.align();
	check_equals(s.get_position(), 25);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.get_position(), 26);
	ret = s.read_bit(); check_equals(ret, 0);
	check_equals(s.get_position(), 26);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.get_position(), 26);
	ret = s.read_bit(); check_equals(ret, 0);
	check_equals(s.get_position(), 26);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.get_position(), 26);
	ret = s.read_bit(); check_equals(ret, 0);
	check_equals(s.get_position(), 26);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.get_position(), 26);
	ret = s.read_bit(); check_equals(ret, 0);
	check_equals(s.get_position(), 26);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.get_position(), 27);
	ret = s.read_bit(); check_equals(ret, 0);
	check_equals(s.get_position(), 27);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.get_position(), 27);
	ret = s.read_bit(); check_equals(ret, 0);
	check_equals(s.get_position(), 27);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.get_position(), 27);
	ret = s.read_bit(); check_equals(ret, 0);
	check_equals(s.get_position(), 27);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.get_position(), 27);
	ret = s.read_bit(); check_equals(ret, 0);
	check_equals(s.get_position(), 27);

	}

	{
	/// bits: 10011001 (0x99)
	br.setByte(0x99);
	stream s(&fakeIn);
	s.set_position(27);

	s.align();
	check_equals(s.get_position(), 27);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.get_position(), 28);
	ret = s.read_bit(); check_equals(ret, 0);
	check_equals(s.get_position(), 28);
	ret = s.read_bit(); check_equals(ret, 0);
	check_equals(s.get_position(), 28);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.get_position(), 28);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.get_position(), 28);
	ret = s.read_bit(); check_equals(ret, 0);
	check_equals(s.get_position(), 28);
	ret = s.read_bit(); check_equals(ret, 0);
	check_equals(s.get_position(), 28);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.get_position(), 28);

	/// bits: 10011001 10011001 (0x999999)

	boost::int16_t s16 = s.read_s16(); check_equals(s16, (boost::int16_t)0x9999);
	check_equals(s.get_position(), 30);
	boost::uint16_t u16 = s.read_u16(); check_equals(u16, (boost::uint16_t)0x9999);
	check_equals(s.get_position(), 32);
	boost::int32_t s32 = s.read_s32(); check_equals(s32, (boost::int32_t)0x99999999);
	check_equals(s.get_position(), 36);
	boost::uint32_t u32 = s.read_u32(); check_equals(u32, (boost::uint32_t)0x99999999);
	check_equals(s.get_position(), 40);

	/// bits: 10011001 10011001 10011001 10011001 (0x99999999)
	///       -
	///        ------- -------- --
	///                           --------

	s.align();
	check_equals(s.get_position(), 40);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.get_position(), 41);
	u16 = s.read_uint(17); check_equals(u16, 26214);
	check_equals(s.get_position(), 43);
	u16 = s.read_uint(7); check_equals(u16, 51);
	check_equals(s.get_position(), 44);

	/// bits: 10011001 10011001 10011001 10011001 (0x99999999)
	///       -
	///        ------- --------   
	///                         ----------

	s.align();
	check_equals(s.get_position(), 44);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.get_position(), 45);
	u16 = s.read_uint(15); check_equals(u16, 6553);
	check_equals(s.get_position(), 46);
	u16 = s.read_uint(9); check_equals(u16, 307);
	check_equals(s.get_position(), 48);

	/// bits: 10011001 10011001 10011001 10011001 (0x99999999)
	///       -
	///        ------- -------- ---       
	///                            ----- -----
	///                                       ---

	s.align();
	check_equals(s.get_position(), 48);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.get_position(), 49);
	u32 = s.read_uint(18); check_equals(u32, 52428);
	check_equals(s.get_position(), 51);
	u16 = s.read_uint(10); check_equals(u16, 819);
	check_equals(s.get_position(), 52);
	u16 = s.read_uint(3); check_equals(u16, 1);
	check_equals(s.get_position(), 52);


	// Test some seeking here...

	s.set_position(52);
	check_equals(s.get_position(), 52);
	s.set_position(0);
	check_equals(s.get_position(), 0);
	s.set_position(325);
	check_equals(s.get_position(), 325);
	s.read_bit(); // might trigger caching
	check_equals(s.get_position(), 326);
	s.set_position(372); // might seek in cache
	check_equals(s.get_position(), 372);
	s.set_position(327); // might seek in cache
	check_equals(s.get_position(), 327);
	s.set_position(326); // might seek in cache
	check_equals(s.get_position(), 326);

	s.set_position(512);
	for (int i=0; i<512; ++i)
	{
		s.read_uint(8); // read_uint triggers caching (or should)
	}
	check_equals(s.get_position(), 1024);
	s.set_position(512); // seek to origin
	check_equals(s.get_position(), 512);

	s.set_position(1000); // seek back (-45)
	check_equals(s.get_position(), 1000);
	s.set_position(200); // long seek back (-800)
	check_equals(s.get_position(), 200);
	s.set_position(220); // short seek forw (+20)
	check_equals(s.get_position(), 220);
	s.set_position(2000); 
	s.read_uint(4);
	check_equals(s.get_position(), 2001);
	s.set_position(1960); 
	check_equals(s.get_position(), 1960);

	}

	return 0;
}

