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

    public class Hello extends MovieClip {

        DEJAGNU_OBJ;

        public function Hello() {

            var i = 1;
            i++;
            check_equals(i, 2);

            i += 2;
            check_equals(i, 4);

            i *= 3;
            check_equals(i, 12);
        
            i /= 8;
            check_equals(i, 1.5);

            i -= 14;
            check_equals(i, -12.5);

            totals(5);

            done();
        }
    }
}
         
