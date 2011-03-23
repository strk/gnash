/***********************************************************************
 *
 *   Copyright (C) 2005, 2006, 2009, 2010, 2011 Free Software Foundation, Inc.
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 ***********************************************************************
 *
 * Test case for mouse events.
 *
 * In a movie of 120x120 pixels, it places a movieclip containing a squared
 * sprite-button in the middle of the stage, and a text area on top.
 *
 * The movie has 3 frames, with the second adding a shape at a lower depth
 * and the third one at an higher depth respect to the button.
 *
 * The following events print the event name in the text area
 * (called _root.textfield) and change the color of the button:
 *
 * RollOut  : red button (initial state)
 * RollOver : yellow button
 * Press    : green button
 * Release  : yellow button (same as MouseOver, but the label on top changes)
 *
 ***********************************************************************/

#include "ming_utils.h"

#include <ming.h>
#include <stdio.h>
#include <stdlib.h>

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "ResolveEventsTest.swf"

int
main(int argc, char **argv)
{
    SWFMovie mo;
    SWFDisplayItem it;
    const char *srcdir=".";
    char fdbfont[256];
    SWFMovieClip dejagnuclip;
    SWFFont font;

    puts("Setting things up");

    Ming_init();
    Ming_useSWFVersion (OUTPUT_VERSION);
    Ming_setScale(20.0); 
 
    mo = newSWFMovie();
    SWFMovie_setDimension(mo, 800, 600);
    SWFMovie_setRate(mo, 1);

    if (argc > 1) srcdir=argv[1];
    else {
        fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
        return 1;
    }

    sprintf(fdbfont, "%s/Bitstream-Vera-Sans.fdb", srcdir);
    FILE* font_file = fopen(fdbfont, "r");
    if (!font_file) {
        perror(fdbfont);
        exit(1);
    }

    font = loadSWFFontFromFile(font_file);

    /* Dejagnu equipment */
    dejagnuclip = get_dejagnu_clip((SWFBlock)font, 10, 0, 0, 800, 600);
    it = SWFMovie_add(mo, (SWFBlock)dejagnuclip);
     
    SWFDisplayItem_setDepth(it, 200); 
    SWFDisplayItem_move(it, 200, 0); 

    SWFMovie_nextFrame(mo);

    add_actions(mo,
            "mc1 = _root.createEmptyMovieClip('mc1', 34);"
            "mc2 = mc1.createEmptyMovieClip('mc2', 35);"
            "with(mc2) {"
            "   moveTo(100, 100);"
            "   beginFill(0x20fff0);"
            "   lineTo(100, 200);"
            "   lineTo(200, 200);"
            "   lineTo(200, 100);"
            "};"
            "_root.onMouseUp = function() { play(); };"
            );
    
    // One has all the functions, one has __resolve.
    add_actions(mo,
            "resolveevents = [];"
            "events = [];"
            "mc1.__resolve = function(a) { resolveevents.push(a); };"
            "mc2.onEnterFrame = function() { events.push('onEnterFrame'); };"
            "mc2.onMouseDown = function() { events.push('onMouseDown'); };"
            "mc2.onMouseUp = function() { events.push('onMouseUp'); };"
            // Gnash sends a bogus event here, but we're not testing that!
            //"mc2.onLoad = function() { events.push('onLoad'); };"
            "mc2.onPress = function() { events.push('onPress'); };"
            "mc2.onRelease = function() { events.push('onRelease'); };"
            "mc2.onRollOver = function() { events.push('onRollOver'); };"
            "mc2.onRollOut = function() { events.push('onRollOut'); };"
            "mc2.onInitialize = function() { events.push('onInitialize'); };"
            "mc2.onConstruct = function() { events.push('onConstruct'); };"
            "mc2.onReleaseOutside = function() { events.push('onReleaseOutside'); };"
            "mc2.onDragOver = function() { events.push('onDragOver'); };"
            "mc2.func = function() { events.push('func'); };"
    );

    add_actions(mo,
            "mc2.func();"
            "mc1.func();"
            "mc2.onEnterFrame();"
            "mc1.onEnterFrame();"
            "mc2.onRollOver();"
            "mc1.onRollOver();"
            );
    
    add_actions(mo, "_root.note('Do not touch anything!');");

    // The ones called manually should appear in both.
    check_equals(mo, "events.toString()", "'func,onEnterFrame,onRollOver'");
    check_equals(mo, "resolveevents.toString()",
            "'func,onEnterFrame,onRollOver'");

    SWFMovie_nextFrame(mo);
    
    check_equals(mo, "events.toString()",
            "'func,onEnterFrame,onRollOver,onEnterFrame'");
    check_equals(mo, "resolveevents.toString()",
            "'func,onEnterFrame,onRollOver'");
    
    // Otherwise the number is unpredictable.
    add_actions(mo, "mc2.onEnterFrame = function() {};");

    add_actions(mo, "stop();");
    add_actions(mo, "_root.note('Move over square, click and release!');");

    SWFMovie_nextFrame(mo);

    // Expect rollover, onMouseDown, onMouseUp, onRelease
    check_equals(mo, "events.toString()",
            "'func,onEnterFrame,onRollOver,onEnterFrame,"
            "onRollOver,onMouseDown,onPress,onMouseUp,onRelease'");
    check_equals(mo, "resolveevents.toString()",
            "'func,onEnterFrame,onRollOver'");

    add_actions(mo, "_root.note('Move out of square, click and release!');");
    add_actions(mo, "stop();");
    
    SWFMovie_nextFrame(mo);
    check_equals(mo, "events.toString()",
            "'func,onEnterFrame,onRollOver,onEnterFrame,onRollOver,"
            "onMouseDown,onPress,onMouseUp,onRelease,onRollOut,onMouseDown,"
            "onMouseUp'");
    check_equals(mo, "resolveevents.toString()",
            "'func,onEnterFrame,onRollOver'");
    
    add_actions(mo,
            "mevents = [];"
            "ressiz = resolveevents.length;"
            "mc2.onMouseMove = function() { mevents.push('onMouseMove'); };");
    // Last test!
    add_actions(mo, "_root.note('Move the mouse about, then click');");

    add_actions(mo, "stop();");
    
    SWFMovie_nextFrame(mo);
    // Check that mouse events aren't carried to resolve.
    check_equals(mo, "mevents[0]", "'onMouseMove'");
    check_equals(mo, "resolveevents.length", "ressiz");

    add_actions(mo, "_root.note('End of test!');");

    // Finish and save
    SWFMovie_nextFrame(mo);
    add_actions(mo, "stop();");

    puts("Saving " OUTPUT_FILENAME );
    SWFMovie_save(mo, OUTPUT_FILENAME);
     
    return 0;
}
