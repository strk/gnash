/* 
 *   Copyright (C) 2007, 2009, 2010, 2011 Free Software Foundation, Inc.
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
 */ 

/*
 * Test for event sounds, self-contained self-documented
 */


#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "eventSoundTest1.swf"

void setupMovie(SWFMovie mo, const char* srcdir);
SWFSound setupSounds(const char* filename);
void runMultipleSoundsTest(SWFMovie mo, SWFSound so, int* frame);
void runNoMultipleSoundsTest(SWFMovie mo, SWFSound so, int* frame);
void runTrimmedSoundsTest(SWFMovie mo, SWFSound so, int* frame);
void runAttachedSoundsTest(SWFMovie mo, SWFSound so, int* frame);
void pauseForNextTest(SWFMovie mo);
void endOfTests(SWFMovie mo);
void printFrameInfo(SWFMovie mo, int i, const char* desc);

void pauseForNextTest(SWFMovie mo)
{
  add_actions(mo, "_root.onMouseDown = function() {"
                  "play(); Mouse.removeListener(_root); };"
                  "Mouse.addListener(_root);"
                  );
  add_actions(mo, "note('Click and "
                  "wait for the test.');"
		  "testReady = true; stop();");
}

void endOfTests(SWFMovie mo)
{
  add_actions(mo, "stop(); _root.endOfTest=1; note('END OF TESTS');");
}

void setupMovie(SWFMovie mo, const char* srcdir)
{
  SWFDisplayItem it;
  char fdbfont[256];
  SWFFont font;
  SWFMovieClip dejagnuclip;
  FILE* font_file;
  
  sprintf(fdbfont, "%s/Bitstream-Vera-Sans.fdb", srcdir);
  font_file = fopen(fdbfont, "r");
  if (font_file == NULL)
  {
    perror(fdbfont);
    exit(1);
  }

  font = loadSWFFontFromFile(font_file);

  /* Add output textfield and DejaGnu stuff */
  dejagnuclip = get_dejagnu_clip((SWFBlock)font, 10, 0, 0, 800, 800);
  it = SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  SWFDisplayItem_setDepth(it, 200); 

}

SWFSound setupSounds(const char* filename)
{
  FILE *soundfile;
  SWFSound so;
  
  printf("Opening sound file: %s\n", filename);

  soundfile = fopen(filename, "r");

  if (!soundfile) {
    perror(filename);
    exit(EXIT_FAILURE);
  }

  so = newSWFSound(soundfile,
     SWF_SOUND_NOT_COMPRESSED |
     SWF_SOUND_22KHZ |
     SWF_SOUND_16BITS |
     SWF_SOUND_STEREO);
  
  return so;
}


void
printFrameInfo(SWFMovie mo, int i, const char* desc)
{
    char acts[2048];

    sprintf(acts, "note('Frame: ' + %d + ' - %s');", i, desc);

    /* Display frame number and description
    at the beginning of each frame */
    add_actions(mo, acts);
}

