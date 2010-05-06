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
    SWFMovieClip mc3, mc2, dejagnuclip;
    SWFAction ac, ac1;
    SWFDisplayItem it;
    SWFShape sha;

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

    sha = newSWFShape();

    // Character ID 2. Has 1 showframe. Is exported first.
    mc2 = newSWFMovieClip();
    SWFMovieClip_add(mc2, (SWFBlock)sha);
    SWFMovieClip_nextFrame(mc2);

    // Export it.
    SWFMovie_addExport(mo, (SWFBlock)mc2, "C2");
	SWFMovie_writeExports(mo);

    // Main timeline actions for frame 1
    add_actions(mo, "trace('frame 1'); gotoAndStop(3);");
    
    // ID 3 is defined here. It has no showframe. It is exported immediately.
    mc3 = newSWFMovieClip();
    SWFMovie_addExport(mo, (SWFBlock)mc3, "ctor");
	SWFMovie_writeExports(mo);

    // Init actions for ID 3
    ac = newSWFAction(
    "   _global.ctor = function () {"
    "       super();"
    "       trace('Object in Frame 2 is constructed');"
    "   };"
    );
    SWFInitAction ia = newSWFInitAction_withId(ac, 3);
    SWFMovie_add(mo, (SWFBlock)ia);
    
    // Init actions for ID 2 (registered class)
    ac1 = newSWFAction("Object.registerClass('C2', ctor); "
            "trace('Registered class');");
    ia = newSWFInitAction_withId(ac1, 2);
    SWFMovie_add(mo, (SWFBlock)ia);
	
    
    // Frame 2
    SWFMovie_nextFrame(mo);
    add_actions(mo, "trace('Frame 2');");
    
    // Place object ID 2 (should be skipped).
    it = SWFMovie_add(mo, (SWFBlock)mc2);
    
    // Frame 3
    SWFMovie_nextFrame(mo);

    add_actions(mo, "trace('frame 3');");
    
    SWFMovie_nextFrame(mo);
  
    puts("Saving " OUTPUT_FILENAME );
    SWFMovie_save(mo, OUTPUT_FILENAME);

    return 0;
}

