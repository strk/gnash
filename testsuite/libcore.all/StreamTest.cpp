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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#undef HAVE_DEJAGNU_H

#ifdef HAVE_DEJAGNU_H
#include "dejagnu.h"
#else
#include "check.h"
#endif

#include <boost/thread/mutex.hpp>

#include "IOChannel.h"
#include "SWFStream.h"
#include "log.h"

#include <cstdio>
#include <iostream>
#include <cassert>

#include "GnashSystemIOHeaders.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sstream>


using namespace std;
using namespace gnash;

#define CHUNK_SIZE 4

TestState runtest;

struct ByteReader : public IOChannel
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

    std::streamsize read(void* dst, std::streamsize bytes) 
	{

		unsigned char* ptr = static_cast<unsigned char*>(dst);
		for (std::streamsize i=0; i<bytes; ++i)
		{
			memcpy(ptr+i, &b, sizeof(unsigned char));
		}

		pos += bytes;
		return bytes;
	}

    std::streampos tell() const
	{
		return pos;
	}

    bool seek(std::streampos newPos)
	{
		pos=newPos;
		return true; 
	}
	
	
	// These here to satisfy the IOChannel interface requirements.
	// I wouldn't call them, if I were you.
	void go_to_end() { abort(); }

	bool eof() const { abort(); return false; }
    
	bool bad() const { return false; }

    size_t size() const { abort(); return -1; }
	
};

