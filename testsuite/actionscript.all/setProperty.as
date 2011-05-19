// 
//   Copyright (C) 2008, 2009, 2010 Free Software Foundation, Inc.
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

// Test case for setProperty/getProperty action opcodes.
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="setProperty.as";
#include "check.as"

#ifdef MING_SUPPORTS_ASM

//
// Test getProperty and setProperty
//
createSprite = ASnative(901, 0); // MovieClip.prototype.createEmptyMovieClip 
createSprite("mc1", 10);

mc1.func = function () {

    pass_check = _root.pass_check;
    xpass_check = _root.xpass_check;
    fail_check = _root.fail_check;
    xfail_check = _root.xfail_check;

    check_equals(this, _root.mc1);
    check_equals(_root.mc1._xscale, 100);
    check_equals(_root._xscale, 100);
    
    // setProperty("", _xscale, 30);
    // for SWF6 and above, it's _root._xscale = 30.
    // for SWF5, it's mc1._xscale = 30;
    asm{
        push "", 2, 30
        setproperty
    };  
#if OUTPUT_VERSION == 5
    check_equals(Math.round(_root.mc1._xscale), 30);
    check_equals(_root._xscale, 100);
#else
    check_equals(_root.mc1._xscale, 100);
    check_equals(Math.round(_root._xscale), 30); 
#endif
    _root._xscale = 60;
    var testvar = 0;
    // testvar = getProperty("", _xscale);
    // for SWF6 and above, it's testvar = _root._xscale;
    // for SWF5, it's testvar = mc1._xscale;
    asm{
        push "testvar"
        push "", 2
        getproperty
        setvariable
    };
#if OUTPUT_VERSION == 5
    check_equals(Math.round(testvar), 30);
#else
    // Gnash might be failed due to accuracy problem.
    // should we be more tolerant here?
    check_equals(Math.round(testvar), 60);
#endif

};
mc1.func();
// restore the _xscale of _root after test.
_root._xscale = 100;

//
// Test setProperty with string argument
//

// setProperty("", _x, "F");
_root._x=100; // set
asm{ push "", 0, "F" setproperty };  
check_equals(_root._x, 100);
_root._x=100; // reset
// setProperty("", _x, "0");
asm{ push "", 0, "0" setproperty };  
check_equals(_root._x, 0);
_root._x=100; // set
_root._x = Infinity; // infinity != nan
xcheck_equals(_root._x, -107374182.4);
_root._x = -Infinity; // infinity != nan
xcheck_equals(_root._x, -107374182.4);
_root._x=0; // reset

// setProperty("", _y, "F");
_root._y=100; // set
asm{ push "", 1, "F" setproperty };  
check_equals(_root._y, 100);
_root._y=100; // reset
// setProperty("", _y, "0");
asm{ push "", 1, "0" setproperty };  
check_equals(_root._y, 0);
_root._y=100; // set
_root._y = Infinity; // infinity != nan
xcheck_equals(_root._y, -107374182.4);
_root._y = -Infinity; // infinity != nan
xcheck_equals(_root._y, -107374182.4);
_root._y=0; // reset

// setProperty("", _xscale, "F");
asm{ push "", 2, "F" setproperty };  
check_equals(_root._xscale, 100);
_root._xscale=100; // reset
// setProperty("", _xscale, "0");
asm{ push "", 2, "0" setproperty };  
check_equals(_root._xscale, 0);
_root._xscale=100; // reset
_root._xscale = Infinity; // infinity != nan
check_equals(_root._xscale, Infinity);
_root._xscale = -Infinity; // infinity != nan
check_equals(_root._xscale, -Infinity);
_root._xscale=100; // reset

// setProperty("", _yscale, "F");
asm{ push "", 3, "F" setproperty };  
check_equals(_root._yscale, 100);
_root._yscale=100; // reset
// setProperty("", _yscale, "0");
asm{ push "", 3, "0" setproperty };  
check_equals(_root._yscale, 0);
_root._yscale=100; // reset
_root._yscale = Infinity; // infinity != nan
check_equals(_root._yscale, Infinity);
_root._yscale = -Infinity; // infinity != nan
check_equals(_root._yscale, -Infinity);
_root._yscale=100; // reset

