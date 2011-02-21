#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 8
#define OUTPUT_FILENAME "PlaceAndRemove.swf"

/* Tests placing and removal; not really action order, but
 * rather what happens when placing happens after removal */

int main(int argc, char* argv[])
{

    SWFMovie mo;
    SWFMovieClip mc50, mc51, mc74;
    SWFDisplayItem it3, it52;

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

    // Empty character 50.
    mc50 = newSWFMovieClip();
    SWFMovieClip_add(mc50, (SWFBlock)newSWFAction("trace('moo ' + ++_global.counter);"));
    SWFMovieClip_nextFrame(mc50);

    mc51 = newSWFMovieClip();
    SWFMovieClip_add(mc51, (SWFBlock)newSWFAction(
                "function doStuff() {"
                "    gotoAndPlay(2);"
                "};"
                "_global.setTimeout(doStuff, 524);"
                "trace('Done');"
                "stop();"
                ));
    SWFMovieClip_nextFrame(mc51);

    it3 = SWFMovieClip_add(mc51, (SWFBlock)mc50);
    SWFDisplayItem_setDepth(it3, 3);

    SWFMovieClip_nextFrame(mc51);


    mc74 = newSWFMovieClip();

    SWFMovieClip_addInitAction(mc74, newSWFAction("_global.counter = 0;"));

    it52 = SWFMovieClip_add(mc74, (SWFBlock)mc51);
    SWFDisplayItem_setDepth(it52, 52);
    SWFDisplayItem_remove(it52);
    SWFMovieClip_nextFrame(mc74);
    it52 = SWFMovieClip_add(mc74, (SWFBlock)mc51);
    SWFMovieClip_nextFrame(mc74);

    SWFMovie_add(mo, (SWFBlock)mc74);
    SWFMovie_nextFrame(mo);

    SWFMovie_save(mo, OUTPUT_FILENAME);

    return EXIT_SUCCESS;

}

