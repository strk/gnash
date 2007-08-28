// tag_loaders.cpp:  for Gnash.
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

/* $Id: tag_loaders.cpp,v 1.134 2007/08/28 13:12:38 strk Exp $ */

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
#include "execute_tag.h" // for set_background_color inheritance 
#include "URL.h"
#include "GnashException.h"
#include "video_stream_def.h"
#include "sound_definition.h"

#ifdef HAVE_ZLIB_H
#include <zlib.h>
#endif
#include <map>

namespace gnash {

	// @@ TODO get rid of this; make it the normal mode.
	extern bool s_no_recurse_while_loading;

}

namespace gnash {

// Forward declaration for functions at end of file
//
// Both modify "data" parameter, allocating memory for it.

static void u8_expand(unsigned char* &data, stream* in,
	int sample_count,  // in stereo, this is number of *pairs* of samples
	bool stereo);

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

	StreamAdapter(stream& str)
		:
		s(str)
	{}

	static int readFunc(void* dst, int bytes, void* appdata) 
	{
		StreamAdapter* br = (StreamAdapter*) appdata;
		return br->s.read((char*)dst, bytes);
	}

	static int closeFunc(void* appdata)
	{
		StreamAdapter* br = (StreamAdapter*) appdata;
		delete br;
		return 0; // ok ? or TU_FILE_CLOSE_ERROR ?
	}

public:

	/// Get a tu_file from a gnash::stream
	static std::auto_ptr<tu_file> getFile(stream& str)
	{
		std::auto_ptr<tu_file> ret ( 
			new tu_file (
				new StreamAdapter(str),
				readFunc,
				0, // write_func wf,
				0, //seek_func sf,
				0, //seek_to_end_func ef,
				0, //tell_func tf,
				0, //get_eof_func gef,
				0, //get_err_func ger
				0, // get_stream_size_func gss,
				closeFunc
			)
		);

		return ret;
	}
};

} // anonymous namespace

// ----------------------------------------------------------------------------
// ADPCMDecoder class
// ----------------------------------------------------------------------------

/// ADPCM decoder utilities
//
/// Algo from http://www.circuitcellar.com/pastissues/articles/richey110/text.htm
/// And also Jansen.
/// Here's another reference: http://www.geocities.com/SiliconValley/8682/aud3.txt
/// Original IMA spec doesn't seem to be on the web :(
///
/// TODO: move in it's own file
///
class ADPCMDecoder {

private:

	// Data from Alexis' SWF reference
	static int _index_update_table_2bits[2];
	static int _index_update_table_3bits[4];
	static int _index_update_table_4bits[8];
	static int _index_update_table_5bits[16];

	static int* s_index_update_tables[4];

	// Data from Jansen.  http://homepages.cwi.nl/~jack/
	// Check out his Dutch retro punk songs, heh heh :)
	static const int STEPSIZE_CT = 89;
	static int s_stepsize[STEPSIZE_CT];


	static void doSample(int n_bits, int& sample, int& stepsize_index, int raw_code)
	{
		assert(raw_code >= 0 && raw_code < (1 << n_bits));								
																
		static const int	HI_BIT = (1 << (n_bits - 1));								
		int*	index_update_table = s_index_update_tables[n_bits - 2];							
																
		/* Core of ADPCM. */												
																
		int	code_mag = raw_code & (HI_BIT - 1);									
		bool	code_sign_bit = (raw_code & HI_BIT) ? 1 : 0;								
		int	mag = (code_mag << 1) + 1;	/* shift in LSB (they do this so that pos & neg zero are different)*/	
																
		int	stepsize = s_stepsize[stepsize_index];									
																
		/* Compute the new sample.  It's the predicted value			*/					
		/* (i.e. the previous value), plus a delta.  The delta			*/					
		/* comes from the code times the stepsize.  going for			*/					
		/* something like: delta = stepsize * (code * 2 + 1) >> code_bits	*/					
		int	delta = (stepsize * mag) >> (n_bits - 1);								
		if (code_sign_bit) delta = -delta;										
																
		sample += delta;												
		sample = iclamp(sample, -32768, 32767);										
																
		/* Update our stepsize index.  Use a lookup table. */								
		stepsize_index += index_update_table[code_mag];									
		stepsize_index = iclamp(stepsize_index, 0, STEPSIZE_CT - 1);							
	}

	/* Uncompress 4096 mono samples of ADPCM. */									
	static void doMonoBlock(int16_t** out_data, int n_bits, int sample_count, stream* in, int sample, int stepsize_index)
	{
		/* First sample doesn't need to be decompressed. */								
		sample_count--;													
		*(*out_data)++ = (int16_t) sample;										
																
		while (sample_count--)												
		{														
			int	raw_code = in->read_uint(n_bits);								
			doSample(n_bits, sample, stepsize_index, raw_code);	/* sample & stepsize_index are in/out params */	
			*(*out_data)++ = (int16_t) sample;									
		}														
	}


