/**/
// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 

#include "noseek_fd_adapter.h"
#include "tu_file.h"

#include <cstdio>
#include <iostream>
#include <cassert>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <dejagnu.h>
#include <sstream>

using namespace std;

#define CHUNK_SIZE 4

TestState runtest;

static void
dump_buffer(char* label, char* buf, size_t size, ostream& os)
{
	os << label << ":" << endl;
	for (size_t i=0; i<size; i++) {
		os << '[' << buf[i] << ']';
	}

}


bool 
compare_reads(tu_file* reader, int fd, char* first, char* second)
{
	char buf[CHUNK_SIZE];
	char buf2[CHUNK_SIZE];

	stringstream ss;


	size_t read_bytes = 0;

	for(;;)
	{
		size_t sz1 = reader->read_bytes(buf, CHUNK_SIZE);
		size_t sz2 = read(fd, buf2, CHUNK_SIZE);

		if ( sz1 != sz2 )
		{
			ss << "Different read size from " << first
				<< " (" << sz1 << ") and " << second
				<< " (" << sz2 << ") file";
			runtest.fail(ss.str());
			dump_buffer("wrapped", buf, sz1, cout);
			dump_buffer("raw", buf2, sz2, cout);
			return false;
		}

		if ( sz1 == 0 ) {
			break;
		}

		if ( memcmp(buf, buf2, sz1) )
		{
			ss << "Different read content from " << first
				<< " and " << second << " file";
			runtest.fail(ss.str());
			dump_buffer("wrapped", buf, sz1, cout);
			dump_buffer("raw", buf2, sz2, cout);
			return false;
		}

		read_bytes+=sz1;
	}

	if ( read_bytes == 0 ) 
	{
		runtest.fail("No bytes read from either " + string(first) + " or " + string(second) + " file");
		return false;
	}

	if ( ! reader->get_eof() )
	{
		ss << "tu_file not at EOF at end of read";
		runtest.fail(ss.str());
		return false;
	}

	ss << "compared " << read_bytes << " bytes from "
		<< first << " and " << second;

	runtest.pass(ss.str());
	return true;


	return true;

}

int
main(int /*argc*/, char** /*argv*/)
{
	const char* input = INPUT; // Should be the path to this file
	const char* cachename = "NoSeekFileTestCache";

	int fd = open(input, O_RDONLY);
	int raw = open(input, O_RDONLY);

	dup2(fd, 0);

	tu_file* reader = noseek_fd_adapter::make_stream(0, cachename);
	assert(reader);

	compare_reads(reader, raw, "wrapped", "raw");

	lseek(raw, 0, SEEK_SET);
	reader->set_position(0);
	compare_reads(reader, raw, "wrapped-rewind", "raw-rewind");

	tu_file orig(cachename, "r");
	lseek(raw, 0, SEEK_SET);
	compare_reads(&orig, raw, "cache", "raw");

	return 0;
}

