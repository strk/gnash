// tag_loaders.cpp: SWF tags loaders, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

/* $Id: tag_loaders.cpp,v 1.169 2007/12/12 20:40:35 strk Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_file.h"
#include "utility.h"
#include "action.h"
#include "action_buffer.h"
#include "button_character_def.h"
#include "font.h"
#include "fontlib.h"
#include "log.h"
#include "morph2_character_def.h"
#include "shape.h"
#include "stream.h"
#include "styles.h"
#include "dlist.h"
#include "timers.h"
#include "image.h"
#include "zlib_adapter.h"
#include "sprite_definition.h"
#include "sprite_instance.h"
#include "swf_function.h"
#include "swf_event.h"
#include "as_function.h"
#include "movie_def_impl.h"
#include "swf.h"
#include "swf/TagLoadersTable.h"
#include "text_character_def.h"
#include "edit_text_character_def.h"
#include "URL.h"
#include "GnashException.h"
#include "video_stream_def.h"
#include "sound_definition.h"
#include "abc_block.h"
#include "SoundInfo.h"

#ifdef HAVE_ZLIB_H
#include <zlib.h>
#endif
#include <map>

namespace gnash {

	// @@ TODO get rid of this; make it the normal mode.
	extern bool s_no_recurse_while_loading;

}

namespace gnash {

namespace SWF {
namespace tag_loaders {


namespace { // anonymous

///
/// tu_file adapter using a stream underneath
///

/// Provide a tu_file interface around a gnash::stream
class StreamAdapter
{
	stream& s;
	unsigned long startPos;
	unsigned long endPos;
	unsigned long currPos;

	StreamAdapter(stream& str, unsigned long maxPos)
		:
		s(str),
		startPos(s.get_position()),
		endPos(maxPos),
		currPos(startPos)
	{
		assert(endPos > startPos);
#if 0
		if (endPos <= startPos)
		{
			log_error("startPos: %lu, endPos: %lu", startPos, endPos);
			abort();
		}
#endif
	}

	static int readFunc(void* dst, int bytes, void* appdata) 
	{
		StreamAdapter* br = (StreamAdapter*) appdata;

		unsigned bytesLeft = br->endPos - br->currPos;
		if ( bytesLeft < (unsigned)bytes )
		{
			if ( ! bytesLeft ) return 0;
			//log_debug("Requested to read past end of stream range");
			bytes = bytesLeft;
		}
		unsigned actuallyRead = br->s.read((char*)dst, bytes);
		br->currPos += actuallyRead;
		return actuallyRead;
	}

	static long int getStreamSizeFunc(void* appdata)
	{
		StreamAdapter* br = (StreamAdapter*) appdata;
		return (br->endPos - br->startPos);
	}

	static int closeFunc(void* appdata)
	{
		StreamAdapter* br = (StreamAdapter*) appdata;
		delete br;
		return 0; // ok ? or TU_FILE_CLOSE_ERROR ?
	}

public:

	/// Get a tu_file from a gnash::stream
	static std::auto_ptr<tu_file> getFile(stream& str, unsigned long endPos)
	{
		std::auto_ptr<tu_file> ret ( 
			new tu_file (
				new StreamAdapter(str, endPos),
				readFunc,
				0, // write_func wf,
				0, //seek_func sf,
				0, //seek_to_end_func ef,
				0, //tell_func tf,
				0, //get_eof_func gef,
				0, //get_err_func ger
				getStreamSizeFunc, // get_stream_size_func gss,
				closeFunc
			)
		);

		return ret;
	}
};

} // anonymous namespace

//
// Some tag implementations
//


//
// Tag loaders
//


// Silently ignore the contents of this tag.
void null_loader(stream* /*in*/, tag_type /*tag*/, movie_definition* /*m*/)
{
}

// Label the current frame of m with the name from the stream.
void
frame_label_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == SWF::FRAMELABEL); // 43

    char*	n = in->read_string();
    m->add_frame_name(n);

    // FIXME: support SWF6 "named anchors"
    //
    // If SWF version is >= 6 check the byte after terminating NULL
    // if it is 1 this label can be accessed by #name and it's
    // entrance sets the browser URL with anchor appended
    //
    // To avoid relaying on stream::get_position (see task #5838)
    // we should add a new method to that class
    // (ie: stream::current_tag_length)
    //
    // See server/sample/test_clipping_layer.swf for a testcase.
    //
    size_t end_tag = in->get_tag_end_position();
    size_t curr_pos = in->get_position();
    if ( end_tag != curr_pos )
    {
	if ( end_tag == curr_pos + 1 )
	{
	    log_unimpl(_("anchor-labeled frame not supported"));
	}
	else
	{
	    IF_VERBOSE_MALFORMED_SWF(
		log_swferror(_("frame_label_loader end position " SIZET_FMT ", "
			       "read up to " SIZET_FMT),
			     end_tag, curr_pos);
	    );
	}
    }

    delete [] n;
}

// Load JPEG compression tables that can be used to load
// images further along in the stream.
void
jpeg_tables_loader(stream* in, tag_type tag, movie_definition* m)
{
    //GNASH_REPORT_FUNCTION;
    assert(tag == SWF::JPEGTABLES);

    IF_VERBOSE_PARSE
    (
        log_parse(_("  jpeg_tables_loader"));
    );

    unsigned long currPos = in->get_position();
    unsigned long endPos = in->get_tag_end_position();

    if ( endPos == currPos )
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("No bytes to read in JPEGTABLES tag at offset %lu"), currPos);
        );
        return;
    }

    std::auto_ptr<jpeg::input> j_in;

    try
    {
        std::auto_ptr<tu_file> ad( StreamAdapter::getFile(*in, endPos) );
        //  transfer ownerhip to the jpeg::input
        j_in.reset(jpeg::input::create_swf_jpeg2_header_only(ad.release(), true));

    }
    catch (std::exception& e)
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror("Error creating header-only jpeg2 input: %s", e.what());
        );
        return;
    }

    log_debug("Setting jpeg loader to %p", j_in.get());
    m->set_jpeg_loader(j_in);
}


