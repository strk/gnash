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

#include "check.as"

package hello {

    import flash.display.MovieClip;

    public class Main extends MovieClip {

        DEJAGNU_OBJ;

        public function Main() {

            check(!QName.prototype.hasOwnProperty("uri"));    
            check(!QName.prototype.hasOwnProperty("localName"));    
            
            xcheck_equals(QName.prototype, "");    
            xcheck_equals(QName.constructor, "[class Class]");    
            check_equals(QName.constructor.prototype, "[object Object]");    

            var q = new QName();
            xcheck_equals(q.constructor, "[class QName]");    
            xcheck_equals(q.constructor.constructor, "[class Class]");    
            check_equals(q.constructor.constructor.prototype,
                                "[object Object]");    
            
            check(q.hasOwnProperty("uri"));
            check(q.hasOwnProperty("localName"));
            xcheck_equals(typeof(q.localName), "string");
            xcheck_equals(typeof(q.uri), "string");
            xcheck_equals(q.localName, "");
            xcheck_equals(q.uri, "");
            xcheck_equals(q.toString(), "");

            // One argument: first arg is localName
            q = new QName("a");
            xcheck_equals(typeof(q.localName), "string");
            xcheck_equals(typeof(q.uri), "string");
            xcheck_equals(q.localName, "a");
            xcheck_equals(q.uri, "");
            xcheck_equals(q.toString(), "a");
            
            // Two arguments: first arg is uri, second is localName
            q = new QName("a", "b");
            xcheck_equals(typeof(q.localName), "string");
            xcheck_equals(typeof(q.uri), "string");
            xcheck_equals(q.localName, "b");
            xcheck_equals(q.uri, "a");
            xcheck_equals(q.toString(), "a::b");
            
            q = new QName("d", "e", "f");
            xcheck_equals(typeof(q.localName), "string");
            xcheck_equals(typeof(q.uri), "string");
            xcheck_equals(q.localName, "e");
            xcheck_equals(q.uri, "d");
            xcheck_equals(q.toString(), "d::e");

            // Copy constructor (not sure what bytecode is produced).
            var q2 = new QName(q);
            xcheck_equals(typeof(q2.localName), "string");
            xcheck_equals(typeof(q2.uri), "string");
            xcheck_equals(q2.localName, "e");
            xcheck_equals(q2.uri, "d");
            xcheck_equals(q.toString(), "d::e");

            q2 = new QName("g", q);
            xcheck_equals(q2.localName, "e");
            xcheck_equals(q2.uri, "g");
            xcheck_equals(q2.toString(), "g::e");

            // A constructor QName(Namespace, string) is documented, but this
            // seems like a bit of a fraud as the Namespace is simply
            // converted to a string so can be used as either argument.
            var ns = new Namespace("nsA");
            q2 = new QName(ns);
            xcheck_equals(q2.localName, "nsA");
            xcheck_equals(q2.uri, "");
            xcheck_equals(q2.toString(), "nsA");

            q2 = new QName(ns, "h");
            xcheck_equals(q2.localName, "h");
            xcheck_equals(q2.uri, "nsA");
            xcheck_equals(q2.toString(), "nsA::h");
            
            q2 = new QName(ns, ns);
            xcheck_equals(q2.localName, "nsA");
            xcheck_equals(q2.uri, "nsA");
            xcheck_equals(q2.toString(), "nsA::nsA");

            ns = new Namespace("prefix", "uri");
            q2 = new QName(ns, ns);
            xcheck_equals(q2.localName, "uri");
            xcheck_equals(q2.uri, "uri");
            xcheck_equals(q2.toString(), "uri::uri");

            // null can be passed as a uri, meaning 'all namespaces'; if it's
            // passed as a localName, it's converted to a string.
            var q3 = new QName(null, null);            
            xcheck_equals(q3.localName, "null");
            check_equals(q3.uri, null);
            xcheck_equals(q3.toString(), "*::null");

            q3 = new QName(null);            
            xcheck_equals(q3.localName, "null");
            xcheck_equals(q3.uri, "");
            xcheck_equals(q3.toString(), "null");

            // Numbers can be used in either position; they are converted to
            // a string.
            q3 = new QName(null, 20);
            xcheck_equals(q3.localName, "20");
            check_equals(q3.uri, null);
            xcheck_equals(q3.toString(), "*::20");

            q3 = new QName(400, 20);
            xcheck_equals(q3.localName, "20");
            xcheck_equals(q3.uri, "400");
            xcheck_equals(q3.toString(), "400::20");
            
            // Check that QName has Object as its prototype.
            var b = Object.prototype;
            b.a = 7;
            var q4 = new QName();
            check_equals(q4.a, 7);

            QName.constructor.prototype.a = "stringy";
            QName.constructor.a = "stringy";
            check_equals(b.a, 7);
            q4 = new QName();
            check_equals(q4.a, 7);    
            
            QName.prototype.a = "string";            
            check_equals(b.a, 7);
            check_equals(Object.prototype.a, 7);
            check_equals(q4.a, "string");   


            totals(68);

            done();
        }
    }
}
         
