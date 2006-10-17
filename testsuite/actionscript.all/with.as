// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

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

// Test case for XML ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

var XMLObj = new XML();

#include "dejagnu.as"
#include "utils.as"

XMLObj.onLoad = function (success) {
    if (success) {
        with (XMLObj.firstChild) {
            if (nodeName == 'TOPNODE') {
                childa = 0;
                while (childa < childNodes.length) {
                    with (childNodes[childa]) {
                        if (nodeName == 'SUBNODE1') {
                            childb = 0;
                            while (childb < childNodes.length) {
                                with (childNodes[childb]) {
                                    if (nodeName == 'SUBSUBNODE1') {
                                        _global.child1 = firstChild.nodeValue;
                                    } else {
                                        if (nodeName == 'SUBNODE2') {
                                            _global.child2 = firstChild.nodeValue;
                                        } else {
                                            if (nodeName == 'SUBSUBNODE1') {
                                                _global.child3 = firstChild.nodeValue;
                                            }
                                        }
                                    }
                                }
                                ++childb;
                            }
                        }
                    }
                    ++childa;
                }
            }
        }
    }
    Root_Path.mv_Everything.mv_System_Info.txt_Sysinfo_child3.text = child3;
    if (Connected == 0) {
        Root_Path.mv_Everything.mv_Loading_Splash.txt_Debug.text = 'Connecting to\'' + child1 + '\' on port ' + child3;
    }
};

// Load
// if (XMLObj.load("testin.xml")) {
//     pass("XML::load() works");
// } else {
//     fail("XML::load() doesn't work");
// }

var xml = "<TOPNODE><SUBNODE1><SUBSUBNODE1>sub sub1 node data 1</SUBSUBNODE1><SUBSUBNODE2>sub sub1 node data 2</SUBSUBNODE2></SUBNODE1><SUBNODE2><SUBSUBNODE1>sub sub2 node data 1</SUBSUBNODE1><SUBSUBNODE2>sub sub2 node data 2</SUBSUBNODE2></SUBNODE2></TOPNODE>";

XMLObj.parseXML(xml);
if (XMLObj.firstChild.nodeName == "TOPNODE") {
    pass("XML::parseXML() works");
} else {
    fail("XML::parseXML() doesn't work");
}

// These tests only work if 'with' processed the XML file correctly
if (_global.child1 == "sub sub1 node data 1") {
    xpass("with level 1 works");
} else {
    xfail("with level 1 doesn't work");
}

if (_global.child2 == "") {
    xpass("with level 2 works");
} else {
    xfail("with level 2 doesn't work");
}

if (_global.child3 == "sub sub2 node data 1") {
    xpass("with level 3 works");
} else {
    xfail("with level 3 doesn't work");
}

// We're done
totals();