// A JPEG image without included tables; those should be in an
// existing jpeg::input object stored in the movie.
void
define_bits_jpeg_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == SWF::DEFINEBITS); // 6
    assert(in);

    boost::uint16_t	character_id = in->read_u16();

    //
    // Read the image data.
    //

    if (m->get_create_bitmaps() != DO_LOAD_BITMAPS) return;

    jpeg::input*	j_in = m->get_jpeg_loader();
    if ( ! j_in )
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("DEFINEBITS: No jpeg loader registered in movie definition - discarding bitmap character %d"), character_id);
        );
        return;
    }

    assert(j_in);
    j_in->discard_partial_buffer();
    
    std::auto_ptr<image::rgb> im;
    try
    {
        im.reset ( image::read_swf_jpeg2_with_tables(j_in) );
    }
    catch (std::exception& e)
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror("Error reading jpeg2 with headers for character id %d: %s", character_id, e.what());
        );
        return;
    }
    
    
    bitmap_character_def* ch = new bitmap_character_def(im);
    
    if ( m->get_bitmap_character_def(character_id) )
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("DEFINEBITS: Duplicate id (%d) for bitmap character - discarding it"), character_id);
        );
    }
    else
    {
        m->add_bitmap_character_def(character_id, ch);
    }
}


void
define_bits_jpeg2_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == SWF::DEFINEBITSJPEG2); // 21

    in->ensureBytes(2);
    boost::uint16_t	character_id = in->read_u16();

    IF_VERBOSE_PARSE
    (
	log_parse(_("  define_bits_jpeg2_loader: charid = %d pos = %ld"),
		  character_id, in->get_position());
    );

    //
    // Read the image data.
    //

    if (m->get_create_bitmaps() == DO_LOAD_BITMAPS)
    {
	std::auto_ptr<tu_file> ad( StreamAdapter::getFile(*in, in->get_tag_end_position()) );
	std::auto_ptr<image::rgb> im ( image::read_jpeg(ad.get()) );

	if ( m->get_bitmap_character_def(character_id) )
	{
	    IF_VERBOSE_MALFORMED_SWF(
		log_swferror(_("DEFINEBITSJPEG2: Duplicate id (%d) for bitmap character - discarding it"), character_id);
	    );
	}
	else
	{
	    bitmap_character_def* ch = new bitmap_character_def(im);
    	    m->add_bitmap_character_def(character_id, ch);
	}
    }
}


#ifdef HAVE_ZLIB_H
void inflate_wrapper(stream& in, void* buffer, int buffer_bytes)
    // Wrapper function -- uses Zlib to uncompress in_bytes worth
    // of data from the input file into buffer_bytes worth of data
    // into *buffer.
{
    assert(buffer);
    assert(buffer_bytes > 0);

    z_stream d_stream; /* decompression stream */

    d_stream.zalloc = (alloc_func)0;
    d_stream.zfree = (free_func)0;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in  = 0;
    d_stream.avail_in = 0;

    d_stream.next_out = (Byte*) buffer;
    d_stream.avail_out = (uInt) buffer_bytes;

    int err = inflateInit(&d_stream);
    if (err != Z_OK) {
	IF_VERBOSE_MALFORMED_SWF(
	log_swferror(_("inflate_wrapper() inflateInit() returned %d (%s)"), err, d_stream.msg);
	);
	return;
    }

#define CHUNKSIZE 256

    boost::uint8_t buf[CHUNKSIZE];
    unsigned long endTagPos = in.get_tag_end_position();

    for (;;)
    {
	unsigned int chunkSize = CHUNKSIZE;
	assert(in.get_position() <= endTagPos);
        unsigned int availableBytes =  endTagPos - in.get_position();
	if ( availableBytes < chunkSize )
	{
		if ( ! availableBytes )
		{
			// nothing more to read
			IF_VERBOSE_MALFORMED_SWF(
			log_swferror(_("inflate_wrapper(): no end of zstream found within swf tag boundaries"));
			);
			break;
		}
		chunkSize = availableBytes;
	}
	
	// Fill the buffer
	assert(sizeof(char) == sizeof(boost::uint8_t));
	in.read((char*)buf, chunkSize);
	d_stream.next_in = &buf[0];
	d_stream.avail_in = chunkSize;

	err = inflate(&d_stream, Z_SYNC_FLUSH);
	if (err == Z_STREAM_END)
	{
		// correct end
		break;
	}

	if (err != Z_OK)
	{
	    IF_VERBOSE_MALFORMED_SWF(
	    log_swferror(_("inflate_wrapper() inflate() returned %d (%s)"), err, d_stream.msg);
	    );
	    break;
	}
    }

    err = inflateEnd(&d_stream);
    if (err != Z_OK)
    {
	    log_error(_("inflate_wrapper() inflateEnd() return %d (%s)"), err, d_stream.msg);
    }
}
#endif // HAVE_ZLIB_H


