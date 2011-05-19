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


rcsid="array.as";
#include "check.as"

 callAndRecurse = function(prop, name) {
 
       // Don't recurse infinitely.
       if (prop == "__proto__") return;
       if (prop == "constructor") return;
 
       // i is the object, name is its name.
       name = name + "." + prop;
       var i = eval(name);
       
       trace("testing " + name + "(" + typeof(i) + ")");
 
       if (typeof(i) == "function") {
 
           // First call with various arguments.
           i();
           i(1);
           i(1, 2);
           i(1, 2, 3);
           i(1, 2, 3, 4);
 
           trace("Trying to construct " + name);
           o = new i();
 
           if (o) {
               trace("Object was constructed");
               ASSetPropFlags(o, null, 0, 1);
               ASSetPropFlags(o.__proto__, null, 0, 1);
               for (p in o) {
                   trace("Testing " + p + "()");
                   o[p]();
                   o[p](1);
                   o[p](1, 2);
                   o[p](1, 2, 3);
                   o[p](1, 2, 3, 4);
 
                   o[p].call();
                   o[p].call(null);
                   o[p].call(null, 1);
                   o[p].call(null, 1, 2);
                   o[p].call(null, 1, 2, 3);
                   o[p].call(null, 1, 2, 3, 4);
 
                   // Recurse for own properties only.
                   if (o.hasOwnProperty(p)) {
                       callAndRecurse(p, name);
                   }
               };
           };
       }
       else if (typeof(i) == "object") {
           ASSetPropFlags(i, null, 0, 1);
           for (p in i) {
               callAndRecurse(p, name);
           };
       };
  };
 
 
 start = "_global";
 obj = eval(start);
 
 ASSetPropFlags(obj, null, 0, 1);
 for (prop in obj) {
     callAndRecurse(prop, start);
 };

pass("Nothing crashed gnash");
