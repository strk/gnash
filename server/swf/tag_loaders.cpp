// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <zlib.h>

#include "tu_file.h"
#include "utility.h"
#include "action.h"
#include "button.h"
#include "impl.h"
#include "font.h"
#include "fontlib.h"
#include "log.h"
#include "morph2.h"
#include "render.h"
#include "shape.h"
#include "stream.h"
#include "styles.h"
#include "dlist.h"
#include "timers.h"
#include "image.h"
#include "jpeg.h"
#include "zlib_adapter.h"
//#include "Sprite.h"
#include "sprite_definition.h"
#include "Function.h"
#include "movie_def_impl.h"
#include "swf.h"
#include "swf/TagLoadersTable.h"
#include "generic_character.h"
#include "text.h" // for text_character_def
#include "edit_text_character_def.h"
#include "execute_tag.h" // for do_action inheritance (DOACTION tag loader)
#include "URL.h"
#include "GnashException.h"

namespace gnash {
// @@ TODO get rid of this; make it the normal mode.
extern bool s_no_recurse_while_loading;
}

namespace gnash {
namespace SWF {
namespace tag_loaders {

//
// Some tag implementations
//


/// Thin wrapper around action_buffer.
struct do_action : public execute_tag
{
	action_buffer m_buf;

	void read(stream* in)
	{
	    m_buf.read(in);
	}

	virtual void execute(movie* m)
	{
	    m->add_action_buffer(&m_buf);
	}

	// Don't override because actions should not be replayed when
	// seeking the movie.
	//void	execute_state(movie* m) {}

	// Tell the caller that we are an action tag.
	virtual bool is_action_tag() const
	{
	    return true;
	}
};

//
// Tag loaders
//


// Silently ignore the contents of this tag.
void	null_loader(stream* in, tag_type tag, movie_definition* m)
{
}

// Label the current frame of m with the name from the stream.
void
frame_label_loader(stream* in, tag_type tag, movie_definition* m)
{
    char*	n = in->read_string();
    m->add_frame_name(n);
    delete [] n;
}

/// SWF Tag SetBackgroundColor (9)
struct set_background_color : public execute_tag
{
    rgba	m_color;

    void	execute(movie* m)
	{
	    float	current_alpha = m->get_background_alpha();
	    m_color.m_a = frnd(current_alpha * 255.0f);
	    m->set_background_color(m_color);
	}

    void	execute_state(movie* m)
	{
	    execute(m);
	}

    void	read(stream* in)
	{
	    m_color.read_rgb(in);

	    IF_VERBOSE_PARSE(log_msg("  set_background_color: (%d %d %d)\n",
				     m_color.m_r, m_color.m_g, m_color.m_b));
	}
};


/// SWF Tag SetBackgroundColor (9)
void
set_background_color_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == SWF::SETBACKGROUNDCOLOR); // 9
    assert(m);

    set_background_color*	t = new set_background_color;
    t->read(in);

    m->add_execute_tag(t);
}

// Load JPEG compression tables that can be used to load
// images further along in the stream.
void
jpeg_tables_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == SWF::JPEGTABLES);

#if TU_CONFIG_LINK_TO_JPEGLIB
    jpeg::input*	j_in = jpeg::input::create_swf_jpeg2_header_only(in->get_underlying_stream());
    assert(j_in);

    m->set_jpeg_loader(j_in);
#endif // TU_CONFIG_LINK_TO_JPEGLIB
}


// A JPEG image without included tables; those should be in an
// existing jpeg::input object stored in the movie.
void
define_bits_jpeg_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == SWF::DEFINEBITS); // 6

    uint16_t	character_id = in->read_u16();

    //
    // Read the image data.
    //
    bitmap_info*	bi = NULL;

    if (m->get_create_bitmaps() == DO_LOAD_BITMAPS)
	{
#if TU_CONFIG_LINK_TO_JPEGLIB
	    jpeg::input*	j_in = m->get_jpeg_loader();
	    assert(j_in);
	    j_in->discard_partial_buffer();

	    image::rgb*	im = image::read_swf_jpeg2_with_tables(j_in);
	    bi = render::create_bitmap_info_rgb(im);
	    delete im;
#else
	    log_error("gnash is not linked to jpeglib -- can't load jpeg image data!\n");
	    bi = render::create_bitmap_info_empty();
#endif
	}
    else
	{
	    bi = render::create_bitmap_info_empty();
	}

    assert(bi->get_ref_count() == 0);

    bitmap_character*	ch = new bitmap_character(bi);

    m->add_bitmap_character(character_id, ch);
}


