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


// Test case for Microphone ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: Microphone.as,v 1.12 2007/09/29 16:22:57 strk Exp $";

#include "check.as"

// There was no Microphone class in SWF5 or lower
#if OUTPUT_VERSION > 5

// test the Microphone class
check_equals(typeof(Microphone), 'function');
check_equals ( typeof(Microphone.prototype.setGain), 'function' );
check_equals ( typeof(Microphone.prototype.setRate), 'function' );
check_equals ( typeof(Microphone.prototype.setSilenceLevel), 'function' );
check_equals ( typeof(Microphone.prototype.setUseEchoSuppression), 'function' );

// test the Microphone constuctor
var microphoneObj = Microphone.get();
xcheck_equals (typeof(microphoneObj), 'object');

// test that Microphone.get() returns a singleton
check_equals(microphoneObj, Microphone.get());

// test that get() method is NOT exported to instances
check_equals (typeof(microphoneObj.get), 'undefined');

// test the Microphone::setGain method
xcheck_equals ( typeof(microphoneObj.setGain), 'function' );

// test the Microphone::setRate method
xcheck_equals ( typeof(microphoneObj.setRate), 'function' );

// test the Microphone::setSilenceLevel method
xcheck_equals ( typeof(microphoneObj.setSilenceLevel), 'function' );

// test the Microphone::setUseEchoSuppression method
xcheck_equals ( typeof(microphoneObj.setUseEchoSuppression), 'function' );

#endif // OUTPUT_VERSION > 5
totals();
