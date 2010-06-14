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

#if 1
    ac = newSWFAction(
        "fs = [];"
		"onEnterFrame = function () {"
        "   fs.push(_level0.mc.Segments.onUnload);"
        "   trace(fs[fs.length - 2] == fs[fs.length - 1]);"
        "   trace((_level0.mc._currentframe+': ')+_level0.mc.Segments.onUnload);"
        "   trace(mc.Segments.c);"
        "};"
	);
	SWFMovie_add(mo, (SWFBlock)ac);
#endif

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
        "_global.expec = [ 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2 ];"
        "_global.pos = 0;"
        "_global.loops = 0;"
        "_global.c = 0;"
        "if( !_global.Bug ) {"
	    "   _global.Bug = function () {"
	    "       this.onUnload = function() {}; "
        "       this.c = _global.c;"
        "       _global.c++;"
	    "   };"
	    "};"
	);

    initac = newSWFInitAction_withId(ac, 4);
    SWFMovie_add(mo, (SWFBlock)initac);
    
    ac = newSWFAction("Object.registerClass('Segments_Name',Bug);");
    initac = newSWFInitAction_withId(ac, 1);
    SWFMovie_add(mo, (SWFBlock)initac);
    
    check_equals(mo, "typeof(_level0.mc.Segments.onUnload)", "'function'");
    check_equals(mo, "_level0.mc.Segments.c", "_global.expec[_global.pos]");
    add_actions(mo, "_global.pos++;");

    // Frame 2 of the main timeline
    SWFMovie_nextFrame(mo);
    
    check_equals(mo, "typeof(_level0.mc.Segments.onUnload)", "'function'");
    check_equals(mo, "_level0.mc.Segments.c", "_global.expec[_global.pos]");
    add_actions(mo, "_global.pos++;");

    add_actions(mo,
        "    if (_global.loops < 5) {"
        "        _global.loops++;"
        "        gotoAndPlay(1);"
        "   }"
        "   else {"
        "      trace(this);"
        "      delete this.onEnterFrame;"
        "      gotoAndPlay(3);"
        "   };"
        );
    
    SWFMovie_nextFrame(mo);
    
    SWFMovie_nextFrame(mo);
    add_actions(mo, "totals(); stop();");

	// SWF_END 
    SWFMovie_save(mo, OUTPUT_FILENAME);

    return 0;
}

