// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

// Test case for Sound ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="$Id: Sound.as,v 1.1 2008/06/17 12:42:22 strk Exp $";
#include "check.as"

// test Sound class and interface availability

check_equals(typeof(Sound), 'function');
check_equals(typeof(Sound.prototype), 'object');
check_equals(typeof(Sound.prototype.__proto__), 'object');
check_equals(Sound.prototype.__proto__, Object.prototype);

check_equals(typeof(Sound.prototype.attachSound), 'function');
check_equals(typeof(Sound.prototype.getPan), 'function');
check_equals(typeof(Sound.prototype.setPan), 'function');
check_equals(typeof(Sound.prototype.start), 'function');
check_equals(typeof(Sound.prototype.stop), 'function');
check_equals(typeof(Sound.prototype.getTransform), 'function');
check_equals(typeof(Sound.prototype.setTransform), 'function');
check_equals(typeof(Sound.prototype.getVolume), 'function');
check_equals(typeof(Sound.prototype.setVolume), 'function');

#if OUTPUT_VERSION > 5
 var functionSinceSWF6 = 'function';
#else
 var functionSinceSWF6 = 'undefined';
#endif
check_equals(typeof(Sound.prototype.getDuration), functionSinceSWF6);
check_equals(typeof(Sound.prototype.setDuration), functionSinceSWF6);
check_equals(typeof(Sound.prototype.loadSound), functionSinceSWF6);
check_equals(typeof(Sound.prototype.getPosition), functionSinceSWF6);
check_equals(typeof(Sound.prototype.setPosition), functionSinceSWF6);
check_equals(typeof(Sound.prototype.getBytesLoaded), functionSinceSWF6);
check_equals(typeof(Sound.prototype.getBytesTotal), functionSinceSWF6);

// The ones below are undefined in SWF5..SWF8, chances are an ASSetPropFlags
// to drop the onlyForSWF9 flag would expose them (TODO: test that)
check_equals(typeof(Sound.prototype.areSoundInaccessible), 'undefined');
check_equals(typeof(Sound.prototype.duration), 'undefined');
check_equals(typeof(Sound.prototype.ID3), 'undefined');
check_equals(typeof(Sound.prototype.checkPolicyFile), 'undefined');
check_equals(typeof(Sound.prototype.position), 'undefined');
check_equals(typeof(Sound.prototype.onID3), 'undefined');
check_equals(typeof(Sound.prototype.onLoad), 'undefined');
check_equals(typeof(Sound.prototype.onSoundComplete), 'undefined');


//-----------------------------------------
// Test Sound constructor
//-----------------------------------------

//
// Use default constructor and check return of all inspectors
//

// It seems like if Sound ctor is passed nothing, undefined or null
// it gets associated to some kind of global volume which is DIFFERENT
// from the one associated with _root...
//
s1 = new Sound();
s2 = new Sound();
check(s1 instanceof Sound);
check_equals(typeof(s1.getDuration()), 'undefined');
xcheck_equals(typeof(s1.getPan()), 'number'); 
xcheck_equals(s1.getPan(), 0); 
check_equals(typeof(s1.getPosition()), 'undefined');
xcheck_equals(typeof(s1.getTransform()), 'object'); // TODO: check composition

// Now, it seems s1 and s2 are both attached to something, and share
// the volume !
check_equals(typeof(s1.getVolume()), 'number'); 
check_equals(s1.getVolume(), 100); // why, since we didn't specify an attached char ??
s1.setVolume(95);
check_equals(s1.getVolume(), 95);
check_equals(s2.getVolume(), 95);

check_equals(typeof(s1.getBytesLoaded()), 'undefined');
check_equals(typeof(s1.getBytesTotal()), 'undefined');
xcheck_equals(typeof(s1.checkPolicyFile), 'boolean');

// The following are undefined in SWF5..SWF8, chances are
// some ASSetPropFlag to drop SWF9-only flag would expose
xcheck_equals(typeof(s1.duration), 'undefined');
check_equals(typeof(s1.ID3), 'undefined');
xcheck_equals(typeof(s1.position), 'undefined');


//
// Use constructor taking a movieclip and check return of all inspectors
//

