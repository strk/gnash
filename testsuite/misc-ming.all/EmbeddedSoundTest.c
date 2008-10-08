/* 
 *   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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

#include "ming_utils.h"

#define OUTPUT_VERSION 6
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

    SWFMovie_addExport(mo, (SWFSound)soundMP3a, "somp3a");
    SWFMovie_addExport(mo, (SWFSound)soundMP3b, "somp3b");
    SWFMovie_addExport(mo, (SWFSound)soundMP3c, "somp3c");
    SWFMovie_addExport(mo, (SWFSound)soundMP3d, "somp3d");

    SWFMovie_writeExports(mo);
}

int
main(int argc, char** argv)
{
	SWFMovie mo;
	const char *srcdir=".";
	SWFMovieClip  dejagnuclip;


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
	SWFMovie_setRate(mo, 12);
	SWFMovie_setDimension(mo, 640, 400);

	/*********************************************
	 *
	 * Body
	 *
	 *********************************************/

	dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 80, 800, 600);
	SWFMovie_add(mo, (SWFBlock)dejagnuclip);
	addSoundExport(mo);

	SWFMovie_nextFrame(mo);  /* end of frame1 */


    add_actions(mo, "a = new Sound(); a.attachSound('somp3a');");
    add_actions(mo, "b = new Sound(); b.attachSound('somp3b');");
    add_actions(mo, "c = new Sound(); c.attachSound('somp3c');");
    add_actions(mo, "d = new Sound(); d.attachSound('somp3d');");

    check_equals(mo, "a.duration", "13740");
    check_equals(mo, "a.position", "0");
    add_actions(mo, "a.playSound();");
    check_equals(mo, "a.position", "0");

    check_equals(mo, "b.duration", "13740");
    check_equals(mo, "b.position", "0");
    add_actions(mo, "b.play();");
    check_equals(mo, "b.position", "0");

    check_equals(mo, "c.duration", "5224");
    check_equals(mo, "c.position", "0");
    add_actions(mo, "c.playSound();");
    check_equals(mo, "c.position", "0");

    check_equals(mo, "d.duration", "5224");
    check_equals(mo, "d.position", "0");
    add_actions(mo, "d.playSound();");
    check_equals(mo, "d.position", "0");
    
    add_actions(mo, "totals(); stop();");

	/*****************************************************
	 *
	 * Output movie
	 *
	 *****************************************************/

	puts("Saving " OUTPUT_FILENAME );

	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