void
define_bits_jpeg2_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == SWF::DEFINEBITSJPEG2); // 21
		
    uint16_t	character_id = in->read_u16();

    IF_VERBOSE_PARSE(log_msg("  define_bits_jpeg2_loader: charid = %d pos = 0x%x\n", character_id, in->get_position()));

    //
    // Read the image data.
    //
		
    bitmap_info*	bi = NULL;

    if (m->get_create_bitmaps() == DO_LOAD_BITMAPS)
	{
#if TU_CONFIG_LINK_TO_JPEGLIB
	    image::rgb* im = image::read_jpeg(in->get_underlying_stream());
	    bi = render::create_bitmap_info_rgb(im);
	    delete im;
#else
	    log_error("gnash is not linked to jpeglib -- can't load jpeg image data!\n");
	    bi = render::create_bitmap_info_empty();
#endif
	}
    else
	{
	    bi = render::create_bitmap_info_empty();
	}

    assert(bi->get_ref_count() == 0);

    bitmap_character*	ch = new bitmap_character(bi);

    m->add_bitmap_character(character_id, ch);
}


#if TU_CONFIG_LINK_TO_ZLIB
void	inflate_wrapper(tu_file* in, void* buffer, int buffer_bytes)
    // Wrapper function -- uses Zlib to uncompress in_bytes worth
    // of data from the input file into buffer_bytes worth of data
    // into *buffer.
{
    assert(in);
    assert(buffer);
    assert(buffer_bytes > 0);

    int err;
    z_stream d_stream; /* decompression stream */

    d_stream.zalloc = (alloc_func)0;
    d_stream.zfree = (free_func)0;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in  = 0;
    d_stream.avail_in = 0;

    d_stream.next_out = (Byte*) buffer;
    d_stream.avail_out = (uInt) buffer_bytes;

    err = inflateInit(&d_stream);
    if (err != Z_OK) {
	log_error("error: inflate_wrapper() inflateInit() returned %d\n", err);
	return;
    }

    uint8_t	buf[1];

    for (;;) {
	// Fill a one-byte (!) buffer.
	buf[0] = in->read_byte();
	d_stream.next_in = &buf[0];
	d_stream.avail_in = 1;

	err = inflate(&d_stream, Z_SYNC_FLUSH);
	if (err == Z_STREAM_END) break;
	if (err != Z_OK)
	    {
		log_error("error: inflate_wrapper() inflate() returned %d\n", err);
	    }
    }

    err = inflateEnd(&d_stream);
    if (err != Z_OK)
	{
	    log_error("error: inflate_wrapper() inflateEnd() return %d\n", err);
	}
}
#endif // TU_CONFIG_LINK_TO_ZLIB


// loads a define_bits_jpeg3 tag. This is a jpeg file with an alpha
// channel using zlib compression.
void
define_bits_jpeg3_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == SWF::DEFINEBITSJPEG3); // 35

    uint16_t	character_id = in->read_u16();

    IF_VERBOSE_PARSE(log_msg("  define_bits_jpeg3_loader: charid = %d pos = 0x%x\n", character_id, in->get_position()));

    uint32_t	jpeg_size = in->read_u32();
    uint32_t	alpha_position = in->get_position() + jpeg_size;

    bitmap_info*	bi = NULL;

    if (m->get_create_bitmaps() == DO_LOAD_BITMAPS)
	{
#if TU_CONFIG_LINK_TO_JPEGLIB == 0 || TU_CONFIG_LINK_TO_ZLIB == 0
	    log_error("gnash is not linked to jpeglib/zlib -- can't load jpeg/zipped image data!\n");
	    bi = render::create_bitmap_info_empty();
#else
	    //
	    // Read the image data.
	    //
		
	    // Read rgb data.
	    image::rgba*	im = image::read_swf_jpeg3(in->get_underlying_stream());

	    // Read alpha channel.
	    in->set_position(alpha_position);

	    int	buffer_bytes = im->m_width * im->m_height;
	    uint8_t*	buffer = new uint8_t[buffer_bytes];

	    inflate_wrapper(in->get_underlying_stream(), buffer, buffer_bytes);

	    for (int i = 0; i < buffer_bytes; i++)
		{
		    im->m_data[4*i+3] = buffer[i];
		}

	    delete [] buffer;

	    bi = render::create_bitmap_info_rgba(im);

	    delete im;
#endif

	}
    else
	{
	    bi = render::create_bitmap_info_empty();
	}

    // Create bitmap character.
    bitmap_character*	ch = new bitmap_character(bi);

    m->add_bitmap_character(character_id, ch);
}


