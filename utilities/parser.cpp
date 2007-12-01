// parser.cpp:  Flash movie parser (gparser command), for Gnash.
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

/* $Id: parser.cpp,v 1.45 2007/12/01 23:42:39 strk Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <cstdio>

extern "C"{
	#include <unistd.h>
#ifdef HAVE_GETOPT_H
	#include <getopt.h>
#endif
#ifndef __GNUC__
	extern int optind, getopt(int, char *const *, const char *);
#endif
}

#ifdef ENABLE_NLS
#include <locale.h>
#endif

#include "gettext.h"
#include "tu_file.h"
#include "zlib_adapter.h"
#include "image.h"
#include "jpeg.h"

#include "stream.h"
#include "log.h"
#include "gnash.h"
#include "rc.h"
#include "debugger.h"

#include <map>

#define TWIPS_TO_PIXELS(x) ((x) / 20.f)
#define PIXELS_TO_TWIPS(x) ((x) * 20.f)

const char *GPARSE_VERSION = "1.0";

bool gofast = false;		// FIXME: this flag gets set based on
				// an XML message written using
				// SendCommand(""). This way a movie
				// can optimize its own performance
				// when needed,

extern int xml_fd;		// FIXME: this is the file descriptor
				// from XMLSocket::connect(). This
				// needs to be propogated up through
				// the layers properly, but first I
				// want to make sure it all works.

using gnash::stream;
using gnash::log_msg;
//using namespace std;
using namespace gnash;

static void usage (const char *);

namespace {
gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
#ifdef USE_DEBUGGER
gnash::Debugger& debugger = gnash::Debugger::getDefaultInstance();
#endif
}

namespace parser
{
static int ident = 0;
static int current_frame = 0;
//std::auto_ptr<tu_file> out;

typedef void (*loader_function)(stream* input, int tag_type);
static std::map<int, loader_function> tag_loaders;
  
void
register_tag_loader(int tag_type, loader_function lf)
{
    assert(lf != NULL);
    bool inserted = tag_loaders.insert(std::make_pair(tag_type, lf)).second;
    assert(inserted);
}
  
// parse a matrix
struct matrix
{
    static float m_[2][3];
    static bool has_scale, has_rotate;
    static void parse(stream* in)
	{
	    in->align();
      
	    memset(&m_[0], 0, sizeof(m_));
	    m_[0][0] = 1;
	    m_[1][1] = 1;
      
	    bool has_scale = in->read_bit();
	    if (has_scale) {
		int	scale_nbits = in->read_uint(5);
		m_[0][0] = in->read_sint(scale_nbits) / 65536.0f;
		m_[1][1] = in->read_sint(scale_nbits) / 65536.0f;
	    }
	    bool has_rotate = in->read_bit();
	    if (has_rotate) {
		int	rotate_nbits = in->read_uint(5);
		m_[1][0] = in->read_sint(rotate_nbits) / 65536.0f;
		m_[0][1] = in->read_sint(rotate_nbits) / 65536.0f;
	    }
	    
	    int	translate_nbits = in->read_uint(5);
	    if (translate_nbits > 0) {
		m_[0][2] = (float) in->read_sint(translate_nbits);
		m_[1][2] = (float) in->read_sint(translate_nbits);
	    }
	}
    static void write()
	{
	    ident++;
	    log_msg(_("has_scale = %d, has_rotate = %d\n"), has_scale, has_rotate);
	    log_msg("| %4.4f %4.4f %4.4f |\n", m_[0][0], m_[0][1], TWIPS_TO_PIXELS(m_[0][2]));
	    log_msg("| %4.4f %4.4f %4.4f |\n", m_[1][0], m_[1][1], TWIPS_TO_PIXELS(m_[1][2]));
	    ident--;
	}
    
};
float matrix::m_[2][3];
bool matrix::has_scale, matrix::has_rotate;

struct rect
{
    static uint32_t x_min,x_max,y_min,y_max;
    static void parse(stream* in)
	{
	    in->align();
	    int	nbits = in->read_uint(5);
	    x_min = in->read_sint(nbits);
	    x_max = in->read_sint(nbits);
	    y_min = in->read_sint(nbits);
	    y_max = in->read_sint(nbits);
	}
    static void write()
	{
	    ident++;
	    log_msg(_("x_min: %i, x_max: %i,	width: %i twips, %4.0f pixels\n"), x_min, x_max, x_max - x_min, TWIPS_TO_PIXELS(x_max - x_min));
	    log_msg(_("y_min: %i, y_max: %i, height: %i twips, %4.0f pixels\n"), y_min, y_max, y_max - y_min, TWIPS_TO_PIXELS(y_max - y_min));
	    ident--;
	}
};
uint32_t rect::x_min;
uint32_t rect::y_min;
uint32_t rect::x_max;
uint32_t rect::y_max;

struct rgb
{
    static uint8_t m_r, m_g, m_b;
    static void parse(stream* in)
	{
	    m_r = in->read_u8();
	    m_g = in->read_u8();
	    m_b = in->read_u8();
	}
    static void write()
	{
	    ident++;
	    log_msg(_("rgb: %d %d %d \n"), m_r, m_g, m_b);
	    ident--;
	}
};
uint8_t rgb::m_r;
uint8_t rgb::m_g;
uint8_t rgb::m_b;

struct rgba
{
    static uint8_t m_r, m_g, m_b, m_a;
    static void parse(stream* in)
	{
	    m_r = in->read_u8();
	    m_g = in->read_u8();
	    m_b = in->read_u8();
	    m_a = in->read_u8();
	}
    static void write()
	{
	    ident++;
	    log_msg(_("rgba: %d %d %d %d\n"), m_r, m_g, m_b, m_a);
	    ident--;
	}
};
uint8_t rgba::m_r;
uint8_t rgba::m_g;
uint8_t rgba::m_b;
uint8_t rgba::m_a;

struct cxform
{
    static float m_[4][2];
    static bool has_add, has_mult;
    
    static void parse_rgb(stream* in)
	{
	    in->align();
	    
	    bool has_add = in->read_bit();
	    bool has_mult = in->read_bit();
	    int	nbits = in->read_uint(4);
	    
	    if (has_mult) {
		m_[0][0] = in->read_sint(nbits) / 255.0f;
		m_[1][0] = in->read_sint(nbits) / 255.0f;
		m_[2][0] = in->read_sint(nbits) / 255.0f;
		m_[3][0] = 1;
	    } else {
		for (int i = 0; i < 4; i++) { m_[i][0] = 1; }
	    }
	    if (has_add) {
		m_[0][1] = (float) in->read_sint(nbits);
		m_[1][1] = (float) in->read_sint(nbits);
		m_[2][1] = (float) in->read_sint(nbits);
		m_[3][1] = 1;
	    } else {
		for (int i = 0; i < 4; i++) {
		    m_[i][1] = 0;
		}
	    }
	}
    static void parse_rgba(stream* in)
	{
	    in->align();
	    
	    bool has_add = in->read_bit();
	    bool has_mult = in->read_bit();
	    int	nbits = in->read_uint(4);
	    
	    if (has_mult) {
		m_[0][0] = in->read_sint(nbits) / 255.0f;
		m_[1][0] = in->read_sint(nbits) / 255.0f;
		m_[2][0] = in->read_sint(nbits) / 255.0f;
		m_[3][0] = in->read_sint(nbits) / 255.0f;
	    } else {
		for (int i = 0; i < 4; i++) {
		    m_[i][0] = 1;
		}
	    }
	    if (has_add) {
		m_[0][1] = (float) in->read_sint(nbits);
		m_[1][1] = (float) in->read_sint(nbits);
		m_[2][1] = (float) in->read_sint(nbits);
		m_[3][1] = (float) in->read_sint(nbits);
	    } else {
		for (int i = 0; i < 4; i++) {
		    m_[i][1] = 0;
		}
	    }
	}
    static void write()
	{
	    ident++;
	    log_msg(_("cxform:\n"));
	    log_msg(_("has_add = %d, has_mult = %d\n"), has_add, has_mult);
	    log_msg("| %4.4f %4.4f |\n", m_[0][0], m_[0][1]);
	    log_msg("| %4.4f %4.4f |\n", m_[1][0], m_[1][1]);
	    log_msg("| %4.4f %4.4f |\n", m_[2][0], m_[2][1]);
	    log_msg("| %4.4f %4.4f |\n", m_[3][0], m_[3][1]);
	    ident--;
	}
};
float cxform::m_[4][2];
bool cxform::has_add;
bool cxform::has_mult;

// tag 0
void parse_end_movie(stream* /* input */, int tag_type)
{
    assert(tag_type == 0);
    ident--;
    log_msg("\n");
    log_msg(_("Movie ended\n\n"));
}