	/* Uncompress 4096 stereo sample pairs of ADPCM. */									
	static void doStereoBlock(
			int16_t** out_data,	// in/out param
			int n_bits,
			int sample_count,
			stream* in,
			int left_sample,
			int left_stepsize_index,
			int right_sample,
			int right_stepsize_index
			)
	{
		/* First samples don't need to be decompressed. */								
		sample_count--;													
		*(*out_data)++ = (int16_t) left_sample;										
		*(*out_data)++ = (int16_t) right_sample;										
																
		while (sample_count--)												
		{														
			int	left_raw_code = in->read_uint(n_bits);								
			doSample(n_bits, left_sample, left_stepsize_index, left_raw_code);					
			*(*out_data)++ = (int16_t) left_sample;									
																
			int	right_raw_code = in->read_uint(n_bits);								
			doSample(n_bits, right_sample, right_stepsize_index, right_raw_code);					
			*(*out_data)++ = (int16_t) right_sample;									
		}														
	}

public:

	// Utility function: uncompress ADPCM data from in stream to
	// out_data[].	The output buffer must have (sample_count*2)
	// bytes for mono, or (sample_count*4) bytes for stereo.
	static void adpcm_expand(
		unsigned char* &data,
		stream* in,
		int sample_count,	// in stereo, this is number of *pairs* of samples (TODO: why is this signed at all ??)
		bool stereo)
	{
		int16_t* out_data = new int16_t[stereo ? sample_count*2 : sample_count];
		data = reinterpret_cast<unsigned char *>(out_data);

		// Read header.
		in->ensureBytes(1); // nbits
		unsigned int n_bits = in->read_uint(2) + 2;	// 2 to 5 bits (TODO: use unsigned...)


#ifndef GNASH_TRUST_SWF_INPUT

		// bitsPerCompSample is the number of bits for each comp sample
		unsigned int bitsPerCompSample = n_bits;
		if (stereo) bitsPerCompSample *= 2;

		// There's going to be one block every 4096 samples ...
		unsigned int blocksCount = sample_count/4096;

		// ... or fraction
		if ( sample_count%4096 ) ++blocksCount;

		// Of the total samples, all but the first sample in a block are comp
		unsigned int compSamples = sample_count - blocksCount;

		// From every block, a fixed 22 bits will be read (16 of which are the uncomp sample)
		unsigned int fixedBitsPerBlock = 22;

		// Total bits needed from stream
		unsigned long bitsNeeded = (compSamples*bitsPerCompSample) + (fixedBitsPerBlock*blocksCount);

		// Now, we convert this number to bytes...
		unsigned long bytesNeeded = bitsNeeded/8;
		// ... requiring one more if the bits in excess are more then
		//     the ones still available in last byte read 
		unsigned int excessBits = bitsNeeded%8;
		if ( excessBits > 6 ) ++bytesNeeded;

		// Take note of the current position to later verify if we got the 
		// number of required bytes right
		unsigned long prevPosition = in->get_position();

		// We substract 1 byte as the 6 excessive of a byte are already in the stream,
		// and we won't require another one unless more then 6 excessive bits are needed
		// WARNING: this is currently disabled due to a bug in this function often resulting
		//          in reads past the end of the stream
		//in->ensureBytes(bytesNeeded-1);

#endif // GNASH_TRUST_SWF_INPUT

		while (sample_count)
		{
			// Read initial sample & index values.

			int	sample = in->read_sint(16);

			int	stepsize_index = in->read_uint(6);
			assert(STEPSIZE_CT >= (1 << 6));	// ensure we don't need to clamp.

			int	samples_this_block = imin(sample_count, 4096);
			sample_count -= samples_this_block;

			if (stereo == false)
			{
#define DO_MONO(n) doMonoBlock(&out_data, n, samples_this_block, in, sample, stepsize_index)

				switch (n_bits)
				{
				default: assert(0); break;
				case 2: DO_MONO(2); break;
				case 3: DO_MONO(3); break;
				case 4: DO_MONO(4); break;
				case 5: DO_MONO(5); break;
				}
			}
			else
			{
				// Stereo.

				// Got values for left channel; now get initial sample
				// & index for right channel.
				int	right_sample = in->read_sint(16);

				int	right_stepsize_index = in->read_uint(6);
				assert(STEPSIZE_CT >= (1 << 6));	// ensure we don't need to clamp.

#define DO_STEREO(n)					\
		doStereoBlock(				\
			&out_data, n, samples_this_block,	\
			in, sample, stepsize_index,		\
			right_sample, right_stepsize_index)
				
				switch (n_bits)
				{
				default: assert(0); break;
				case 2: DO_STEREO(2); break;
				case 3: DO_STEREO(3); break;
				case 4: DO_STEREO(4); break;
				case 5: DO_STEREO(5); break;
				}
			}
		}

#ifndef GNASH_TRUST_SWF_INPUT
		unsigned long curPos = in->get_position();
		unsigned long bytesRead = curPos - prevPosition;
		if ( bytesRead != bytesNeeded )
		{
			// This would happen if the computation of bytesNeeded doesn't match the current
			// implementation.
			// NOTE That the current implementation seems pretty much bogus as we *often* end
			// up reading past the end of the tag. Once we fix the decoding we shoudl also fix
			// the computation of bytes needed
			log_error("admcp_expand: we expected to read %lu bytes, but we read %lu instead (%ld error)",
				bytesNeeded, bytesRead, bytesNeeded-bytesRead);
			// abort();
		}

		unsigned long endTagPos = in->get_tag_end_position();
		if ( curPos > endTagPos )
		{
			// This happens when our decoder reads past the end of the tag.
			// In general, we should aborth parsing of the current tag when this happens,
			// anyway, it seems that *some* sound can be heard nonetheless so we keep going.
			log_error("admcp_expand: read past tag boundary: current position: %lu, end of tag position: %lu (overflow: %lu bytes)",
				curPos, endTagPos, curPos-endTagPos);

#if 0
			log_debug("      stereo:%d, sample_count:%u, compressedSamples:%d, bitsPerCompSample:%u, "
				"blocksCount:%u, bitsPerBlock:%u, bitsNeeded:%lu, excessBits:%u, bytesNeeded:%lu, bytesLeft:%lu",
				stereo, sample_count, compSamples, bitsPerCompSample, blocksCount, fixedBitsPerBlock,
				bitsNeeded, excessBits, bytesNeeded, in->get_tag_end_position()-in->get_position());
#endif
		}
#endif // GNASH_TRUST_SWF_INPUT

	}


};

