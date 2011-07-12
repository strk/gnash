/* 
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 */ 

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>
#include <errno.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 8
#define OUTPUT_FILENAME "EmbeddedSoundTest.swf"

void addSoundExport(SWFMovie mo);

void
addSoundExport(SWFMovie mo)
{
    SWFSound soundMP3a;
    SWFSound soundMP3b;
    SWFSound soundMP3c;
    SWFSound soundMP3d;

    FILE* f;
    FILE* f2;

    f = fopen(MEDIADIR"/mono44.mp2", "r");

    if (!f)
    {
	perror(MEDIADIR"/mono44.mp2");
        exit(EXIT_FAILURE);
    }
    soundMP3a = newSWFSound(f, SWF_SOUND_MP3_COMPRESSED |
            SWF_SOUND_44KHZ |
            SWF_SOUND_16BITS |
            SWF_SOUND_MONO);
    
    soundMP3b = newSWFSound(f, SWF_SOUND_MP3_COMPRESSED |
            SWF_SOUND_22KHZ |
            SWF_SOUND_16BITS |
            SWF_SOUND_STEREO);


    f2 = fopen(MEDIADIR"/stereo8.mp3", "r");
    if (f2 == NULL)
    {
	perror(MEDIADIR"/stereo8.mp3");
        exit(EXIT_FAILURE);
    }

    soundMP3c = newSWFSound(f2, SWF_SOUND_MP3_COMPRESSED |
            SWF_SOUND_44KHZ |
            SWF_SOUND_16BITS |
            SWF_SOUND_MONO);
    
    soundMP3d = newSWFSound(f2, SWF_SOUND_MP3_COMPRESSED |
            SWF_SOUND_5KHZ |
            SWF_SOUND_16BITS |
            SWF_SOUND_STEREO);

    SWFMovie_addExport(mo, (SWFBlock)soundMP3a, "mono22_mp2");
    SWFMovie_addExport(mo, (SWFBlock)soundMP3b, "mono22_mp2b");
    SWFMovie_addExport(mo, (SWFBlock)soundMP3c, "stereo8_mp3");
    SWFMovie_addExport(mo, (SWFBlock)soundMP3d, "stereo8_mp3b");

    SWFMovie_writeExports(mo);
}

int
main(int argc, char** argv)
{
	SWFMovie mo;
	const char *srcdir=".";
	SWFMovieClip  dejagnuclip;
    SWFDisplayItem it;


	/*********************************************
	 *
	 * Initialization
	 *
	 *********************************************/

	if ( argc>1 ) srcdir=argv[1];
	else
	{
		fprintf(stderr, "Usage: %s\n", argv[0]);
		return 1;
	}

	puts("Setting things up");

	Ming_init();
  Ming_useSWFVersion (OUTPUT_VERSION);
	Ming_setScale(20.0); /* let's talk pixels */
 
	mo = newSWFMovie();
	SWFMovie_setRate(mo, 1.33);
	SWFMovie_setDimension(mo, 640, 400);

	/*********************************************
	 *
	 * Body
	 *
	 *********************************************/

	dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 80, 800, 600);
	it = SWFMovie_add(mo, (SWFBlock)dejagnuclip);
	addSoundExport(mo);

	SWFMovie_nextFrame(mo);  /* end of frame1 */

    // Tracker variables for calls to onSoundComplete
    add_actions(mo, "c_soundComplete = 0;");
    add_actions(mo, "d_soundComplete = 0;");
    add_actions(mo, "e_soundComplete = 0;");

    add_actions(mo, "a = new Sound();");
    check_equals(mo, "a.duration", "undefined");
    add_actions(mo, "a.attachSound('mono22_mp2');");
    check_equals(mo, "a.duration", "13740");

    add_actions(mo, "b = new Sound(); b.attachSound('mono22_mp2b');");
    add_actions(mo, "c = new Sound(); c.attachSound('stereo8_mp3');");

    // Two different Sounds with the same exported sound.
    add_actions(mo, "d = new Sound(); d.attachSound('stereo8_mp3b');");
    add_actions(mo, "e = new Sound(); e.attachSound('stereo8_mp3b');");

    add_actions(mo, "check_equals(a.getBytesTotal(), undefined);");
    add_actions(mo, "check_equals(a.getBytesLoaded(), undefined);");
    add_actions(mo, "check_equals(a.id3, undefined);");
    check_equals(mo, "a.position", "0");
    add_actions(mo, "a.start();");
    // This isn't very consistent either. Please re-enable when it is.
    //check_equals(mo, "a.position", "0");

    check_equals(mo, "b.duration", "13740");
    check_equals(mo, "b.position", "0");
    add_actions(mo, "b.start();");

    // Here, gst gives 46, ffmpeg 0.
    //check_equals(mo, "b.position", "0");

    check_equals(mo, "c.duration", "5224");
    check_equals(mo, "c.position", "0");
    // Play twice (loop).
    add_actions(mo, "c.start(0, 2);");
    check_equals(mo, "c.position", "0");

    check_equals(mo, "d.duration", "5224");
    check_equals(mo, "d.position", "0");
    // Start twice.
    add_actions(mo, "d.start();");
    add_actions(mo, "d.start(4);");
    check_equals(mo, "d.position", "0");

    add_actions(mo, "e.start();");

    SWFMovie_nextFrame(mo);

    add_actions(mo, "stop();"
            "note('will wait for onSoundComplete to finish the test (about "
            "13 seconds).');");

    // This is the longest sound, so the test should end when this is called.
    add_actions(mo, "a.onSoundComplete = function() {"
            "check_equals(arguments.length, 0);"
            "check_equals(a.position, 13740, 'a.position at a.onSoundComplete time');"
            "check_equals(c_soundComplete, 1, 'c_soundComplete at a.onSoundComplete time');"
            "check_equals(d_soundComplete, 1, 'd_soundComplete at a.onSoundComplete time');"
            "check_equals(e_soundComplete, 2, 'e_soundComplete at a.onSoundComplete time');"
            "totals(26); "
            "finished = true;"
            "};");

    // Check position of b, c, d, and e after the first loop of c.
    add_actions(mo, "c.onSoundComplete = function() {"
            // I'm not sure how reliable this is:
            "check_equals(b.position, 10472, 'b.position at c.onSoundComplete time');"
            "check_equals(c.position, 5224, 'c.position at c.onSoundComplete time');"
            "check_equals(d.position, 5224, 'd.position at c.onSoundComplete time');"
            "check_equals(e.position, 5224, 'e.position at c.onSoundComplete time');"
            "c_soundComplete++;"
            "note('c.onSoundComplete() called '+c_soundComplete+' time(s).');"
            "};");

    add_actions(mo, "d.onSoundComplete = function() {"
            "check_equals(d.position, 5224, 'd.position at d.onSoundComplete time');"
            "d_soundComplete++;"
            "note('d.onSoundComplete() called '+d_soundComplete+' time(s).');"
            "};");

    // This starts e again. It should run twice before the longest
    // sound stops.
    add_actions(mo, "e.onSoundComplete = function() {"
            "check_equals(e.position, 5224, 'e.position at e.onSoundComplete time');"
            "e_soundComplete++;"
            "note('e.onSoundComplete() called '+e_soundComplete+' time(s).');"
            "if ( e_soundComplete < 2 ) e.start();"
            "};");


	/*****************************************************
	 *
	 * Output movie
	 *
	 *****************************************************/

	puts("Saving " OUTPUT_FILENAME );

	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