// loads a define_bits_jpeg3 tag. This is a jpeg file with an alpha
// channel using zlib compression.
void
define_bits_jpeg3_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == SWF::DEFINEBITSJPEG3); // 35

    boost::uint16_t	character_id = in->read_u16();

    IF_VERBOSE_PARSE
    (
	log_parse(_("  define_bits_jpeg3_loader: charid = %d pos = %lx"),
		  character_id, in->get_position());
    );

    boost::uint32_t	jpeg_size = in->read_u32();
    boost::uint32_t	alpha_position = in->get_position() + jpeg_size;

    if (m->get_create_bitmaps() == DO_LOAD_BITMAPS)
    {

#ifndef HAVE_ZLIB_H
	log_error(_("gnash is not linked to zlib -- can't load jpeg3 image data"));
	return;
#else
	//
	// Read the image data.
	//

	// Read rgb data.
	std::auto_ptr<tu_file> ad( StreamAdapter::getFile(*in, alpha_position) );
	std::auto_ptr<image::rgba> im( image::read_swf_jpeg3(ad.get()) );

	// Read alpha channel.
	in->set_position(alpha_position);

	size_t imWidth = im->width();
	size_t imHeight = im->height();

	size_t	buffer_bytes = imWidth * imHeight;

	boost::scoped_array<boost::uint8_t> buffer ( new boost::uint8_t[buffer_bytes] );

	inflate_wrapper(*in, buffer.get(), buffer_bytes);

	// TESTING:
	// magical trevor contains this tag
	//  ea8bbad50ccbc52dd734dfc93a7f06a7  6964trev3c.swf
	//
	boost::uint8_t* data = im->data();
	for (size_t i = 0; i < buffer_bytes; ++i)
	{
	    data[4*i+3] = buffer[i];
	}

	// Create bitmap character.
	bitmap_character_def* ch = new bitmap_character_def(im);

	m->add_bitmap_character_def(character_id, ch);
#endif
    }
}


