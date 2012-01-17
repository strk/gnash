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

#ifdef HAVE_DEJAGNU_H
#include "dejagnu.h"
#else
#include "check.h"
#endif

#include "noseek_fd_adapter.h"
#include "IOChannel.h"
#include "tu_file.h"

#include <cstdio>
#include <iostream>
#include <cassert>
#include <memory>

#include "GnashSystemIOHeaders.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sstream>
#include <limits>

using namespace std;

#define CHUNK_SIZE 4

TestState runtest;

static void
dump_buffer(const char* label, char* buf, size_t size, ostream& os)
{
	os << label << ":" << endl;
	for (size_t i=0; i<size; i++) {
		os << '[' << buf[i] << ']';
	}

}


static bool 
compare_reads(gnash::IOChannel* reader, int fd, const char* first, const char* second)
{
	char buf[CHUNK_SIZE];
	char buf2[CHUNK_SIZE];

	stringstream ss;


	size_t consumed = 0;

	for(;;)
	{
		size_t sz1 = reader->read(buf, CHUNK_SIZE);
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

		consumed+=sz1;
	}

	if ( consumed == 0 ) 
	{
		runtest.fail("No bytes read from either " + string(first) + " or " + string(second) + " file");
		return false;
	}

	if ( ! reader->eof() )
	{
		ss << "tu_file not at EOF at end of read";
		runtest.fail(ss.str());
		return false;
	}

	ss << "compared " << consumed << " bytes from "
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

	gnash::IOChannel* reader = gnash::noseek_fd_adapter::make_stream(0, cachename);
	assert(reader);

	compare_reads(reader, raw, "wrapped", "raw");

	lseek(raw, 0, SEEK_SET);
	reader->seek(0);
	compare_reads(reader, raw, "wrapped-rewind", "raw-rewind");


    FILE* f = std::fopen(cachename, "r");
    std::auto_ptr<gnash::IOChannel> orig = gnash::makeFileChannel(f, false);
	lseek(raw, 0, SEEK_SET);
	compare_reads(orig.get(), raw, "cache", "raw");


    if (sizeof(size_t) != sizeof(std::streamoff)) {
        std::streampos pos = std::numeric_limits<size_t>::max();
        pos += orig->size() / 2;
        // Check that seek() handles integer overflow situations gracefully.
        if (orig->seek(pos)) {
            runtest.fail("Successfully sought to an invalid position.");
        } else {
            runtest.pass("Gracefully handled invalid seek.");
        }
    }



	return 0;
}

