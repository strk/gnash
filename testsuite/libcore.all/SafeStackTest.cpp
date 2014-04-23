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

#include "SafeStack.h"
#include "log.h"

#include <iostream>
#include <sstream>
#include <cassert>
#include <cmath>
#include <string>

#include "check.h"

#include "utility.h"

using namespace gnash;
using std::cout;
using std::endl;

int
main(int /*argc*/, char** /*argv*/)
{
	SafeStack<int> st;

	LogFile& lgf = LogFile::getDefaultInstance();
	lgf.setVerbosity(2);

	check_equals(st.size(), 0);
	check(st.empty());

	bool gotException = false;
	try { st.top(0); } catch (StackException&) { gotException=true; }
	check(gotException); gotException = false;

	try { st.value(0); } catch (StackException&) { gotException=true; }
	check(gotException); gotException = false;

	try { st.drop(1); } catch (StackException&) { gotException=true; }
	check(gotException); gotException = false;

	st.push(4);
	check_equals(st.size(), 1);
	check_equals(st.getDownstop(), 0); // downstop shouldn't change on push

	check_equals(st.top(0), 4);
	try { st.top(1); } catch (StackException&) { gotException=true; }
	check(gotException); gotException = false;

	check_equals(st.value(0), 4);
	try { st.value(1); } catch (StackException&) { gotException=true; }
	check(gotException); gotException = false;

	int popped = st.pop();
	check_equals(st.size(), 0);
	check_equals(popped, 4);
	check(st.empty());

	st.push(2);
	st.push(3);

	check_equals(st.top(0), 3);
	check_equals(st.value(1), 3);
	check_equals(st.value(0), 2);
	check_equals(st.top(1), 2);

	st.push(4);
	st.push(5);

	check_equals(st.size(), 4);

	check_equals(st.top(0), 5);
	check_equals(st.value(3), 5);

	check_equals(st.top(1), 4);
	check_equals(st.value(2), 4);

	check_equals(st.top(2), 3);
	check_equals(st.value(1), 3);

	check_equals(st.top(3), 2);
	check_equals(st.value(0), 2);

	st.drop(4);
	check(st.empty());

	for (int i=0; i<100; ++i) st.push(i);
	check_equals(st.top(0), 99);
	check_equals(st.value(63), 63);
	check_equals(st.value(64), 64);
	check_equals(st.value(65), 65);

	check_equals(st.value(50), 50);
	st.setDownstop(50);
	check_equals(st.value(0), 50);
	check_equals(st.size(), 50);

	unsigned int oldDS = st.fixDownstop();
	check_equals(oldDS, 50); // old downstop
	check(st.empty());
	st.push(100);
	check_equals(st.value(0), 100);
	check_equals(st.top(0), 100);
	return 0;
}

