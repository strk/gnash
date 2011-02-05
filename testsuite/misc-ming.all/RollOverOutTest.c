/***********************************************************************
 *
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
 *   2011 Free Software Foundation, Inc.
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
 * The movie has 3 frames.
 * Test1: mouse events.  Test2: hitTest.
 *
 *  - frame1: initialization
 *  - frame2: place a red and a green square, red square is set to invisible.
 *  - frame3: red square is set to visible, green square is set to invisible.
 *  - frame4: remove the red and green squares, place 4 black squares.
 *
 * In frame2, rollOver event moves the playhead to frame3.
 * In frame3, rollOut  event moves the playhead to frame2.
 * In frame3, press    event moves the playhead to frame4.
 *
 ***********************************************************************/

#include "ming_utils.h"

#include <ming.h>
#include <stdio.h>
#include <stdlib.h>

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "RollOverOutTest.swf"

SWFFont font;

void add_event(SWFMovie mo, const char* name, const char* event, const char* action);
void add_text_field(SWFMovie mo, const char* name, int depth);
SWFDisplayItem add_square(SWFMovie mo, byte r, byte g, byte b, int depth);

void
add_event(SWFMovie mo, const char* name, const char* event, const char* action)
{
    SWFAction ac;
    char buf[1024];

    sprintf(buf,
    "event=undefined;"
    "%s.on%s=function() { %s; };"
    , name, event, action
    );
    ac = compileSWFActionCode(buf);

    SWFMovie_add(mo, (SWFBlock)ac);
}

SWFDisplayItem add_square(SWFMovie mo, byte r, byte g, byte b, int depth)
{
    SWFDisplayItem it;
    SWFMovieClip mc;
    SWFShape sh;
    mc = newSWFMovieClip();
    sh = make_fill_square(0, 0, 40, 40, r, g, b, r, g, b);
    SWFMovieClip_add(mc, (SWFBlock)sh);
    SWFMovieClip_nextFrame(mc); /* showFrame */
    it = SWFMovie_add(mo, (SWFBlock)mc);
    SWFDisplayItem_setDepth(it, depth);
    return it;
}

void
add_text_field(SWFMovie mo, const char* name, int depth)
{
    SWFDisplayItem it;
    SWFTextField tf = newSWFTextField();
    SWFTextField_setFlags(tf, SWFTEXTFIELD_DRAWBOX);

    SWFTextField_setFont(tf, (void*)font);
    SWFTextField_addChars(tf, " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ012345689:.,/\\#@?!");
    SWFTextField_addString(tf, " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ012345689:.,/\\#@?!");
    SWFTextField_setVariableName(tf, name);

    it = SWFMovie_add(mo, (SWFBlock)tf);
    SWFDisplayItem_moveTo(it, 0, 10);
    SWFTextField_setBounds(tf, 600, 20);
    SWFDisplayItem_setDepth(it, depth);
}