// tag 1
void parse_show_frame(stream* /* input */, int tag_type)
{
    assert(tag_type == 1);
    ident--;
    current_frame++;
    log_msg("\n");
    log_msg("show frame %i\n\n", current_frame);
    ident++;
}

// tag 2, 22, 32
void parse_define_shape123(stream* input, int tag_type)
{
    assert(tag_type == 2 || tag_type == 22 || tag_type == 32);
    if(tag_type == 2) {
	log_msg("define_shape:\n");
    }
    if(tag_type == 22) {
	log_msg("define_shape2:\n");
    }
    if(tag_type == 32) {
	log_msg("define_shape3:\n");
    }
    
    ident++;
    log_msg("character ID: %i\n", input->read_u16());
    ident--;
}

// tag 4, 26
void parse_place_object12(stream* input, int tag_type)
{
    assert(tag_type == 4 || tag_type == 26);
    
    if (tag_type == 4) {
	log_msg("place_object:\n");
	ident++;
	log_msg("character ID: %i\n",input->read_u16());
	log_msg("depth: %i\n",input->read_u16());
	log_msg("matrix:\n");
	matrix::parse(input);
	matrix::write();
	
	if (input->get_position() < input->get_tag_end_position()) {
	    log_msg("color transform:\n");
	    cxform::parse_rgb(input);
	    cxform::write();
	}
	ident--;
    } else if (tag_type == 26) {
	input->align();
	
	log_msg("place_object2:\n");
	ident++;
	
	bool	has_actions = input->read_bit();
	bool	has_clip_depth = input->read_bit();
	bool	has_name = input->read_bit();
	bool	has_ratio = input->read_bit();
	bool	has_cxform = input->read_bit();
	bool	has_matrix = input->read_bit();
	bool	has_char = input->read_bit();
	bool	flag_move = input->read_bit();
	
	UNUSED(has_actions);
	
	log_msg("depth: %i",input->read_u16());
	
	if (has_char) {
	    log_msg("character ID: %i",input->read_u16());
	}
	if (has_matrix) {
	    log_msg("matrix:");
	    matrix::parse(input);
	    matrix::write();
	}
	if (has_cxform) {
	    log_msg("color transform:");
	    cxform::parse_rgba(input);
	    cxform::write();
	}			
	if (has_ratio) {
	    log_msg("ratio: %i",input->read_u16());
	}			
	if (has_name) {
	    log_msg("name: %s",input->read_string());
	}
	if (has_clip_depth) {
	    log_msg("clipdepth: %i",input->read_u16());
	}			
	if (has_clip_depth) {
	    log_msg("has_actions: to be implemented");
	}
	
	if (has_char == true && flag_move == true) {
	    log_msg("replacing a character previously at this depth");
	} else if (has_char == false && flag_move == true) {
	    log_msg("moving a character previously at this depth");
	} else if (has_char == true && flag_move == false) {
	    log_msg("placing a character first time at this depth");
	}
	
	ident--;
    }
}