void
define_bits_lossless_2_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == 20 || tag == 36);

    uint16_t	character_id = in->read_u16();
    uint8_t	bitmap_format = in->read_u8();	// 3 == 8 bit, 4 == 16 bit, 5 == 32 bit
    uint16_t	width = in->read_u16();
    uint16_t	height = in->read_u16();

    IF_VERBOSE_PARSE(log_msg("  defbitslossless2: tag = %d, id = %d, fmt = %d, w = %d, h = %d\n",
			     tag,
			     character_id,
			     bitmap_format,
			     width,
			     height));

    bitmap_info*	bi = NULL;
    if (m->get_create_bitmaps() == DO_LOAD_BITMAPS)
	{
#if TU_CONFIG_LINK_TO_ZLIB == 0
	    log_error("gnash is not linked to zlib -- can't load zipped image data!\n");
	    return;
#else
	    if (tag == 20)
		{
		    // RGB image data.
		    image::rgb*	image = image::create_rgb(width, height);

		    if (bitmap_format == 3)
			{
			    // 8-bit data, preceded by a palette.

			    const int	bytes_per_pixel = 1;
			    int	color_table_size = in->read_u8();
			    color_table_size += 1;	// !! SWF stores one less than the actual size

			    int	pitch = (width * bytes_per_pixel + 3) & ~3;

			    int	buffer_bytes = color_table_size * 3 + pitch * height;
			    uint8_t*	buffer = new uint8_t[buffer_bytes];

			    inflate_wrapper(in->get_underlying_stream(), buffer, buffer_bytes);
			    assert(in->get_position() <= in->get_tag_end_position());

			    uint8_t*	color_table = buffer;

			    for (int j = 0; j < height; j++)
				{
				    uint8_t*	image_in_row = buffer + color_table_size * 3 + j * pitch;
				    uint8_t*	image_out_row = image::scanline(image, j);
				    for (int i = 0; i < width; i++)
					{
					    uint8_t	pixel = image_in_row[i * bytes_per_pixel];
					    image_out_row[i * 3 + 0] = color_table[pixel * 3 + 0];
					    image_out_row[i * 3 + 1] = color_table[pixel * 3 + 1];
					    image_out_row[i * 3 + 2] = color_table[pixel * 3 + 2];
					}
				}

			    delete [] buffer;
			}
		    else if (bitmap_format == 4)
			{
			    // 16 bits / pixel
			    const int	bytes_per_pixel = 2;
			    int	pitch = (width * bytes_per_pixel + 3) & ~3;

			    int	buffer_bytes = pitch * height;
			    uint8_t*	buffer = new uint8_t[buffer_bytes];

			    inflate_wrapper(in->get_underlying_stream(), buffer, buffer_bytes);
			    assert(in->get_position() <= in->get_tag_end_position());
			
			    for (int j = 0; j < height; j++)
				{
				    uint8_t*	image_in_row = buffer + j * pitch;
				    uint8_t*	image_out_row = image::scanline(image, j);
				    for (int i = 0; i < width; i++)
					{
					    uint16_t	pixel = image_in_row[i * 2] | (image_in_row[i * 2 + 1] << 8);
					
					    // @@ How is the data packed???  I'm just guessing here that it's 565!
					    image_out_row[i * 3 + 0] = (pixel >> 8) & 0xF8;	// red
					    image_out_row[i * 3 + 1] = (pixel >> 3) & 0xFC;	// green
					    image_out_row[i * 3 + 2] = (pixel << 3) & 0xF8;	// blue
					}
				}
			
			    delete [] buffer;
			}
		    else if (bitmap_format == 5)
			{
			    // 32 bits / pixel, input is ARGB format (???)
			    const int	bytes_per_pixel = 4;
			    int	pitch = width * bytes_per_pixel;

			    int	buffer_bytes = pitch * height;
			    uint8_t*	buffer = new uint8_t[buffer_bytes];

			    inflate_wrapper(in->get_underlying_stream(), buffer, buffer_bytes);
			    assert(in->get_position() <= in->get_tag_end_position());
			
			    // Need to re-arrange ARGB into RGB.
			    for (int j = 0; j < height; j++)
				{
				    uint8_t*	image_in_row = buffer + j * pitch;
				    uint8_t*	image_out_row = image::scanline(image, j);
				    for (int i = 0; i < width; i++)
					{
					    uint8_t	a = image_in_row[i * 4 + 0];
					    uint8_t	r = image_in_row[i * 4 + 1];
					    uint8_t	g = image_in_row[i * 4 + 2];
					    uint8_t	b = image_in_row[i * 4 + 3];
					    image_out_row[i * 3 + 0] = r;
					    image_out_row[i * 3 + 1] = g;
					    image_out_row[i * 3 + 2] = b;
					    a = a;	// Inhibit warning.
					}
				}

			    delete [] buffer;
			}

//				bitmap_character*	ch = new bitmap_character(image);
		    bi = render::create_bitmap_info_rgb(image);
		    delete image;

// 				// add image to movie, under character id.
// 				m->add_bitmap_character(character_id, ch);
		}
	    else
		{
		    // RGBA image data.
		    assert(tag == 36);

		    image::rgba*	image = image::create_rgba(width, height);

		    if (bitmap_format == 3)
			{
			    // 8-bit data, preceded by a palette.

			    const int	bytes_per_pixel = 1;
			    int	color_table_size = in->read_u8();
			    color_table_size += 1;	// !! SWF stores one less than the actual size

			    int	pitch = (width * bytes_per_pixel + 3) & ~3;

			    int	buffer_bytes = color_table_size * 4 + pitch * height;
			    uint8_t*	buffer = new uint8_t[buffer_bytes];

			    inflate_wrapper(in->get_underlying_stream(), buffer, buffer_bytes);
			    assert(in->get_position() <= in->get_tag_end_position());

			    uint8_t*	color_table = buffer;

			    for (int j = 0; j < height; j++)
				{
				    uint8_t*	image_in_row = buffer + color_table_size * 4 + j * pitch;
				    uint8_t*	image_out_row = image::scanline(image, j);
				    for (int i = 0; i < width; i++)
					{
					    uint8_t	pixel = image_in_row[i * bytes_per_pixel];
					    image_out_row[i * 4 + 0] = color_table[pixel * 4 + 0];
					    image_out_row[i * 4 + 1] = color_table[pixel * 4 + 1];
					    image_out_row[i * 4 + 2] = color_table[pixel * 4 + 2];
					    image_out_row[i * 4 + 3] = color_table[pixel * 4 + 3];
					}
				}

			    delete [] buffer;
			}
		    else if (bitmap_format == 4)
			{
			    // 16 bits / pixel
			    const int	bytes_per_pixel = 2;
			    int	pitch = (width * bytes_per_pixel + 3) & ~3;

			    int	buffer_bytes = pitch * height;
			    uint8_t*	buffer = new uint8_t[buffer_bytes];

			    inflate_wrapper(in->get_underlying_stream(), buffer, buffer_bytes);
			    assert(in->get_position() <= in->get_tag_end_position());
			
			    for (int j = 0; j < height; j++)
				{
				    uint8_t*	image_in_row = buffer + j * pitch;
				    uint8_t*	image_out_row = image::scanline(image, j);
				    for (int i = 0; i < width; i++)
					{
					    uint16_t	pixel = image_in_row[i * 2] | (image_in_row[i * 2 + 1] << 8);
					
					    // @@ How is the data packed???  I'm just guessing here that it's 565!
					    image_out_row[i * 4 + 0] = 255;			// alpha
					    image_out_row[i * 4 + 1] = (pixel >> 8) & 0xF8;	// red
					    image_out_row[i * 4 + 2] = (pixel >> 3) & 0xFC;	// green
					    image_out_row[i * 4 + 3] = (pixel << 3) & 0xF8;	// blue
					}
				}
			
			    delete [] buffer;
			}
		    else if (bitmap_format == 5)
			{
			    // 32 bits / pixel, input is ARGB format

			    inflate_wrapper(in->get_underlying_stream(), image->m_data, width * height * 4);
			    assert(in->get_position() <= in->get_tag_end_position());
			
			    // Need to re-arrange ARGB into RGBA.
			    for (int j = 0; j < height; j++)
				{
				    uint8_t*	image_row = image::scanline(image, j);
				    for (int i = 0; i < width; i++)
					{
					    uint8_t	a = image_row[i * 4 + 0];
					    uint8_t	r = image_row[i * 4 + 1];
					    uint8_t	g = image_row[i * 4 + 2];
					    uint8_t	b = image_row[i * 4 + 3];
					    image_row[i * 4 + 0] = r;
					    image_row[i * 4 + 1] = g;
					    image_row[i * 4 + 2] = b;
					    image_row[i * 4 + 3] = a;
					}
				}
			}

		    bi = render::create_bitmap_info_rgba(image);
//				bitmap_character*	ch = new bitmap_character(image);
		    delete image;

//	 			// add image to movie, under character id.
//	 			m->add_bitmap_character(character_id, ch);
		}
#endif // TU_CONFIG_LINK_TO_ZLIB
	}
    else
	{
	    bi = render::create_bitmap_info_empty();
	}
    assert(bi->get_ref_count() == 0);

    bitmap_character*	ch = new bitmap_character(bi);

    // add image to movie, under character id.
    m->add_bitmap_character(character_id, ch);
}

