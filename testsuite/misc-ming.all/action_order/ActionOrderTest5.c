/*
 *   Copyright (C) 2005, 2006, 2007, 2009, 2010 Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */ 
#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 8
#define OUTPUT_FILENAME "ActionOrderTest5.swf"

// A relatively simple looping test checking order of unload events.
//
// See InitActionTest3 and InitActionTest4. This is the same test but without
// static event handlers.

int main(int argc, char* argv[])
{

    SWFMovie mo;
    SWFMovieClip mc1, mc2, mc3, mc4, dejagnuclip;
    SWFDisplayItem it;
    SWFAction ac;
    SWFInitAction initac;

    const char *srcdir=".";
    if (argc > 1) srcdir = argv[1];
    else {
        fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
        return 1;
    }

    Ming_init();
    mo = newSWFMovieWithVersion(OUTPUT_VERSION);
    SWFMovie_setDimension(mo, 800, 600);
    SWFMovie_setRate (mo, 1.0);

    add_actions(mo,
            "if (!_global.hasOwnProperty('arr')) { _global.arr = []; };"
            "_global.ch = function(a, b) {"
            "   trace(a);"
            "   if (typeof(b)=='undefined' || (typeof(b)=='boolean' && b)) {"
            "       _global.arr.push(a);"
            "   };"
            "};"
            "this.onEnterFrame = function() { "
            "   _global.ch('onEnterFrame', false);"
            "};"
            );

    SWFMovie_nextFrame(mo);

    //  MovieClip 1 
    mc1 = newSWFMovieClip(); // 1 frames 

    // SWF_EXPORTASSETS 
    SWFMovie_addExport(mo, (SWFBlock)mc1, "Segments_Name");
    SWFMovie_writeExports(mo);

    //  MovieClip mc3 has two frames. In each frame a different MovieClip
    //  is placed with the name Segments.
    mc3 = newSWFMovieClip(); // 2 frames 

    //  MovieClip 2 
    mc2 = newSWFMovieClip(); // 1 frames 

    // Add mc2 to mc3 (first frame)
    it = SWFMovieClip_add(mc3, (SWFBlock)mc2);
    SWFDisplayItem_setDepth(it, 1);
    SWFDisplayItem_setName(it, "Segments");

    // Frame 2 of mc3
    SWFMovieClip_nextFrame(mc3);

    // Remove mc2
    SWFDisplayItem_remove(it);

    // Add mc1
    it = SWFMovieClip_add(mc3, (SWFBlock)mc1);
    SWFDisplayItem_setDepth(it, 1);
    SWFDisplayItem_setName(it, "Segments");

    SWFMovieClip_nextFrame(mc3);

    // End mc3


    // This is frame 1 of the main timeline

    // Put our sprite mc3 on stage.
    it = SWFMovie_add(mo, (SWFBlock)mc3);
    SWFDisplayItem_setDepth(it, 1);
    SWFDisplayItem_setName(it, "mc");

    //  mc4 is just for executing init actions.
    mc4 = newSWFMovieClip(); 
    SWFMovie_addExport(mo, (SWFBlock)mc4, "__Packages.Bug");
    SWFMovie_writeExports(mo);
    
    dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10,
                0, 0, 800, 600);
    SWFMovie_add(mo, (SWFBlock)dejagnuclip);

    ac = newSWFAction(
        "_global.loops = 0;"
        "_global.c = 0;"
        "if( !_global.Bug ) {"
        "   _global.Bug = function () {"
        "       this.onUnload = function() { "
        "           _global.ch('dynamic unload: ' + this.c);"
        "       }; "
        "       this.onLoad = function() { "
        "           _global.ch('dynamic load: ' + this.c);"
        "       }; "
        "       this.c = _global.c;"
        "       _global.ch('ctor: ' + _global.c);"
        "       _global.c++;"
        "   };"
        "};"
    );

    initac = newSWFInitAction_withId(ac, 4);
    SWFMovie_add(mo, (SWFBlock)initac);
    
    ac = newSWFAction("Object.registerClass('Segments_Name',Bug);");
    initac = newSWFInitAction_withId(ac, 1);
    SWFMovie_add(mo, (SWFBlock)initac);
    add_actions(mo, "_global.ch('Frame ' + "
                            "_level0._currentframe + ' actions: ' "
                            "+ _level0.mc.Segments.c);");

    // Frame 2 of the main timeline
    SWFMovie_nextFrame(mo);
    add_actions(mo, "_global.ch('Frame ' + "
                            "_level0._currentframe + ' actions: ' "
                            "+ _level0.mc.Segments.c);");
    
    add_actions(mo,
        "    if (_global.loops < 5) {"
        "        _global.loops++;"
        "        gotoAndPlay(2);"
        "   }"
        "   else {"
        "      mc.stop();"
        "      delete this.onEnterFrame;"
        "      gotoAndPlay(4);"
        "   };"
        );
    
    SWFMovie_nextFrame(mo);
    
    check_equals(mo, "_global.arr.length", "20");
    check_equals(mo, "_global.arr[0]", "'Frame 2 actions: undefined'");
    check_equals(mo, "_global.arr[1]", "'ctor: 0'");
    xcheck_equals(mo, "_global.arr[2]", "'Frame 3 actions: 0'");
    xcheck_equals(mo, "_global.arr[3]", "'dynamic load: 0'");
    check_equals(mo, "_global.arr[4]", "'Frame 2 actions: 0'");
    check_equals(mo, "_global.arr[5]", "'Frame 3 actions: 0'");
    check_equals(mo, "_global.arr[6]", "'Frame 2 actions: 0'");
    check_equals(mo, "_global.arr[7]", "'ctor: 1'");
    check_equals(mo, "_global.arr[8]", "'dynamic unload: 0'");
    xcheck_equals(mo, "_global.arr[9]", "'Frame 3 actions: 0'");
    check_equals(mo, "_global.arr[11]", "'Frame 2 actions: 0'");
    check_equals(mo, "_global.arr[12]", "'Frame 3 actions: 1'");
    check_equals(mo, "_global.arr[13]", "'Frame 2 actions: 1'");
    check_equals(mo, "_global.arr[14]", "'ctor: 2'");
    check_equals(mo, "_global.arr[15]", "'dynamic unload: 1'");
    xcheck_equals(mo, "_global.arr[16]", "'Frame 3 actions: 1'");
    xcheck_equals(mo, "_global.arr[17]", "'dynamic load: 2'");
    check_equals(mo, "_global.arr[18]", "'Frame 2 actions: 1'");
    check_equals(mo, "_global.arr[19]", "'Frame 3 actions: 2'");
    xcheck_equals(mo, "_global.arr.toString()",
		    "'Frame 2 actions: undefined,ctor: 0,Frame 3 actions: 0,dynamic load: 0,Frame 2 actions: 0,Frame 3 actions: 0,Frame 2 actions: 0,ctor: 1,dynamic unload: 0,Frame 3 actions: 0,dynamic load: 1,Frame 2 actions: 0,Frame 3 actions: 1,Frame 2 actions: 1,ctor: 2,dynamic unload: 1,Frame 3 actions: 1,dynamic load: 2,Frame 2 actions: 1,Frame 3 actions: 2'");

    SWFMovie_nextFrame(mo);
    add_actions(mo, "totals(21); stop();");
    
    SWFMovie_nextFrame(mo);

    // SWF_END 
    SWFMovie_save(mo, OUTPUT_FILENAME);

    return 0;
}

