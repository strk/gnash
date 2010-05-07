#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 8
#define OUTPUT_FILENAME "RegisterClassTest3.swf"


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

    mc1 = newSWFMovieClip();
    SWFMovieClip_nextFrame(mc1);
    
    add_actions(mo, "onEnterFrame = function() { "
            "trace(_level0.mc._currentframe + ':' + "
            "           _level0.mc.Segments.a);"
            "};"
            );
    
    mc2 = newSWFMovieClip();
    SWFMovieClip_nextFrame(mc2);
    SWFMovie_addExport(mo, (SWFBlock)mc2, "MC1");
    SWFMovie_writeExports(mo);

    mc3 = newSWFMovieClip();
    it = SWFMovieClip_add(mc3, (SWFBlock)mc2);
    SWFDisplayItem_setName(it, "Segments");
    SWFMovieClip_nextFrame(mc3);
    SWFMovieClip_remove(mc3, it);
    it = SWFMovieClip_add(mc3, (SWFBlock)mc1);
    SWFMovieClip_nextFrame(mc3);

    it = SWFMovie_add(mo, (SWFBlock)mc3);
    SWFDisplayItem_setName(it, "mc");

    mc4 = newSWFMovieClip();
    
    SWFMovie_addExport(mo, (SWFBlock)mc4, "MC4");
    SWFMovie_writeExports(mo);

    ia = newSWFInitAction_withId(newSWFAction(
                "if (!_global.Bug) {"
                "   _global.Bug = function() {"
                "       super();"
                "       this.onLoad = function() {};"
                "       this.onUnload = function() {};"
                "       this.a = 5;"
                "   };"
                "} "
                ), 4);
    SWFMovie_add(mo, (SWFBlock)ia);
    
    ia = newSWFInitAction_withId(newSWFAction(
                "Object.registerClass('MC1', Bug);"
                ), 1);
    SWFMovie_add(mo, (SWFBlock)ia);

    //dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10,
    //		    0, 0, 800, 600);
    //SWFMovie_add(mo, (SWFBlock)dejagnuclip);

    SWFMovie_nextFrame(mo);
  
    puts("Saving " OUTPUT_FILENAME );
    SWFMovie_save(mo, OUTPUT_FILENAME);

    return 0;
}