// This is like null_loader except it prints a message to nag us to fix it.
void
fixme_loader(stream* in, tag_type tag, movie_definition* m)
{
    IF_VERBOSE_PARSE(log_msg("  FIXME: tagtype = %d\n", tag));
}

void define_shape_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == SWF::DEFINESHAPE
	   || tag == SWF::DEFINESHAPE2
	   || tag == SWF::DEFINESHAPE3);

    uint16_t	character_id = in->read_u16();
    IF_VERBOSE_PARSE(log_msg("  shape_loader: id = %d\n", character_id));

    shape_character_def*	ch = new shape_character_def;
    ch->read(in, tag, true, m);

    IF_VERBOSE_PARSE(log_msg("  bound rect:"); ch->get_bound().print());

    m->add_character(character_id, ch);
}

void define_shape_morph_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == 46);
    uint16_t character_id = in->read_u16();
    IF_VERBOSE_PARSE(log_msg("  shape_morph_loader: id = %d\n", character_id));
    morph2_character_def* morph = new morph2_character_def;
    morph->read(in, tag, true, m);
    m->add_character(character_id, morph);
}

//
// font loaders
//


void	define_font_loader(stream* in, tag_type tag, movie_definition* m)
    // Load a DefineFont or DefineFont2 tag.
{
    assert(tag == 10 || tag == 48);

    uint16_t	font_id = in->read_u16();
		
    font*	f = new font;
    f->read(in, tag, m);

    m->add_font(font_id, f);

    // Automatically keeping fonts in fontlib is
    // problematic.  The app should be responsible for
    // optionally adding fonts to fontlib.
    // //fontlib::add_font(f);
}