void
define_bits_lossless_2_loader(stream* in, tag_type tag, movie_definition* m)
{
    // tags 20 || 36
    assert(tag == SWF::DEFINELOSSLESS || tag == SWF::DEFINELOSSLESS2);

    in->ensureBytes(2+2+2+1); // the initial header 

    boost::uint16_t	character_id = in->read_u16();
    boost::uint8_t	bitmap_format = in->read_u8();	// 3 == 8 bit, 4 == 16 bit, 5 == 32 bit
    boost::uint16_t	width = in->read_u16();
    boost::uint16_t	height = in->read_u16();

    IF_VERBOSE_PARSE
    (
	log_parse(_("  defbitslossless2: tag = %d, id = %d, "
		    "fmt = %d, w = %d, h = %d"),
		  tag, character_id, bitmap_format, width, height);
    );

    // TODO: there's a lot of duplicated code in this function, we should clean it up

    //bitmap_info*	bi = NULL;
    if (m->get_create_bitmaps() == DO_LOAD_BITMAPS)
    {
#ifndef HAVE_ZLIB_H
	log_error(_("gnash is not linked to zlib -- can't load zipped image data"));
	return;
#else
	if (tag == SWF::DEFINELOSSLESS) // 20
	{

	    // RGB image data.
	    std::auto_ptr<image::rgb> image ( image::create_rgb(width, height) );

	    if (bitmap_format == 3)
	    {
		// 8-bit data, preceded by a palette.

		const int bytes_per_pixel = 1;

    		in->ensureBytes(1); // color table size
		int color_table_size = in->read_u8();
		color_table_size++;	// !! SWF stores one less than the actual size

		int pitch = (width * bytes_per_pixel + 3) & ~3;

		int buffer_bytes = color_table_size * 3 + pitch * height;
		boost::scoped_array<boost::uint8_t> buffer ( new boost::uint8_t[buffer_bytes] );

		inflate_wrapper(*in, buffer.get(), buffer_bytes);
		assert(in->get_position() <= in->get_tag_end_position());

		boost::uint8_t* color_table = buffer.get();

		for (int j = 0; j < height; j++)
		{
		    boost::uint8_t*	image_in_row = buffer.get() + color_table_size * 3 + j * pitch;
		    boost::uint8_t*	image_out_row = image->scanline(j);
		    for (int i = 0; i < width; i++)
		    {
			boost::uint8_t	pixel = image_in_row[i * bytes_per_pixel];
			image_out_row[i * 3 + 0] = color_table[pixel * 3 + 0];
			image_out_row[i * 3 + 1] = color_table[pixel * 3 + 1];
			image_out_row[i * 3 + 2] = color_table[pixel * 3 + 2];
		    }
		}

	    }
	    else if (bitmap_format == 4)
	    {
		// 16 bits / pixel
		const int bytes_per_pixel = 2;
		int pitch = (width * bytes_per_pixel + 3) & ~3;

		int buffer_bytes = pitch * height;
		boost::scoped_array<boost::uint8_t> buffer ( new boost::uint8_t[buffer_bytes] );

		inflate_wrapper(*in, buffer.get(), buffer_bytes);
		assert(in->get_position() <= in->get_tag_end_position());

		for (int j = 0; j < height; j++)
		{
		    boost::uint8_t*	image_in_row = buffer.get() + j * pitch;
		    boost::uint8_t*	image_out_row = image->scanline(j);
		    for (int i = 0; i < width; i++)
		    {
			boost::uint16_t	pixel = image_in_row[i * 2] | (image_in_row[i * 2 + 1] << 8);

			// @@ How is the data packed???  I'm just guessing here that it's 565!
			image_out_row[i * 3 + 0] = (pixel >> 8) & 0xF8;	// red
			image_out_row[i * 3 + 1] = (pixel >> 3) & 0xFC;	// green
			image_out_row[i * 3 + 2] = (pixel << 3) & 0xF8;	// blue
		    }
		}

	    }
	    else if (bitmap_format == 5)
	    {
		// 32 bits / pixel, input is ARGB format (???)
		const int bytes_per_pixel = 4;
		int pitch = width * bytes_per_pixel;

		int buffer_bytes = pitch * height;
		boost::scoped_array<boost::uint8_t> buffer ( new boost::uint8_t[buffer_bytes] );

		inflate_wrapper(*in, buffer.get(), buffer_bytes);
		assert(in->get_position() <= in->get_tag_end_position());

		// Need to re-arrange ARGB into RGB.
		for (int j = 0; j < height; j++)
		{
		    boost::uint8_t*	image_in_row = buffer.get() + j * pitch;
		    boost::uint8_t*	image_out_row = image->scanline(j);
		    for (int i = 0; i < width; i++)
		    {
			boost::uint8_t a = image_in_row[i * 4 + 0];
			boost::uint8_t r = image_in_row[i * 4 + 1];
			boost::uint8_t g = image_in_row[i * 4 + 2];
			boost::uint8_t b = image_in_row[i * 4 + 3];
			image_out_row[i * 3 + 0] = r;
			image_out_row[i * 3 + 1] = g;
			image_out_row[i * 3 + 2] = b;
			a = a;	// Inhibit warning.
		    }
		}

	    }

	    if ( m->get_bitmap_character_def(character_id) )
	    {
		IF_VERBOSE_MALFORMED_SWF(
		    log_swferror(_("DEFINEBITSLOSSLESS: Duplicate id (%d) for bitmap character - discarding it"), character_id);
		);
	    }
	    else
	    {
		bitmap_character_def* ch = new bitmap_character_def(image);

		// add image to movie, under character id.
		m->add_bitmap_character_def(character_id, ch);
	    }
	}
	else
	{
	    // RGBA image data.
	    assert(tag == SWF::DEFINELOSSLESS2); // 36

	    std::auto_ptr<image::rgba> image ( image::create_rgba(width, height) );

	    if (bitmap_format == 3)
	    {
		// 8-bit data, preceded by a palette.

		const int bytes_per_pixel = 1;
    		in->ensureBytes(1); // color table size
		int color_table_size = in->read_u8();
		color_table_size++;	// !! SWF stores one less than the actual size

		int pitch = (width * bytes_per_pixel + 3) & ~3;

		int buffer_bytes = color_table_size * 4 + pitch * height;
		boost::scoped_array<boost::uint8_t> buffer ( new boost::uint8_t[buffer_bytes] );

		inflate_wrapper(*in, buffer.get(), buffer_bytes);
		assert(in->get_position() <= in->get_tag_end_position());

		boost::uint8_t* color_table = buffer.get();

		for (int j = 0; j < height; j++)
		{
		    boost::uint8_t*	image_in_row = buffer.get() + color_table_size * 4 + j * pitch;
		    boost::uint8_t*	image_out_row = image->scanline(j);
		    for (int i = 0; i < width; i++)
		    {
			boost::uint8_t	pixel = image_in_row[i * bytes_per_pixel];
			image_out_row[i * 4 + 0] = color_table[pixel * 4 + 0];
			image_out_row[i * 4 + 1] = color_table[pixel * 4 + 1];
			image_out_row[i * 4 + 2] = color_table[pixel * 4 + 2];
			image_out_row[i * 4 + 3] = color_table[pixel * 4 + 3];
		    }
		}

	    }
	    else if (bitmap_format == 4)
	    {
		// 16 bits / pixel
		const int bytes_per_pixel = 2;
		int pitch = (width * bytes_per_pixel + 3) & ~3;

		int buffer_bytes = pitch * height;
		boost::scoped_array<boost::uint8_t> buffer ( new boost::uint8_t[buffer_bytes] );

		inflate_wrapper(*in, buffer.get(), buffer_bytes);
		assert(in->get_position() <= in->get_tag_end_position());

		for (int j = 0; j < height; j++)
		{
		    boost::uint8_t*	image_in_row = buffer.get() + j * pitch;
		    boost::uint8_t*	image_out_row = image->scanline(j);
		    for (int i = 0; i < width; i++)
		    {
		        boost::uint16_t	pixel = image_in_row[i * 2] | (image_in_row[i * 2 + 1] << 8);

		        // @@ How is the data packed???  I'm just guessing here that it's 565!
		        image_out_row[i * 4 + 0] = 255;			// alpha
		        image_out_row[i * 4 + 1] = (pixel >> 8) & 0xF8;	// red
		        image_out_row[i * 4 + 2] = (pixel >> 3) & 0xFC;	// green
		        image_out_row[i * 4 + 3] = (pixel << 3) & 0xF8;	// blue
		    }
		}

	    }
	    else if (bitmap_format == 5)
	    {
		// 32 bits / pixel, input is ARGB format

		inflate_wrapper(*in, image->data(), width * height * 4);
		assert(in->get_position() <= in->get_tag_end_position());

		// Need to re-arrange ARGB into RGBA.
		for (int j = 0; j < height; j++)
		{
		    boost::uint8_t*	image_row = image->scanline(j);
		    for (int i = 0; i < width; i++)
		    {
			boost::uint8_t	a = image_row[i * 4 + 0];
			boost::uint8_t	r = image_row[i * 4 + 1];
			boost::uint8_t	g = image_row[i * 4 + 2];
			boost::uint8_t	b = image_row[i * 4 + 3];
			image_row[i * 4 + 0] = r;
			image_row[i * 4 + 1] = g;
			image_row[i * 4 + 2] = b;
			image_row[i * 4 + 3] = a;
		    }
		}
	    }

	    bitmap_character_def* ch = new bitmap_character_def(image);

	    // add image to movie, under character id.
	    m->add_bitmap_character_def(character_id, ch);
	}
#endif // HAVE_ZLIB_H

#if 0
    assert(bi->get_ref_count() == 0);

    bitmap_character*	ch = new bitmap_character(bi);

    // add image to movie, under character id.
    m->add_bitmap_character(character_id, ch);
#endif
    	}
}