int ADPCMDecoder::_index_update_table_2bits[2] = { -1,  2 };
int ADPCMDecoder::_index_update_table_3bits[4] = { -1, -1,  2,  4 };
int ADPCMDecoder::_index_update_table_4bits[8] = { -1, -1, -1, -1,  2,  4,  6,  8 };
int ADPCMDecoder::_index_update_table_5bits[16] = { -1, -1, -1, -1, -1, -1, -1, -1, 1,  2,  4,  6,  8, 10, 13, 16 };

int* ADPCMDecoder::s_index_update_tables[4] = {
	ADPCMDecoder::_index_update_table_2bits,
	ADPCMDecoder::_index_update_table_3bits,
	ADPCMDecoder::_index_update_table_4bits,
	ADPCMDecoder::_index_update_table_5bits
};

int ADPCMDecoder::s_stepsize[STEPSIZE_CT] = {
	7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
	19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
	50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
	130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
	337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
	876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
	2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
	5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

// ----------------------------------------------------------------------------
// END OF ADPCMDecoder class
// ----------------------------------------------------------------------------


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

/// SWF Tag SetBackgroundColor (9)
//
/// TODO: Move in it's own SetBackgroundColorTag files
///
class set_background_color : public execute_tag
{
private:
    rgba	m_color;

public:
    void	execute(sprite_instance* m) const
	{
	    float	current_alpha = m->get_background_alpha();
	    rgba newcolor = m_color; // to avoid making m_color mutable
	    newcolor.m_a = frnd(current_alpha * 255.0f);
	    m->set_background_color(newcolor);
	}

    void	execute_state(sprite_instance* m) const
	{
	    execute(m);
	}

    void	read(stream* in)
	{
	    m_color.read_rgb(in);

	    IF_VERBOSE_PARSE
	    (
		log_parse(_("  set_background_color: (%d %d %d)"),
			  m_color.m_r, m_color.m_g, m_color.m_b);
	    );
	}
};


// SWF Tag SetBackgroundColor (9)
void
set_background_color_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == SWF::SETBACKGROUNDCOLOR); // 9
    assert(m);

    set_background_color* t = new set_background_color;
    t->read(in);

    m->add_execute_tag(t);
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

    std::auto_ptr<jpeg::input> j_in;

    try
    {
	std::auto_ptr<tu_file> ad( StreamAdapter::getFile(*in) );
	//  transfer ownerhip to the jpeg::input
        j_in.reset(jpeg::input::create_swf_jpeg2_header_only(ad.release(), true));

        //j_in.reset(jpeg::input::create_swf_jpeg2_header_only(in->get_underlying_stream()));
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

    uint16_t	character_id = in->read_u16();

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

    uint16_t	character_id = in->read_u16();

    IF_VERBOSE_PARSE
    (
	log_parse(_("  define_bits_jpeg2_loader: charid = %d pos = %lx"),
		  character_id, in->get_position());
    );

    //
    // Read the image data.
    //

    if (m->get_create_bitmaps() == DO_LOAD_BITMAPS)
    {
	std::auto_ptr<tu_file> ad( StreamAdapter::getFile(*in) );
	std::auto_ptr<image::rgb> im ( image::read_jpeg(ad.get()) );
	//std::auto_ptr<image::rgb> im ( image::read_jpeg(in->get_underlying_stream()) );

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
	log_swferror(_("inflate_wrapper() inflateInit() returned %d"), err);
	);
	return;
    }

    uint8_t buf[1];

    for (;;) {
	// Fill a one-byte (!) buffer.
	// TODO: sub-optimal, read_u8 also calls align needlessly
	buf[0] = in.read_u8();
	d_stream.next_in = &buf[0];
	d_stream.avail_in = 1;

	err = inflate(&d_stream, Z_SYNC_FLUSH);
	if (err == Z_STREAM_END) break;
	if (err != Z_OK)
	{
	    IF_VERBOSE_MALFORMED_SWF(
	    log_swferror(_("inflate_wrapper() inflate() returned %d"), err);
	    );
	    break;
	}
    }

    err = inflateEnd(&d_stream);
    if (err != Z_OK)
    {
	    log_error(_("inflate_wrapper() inflateEnd() return %d"), err);
    }
}
#endif // HAVE_ZLIB_H