void
runAttachedSoundsTest(SWFMovie mo, SWFSound so, int* frame)
{
    const char* frameDesc[5];
    int i;

    SWFMovie_nextFrame(mo);
    add_actions(mo,
              "note('\nAttached Embedded Sound Test.\n"
              "The notes should start exactly at the beginning of a frame "
              "(to coincide with the appearance of the description text).\n"
              "Test should start in two seconds.');");

    /* This is what is supposed to happen in each frame */
    frameDesc[0] = "Two notes (C, E)";
    frameDesc[1] = "Two notes (G-C, E)";
    frameDesc[2] = "Two notes (G-C, E)";
    frameDesc[3] = "Two notes (G-C, E)";
    frameDesc[4] = "Nothing";

    add_actions(mo, "t = _root.createEmptyMovieClip('mc', 9);"
            "cs = 0; cs2 = 0; onLoadCalls = 0; onLoadCalls2 = 0;"
            "s = new Sound(mc);"
            "s2 = new Sound(mc);"
            "s.attachSound('es');"
            "s2.attachSound('es');"
            "s.onLoad = function() { onLoadCalls++; };"
            "s2.onLoad = function() { onLoadCalls2++; };"
            "s.onSoundComplete = function() { cs++; };"
            "s2.onSoundComplete = function() { cs2++; };"
            );
    
    /// Start an embedded sound using a tag to make sure onSoundComplete
    /// isn't called for Sound s.
    SWFMovie_startSound(mo, so);
    
    /// Start the same embedded sound from another Sound object also to
    /// make sure the correct object is notified.
    add_actions(mo, "s2.start(); delete s2;");

    printFrameInfo(mo, 0, frameDesc[0]);

    for (i = 1; i < 4; i++)
    {
        SWFMovie_nextFrame(mo);
        
        (*frame)++;

        printFrameInfo(mo, i, frameDesc[i]);
        add_actions(mo, "s.start();");
    }

    xcheck_equals(mo, "cs", "1");

    SWFMovie_nextFrame(mo);
    
    xcheck_equals(mo, "cs", "2");
    
    // Check that Sound.onSoundComplete isn't executed if the Sound is
    // deleted. This only passes currently because onSoundComplete is never
    // called under any circumstances for embedded sounds.
    check_equals(mo, "cs2", "0");

    // Check that Sound.onLoad isn't executed for embedded sounds
    check_equals(mo, "onLoadCalls", "0");
    check_equals(mo, "onLoadCalls2", "0");

    check_equals(mo, "s.duration", "3000");

    /* about 2/3 of the sound is played a this time. exact time
     * is unreliable outside of controlled testing infrastructure:
     * PP 10 gives 1997 ms, gtk-gnash gives 1973.
     * MovieTester (controlling exact timeing) gives plain 2000
     */
    /* add_actions(mo, "note('Position: '+s.position);"); */
    check_equals(mo, "Math.round(s.position/1000)", "2");

    add_actions(mo, "s.stop();");

    printFrameInfo(mo, i, frameDesc[i]);

}


void
runMultipleSoundsTest(SWFMovie mo, SWFSound so, int* frame)
{
    const char* frameDesc[5];
    int i;

    SWFMovie_nextFrame(mo);
    add_actions(mo,
              "note('\nMultiple Sound Test.\n"
              "The notes should start exactly at the beginning of a frame "
              "(to coincide with the appearance of the description text)"
              "');\n");

    /* This is what is supposed to happen in each frame */
    frameDesc[0] = "Two notes (C, E)";
    frameDesc[1] = "Two notes (G-C, E)";
    frameDesc[2] = "Two notes (G-C, E)";
    frameDesc[3] = "Two notes (G-C, E)";
    frameDesc[4] = "Nothing";

    for (i = 0; i < 4; i++)
    {

        printFrameInfo(mo, i, frameDesc[i]);

        SWFMovie_startSound(mo, so);

        SWFMovie_nextFrame(mo); (*frame)++;
    }

    //SWFMovie_nextFrame(mo);

    printFrameInfo(mo, i, frameDesc[i]);
    SWFMovie_stopSound(mo, so);

}


void
runNoMultipleSoundsTest(SWFMovie mo, SWFSound so, int* frame)
{
  const char* frameDesc[5];
  int i;

  SWFMovie_nextFrame(mo);
  add_actions(mo,
              "note('\nNon-multiple Sound Test\n"
              "The notes should start exactly at the beginning "
              "of a frame (to coincide with the appearance of the description "
              "text)');");
            

  /* This is what is supposed to happen in each frame */
  frameDesc[0] = "Two notes (C, E)";
  frameDesc[1] = "One note (G)";
  frameDesc[2] = "Two notes (C, E) ";
  frameDesc[3] = "One note (G)";
  frameDesc[4] = "Nothing";


    for (i = 0; i < 4; i++)
    {

        printFrameInfo(mo, i, frameDesc[i]);

        SWFSoundInstance so_in = SWFMovie_startSound(mo, so);
        SWFSoundInstance_setNoMultiple(so_in);

        SWFMovie_nextFrame(mo); (*frame)++;
    }

    //SWFMovie_nextFrame(mo);

    printFrameInfo(mo, i, frameDesc[i]);
    SWFMovie_stopSound(mo, so);

}