void	define_font_info_loader(stream* in, tag_type tag, movie_definition* m)
    // Load a DefineFontInfo tag.  This adds information to an
    // existing font.
{
    assert(tag == 13);

    uint16_t	font_id = in->read_u16();
		
    font*	f = m->get_font(font_id);
    if (f)
	{
	    f->read_font_info(in);
	}
    else
	{
	    log_error("define_font_info_loader: can't find font w/ id %d\n", font_id);
	}
}

/// SWF Tag PlaceObject2 (9) 
struct place_object_2 : public execute_tag
{
    int	m_tag_type;
    char*	m_name;
    float	m_ratio;
    cxform	m_color_transform;
    matrix	m_matrix;
    bool	m_has_matrix;
    bool	m_has_cxform;
    uint16_t	m_depth;
    uint16_t	m_character_id;
    uint16_t 	m_clip_depth;
    enum place_type {
	PLACE,
	MOVE,
	REPLACE
    } m_place_type;
    std::vector<swf_event*>	m_event_handlers;


    place_object_2()
	:
	m_tag_type(0),
	m_name(NULL),
	m_ratio(0),
	m_has_matrix(false),
	m_has_cxform(false),
	m_depth(0),
	m_character_id(0),
	m_clip_depth(0),
	m_place_type(PLACE)
	{
	}

    ~place_object_2()
	{
	    delete [] m_name;
	    m_name = NULL;

	    for (int i = 0, n = m_event_handlers.size(); i < n; i++)
		{
		    delete m_event_handlers[i];
		}
	    m_event_handlers.resize(0);
	}

    void	read(stream* in, tag_type tag, int movie_version)
	{
	    assert(tag == 4 || tag == 26);

	    m_tag_type = tag;

	    if (tag == 4)
		{
		    // Original place_object tag; very simple.
		    m_character_id = in->read_u16();
		    m_depth = in->read_u16();
		    m_matrix.read(in);

		    IF_VERBOSE_PARSE(
			log_msg("  char_id = %d\n"
				"  depth = %d\n"
				"  mat = \n",
				m_character_id,
				m_depth);
			m_matrix.print());

		    if (in->get_position() < in->get_tag_end_position())
			{
			    m_color_transform.read_rgb(in);
			    IF_VERBOSE_PARSE(log_msg("  cxform:\n"); m_color_transform.print());
			}
		}
	    else if (tag == 26)
		{
		    in->align();

		    bool	has_actions = in->read_uint(1) ? true : false;
		    bool	has_clip_bracket = in->read_uint(1) ? true : false;
		    bool	has_name = in->read_uint(1) ? true : false;
		    bool	has_ratio = in->read_uint(1) ? true : false;
		    bool	has_cxform = in->read_uint(1) ? true : false;
		    bool	has_matrix = in->read_uint(1) ? true : false;
		    bool	has_char = in->read_uint(1) ? true : false;
		    bool	flag_move = in->read_uint(1) ? true : false;

		    m_depth = in->read_u16();
		    IF_VERBOSE_PARSE(log_msg("  depth = %d\n", m_depth));

		    if (has_char) {
			m_character_id = in->read_u16();
			IF_VERBOSE_PARSE(log_msg("  char id = %d\n", m_character_id));
		    }

		    if (has_matrix) {
			m_has_matrix = true;
			m_matrix.read(in);
			IF_VERBOSE_PARSE(log_msg("  mat:\n"); m_matrix.print());
		    }
		    if (has_cxform) {
			m_has_cxform = true;
			m_color_transform.read_rgba(in);
			IF_VERBOSE_PARSE(log_msg("  cxform:\n"); m_color_transform.print());
		    }
				
		    if (has_ratio) {
			m_ratio = (float)in->read_u16() / (float)65535;
			IF_VERBOSE_PARSE(log_msg("  ratio: %f\n", m_ratio));
		    }
				
		    if (has_name) {
			m_name = in->read_string();
			IF_VERBOSE_PARSE(log_msg("  name = %s\n", m_name ? m_name : "<null>"));
		    }
		    if (has_clip_bracket) {
			m_clip_depth = in->read_u16(); 
			IF_VERBOSE_PARSE(log_msg("  clip_depth = %d\n", m_clip_depth));
		    }
		    if (has_actions)
			{
			    uint16_t	reserved = in->read_u16();
			    UNUSED(reserved);

			    // The logical 'or' of all the following handlers.
			    // I don't think we care about this...
			    uint32_t	all_flags = 0;
			    if (movie_version >= 6)
				{
				    all_flags = in->read_u32();
				}
			    else
				{
				    all_flags = in->read_u16();
				}
			    UNUSED(all_flags);

			    IF_VERBOSE_PARSE(log_msg("  actions: flags = 0x%X\n", all_flags));

			    // Read swf_events.
			    for (;;)
				{
				    // Read event.
				    in->align();

				    uint32_t	this_flags = 0;
				    if (movie_version >= 6)
					{
					    this_flags = in->read_u32();
					}
				    else
					{
					    this_flags = in->read_u16();
					}

				    if (this_flags == 0)
					{
					    // Done with events.
					    break;
					}

				    swf_event*	ev = new swf_event;
				    ev->read(in, this_flags);

				    m_event_handlers.push_back(ev);
				}
			}


		    if (has_char == true && flag_move == true)
			{
			    // Remove whatever's at m_depth, and put m_character there.
			    m_place_type = REPLACE;
			}
		    else if (has_char == false && flag_move == true)
			{
			    // Moves the object at m_depth to the new location.
			    m_place_type = MOVE;
			}
		    else if (has_char == true && flag_move == false)
			{
			    // Put m_character at m_depth.
			    m_place_type = PLACE;
			}
                                
		    //log_msg("place object at depth %i\n", m_depth);
		}
	}

		
    void	execute(movie* m)
	// Place/move/whatever our object in the given movie.
	{
	    switch (m_place_type) {
	      case PLACE:
		  m->add_display_object(
		      m_character_id,
		      m_name,
		      m_event_handlers,
		      m_depth,
		      m_tag_type != 4,	// original place_object doesn't do replacement
		      m_color_transform,
		      m_matrix,
		      m_ratio,
		      m_clip_depth);
		  break;
		  
	      case MOVE:
		  m->move_display_object(
		      m_depth,
		      m_has_cxform,
		      m_color_transform,
		      m_has_matrix,
		      m_matrix,
		      m_ratio,
		      m_clip_depth);
		  break;
		  
	      case REPLACE:
		  m->replace_display_object(
		      m_character_id,
		      m_name,
		      m_depth,
		      m_has_cxform,
		      m_color_transform,
		      m_has_matrix,
		      m_matrix,
		      m_ratio,
		      m_clip_depth);
		  break;
	    }
	}
    
