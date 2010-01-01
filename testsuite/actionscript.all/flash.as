// 
//   Copyright (C) 2008, 2009, 2010 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
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
//
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// This is for miscellaneous testing of SWF8 flash package and related
// functions.

#include "check.as"

#if OUTPUT_VERSION == 5
MovieClip.prototype.hasOwnProperty = ASNative(101, 5);
#endif

#if OUTPUT_VERSION < 8

check_equals(typeof(flash), 'undefined');

// The transform property of MovieClip is not visible to SWF6 or SWF7
check_equals(MovieClip.prototype.transform, undefined);
check(MovieClip.prototype.hasOwnProperty("transform"));
MovieClip.prototype.transform = 8;
xcheck_equals(MovieClip.prototype.transform, 8);

check_totals(4);

#else

r = "";
for (i in flash) {
    r += i + ",";
};
check_equals(r, "external,net,geom,filters,display,text,");

check_equals(flash.external.toString(), "[object Object]");
check_equals(flash.net.toString(), "[object Object]");
check_equals(flash.geom.toString(), "[object Object]");
check_equals(flash.filters.toString(), "[object Object]");
check_equals(flash.display.toString(), "[object Object]");
check_equals(flash.text.toString(), "[object Object]");

// Check that AS3 packages are not available here.
check_equals(flash.ui.toString(), undefined);
check_equals(flash.accessibility.toString(), undefined);
check_equals(flash.desktop.toString(), undefined);
check_equals(flash.errors.toString(), undefined);
check_equals(flash.events.toString(), undefined);
check_equals(flash.media.toString(), undefined);
check_equals(flash.printing.toString(), undefined);
check_equals(flash.sampler.toString(), undefined);
check_equals(flash.system.toString(), undefined);
check_equals(flash.utils.toString(), undefined);
check_equals(flash.xml.toString(), undefined);

ASSetPropFlags(_global.flash, null, 6, true);

r = "";
for (i in flash) {
    r += i + ",";
};

// Gnash has no constructor, but otherwise okay.
xcheck_equals(r, "external,net,geom,filters,display,text,__proto__,constructor,");
check_equals(flash.ui.toString(), undefined);
check_equals(flash.accessibility.toString(), undefined);
check_equals(flash.desktop.toString(), undefined);
check_equals(flash.errors.toString(), undefined);
check_equals(flash.events.toString(), undefined);
check_equals(flash.media.toString(), undefined);
check_equals(flash.printing.toString(), undefined);
check_equals(flash.sampler.toString(), undefined);
check_equals(flash.system.toString(), undefined);
check_equals(flash.utils.toString(), undefined);
check_equals(flash.xml.toString(), undefined);

// MovieClip.transform is protected from deletion and change.
check_equals(MovieClip.prototype.transform, undefined);
check(MovieClip.prototype.hasOwnProperty("transform"));
MovieClip.prototype.transform = 8;
check_equals(MovieClip.prototype.transform, undefined);
delete MovieClip.prototype.transform;
check_equals(MovieClip.prototype.transform, undefined);
check(MovieClip.prototype.hasOwnProperty("transform"));

totals(35);

#endif

