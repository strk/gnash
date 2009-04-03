// 
//   Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include "DisplayList.h"
#include "as_value.h"
#include "DisplayObject.h"
#include "log.h"
#include "VM.h"
#include "DummyMovieDefinition.h"
#include "DummyCharacter.h"
#include "movie_definition.h"
#include "ManualClock.h"

#include <iostream>
#include <sstream>
#include <cassert>
#include <string>

#include "check.h"

using namespace std;
using namespace gnash;

int
main(int /*argc*/, char** /*argv*/)
{
	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile.setVerbosity();

	// Initialize gnash lib
	gnashInit();

	// Initialize a VM
	boost::intrusive_ptr<movie_definition> md5 ( new DummyMovieDefinition(5) );
	boost::intrusive_ptr<movie_definition> md6 ( new DummyMovieDefinition(6) );

	ManualClock clock;
    RunInfo ri("");
    movie_root stage(*md5, clock, ri);

	movie_instance* root = md5->create_movie_instance();
    stage.setRootMovie( root );

	DisplayList dlist1;

	check_equals(dlist1, dlist1);

	DisplayList dlist2 = dlist1;

	check_equals(dlist1, dlist2);

	// just a couple of DisplayObjects
	boost::intrusive_ptr<DisplayObject> ch1 ( new DummyCharacter(root) );
	boost::intrusive_ptr<DisplayObject> ch2 ( new DummyCharacter(root) );

	dlist1.place_DisplayObject( ch1.get(), 1);
	dlist1.place_DisplayObject( ch2.get(), 2);

	check(dlist1 != dlist2);

	dlist2.place_DisplayObject( ch2.get(), 1);
	dlist2.place_DisplayObject( ch1.get(), 2);

	// Resort dlist1 as depth of it's chars has been changed
	// by place_DisplayObject calls above :/
	dlist1.sort();

	check_equals(dlist1, dlist2);

}

