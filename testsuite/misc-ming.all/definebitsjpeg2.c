#include <stdio.h>
#include <ming.h>

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "definebitsjpeg2.swf"

#define URL_LYNCH "lynch.jpg"

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

	SWFMovie_add(mo, (SWFBlock)mc_lynch);


	puts("Saving " OUTPUT_FILENAME );

	SWFMovie_nextFrame(mo); /* showFrame */

	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