// setProperty("", _alpha, "F");
asm{ push "", 6, "F" setproperty };  
check_equals(_root._alpha, 100);
_root._alpha=100; // reset
// setProperty("", _alpha, "0");
asm{ push "", 6, "0" setproperty };  
check_equals(_root._alpha, 0);
_root._alpha=100; // reset
_root._alpha = Infinity; // infinity != nan
check_equals(_root._alpha, -12800);
_root._alpha=100; // reset

check_equals(typeof(_root._visible), 'boolean');
check_equals(_root._visible, true);
// setProperty("", _visible, "F"); [ won't change the value ! ]
asm{ push "", 7, "F" setproperty };  
check_equals(typeof(_root._visible), 'boolean');
check_equals(_root._visible, true); 
_root._visible=true; // reset
// setProperty("", _visible, "1");
asm{ push "", 7, "1" setproperty };  
check_equals(typeof(_root._visible), 'boolean');
check_equals(_root._visible, true); 
// setProperty("", _visible, "0");
asm{ push "", 7, "0" setproperty };  
check_equals(typeof(_root._visible), 'boolean');
check_equals(_root._visible, false); 
_root._visible=true; // reset
_root._visible = Infinity; // infinity is skipped
check_equals(_root._visible, true);
_root._visible=true; // reset

// TODO: _width & _height

_root._rotation=90;
// setProperty("", _rotation, "F");
asm{ push "", 10, "F" setproperty };  
check_equals(_root._rotation, 90);
_root._rotation=100; // reset
// setProperty("", _rotation, "0");
asm{ push "", 10, "0" setproperty };  
check_equals(_root._rotation, 0);
_root._rotation = Infinity; // infinity != nan
check(isNaN(_root._rotation));
_root._rotation=0; // reset

//
//  test removeMovieClip(""), opcode 37
//

createSprite("mc2", 20);
mc2.remove = function () {
    // in SWF6 and obove, it's no effect(probably results _root.removeMovieClip()).
    // in SWF5, mc2.removeMovieClip().
    removeMovieClip("");
};
mc2.remove();
#if OUTPUT_VERSION == 5
    check_equals(typeof(mc2), 'undefined');
#else
    check_equals(typeof(mc2), 'movieclip');
#endif


//
//  test setTarget(""), opcode 139 
//
createSprite("mc3", 30);
_root.checkpoint = 0;
_root.mc3thisPtr = 0;

mc3.set_target = function() {
    // for SWF6 and above, it's setTarget("_root");
    // for SWF5, it's setTarget("mc3");
    setTarget("");
    _root.mc3thisPtr = this;
    asm{
        push '_root.checkpoint', '', 11         
        getproperty  //_target         
        setvariable             
    };
};
mc3.set_target();

check_equals(mc3thisPtr, mc3);

#if OUTPUT_VERSION == 5
    // gnash fails because it sets the *current* target
    // rather then the *original* target on function call
    check_equals(_root.checkpoint, "/mc3");
#else
    check_equals(_root.checkpoint, "/");
#endif

//
// Test scope chain in swf5.
//
// The scope chain in swf5 and below works differently, which  
// might explain why AS in swf5(and below) was so unstable. All 
// tests above can also be explained by the scope chain difference
// in swf5.
//
_root.checkpoint = "var_in_root";
createSprite("mc4", 40);
mc4.checkpoint = "var_in_mc4";
mc4.func = function () {

    pass_check = _root.pass_check;
    xpass_check = _root.xpass_check;
    fail_check = _root.fail_check;
    xfail_check = _root.xfail_check;

    check_equals(this, _root.mc4);
#if OUTPUT_VERSION == 5
    // for swf5, mc4 is the top of the scope chain.
    check_equals(checkpoint, "var_in_mc4");
    check_equals(_target, "/mc4");
    checkpoint = 0;
    check_equals(this.checkpoint, 0);
#else
    // for swf6 and above, current target is the top of the scope chain.
    check_equals(checkpoint, "var_in_root");
    check_equals(_target, "/");
    checkpoint = 0;
    check_equals(_root.checkpoint, 0);
#endif
};
mc4.func();

_root.checkpoint = "var_in_root";
obj.checkpoint = "var_in_obj";
obj = new Object();
obj.func = function () {
    check_equals(this, obj);
    // "this" object is not part of the scope chain for all swf versions.
    // But if "this" is a sprite(tests right above), the situation is different...
    check_equals(checkpoint, "var_in_root");
};
obj.func();

check_totals(46);

#endif //MING_SUPPORTS_ASM
