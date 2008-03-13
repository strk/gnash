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


rcsid="$Id: Microphone.as,v 1.16 2008/03/13 17:25:39 bwy Exp $";
#include "check.as"

// There was no Microphone class in SWF5 or lower
#if OUTPUT_VERSION > 5

// test the Microphone class
check_equals(typeof(Microphone), 'function');
check_equals ( typeof(Microphone.prototype.setGain), 'function' );
check_equals ( typeof(Microphone.prototype.setRate), 'function' );
check_equals ( typeof(Microphone.prototype.setSilenceLevel), 'function' );
check_equals ( typeof(Microphone.prototype.setUseEchoSuppression), 'function' );

// Documented to be an array.
xcheck ( Microphone.hasOwnProperty("names"));
xcheck_equals (typeof (Microphone.names), 'object');

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

/// Microphone properties

xcheck_equals ( typeof(microphoneObj.activityLevel), 'number' );
xcheck_equals ( typeof(microphoneObj.gain), 'number' );
xcheck_equals ( typeof(microphoneObj.index), 'number' );
xcheck_equals ( typeof(microphoneObj.muted), 'boolean' );
xcheck_equals ( typeof(microphoneObj.name), 'string' );
xcheck_equals ( typeof(microphoneObj.rate), 'number' );
xcheck_equals ( typeof(microphoneObj.silenceTimeout), 'number' );
// Documented to be boolean
xcheck_equals ( typeof(microphoneObj.useEchoSuppression), 'number' );

// Starting values // values before microphone is activated
xcheck_equals ( microphoneObj.activityLevel, -1 );
xcheck_equals ( microphoneObj.gain, 50 );
xcheck_equals ( microphoneObj.index, 0 );
xcheck_equals ( microphoneObj.muted, true );
xcheck_equals ( microphoneObj.rate, 8 );
xcheck_equals ( microphoneObj.silenceTimeout, 2000 );


// Setting and getting

microphoneObj.setGain(5);
xcheck_equals (microphoneObj.gain, 5);

microphoneObj.setSilenceLevel(16);
xcheck_equals (microphoneObj.silenceLevel, 16);


// 5, 8, 11, 22, or 44 documented...
microphoneObj.setRate(1);
xcheck_equals (microphoneObj.rate, 5);
microphoneObj.setRate(7);
xcheck_equals (microphoneObj.rate, 8);
microphoneObj.setRate(10);
xcheck_equals (microphoneObj.rate, 11);
microphoneObj.setRate(15);
xcheck_equals (microphoneObj.rate, 16);
microphoneObj.setRate(17);
xcheck_equals (microphoneObj.rate, 22);
microphoneObj.setRate(27);
xcheck_equals (microphoneObj.rate, 44);
microphoneObj.setRate(34);
xcheck_equals (microphoneObj.rate, 44);
microphoneObj.setRate(1000000);
xcheck_equals (microphoneObj.rate, 44);


/// It's still a number, though.
microphoneObj.setUseEchoSuppression(false);
xcheck_equals (microphoneObj.useEchoSuppression, false);

microphoneObj.setUseEchoSuppression(16);
xcheck_equals (microphoneObj.useEchoSuppression, true);

// listen to the microphone.
_root.attachAudio(microphoneObj);



#endif // OUTPUT_VERSION > 5
totals();
