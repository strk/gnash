#include <stdio.h>
#include <ming.h>

#define OUTPUT_VERSION 7
#define OUTPUT_FILENAME "moviecliploader_test_ming.swf"

#define URL_LYNCH "file://lynch.jpg"
#define URL_GREEN "file://green.jpg"
#define URL_OFFSPRING "file://offspring.jpg"

#define URL_LYNCH "lynch.jpg"
#define URL_GREEN "green.jpg"
#define URL_OFFSPRING "offspring.jpg"

#define URL_LYNCH "text_sizes.swf"
#define URL_GREEN "text_formatting.swf"
#define URL_OFFSPRING "visible_and_transparency.swf"



main()
{
	SWFMovie mo;
	const char *file_lynch="lynch.jpg";
	const char *file_green="green.jpg";
	const char *file_offspring="offspring.jpg";
	FILE *fd_lynch, *fd_green, *fd_offspring;
	SWFJpegBitmap bm_lynch, bm_green, bm_offspring;
	SWFShape sh_lynch, sh_green, sh_offspring, sh_coverart;
	SWFMovieClip mc_lynch, mc_green, mc_offspring, mc_coverart;
	SWFFillStyle fstyle;
	SWFDisplayItem it;
	SWFAction ac;

	/*********************************************
	 *
	 * Initialization
	 *
	 *********************************************/

	puts("Setting things up");

	Ming_init();
        Ming_useSWFVersion (OUTPUT_VERSION);
	Ming_setScale(1.0); /* so we talk twips */
 
	mo = newSWFMovie();
        SWFMovie_setDimension (mo, 11000, 8000); 
        SWFMovie_setRate (mo, 12.0); 
        SWFMovie_setBackground (mo, 255, 255, 255); 

	/*****************************************************
	 *
	 * MovieClipLoader class
	 *
	 *****************************************************/

	puts("Compiling MovieClipLoader actionscript");

	/* Action for first frame */
	ac = compileSWFActionCode
(" \
stop();  \
CoverArtLoader = new MovieClipLoader(); \
");
	SWFMovie_add(mo, (SWFBlock)ac);


	/*****************************************************
	 *
	 * Add the LYNCH  clip
	 *
	 *****************************************************/

	puts("Adding lynch");

	fd_lynch = fopen(file_lynch, "r");
	if ( ! fd_lynch ) perror(file_lynch);
	bm_lynch = newSWFJpegBitmap(fd_lynch);
	sh_lynch = newSWFShapeFromBitmap((SWFBitmap)bm_lynch, SWFFILL_CLIPPED_BITMAP);
	mc_lynch = newSWFMovieClip();
	SWFMovieClip_add(mc_lynch, (SWFBlock)sh_lynch);
	SWFMovieClip_nextFrame(mc_lynch); /* showFrame */
	it = SWFMovie_add(mo, (SWFBlock)mc_lynch);
	SWFDisplayItem_setName(it, "lynch");
	SWFDisplayItem_moveTo(it, 200, 4419);
	/* "Click" handler */
	ac = compileSWFActionCode
(" \
lynch.onPress = function () { \
	_root.CoverArtLoader.loadClip('" URL_LYNCH "', coverart); \
}; \
");
	SWFMovie_add(mo, (SWFBlock)ac);

	/*****************************************************
	 *
	 * Add the GREEN  clip
	 *
	 *****************************************************/

	puts("Adding green");

	fd_green = fopen(file_green, "r");
	if ( ! fd_green ) perror(file_green);
	bm_green = newSWFJpegBitmap(fd_green);
	sh_green = newSWFShapeFromBitmap((SWFBitmap)bm_green, SWFFILL_CLIPPED_BITMAP);
	mc_green = newSWFMovieClip();
	SWFMovieClip_add(mc_green, (SWFBlock)sh_green);
	SWFMovieClip_nextFrame(mc_green); /* showFrame */
	it = SWFMovie_add(mo, (SWFBlock)mc_green);
	SWFDisplayItem_setName(it, "green");
	SWFDisplayItem_moveTo(it, 3800, 4419);
	/* "Click" handler */
	ac = compileSWFActionCode
(" \
green.onPress = function () { \
	_root.CoverArtLoader.loadClip('" URL_GREEN "', coverart); \
}; \
");
	SWFMovie_add(mo, (SWFBlock)ac);
	

	/*****************************************************
	 *
	 * Add the OFFSPRING  clip
	 *
	 *****************************************************/

	puts("Adding offspring");

	fd_offspring = fopen(file_offspring, "r");
	if ( ! fd_offspring ) perror(file_offspring);
	bm_offspring = newSWFJpegBitmap(fd_offspring);
	sh_offspring = newSWFShapeFromBitmap((SWFBitmap)bm_offspring, SWFFILL_CLIPPED_BITMAP);
	mc_offspring = newSWFMovieClip();
	SWFMovieClip_add(mc_offspring, (SWFBlock)sh_offspring);
	SWFMovieClip_nextFrame(mc_offspring); /* showFrame */
	it = SWFMovie_add(mo, (SWFBlock)mc_offspring);
	SWFDisplayItem_setName(it, "offspring"); 
	SWFDisplayItem_moveTo(it, 7400, 4419);
	/* "Click" handler */
	ac = compileSWFActionCode
(" \
offspring.onPress = function () { \
	_root.CoverArtLoader.loadClip('" URL_OFFSPRING "', coverart); \
}; \
");
	SWFMovie_add(mo, (SWFBlock)ac);
	

	/*****************************************************
	 *
	 * Add the coverart clip
	 *
	 *****************************************************/

	puts("Adding coverart");

	sh_coverart = newSWFShape();
	fstyle = SWFShape_addSolidFillStyle(sh_coverart, 0,0,0,255);
	SWFShape_setRightFillStyle(sh_coverart, fstyle);
	SWFShape_movePenTo(sh_coverart, 3400, 3400);
	SWFShape_drawLine(sh_coverart, -3400, 0);
	SWFShape_drawLine(sh_coverart, 0, -3400);
	SWFShape_drawLine(sh_coverart, 3400, 0);
	SWFShape_drawLine(sh_coverart, 0, 3400);

	mc_coverart = newSWFMovieClip();
	SWFMovieClip_add(mc_coverart, (SWFBlock)sh_coverart);
	SWFMovieClip_nextFrame(mc_coverart); /* showFrame */
	it = SWFMovie_add(mo, (SWFBlock)mc_coverart);
	SWFDisplayItem_setName(it, "coverart"); 
	SWFDisplayItem_moveTo(it, 3800, 500);

	/*****************************************************
	 *
	 * Output movie
	 *
	 *****************************************************/

	puts("Saving " OUTPUT_FILENAME );

	SWFMovie_nextFrame(mo); /* showFrame */

	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
