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
    SWFMovieClip mc1, mc2, mc3, mc4, dejagnuclip;
    SWFAction ac;
    SWFDisplayItem it;
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
            newSWFAction("_global.val4 = 'mc4';"), 4);
    SWFMovie_add(mo, (SWFBlock)ia);

    // Check in first frame:
    check(mo, "_global.val4 == undefined");

    // Frame 2
    SWFMovie_nextFrame(mo);
    
    // Check in next frame:
    check(mo, "_global.val4 == undefined");
    
    // Frame 4
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

    // MovieClip *must be exported*, SWFInitAction *must be after export*
    ia = newSWFInitAction_withId(
            newSWFAction("_global.val4 = 'mc4';"), 4);
    SWFMovie_add(mo, (SWFBlock)ia);
    check(mo, "_global.val4 == 'mc4'");
    
    add_actions(mo, "stop();");
  
    puts("Saving " OUTPUT_FILENAME );
    SWFMovie_save(mo, OUTPUT_FILENAME);

    return 0;
}

