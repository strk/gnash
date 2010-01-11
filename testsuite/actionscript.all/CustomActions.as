// 
//   Copyright (C) 2005, 2006, 2007, 2009, 2010 Free Software
//   Foundation, Inc
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
//


// Test case for CustomActions ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


// CustomActions is only intended foro use in the Flash authoring tool,
// See http://sephiroth.it/reference.php?id=156&cat=1


rcsid="$Id: CustomActions.as,v 1.13 2008/03/11 19:31:47 strk Exp $";
#include "check.as"

note();
note(' CustomActions is only intended foro use in the Flash authoring tool, so this file performs no tests');
note(' See http://sephiroth.it/reference.php?id=156&cat=1');

#if 0


var customactionsObj = new CustomActions;

// test the CustomActions constuctor
check (customactionsObj != undefined);

// test the CustomActions::get method
check (customactionsObj.get != undefined);
// test the CustomActions::install method
check (customactionsObj.install != undefined);
// test the CustomActions::list method
check (customactionsObj.list != undefined);
// test the CustomActions::uninstall method
check (customactionsObj.uninstall != undefined);

#endif
totals();
