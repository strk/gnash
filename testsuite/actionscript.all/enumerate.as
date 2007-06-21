 // 
//   Copyright (C) 2007 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modchecky
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; check not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fcheckth Floor, Boston, MA  02110-1301  USA
//

/*
 *  Zou Lunkai, zoulunkai@gmail.com
 *
 *  Test ActionEnumerate/2
 */

#include "check.as"

#if OUTPUT_VERSION > 5

//
// enumerate characters of a sprite
//
{
  // Create a recorder and init it.
  recorder =  new Array(10);
  for(i=0; i<10; i++){
    recorder[i] = 0;
  }

  // Create six characters
  for(i=0; i<6; i++){
    _root.createEmptyMovieClip("mc"+i, i+1);
  }

  // Use the recorder to record the enumerated results in _root
  i = 0;
  for (var j in _root){
    recorder[i] = _root[j];
    i++;
  } 

  // Test what we got in the above step. Tests show that characters were also got enumerated!
  // Note: don't use clip names for testing here, the order in enumerating is not defined.
  // However, the tests bellow are dependent on enumerating order!
  // Since recorder[0] and recorder[1] and recorder[2] might not be movieclips.
  // TODO: find a proper way to test this.
  xcheck_equals(typeof(recorder[3]), 'movieclip');
  xcheck_equals(typeof(recorder[4]), 'movieclip');
  xcheck_equals(typeof(recorder[5]), 'movieclip');
  xcheck_equals(typeof(recorder[6]), 'movieclip');  
  xcheck_equals(typeof(recorder[7]), 'movieclip');
  xcheck_equals(typeof(recorder[8]), 'movieclip');
  
  delete recorder;
}

//
// enumerate properties of an AS object
//
{
  recorder =  new Array(10);
  
  // initialize the recorder
  for(i=0; i<10; i++){
    recorder[i] = 0;
  }

  obj = new Object();
  obj.x1 = function () {};
  obj.x2 = obj.x1;
  obj.x3 = obj.x2;

  i = 0;
  for (var j in obj){
    recorder[i] = obj[j];
    i++;
  } 
  
  check_equals(typeof(recorder[0]), 'function');
  check_equals(typeof(recorder[1]), 'function');
  check_equals(typeof(recorder[2]), 'function');
  
  delete recorder;
}

#endif  // OUTPUT_VERSION > 5