// tag 5, 28
void parse_remove_object12(stream* input, int tag_type)
{
    assert(tag_type == 5 || tag_type == 28);
    if (tag_type==5) {
	log_msg("remove_object");
	ident++;
	log_msg("character ID: %i", input->read_u16());
	log_msg("depth: %i", input->read_u16());
	ident--;
    }
    if (tag_type==28) {
	log_msg("remove_object_2");
	ident++;
	log_msg("depth: %i", input->read_u16());
	ident--;
    }
}

// tag 46
void parse_define_shape_morph(stream *input, int tag_type)
{
    assert(tag_type == 46);
    log_msg("define_shape_morph");
    ident++;
    log_msg("character ID: %i", input->read_u16());
    ident--;
}

// tag 6
void parse_define_bits(stream* input, int tag_type)
{
    assert(tag_type==6);
    log_msg("define jpeg bits");
    ident++;
    log_msg("character ID: %i", input->read_u16());
    ident--;
}

void parse_jpeg_tables(stream* /* input */, int tag_type)
{
    assert(tag_type==8);
    log_msg("define jpeg table");
}	

void parse_set_background_color(stream* input, int tag_type)
{
    assert(tag_type==9);
    rgb::parse(input);
    log_msg("set background color to:");
    rgb::write();		
}