TRYMAIN(_runtest);
int
trymain(int /*argc*/, char** /*argv*/)
{
	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile.setVerbosity(1);

	ByteReader br(0xAA);

//	tu_file fakeIn(
//		&br,
//		ByteReader::readFunc,
//		0, // write_func wf,
//		ByteReader::seekFunc, // seek_func sf,
//		0, //seek_to_end_func ef,
//		ByteReader::tellFunc, // tell_func tf,
//		0, //get_eof_func gef,
//		0, //get_err_func ger
//		0, // get_stream_size_func gss,
//		0 // close_func cf
//	);

	int ret;

	{
	/// bits: 10101010 (0xAA)
	br.setByte(0xAA);
	SWFStream s(&br);


	check_equals(s.tell(), 0);
	s.align();
	check_equals(s.tell(), 0);
	ret = s.read_uint(1); check_equals(ret, 1);
	check_equals(s.tell(), 1);
	ret = s.read_uint(1); check_equals(ret, 0);
	check_equals(s.tell(), 1);
	ret = s.read_uint(1); check_equals(ret, 1);
	check_equals(s.tell(), 1);
	ret = s.read_uint(1); check_equals(ret, 0);
	check_equals(s.tell(), 1);
	ret = s.read_uint(1); check_equals(ret, 1);
	check_equals(s.tell(), 1);
	ret = s.read_uint(1); check_equals(ret, 0);
	check_equals(s.tell(), 1);
	ret = s.read_uint(1); check_equals(ret, 1);
	check_equals(s.tell(), 1);
	ret = s.read_uint(1); check_equals(ret, 0);
	check_equals(s.tell(), 1);

	/// bits: 10101010 (0xAA)

	s.align();
	// align just marks all bits in current byte as used, but doesn't read more
	check_equals(s.tell(), 1);
	ret = s.read_uint(2); check_equals(ret, 2);
	check_equals(s.tell(), 2);
	ret = s.read_uint(2); check_equals(ret, 2);
	check_equals(s.tell(), 2);
	ret = s.read_uint(2); check_equals(ret, 2);
	check_equals(s.tell(), 2);
	ret = s.read_uint(2); check_equals(ret, 2);
	check_equals(s.tell(), 2);

	/// bits: 10101010 (0xAA)

	s.align();
	// align just marks all bits in current byte as used, but doesn't read more
	check_equals(s.tell(), 2);
	ret = s.read_uint(3); check_equals(ret, 5);
	check_equals(s.tell(), 3);
	ret = s.read_uint(3); check_equals(ret, 2);
	check_equals(s.tell(), 3);
	ret = s.read_uint(2); check_equals(ret, 2);
	check_equals(s.tell(), 3);

	/// bits: 10101010 (0xAA)

	s.align();
	check_equals(s.tell(), 3);
	ret = s.read_uint(3); check_equals(ret, 5);
	check_equals(s.tell(), 4);
	ret = s.read_uint(2); check_equals(ret, 1);
	check_equals(s.tell(), 4);
	ret = s.read_uint(3); check_equals(ret, 2);
	check_equals(s.tell(), 4);

	/// bits: 10101010 (0xAA)

	s.align();
	check_equals(s.tell(), 4);
	ret = s.read_uint(2); check_equals(ret, 2);
	check_equals(s.tell(), 5);
	ret = s.read_uint(3); check_equals(ret, 5);
	check_equals(s.tell(), 5);
	ret = s.read_uint(3); check_equals(ret, 2);
	check_equals(s.tell(), 5);

	/// bits: 10101010 (0xAA)

	s.align();
	check_equals(s.tell(), 5);
	ret = s.read_uint(4); check_equals(ret, 10);
	check_equals(s.tell(), 6);
	ret = s.read_uint(4); check_equals(ret, 10);
	check_equals(s.tell(), 6);

	/// bits: 10101010 (0xAA)

	s.align();
	check_equals(s.tell(), 6);
	ret = s.read_uint(5); check_equals(ret, 21);
	check_equals(s.tell(), 7);
	ret = s.read_uint(3); check_equals(ret, 2);
	check_equals(s.tell(), 7);

	/// bits: 10101010 (0xAA)

	s.align();
	check_equals(s.tell(), 7);
	ret = s.read_uint(3); check_equals(ret, 5);
	check_equals(s.tell(), 8);
	ret = s.read_uint(5); check_equals(ret, 10);
	check_equals(s.tell(), 8);

	/// bits: 10101010 (0xAA)

	s.align();
	check_equals(s.tell(), 8);
	ret = s.read_uint(6); check_equals(ret, 42);
	check_equals(s.tell(), 9);
	ret = s.read_uint(2); check_equals(ret, 2);
	check_equals(s.tell(), 9);

	/// bits: 10101010 (0xAA)

	s.align();
	check_equals(s.tell(), 9);
	ret = s.read_uint(2); check_equals(ret, 2);
	check_equals(s.tell(), 10);
	ret = s.read_uint(6); check_equals(ret, 42);
	check_equals(s.tell(), 10);

	/// bits: 10101010 (0xAA)

	s.align();
	check_equals(s.tell(), 10);
	ret = s.read_uint(7); check_equals(ret, 85);
	check_equals(s.tell(), 11);
	ret = s.read_uint(1); check_equals(ret, 0);
	check_equals(s.tell(), 11);

	/// bits: 10101010 (0xAA)

	s.align();
	check_equals(s.tell(), 11);
	ret = s.read_uint(1); check_equals(ret, 1);
	check_equals(s.tell(), 12);
	ret = s.read_uint(7); check_equals(ret, 42);
	check_equals(s.tell(), 12);

	/// bits: 10101010 (0xAA)

	s.align();
	check_equals(s.tell(), 12);
	ret = s.read_uint(8); check_equals(ret, 170);
	check_equals(s.tell(), 13);

	/// bits: 10101 01010 10101 0 (0xAAAA)

	s.align();
	check_equals(s.tell(), 13);
	ret = s.read_uint(5); check_equals(ret, 21);
	check_equals(s.tell(), 14);
	ret = s.read_uint(5); check_equals(ret, 10);
	check_equals(s.tell(), 15);
	ret = s.read_uint(5); check_equals(ret, 21);
	check_equals(s.tell(), 15);

	/// bits: 101010 101 0101010 (0xAAAA)

	s.align();
	check_equals(s.tell(), 15);
	ret = s.read_uint(6); check_equals(ret, 42);
	check_equals(s.tell(), 16);
	ret = s.read_uint(3); check_equals(ret, 5);
	check_equals(s.tell(), 17);
	ret = s.read_uint(7); check_equals(ret, 42);
	check_equals(s.tell(), 17);

	/// bits: 1010101010101010 (0xAAAA)

	s.align();
	check_equals(s.tell(), 17);
	ret = s.read_uint(16); check_equals(ret, 43690);
	check_equals(s.tell(), 19);

	/// bits: 101010 10101010101010 1010 (0xAAAAAA)

	s.align();
	check_equals(s.tell(), 19);
	ret = s.read_uint(6); check_equals(ret, 42);
	check_equals(s.tell(), 20);
	ret = s.read_uint(14); check_equals(ret, 10922);
	check_equals(s.tell(), 22);
	ret = s.read_uint(4); check_equals(ret, 10);
	check_equals(s.tell(), 22);

	/// bits: 101010101010101010101010 (0xAAAAAA)

	s.align();
	check_equals(s.tell(), 22);
	ret = s.read_uint(24); check_equals(ret, 11184810);
	check_equals(s.tell(), 25);

	/// bits: 1010101010101010 (0xAAAA)

	s.align();
	check_equals(s.tell(), 25);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.tell(), 26);
	ret = s.read_bit(); check_equals(ret, 0);
	check_equals(s.tell(), 26);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.tell(), 26);
	ret = s.read_bit(); check_equals(ret, 0);
	check_equals(s.tell(), 26);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.tell(), 26);
	ret = s.read_bit(); check_equals(ret, 0);
	check_equals(s.tell(), 26);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.tell(), 26);
	ret = s.read_bit(); check_equals(ret, 0);
	check_equals(s.tell(), 26);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.tell(), 27);
	ret = s.read_bit(); check_equals(ret, 0);
	check_equals(s.tell(), 27);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.tell(), 27);
	ret = s.read_bit(); check_equals(ret, 0);
	check_equals(s.tell(), 27);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.tell(), 27);
	ret = s.read_bit(); check_equals(ret, 0);
	check_equals(s.tell(), 27);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.tell(), 27);
	ret = s.read_bit(); check_equals(ret, 0);
	check_equals(s.tell(), 27);

	}

	{
	/// bits: 10011001 (0x99)
	br.setByte(0x99);
	SWFStream s(&br);
	s.seek(27);

	s.align();
	check_equals(s.tell(), 27);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.tell(), 28);
	ret = s.read_bit(); check_equals(ret, 0);
	check_equals(s.tell(), 28);
	ret = s.read_bit(); check_equals(ret, 0);
	check_equals(s.tell(), 28);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.tell(), 28);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.tell(), 28);
	ret = s.read_bit(); check_equals(ret, 0);
	check_equals(s.tell(), 28);
	ret = s.read_bit(); check_equals(ret, 0);
	check_equals(s.tell(), 28);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.tell(), 28);

	/// bits: 10011001 10011001 (0x999999)

	boost::int16_t s16 = s.read_s16(); check_equals(s16, (boost::int16_t)0x9999);
	check_equals(s.tell(), 30);
	boost::uint16_t u16 = s.read_u16(); check_equals(u16, (boost::uint16_t)0x9999);
	check_equals(s.tell(), 32);
	boost::int32_t s32 = s.read_s32(); check_equals(s32, (boost::int32_t)0x99999999);
	check_equals(s.tell(), 36);
	boost::uint32_t u32 = s.read_u32(); check_equals(u32, (boost::uint32_t)0x99999999);
	check_equals(s.tell(), 40);

	/// bits: 10011001 10011001 10011001 10011001 (0x99999999)
	///       -
	///        ------- -------- --
	///                           --------

	s.align();
	check_equals(s.tell(), 40);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.tell(), 41);
	u16 = s.read_uint(17); check_equals(u16, 26214);
	check_equals(s.tell(), 43);
	u16 = s.read_uint(7); check_equals(u16, 51);
	check_equals(s.tell(), 44);

	/// bits: 10011001 10011001 10011001 10011001 (0x99999999)
	///       -
	///        ------- --------   
	///                         ----------

	s.align();
	check_equals(s.tell(), 44);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.tell(), 45);
	u16 = s.read_uint(15); check_equals(u16, 6553);
	check_equals(s.tell(), 46);
	u16 = s.read_uint(9); check_equals(u16, 307);
	check_equals(s.tell(), 48);

	/// bits: 10011001 10011001 10011001 10011001 (0x99999999)
	///       -
	///        ------- -------- ---       
	///                            ----- -----
	///                                       ---

	s.align();
	check_equals(s.tell(), 48);
	ret = s.read_bit(); check_equals(ret, 1);
	check_equals(s.tell(), 49);
	u32 = s.read_uint(18); check_equals(u32, 52428);
	check_equals(s.tell(), 51);
	u16 = s.read_uint(10); check_equals(u16, 819);
	check_equals(s.tell(), 52);
	u16 = s.read_uint(3); check_equals(u16, 1);
	check_equals(s.tell(), 52);


	// Test some seeking here...

	s.seek(52);
	check_equals(s.tell(), 52);
	s.seek(0);
	check_equals(s.tell(), 0);
	s.seek(325);
	check_equals(s.tell(), 325);
	s.read_bit(); // might trigger caching
	check_equals(s.tell(), 326);
	s.seek(372); // might seek in cache
	check_equals(s.tell(), 372);
	s.seek(327); // might seek in cache
	check_equals(s.tell(), 327);
	s.seek(326); // might seek in cache
	check_equals(s.tell(), 326);

	s.seek(512);
	for (int i=0; i<512; ++i)
	{
		s.read_uint(8); // read_uint triggers caching (or should)
	}
	check_equals(s.tell(), 1024);
	s.seek(512); // seek to origin
	check_equals(s.tell(), 512);

	s.seek(1000); // seek back (-45)
	check_equals(s.tell(), 1000);
	s.seek(200); // long seek back (-800)
	check_equals(s.tell(), 200);
	s.seek(220); // short seek forw (+20)
	check_equals(s.tell(), 220);
	s.seek(2000); 
	s.read_uint(4);
	check_equals(s.tell(), 2001);
	s.seek(1960); 
	check_equals(s.tell(), 1960);

	}

	return 0;
}

