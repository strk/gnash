// image_filters.cpp	-- Original code by Dale Schumacher, public domain 1991

// See _Graphics Gems III_ "General Filtered Image Rescaling", Dale A. Schumacher

// Modifications by Thatcher Ulrich <tu@tulrich.com> 2002

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A series of image rescaling functions.  tulrich: Mostly I just
// converted from K&R C to C-like C++, changed the interfaces a bit,
// etc.

/* $Id: image_filters.cpp,v 1.18 2007/12/12 10:06:59 zoulunkai Exp $ */

#include "image.h"
#include "utility.h"
#include "container.h"
#include "tu_math.h"
#include <cstdio>
#include <cstring>
#include <cmath>
#include "log.h"

using namespace gnash;
using namespace std;

namespace {
// anonymous namespace to hold local stuff.


inline void* my_calloc(int count, int size)
{
    void *mem = (void *)new char[count * size];
    memset(mem, 0, count * size);
    return mem;
}

inline void	my_cfree(void* mem)
{
    delete [] (char*) mem;
}


void	get_row(boost::uint8_t* row, image::rgb* image, int x0, int xsize, int y)
// Copy RGB data from the specified row into the given buffer.
{
    y = iclamp(y, 0, image->height() - 1);
    int	x1 = x0 + xsize - 1;
    if (x1 >= image->width()) {
	// clip, then extend.
	int	extra_pixels = x1 - image->width() + 1;
	boost::uint8_t*	p = ((boost::uint8_t*) image->data()) + (y * image->pitch());
	memcpy(row, p + x0 * 3, (3 * (image->width() - x0)));
	// repeat last pixel
	p = p + (image->width() - 1) * 3;
	boost::uint8_t*	q = row + (image->width() - x0) * 3;
	while (extra_pixels > 0) {
	    *(q + 0) = *(p + 0);
	    *(q + 1) = *(p + 1);
	    *(q + 2) = *(p + 2);
	    q += 3;
	    extra_pixels--;
	}
    }
    else
	{
	    memcpy(row, ((boost::uint8_t*) image->data()) + (y * image->pitch()) + x0 * 3, (3 * xsize));
	}
}


void	get_row(boost::uint8_t* row, image::rgba* image, int x0, int xsize, int y)
// Copy RGBA data from the specified row into the given buffer.
{
    y = iclamp(y, 0, image->height() - 1);
    int	x1 = x0 + xsize - 1;
    if (x1 >= image->width()) {
	// clip, then extend.
	int	extra_pixels = x1 - image->width() + 1;
	boost::uint8_t*	p = ((boost::uint8_t*) image->data()) + (y * image->pitch());
	memcpy(row, p + x0 * 4, (4 * (image->width() - x0)));
	// repeat last pixel
	p = p + (image->width() - 1) * 4;
	boost::uint8_t*	q = row + (image->width() - x0) * 4;
	while (extra_pixels > 0) {
	    *(q + 0) = *(p + 0);
	    *(q + 1) = *(p + 1);
	    *(q + 2) = *(p + 2);
	    *(q + 3) = *(p + 3);
	    q += 4;
	    extra_pixels--;
	}
    }
    else
	{
	    memcpy(row, ((boost::uint8_t*) image->data()) + (y * image->pitch()) + x0 * 4, (4 * xsize));
	}
}


void	get_column(boost::uint8_t* column, image::rgb* image, int x)
// Copy RGB data from the specified column into the given buffer.
{

    if ((x < 0) || (x >= image->width())) {
	abort();
	x = iclamp(x, 0, image->width() - 1);
    }

    int d = image->pitch();
    boost::uint8_t* p = ((boost::uint8_t*) image->data()) + x * 3;
    for (int i = image->height(); i-- > 0; p += d) {
	*column++ = *p;
	*column++ = *(p + 1);
	*column++ = *(p + 2);
    }
}


void	get_column(boost::uint8_t* column, image::rgba* image, int x)
// Copy RGBA data from the specified column into the given buffer.
{
    if ((x < 0) || (x >= image->width())) {
	abort();
	x = iclamp(x, 0, image->width() - 1);
    }

    int d = image->pitch();
    boost::uint8_t* p = ((boost::uint8_t*) image->data()) + x * 4;
    for (int i = image->height(); i-- > 0; p += d) {
	*column++ = *p;
	*column++ = *(p + 1);
	*column++ = *(p + 2);
	*column++ = *(p + 3);
    }
}


void	put_pixel(image::rgb* image, int x, int y, float r, float g, float b)
// Clamp {r, g, b} to [0,255], and write pixel data to the given image
// at (x, y).
{
    static image::rgb*	im = NULL;
    static int		yy = -1;
    static boost::uint8_t*	p = NULL;

    if ((x < 0) || (x >= image->width()) || (y < 0) || (y >= image->height())) {
	abort();
	return;
    }
    if ((im != image) || (yy != y)) {
	im = image;
	yy = y;
	p = ((boost::uint8_t*) image->data()) + (y * image->pitch());
    }
    p[x * 3 + 0] = iclamp(frnd(r), 0, 255);
    p[x * 3 + 1] = iclamp(frnd(g), 0, 255);
    p[x * 3 + 2] = iclamp(frnd(b), 0, 255);
}


void	put_pixel(image::rgba* image, int x, int y, float r, float g, float b, float a)
// Clamp {r, g, b, a} to [0,255], and write pixel data to the given image
// at (x, y).
{
    static image::rgba*	im = NULL;
    static int		yy = -1;
    static boost::uint8_t*	p = NULL;

    if ((x < 0) || (x >= image->width()) || (y < 0) || (y >= image->height())) {
	abort();
	return;
    }
    if ((im != image) || (yy != y)) {
	im = image;
	yy = y;
	p = ((boost::uint8_t*) image->data()) + (y * image->pitch());
    }
    p[x * 4	+ 0] = iclamp(frnd(r), 0, 255);
    p[x * 4	+ 1] = iclamp(frnd(g), 0, 255);
    p[x * 4	+ 2] = iclamp(frnd(b), 0, 255);
    p[x * 4	+ 3] = iclamp(frnd(a), 0, 255);
}


/*
 *	filter function definitions
 */


// SOME_CUBIC

#define	cubic_filter_support		(1.0f)

float	cubic_filter(float t)
// Cubix approximation to the central hump of Sinc.
{
    /* f(t) = 2|t|^3 - 3|t|^2 + 1, -1 <= t <= 1 */
    if(t < 0.0f) t = -t;
    if(t < 1.0f) return((2.0f * t - 3.0f) * t * t + 1.0f);
    return(0.0f);
}


// BOX

#define	box_support		(0.5f)

float	box_filter(float t)
{
    if((t > -0.5) && (t <= 0.5)) return(1.0);
    return(0.0);
}


// TRIANGLE

#define	triangle_support	(1.0)

float	triangle_filter(float t)
{
    if(t < 0.0f) t = -t;
    if(t < 1.0f) return(1.0f - t);
    return(0.0f);
}


// BELL

#define	bell_support		(1.5)

float	bell_filter(float t)
/* box (*) box (*) box */
{
    if(t < 0) t = -t;
    if(t < 0.5f) return(0.75f - (t * t));
    if(t < 1.5f) {
	t = (t - 1.5f);
	return(0.5f * (t * t));
    }
    return(0.0f);
}


// B_SPLINE

#define	B_spline_support	(2.0f)

float	B_spline_filter(float t)
/* box (*) box (*) box (*) box */
{
    float	tt;

    if(t < 0.0f) t = -t;
    if(t < 1.0f) {
	tt = t * t;
	return((0.5f * tt * t) - tt + (2.0f / 3.0f));
    } else if (t < 2.0f) {
	t = 2.0f - t;
	return((1.0f / 6.0f) * (t * t * t));
    }
    return(0.0f);
}


// LANCZOS3

float	sinc(float x)
{
    x *= (float) M_PI;
    if (x != 0.0f) return(sinf(x) / x);
    return(1.0f);
}

#define	Lanczos3_support	(3.0f)

float	Lanczos3_filter(float t)
{
    if (t < 0.0f) t = -t;
    if (t < 3.0f) return(sinc(t) * sinc(t/3.0f));
    return(0.0f);
}


// MITCHELL

#define	Mitchell_support	(2.0f)

#define	B	(1.0f / 3.0f)
#define	C	(1.0f / 3.0f)

float	Mitchell_filter(float t)
{
    float tt = t * t;
    if (t < 0.0f) t = -t;
    if (t < 1.0f) {
	t = (((12.0f - 9.0f * B - 6.0f * C) * (t * tt))
	     + ((-18.0f + 12.0f * B + 6.0f * C) * tt)
	     + (6.0f - 2.0f * B));
	return(t / 6.0f);
    } else if(t < 2.0f) {
	t = (((-1.0f * B - 6.0f * C) * (t * tt))
	     + ((6.0f * B + 30.0f * C) * tt)
	     + ((-12.0f * B - 48.0f * C) * t)
	     + (8.0f * B + 24 * C));
	return(t / 6.0f);
    }
    return(0.0f);
}


struct CONTRIB {
    int	pixel;
    float	weight;