void
runTrimmedSoundsTest(SWFMovie mo, SWFSound so, int* frame)
{
    SWFSoundInstance so_in;

    SWFMovie_nextFrame(mo);
    add_actions(mo,
              "note('\nTrimmed Sound Test.\n"
              "The notes should start exactly at the beginning of a frame "
              "(to coincide with the appearance of the description text');");

    // outPoint
    so_in = SWFMovie_startSound(mo, so);
    SWFSoundInstance_setLoopOutPoint(so_in, 44100*2);
    printFrameInfo(mo, 0, "Two notes (C, E)");
    SWFMovie_nextFrame(mo); (*frame)++;

    // inPoint
    so_in = SWFMovie_startSound(mo, so);
    SWFSoundInstance_setLoopInPoint(so_in, 44100);
    // We ask not to start this if previous is still going,
    // so if outPoint didn't work it's easly to detect
    SWFSoundInstance_setNoMultiple(so_in);

    printFrameInfo(mo, 1, "Two notes (E, G)");
    SWFMovie_nextFrame(mo); (*frame)++;

    // inPoint + outPoint + loop
    so_in = SWFMovie_startSound(mo, so);
    SWFSoundInstance_setLoopInPoint(so_in, 44100);
    SWFSoundInstance_setLoopOutPoint(so_in, 44100*2);
    SWFSoundInstance_setLoopCount(so_in, 2);
    printFrameInfo(mo, 2, "One note (E)");
    SWFMovie_nextFrame(mo); (*frame)++;

    printFrameInfo(mo, 3, "One note (E)");
    SWFMovie_nextFrame(mo); (*frame)++;

    printFrameInfo(mo, 4, "Nothing");
    SWFMovie_nextFrame(mo); (*frame)++;

}


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFSound so;
  const char* soundFile;
  const char* srcdir;
  int frame;

  if (argc > 1) {
    soundFile = argv[1];
  }
  else {
    soundFile = "brokenchord.wav";
  }
  if (argc > 2) {
    srcdir = argv[2];
  }
  else {
    srcdir = ".";
  }

  /* setup ming and basic movie properties */
  Ming_init();  
  Ming_useSWFVersion(OUTPUT_VERSION);

  mo = newSWFMovie();
  SWFMovie_setDimension(mo, 800, 600);
  SWFMovie_setRate(mo, 0.5);

  setupMovie(mo, srcdir);
  so = setupSounds(soundFile);
  
  /// Add as an export so we can attach it.
  SWFMovie_addExport(mo, (SWFBlock)so, "es");
  SWFMovie_writeExports(mo);

  add_actions(mo, "c = 0;");

  SWFMovie_nextFrame(mo);

  add_actions(mo,
       "note('You will hear several short tests with a succession of sounds. "
       "Each frame is two seconds long.\n"
       "The movie will describe what you should hear at the beginning of "
       "the frame.');");
		  
  frame = 0;

  pauseForNextTest(mo);
  runMultipleSoundsTest(mo, so, &frame);
  
  pauseForNextTest(mo);
  runNoMultipleSoundsTest(mo, so, &frame);

  pauseForNextTest(mo);
  runTrimmedSoundsTest(mo, so, &frame);

  pauseForNextTest(mo);
  runAttachedSoundsTest(mo, so, &frame);

  // TODO: test start(<sec_offset>) (+ with loop ?)

  endOfTests(mo);

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