// This is like null_loader except it prints a message to nag us to fix it.
void
fixme_loader(stream* /*in*/, tag_type tag, movie_definition* /*m*/)
{
	static std::map<tag_type, bool> warned;
	if ( ! warned[tag] )
	{
		log_unimpl(_("  FIXME: tagtype = %d"), tag);
		warned[tag] = true;
	}
}

void define_shape_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == SWF::DEFINESHAPE
	   || tag == SWF::DEFINESHAPE2
	   || tag == SWF::DEFINESHAPE3
	   || tag == SWF::DEFINESHAPE4 || tag == SWF::DEFINESHAPE4_);

    boost::uint16_t	character_id = in->read_u16();
    IF_VERBOSE_PARSE
    (
	log_parse(_("  shape_loader: id = %d"), character_id);
    );

    shape_character_def*	ch = new shape_character_def;
    ch->read(in, tag, true, m);

    m->add_character(character_id, ch);
}

void define_shape_morph_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == SWF::DEFINEMORPHSHAPE
		|| tag == SWF::DEFINEMORPHSHAPE2
		|| tag == SWF::DEFINEMORPHSHAPE2_); 

    boost::uint16_t character_id = in->read_u16();

    IF_VERBOSE_PARSE
    (
	log_parse(_("  shape_morph_loader: id = %d"), character_id);
    );

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
    assert(tag == SWF::DEFINEFONT
	   || tag == SWF::DEFINEFONT2
	   || tag == SWF::DEFINEFONT3 ); // 10 || 48 || 75

    boost::uint16_t font_id = in->read_u16();

    font* f = new font;
    f->read(in, tag, m);

    m->add_font(font_id, f);

    // Automatically keeping fonts in fontlib is
    // problematic.  The app should be responsible for
    // optionally adding fonts to fontlib.
    // //fontlib::add_font(f);
}


// See description in header
void	define_font_info_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == SWF::DEFINEFONTINFO || tag == SWF::DEFINEFONTINFO2);

    boost::uint16_t font_id = in->read_u16();

    font* f = m->get_font(font_id);
    if (f)
    {
	f->read_font_info(in, tag, m);
    }
    else
    {
	IF_VERBOSE_MALFORMED_SWF(
	    log_swferror(_("define_font_info_loader: "
			   "can't find font w/ id %d"), font_id);
	);
    }
}

// Set font name for a font.
void define_font_name_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == SWF::DEFINEFONTNAME);

    boost::uint16_t font_id = in->read_u16();

    font* f = m->get_font(font_id);
    if (f)
    {
        f->read_font_name(in, tag, m);
    }
    else
    {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("define_font_name_loader: "
                           "can't find font w/ id %d"), font_id);
        );
    }
}

// Create and initialize a sprite, and add it to the movie.
void
sprite_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == SWF::DEFINESPRITE); // 39 - DefineSprite

    int	character_id = in->read_u16();

    IF_VERBOSE_PARSE
    (
	log_parse(_("  sprite:  char id = %d"), character_id);
    );

    // A DEFINESPRITE tag as part of a DEFINESPRITE
    // would be a malformed SWF
    if ( ! dynamic_cast<movie_def_impl*>(m) )
    {
	IF_VERBOSE_MALFORMED_SWF(
	    log_swferror(_("nested DEFINESPRITE tags"));
	);
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

void	button_sound_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == SWF::DEFINEBUTTONSOUND); // 17

    int	button_character_id = in->read_u16();
    character_def* chdef = m->get_character_def(button_character_id);

    assert ( dynamic_cast<button_character_definition*> (chdef) );
    button_character_definition* ch = static_cast<button_character_definition*> (chdef);
    assert(ch != NULL);

    ch->read(in, tag, m);
}


void	button_character_loader(stream* in, tag_type tag, movie_definition* m)
{
    // 7 || 34
    assert(tag == SWF::DEFINEBUTTON || tag == SWF::DEFINEBUTTON2);

    int	character_id = in->read_u16();

    IF_VERBOSE_PARSE
    (
	log_parse(_("  button character loader: char_id = %d"), character_id);
    );

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
    assert(tag == SWF::EXPORTASSETS); // 56

    int	count = in->read_u16();

    IF_VERBOSE_PARSE
    (
	log_parse(_("  export: count = %d"), count);
    );

    // Read the exports.
    for (int i = 0; i < count; i++)
    {
	boost::uint16_t	id = in->read_u16();
	char*	symbol_name = in->read_string();

	IF_VERBOSE_PARSE (
	    log_parse(_("  export: id = %d, name = %s"), id, symbol_name);
	);

	if (font* f = m->get_font(id))
	{
	    // Expose this font for export.
	    m->export_resource(symbol_name, f);
	}
	else if (character_def* ch = m->get_character_def(id))
	{
	    // Expose this movie/button/whatever for export.
	    m->export_resource(symbol_name, ch);
	}
	else if (sound_sample* ch = m->get_sound_sample(id))
	{
	    m->export_resource(symbol_name, ch);
	}
	else
	{
	    log_error(_("don't know how to export resource '%s' "
			"with id %d (can't find that id)"),
		      symbol_name, id);
	}

	delete [] symbol_name;
    }
}


//
// import
//


