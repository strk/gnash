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

#include "NetworkAdapter.h"
#include "tu_file.h"
#include "IOChannel.h"

#include <memory>               // for auto_ptr
#include <cstdio>
#include <iostream>
#include <cassert>

using namespace std;

const char* post = NULL;

#define CHUNK_SIZE 4

static void
dump_curl(const char* url, ostream& os)
{
	std::auto_ptr<gnash::IOChannel> reader;
	if ( post )
	{
		reader.reset( gnash::NetworkAdapter::make_stream(url, post) );
	}
	else
	{
		reader.reset( gnash::NetworkAdapter::make_stream(url) );
	}

	assert(reader.get());

	char buf[CHUNK_SIZE];

	while (size_t read = reader->read(buf, CHUNK_SIZE) )
	{
		for (size_t i=0; i<read; i++) {
			os << buf[i];
		}
	}

}

static void
dump_tu_file(const char* url, ostream& os)
{
    std::auto_ptr<IOChannel> reader = makeFileChannel(url, true);
	assert(reader);
	if (reader->bad()) return;

	char buf[CHUNK_SIZE];

	while ( size_t read = reader->read(buf, CHUNK_SIZE) )
	{
		for (size_t i=0; i<read; i++) {
			os << buf[i];
		}
	}

	if ( reader->eof() )
		printf("-EOF-\n");

}

static void
dump_file(const char* url, ostream& os)
{
	FILE* f = fopen(url, "r");
	if (!f) return;

	char buf[CHUNK_SIZE];

	while ( size_t read=fread(buf, 1, CHUNK_SIZE, f) )
	{
		for (size_t i=0; i<read; i++) {
			os << buf[i];
		}
	}
	fclose(f);
}

int
main(int argc, char** argv)
{
	const char* input = INPUT; // Should be the path to this file

	if ( argc == 1 )
	{
		cerr << "Usage: " << argv[0] << " <url> [<postdata>]" << endl;
		exit(EXIT_FAILURE);
	}

	if ( argc > 1 ) input = argv[1];

	if ( argc > 2 ) post = argv[2];

	cout << "input: " << input << endl;
	if ( post ) cout << "post data: " << post << endl;

#if 0
	cout << "FILE" << endl;
	dump_file(input, cout);
	cout << "TU_FILE" << endl;
	dump_tu_file(input, cout);
#endif
	cout << "CURL" << endl;
	dump_curl(input, cout);

	return 0;
}

