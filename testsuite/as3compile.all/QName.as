//
//   Copyright (C) 2005, 2006, 2007, 2009 Free Software Foundation, Inc.
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

#include "check.as"

package hello {

    import flash.display.MovieClip;

    public class Main extends MovieClip {

        DEJAGNU_OBJ;

        public function Main() {

            check(!QName.prototype.hasOwnProperty("uri"));    
            check(!QName.prototype.hasOwnProperty("localName"));    

            var q = new QName();
            check(q.hasOwnProperty("uri"));
            check(q.hasOwnProperty("localName"));
            check_equals(typeof(q.localName), "string");
            check_equals(typeof(q.uri), "string");
            check_equals(q.localName, "");
            check_equals(q.uri, "");
            check_equals(q.toString(), "");

            // One argument: first arg is localName
            q = new QName("a");
            check_equals(typeof(q.localName), "string");
            check_equals(typeof(q.uri), "string");
            check_equals(q.localName, "a");
            check_equals(q.uri, "");
            check_equals(q.toString(), "a");
            
            // Two arguments: first arg is uri, second is localName
            q = new QName("a", "b");
            check_equals(typeof(q.localName), "string");
            check_equals(typeof(q.uri), "string");
            check_equals(q.localName, "b");
            check_equals(q.uri, "a");
            check_equals(q.toString(), "a::b");
            
            q = new QName("d", "e", "f");
            check_equals(typeof(q.localName), "string");
            check_equals(typeof(q.uri), "string");
            check_equals(q.localName, "e");
            check_equals(q.uri, "d");
            check_equals(q.toString(), "d::e");

            // Copy constructor (not sure what bytecode is produced).
            var q2 = new QName(q);
            check_equals(typeof(q2.localName), "string");
            check_equals(typeof(q2.uri), "string");
            check_equals(q2.localName, "e");
            check_equals(q2.uri, "d");
            check_equals(q.toString(), "d::e");

            q2 = new QName("g", q);
            check_equals(q2.localName, "e");
            check_equals(q2.uri, "g");
            check_equals(q2.toString(), "g::e");

            // A constructor QName(Namespace, string) is documented, but this
            // seems like a bit of a fraud as the Namespace is simply
            // converted to a string so can be used as either argument.
            var ns = new Namespace("nsA");
            q2 = new QName(ns);
            check_equals(q2.localName, "nsA");
            check_equals(q2.uri, "");
            check_equals(q2.toString(), "nsA");

            q2 = new QName(ns, "h");
            check_equals(q2.localName, "h");
            check_equals(q2.uri, "nsA");
            check_equals(q2.toString(), "nsA::h");
            
            q2 = new QName(ns, ns);
            check_equals(q2.localName, "nsA");
            check_equals(q2.uri, "nsA");
            check_equals(q2.toString(), "nsA::nsA");

            ns = new Namespace("prefix", "uri");
            q2 = new QName(ns, ns);
            check_equals(q2.localName, "uri");
            check_equals(q2.uri, "uri");
            check_equals(q2.toString(), "uri::uri");

            // null can be passed as a uri, meaning 'all namespaces'; if it's
            // passed as a localName, it's converted to a string.
            var q3 = new QName(null, null);            
            check_equals(q3.localName, "null");
            check_equals(q3.uri, null);
            check_equals(q3.toString(), "*::null");

            q3 = new QName(null);            
            check_equals(q3.localName, "null");
            check_equals(q3.uri, "");
            check_equals(q3.toString(), "null");

            // Numbers can be used in either position; they are converted to
            // a string.
            q3 = new QName(null, 20);
            check_equals(q3.localName, "20");
            check_equals(q3.uri, null);
            check_equals(q3.toString(), "*::20");

            q3 = new QName(400, 20);
            check_equals(q3.localName, "20");
            check_equals(q3.uri, "400");
            check_equals(q3.toString(), "400::20");
            totals(56);

            done();
        }
    }
}
         