void	import_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == SWF::IMPORTASSETS || tag == SWF::IMPORTASSETS2);

    char* source_url = in->read_string();

    // Resolve relative urls against baseurl
    URL abs_url(source_url, get_base_url());

    unsigned char import_version = 0;

    if ( tag == SWF::IMPORTASSETS2 )
    {
	import_version = in->read_uint(8);
	unsigned char reserved = in->read_uint(8);
	UNUSED(reserved);
    }

    int count = in->read_u16();

    IF_VERBOSE_PARSE
    (
	log_parse(_("  import: version = %u, source_url = %s (%s), count = %d"), import_version, abs_url.str().c_str(), source_url, count);
    );


    // Try to load the source movie into the movie library.
    movie_definition*	source_movie = NULL;

    if (s_no_recurse_while_loading == false)
    {
	try {
	    source_movie = create_library_movie(abs_url);
	} catch (gnash::GnashException& e) {
	    log_error(_("Exception: %s"), e.what());
	    source_movie = NULL;
	}
	if (source_movie == NULL)
	{
	    // Give up on imports.
	    log_error(_("can't import movie from url %s"), abs_url.str().c_str());
	    return;
	}

	// Quick consistency check, we might as well do
	// something smarter, if we agree on semantic
	if (source_movie == m)
	{
	    IF_VERBOSE_MALFORMED_SWF(
		log_swferror(_("Movie attempts to import symbols from itself."));
	    );
	    return;
	}
    }

    // Get the imports.
    for (int i = 0; i < count; i++)
    {
	boost::uint16_t	id = in->read_u16();
	char*	symbol_name = in->read_string();
	IF_VERBOSE_PARSE
	(
	    log_parse(_("  import: id = %d, name = %s"), id, symbol_name);
	);

	if (s_no_recurse_while_loading)
	{
	    m->add_import(source_url, id, symbol_name);
	}
	else
	{
	    // @@ TODO get rid of this, always use
	    // s_no_recurse_while_loading, change
	    // create_movie().

	    boost::intrusive_ptr<resource> res = source_movie->get_exported_resource(symbol_name);
	    if (res == NULL)
	    {
		log_error(_("import error: could not find resource '%s' in movie '%s'"),
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
		log_error(_("import error: resource '%s' from movie '%s' has unknown type"),
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

    boost::uint16_t	character_id = in->read_u16();

    edit_text_character_def* ch = new edit_text_character_def(m);
    IF_VERBOSE_PARSE
    (
	log_parse(_("edit_text_char, id = %d"), character_id);
    );
    ch->read(in, tag, m);

    m->add_character(character_id, ch);
}

// See description in header
void
define_text_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == SWF::DEFINETEXT || tag == SWF::DEFINETEXT2);

    boost::uint16_t	character_id = in->read_u16();

    text_character_def* ch = new text_character_def(m);
    IF_VERBOSE_PARSE
    (
	log_parse(_("text_character, id = %d"), character_id);
    );
    ch->read(in, tag, m);

    // IF_VERBOSE_PARSE(print some stuff);

    m->add_character(character_id, ch);
}

//
// Sound
//

// Forward declaration
/*static void sound_expand(stream *in, media::sound_handler::format_type &format,
	bool sample_16bit, bool stereo, unsigned int &sample_count,
	unsigned char* &data, unsigned &data_bytes);
*/
// Common data
static int	s_sample_rate_table[] = { 5512, 11025, 22050, 44100 };

// @@ There are two sets of code to decode/expand/byteswap audio here.
// @@ There should be one (search for ADPCM).

// Load a DefineSound tag.
void
define_sound_loader(stream* in, tag_type tag, movie_definition* m)
{
	assert(tag == SWF::DEFINESOUND); // 14

	media::sound_handler* handler = get_sound_handler();

	in->ensureBytes(2+4+1+4); // character id + flags + sample count

	boost::uint16_t	character_id = in->read_u16();

	media::audioCodecType	format = static_cast<media::audioCodecType>(in->read_uint(4));
	int	sample_rate = in->read_uint(2);	// multiples of 5512.5
	bool	sample_16bit = in->read_bit(); 
	bool	stereo = in->read_bit(); 

	unsigned int	sample_count = in->read_u32();

	if (format == media::AUDIO_CODEC_MP3) {
		in->ensureBytes(2);
		boost::int16_t	delay_seek = in->read_s16();	// FIXME - not implemented/used
		// The DelaySeek field has the following meaning:
		// * If this value is positive, the player seeks this number of
		//   samples into the sound block before the sound is played.
		//   However, the seek is only performed if the player reached
		//   the current frame via a GotoFrame action, otherwise no
		//   seek is performed.
		//
		// * If this value is negative the player plays this number of
		//   silent samples before playing the sound block. The player
		//   behaves this way, regardless of how the current frame was
		//   reached.
		//
		// quoted from
		// http://www-lehre.informatik.uni-osnabrueck.de/~fbstark/diplom/docs/swf/Sounds.htm
	}

	IF_VERBOSE_PARSE
	(
	    log_parse(_("define sound: ch=%d, format=%d, "
			"rate=%d, 16=%d, stereo=%d, ct=%d"),
		      character_id, int(format), sample_rate,
		      int(sample_16bit), int(stereo), sample_count);
	);

	// If we have a sound_handler, ask it to init this sound.

	if (handler)
	{
	    if (! (sample_rate >= 0 && sample_rate <= 3))
	    {
		IF_VERBOSE_MALFORMED_SWF(
		    log_swferror(_("Bad sound sample rate %d read from SWF header"), sample_rate);
		);
		return;
	    }

	    // First it is the amount of data from file,
	    // then the amount allocated at *data (it may grow)
	    unsigned data_bytes = in->get_tag_end_position() - in->get_position();
	    unsigned char *data = new unsigned char[data_bytes];

	    in->read((char*)data, data_bytes);

	    // Store all the data in a SoundInfo object
	    std::auto_ptr<media::SoundInfo> sinfo;
	    sinfo.reset(new media::SoundInfo(format, stereo, s_sample_rate_table[sample_rate], sample_count, sample_16bit));

	    // Stores the sounddata in the soundhandler, and the ID returned
	    // can be used to starting, stopping and deleting that sound
	    // NOTE: ownership of 'data' is transferred to the sound hanlder 
	    int	handler_id = handler->create_sound(data, data_bytes, sinfo);

	    if (handler_id >= 0)
	    {
		sound_sample* sam = new sound_sample(handler_id);
		m->add_sound_sample(character_id, sam);
	    }

	}
	else
	{
	    // is this nice to do?
	    log_error(_("There is no sound handler currently active, "
			"so character with id %d will NOT be added to "
			"the dictionary"),
		      character_id);
	}
}

// Load a SoundStreamHead(2) tag.
void
sound_stream_head_loader(stream* in, tag_type tag, movie_definition* m)
{
    media::sound_handler* handler = get_sound_handler();

    // 18 || 45
    assert(tag == SWF::SOUNDSTREAMHEAD || tag == SWF::SOUNDSTREAMHEAD2);

    // If we don't have a sound_handler registered stop here
    if (!handler) return;

    // FIXME:
    // no character id for soundstreams... so we make one up...
    // This only works if there is only one stream in the movie...
    // The right way to do it is to make seperate structures for streams
    // in movie_def_impl.

    // 1 byte for playback info, 1 for stream info, 2 for sample count
    in->ensureBytes(4);

    // These are all unused by current implementation
    int reserved = in->read_uint(4); UNUSED(reserved);
    int playbackSoundRate = in->read_uint(2);
    bool playbackSound16bit = in->read_bit();
    bool playbackSoundStereo = in->read_bit();

    // These are the used ones
    media::audioCodecType format = static_cast<media::audioCodecType>(in->read_uint(4)); // TODO: check input !
    int streamSoundRate = in->read_uint(2);	// multiples of 5512.5
    bool streamSound16bit = in->read_bit(); 
    bool streamSoundStereo = in->read_bit(); 

    // checks if this is a new streams header or just one in the row
    if (format == 0 && streamSoundRate == 0 && !streamSound16bit && !streamSoundStereo) return;

    // 2 bytes here
    unsigned int sampleCount = in->read_u16();

    if ( ! sampleCount )
    {
        log_error("No samples advertised for sound stream");
    }

    int latency = 0;
    if (format == media::AUDIO_CODEC_MP3)
    {
        try
	{
                in->ensureBytes(2);
                latency = in->read_s16(); // UNUSED !!
        }
	catch (ParserException& ex)
	{
		// See https://savannah.gnu.org/bugs/?21729 for an example 
		// triggering this.
		log_swferror("MP3 sound stream lacks a 'latency' field");
        }
        //garbage = in->read_uint(16);
    }

    IF_VERBOSE_PARSE
    (
	log_parse(_("sound stream head: format=%d, rate=%d, 16=%d, stereo=%d, ct=%d, latency=%d"),
		  int(format), streamSoundRate, int(streamSound16bit), int(streamSoundStereo), sampleCount, latency);
    );

    // Wot about reading the sample_count samples?

    if (! (streamSoundRate >= 0 && streamSoundRate <= 3))
    {
        IF_VERBOSE_MALFORMED_SWF(
	    log_swferror(_("Bad sound sample rate %d read from SWF header"),
			 streamSoundRate);
	    );
        return;
    }

	// Store all the data in a SoundInfo object
	std::auto_ptr<media::SoundInfo> sinfo;
	sinfo.reset(new media::SoundInfo(format, streamSoundStereo, s_sample_rate_table[streamSoundRate], sampleCount, streamSound16bit));

	// Stores the sounddata in the soundhandler, and the ID returned
	// can be used to starting, stopping and deleting that sound
	int	handler_id = handler->create_sound(NULL, 0, sinfo);

    m->set_loading_sound_stream_id(handler_id);
}


// Load a SoundStreamBlock tag.
void
sound_stream_block_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == SWF::SOUNDSTREAMBLOCK); // 19

    media::sound_handler* handler = get_sound_handler();

    // If we don't have a sound_handler registered stop here
    if (!handler) return;

    // Get the ID of the sound stream currently being loaded
    int handle_id = m->get_loading_sound_stream_id();

    // Get the SoundInfo object that contains info about the sound stream.
    // Ownership of the object is in the soundhandler
    media::SoundInfo* sinfo = handler->get_sound_info(handle_id);

    // If there is no SoundInfo something is wrong...
    if (!sinfo) return;

    media::audioCodecType format = sinfo->getFormat();
    unsigned int sample_count = sinfo->getSampleCount();

    // discard garbage data if format is MP3
    if (format == media::AUDIO_CODEC_MP3)
    {
        // 2bytes is sampleCount
        // 2bytes is seekSamples
        //log_debug("Skipping 4 garbage bytes of MP3 format... (2 are samples count, 2 are seek samples!)");
        in->ensureBytes(4);
        in->skip_bytes(4);
    }

    unsigned int data_bytes = in->get_tag_end_position() - in->get_position();
    if ( ! data_bytes )
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror("No data bytes left to read in SOUNDSTREAMBLOCK tag");
        );
        return;
    }

    unsigned char *data = new unsigned char[data_bytes];
    in->read((char*)data, data_bytes);

    // Fill the data on the apropiate sound, and receives the starting point
    // for later "start playing from this frame" events.
    //
    // ownership of 'data' is transferred here
    //
    long start = handler->fill_stream_data(data, data_bytes, sample_count, handle_id);

    start_stream_sound_tag*	ssst = new start_stream_sound_tag();
    ssst->read(m, handle_id, start);
}