void parse_do_action(stream* /* input */, int tag_type)
{
    assert(tag_type==12);
    log_msg("do action:");
    ident++;
    log_msg("to be implemented");  // FIXME!
    ident--;
}		

void parse_define_sprite(stream* input, int tag_type)
{
    assert(tag_type==39);
    log_msg("define a new sprite:");
    ident++;
    int	tag_end = input->get_tag_end_position();
    uint32_t char_id = input->read_u16();
    uint32_t sprite_frame_count = input->read_u16();
    log_msg("character ID: %i", char_id);
    log_msg("frame count of sprite: %i", sprite_frame_count);
    uint32_t old_current_frame = current_frame;
    current_frame = 0;
    
    ident++;
    log_msg("\n");		
    log_msg("starting frame 0\n");
    ident++;
    
    while ((uint32_t) input->get_position() < (uint32_t) tag_end) {
	int	tag_type = input->open_tag();
	loader_function lf = NULL;
	
	if (tag_type == 0) {
	    ident--;
	    ident--;
	    ident--;
	    log_msg("end of sprite definition\n");
	} else if (lf = tag_loaders[tag_type]) {
	    (*lf)(input, tag_type);
	} else {
	    log_msg("warning: no tag loader for tag_type %d", tag_type);
	}
	input->close_tag();
    }
    current_frame = old_current_frame;
}	

void parse_set_framelabel(stream* input, int tag_type)
{
    assert(tag_type==43);
    log_msg("current framelabel:");
    ident++;
    char* str = input->read_string();
    log_msg("%s",str);
    delete str;
    
    if (input->get_position() < input->get_tag_end_position()) {
	//TODOm_color_transform.read_rgb(in);
    }
    ident--;
}	

void register_all_loaders(void)
{
    register_tag_loader(0,parse_end_movie);		
    register_tag_loader(1,parse_show_frame);
    register_tag_loader(2,parse_define_shape123);
    register_tag_loader(4,parse_place_object12);
    register_tag_loader(5,parse_remove_object12);
    register_tag_loader(6,parse_define_bits);			
    register_tag_loader(8,parse_jpeg_tables);				
    register_tag_loader(9,parse_set_background_color);
    register_tag_loader(12,parse_do_action);	
    register_tag_loader(22,parse_define_shape123);
    register_tag_loader(26,parse_place_object12);
    register_tag_loader(28,parse_remove_object12);
    register_tag_loader(32,parse_define_shape123);
    register_tag_loader(39,parse_define_sprite);	
    register_tag_loader(43,parse_set_framelabel);		
    register_tag_loader(46,parse_define_shape_morph);  
}
    