    CONTRIB()
	: pixel(0), weight(0.f)
	{
	}

    CONTRIB(int p, float w)
	: pixel(p), weight(w)
	{
	}
};


}	// end anonymous namespace


namespace image {


enum filter_type {
FILTER0 = 0,
    BOX = FILTER0,
    TRIANGLE,
    BELL,
    B_SPLINE,
    SOME_CUBIC,	// Cubic approximation of Sinc's hump (but no tails).
    LANCZOS3,
    MITCHELL,	// This one is alleged to be pretty nice.

    FILTER_COUNT
    };

struct filter_table {
    float	(*filter_function)(float);
    float	support;
} filter_table[] =
{
    { box_filter, box_support },
    { triangle_filter, triangle_support },
    { bell_filter, bell_support },
    { B_spline_filter, B_spline_support },
    { cubic_filter, cubic_filter_support },
    { Lanczos3_filter, Lanczos3_support },
    { Mitchell_filter, Mitchell_support },
};


// TODO: experiment with different filter functions.
filter_type	default_type = TRIANGLE;


void	resample(image::rgb* out, int out_x0, int out_y0, int out_x1, int out_y1,
		 image::rgb* in, float in_x0, float in_y0, float in_x1, float in_y1)
// Rescale the specified portion of the input image into the specified
// portion of the output image.  Coordinates are *inclusive*.
{
    GNASH_REPORT_FUNCTION;

    assert(out_x0 <= out_x1);
    assert(out_y0 <= out_y1);
    assert(out_x0 >= 0 && out_x0 < out->width());
    assert(out_x1 >= 0 && out_x1 < out->width());
    assert(out_y0 >= 0 && out_y0 < out->height());
    assert(out_y1 >= 0 && out_y1 < out->height());

    float	(*filter_function)(float);
    float	support;

    // Pick a filter function & support.
    assert(default_type >= FILTER0 && default_type < FILTER_COUNT);
    filter_function = filter_table[default_type].filter_function;
    support = filter_table[default_type].support;


    int i, k;			/* loop variables */
    unsigned int j;			/* loop variables */
    int n;				/* pixel number */
    float center; int left, right;	/* filter calculation variables */
    float width, fscale, weight;	/* filter calculation variables */
    boost::uint8_t*	raster;			/* a row or column of pixels */

    std::vector< std::vector<CONTRIB> >	contrib;

    int	out_width = out_x1 - out_x0 + 1;
    int	out_height = out_y1 - out_y0 + 1;
    assert(out_width > 0);
    assert(out_height > 0);

    float	in_width = in_x1 - in_x0;
    float	in_height = in_y1 - in_y0;
    assert(in_width > 0);
    assert(in_height > 0);

    int	in_window_w = int(ceilf(in_x1) - floorf(in_x0) + 1);
    int	in_window_h = int(ceilf(in_y1) - floorf(in_y0) + 1);

    /* create intermediate image to hold horizontal zoom */
    std::auto_ptr<image::rgb> tmp ( image::create_rgb(out_width, in_window_h) );
    float xscale = (float) (out_width - 1) / in_width;
    float yscale = (float) (out_height - 1) / in_height;

    // xxxx protect against division by 0
    if (yscale == 0) { yscale = 1.0f; }
    if (xscale == 0) { xscale = 1.0f; }

    /* pre-calculate filter contributions for a row */
    contrib.resize(tmp->width());
    if(xscale < 1.0f) {
	width = support / xscale;
	fscale = 1.0f / xscale;
	for (i = 0; i < tmp->width(); ++i) {
	    contrib[i].resize(0);

	    center = (float) i / xscale;
	    left = int(ceilf(center - width));
	    right = int(floorf(center + width));
	    for (k = left; k <= right; ++k) {
		weight = center - (float) k;
		weight = (*filter_function)(weight / fscale) / fscale;
		n = iclamp(k, 0, in_window_w - 1);
		contrib[i].push_back(CONTRIB(n, weight));
	    }
	}
    } else {
	for (i = 0; i < tmp->width(); ++i) {
	    contrib[i].resize(0);
	    center = (float) i / xscale;
	    left = int(ceilf(center - support));
	    right = int(floorf(center + support));
	    for(k = left; k <= right; ++k) {
		weight = center - (float) k;
		weight = (*filter_function)(weight);
		n = iclamp(k, 0, in_window_w - 1);
		contrib[i].push_back(CONTRIB(n, weight));
	    }
	}
    }

    /* apply filter to zoom horizontally from src to tmp */
    raster = (boost::uint8_t*) my_calloc(in_window_w, 3);
    for (k = 0; k < tmp->height(); ++k) {
	get_row(raster, in, int(floorf(in_x0)), in_window_w, k);
	for (i = 0; i < tmp->width(); ++i) {
	    float	red = 0.0f;
	    float	green = 0.0f;
	    float	blue = 0.0f;
	    for(j = 0; j < contrib[i].size(); ++j) {
		int	pixel = contrib[i][j].pixel;
		red	+= raster[pixel * 3 + 0] * contrib[i][j].weight;
		green	+= raster[pixel * 3 + 1] * contrib[i][j].weight;
		blue	+= raster[pixel * 3 + 2] * contrib[i][j].weight;
	    }
	    put_pixel(tmp.get(), i, k, red, green, blue);
	}
    }
    my_cfree(raster);

    contrib.resize(out_height);

    if (yscale < 1.0f) {
	width = support / yscale;
	fscale = 1.0f / yscale;
	for (i = 0; i < out_height; ++i) {
	    contrib[i].resize(0);

	    center = (float) i / yscale;
	    left = int(ceilf(center - width));
	    right = int(floorf(center + width));
	    for (k = left; k <= right; ++k) {
		weight = center - (float) k;
		weight = (*filter_function)(weight / fscale) / fscale;
		n = iclamp(k, 0, tmp->height() - 1);
		contrib[i].push_back(CONTRIB(n, weight));
	    }
	}
    } else {
	for (i = 0; i < out_height; ++i) {
	    contrib[i].resize(0);
	    center = (float) i / yscale;
	    left = int(ceilf(center - support));
	    right = int(floorf(center + support));
	    for(k = left; k <= right; ++k) {
		weight = center - (float) k;
		weight = (*filter_function)(weight);
		n = iclamp(k, 0, tmp->height() - 1);
		contrib[i].push_back(CONTRIB(n, weight));
	    }
	}
    }

    /* apply filter to zoom vertically from tmp to dst */
    raster = (boost::uint8_t*) my_calloc(tmp->height(), 3);
    for (k = 0; k < tmp->width(); ++k) {
	get_column(raster, tmp.get(), k);
	for (i = 0; i < out_height; ++i) {
	    float	red = 0.0f;
	    float	green = 0.0f;
	    float	blue = 0.0f;
	    for (j = 0; j < contrib[i].size(); ++j) {
		int	pixel = contrib[i][j].pixel;
		red	+= raster[pixel * 3 + 0] * contrib[i][j].weight;
		green	+= raster[pixel * 3 + 1] * contrib[i][j].weight;
		blue	+= raster[pixel * 3 + 2] * contrib[i][j].weight;
	    }
	    put_pixel(out, k + out_x0, i + out_y0, red, green, blue);
	}
    }
    my_cfree(raster);

    contrib.resize(0);

}


void	resample(image::rgba* out, int out_x0, int out_y0, int out_x1, int out_y1,
		 image::rgba* in, float in_x0, float in_y0, float in_x1, float in_y1)
// Rescale the specified portion of the input image into the specified
// portion of the output image.  Coordinates are *inclusive*.
//
// Same as above, but with an alpha channel.
{
    GNASH_REPORT_FUNCTION;
    assert(out_x0 <= out_x1);
    assert(out_y0 <= out_y1);
    assert(out_x0 >= 0 && out_x0 < out->width());
    assert(out_x1 >= 0 && out_x1 < out->width());
    assert(out_y0 >= 0 && out_y0 < out->height());
    assert(out_y1 >= 0 && out_y1 < out->height());

    float	(*filter_function)(float);
    float	support;

    // Pick a filter function & support.
    assert(default_type >= FILTER0 && default_type < FILTER_COUNT);
    filter_function = filter_table[default_type].filter_function;
    support = filter_table[default_type].support;


    float	xscale, yscale;		/* zoom scale factors */
    int i, k;			/* loop variables */
    unsigned int j;			/* loop variables */
    int n;				/* pixel number */
    float center; int left, right;	/* filter calculation variables */
    float width, fscale, weight;	/* filter calculation variables */
    boost::uint8_t*	raster;			/* a row or column of pixels */

    std::vector< std::vector<CONTRIB> >	contrib;

    int	out_width = out_x1 - out_x0 + 1;
    int	out_height = out_y1 - out_y0 + 1;
    assert(out_width > 0);
    assert(out_height > 0);

    float	in_width = in_x1 - in_x0;
    float	in_height = in_y1 - in_y0;
    assert(in_width > 0);
    assert(in_height > 0);

    int	in_window_w = int(ceilf(in_x1) - floorf(in_x0) + 1);
    int	in_window_h = int(ceilf(in_y1) - floorf(in_y0) + 1);

    /* create intermediate image to hold horizontal zoom */
    std::auto_ptr<image::rgba>	tmp( image::create_rgba(out_width, in_window_h) );
    xscale = (float) (out_width - 1) / in_width;
    yscale = (float) (out_height - 1) / in_height;

    // xxxx protect against division by 0
    if (yscale == 0) { yscale = 1.0f; }
    if (xscale == 0) { xscale = 1.0f; }

    /* pre-calculate filter contributions for a row */
    contrib.resize(tmp->width());
    if(xscale < 1.0f) {
	width = support / xscale;
	fscale = 1.0f / xscale;
	for (i = 0; i < tmp->width(); ++i) {
	    contrib[i].resize(0);

	    center = (float) i / xscale;
	    left = int(ceilf(center - width));
	    right = int(floorf(center + width));
	    for (k = left; k <= right; ++k) {
		weight = center - (float) k;
		weight = (*filter_function)(weight / fscale) / fscale;
		n = iclamp(k, 0, in_window_w - 1);
		contrib[i].push_back(CONTRIB(n, weight));
	    }
	}
    } else {
	for (i = 0; i < tmp->width(); ++i) {
	    contrib[i].resize(0);
	    center = (float) i / xscale;
	    left = int(ceilf(center - support));
	    right = int(floorf(center + support));
	    for(k = left; k <= right; ++k) {
		weight = center - (float) k;
		weight = (*filter_function)(weight);
		n = iclamp(k, 0, in_window_w - 1);
		contrib[i].push_back(CONTRIB(n, weight));
	    }
	}
    }

    /* apply filter to zoom horizontally from src to tmp */
    raster = (boost::uint8_t*) my_calloc(in_window_w, 4);
    for (k = 0; k < tmp->height(); ++k) {
	get_row(raster, in, int(floorf(in_x0)), in_window_w, k);
	for (i = 0; i < tmp->width(); ++i) {
	    float	red = 0.0f;
	    float	green = 0.0f;
	    float	blue = 0.0f;
	    float	alpha = 0.0f;
	    for(j = 0; j < contrib[i].size(); ++j) {
		int	pixel = contrib[i][j].pixel;
		red	+= raster[pixel * 4 + 0] * contrib[i][j].weight;
		green	+= raster[pixel * 4 + 1] * contrib[i][j].weight;
		blue	+= raster[pixel * 4 + 2] * contrib[i][j].weight;
		alpha	+= raster[pixel * 4 + 3] * contrib[i][j].weight;
	    }
	    put_pixel(tmp.get(), i, k, red, green, blue, alpha);
	}
    }
    my_cfree(raster);

    contrib.resize(out_height);

    if (yscale < 1.0f) {
	width = support / yscale;
	fscale = 1.0f / yscale;
	for (i = 0; i < out_height; ++i) {
	    contrib[i].resize(0);

	    center = (float) i / yscale;
	    left = int(ceilf(center - width));
	    right = int(floorf(center + width));
	    for (k = left; k <= right; ++k) {
		weight = center - (float) k;
		weight = (*filter_function)(weight / fscale) / fscale;
		n = iclamp(k, 0, tmp->height() - 1);
		contrib[i].push_back(CONTRIB(n, weight));
	    }
	}
    } else {
	for (i = 0; i < out_height; ++i) {
	    contrib[i].resize(0);
	    center = (float) i / yscale;
	    left = int(ceilf(center - support));
	    right = int(floorf(center + support));
	    for(k = left; k <= right; ++k) {
		weight = center - (float) k;
		weight = (*filter_function)(weight);
		n = iclamp(k, 0, tmp->height() - 1);
		contrib[i].push_back(CONTRIB(n, weight));
	    }
	}
    }

    /* apply filter to zoom vertically from tmp to dst */
    raster = (boost::uint8_t*) my_calloc(tmp->height(), 4);
    for (k = 0; k < tmp->width(); ++k) {
	get_column(raster, tmp.get(), k);
	for (i = 0; i < out_height; ++i) {
	    float	red = 0.0f;
	    float	green = 0.0f;
	    float	blue = 0.0f;
	    float	alpha = 0.0f;
	    for (j = 0; j < contrib[i].size(); ++j) {
		int	pixel = contrib[i][j].pixel;
		red	+= raster[pixel * 4 + 0] * contrib[i][j].weight;
		green	+= raster[pixel * 4 + 1] * contrib[i][j].weight;
		blue	+= raster[pixel * 4 + 2] * contrib[i][j].weight;
		alpha	+= raster[pixel * 4 + 3] * contrib[i][j].weight;
	    }
	    put_pixel(out, k + out_x0, i + out_y0, red, green, blue, alpha);
	}
    }
    my_cfree(raster);

    contrib.resize(0);

}

} // end namespace image



// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

