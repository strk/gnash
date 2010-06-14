#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 8
#define OUTPUT_FILENAME "RegisterClassTest4.swf"

int main(int argc, char* argv[])
{

    SWFMovie mo;
    SWFMovieClip mc1, mc2, mc3, mc4;
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
	// SWF_SHOWFRAME 
    SWFMovieClip_nextFrame(mc1);

	// SWF_EXPORTASSETS 
    SWFMovie_addExport(mo, (SWFBlock)mc1, "Segments_Name");
    SWFMovie_writeExports(mo);

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

	// SWF_DEFINESPRITE 

    //  MovieClip 2 
	mc2 = newSWFMovieClip(); // 1 frames 
	SWFMovieClip_nextFrame(mc2); // end of clip frame 1 

	// SWF_DEFINESPRITE 

    //  MovieClip 3 
	mc3 = newSWFMovieClip(); // 2 frames 

	// SWF_PLACEOBJECT2 
	it = SWFMovieClip_add(mc3, (SWFBlock)mc2);
    SWFDisplayItem_setDepth(it, 1);
	SWFDisplayItem_setName(it, "Segments");

    SWFMovieClip_nextFrame(mc3);

	// SWF_REMOVEOBJECT2 
	SWFDisplayItem_remove(it);

    it = SWFMovieClip_add(mc3, (SWFBlock)mc1);
    SWFDisplayItem_setDepth(it, 1);
	SWFDisplayItem_setName(it, "Segments");

	// SWF_SHOWFRAME 
	SWFMovieClip_nextFrame(mc3); // end of clip frame 2 

	// SWF_END 

	// SWF_PLACEOBJECT2 
	it = SWFMovie_add(mo, (SWFBlock)mc3);
    SWFDisplayItem_setDepth(it, 1);
    SWFDisplayItem_setName(it, "mc");


	// SWF_DEFINESPRITE 

    //  MovieClip 4 
	mc4 = newSWFMovieClip(); // 0 frames 

	// SWF_END 

	// SWF_EXPORTASSETS 
    SWFMovie_addExport(mo, (SWFBlock)mc4, "__Packages.Bug");
    SWFMovie_writeExports(mo);

    ac = newSWFAction(
        "trace('hhoho');"
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
    
    // Frame 2
    SWFMovie_nextFrame(mo);

    add_actions(mo,
//        "   trace('frame 2');"
//        "   trace('loops ' + _global.loops);"
        "    if (_global.loops < 5) {"
        "        _global.loops++;"
        "        gotoAndPlay(1);"
        "   }"
        "   else {"
//        "      trace('Should be finished');"
        "      delete this.onEnterFrame;"
        "   };"
        );
    
    SWFMovie_nextFrame(mo);
    
    add_actions(mo, "stop();");

	// SWF_END 
    SWFMovie_save(mo, OUTPUT_FILENAME);

    return 0;
}

