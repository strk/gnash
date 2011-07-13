/* 
 *   Copyright (C) 2007, 2009, 2010, 2011 Free Software Foundation, Inc.
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
#define OUTPUT_FILENAME "RegisterClassTest4.swf"

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
    SWFMovie_setRate (mo, 12.0);

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

    // Add mc2
	it = SWFMovieClip_add(mc3, (SWFBlock)mc2);
    SWFDisplayItem_setDepth(it, 1);
	SWFDisplayItem_setName(it, "Segments");

    // Frame 2
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
        "_global.real = [];"
        "_global.fns = [];"
        "_global.loops = 0;"
        "_global.c = 0;"
        "_global.ctorcalls = 0;"
        "if( !_global.Bug ) {"
	    "   _global.Bug = function () {"
        "       _global.ctorcalls++;"
	    "       this.onUnload = function() { "
        "           trace('unload ' + this + ' c: ' + this.c); "
        "       }; "
	    "       this.onLoad = function() { "
        "           trace('load ' + this + ' c: ' + this.c); "
        "       }; "
        "       this.c = _global.c;"
        "       trace('Bug ctor: ' + _global.c);"
        "       _global.c++;"
	    "   };"
	    "};"
	);

    initac = newSWFInitAction_withId(ac, 4);
    SWFMovie_add(mo, (SWFBlock)initac);
    
    ac = newSWFAction("Object.registerClass('Segments_Name',Bug);");
    initac = newSWFInitAction_withId(ac, 1);
    SWFMovie_add(mo, (SWFBlock)initac);
    
    add_actions(mo, "_global.fns.push(typeof(_level0.mc.Segments.onUnload));");
    add_actions(mo, "_global.real.push(_level0.mc.Segments.c);");
    add_actions(mo, "trace(_level0._currentframe + ' ' + _level0.mc.Segments.c);");

    // Frame 2 of the main timeline
    SWFMovie_nextFrame(mo);
    
    add_actions(mo, "_global.fns.push(typeof(_level0.mc.Segments.onUnload));");
    add_actions(mo, "_global.real.push(_level0.mc.Segments.c);");
    add_actions(mo, "trace(_level0._currentframe + ' ' + _level0.mc.Segments.c);");

    add_actions(mo,
        "    if (_global.loops < 5) {"
        "        _global.loops++;"
        "        gotoAndPlay(1);"
        "   }"
        "   else {"
        "      gotoAndPlay(3);"
        "   };"
        );
    
    SWFMovie_nextFrame(mo);
    
    check_equals(mo, "_global.real.length", "12");
    check_equals(mo, "_global.real.toString()",
            "'undefined,0,0,0,0,0,0,1,1,1,1,2'");

    check_equals(mo, "_global.fns.length", "12");
    check_equals(mo, "_global.fns[0]", "'undefined'");
    check_equals(mo, "_global.fns[1]", "'function'");
    check_equals(mo, "_global.fns[2]", "'function'");
    check_equals(mo, "_global.fns[3]", "'function'");
    check_equals(mo, "_global.fns[4]", "'function'");
    check_equals(mo, "_global.fns[5]", "'function'");
    check_equals(mo, "_global.fns[6]", "'function'");
    check_equals(mo, "_global.fns[7]", "'function'");
    check_equals(mo, "_global.fns[8]", "'function'");
    check_equals(mo, "_global.fns[9]", "'function'");
    check_equals(mo, "_global.fns[10]", "'function'");
    check_equals(mo, "_global.fns[11]", "'function'");
    
    check_equals(mo, "_global.ctorcalls", "3");

    SWFMovie_nextFrame(mo);
    add_actions(mo, "totals(16); stop();");

	// SWF_END 
    SWFMovie_save(mo, OUTPUT_FILENAME);

    return 0;
}