void parse_swf(std::auto_ptr<tu_file> file)
{
    ident = 1;
    
    uint32_t header = file->read_le32();
    uint32_t file_length = file->read_le32();
    
    uint32_t version = (header >> 24) & 255;
    if ((header & 0x0FFFFFF) != 0x00535746 && (header & 0x0FFFFFF) != 0x00535743) {
	log_error("No valid SWF file, header is incorrect");
	return;
    }
    
    bool compressed = (header & 255) == 'C';
    
    log_msg("SWF version %i, file length = %i bytes", version, file_length);
    
    if (compressed) {
	log_msg("file is compressed.");
	file = zlib_adapter::make_inflater(file);
	file_length -= 8;
    }
    
    stream str(file.get());
    
    rect::parse(&str);
    float frame_rate = str.read_u16() / 256.0f;
    int frame_count = str.read_u16();
    
    log_msg("viewport:");
    rect::write();
    log_msg("frame rate = %f, number of frames = %d", frame_rate, frame_count);
    
    log_msg(" "); // A space to shut compiler warning up about ""
    log_msg("starting frame 0\n");
    ident++;
    
    while ((uint32_t) str.get_position() < file_length) {
	int	tag_type = str.open_tag();
	
	loader_function	lf = NULL;
	
	if (lf = tag_loaders[tag_type]) {
	    (*lf)(&str, tag_type);	    
	} else {
	    log_msg("warning: no tag loader for tag_type %d", tag_type);
	}
	
	str.close_tag();
	
	if (tag_type == 0) {
	    if ((unsigned int)str.get_position() != file_length) {
		log_error("warning: end of file tag found, while not at the end of the file, aborting");
		break;
	    }
	}
    }
    
}

} // end of namespace parser

int
main(int argc, char *argv[])
{
    int c;

    // Enable native language support, i.e. internationalization
#ifdef ENABLE_NLS
    setlocale (LC_MESSAGES, "");
    bindtextdomain (PACKAGE, LOCALEDIR);
    textdomain (PACKAGE);
#endif
    // scan for the two main standard GNU options
    for (c = 0; c < argc; c++) {
	if (strcmp("--help", argv[c]) == 0) {
	    usage(argv[0]);
            dbglogfile.removeLog();
	    exit(0);
	}
	if (strcmp("--version", argv[c]) == 0) {
	    log_msg (_("Gnash gparser version: %s, Gnash version: %s"), 
			GPARSE_VERSION, VERSION);
            dbglogfile.removeLog();
	    exit(0);
	}
    }

    std::vector<const char*> infiles;
      
    RcInitFile& rcfile = RcInitFile::getDefaultInstance();
    rcfile.loadFiles();
    if (rcfile.verbosityLevel() > 0) {
        dbglogfile.setVerbosity(rcfile.verbosityLevel());
    }
    
    if (rcfile.useActionDump()) {
        dbglogfile.setActionDump(true);
        dbglogfile.setVerbosity();
    }
    
    if (rcfile.useParserDump()) {
        dbglogfile.setParserDump(true);
        dbglogfile.setVerbosity();
    }
    
    while ((c = getopt (argc, argv, "hg")) != -1) {
	switch (c) {
	  case 'h':
	      usage (argv[0]);
              dbglogfile.removeLog();
	      exit(0);
          case 'g':
#ifdef USE_DEBUGGER
              debugger.enabled(true);
              debugger.console();
              log_msg (_("Setting debugger ON"));
#else
              log_error (_("The debugger has been disabled at configuration time"));
#endif
	  default:
	      break;
	}
    }
  
    // get the file name from the command line
    while (optind < argc) {
	infiles.push_back(argv[optind]);
	optind++;
    }
  
    // No file names were supplied
    if (infiles.size() == 0) {
	log_error (_("No input files"));
	usage(argv[0]);
        dbglogfile.removeLog();
	exit(1);
    }
  
    // We always want output from this program
    dbglogfile.setVerbosity(1);

    parser::register_all_loaders();
    for (int i = 0, n = infiles.size(); i < n; i++) {
        std::auto_ptr<tu_file> in ( new tu_file(infiles[i], "rb") );
	log_msg (_("Processing file: %s"), infiles[i]);
	if (in->get_error()) {
	    log_error(_("Can't open file '%s' for input"), infiles[i]);
	    exit(1);
	}
    
	parser::parse_swf(in);
    }
  
    return 0;
}


static void
usage (const char *)
{
    printf(_(
	"gparser -- an SWF parser for Gnash.\n"
	"\n"
	"usage: gparser [swf files to process...]\n"
	"  --help(-h)  Print this info.\n"
	"  --version   Print the version numbers.\n"
	"  -g          Start the Flash debugger.\n"
	));
}

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
