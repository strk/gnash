// ColorTransform_as.hx:  ActionScript 3 "ColorTransform" class, for Gnash.
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

#if (flash9 || flash8)
import flash.geom.ColorTransform;
#end
import flash.Lib;
import Type;
import Std;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class ColorTransform_as {
    static function main() {
#if !(flash6 || flash7)
    var x1:ColorTransform = new ColorTransform();

    //Make sure we actually get a valid class       
    if (Std.is(x1, ColorTransform)) {
        DejaGnu.pass("ColorTransform class exists");
	} else {
            DejaGnu.fail("ColorTransform class doesn't exist");
	}

    // Tests to see if all the properties exist. All these do is test for
    // existance of a property, and don't test the functionality at all. This
    // is primarily useful only to test completeness of the API implementation.
	#if flash9
	if (Std.is(x1.alphaMultiplier, Float)) {
		DejaGnu.pass("ColorTransform::alphaMultiplier property exists");
    } else {
        DejaGnu.fail("ColorTransform::alphaMultiplier property doesn't exist");
    }
	if (Std.is(x1.alphaOffset, Float)) {
		DejaGnu.pass("ColorTransform::alphaOffset property exists");
    } else {
		DejaGnu.fail("ColorTransform::alphaOffset property doesn't exist");
    }
	if (Std.is(x1.blueMultiplier, Float)) {
		DejaGnu.pass("ColorTransform::blueMultiplier property exists");
    } else {
        DejaGnu.fail("ColorTransform::blueMultiplier property doesn't exist");
    }
	if (Std.is(x1.blueOffset, Float)) {
		DejaGnu.pass("ColorTransform::blueOffset property exists");
    } else {
        DejaGnu.fail("ColorTransform::blueOffset property doesn't exist");
    }
	if (Std.is(x1.color, Int)) {
		DejaGnu.pass("ColorTransform::color property exists");
    } else {
        DejaGnu.fail("ColorTransform::color property doesn't exist");
    }
	if (Std.is(x1.greenMultiplier, Float)) {
		DejaGnu.pass("ColorTransform::greenMultiplier property exists");
    } else {
        DejaGnu.fail("ColorTransform::greenMultiplier property doesn't exist");
    }
	if (Std.is(x1.greenOffset, Float)) {
		DejaGnu.pass("ColorTransform::greenOffset property exists");
    } else {
        DejaGnu.fail("ColorTransform::greenOffset property doesn't exist");
    }
	if (Std.is(x1.redMultiplier, Float)) {
		DejaGnu.pass("ColorTransform::redMultiplier property exists");
    } else {
        DejaGnu.fail("ColorTransform::redMultiplier property doesn't exist");
    }	
	if (Std.is(x1.redOffset, Float)) {
		DejaGnu.pass("ColorTransform::redOffset property exists");
    } else {
        DejaGnu.fail("ColorTransform::redOffset property doesn't exist");
    }
	#end
	
	#if flash8
    if (untyped ColorTransform.prototype.hasOwnProperty("alphaMultiplier")) {
         DejaGnu.pass("ColorTransform.prototype.hasOwnProperty('alphaMultiplier')");
    } else {
        DejaGnu.fail("!ColorTransform.prototype.hasOwnProperty('alphaMultiplier')");
    }
    if (untyped ColorTransform.prototype.hasOwnProperty("alphaOffset")) {
         DejaGnu.pass("ColorTransform.prototype.hasOwnProperty('alphaOffset')");
    } else {
        DejaGnu.fail("!ColorTransform.prototype.hasOwnProperty('alphaOffset')");
    }
    if (untyped ColorTransform.prototype.hasOwnProperty("blueMultiplier")) {
           DejaGnu.pass("ColorTransform.prototype.hasOwnProperty('blueMultiplier')");
    } else {
        DejaGnu.fail("!ColorTransform.prototype.hasOwnProperty('blueMultiplier')");
    }
    if (untyped ColorTransform.prototype.hasOwnProperty("blueOffset")) {
        DejaGnu.pass("ColorTransform.blueOffset property exists");
    } else {
        DejaGnu.fail("ColorTransform.blueOffset property doesn't exist");
    }
	if (untyped ColorTransform.prototype.hasOwnProperty("greenMultiplier")) {
        DejaGnu.pass("ColorTransform.greenMultiplier property exists");
    } else {
        DejaGnu.fail("ColorTransform.greenMultiplier property doesn't exist");
    }
	if (untyped ColorTransform.prototype.hasOwnProperty("greenOffset")) {
        DejaGnu.pass("ColorTransform.greenOffset property exists");
    } else {
        DejaGnu.fail("ColorTransform.greenOffset property doesn't exist");
    }
    if (untyped ColorTransform.prototype.hasOwnProperty("redMultiplier")) {
        DejaGnu.pass("ColorTransform.redMultiplier property exists");
    } else {
        DejaGnu.fail("ColorTransform.redMultiplier property doesn't exist");
    }
	if (untyped ColorTransform.prototype.hasOwnProperty("redOffset")) {
		DejaGnu.pass("ColorTransform.redOffset property exists");
    } else {
        DejaGnu.fail("ColorTransform.redOffset property doesn't exist");
    }
	if (untyped ColorTransform.prototype.hasOwnProperty("rgb")) {
		DejaGnu.pass("ColorTransform.rgb property exists");
	} else {
		DejaGnu.fail("ColorTransform.rgb property doesn't exist");
	}
	#end

    // Tests to see if all the methods exist. All these do is test for
    // existance of a method, and don't test the functionality at all. This
    // is primarily useful only to test completeness of the API implementation.
    if (Type.typeof(x1.concat)==ValueType.TFunction) {
          DejaGnu.pass("ColorTransform::concat() method exists");
    } else {
        DejaGnu.fail("ColorTransform::concat() method doesn't exist");
    }
    if (Type.typeof(x1.toString)==ValueType.TFunction) {
        DejaGnu.pass("ColorTransform::toString() method exists");
    } else {
        DejaGnu.fail("ColorTransform::toString() method doesn't exist");
    }

	#if flash8
    if (Type.typeof(untyped ColorTransform.prototype)==ValueType.TObject) {
        DejaGnu.pass("typeof(ColorTransform.prototype)=='object'");
    } else {
        DejaGnu.fail("typeof(ColorTransform.prototype)!='object'");
    }
	#end
   
    // Test constructor
    var c : ColorTransform = new ColorTransform();
   
	#if flash8
    if (Type.typeof(c)==ValueType.TObject) {
        DejaGnu.pass("typeof(c)=='object'");
    } else {
        DejaGnu.fail("typeof(c)!='object' " + Type.typeof(c));
    }
	#end
	
	if (Std.is(c, ColorTransform)) {
        DejaGnu.pass("c instanceof ColorTransform");
    } else {
        DejaGnu.fail("! c instanceof ColorTransform");
    }
   
    if (c.redMultiplier==1) {
        DejaGnu.pass("c.redMultipler==1");
    } else {
        DejaGnu.fail("c.redMultiplier!=1");
    }
    if (c.blueMultiplier==1) {
        DejaGnu.pass("c.blueMultiplier==1");
    } else {
        DejaGnu.fail("c.blueMultiplier!=1");
    }
    if (c.greenMultiplier==1) {
        DejaGnu.pass("c.greenMultiplier==1");
    } else {
        DejaGnu.fail("c.greenMultiplier!=1");
    }
    if (c.alphaMultiplier==1) {
        DejaGnu.pass("c.alphaMultiplier==1");
    } else {
        DejaGnu.fail("c.alphaMultiplier!=1");
    }
    if (c.redOffset==0) {
        DejaGnu.pass("c.redOffset==0");
    } else {
        DejaGnu.fail("c.redOffset!=0");
    }
    if (c.blueOffset==0) {
        DejaGnu.pass("c.blueOffset==0");
    } else {
        DejaGnu.fail("c.blueOffset!=0");
    }
    if (c.greenOffset==0) {
        DejaGnu.pass("c.greenOffset==0");
    } else {
        DejaGnu.fail("c.greenOffset!=0");
    }
    if (c.alphaOffset==0) {
        DejaGnu.pass("c.alphaOffset==0");
    } else {
        DejaGnu.fail("c.alphaOffset!=0");
    }   
   
    if (c.toString() == "(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=1, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)") {
        DejaGnu.pass("c.toString()=='(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=1, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)'");
    } else {
        DejaGnu.fail("c.toString()!='(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=1, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)'");
    }
   
    Reflect.setField(c, "redMultiplier", 5.4);
   
    if (c.redMultiplier==5.4) {
        DejaGnu.pass("c.redMultiplier==5.4");
    } else {
        DejaGnu.fail("c.redMultiplier!=5.4");
    }
   
    Reflect.setField(c, "alphaMultiplier", -0.3);
   
    if (c.alphaMultiplier==-0.3) {
        DejaGnu.pass("c.alphaMultiplier==-0.3");
    } else {
        DejaGnu.fail("c.alphaMultiplier!=-0.3");
    }
   
    Reflect.setField(c, "redOffset", 123);
   
    if (c.redOffset==123) {
        DejaGnu.pass("c.redOffset==123");
    } else {
        DejaGnu.fail("c.redOffset!=123");
    }
   
    Reflect.setField(c, "greenOffset", 287);
   
    if (c.greenOffset==287) {
        DejaGnu.pass("c.greenOffset==287");
    } else {
        DejaGnu.fail("c.greenOffset!=287");
    }

    var o : Dynamic = {};
    var t : Dynamic = {};
	var s : String = "string";
    var p : Bool = true;	
	
    var e : Dynamic = {};
       
    o.valueOf = function() {
        return 456;
    }
   
	#if flash8
	// UNRESOLVED
	// This test is not possible with haXe...you cannot pass in other
	// data types as variables.  You can pass in empty objects, but that
	// is it.  We can produce functionality by simply passing in empty
	// objects whenever the field is NaN.  
	//c = new ColorTransform(new Object, 3, "string", true, ASnative(100,9), new Error("custom error"), undefined, o);
	
    var c : ColorTransform = new ColorTransform(t, 3, t, 1, t, t, t, o);
   
    if (c.toString()=="(redMultiplier=NaN, greenMultiplier=3, blueMultiplier=NaN, alphaMultiplier=1, redOffset=NaN, greenOffset=NaN, blueOffset=NaN, alphaOffset=456)") {
        DejaGnu.unresolved("c.toString()=='(redMultiplier=NaN, greenMultiplier=3, blueMultiplier=NaN, alphaMultiplier=1, redOffset=NaN, greenOffset=NaN, blueOffset=NaN, alphaOffset=456)'");
    } else {
        DejaGnu.fail("c.toString()!='(redMultiplier=NaN, greenMultiplier=3, blueMultiplier=NaN, alphaMultiplier=1, redOffset=NaN, greenOffset=NaN, blueOffset=NaN, alphaOffset=456)' " + c.toString());
    }
   
    c = new ColorTransform(0, 2, 3);
   
    if (c.toString() == "(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=1, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)") {
        DejaGnu.pass("c.toString()=='(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=1, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)'");
    } else {
        DejaGnu.fail("c.toString()!='(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=1, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)' " + c.toString());
    }
   
    c = new ColorTransform(0, 2, 3, 4);
   
    if (c.toString()=="(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=1, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)") {
        DejaGnu.pass("c.toString()=='(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=1, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)'");
    } else {
        DejaGnu.fail("c.toString()!='(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=1, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)' " + c.toString());
    }
   
    c = new ColorTransform(0, 2, 3, 4, 5, 6, 7);
   
    if (c.toString()=="(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=1, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)") {
        DejaGnu.pass("c.toString()=='(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=1, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)'");
    } else {
        DejaGnu.fail("c.toString()!='(redMultiplier=1, greenMultiplier=1, blueMultiplier=1, alphaMultiplier=1, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0)' " + c.toString());
    }

    c = new ColorTransform(0, 2, 3, 4, 5, 6, 7, 8);
   
    if (c.toString()=="(redMultiplier=0, greenMultiplier=2, blueMultiplier=3, alphaMultiplier=4, redOffset=5, greenOffset=6, blueOffset=7, alphaOffset=8)") {
        DejaGnu.pass("c.toString()=='(redMultiplier=0, greenMultiplier=2, blueMultiplier=3, alphaMultiplier=4, redOffset=5, greenOffset=6, blueOffset=7, alphaOffset=8)'");
    } else {
        DejaGnu.fail("c.toString()!='(redMultiplier=0, greenMultiplier=2, blueMultiplier=3, alphaMultiplier=4, redOffset=5, greenOffset=6, blueOffset=7, alphaOffset=8)' " + c.toString());
    }
	#end
   
    c = new ColorTransform(0, 2, 3, 4, 5, 6, -8.334, 9.7);

    if (c.toString()=="(redMultiplier=0, greenMultiplier=2, blueMultiplier=3, alphaMultiplier=4, redOffset=5, greenOffset=6, blueOffset=-8.334, alphaOffset=9.7)") {
        DejaGnu.pass("c.toString()=='(redMultiplier=0, greenMultiplier=2, blueMultiplier=3, alphaMultiplier=4, redOffset=5, greenOffset=6, blueOffset=-8.334, alphaOffset=9.7)'");
    } else {
        DejaGnu.fail("c.toSTring()!='(redMultiplier=0, greenMultiplier=2, blueMultiplier=3, alphaMultiplier=4, redOffset=5, greenOffset=6, blueOffset=-8.334, alphaOffset=9.7)' " + c.toString());
    }
 
	// UNRESOLVED
	// haXe cannot compile a constructor call with too many arguments
	// in this case, ColorTransform can only call 8.  Apparently, AS
	// can compile such a case, so instead of calling 9 arguments as in
	// the line below, we will only call the first 8 ones, which AS
	// interprets
	//c = new ColorTransform(0, 2, 3, 4, 5, 6, 7, 8, 9)
	
	c = new ColorTransform(0, 2, 3, 4, 5, 6, 7, 8);
	
    if (c.toString()=="(redMultiplier=0, greenMultiplier=2, blueMultiplier=3, alphaMultiplier=4, redOffset=5, greenOffset=6, blueOffset=7, alphaOffset=8)") {
        DejaGnu.unresolved("c.toString()=='(redMultiplier=0, greenMultiplier=2, blueMultiplier=3, alphaMultiplier=4, redOffset=5, greenOffset=6, blueOffset=7, alphaOffset=8)'");
    } else {
        DejaGnu.fail("c.toString()!='(redMultiplier=0, greenMultiplier=2, blueMultiplier=3, alphaMultiplier=4, redOffset=5, greenOffset=6, blueOffset=7, alphaOffset=8)' " + c.toString());
    }
   
	// rgb property not in Flash9
	#if flash8
    // Test ColorTransform.rgb	
	untyped c.rgb = 0xFFFF00;

	if (c.toString()=="(redMultiplier=0, greenMultiplier=0, blueMultiplier=0, alphaMultiplier=4, redOffset=255, greenOffset=255, blueOffset=0, alphaOffset=8)") {
		DejaGnu.pass("c.toString()=='(redMultiplier=0, greenMultiplier=0, blueMultiplier=0, alphaMultiplier=4, redOffset=255, greenOffset=255, blueOffset=0, alphaOffset=8)'");
	} else {
		DejaGnu.fail("c.toString()!='(redMultiplier=0, greenMultiplier=0, blueMultiplier=0, alphaMultiplier=4, redOffset=255, greenOffset=255, blueOffset=0, alphaOffset=8)' " + c.toString());
	}
	if (untyped c.rgb.toString()=="16776960") {
		DejaGnu.pass("c.rgb.toString()=='16776960'");
	} else {
		DejaGnu.fail("c.rgb.toString()!='16776960' " + untyped c.rgb.toString());
	}
	
	c = new ColorTransform(1, 1, -0.5, 4, 2, 2, 2, 8);
	untyped c.rgb = 0xFF34FF;

	if (c.toString()== "(redMultiplier=0, greenMultiplier=0, blueMultiplier=0, alphaMultiplier=4, redOffset=255, greenOffset=52, blueOffset=255, alphaOffset=8)") {
		DejaGnu.pass("c.toString()=='(redMultiplier=0, greenMultiplier=0, blueMultiplier=0, alphaMultiplier=4, redOffset=255, greenOffset=52, blueOffset=255, alphaOffset=8)'");
	} else {
		DejaGnu.fail("c.toString()!='(redMultiplier=0, greenMultiplier=0, blueMultiplier=0, alphaMultiplier=4, redOffset=255, greenOffset=52, blueOffset=255, alphaOffset=8)'");
	}
	if (untyped c.rgb.toString()=="16725247") {
		DejaGnu.pass("c.rgb.toString()=='16725247'");
	} else {
		DejaGnu.fail("c.rgb.toString()!='16725247'");
	}

	untyped c.rgb = 0x000000;
	
	if (c.toString()=="(redMultiplier=0, greenMultiplier=0, blueMultiplier=0, alphaMultiplier=4, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=8)") {
		DejaGnu.pass("c.toString()=='(redMultiplier=0, greenMultiplier=0, blueMultiplier=0, alphaMultiplier=4, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=8)'");
	} else {
		DejaGnu.fail("c.toString()!='(redMultiplier=0, greenMultiplier=0, blueMultiplier=0, alphaMultiplier=4, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=8)'");
	}
	if (untyped c.rgb.toString()=="0") {
		DejaGnu.pass("c.rgb.toString()=='0'");
	} else {
		DejaGnu.fail("c.rgb.toString()!='0'");
	}
	
	untyped c.rgb = -4534;
	
	if (c.toString()=="(redMultiplier=0, greenMultiplier=0, blueMultiplier=0, alphaMultiplier=4, redOffset=255, greenOffset=238, blueOffset=74, alphaOffset=8)") {
		DejaGnu.pass("c.toString()=='(redMultiplier=0, greenMultiplier=0, blueMultiplier=0, alphaMultiplier=4, redOffset=255, greenOffset=238, blueOffset=74, alphaOffset=8)'");
	} else {
		DejaGnu.fail("c.toString()!='(redMultiplier=0, greenMultiplier=0, blueMultiplier=0, alphaMultiplier=4, redOffset=255, greenOffset=238, blueOffset=74, alphaOffset=8)'");
	}
	if (untyped c.rgb.toString()=="16772682") {
		DejaGnu.pass("c.rgb.toString()=='16772682'");
	} else {
		DejaGnu.fail("c.rgb.toString()!='16772682'");
	}

	untyped c.rgb = 0xFFFFFFFF;

	if (c.toString()=="(redMultiplier=0, greenMultiplier=0, blueMultiplier=0, alphaMultiplier=4, redOffset=255, greenOffset=255, blueOffset=255, alphaOffset=8)") {
		DejaGnu.pass("c.toString()=='(redMultiplier=0, greenMultiplier=0, blueMultiplier=0, alphaMultiplier=4, redOffset=255, greenOffset=255, blueOffset=255, alphaOffset=8)'");
	} else {
		DejaGnu.fail("c.toString()=='(redMultiplier=0, greenMultiplier=0, blueMultiplier=0, alphaMultiplier=4, redOffset=255, greenOffset=255, blueOffset=255, alphaOffset=8)'");
	}
	if (untyped c.rgb.toString()=="16777215") {
		DejaGnu.pass("c.rgb.toString()=='16777215'");
	} else {
		DejaGnu.fail("c.rgb.toString()!='16777215'");
	}
	
	// It's just bitshifting...
	c = new ColorTransform(1, 1, 1, 1, 1000, 1000, 1000, 0);
	
	if (untyped c.rgb.toString()=="65793000") {
		DejaGnu.pass("c.rgb.toString()=='65793000'");
	} else {
		DejaGnu.fail("c.rgb.toString()!='65793000'");
	}
	
	c = new ColorTransform(1, 1, 1, 1, 1000000, 1000, 1000, 0);

	if (untyped c.rgb.toString()=="1111747560") {
		DejaGnu.pass("c.rgb.toString()=='1111747560'");
	} else {
		DejaGnu.fail("c.rgb.toString()!='1111747560'"); 
	} 
	
	c = new ColorTransform(1, 1, 1, 1, 100000000000000000, 10000, 1000, 0);

	if (untyped c.rgb.toString()=="2561000") {
		DejaGnu.pass("c.rgb.toString()=='2561000'");
	} else {
		DejaGnu.fail("c.rgb.toString()!='2561000'");
	}

	o = { redMultiplier:2, greenMultiplier:3, blueMultiplier:3, alphaMultiplier:0, redOffset:3, greenOffset:4, blueOffset:3, alphaOffset:3 };
	untyped o.toString =  ColorTransform.toString();

	if (Type.typeof(o)==ValueType.TObject) {
		DejaGnu.pass("o.toString()=='[object Object]'");
	} else {
		DejaGnu.fail("o.toString()!='[object Object]'");
	}
	#end
    
	// Call this after finishing all tests. It prints out the totals.
    DejaGnu.done();
#else
    DejaGnu.note("This class (ColorTransform) is only available in flash8 and flash9");
#end
    }

}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
