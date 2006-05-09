#include <stdio.h>
#include <ming.h>

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "definebitsjpeg2.swf"

#define URL_LYNCH "lynch.jpg"

int
main(int argc, char **argv)
{
	SWFMovie mo;
	const char *jpeg_filename="lynch.jpg";
	FILE *jpeg_fd;
	SWFJpegBitmap jpeg_bm;
	SWFShape jpeg_sh;
	SWFMovieClip jpeg_mc;

	/*********************************************
	 *
	 * Initialization
	 *
	 *********************************************/

	if ( argc > 1 ) jpeg_filename=argv[1];
	else
	{
		fprintf(stderr, "Usage: %s <jpegfile>\n", argv[0]);
		return 1;
	}

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

	jpeg_fd = fopen(jpeg_filename, "r");
	if ( ! jpeg_fd ) {
		perror(jpeg_filename);
		return 1;
	}
	jpeg_bm = newSWFJpegBitmap(jpeg_fd);
	jpeg_sh = newSWFShapeFromBitmap((SWFBitmap)jpeg_bm, SWFFILL_CLIPPED_BITMAP);
	jpeg_mc = newSWFMovieClip();
	SWFMovieClip_add(jpeg_mc, (SWFBlock)jpeg_sh);
	SWFMovieClip_nextFrame(jpeg_mc); /* showFrame */

	SWFMovie_add(mo, (SWFBlock)jpeg_mc);


	puts("Saving " OUTPUT_FILENAME );

	SWFMovie_nextFrame(mo); /* showFrame */

	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