    void	execute_state(movie* m)
	{
	    execute(m);
	}
    
    void	execute_state_reverse(movie* m, int frame)
	{
	    switch (m_place_type) {
	      case PLACE:
		  // reverse of add is remove
		  m->remove_display_object(m_depth, m_tag_type == 4 ? m_character_id : -1);
		  break;
		  
	      case MOVE:
		  // reverse of move is move
		  m->move_display_object(
		      m_depth,
		      m_has_cxform,
		      m_color_transform,
		      m_has_matrix,
		      m_matrix,
		      m_ratio,
		      m_clip_depth);
		  break;
		  
	      case REPLACE:
	      {
		  // reverse of replace is to re-add the previous object.
		  execute_tag*	last_add = m->find_previous_replace_or_add_tag(frame, m_depth, -1);
		  if (last_add) {
		      last_add->execute_state(m);
		  } else {
		      log_error("reverse REPLACE can't find previous replace or add tag(%d, %d)\n",
				frame, m_depth);
		      
		  }
		  break;
	      }
	    }
	}
    
    virtual uint32	get_depth_id_of_replace_or_add_tag() const
	// "depth_id" is the 16-bit depth & id packed into one 32-bit int.
	{
	    if (m_place_type == PLACE || m_place_type == REPLACE)
		{
		    int	id = -1;
		    if (m_tag_type == 4)
			{
			    // Old-style PlaceObject; the corresponding Remove
			    // is specific to the character_id.
			    id = m_character_id;
			}
		    return ((m_depth & 0x0FFFF) << 16) | (id & 0x0FFFF);
		}
	    else
		{
		    return (uint32) -1;
		}
	}
};


	
void	place_object_2_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == 4 || tag == 26);

    IF_VERBOSE_PARSE(log_msg("  place_object_2\n"));

    place_object_2*	ch = new place_object_2;
    ch->read(in, tag, m->get_version());

    m->add_execute_tag(ch);
}

// Create and initialize a sprite, and add it to the movie.
void
sprite_loader(stream* in, tag_type tag, movie_definition* m)
{
	assert(tag == SWF::DEFINESPRITE); // 39 - DefineSprite
                
	int	character_id = in->read_u16();

	IF_VERBOSE_PARSE(log_msg("  sprite\n  char id = %d\n", character_id));

	/// A DEFINESPRITE tag as part of a DEFINESPRITE
	/// would be a malformed SWF
	if ( ! dynamic_cast<movie_def_impl*>(m) )
	{
		log_error("Malformed SWF (nested DEFINESPRITE tags)");
	}

	// will automatically read the sprite
	sprite_definition* ch = new sprite_definition(m, in);
	//ch->read(in);

	m->add_character(character_id, ch);
}



//
// end_tag
//

// end_tag doesn't actually need to exist.

void	end_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == 0);
    assert(in->get_position() == in->get_tag_end_position());
}


/// SWF Tag RemoveObject2 (28) 
struct remove_object_2 : public execute_tag
{
    int	m_depth, m_id;