void
define_video_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == SWF::DEFINEVIDEOSTREAM); // 60
    boost::uint16_t character_id = in->read_u16();

    std::auto_ptr<video_stream_definition> chdef ( new video_stream_definition(character_id) );
    chdef->readDefineVideoStream(in, tag, m);

    m->add_character(character_id, chdef.release());

}

void
video_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == SWF::VIDEOFRAME); // 61

    boost::uint16_t character_id = in->read_u16();
    character_def* chdef = m->get_character_def(character_id);

    if ( ! chdef )
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("VideoFrame tag refers to unknown video stream id %d"), character_id);
        );
        return;
    }

    // TODO: add a character_def::cast_to_video_def ?
    video_stream_definition* vdef = dynamic_cast<video_stream_definition*> (chdef);
    if ( ! vdef )
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("VideoFrame tag refers to a non-video character %d (%s)"), character_id, typeName(*chdef).c_str());
        );
        return;
    }

    vdef->readDefineVideoFrame(in, tag, m);
}

void
file_attributes_loader(stream* in, tag_type tag, movie_definition* /*m*/)
{
    assert(tag == SWF::FILEATTRIBUTES); // 69

    typedef struct file_attrs_flags_t {
	unsigned reserved1:3;
	unsigned has_metadata:1;
	unsigned reserved2:3;
	unsigned use_network:1;
	unsigned reserved3:24;
    } file_attrs_flags;

    file_attrs_flags flags;

    flags.reserved1 = in->read_uint(3);
    flags.has_metadata = in->read_bit(); 
    flags.reserved2 = in->read_uint(3);
    flags.use_network = in->read_bit(); 
    flags.reserved3 = in->read_uint(24);

    IF_VERBOSE_PARSE
    (
	log_parse(_("  file attributes: has_metadata=%s use_network=%s"),
		  flags.has_metadata ? _("true") : _("false"),
		  flags.use_network ? _("true") : _("false"))
    );

    if ( ! flags.use_network )
    {
	log_unimpl(_("FileAttributes tag in the SWF requests that "
		    "network access is not granted to this movie "
		    "(or application?) when loaded from the filesystem. "
	            "Anyway Gnash won't care; "
		    "use white/black listing in your .gnashrc instead"));
    }

    // TODO:
    // 	- attach info to movie_definition.
    // 	- don't allow later FileAttributes tags in the same movie
    // 	  to override the first one used.
    // 	- only use if it is the *first* tag in the stream.
}

