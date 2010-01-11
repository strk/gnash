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

// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


var runtest = new DejaGnu();
var db = new MySQL();

// See if the new AS class and it's methods exist
if (db) {
    runtest.pass("DejaGnu() constructor");
} else {
    runtest.fail("DejaGnu() constructor");
}

if (db) {
if (db.connect) {
    runtest.pass("MySQL::connect exists");
} else {
    runtest.fail("MySQL::connect doesn't exist");
}

// if (db.getData) {
//     runtest.pass("MySQL::getData exists");
// } else {
//     runtest.fail("MySQL::getData doesn't exist");
// }    

if (db.disconnect) {
    runtest.pass("MySQL::disconnect exists");
} else {
    runtest.fail("MySQL::disconnect doesn't exist");
}

// See if the API minimally works
if (db.connect("localhost", "gnash", "gnash", "gnash")) {
    runtest.pass("MySQL::connect connects");
} else {
    runtest.fail("MySQL::connect doesn't connect");
}

var arr1 = new Array();
var arr2 = new Array();

if (db.query("SELECT * from test;")) {
    runtest.pass("MySQL::query works");
} else {
    runtest.fail("MySQL::query doesn't work");
}
    
if (db.store_results()) {
    runtest.pass("MySQL::store_results works");
} else {
    runtest.fail("MySQL::store_results doesn't work");
}

if (db.num_fields() > 0) {
    runtest.pass("MySQL::num_fields works");
} else {
    runtest.fail("MySQL::num_fields doesn't work");
}

arr1 = db.fetch_row();
trace(arr1[0]);

if (db.qetData("SELECT * from test;", arr2)) {
    runtest.pass("MySQL::qetData works");
} else {
    runtest.fail("MySQL::qetData doesn't work");
}

for (i=0; i<arr2.size(); i++) {
    trace(arr2[i]);
}
trace(arr2[0]);
trace(arr2[1]);

// all done
if (db.disconnect()) {
    runtest.pass("MySQL::disconnect disconnects");
} else {
    runtest.fail("MySQL::disconnect doesn't disconnect");
}

} else {
    trace("UNTESTED: Extensions not built!");
}
