// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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
#include "movie_root.h"
#include "as_value.h"
#include "DisplayObject.h"
#include "log.h"
#include "VM.h"
#include "DummyMovieDefinition.h"
#include "DummyCharacter.h"
#include "movie_definition.h"
#include "ManualClock.h"
#include "RunResources.h"
#include "StreamProvider.h"

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
    
    RunResources ri;
    const URL url("");
    ri.setStreamProvider(
            boost::shared_ptr<StreamProvider>(new StreamProvider(url, url)));
    
    // Initialize a VM
    boost::intrusive_ptr<movie_definition> md5(new DummyMovieDefinition(ri, 5));
    boost::intrusive_ptr<movie_definition> md6(new DummyMovieDefinition(ri, 6));
    
    ManualClock clock;
    movie_root stage(*md5, clock, ri);
    
    MovieClip::MovieVariables v;
    stage.init(md5.get(), v);
    
    DisplayList dlist1;
    
    check_equals(dlist1, dlist1);
    
    DisplayList dlist2 = dlist1;
    
    check_equals(dlist1, dlist2);
    
    MovieClip* root = const_cast<Movie*>(&stage.getRootMovie());
    
    // just a couple of DisplayObjects
    as_object* ob1 = createObject(getGlobal(*getObject(root)));
    as_object* ob2 = createObject(getGlobal(*getObject(root)));
    
    DisplayObject* ch1 ( new DummyCharacter(ob1, root) );
    DisplayObject* ch2 ( new DummyCharacter(ob2, root) );
    
    dlist1.placeDisplayObject(ch1, 1);
    dlist1.placeDisplayObject(ch2, 2);
    
    check(dlist1 != dlist2);
    
    dlist2.placeDisplayObject(ch2, 1);
    dlist2.placeDisplayObject(ch1, 2);
    
    
}

