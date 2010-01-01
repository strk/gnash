// Microphone_as.hx:  ActionScript 3 "Microphone" class, for Gnash.
//
// This test is only valid for Flash v9 and higher.
//
// Generated on: 20090602 by "bnaugle". Remove this
// after any hand editing loosing changes.
//
//   Copyright (C) 2009, 2010 Free Software Foundation, Inc.
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

// This test case must be processed by CPP before compiling to include the
//  DejaGnu.hx header file for the testing framework support.

#if flash9
import flash.media.Microphone;
import flash.media.SoundTransform;
import flash.display.MovieClip;
#else
import flash.Microphone;
#end
import flash.Lib;
import Type;
import Std;

import DejaGnu;

// Class must be named with the PP prefix, as that's the name the
// file passed to haxe will have after the preprocessing step
class Microphone_as {
    static function typeof(thing:Dynamic) {
        return ("" + untyped __typeof__(thing));
    }
    
    static function main() {
#if flash9
        var x1:Microphone = Microphone.getMicrophone();
#else
		var x1:Microphone = Microphone.get();
#end

        // Make sure we actually get a valid class        
        if (Std.is(x1, Microphone)) {
            DejaGnu.pass("Microphone class exists");
        } else {
            DejaGnu.xfail("Microphone class doesn't exist");
        }
// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	if (typeof(x1.activityLevel) == "number") {
	    DejaGnu.pass("Microphone.activityLevel property exists");
	} else {
	    DejaGnu.fail("Microphone.activityLevel property doesn't exist");
	}
	
	
	//FIXME: this will need to be implemented if speex codec is supported
	//FIXME: it would be nice if this analyzed the string to see if it is
	//a supported codec
	//if (Type.typeof(x1.codec) == ValueType.TObject)  {
	//	DejaGnu.pass("Microphone::codec property exists");
	//} else {
	//	DejaGnu.fail("Microphone::codec property doesn't exist");
	//}
	//if (Type.typeof(x1.encodeQuality) == ValueType.TInt)  {
	//	DejaGnu.pass("Microphone::encodeQuality (speex) property exists");
	//} else {
	//	DejaGnu.fail("Microphone::encodeQuality (speex) property doesn't exist");
	//}
	//if (Type.typeof(x1.framesPerPacket) == ValueType.TInt) {
	//	DejaGnu.pass("Microphone::framesPerPacket (speex) property exists");
	//} else {
	//	DejaGnu.fail("Microphone::framesPerPacket (speex) property doesn't exist");
	//}
	
	
	if (typeof(x1.gain) == "number") {
	    DejaGnu.pass("Microphone::gain property exists");
	} else {
	    DejaGnu.fail("Microphone::gain property doesn't exist");
	}
	if (typeof(x1.index) == "number") {
	    DejaGnu.pass("Microphone::index property exists");
	} else {
	    DejaGnu.fail("Microphone::index property doesn't exist");
	}
	if (typeof(x1.muted) == "boolean") {
	    DejaGnu.pass("Microphone::muted property exists");
	} else {
	    DejaGnu.fail("Microphone::muted property doesn't exist");
	}
	//FIXME: it would be nice if this parsed and checked the name string
	if (typeof(x1.name) == "string") {
	    DejaGnu.pass("Microphone::name property exists");
	} else {
	    DejaGnu.fail("Microphone::name property doesn't exist");
	}
	if (typeof(x1.rate) == "number") {
	    DejaGnu.pass("Microphone::rate property exists");
	} else {
	    DejaGnu.fail("Microphone::rate property doesn't exist");
	}
	if (typeof(x1.silenceLevel) == "number") {
	    DejaGnu.pass("Microphone::silenceLevel property exists");
	} else {
	    DejaGnu.fail("Microphone::silenceLevel property doesn't exist");
	}
#if flash9
	if (Std.is(x1.soundTransform, SoundTransform)) {
	    DejaGnu.pass("Microphone::soundTransform property exists");
	} else {
	    DejaGnu.xfail("Microphone::soundTransform property doesn't exist");
	}
	if (Type.typeof(x1.silenceTimeout) == ValueType.TInt) {
	    DejaGnu.pass("Microphone::silenceTimeout property exists");
	} else {
	    DejaGnu.xfail("Microphone::silenceTimeout property doesn't exist");
	}
	if (typeof(x1.useEchoSuppression) == "boolean") {
	    DejaGnu.pass("Microphone::useEchoSuppression property exists");
	} else {
	    DejaGnu.fail("Microphone::useEchoSuppression property doesn't exist");
	}
#else
	if (typeof(untyped x1.silenceTimeout) == "number") {
	    DejaGnu.pass("Microphone::silenceTimeOut property exists");
	} else {
	    DejaGnu.fail("Microphone::silenceTimeOut property doesn't exist");
	}
	if (typeof(x1.useEchoSuppression) == "number") {
	    DejaGnu.pass("Microphone::useEchoSuppression property exists");
	} else {
	    DejaGnu.fail("Microphone::useEchoSuppression property doesn't exist");
	}
#end

// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
#if flash9
	if (Type.typeof(x1.setLoopBack) == ValueType.TFunction) {
	    DejaGnu.pass("Microphone::setLoopBack() method exists");
	} else {
	    DejaGnu.fail("Microphone::setLoopBack() method doesn't exist");
	}
#else
	if (Type.typeof(x1.setRate) == ValueType.TFunction) {
	    DejaGnu.pass("Microphone::setRate() method exists");
	} else {
	    DejaGnu.fail("Microphone::setRate() method doesn't exist");
	}
	if (Type.typeof(x1.setGain) == ValueType.TFunction) {
	    DejaGnu.pass("Microphone::setGain() method exists");
	} else {
	    DejaGnu.fail("Microphone::setGain() method doesn't exist");
	}
    