// loads a define_bits_jpeg3 tag. This is a jpeg file with an alpha
// channel using zlib compression.
void
define_bits_jpeg3_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == SWF::DEFINEBITSJPEG3); // 35

    uint16_t	character_id = in->read_u16();

    IF_VERBOSE_PARSE
    (
	log_parse(_("  define_bits_jpeg3_loader: charid = %d pos = %lx"),
		  character_id, in->get_position());
    );

    uint32_t	jpeg_size = in->read_u32();
    uint32_t	alpha_position = in->get_position() + jpeg_size;

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
	std::auto_ptr<tu_file> ad( StreamAdapter::getFile(*in) );
	std::auto_ptr<image::rgba> im( image::read_swf_jpeg3(ad.get()) );
	//std::auto_ptr<image::rgba> im( image::read_swf_jpeg3(in->get_underlying_stream()) );

	// Read alpha channel.
	in->set_position(alpha_position);

	int	buffer_bytes = im->m_width * im->m_height;
	uint8_t*      buffer = new uint8_t[buffer_bytes];

	inflate_wrapper(*in, buffer, buffer_bytes);

	for (int i = 0; i < buffer_bytes; i++)
	{
	    im->m_data[4*i+3] = buffer[i];
	}

	delete [] buffer;

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

    uint16_t	character_id = in->read_u16();
    uint8_t	bitmap_format = in->read_u8();	// 3 == 8 bit, 4 == 16 bit, 5 == 32 bit
    uint16_t	width = in->read_u16();
    uint16_t	height = in->read_u16();

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
		boost::scoped_array<uint8_t> buffer ( new uint8_t[buffer_bytes] );

		inflate_wrapper(*in, buffer.get(), buffer_bytes);
		assert(in->get_position() <= in->get_tag_end_position());

		uint8_t* color_table = buffer.get();

		for (int j = 0; j < height; j++)
		{
		    uint8_t*	image_in_row = buffer.get() + color_table_size * 3 + j * pitch;
		    uint8_t*	image_out_row = image::scanline(image.get(), j);
		    for (int i = 0; i < width; i++)
		    {
			uint8_t	pixel = image_in_row[i * bytes_per_pixel];
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
		boost::scoped_array<uint8_t> buffer ( new uint8_t[buffer_bytes] );

		inflate_wrapper(*in, buffer.get(), buffer_bytes);
		assert(in->get_position() <= in->get_tag_end_position());

		for (int j = 0; j < height; j++)
		{
		    uint8_t*	image_in_row = buffer.get() + j * pitch;
		    uint8_t*	image_out_row = image::scanline(image.get(), j);
		    for (int i = 0; i < width; i++)
		    {
			uint16_t	pixel = image_in_row[i * 2] | (image_in_row[i * 2 + 1] << 8);

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
		boost::scoped_array<uint8_t> buffer ( new uint8_t[buffer_bytes] );

		inflate_wrapper(*in, buffer.get(), buffer_bytes);
		assert(in->get_position() <= in->get_tag_end_position());

		// Need to re-arrange ARGB into RGB.
		for (int j = 0; j < height; j++)
		{
		    uint8_t*	image_in_row = buffer.get() + j * pitch;
		    uint8_t*	image_out_row = image::scanline(image.get(), j);
		    for (int i = 0; i < width; i++)
		    {
			uint8_t a = image_in_row[i * 4 + 0];
			uint8_t r = image_in_row[i * 4 + 1];
			uint8_t g = image_in_row[i * 4 + 2];
			uint8_t b = image_in_row[i * 4 + 3];
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
		boost::scoped_array<uint8_t> buffer ( new uint8_t[buffer_bytes] );

		inflate_wrapper(*in, buffer.get(), buffer_bytes);
		assert(in->get_position() <= in->get_tag_end_position());

		uint8_t* color_table = buffer.get();

		for (int j = 0; j < height; j++)
		{
		    uint8_t*	image_in_row = buffer.get() + color_table_size * 4 + j * pitch;
		    uint8_t*	image_out_row = image::scanline(image.get(), j);
		    for (int i = 0; i < width; i++)
		    {
			uint8_t	pixel = image_in_row[i * bytes_per_pixel];
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
		boost::scoped_array<uint8_t> buffer ( new uint8_t[buffer_bytes] );

		inflate_wrapper(*in, buffer.get(), buffer_bytes);
		assert(in->get_position() <= in->get_tag_end_position());

		for (int j = 0; j < height; j++)
		{
		    uint8_t*	image_in_row = buffer.get() + j * pitch;
		    uint8_t*	image_out_row = image::scanline(image.get(), j);
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

	    }
	    else if (bitmap_format == 5)
	    {
		// 32 bits / pixel, input is ARGB format

		inflate_wrapper(*in, image->m_data, width * height * 4);
		assert(in->get_position() <= in->get_tag_end_position());

		// Need to re-arrange ARGB into RGBA.
		for (int j = 0; j < height; j++)
		{
		    uint8_t*	image_row = image::scanline(image.get(), j);
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
	   || tag == SWF::DEFINESHAPE3);

    uint16_t	character_id = in->read_u16();
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
    assert(tag == SWF::DEFINEMORPHSHAPE); // 46
    uint16_t character_id = in->read_u16();

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

    uint16_t font_id = in->read_u16();

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

    uint16_t font_id = in->read_u16();

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

    uint16_t font_id = in->read_u16();

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

void	end_loader(stream* in, tag_type tag, movie_definition* /*m*/)
{
    assert(tag == SWF::END); // 0
    assert(in->get_position() == in->get_tag_end_position());
}

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
	uint16_t	id = in->read_u16();
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
	uint16_t	id = in->read_u16();
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

    uint16_t	character_id = in->read_u16();

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

    uint16_t	character_id = in->read_u16();

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
static void sound_expand(stream *in, sound_handler::format_type &format,
	bool sample_16bit, bool stereo, unsigned int &sample_count,
	unsigned char* &data, unsigned &data_bytes);

// Common data
static int	s_sample_rate_table[] = { 5512, 11025, 22050, 44100 };

// @@ There are two sets of code to decode/expand/byteswap audio here.
// @@ There should be one (search for ADPCM).

// Load a DefineSound tag.
void
define_sound_loader(stream* in, tag_type tag, movie_definition* m)
{
	assert(tag == SWF::DEFINESOUND); // 14

	sound_handler* handler = get_sound_handler();

	in->ensureBytes(2+4+1+4); // character id + flags + sample count

	uint16_t	character_id = in->read_u16();

	sound_handler::format_type	format = static_cast<sound_handler::format_type>(in->read_uint(4));
	sound_handler::format_type	orgFormat = format;
	int	sample_rate = in->read_uint(2);	// multiples of 5512.5
	bool	sample_16bit = in->read_bit(); 
	bool	stereo = in->read_bit(); 

	unsigned int	sample_count = in->read_u32();

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

	    unsigned char *data; // Expanded audio data ready for playing

	    // First it is the amount of data from file,
	    // then the amount allocated at *data (it may grow)
            unsigned data_bytes = in->get_tag_end_position() - in->get_position();

	    // sound_expand allocates storage for data[].
	    // and modifies 3 parameters: format, data and data_bytes.
	    sound_expand(in, format, sample_16bit, stereo, sample_count, data, data_bytes);

	    // Store all the data in a SoundInfo object
	    std::auto_ptr<SoundInfo> sinfo;
	    sinfo.reset(new SoundInfo(format, orgFormat, stereo, s_sample_rate_table[sample_rate], sample_count, sample_16bit));

	    // Stores the sounddata in the soundhandler, and the ID returned
		// can be used to starting, stopping and deleting that sound
	    int	handler_id = handler->create_sound(data, data_bytes, sinfo);

	    if (handler_id >= 0)
	    {
		sound_sample* sam = new sound_sample(handler_id);
		m->add_sound_sample(character_id, sam);
	    }

	    delete [] data;
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


// Load a StartSound tag.
void
start_sound_loader(stream* in, tag_type tag, movie_definition* m)
{
    sound_handler* handler = get_sound_handler();

    assert(tag == SWF::STARTSOUND); // 15 

    uint16_t	sound_id = in->read_u16();

    sound_sample* sam = m->get_sound_sample(sound_id);
    if (sam)
    {
	start_sound_tag*	sst = new start_sound_tag();
	sst->read(in, tag, m, sam);

	IF_VERBOSE_PARSE
	(
	    log_parse(_("start_sound tag: id=%d, stop = %d, loop ct = %d"),
		      sound_id, int(sst->m_stop_playback), sst->m_loop_count);
	);
    }
    else
    {
	if (handler)
	{
	    IF_VERBOSE_MALFORMED_SWF(
		log_swferror(_("start_sound_loader: sound_id %d is not defined"), sound_id);
	    );
	}
    }
}

// Load a SoundStreamHead(2) tag.
void
sound_stream_head_loader(stream* in, tag_type tag, movie_definition* m)
{
    sound_handler* handler = get_sound_handler();

    // 18 || 45
    assert(tag == SWF::SOUNDSTREAMHEAD || tag == SWF::SOUNDSTREAMHEAD2);

    // If we don't have a sound_handler registered stop here
    if (!handler) return;

    // FIXME:
    // no character id for soundstreams... so we make one up...
    // This only works if there is only one stream in the movie...
    // The right way to do it is to make seperate structures for streams
    // in movie_def_impl.

    // extract garbage data
    int	garbage = in->read_uint(8);

    sound_handler::format_type format = static_cast<sound_handler::format_type>(in->read_uint(4));
	sound_handler::format_type orgFormat = format;
    int sample_rate = in->read_uint(2);	// multiples of 5512.5
    bool sample_16bit = in->read_bit(); 
    bool stereo = in->read_bit(); 

    // checks if this is a new streams header or just one in the row
    if (format == 0 && sample_rate == 0 && !sample_16bit && !stereo) return;

    unsigned int sample_count = in->read_u16();
	int latency = 0;
    if (format == sound_handler::FORMAT_MP3) {
		latency = in->read_s16();
		garbage = in->read_uint(16);
	}

    IF_VERBOSE_PARSE
    (
	log_parse(_("sound stream head: format=%d, rate=%d, 16=%d, stereo=%d, ct=%d"),
		  int(format), sample_rate, int(sample_16bit), int(stereo), sample_count);
    );

   // Wot about reading the sample_count samples?

    // Ask sound_handler it to init this sound.
    int	data_bytes = 0;

    if (! (sample_rate >= 0 && sample_rate <= 3))
    {
	IF_VERBOSE_MALFORMED_SWF(
	    log_swferror(_("Bad sound sample rate %d read from SWF header"),
			 sample_rate);
	    );
	return;
    }

    // Tell create_sound what format it will be receiving, in case it cares
    // at this stage.
    switch (format) {
    case sound_handler::FORMAT_ADPCM:
    case sound_handler::FORMAT_RAW:
    case sound_handler::FORMAT_UNCOMPRESSED:
	format = sound_handler::FORMAT_NATIVE16;
	break;
    // Shut fussy compilers up...
    case sound_handler::FORMAT_MP3:
    case sound_handler::FORMAT_NELLYMOSER:
    case sound_handler::FORMAT_NATIVE16:
    case sound_handler::FORMAT_NELLYMOSER_8HZ_MONO:
	break;
    }

	// Store all the data in a SoundInfo object
	std::auto_ptr<SoundInfo> sinfo;
	sinfo.reset(new SoundInfo(format, orgFormat, stereo, s_sample_rate_table[sample_rate], sample_count, sample_16bit));

	// Stores the sounddata in the soundhandler, and the ID returned
	// can be used to starting, stopping and deleting that sound
	int	handler_id = handler->create_sound(NULL, data_bytes, sinfo);

    m->set_loading_sound_stream_id(handler_id);
}


// Load a SoundStreamBlock tag.
void
sound_stream_block_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == SWF::SOUNDSTREAMBLOCK); // 19

    sound_handler* handler = get_sound_handler();

    // If we don't have a sound_handler registered stop here
    if (!handler) return;

	// Get the ID of the sound stream currently being loaded
    int handle_id = m->get_loading_sound_stream_id();

	// Get the SoundInfo object that contains info about the sound stream.
	// Ownership of the object is in the soundhandler
	SoundInfo* sinfo = handler->get_sound_info(handle_id);

    // If there is no SoundInfo something is wrong...
    if (!sinfo) return;

    sound_handler::format_type format = sinfo->getOrgFormat();
    unsigned int sample_count = sinfo->getSampleCount();

	// discard garbage data if format is MP3
    if (format == sound_handler::FORMAT_MP3) in->skip_bytes(4);

    unsigned char *data;	// Storage is allocated by sound_expand()
    unsigned int data_bytes = in->get_tag_end_position() - in->get_position();

    sound_expand(in, format,
		 sinfo->is16bit(), sinfo->isStereo(), sample_count,
		 data, data_bytes);
    // "format" now reflects what we hand(ed) to the sound drivers.
    // "data_bytes" now reflects the size of the uncompressed data.

    // Fill the data on the apropiate sound, and receives the starting point
    // for later "start playing from this frame" events.
    long start = handler->fill_stream_data(data, data_bytes, sample_count, handle_id);

    delete [] data;

    start_stream_sound_tag*	ssst = new start_stream_sound_tag();
    ssst->read(m, handle_id, start);
}

// sound_expand: Expand audio data to 16-bit host endian.
//
// This modifies three of its parameters:
// On entry, "format" is the format of the original data. If this routine
// expands that to 16-bit native-endian, it will also modify "format" to
// FORMAT_NATIVE16. Otherwise it leaves it alone (MP3 and NELLYMOSER).
//
// Storage for "data" is allocated here, and the the "data" pointer is modified.
//
// On entry, data_bytes is the amount of sound data to be read from "in";
// on exit it reflects the number of bytes that "data" now points to.
static void
sound_expand(stream *in, sound_handler::format_type &format,
	bool sample_16bit, bool stereo, unsigned int &sample_count,
	unsigned char* &data, unsigned int &data_bytes)
{

    // Make sure that an unassigned pointer cannot get through
    data = NULL;

    switch (format) {

    case sound_handler::FORMAT_ADPCM:
      {
	//log_debug("ADPCM format");
	// Uncompress the ADPCM before handing data to host.
	if (sample_count == 0) sample_count = data_bytes / ( stereo ? 4 : 2 );
	ADPCMDecoder::adpcm_expand(data, in, sample_count, stereo);
	data_bytes = sample_count * (stereo ? 4 : 2);
	format = sound_handler::FORMAT_NATIVE16;
	break;
      }
    case sound_handler::FORMAT_RAW:
	//log_debug("RAW format");
	// 8- or 16-bit mono or stereo host-endian audio
	// Convert to 16-bit host-endian
	if (sample_16bit) {
	    // FORMAT_RAW 16-bit is exactly what we want!
	    in->ensureBytes(data_bytes); 
	    data = new unsigned char[data_bytes];
	    in->read((char *)data, data_bytes);
	} else {
	    // Convert 8-bit signed to 16-bit range
	    // Allocate as many shorts as there are samples
	    if (sample_count == 0) sample_count = data_bytes / (stereo ? 2 : 1);
	    u8_expand(data, in, sample_count, stereo);
		data_bytes = sample_count * (stereo ? 4 : 2);
	}
	format = sound_handler::FORMAT_NATIVE16;
	break;

    case sound_handler::FORMAT_UNCOMPRESSED:
	//log_debug("UNCOMPRESSED format");
	// 8- or 16-bit mono or stereo little-endian audio
	// Convert to 16-bit host-endian.
	if (!sample_16bit)
	{
	    // Convert 8-bit signed to 16-bit range
	    // Allocate as many shorts as there are 8-bit samples
	    if (sample_count == 0) sample_count = data_bytes / (stereo ? 2 : 1);
	    u8_expand(data, in, sample_count, stereo);
		data_bytes = sample_count * (stereo ? 4 : 2);

	} else {
	    // Read 16-bit data into buffer
	    in->ensureBytes(data_bytes); 
	    data = new unsigned char[data_bytes];
	    in->read((char *)data, data_bytes);

	    // Convert 16-bit little-endian data to host-endian.

	    // Runtime detection of host endianness costs almost
	    // nothing and is less of a continual maintenance headache
	    // than compile-time detection.
	    union u {
	    	uint16_t s;
		struct {
		    uint8_t c0;
		    uint8_t c1;
		} c;
	    } u = { 0x0001 };

	    switch (u.c.c0) {
	    case 0x01:	// Little-endian host: sample is already native.
		break;
	    case 0x00:  // Big-endian host
	        // Swap sample bytes to get big-endian format.
		assert(data_bytes & 1 == 0);
	        for (unsigned i = 0; i < data_bytes; i+=2)
	        {
		    swap(&data[i], &data[i+1]);
	        }
		break;
	    default:	// Impossible
		log_error(_("Host endianness not detected in define_sound_loader"));
		// Just carry on anyway...
	    }
	}
	format = sound_handler::FORMAT_NATIVE16;
	break;

    case sound_handler::FORMAT_MP3:
	//log_debug("MP3 format");
	// Decompressed elsewhere
	in->ensureBytes(data_bytes); 
	data = new unsigned char[data_bytes];
	in->read((char *)data, data_bytes);
	break;

    case sound_handler::FORMAT_NELLYMOSER_8HZ_MONO:
    case sound_handler::FORMAT_NELLYMOSER:
	//log_debug("NELLYMOSER format");
	// One day...
	in->ensureBytes(data_bytes); 
	in->skip_bytes(data_bytes);
	data = NULL;
	break;

    // This is impossible as an input but stops fussy compilers
    // complaining about unhandled enum values.
    case sound_handler::FORMAT_NATIVE16:
	//log_debug("NATIVE16 format");
	break;
    }
}

void
define_video_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == SWF::DEFINEVIDEOSTREAM); // 60
    uint16_t character_id = in->read_u16();

    video_stream_definition* ch = new video_stream_definition(character_id);
    ch->read(in, tag, m);

    m->add_character(character_id, ch);

}

void
video_loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == SWF::VIDEOFRAME); // 61

    uint16_t character_id = in->read_u16();
    character_def* chdef = m->get_character_def(character_id);

    assert ( dynamic_cast<video_stream_definition*> (chdef) );
    video_stream_definition* ch = static_cast<video_stream_definition*> (chdef);
    assert(ch != NULL);

    ch->read(in, tag, m);
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
		    "(or application?). Anyway Gnash won't care; "
		    "use white/black listing in your .gnashrc instead"));
    }

    // TODO: attach info to movie_definition
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

    std::string serial;
    in->read_string_with_length(in->get_tag_length(), serial);

    IF_VERBOSE_PARSE (
	log_parse(_("  serialnumber = [[\n%s\n]]"), serial.c_str());
    );

    log_msg(_("SERIALNUMBER: %s"), serial.c_str());

    // attach to movie_definition ?
}

void
reflex_loader(stream* in, tag_type tag, movie_definition* /*m*/)
{
    assert(tag == SWF::REFLEX); // 777

    in->ensureBytes(3);
    uint8_t first = in->read_u8();
    uint8_t second = in->read_u8();
    uint8_t third = in->read_u8();

    IF_VERBOSE_PARSE (
	log_parse(_("  reflex = \"%c%c%c\""), first, second, third);
    );

    log_unimpl(_("REFLEX tag parsed (\"%c%c%c\") but unused"), first, second, third);

}


} // namespace gnash::SWF::tag_loaders
} // namespace gnash::SWF

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