int
main(int argc, char **argv)
{
    SWFMovie mo;
    SWFDisplayItem it1, it2, it3;
    SWFMovieClip dejagnuclip;
    char fdbfont[256];

    /*********************************************
     *
     * Initialization
     *
     *********************************************/
    const char *srcdir=".";
    if ( argc>1 ) 
        srcdir=argv[1];
    else
    {
       fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
       return 1;
    }

    puts("Setting things up");

    Ming_init();
    Ming_useSWFVersion (OUTPUT_VERSION);
    Ming_setScale(20.0); 
 
    mo = newSWFMovie(); 
    SWFMovie_setDimension(mo, 800, 600);

    dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
    SWFMovie_add(mo, (SWFBlock)dejagnuclip);

    sprintf(fdbfont, "%s/Bitstream-Vera-Sans.fdb", srcdir);
    FILE *font_file = fopen(fdbfont, "r");
    if ( font_file == NULL )
    {
        perror(fdbfont);
        exit(1);
    }
    font = loadSWFFontFromFile(font_file);
    add_text_field(mo, "_root.msg", 100);
    
    SWFMovie_nextFrame(mo); // frame1

    /*****************************************************
     *
     * Add squares
     *
     *****************************************************/

    it1 = add_square(mo, 255, 0, 0, 3);
    SWFDisplayItem_moveTo(it1, 40, 40);
    SWFDisplayItem_setName(it1, "square1");

    it2 = add_square(mo, 0, 255, 0, 4);
    SWFDisplayItem_moveTo(it2, 40, 40);
    SWFDisplayItem_setName(it2, "square2");

    /*****************************************************
     *
     * Frame 2: display a red square.
     *          onRollOver: goto frame3
     *
     *****************************************************/

    add_actions(mo, "_root.msg = 'Frame2: move the mouse on the square'; ");
    add_actions(mo, "square1._visible = true; square2._visible=false;");

    add_event(mo, "square1", "RollOver", "gotoAndPlay(3)");

    add_actions(mo, "stop();");
    SWFMovie_nextFrame(mo); // frame2

    /*****************************************************
     *
     * Frame 3: display a green square.
     *          onRollOut: goto frame2
     *
     *****************************************************/

    add_actions(mo, 
        " _root.msg = 'Frame3: move the mouse off the square"
        " or press the square with the left button'; ");
    add_actions(mo, "square2._visible = true; square1._visible=false;");

    add_event(mo, "square2", "RollOut", "gotoAndPlay(2)");
    add_event(mo, "square2", "Press", "gotoAndPlay(4)"  );

    add_actions(mo, "stop();");
    SWFMovie_nextFrame(mo); // frame3

    //
    //  Frame4: 
    //  (0) remove the squares placed at frame1.
    //  (1) place a visible black square. 
    //  (2) place a invisible black square.
    //  (3) place a static mask.
    //  (4) place a dynamic mask.
    //  
    // Expected behaviour:
    //  (1)hitTest works for both visible and invisible sprites.
    //  (2)hitTest works for both static mask and maskee sprites.
    //  (3)hitTest does not work for dynamic masks created by drawing API.
    //
    SWFDisplayItem_remove(it1);
    SWFDisplayItem_remove(it2);
    add_actions(mo, "_root.msg = 'Frame4: move the mouse on the squares'; ");
    
    it3 = add_square(mo, 0, 0, 0, 10);
    SWFDisplayItem_moveTo(it3, 60, 100);
    SWFDisplayItem_setName(it3, "visible_mc");
    
    it3 = add_square(mo, 0, 0, 0, 11);
    SWFDisplayItem_moveTo(it3, 60, 150);
    SWFDisplayItem_setName(it3, "invisible_mc");
    add_actions(mo, "invisible_mc._visible = false; ");
    
    it3 = add_square(mo, 0, 0, 0, 12);
    SWFDisplayItem_moveTo(it3, 60, 200);
    SWFDisplayItem_setName(it3, "static_mask");
    SWFDisplayItem_setMaskLevel(it3, 13);
    
    add_actions(mo, 
        " _root.createEmptyMovieClip('dynamic_mask', 15); "
        " _root.createEmptyMovieClip('maskee', 14); "
        " with(maskee) {"
        "   beginFill(0); "
        "    moveTo( 60, 250); "
        "    lineTo(100, 250); "
        "    lineTo(100, 290); "
        "    lineTo( 60, 290); "
        "}"
        " maskee.setMask(dynamic_mask); "
        " with(dynamic_mask) { "
        "    beginFill(0xff0000); "
        "    moveTo( 60, 250); "
        "    lineTo(100, 250); "
        "    lineTo(100, 290); "
        "    lineTo( 60, 290); "
        "}"
        "stop();");
    // hitTest works for visible sprites.
    check(mo, "visible_mc.hitTest(80, 120, true)");
    // hitTest works for invisible sprites.
    check(mo, "invisible_mc.hitTest(80, 180, true)");
    // hitTest works for static placed maskes.
    check(mo, "static_mask.hitTest(80, 240, true)");
    // hitTest does not work for dynamic masks created by drawing API.
    check(mo, "! dynamic_mask.hitTest(80, 280, true)");
    // hitTest works for maskee sprites.
    check(mo, "maskee.hitTest(80, 280, true)");
    SWFMovie_nextFrame(mo); // frame4
        
    // save this file to a swf.
    puts("Saving " OUTPUT_FILENAME );
    SWFMovie_save(mo, OUTPUT_FILENAME);

    return 0;
}