    //try some values now
    
    //set the rate to proper values and make sure it holds them correctly
    x1.setRate(5);
    if (x1.rate == 5) {
        DejaGnu.pass("setRate(5) worked");
    } else {
        DejaGnu.fail("setRate(5) failed, got " + x1.rate);
    }
    
    x1.setRate(8);
    if (x1.rate == 8) {
        DejaGnu.pass("setRate(8) worked");
    } else {
        DejaGnu.fail("setRate(8) failed, got " + x1.rate);
    }
    
    x1.setRate(11);
    if (x1.rate == 11) {
        DejaGnu.pass("setRate(11) worked");
    } else {
        DejaGnu.fail("setRate(11) failed, got " + x1.rate);
    }
    
    x1.setRate(22);
    if (x1.rate == 22) {
        DejaGnu.pass("setRate(22) worked");
    } else {
        DejaGnu.fail("setRate(22) failed, got " + x1.rate);
    }
    
    x1.setRate(44);
    if (x1.rate == 44) {
        DejaGnu.pass("setRate(44) worked");
    } else {
        DejaGnu.fail("setRate(44) failed, got " + x1.rate);
    }
    
    
    //now try bad values
    x1.setRate(10000000);
    if (x1.rate == 44) {
        DejaGnu.pass("setRate(10000000) returned the proper default");
    } else {
        DejaGnu.fail("setRate(10000000) didn't return the proper default (44)");
    }
    x1.setRate(-1);
    if (x1.rate == 5) {
        DejaGnu.pass("setRate(-1) returned the proper default");
    } else {
        DejaGnu.fail("setRate(-1) didn't return the proper default (5)");
    }
    
    //test setgain
    x1.setGain(0);
    if (x1.gain == 0) {
        DejaGnu.pass("setGain(0) worked");
    } else {
        DejaGnu.fail("setGain(0) failed, got " + x1.gain);
    }
    
    x1.setGain(100);
    if (x1.gain == 100) {
        DejaGnu.pass("setGain(100) worked");
    } else {
        DejaGnu.fail("setGain(100) failed, got " + x1.gain);
    }
    
    x1.setGain(77);
    if (x1.gain == 77) {
        DejaGnu.pass("setGain(77) worked");
    } else {
        DejaGnu.fail("setGain(77) failed, got " + x1.gain);
    }
    
    //try bad values
    x1.setGain(300);
    if (x1.gain == 100) {
        DejaGnu.pass("setGain(300) returned the default value");
    } else {
        DejaGnu.fail("setGain(300) didn't return the default value");
    }
    x1.setGain(-20);
    if (x1.gain == 0) {
        DejaGnu.pass("setGain(-20) returned the default value");
    } else {
        DejaGnu.fail("setGain(-20) didn't return the default value");
    }
#end
	if (Type.typeof(x1.setSilenceLevel) == ValueType.TFunction) {
	    DejaGnu.pass("Microphone::setSilenceLevel() method exists");
	} else {
	    DejaGnu.xfail("Microphone::setSilenceLevel() method doesn't exist");
	}
	if (Type.typeof(x1.setUseEchoSuppression) == ValueType.TFunction) {
	    DejaGnu.pass("Microphone::setUseEchoSuppression() method exists");
	} else {
	    DejaGnu.xfail("Microphone::setUseEchoSuppression() method doesn't exist");
	}

#if !flash9
    //try some values now
    x1.setSilenceLevel(0, 2000);
    if ((x1.silenceLevel == 0) && (untyped x1.silenceTimeout == 2000)) {
        DejaGnu.pass("setSilenceLevel(0,2000) worked");
    } else {
        DejaGnu.fail("setSilenceLevel(0,2000) failed");
    }
    x1.setSilenceLevel(100, 5500);
    if ((x1.silenceLevel == 100) && (untyped x1.silenceTimeout == 5500)) {
        DejaGnu.pass("setSilenceLevel(100,5500) worked");
    } else {
        DejaGnu.fail("setSilenceLevel(100,5500) failed");
    }
    
    //try bad values now
    x1.setSilenceLevel(200, -1000);
    if ((x1.silenceLevel == 100) && (untyped x1.silenceTimeout == 0)) {
        DejaGnu.pass("setSilenceLevel(200,-1000) worked as intended");
    } else {
        DejaGnu.fail("setSilenceLevel(200,-1000) didn't work as intended");
    }
    
    x1.setSilenceLevel(-10000, 8000);
    if ((x1.silenceLevel == 0) && (untyped x1.silenceTimeout == 8000)) {
        DejaGnu.pass("setSilenceLevel(-10000, 8000) worked as intended");
    } else {
        DejaGnu.fail("setSilenceLevel(-10000, 8000) didn't work as intended");
    }
    
    x1.setUseEchoSuppression(true);
    if (x1.useEchoSuppression == true) {
        DejaGnu.pass("setUseEchoSuppression(true) worked");
    } else {
        DejaGnu.fail("setUseEchoSuppression(true) failed");
    }
    
    x1.setUseEchoSuppression(false);
    if (x1.useEchoSuppression == false) {
        DejaGnu.pass("setUseEchoSuppression(false) worked");
    } else {
        DejaGnu.fail("setUseEchoSuppression(false) failed");
    }
#end

        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
    }
}
