// gameswf_freetype.cpp	-- Vitaly Alexeev <tishka92@yahoo.com>	2007

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// TrueType font rasterizer based on freetype library,
// used code from demos/font_output/font_output.cpp

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "FreetypeRasterizer.h"
#include "smart_ptr.h" // for intrusive_ptr
#include "image.h" // for create_alpha
#include "GnashException.h"
#include "render.h"
#include "log.h"

#include <cstdio> // for snprintf
#include <string>
#include <memory> // for auto_ptr

#define FREETYPE_MAX_FONTSIZE 96

namespace gnash {

#ifdef HAVE_FREETYPE2 

// static
FT_Library FreetypeRasterizer::m_lib;

// static private
void FreetypeRasterizer::init()
{
	int	error = FT_Init_FreeType(&m_lib);
	if (error)
	{
		fprintf(stderr, "can't init FreeType!  error = %d\n", error);
		exit(1);
	}
}

// static private
void FreetypeRasterizer::close()
{
	int error = FT_Done_FreeType(m_lib);
	if (error)
	{
		fprintf(stderr, "can't close FreeType!  error = %d\n", error);
	}
}

// private
std::auto_ptr<image::alpha>
FreetypeRasterizer::draw_bitmap(const FT_Bitmap& bitmap)
{
	// You must use power-of-two dimensions!!
	int	w = 1; while (w < bitmap.pitch) { w <<= 1; }
	int	h = 1; while (h < bitmap.rows) { h <<= 1; }

	std::auto_ptr<image::alpha> alpha ( image::create_alpha(w, h) );

	memset(alpha->m_data, 0, alpha->m_width * alpha->m_height);

	// copy image to alpha
	for (int i = 0; i < bitmap.rows; i++)
	{
		uint8_t*	src = bitmap.buffer + bitmap.pitch * i;
		uint8_t*	dst = alpha->m_data + alpha->m_pitch * i;
		int	x = bitmap.width;
		while (x-- > 0)
		{
			*dst++ = *src++;
		}
	}

	return alpha;
}

#if 0
// private
float
FreetypeRasterizer::get_advance_x(uint16_t code)
{
	FT_Set_Pixel_Sizes(m_face, 0, FREETYPE_MAX_FONTSIZE);
	if (FT_Load_Char(m_face, code, FT_LOAD_RENDER))
	{
		return 0;
	}
	return (float) m_face->glyph->metrics.horiAdvance * s_advance_scale;
}
#endif

// private
bool
FreetypeRasterizer::getFontFilename(const std::string& name,
		bool bold, bool italic, std::string& filename)
{
	// TODO: implement
	log_error("FIXME: font name to filename mapping unimplemented");
	return false;
}

#endif // HAVE_FREETYPE2 

#ifdef HAVE_FREETYPE2 
// static
std::auto_ptr<FreetypeRasterizer>
FreetypeRasterizer::createFace(const std::string& name, bool bold, bool italic)
{

	std::auto_ptr<FreetypeRasterizer> ret;

	try { 
		ret.reset( new FreetypeRasterizer(name, bold, italic) );
	} catch (GnashException& ge) {
		log_error(ge.what());
		assert(! ret.get());
	}

	return ret;

}
#else // ndef HAVE_FREETYPE2 
std::auto_ptr<FreetypeRasterizer>
FreetypeRasterizer::createFace(const std::string&, bool, bool)
{
	log_error("Freetype not supported");
	return std::auto_ptr<FreetypeRasterizer>(NULL);
}
#endif // ndef HAVE_FREETYPE2 

#ifdef HAVE_FREETYPE2 
FreetypeRasterizer::FreetypeRasterizer(const std::string& name, bool bold, bool italic)
	:
	m_face(NULL)
{
	const unsigned maxerrlen = 64;
	char buf[maxerrlen];

	if (m_lib == NULL)
	{
		init();
	}

	std::string filename;
	if (getFontFilename(name, bold, italic, filename) == false)
	{
		snprintf(buf, maxerrlen, _("Can't find font file for font '%s'"), name.c_str());
		buf[maxerrlen-1] = '\0';
		throw GnashException(buf);
	}

	int error = FT_New_Face(m_lib, filename.c_str(), 0, &m_face);
	switch (error)
	{
		case 0:
			break;

		case FT_Err_Unknown_File_Format:
			snprintf(buf, maxerrlen, _("Font file '%s' has bad format"), filename.c_str());
			buf[maxerrlen-1] = '\0';
			throw GnashException(buf);
			break;

		default:
			// TODO: return a better error message !
			snprintf(buf, maxerrlen, _("Some error opening font '%s'"), filename.c_str());
			buf[maxerrlen-1] = '\0';
			throw GnashException(buf);
			break;
	}
}
#else // ndef(HAVE_FREETYPE2)
FreetypeRasterizer::FreetypeRasterizer(const std::string&, bool, bool)
{
	assert(0); // should never be called
}
#endif // ndef HAVE_FREETYPE2 

#ifdef HAVE_FREETYPE2
boost::intrusive_ptr<bitmap_info>
FreetypeRasterizer::getRenderedGlyph(uint16_t code, rect& box, float& advance)
{
	boost::intrusive_ptr<bitmap_info> bi;

	FT_Set_Pixel_Sizes(m_face, 0, FREETYPE_MAX_FONTSIZE);
	if (FT_Load_Char(m_face, code, FT_LOAD_RENDER))
	{
		return bi;
	}


	std::auto_ptr<image::alpha> im ( draw_bitmap(m_face->glyph->bitmap) );
	bi = render::create_bitmap_info_alpha(im->m_width, im->m_height, im->m_data);

	float xmax = float(m_face->glyph->bitmap.width) / float(bi->m_suspended_image->m_width);
	float ymax = float(m_face->glyph->bitmap.rows) / float(bi->m_suspended_image->m_height);

	float xmin = float(m_face->glyph->metrics.horiBearingX) / float(m_face->glyph->metrics.width);
	float ymin = float(m_face->glyph->metrics.horiBearingY) / float(m_face->glyph->metrics.height);

	// ???
	xmin *= -xmax;
	ymin *= ymax;

	box.enclose_point(xmin, ymin);
	box.expand_to_point(xmax, ymax);
	
	static float s_advance_scale = 0.16666666f; //vv hack
	advance = (float) m_face->glyph->metrics.horiAdvance * s_advance_scale;

	return bi;
}
#else // ndef(HAVE_FREETYPE2)
boost::intrusive_ptr<bitmap_info>
FreetypeRasterizer::getRenderedGlyph(uint16_t, rect& , float&)
{
	assert(0); // should never be called... 
}
#endif // ndef(HAVE_FREETYPE2)

} // namespace gnash

