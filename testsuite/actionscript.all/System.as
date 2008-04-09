// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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


rcsid="$Id: System.as,v 1.19 2008/04/09 11:57:35 bwy Exp $";
#include "check.as"

check_equals(typeof(System), 'object');

// _global.System is NOT a class, just an object 
var systemObj = new System;
check_equals ( typeof(systemObj), 'undefined' );

// test the System::security.allowDomain method
check_equals ( typeof(System.security.allowDomain), 'function' );

// test the System.security.loadPolicyFile method
check_equals ( typeof(System.security.loadPolicyFile), 'function');

// test the System.setClipboard method
check_equals ( typeof(System.setClipboard), 'function');

// test the System.showSettings method
check_equals ( typeof(System.showSettings), 'function');

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
xcheck_equals(typeof(System.capabilities.screenDPI), 'number');
xcheck_equals(typeof(System.capabilities.screenColor), 'string');
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

#if OUTPUT_VERSION >= 6
check(this.hasOwnProperty("$version"));
check(! MovieClip.prototype.hasOwnProperty("$version") );
#endif

#if OUTPUT_VERSION > 6
 check_totals(40);
#else
# if OUTPUT_VERSION == 6
   check_totals(39);
# else
   check_totals(37);
# endif
#endif
