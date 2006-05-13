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

#include "curl_adapter.h"
#include "tu_file.h"

#include <cstdio>
#include <iostream>
#include <cassert>

using namespace std;

#define CHUNK_SIZE 4

static void
dump_curl(const char* url, ostream& os)
{
	tu_file* reader = curl_adapter::make_stream(url);
	assert(reader);

	char buf[CHUNK_SIZE];

	while (size_t read = reader->read_bytes(buf, CHUNK_SIZE) )
	{
		for (size_t i=0; i<read; i++) {
			os << buf[i];
		}
	}

}

static void
dump_tu_file(const char* url, ostream& os)
{
	tu_file* reader = new tu_file(url, "r");
	assert(reader);
	if (!reader->get_error() == TU_FILE_NO_ERROR) return;

	char buf[CHUNK_SIZE];

	while ( size_t read = reader->read_bytes(buf, CHUNK_SIZE) )
	{
		for (size_t i=0; i<read; i++) {
			os << buf[i];
		}
	}

	if ( reader->get_eof() )
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

}

int
main(int argc, char** argv)
{
	const char* input = INPUT; // Should be the path to this file

	if ( argc > 1 ) input = argv[1];

	cout << "input: " << input << endl;

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

