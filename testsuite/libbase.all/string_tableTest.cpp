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

#include "string_table.h"
#include "log.h"

#include <iostream>
#include <sstream>
#include <cassert>
#include <cmath>
#include <string>

#include "check.h"

#include "utility.h"
using namespace gnash;
int
main(int /*argc*/, char** /*argv*/)
{
	string_table st;
	
	LogFile& lgf = LogFile::getDefaultInstance();
	lgf.setVerbosity(2);

	st.insert("A");
	st.insert("B");
	st.insert("C");

	check_equals(st.find("D",false),0);
	check(st.find("D") > 0);
	check(st.find("A") > 0);
	check(st.find("B") > 0);
	check(st.find("C") > 0);

    const bool nocase = true;
    check(equal(st, st.find("AbAb"), st.find("abaB"), nocase));
    check(equal(st, st.find("AbAb"), st.find("ABAB"), nocase));
    check(!equal(st, st.find("AbAb"), st.find("abaB"), false));
    check(!equal(st, st.find("AbAb"), st.find("ABAB"), false));


}