// Expand ADPCM, 8-bit and non-host-endian 16-bit audio to 16-bit host-endian
//
// Provides:
//
// void	adpcm_expand(unsigned char* &data, stream* in,
//	int sample_count, // in stereo, this is number of *pairs* of samples
//	bool stereo)
//
//	Uncompress ADPCM data from in stream to out_data[].
//	The output buffer must have (sample_count*2) bytes for mono,
//	or (sample_count*4) bytes for stereo.
//
// This code has wandered round the Gnash source tree and has still not found
// its proper home yet.


//
// Unsigned 8-bit expansion (128 is silence)
//
// u8_expand allocates the memory for its "data" pointer.
//

static void u8_expand(
	unsigned char * &data,
	stream* in,
	int sample_count,	// in stereo, this is number of *pairs* of samples
	bool stereo)
{
	unsigned total_samples = stereo ? sample_count*2 : sample_count;

	in->ensureBytes(total_samples); 

	boost::scoped_array<uint8_t> in_data ( new uint8_t[total_samples] );
	int16_t	*out_data = new int16_t[total_samples];

	in->read((char *)in_data.get(), total_samples); // Read 8-bit samples

	// Convert 8-bit to 16
	uint8_t *inp = in_data.get();
	int16_t *outp = out_data;
	for (unsigned i=total_samples; i>0; i--) {
		*outp++ = ((int16_t)(*inp++) - 128) * 256;
	}
	
	data = (unsigned char *)out_data;
}


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