    remove_object_2() : m_depth(-1), m_id(-1) {}

    void	read(stream* in, int tag)
	{
	    assert(tag == 5 || tag == 28);

	    if (tag == 5)
		{
		    // Older SWF's allow multiple objects at the same depth;
		    // this m_id disambiguates.  Later SWF's just use one
		    // object per depth.
		    m_id = in->read_u16();
		}
	    m_depth = in->read_u16();
	}

    virtual void	execute(movie* m)
	{
	    m->remove_display_object(m_depth, m_id);
	}

    virtual void	execute_state(movie* m)
	{
	    execute(m);
	}

    virtual void	execute_state_reverse(movie* m, int frame)
	{
	    // reverse of remove is to re-add the previous object.
	    execute_tag*	last_add = m->find_previous_replace_or_add_tag(frame, m_depth, m_id);
	    if (last_add)
		{
		    last_add->execute_state(m);
		}
	    else
		{
		    log_error("reverse REMOVE can't find previous replace or add tag(%d, %d)\n",
			      frame, m_depth);
					
		}
	}

    virtual bool	is_remove_tag() const { return true; }
};


void	remove_object_2_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == 5 || tag == 28);

    remove_object_2*	t = new remove_object_2;
    t->read(in, tag);

    IF_VERBOSE_PARSE(log_msg("  remove_object_2(%d)\n", t->m_depth));

    m->add_execute_tag(t);
}


void	button_sound_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == 17);

    int	button_character_id = in->read_u16();
    button_character_definition* ch = (button_character_definition*) m->get_character_def(button_character_id);
    assert(ch != NULL);

    ch->read(in, tag, m);
}


void	button_character_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == 7 || tag == 34);

    int	character_id = in->read_u16();

    IF_VERBOSE_PARSE(log_msg("  button character loader: char_id = %d\n", character_id));

    button_character_definition*	ch = new button_character_definition;
    ch->read(in, tag, m);

    m->add_character(character_id, ch);
}


//
// export
//


void	export_loader(stream* in, tag_type tag, movie_definition* m)
    // Load an export tag (for exposing internal resources of m)
{
    assert(tag == 56);

    int	count = in->read_u16();

    IF_VERBOSE_PARSE(log_msg("  export: count = %d\n", count));

    // Read the exports.
    for (int i = 0; i < count; i++)
	{
	    uint16_t	id = in->read_u16();
	    char*	symbol_name = in->read_string();
	    IF_VERBOSE_PARSE(log_msg("  export: id = %d, name = %s\n", id, symbol_name));

	    if (font* f = m->get_font(id))
		{
		    // Expose this font for export.
		    m->export_resource(tu_string(symbol_name), f);
		}
	    else if (character_def* ch = m->get_character_def(id))
		{
		    // Expose this movie/button/whatever for export.
		    m->export_resource(tu_string(symbol_name), ch);
		}
	    else if (sound_sample* ch = m->get_sound_sample(id))
		{
		    m->export_resource(tu_string(symbol_name), ch);
		}
	    else
		{
		    log_error("export error: don't know how to export resource '%s'\n",
			      symbol_name);
		}

	    delete [] symbol_name;
	}
}


//
// import
//


void	import_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == SWF::IMPORTASSETS);

    char*	source_url = in->read_string();
    int	count = in->read_u16();

    IF_VERBOSE_PARSE(log_msg("  import: source_url = %s, count = %d\n", source_url, count));

    // Try to load the source movie into the movie library.
    movie_definition*	source_movie = NULL;

    if (s_no_recurse_while_loading == false)
	{
		try {
			source_movie = create_library_movie(URL(source_url));
		} catch (gnash::GnashException& e) {
			log_error("%s\n", e.what());
			source_movie = NULL;
		}
		if (source_movie == NULL)
		{
		    // Give up on imports.
		    log_error("can't import movie from url %s\n", source_url);
		    return;
		}
	}

    // Get the imports.
    for (int i = 0; i < count; i++)
	{
	    uint16_t	id = in->read_u16();
	    char*	symbol_name = in->read_string();
	    IF_VERBOSE_PARSE(log_msg("  import: id = %d, name = %s\n", id, symbol_name));

	    if (s_no_recurse_while_loading)
		{
		    m->add_import(source_url, id, symbol_name);
		}
	    else
		{
		    // @@ TODO get rid of this, always use
		    // s_no_recurse_while_loading, change
		    // create_movie().

		    smart_ptr<resource> res = source_movie->get_exported_resource(symbol_name);
		    if (res == NULL)
			{
			    log_error("import error: resource '%s' is not exported from movie '%s'\n",
				      symbol_name, source_url);
			}
		    else if (font* f = res->cast_to_font())
			{
			    // Add this shared font to the currently-loading movie.
			    m->add_font(id, f);
			}
		    else if (character_def* ch = res->cast_to_character_def())
			{
			    // Add this character to the loading movie.
			    m->add_character(id, ch);
			}
		    else
			{
			    log_error("import error: resource '%s' from movie '%s' has unknown type\n",
				      symbol_name, source_url);
			}
		}

	    delete [] symbol_name;
	}

    delete [] source_url;
}

