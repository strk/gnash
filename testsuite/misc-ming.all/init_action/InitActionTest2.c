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
#define OUTPUT_FILENAME "InitActionTest2.swf"


int
main(int argc, char** argv)
{
    SWFMovie mo;
    SWFMovieClip mc4, mc5, mc6, mc7, mc8, dejagnuclip;
    SWFDisplayItem it, it2;
    SWFAction ac;
    SWFInitAction ia;

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

    // Character ID: 1, 2
    dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10,
             0, 0, 800, 600);
    SWFMovie_add(mo, (SWFBlock)dejagnuclip);

    // Character ID: 4
    mc4 = newSWFMovieClip();
    SWFMovieClip_nextFrame(mc4);
    it = SWFMovie_add(mo, (SWFBlock)mc4);

    // InitActions for ID 2 parsed here:
    ia = newSWFInitAction_withId(
            newSWFAction("trace('mc4'); _global.val4 = 'mc4';"), 4);
    SWFMovie_add(mo, (SWFBlock)ia);
    
    // InitActions for non-existent ID 25 parsed here:
    ia = newSWFInitAction_withId(
            newSWFAction("fail('InitActions executed for bogus id');"), 25);
    SWFMovie_add(mo, (SWFBlock)ia);

    // Check in first frame:
    check(mo, "_global.val4 == undefined");

    // Frame 2
    SWFMovie_nextFrame(mo);
    
    // Check in next frame:
    check(mo, "_global.val4 == undefined");
    
    // Frame 3
    SWFMovie_nextFrame(mo);

    // Action is before export tag.
    check(mo, "_global.val4 == undefined");
    SWFMovie_addExport(mo, (SWFBlock)mc4, "export4");
    SWFMovie_writeExports(mo);
    check(mo, "_global.val4 == undefined");

    // Frame 4
    SWFMovie_nextFrame(mo);
    check(mo, "_global.val4 == undefined");

    // Add it again
    SWFMovie_add(mo, (SWFBlock)mc4);
    check(mo, "_global.val4 == undefined");

    // Frame 5
    SWFMovie_nextFrame(mo);
    check(mo, "_global.val4 == undefined");
    
    // Add it again, export it again:
    SWFMovie_add(mo, (SWFBlock)mc4);
    SWFMovie_addExport(mo, (SWFBlock)mc4, "export4");
    SWFMovie_writeExports(mo);
    check(mo, "_global.val4 == undefined");
    
    // Frame 6
    SWFMovie_nextFrame(mo);

    // MovieClip *must be exported*, SWFInitAction *must be after export*,
    // but not necessarily in the same frame.
    ia = newSWFInitAction_withId(
            newSWFAction("_global.val4 = 'mc4a';"), 4);
    SWFMovie_add(mo, (SWFBlock)ia);
    check(mo, "_global.val4 == 'mc4a'");
    
    // Frame 7
    SWFMovie_nextFrame(mo);
    
    // The MovieClip does not have to be placed, but must be exported.
    mc5 = newSWFMovieClip();
    SWFMovie_addExport(mo, (SWFBlock)mc5, "export5");
    SWFMovie_writeExports(mo);
    
    // Action is written before InitAction, but this does not matter. As
    // long as it's in the same frame it will work.
    check(mo, "_global.val5 == 'mc5'");
    
    ia = newSWFInitAction_withId(
            newSWFAction("trace('mc5'); _global.val5 = 'mc5';"), 5);
    SWFMovie_add(mo, (SWFBlock)ia);
    check(mo, "_global.val5 == 'mc5'");

    // Frame 8
    SWFMovie_nextFrame(mo);

    // Add new MovieClip and export.
    mc6 = newSWFMovieClip();
    SWFMovie_addExport(mo, (SWFBlock)mc6, "export6");
    SWFMovie_writeExports(mo);

    // Skip next frame
    add_actions(mo, "gotoAndPlay(10);");
    
    // Frame 9
    SWFMovie_nextFrame(mo);

    // This frame is skipped but contains init actions.
    ia = newSWFInitAction_withId(
            newSWFAction("trace('mc6'); _global.val6 = 'mc6';"), 6);
    SWFMovie_add(mo, (SWFBlock)ia);
    add_actions(mo, "fail('Actions in skipped frame executed!');");

    // Frame 10
    SWFMovie_nextFrame(mo);

    // Check that the skipped InitActions are executed.
    check(mo, "_global.val6 == 'mc6'");
    
    /// Now check what happens on a loop. The situation is:
    //
    //  11. Initactions defined for chars 7 and 8
    //  12. Chars 7 and 8 placed.
    //  13. Char 7 removed again, second time go to 15
    //  14. Checks, go to 11
    //  15. Checks
    //
    //  We expect that neither initaction tag is executed the first time,
    //  but both the second time, even though one of the characters was
    //  removed after being placed.

    // Frame 11
    SWFMovie_nextFrame(mo);

    // This should not be present either time because it's removed.
    check_equals(mo, "_root.mc7", "undefined");

    // This should not be present the first time
    check_equals(mo, "_global.goneBack ||! _root.mc8", "true");

    ac = newSWFAction("trace('init7'); _global.init7 = true;");
    ia = newSWFInitAction_withId(ac, 7);
    SWFMovie_add(mo, (SWFBlock)ia);
    
    ac = newSWFAction("trace('init8'); _global.init8 = true;");
    ia = newSWFInitAction_withId(ac, 8);
    SWFMovie_add(mo, (SWFBlock)ia);

    // Frame 12
    SWFMovie_nextFrame(mo);

    mc7 = newSWFMovieClip();
    it = SWFMovie_add(mo, (SWFBlock)mc7);
    SWFDisplayItem_setName(it, "mc7");

    mc8 = newSWFMovieClip();
    it2 = SWFMovie_add(mo, (SWFBlock)mc8);
    SWFDisplayItem_setName(it2, "mc8");
    
    // Frame 13
    SWFMovie_nextFrame(mo);

    // Remove mc7
    SWFMovie_remove(mo, it);

    ac = newSWFAction(
            "if (_global.goneBack) {"
            "   gotoAndPlay(15);"
            "};"
            );
    SWFMovie_add(mo, (SWFBlock)ac);
    
    // Frame 14
    SWFMovie_nextFrame(mo);

    // This frame performs checks after the first execution of the previous
    // frames and should be skipped the second time!
    
    // Check that neither initaction tag was executed.
    check_equals(mo, "_global.init7", "undefined");
    check_equals(mo, "_global.init8", "undefined");
    
    // Check that mc7 has been removed and mc8 is still there.
    check_equals(mo, "typeof(_root.mc7)", "'undefined'");
    check_equals(mo, "typeof(_root.mc8)", "'movieclip'");

    ac = newSWFAction(
            "if (!_global.goneBack) { "
            "   trace('Going back');"
            "   _global.goneBack = true;"
            // Go to frame 11 where the initactions are.
            "   gotoAndPlay(11);"
            "};"
            );
    
    SWFMovie_add(mo, (SWFBlock)ac);
    
    // Frame 15
    SWFMovie_nextFrame(mo);
            
    // Check that both initaction tags were executed.
    check_equals(mo, "_global.init7", "true");
    check_equals(mo, "_global.init8", "true");

    add_actions(mo, "stop();");
  
    puts("Saving " OUTPUT_FILENAME );
    SWFMovie_save(mo, OUTPUT_FILENAME);

    return 0;
}

