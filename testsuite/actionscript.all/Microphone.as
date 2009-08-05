// 
//   Copyright (C) 2005, 2006, 2007, 2009 Free Software Foundation, Inc.
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


rcsid="$Id: Microphone.as,v 1.18 2008/06/04 13:43:24 bwy Exp $";
#include "check.as"

// There was no Microphone class in SWF5 or lower
#if OUTPUT_VERSION > 5

// test the Microphone class
check_equals(typeof(Microphone), 'function');
check_equals ( typeof(Microphone.prototype.setGain), 'function' );
check_equals ( typeof(Microphone.prototype.setRate), 'function' );
check_equals ( typeof(Microphone.prototype.setSilenceLevel), 'function' );
check_equals ( typeof(Microphone.prototype.setUseEchoSuppression), 'function' );

check(Microphone.prototype.hasOwnProperty("setGain"));
check(Microphone.prototype.hasOwnProperty("setRate"));
check(Microphone.prototype.hasOwnProperty("setSilenceLevel"));
check(Microphone.prototype.hasOwnProperty("setUseEchoSuppression"));

// These aren't present yet.
check(Microphone.prototype.hasOwnProperty("get"));
check(Microphone.prototype.hasOwnProperty("activityLevel"));
check(Microphone.prototype.hasOwnProperty("gain"));
check(Microphone.prototype.hasOwnProperty("index"));
check(Microphone.prototype.hasOwnProperty("muted"));
check(Microphone.prototype.hasOwnProperty("name"));
check(Microphone.prototype.hasOwnProperty("names"));
xcheck(Microphone.prototype.hasOwnProperty("onActivity"));
xcheck(Microphone.prototype.hasOwnProperty("onStatus"));
check(Microphone.prototype.hasOwnProperty("rate"));
check(Microphone.prototype.hasOwnProperty("silenceLevel"));
check(Microphone.prototype.hasOwnProperty("silenceTimeOut"));
check(Microphone.prototype.hasOwnProperty("useEchoSuppression"));

f = new Microphone;

// Called with new, Microphone returns an object with static
// properties.
check_equals(typeof(f), 'object');
check_equals(typeof(f.setGain), 'function')
check_equals(typeof(f.gain), 'number')
check_equals(typeof(f.rate), 'number')

// Still not present
check(Microphone.prototype.hasOwnProperty("get"));
check(Microphone.prototype.hasOwnProperty("activityLevel"));
check(Microphone.prototype.hasOwnProperty("gain"));
check(Microphone.prototype.hasOwnProperty("index"));
check(Microphone.prototype.hasOwnProperty("muted"));
check(Microphone.prototype.hasOwnProperty("name"));
check(Microphone.prototype.hasOwnProperty("names"));
check(!Microphone.prototype.hasOwnProperty("onActivity"));
check(!Microphone.prototype.hasOwnProperty("onStatus"));
check(Microphone.prototype.hasOwnProperty("rate"));
check(Microphone.prototype.hasOwnProperty("silenceLevel"));
check(Microphone.prototype.hasOwnProperty("silenceTimeOut"));
check(Microphone.prototype.hasOwnProperty("useEchoSuppression"));

// Documented to be an array.
check ( Microphone.hasOwnProperty("names"));
xcheck_equals (typeof (Microphone.names), 'object');

// test the Microphone constuctor
var microphoneObj = Microphone.get();
check_equals (typeof(microphoneObj), 'object');

// Microphone.get() adds these properties.
// Other properties are probably dependent on whether a microphone
// is present or not.
check(Microphone.prototype.hasOwnProperty("get"));
check(Microphone.prototype.hasOwnProperty("activityLevel"));
check(Microphone.prototype.hasOwnProperty("gain"));
check(Microphone.prototype.hasOwnProperty("index"));
check(Microphone.prototype.hasOwnProperty("muted"));
check(Microphone.prototype.hasOwnProperty("name"));
check(Microphone.prototype.hasOwnProperty("rate"));
check(Microphone.prototype.hasOwnProperty("silenceLevel"));
check(Microphone.prototype.hasOwnProperty("useEchoSuppression"));

// test that get() method is NOT exported to instances
check_equals (typeof(microphoneObj.get), 'function');

// test the Microphone::setGain method
check_equals ( typeof(microphoneObj.setGain), 'function' );

// test the Microphone::setRate method
check_equals ( typeof(microphoneObj.setRate), 'function' );

// test the Microphone::setSilenceLevel method
check_equals ( typeof(microphoneObj.setSilenceLevel), 'function' );

// test the Microphone::setUseEchoSuppression method
check_equals ( typeof(microphoneObj.setUseEchoSuppression), 'function' );

/// Microphone properties

check_equals ( typeof(microphoneObj.activityLevel), 'number' );
check_equals ( typeof(microphoneObj.gain), 'number' );
check_equals ( typeof(microphoneObj.index), 'number' );
check_equals ( typeof(microphoneObj.muted), 'boolean' );
check_equals ( typeof(microphoneObj.name), 'string' );
check_equals ( typeof(microphoneObj.rate), 'number' );
check_equals ( typeof(microphoneObj.silenceTimeOut), 'number' );
// Documented to be boolean
check_equals ( typeof(microphoneObj.useEchoSuppression), 'boolean' );

// Starting values // values before microphone is activated
check_equals ( microphoneObj.activityLevel, -1 );
//check_equals ( microphoneObj.gain, 50 );
check_equals ( microphoneObj.index, 0 );
xcheck_equals ( microphoneObj.muted, true);
check_equals ( microphoneObj.rate, 8 );
check_equals ( microphoneObj.silenceTimeOut, 2000 );


// Setting and getting

microphoneObj.setGain(5);
check_equals (microphoneObj.gain, 5);

microphoneObj.setSilenceLevel(16);
check_equals (microphoneObj.silenceLevel, 16);


// 5, 8, 11, 22, or 44 documented...
microphoneObj.setRate(1);
check_equals (microphoneObj.rate, 11);
microphoneObj.setRate(7);
check_equals (microphoneObj.rate, 11);
microphoneObj.setRate(10);
check_equals (microphoneObj.rate, 11);
microphoneObj.setRate(15);
check_equals (microphoneObj.rate, 11);
microphoneObj.setRate(17);
check_equals (microphoneObj.rate, 11);
microphoneObj.setRate(27);
check_equals (microphoneObj.rate, 11);
microphoneObj.setRate(34);
check_equals (microphoneObj.rate, 11);
microphoneObj.setRate(1000000);
check_equals (microphoneObj.rate, 11);


/// It's still a number, though.
microphoneObj.setUseEchoSuppression(false);
check_equals (microphoneObj.useEchoSuppression, false);

microphoneObj.setUseEchoSuppression(16);
check_equals (microphoneObj.useEchoSuppression, true);

// listen to the microphone.
_root.attachAudio(microphoneObj);



#endif // OUTPUT_VERSION > 5
totals();