// Read a DefineText tag.
void	define_edit_text_loader(stream* in, tag_type tag, movie_definition* m)
{
	assert(tag == SWF::DEFINEEDITTEXT); // 37

	uint16_t	character_id = in->read_u16();

	edit_text_character_def* ch = new edit_text_character_def(m);
	IF_VERBOSE_PARSE(log_msg("edit_text_char, id = %d\n", character_id));
	ch->read(in, tag, m);

	m->add_character(character_id, ch);
}

/// Read a DefineText tag.
void
define_text_loader(stream* in, tag_type tag, movie_definition* m)
{
	assert(tag == SWF::DEFINETEXT || tag == SWF::DEFINETEXT2);

	uint16_t	character_id = in->read_u16();
	
	text_character_def* ch = new text_character_def(m);
	IF_VERBOSE_PARSE(log_msg("text_character, id = %d\n", character_id));
	ch->read(in, tag, m);

	// IF_VERBOSE_PARSE(print some stuff);

	m->add_character(character_id, ch);
}

//
// do_action
//




void
do_action_loader(stream* in, tag_type tag, movie_definition* m)
{
	IF_VERBOSE_PARSE(log_msg("tag %d: do_action_loader\n", tag));

	IF_VERBOSE_ACTION(log_msg("-- actions in frame %d\n",
		m->get_loading_frame()));

	assert(in);
	assert(tag == 12);
	assert(m);
		
	do_action*	da = new do_action;
	da->read(in);

	m->add_execute_tag(da);
}

void
do_init_action_loader(stream* in, tag_type tag, movie_definition* m)
{
	assert(tag == 59);

	int sprite_character_id = in->read_u16();

	IF_VERBOSE_PARSE(log_msg("  tag %d: do_init_action_loader\n", tag));
	IF_VERBOSE_ACTION(log_msg("  -- init actions for sprite %d\n",
		sprite_character_id));

	do_action* da = new do_action;
	da->read(in);
	m->add_init_action(sprite_character_id, da);
}


} // namespace gnash::SWF::tag_loaders
} // namespace gnash::SWF

void swf_event::read(stream* in, uint32_t flags)
{
    assert(flags != 0);

    // Scream if more than one bit is set, since we're not set up to handle
    // that, and it doesn't seem possible to express in ActionScript source,
    // so it's important to know if this ever occurs in the wild.
    if (flags & (flags - 1))
	{
	    log_error("error: swf_event::read() -- more than one event type encoded!  "
		      "unexpected! flags = 0x%x\n", flags);
	}

    // 14 bits reserved, 18 bits used

    static const event_id	s_code_bits[18] =
	{
	    event_id::LOAD,
	    event_id::ENTER_FRAME,
	    event_id::UNLOAD,
	    event_id::MOUSE_MOVE,
	    event_id::MOUSE_DOWN,
	    event_id::MOUSE_UP,
	    event_id::KEY_DOWN,
	    event_id::KEY_UP,
	    event_id::DATA,
	    event_id::INITIALIZE,
	    event_id::PRESS,
	    event_id::RELEASE,
	    event_id::RELEASE_OUTSIDE,
	    event_id::ROLL_OVER,
	    event_id::ROLL_OUT,
	    event_id::DRAG_OVER,
	    event_id::DRAG_OUT,
	};

    // Let's see if the event flag we received is for an event that we know of
    if ((pow(2,int(sizeof(s_code_bits)/sizeof(s_code_bits[0])))-1) < flags)
	{
	    log_error("error: swf_event::read() -- unknown / unhandled event type received, flags = 0x%x\n", flags);
	}

    for (int i = 0, mask = 1; i < int(sizeof(s_code_bits)/sizeof(s_code_bits[0])); i++, mask <<= 1)
	{
	    if (flags & mask)
		{
		    m_event = s_code_bits[i];
		    break;
		}
	}

    // what to do w/ key_press???  Is the data in the reserved parts of the flags???
    if (flags & (1 << 17))
	{
	    log_error("swf_event::read -- KEY_PRESS found, not handled yet, flags = 0x%x\n", flags);
	}

    uint32_t	event_length = in->read_u32();
    UNUSED(event_length);

    // Read the actions.
    IF_VERBOSE_ACTION(log_msg("---- actions for event %s\n", m_event.get_function_name().c_str()));
    m_action_buffer.read(in);

    if (m_action_buffer.get_length() != (int) event_length)
	{
	    log_error("error -- swf_event::read(), event_length = %d, but read %d\n",
		      event_length,
		      m_action_buffer.get_length());
	    // @@ discard this event handler??
	}

    // Create a function to execute the actions.
    std::vector<with_stack_entry>	empty_with_stack;
    function_as_object*	func = new function_as_object(&m_action_buffer, NULL, 0, empty_with_stack);
    func->set_length(m_action_buffer.get_length());

    m_method.set_function_as_object(func);
}


} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