void
metadata_loader(stream* in, tag_type tag, movie_definition* /*m*/)
{
    assert(tag == SWF::METADATA); // 77

    // this is supposed to be an XML string
    char* metadata = in->read_string();

    IF_VERBOSE_PARSE (
	log_parse(_("  metadata = [[\n%s\n]]"), metadata);
    );

    log_unimpl(_("METADATA tag unused: %s"), metadata);

    // TODO: attach to movie_definition instead
    //       (should we parse the XML maybe?)

    delete [] metadata;

}

void
serialnumber_loader(stream* in, tag_type tag, movie_definition* /*m*/)
{
    assert(tag == SWF::SERIALNUMBER); // 41

	in->ensureBytes(26);

    double id = in->read_u32();
    double edition = in->read_u32();
    int major = in->read_u8();
    int minor = in->read_u8();

    boost::uint32_t buildL = in->read_u32();
    boost::uint32_t buildH = in->read_u32();
    boost::uint64_t build = (((boost::uint64_t)buildH) << 32) + buildL;

    boost::uint32_t timestampL = in->read_u32();
    boost::uint32_t timestampH = in->read_u32();
    // This timestamp is number of milliseconds since 1 Jan 1970 (epoch)
    boost::uint64_t timestamp = (((boost::uint64_t)timestampH) << 32) + timestampL;

    std::stringstream ss;
    ss << "SERIALNUMBER: Version " << id << "." << edition << "." << major << "." << minor;
    ss << " - Build " << build;
    ss << " - Timestamp " << timestamp;

    log_msg("%s", ss.str().c_str());

    // attach to movie_definition ?
}

void
reflex_loader(stream* in, tag_type tag, movie_definition* /*m*/)
{
    assert(tag == SWF::REFLEX); // 777

    in->ensureBytes(3);
    boost::uint8_t first = in->read_u8();
    boost::uint8_t second = in->read_u8();
    boost::uint8_t third = in->read_u8();

    IF_VERBOSE_PARSE (
	log_parse(_("  reflex = \"%c%c%c\""), first, second, third);
    );

    log_unimpl(_("REFLEX tag parsed (\"%c%c%c\") but unused"), first, second, third);

}

void
abc_loader(stream* in, tag_type tag, movie_definition* /*m*/)
{
	assert(tag == SWF::DOABC
		|| tag == SWF::DOABCDEFINE); // 72 or 82

	abc_block a;

	if (tag == SWF::DOABCDEFINE)
	{
		// Skip the 'flags' until they are actually used.
		static_cast<void> (in->read_u32());
		std::string name = in->read_string();
		name.c_str();
	}

	//TODO: Move this to execution time so that as_object can be used. bool success = a.read(in);

	log_unimpl(_("%s tag parsed but not yet used"), tag == SWF::DOABC ? "DOABC" : "DOABCDEFINE");
}

} // namespace gnash::SWF::tag_loaders
} // namespace gnash::SWF

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:



// @@ lots of macros here!  It seems that VC6 can't correctly
// handle integer template args, although it's happy to
// compile them?!


class in_stream
{
public:
	const unsigned char*	m_in_data;
	int	m_current_bits;
	int	m_unused_bits;

	in_stream(const unsigned char* data)
		:
		m_in_data(data),
		m_current_bits(0),
		m_unused_bits(0)
	{
	}
};


} // namespace gnash
