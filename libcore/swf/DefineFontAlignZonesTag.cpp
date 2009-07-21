// DefineFontAlignZonesTag.cpp:  for Gnash.
//
//   Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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


#include "Font.h"
#include "log.h"
#include "SWFStream.h"
#include "movie_definition.h"
#include "DefineFontAlignZonesTag.h"
#include <iostream>

namespace gnash {
namespace SWF {

DefineFontAlignZonesTag::DefineFontAlignZonesTag(movie_definition& /* m */,
	SWFStream& /* in */)
    :
    _csm_table_int(2)
{
}

/* public static */
void
DefineFontAlignZonesTag::loader(SWFStream& in, TagType tag,
        movie_definition& m, const RunResources& /*r*/)
{
	assert(tag == SWF::DEFINEALIGNZONES);

	in.ensureBytes(2);
	unsigned short ref = in.read_u16(); // must reference a valid DEFINEFONT3 tag
	Font* referencedFont = m.get_font(ref);
	if ( ! referencedFont )
	{
		IF_VERBOSE_MALFORMED_SWF(
		log_swferror(_("DefineFontAlignZones tag references an undefined "
               "font %d"), ref);
		);
		in.skip_to_tag_end();
		return;
	}

	in.ensureBytes(1);
	unsigned flags = in.read_u8(); // 2bits are cms table, 6bits are reserved

	// TODO:
	// 	- parse swf_zone_array
	// 	- construct a DefineFontAlignZonesTag class
	// 	- register the tag with the referencedFont

	IF_VERBOSE_PARSE (
	log_parse(_(" ** DefineFontAlignZones: font=%d, flags=%d"), ref, flags);
	);

//	Si added here	
//      The following codes are based on Alexis' SWF Reference.
//	They are not guaranteed to be correct and completed.
	unsigned short csm_table_int_temp=flags>>6;
			
	assert(csm_table_int_temp == 0 || csm_table_int_temp == 1 || csm_table_int_temp == 2  );

//	log_debug(_("The value of the 'tag': %d "),tag);	// You will get 73 here!!  :)	
//	log_debug(_("The value of the 'ref': %d "), ref); 
//	log_debug(_("The value of the 'flags' : %d "),flags); 		 
//	log_debug(_("The value of the 'csm_table_int_temp': %d "),csm_table_int_temp);	
//	log_debug(_("The value of the 'font3.f_font2_glyphs_count: %d "),referencedFont->glyphCount() );

//	log_debug(_("The value of the sizeof 'tag': %d \n"),sizeof(tag));	//4
//	log_debug(_("The value of the sizeof 'ref': %d \n"), sizeof(ref)); 	//2
//	log_debug(_("The value of the sizeof 'flags' : %d \n"),sizeof(flags)); //4		 
//	log_debug(_("The value of the sizeof 'csm_table_int_temp': %d  \n"),sizeof(csm_table_int_temp));	 //2

//	log_debug(_("sizeof(int): %d \n"), sizeof(int) );    //4
//	log_debug(_("sizeof(int): %d \n" ), sizeof(short int) );	//2
//	log_debug(_("sizeof(unsigned short): %d \n"),sizeof(unsigned short) );	//2
//	log_debug(_("sizeof(unsigned): %d \n"), sizeof(unsigned) ); //4
//      log_debug(_( "sizeof(float): %d \n"),sizeof(float) ); //4
//      log_debug(_("sizeof(double): %d \n"), sizeof(double) );//8
//      log_debug(_("sizeof(char): %d \n"), sizeof(char) );//1
//	log_debug(_("****The value of the 'csm_table_int_temp': %d  \n"),csm_table_int_temp);	
//	log_debug(_("Hello, Let us try to parse all the tag information \n") );

	Font::GlyphInfoRecords::size_type glyphs_count_temp=referencedFont->glyphCount();

//	Let us have a loop to read all the information

	for (int i=0; i!=int(glyphs_count_temp); i++){
			in.ensureBytes(1);
			unsigned int nouse;
			nouse=in.read_u8();		
//			log_debug(_("The value of f_zone_count= %d  \n"),nouse );
			
			for (int j=0; j!=2; j++){
				in.ensureBytes(2);
				float		f_zone_position_temp=in.read_u16();
				in.ensureBytes(2);
				float		f_zone_size_temp=in.read_u16();
//				log_debug(_("   In the subloop:  glyph: %d zone= %d position= %f size= %f \n"),i+1,j+1,f_zone_position_temp, f_zone_size_temp );
				}		
			in.ensureBytes(1);
			nouse=in.read_u8();
//			log_debug(_("new output: glyph: %d, nouse =%d  \n"),i+1,nouse );
			unsigned f_zone_x_temp=nouse & 0x0001;
			nouse = (nouse >> 1);
			unsigned f_zone_y_temp=nouse & 0x0001;
			
//			log_debug(_("new output: glyph: %d, f_zone_y= %d, f_zone_x=%d  \n"),i+1,f_zone_y_temp,f_zone_x_temp );
			}
		
		

//	struct swf_definefontalignzones {
//		swf_tag			f_tag;		/* 73 */
//		unsigned short		f_font2_id_ref;                 // 1 bytes
//		unsigned		f_csm_table_hint : 2;           //Read in flag? 2bits
//		unsigned		f_reserved : 6;			//Read in flag? 6bits
//		swf_zone_array		f_zones[corresponding define font3.f_font2_glyphs_count];
//	}

	//ref readed
	//tag readed 


	// The f_font2_glyphs_count does not exist at all.
	// I use the function glyphCount() defined in the Font class.
	// This function retrieve the number of embedded glyphs in this font.

	

	//Now read for swf_zone_array.
	///But how?

//	swf_zone_array f_zones_temp[2];
	
//	swf_zone_array f_zones_temp[glyphs_count_tempt];

//	unsigned f_zone_count = in.read_u2(); // 2bits are cms table, 6bits are reserved
//	for (int i=1; i<=f_zone_count; i++)
//			{
//			}
	
//	in.ensureBytes(1);

		
/*
		struct swf_zone_array {
			unsigned char		f_zone_count;		// always 2 in V8.0           //??Why
			swf_zone_data		f_zone_data[f_zone_count];
			// I inverted the bits below, but I'm not too sure what is correct, do you know? 
			unsigned		f_reserved : 6;
			unsigned		f_zone_y : 1;		// probably always 1 in V8.0 
			unsigned		f_zone_x : 1;		// probably always 1 in V8.0 
		};
		
		//What is the size here? char?
		in.ensureBytes(1);
		unsigned char uchar = in.read_u8();

			struct swf_zone_data {
				short float		f_zone_position;
				short float		f_zone_size;
			};
			in.ensureBytes(2);
			short float f_zone_position_now = in.read_u16();
			in.ensureBytes(2);
			short float f_zone_size = in.read_u16();
	
		 in.ensureBytes(1);
		 unsigned flags2 = in.read_u8(); // 6 bits are resered, 1 for f_zone_y, 1 for f_zone_
	
//	    referencedFont->f_font2_id_ref=;
//	    referencedFont->f_csm_table_hint=2;
//	    referencedFont->f_reserved=6;
//	    referencedFont->f_zones=


        in.ensureBytes(1);
	unsigned short f_r_y_x = in.read_u8(); // must reference a valid DEFINEFONT3 tag
*/	

//	boost::uint16_t id = referencedFont->fontID;

//	DefineFontAlignZonesTag* ch = new DefineFontAlignZonesTag(m,in);
//	m.addControlTag(ch);

	in.skip_to_tag_end();
	LOG_ONCE(log_unimpl(_("*** DefineFontAlignZoneTag")));

}


} // namespace gnash::SWF
} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End