s2 = new Sound(_root);
check(s2 instanceof Sound);
check_equals(typeof(s2.getDuration()), 'undefined');
xcheck_equals(typeof(s2.getPan()), 'number'); 
xcheck_equals(s2.getPan(), 0); 
check_equals(typeof(s2.getPosition()), 'undefined');
xcheck_equals(typeof(s2.getTransform()), 'object'); // TODO: check composition
check_equals(typeof(s2.getVolume()), 'number'); 
check_equals(s2.getVolume(), 100); 
check_equals(typeof(s2.getBytesLoaded()), 'undefined');
check_equals(typeof(s2.getBytesTotal()), 'undefined');
xcheck_equals(typeof(s2.checkPolicyFile), 'boolean');

// The following are undefined in SWF5..SWF8, chances are
// some ASSetPropFlag to drop SWF9-only flag would expose
xcheck_equals(typeof(s2.duration), 'undefined');
check_equals(typeof(s2.ID3), 'undefined');
xcheck_equals(typeof(s2.position), 'undefined');

// this is still valid, altought getVolume would return undefined
s3 = new Sound(33);
check(s3 instanceof Sound);
check_equals(typeof(s3.getVolume()), 'undefined');

//-----------------------------------------
// Test association of Sound to characters
//-----------------------------------------


s1a = new Sound(); 
s1b = new Sound();
check_equals(s1a.getVolume(), 95);
check_equals(s1b.getVolume(), 95);
s1b.setVolume(76);
check_equals(s1a.getVolume(), 76);
check_equals(s1b.getVolume(), 76);

// Numbers are invalid controllable-characters
s1c = new Sound(54); 
s1d = new Sound(54);
check_equals(typeof(s1c.getVolume()), 'undefined');
check_equals(typeof(s1d.getVolume()), 'undefined');
s1c.setVolume(54);
check_equals(typeof(s1c.getVolume()), 'undefined');
check_equals(typeof(s1d.getVolume()), 'undefined');

s1e = new Sound(null); 
s1f = new Sound(undefined);
check_equals(s1e.getVolume(), 76);
check_equals(s1f.getVolume(), 76);

// Objects are invalid controllable characters
o = new Object;
s1g = new Sound(o); 
s1h = new Sound(o);
check_equals(typeof(s1g.getVolume()), 'undefined');
check_equals(typeof(s1h.getVolume()), 'undefined');
s1g.setVolume(54);
check_equals(typeof(s1g.getVolume()), 'undefined');
check_equals(typeof(s1h.getVolume()), 'undefined');

s2 = new Sound(_root);
s3 = new Sound(_root);

check_equals(s2.getVolume(), 100);
check_equals(s3.getVolume(), 100);
s2.setVolume(80);
check_equals(s2.getVolume(), 80);
check_equals(s3.getVolume(), 80);


// Check association to unloaded characters, and rebinding
MovieClip.prototype.createEmptyMovieClip = ASnative(901, 0);
createEmptyMovieClip('c1', 1);
s2 = new Sound(c1);
s3 = new Sound(c1);
check_equals(s2.getVolume(), 100);
check_equals(s3.getVolume(), 100);
c1.removeMovieClip(); // unload, no unload handler
check_equals(typeof(s2.getVolume()), 'undefined');
check_equals(typeof(s3.getVolume()), 'undefined');
createEmptyMovieClip('c1', 2); // rebind ref to _level0.c1
check_equals(s2.getVolume(), 100);
check_equals(s3.getVolume(), 100);
s2.setVolume(80);
check_equals(s2.getVolume(), 80);
check_equals(s3.getVolume(), 80);
c1.removeMovieClip(); // unload, no unload handler
check_equals(typeof(s2.getVolume()), 'undefined');
check_equals(typeof(s3.getVolume()), 'undefined');
createEmptyMovieClip('c1', 3); // rebind ref to _level0.c1
check_equals(s2.getVolume(), 100); // old volume isn't remembered
check_equals(s3.getVolume(), 100);
s2.setVolume(80);
check_equals(s2.getVolume(), 80);
check_equals(s3.getVolume(), 80);
c1.onUnload = function() {}; // give c1 an unload handler
c1.removeMovieClip(); // there's an unload handler now
#if OUTPUT_VERSION < 6
 // No matter onUnload, SWF5 looses association
 xcheck_equals(typeof(s2.getVolume()), 'undefined');
 xcheck_equals(typeof(s3.getVolume()), 'undefined'); 
#else
 // in SWF6 up, presence of an onUnload handler
 // prevents loosing the association
 check_equals(s2.getVolume(), 80);
 check_equals(s3.getVolume(), 80); 
#endif

// TODO: test association with other kind of characters:
//       - TextField
//       - Video
//       - ...

//-----------------------------------------
// END OF TEST
//-----------------------------------------

check_totals(94);
