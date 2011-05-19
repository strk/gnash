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

// Test case for System ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="System.as";
#include "check.as"

check_equals(typeof(System), 'object');

#if OUTPUT_VERSION > 5
check(System.capabilities.hasOwnProperty("version"));
check(System.capabilities.hasOwnProperty("os"));
check(System.capabilities.hasOwnProperty("manufacturer"));
check(System.capabilities.hasOwnProperty("playerType"));
check(System.capabilities.hasOwnProperty("serverString"));
check(System.capabilities.hasOwnProperty("screenResolutionX"));
check(System.capabilities.hasOwnProperty("screenResolutionY"));
check(System.capabilities.hasOwnProperty("screenDPI"));
check(System.capabilities.hasOwnProperty("screenColor"));
check(System.capabilities.hasOwnProperty("pixelAspectRatio"));
check(System.capabilities.hasOwnProperty("localFileReadDisable"));
check(System.capabilities.hasOwnProperty("language"));
check(System.capabilities.hasOwnProperty("isDebugger"));
check(System.capabilities.hasOwnProperty("hasVideoEncoder"));
check(System.capabilities.hasOwnProperty("hasStreamingVideo"));
check(System.capabilities.hasOwnProperty("hasStreamingAudio"));
check(System.capabilities.hasOwnProperty("hasScreenPlayback"));
check(System.capabilities.hasOwnProperty("hasScreenBroadcast"));
check(System.capabilities.hasOwnProperty("hasPrinting"));
check(System.capabilities.hasOwnProperty("hasMP3"));
check(System.capabilities.hasOwnProperty("hasEmbeddedVideo"));
check(System.capabilities.hasOwnProperty("hasAudioEncoder"));
check(System.capabilities.hasOwnProperty("hasAudio"));
check(System.capabilities.hasOwnProperty("hasAccessibility"));
check(System.capabilities.hasOwnProperty("avHardwareDisable"));
check(System.capabilities.hasOwnProperty("windowlessDisable"));

check(System.hasOwnProperty("exactSettings"));
#endif


// _global.System is NOT a class, just an object 
var systemObj = new System;
check_equals ( typeof(systemObj), 'undefined' );

check_equals (typeof(System.__proto__), 'object');
check_equals (System.__proto__, Object.prototype)

// test the System.setClipboard method
check_equals ( typeof(System.setClipboard), 'function');

// test the System.showSettings method
check_equals ( typeof(System.showSettings), 'function');

// test the System::security.allowDomain method
check_equals ( typeof(System.security.allowDomain), 'function' );

// test the System.security.loadPolicyFile method
check_equals ( typeof(System.security.loadPolicyFile), 'function');

#if OUTPUT_VERSION >= 7

// test the System.security.allowInsecureDomain method
// added in player 7
check_equals ( typeof(System.security.allowInsecureDomain), 'function' );

#endif // OUTPUT_VERSION >= 7

// test System.capabilities
check_equals(typeof(System.capabilities), 'object');

