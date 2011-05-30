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

// Test case for Video ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="Video.as";
#include "check.as"

#if OUTPUT_VERSION < 6

check_equals(Video, undefined);
check_totals(1);

#else

check(Video.prototype.hasOwnProperty("attachVideo"));
check(Video.prototype.hasOwnProperty("clear"));
check(!Video.prototype.hasOwnProperty("smoothing"));
check(!Video.prototype.hasOwnProperty("deblocking"));
check(!Video.prototype.hasOwnProperty("_alpha"));
check(!Video.prototype.hasOwnProperty("_height"));
check(!Video.prototype.hasOwnProperty("height"));
check(!Video.prototype.hasOwnProperty("_name"));
check(!Video.prototype.hasOwnProperty("_parent"));
check(!Video.prototype.hasOwnProperty("_rotation"));
check(!Video.prototype.hasOwnProperty("_visible"));
check(!Video.prototype.hasOwnProperty("_width"));
check(!Video.prototype.hasOwnProperty("width"));
check(!Video.prototype.hasOwnProperty("_x"));
check(!Video.prototype.hasOwnProperty("_xmouse"));
check(!Video.prototype.hasOwnProperty("_xscale"));
check(!Video.prototype.hasOwnProperty("_y"));
check(!Video.prototype.hasOwnProperty("_ymouse"));
check(!Video.prototype.hasOwnProperty("_yscale"));
check(!Video.prototype.hasOwnProperty("_xmouse"));

// test Video class an interface availability
check_equals(typeof(Video), 'function');
check_equals(typeof(Video.prototype), 'object');
check_equals(typeof(Video.prototype.__proto__), 'object');
check_equals(Video.prototype.__proto__, Object.prototype);
check_equals(typeof(Video.prototype.attachVideo), 'function');
check_equals(typeof(Video.prototype.clear), 'function');

// test Video instance
var videoObj = new Video;
check_equals (typeof(videoObj), 'object');
check_equals (typeof(videoObj.attachVideo), 'function');
check_equals (typeof(videoObj.clear), 'function');
check_equals (typeof(videoObj._x), 'undefined');
check_equals (typeof(videoObj._y), 'undefined');
check_equals (typeof(videoObj._width), 'undefined');
check_equals (typeof(videoObj._height), 'undefined');
check_equals (typeof(videoObj._xscale), 'undefined');
check_equals (typeof(videoObj._yscale), 'undefined');
check_equals (typeof(videoObj._xmouse), 'undefined');
check_equals (typeof(videoObj._ymouse), 'undefined');
check_equals (typeof(videoObj._alpha), 'undefined');
check_equals (typeof(videoObj._rotation), 'undefined');
check_equals (typeof(videoObj._target), 'undefined');
check_equals (typeof(videoObj._parent), 'undefined');

nc = new NetConnection();
nc.connect(null);
ns = new NetStream(nc);
videoObj.attachVideo(ns);
ns.play(MEDIA(square.flv));

check(!videoObj.hasOwnProperty("attachVideo"));
check(!videoObj.hasOwnProperty("smoothing"));
check(!videoObj.hasOwnProperty("deblocking"));
check(!videoObj.hasOwnProperty("clear"));
check(!videoObj.hasOwnProperty("_alpha"));
check(!videoObj.hasOwnProperty("_height"));
check(!videoObj.hasOwnProperty("height"));
check(!videoObj.hasOwnProperty("_name"));
check(!videoObj.hasOwnProperty("_parent"));
check(!videoObj.hasOwnProperty("_rotation"));
check(!videoObj.hasOwnProperty("_visible"));
check(!videoObj.hasOwnProperty("_width"));
check(!videoObj.hasOwnProperty("width"));
check(!videoObj.hasOwnProperty("_x"));
check(!videoObj.hasOwnProperty("_xmouse"));
check(!videoObj.hasOwnProperty("_xscale"));
check(!videoObj.hasOwnProperty("_y"));
check(!videoObj.hasOwnProperty("_ymouse"));
check(!videoObj.hasOwnProperty("_yscale"));
check(!videoObj.hasOwnProperty("_xmouse"));

check(!Video.prototype.hasOwnProperty("smoothing"));
check(!Video.prototype.hasOwnProperty("deblocking"));
check(!Video.prototype.hasOwnProperty("_alpha"));
check(!Video.prototype.hasOwnProperty("_height"));
check(!Video.prototype.hasOwnProperty("height"));
check(!Video.prototype.hasOwnProperty("_name"));
check(!Video.prototype.hasOwnProperty("_parent"));
check(!Video.prototype.hasOwnProperty("_rotation"));
check(!Video.prototype.hasOwnProperty("_visible"));
check(!Video.prototype.hasOwnProperty("_width"));
check(!Video.prototype.hasOwnProperty("width"));
check(!Video.prototype.hasOwnProperty("_x"));
check(!Video.prototype.hasOwnProperty("_xmouse"));
check(!Video.prototype.hasOwnProperty("_xscale"));
check(!Video.prototype.hasOwnProperty("_y"));
check(!Video.prototype.hasOwnProperty("_ymouse"));
check(!Video.prototype.hasOwnProperty("_yscale"));
check(!Video.prototype.hasOwnProperty("_xmouse"));

check_totals(79);

#endif