// test System.version (should also match the global $version)
check_equals(typeof(System.capabilities.version), 'string');
check_equals(typeof($version), 'string');
check_equals(System.capabilities.version, $version);
check_equals(typeof(_global.$version), 'undefined');
check_equals(typeof(this.$version), 'string');
check_equals(this.$version, System.capabilities.version);
check_equals(typeof(System.capabilities.os), 'string');
check_equals(typeof(System.capabilities.manufacturer), 'string');
check_equals(typeof(System.capabilities.playerType), 'string');
check_equals(typeof(System.capabilities.serverString), 'string');
check_equals(typeof(System.capabilities.screenResolutionX), 'number');
check_equals(typeof(System.capabilities.screenResolutionY), 'number');
check_equals(typeof(System.capabilities.screenDPI), 'number');
check_equals(typeof(System.capabilities.screenColor), 'string');
check_equals(typeof(System.capabilities.pixelAspectRatio), 'string');
check_equals(typeof(System.capabilities.localFileReadDisable), 'boolean');
check_equals(typeof(System.capabilities.language), 'string');
check_equals(typeof(System.capabilities.isDebugger), 'boolean');
check_equals(typeof(System.capabilities.hasVideoEncoder), 'boolean');
check_equals(typeof(System.capabilities.hasStreamingVideo), 'boolean');
check_equals(typeof(System.capabilities.hasStreamingAudio), 'boolean');
check_equals(typeof(System.capabilities.hasScreenPlayback), 'boolean');
check_equals(typeof(System.capabilities.hasScreenBroadcast), 'boolean');
check_equals(typeof(System.capabilities.hasPrinting), 'boolean');
check_equals(typeof(System.capabilities.hasMP3), 'boolean');
check_equals(typeof(System.capabilities.hasEmbeddedVideo), 'boolean');
check_equals(typeof(System.capabilities.hasAudioEncoder), 'boolean');
check_equals(typeof(System.capabilities.hasAudio), 'boolean');
check_equals(typeof(System.capabilities.hasAccessibility), 'boolean');
check_equals(typeof(System.capabilities.avHardwareDisable), 'boolean');
check_equals(typeof(System.capabilities.windowlessDisable), 'boolean');

// Not present on any known Linux player versions.
check_equals(typeof(System.capabilities.hasIME), 'undefined');
note("    System.capabilities.hasIME certainly fails on the pp on\n\
    some platforms. There's no verification that it exists at all so far.");

// Added in Player version 9.
check_equals(typeof(System.capabilities.hasTLS), 'boolean');


// System.exactSettings
#if OUTPUT_VERSION > 5
check_equals(typeof(System.exactSettings), 'boolean');
System.exactSettings = true;
check_equals(System.exactSettings, true);
System.exactSettings = false;
xcheck_equals(System.exactSettings, false);
#else
check_equals(typeof(System.exactSettings), 'undefined');
#endif

#if OUTPUT_VERSION >= 6
check(this.hasOwnProperty("$version"));
check(! MovieClip.prototype.hasOwnProperty("$version") );
#endif

//
// Undocumented System methods
//

// Directs the player to use Latin1 instead of unicode.
check_equals(typeof(System.useCodepage), 'boolean');
System.useCodepage = false;
check_equals(System.useCodepage, false);
System.useCodepage = true;
xcheck_equals(System.useCodepage, true);

// Pops up settings dialogue box with variable settings.
// System.showSettings(0): camera / microphone access;
// 1: shared object settings.
// 2: camera.
// 3: microphone.
check_equals(typeof(System.showSettings), 'function');

xcheck_equals(typeof(System.Product), 'function');

#if OUTPUT_VERSION > 5
xcheck (System.Product.prototype.hasOwnProperty('launch'));
xcheck (System.Product.prototype.hasOwnProperty('download'));
#endif

p = new System.Product("whatisthis");
xcheck_equals(typeof(p), 'object');

// Tries to do something with 'whatisthis'
xcheck_equals(typeof(p.download), 'function');
// Tries to exec whatisthis from a particular location?
xcheck_equals(typeof(p.launch), 'function');

ret = System.security.allowDomain(1);
check_equals(typeof(ret), "boolean");
check_equals(ret, true);
ret = System.security.allowDomain(false);
check_equals(ret, true);
ret = System.security.allowDomain("string");
check_equals(ret, true);
ret = System.security.allowDomain("http://www.gnashdev.org");
check_equals(ret, true);
ret = System.security.allowDomain();
check_equals(ret, false);
ret = System.security.allowDomain("string again");
check_equals(ret, true);
ret = System.security.allowDomain(undefined);
check_equals(ret, true);

#if OUTPUT_VERSION > 6
 check_totals(93);
#else
# if OUTPUT_VERSION == 6
   check_totals(92);
# else
   check_totals(59);
# endif
#endif
