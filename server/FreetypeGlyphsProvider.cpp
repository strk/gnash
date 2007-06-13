// FreetypeGlyphsProvider.cpp:  Freetype glyphs manager
// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "FreetypeGlyphsProvider.h"
#include "smart_ptr.h" // for intrusive_ptr
#include "image.h" // for create_alpha
#include "GnashException.h"
#include "render.h"
#include "DynamicShape.h"
#include "log.h"

#ifdef HAVE_FREETYPE2 
# include <ft2build.h>
# include FT_OUTLINE_H
# include FT_BBOX_H
#endif

#ifdef HAVE_FONTCONFIG_FONTCONFIG_H
# define HAVE_FONTCONFIG 1
#endif

#ifdef HAVE_FONTCONFIG
# include <fontconfig/fontconfig.h>
# include <fontconfig/fcfreetype.h>
#endif

#include <cstdio> // for snprintf
#include <string>
#include <memory> // for auto_ptr

// Define the following to make outline decomposition verbose
//#define DEBUG_OUTLINE_DECOMPOSITION 1

// Define the following to make glyph rendering verbose
//#define DEBUG_GLYPH_RENDERING 1

// TODO: drop this ?
#define FREETYPE_MAX_FONTSIZE 96

namespace gnash {

#ifdef HAVE_FREETYPE2 

static int
walkMoveTo(FT_Vector* to, void* ptr)
{
	DynamicShape* sh = static_cast<DynamicShape*>(ptr);
#ifdef DEBUG_OUTLINE_DECOMPOSITION 
	log_debug("moveTo: %ld,%ld", to->x, to->y);
#endif
	sh->moveTo(to->x, -to->y);
	return 0;
}

static int
walkLineTo(FT_Vector* to, void* ptr)
{
	DynamicShape* sh = static_cast<DynamicShape*>(ptr);
#ifdef DEBUG_OUTLINE_DECOMPOSITION 
	log_debug("lineTo: %ld,%ld", to->x, to->y);
#endif
	sh->lineTo(to->x, -to->y);
	return 0;
}

static int
walkConicTo(FT_Vector* ctrl, FT_Vector* to, void* ptr)
{
	DynamicShape* sh = static_cast<DynamicShape*>(ptr);
#ifdef DEBUG_OUTLINE_DECOMPOSITION 
	log_debug("conicTo: %ld,%ld %ld,%ld", ctrl->x, ctrl->y, to->x, to->y);
#endif
	sh->curveTo(ctrl->x, -ctrl->y, to->x, -to->y);
	return 0;
}

static int
walkCubicTo(FT_Vector* ctrl1, FT_Vector* ctrl2, FT_Vector* to, void* ptr)
{
	DynamicShape* sh = static_cast<DynamicShape*>(ptr);
#ifdef DEBUG_OUTLINE_DECOMPOSITION 
	log_debug("cubicTo: %ld,%ld %ld,%ld %ld,%ld", ctrl1->x, ctrl1->y, ctrl2->x, ctrl2->y, to->x, to->y);
#endif

	float x = ctrl1->x + ( (ctrl2->x - ctrl1->x) * 0.5 );
	float y = ctrl1->y + ( (ctrl2->y - ctrl1->y) * 0.5 );

	sh->curveTo(x, -y, to->x, -to->y);

	return 0;
}

// static
FT_Library FreetypeGlyphsProvider::m_lib;

// static private
void FreetypeGlyphsProvider::init()
{
	int	error = FT_Init_FreeType(&m_lib);
	if (error)
	{
		fprintf(stderr, "can't init FreeType!  error = %d\n", error);
		exit(1);
	}
}

// static private
void FreetypeGlyphsProvider::close()
{
	int error = FT_Done_FreeType(m_lib);
	if (error)
	{
		fprintf(stderr, "can't close FreeType!  error = %d\n", error);
	}
}

// private
std::auto_ptr<image::alpha>
FreetypeGlyphsProvider::draw_bitmap(const FT_Bitmap& bitmap)
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
FreetypeGlyphsProvider::get_advance_x(uint16_t code)
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
FreetypeGlyphsProvider::getFontFilename(const std::string& name,
		bool /*bold*/, bool /*italic*/, std::string& filename)
{

#define DEFAULT_FONTFILE "/usr/share/fonts/truetype/freefont/FreeSans.ttf"

#ifdef HAVE_FONTCONFIG

	if (!FcInit ())
	{

		log_error("Can't init fontconfig library, using hard-coded font filename");
		filename = DEFAULT_FONTFILE;
		return true;
		//return false;
	}

	FcResult    result;

	FcPattern* pat = FcNameParse((const FcChar8*)name.c_str());

	FcConfigSubstitute (0, pat, FcMatchPattern);
	FcDefaultSubstitute (pat);

	FcPattern   *match;
	match = FcFontMatch (0, pat, &result);
	FcPatternDestroy (pat);

	FcFontSet* fs = NULL;
	if (match)
	{
		fs = FcFontSetCreate ();
		FcFontSetAdd (fs, match);
	}

	if ( fs )
	{
		log_debug("Found %d fonts matching the family %s (using first)", fs->nfont, name.c_str());

		for (int j = 0; j < fs->nfont; j++)
		{
			FcChar8 *file;
			if (FcPatternGetString (fs->fonts[j], FC_FILE, 0, &file) != FcResultMatch)
			{
		log_debug("Matching font %d has unknown filename, skipping", j);
		continue;
			}

			filename = (char *)file;
			return true;

		}

		FcFontSetDestroy(fs);
	}

	log_error("No device font matches the name '%s', using hard-coded font filename", name.c_str());
	filename = DEFAULT_FONTFILE;
	return true;
#else
	log_error("Font filename matching not implemented (no fontconfig support built-in), using hard-coded font filename",
			name.c_str());
	filename = DEFAULT_FONTFILE;
	return true;
#endif
}

#endif // HAVE_FREETYPE2 

#ifdef HAVE_FREETYPE2 
// static
std::auto_ptr<FreetypeGlyphsProvider>
FreetypeGlyphsProvider::createFace(const std::string& name, bool bold, bool italic)
{

	std::auto_ptr<FreetypeGlyphsProvider> ret;

	try { 
		ret.reset( new FreetypeGlyphsProvider(name, bold, italic) );
	} catch (GnashException& ge) {
		log_error(ge.what());
		assert(! ret.get());
	}

	return ret;

}
#else // ndef HAVE_FREETYPE2 
std::auto_ptr<FreetypeGlyphsProvider>
FreetypeGlyphsProvider::createFace(const std::string&, bool, bool)
{
	log_error("Freetype not supported");
	return std::auto_ptr<FreetypeGlyphsProvider>(NULL);
}
#endif // ndef HAVE_FREETYPE2 

#ifdef HAVE_FREETYPE2 
FreetypeGlyphsProvider::FreetypeGlyphsProvider(const std::string& name, bool bold, bool italic)
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
FreetypeGlyphsProvider::FreetypeGlyphsProvider(const std::string&, bool, bool)
{
	assert(0); // should never be called
}
#endif // ndef HAVE_FREETYPE2 

#ifdef HAVE_FREETYPE2
boost::intrusive_ptr<shape_character_def>
FreetypeGlyphsProvider::getGlyph(uint16_t code, float& advance)
{
	boost::intrusive_ptr<DynamicShape> sh;

	FT_Error error = FT_Load_Char(m_face, code, FT_LOAD_NO_BITMAP|FT_LOAD_NO_SCALE);
	if ( error != 0 )
	{
		log_error("Error loading freetype outline glyph for char '%c' (error: %d)", code, error);
		return sh.get();
	}

	// TODO: check this. Also check FT_FaceRec::units_per_EM
	advance = m_face->glyph->metrics.horiAdvance;

	assert(m_face->glyph->format == FT_GLYPH_FORMAT_OUTLINE);

	FT_Outline* outline = &(m_face->glyph->outline);

	//FT_BBox	glyphBox;
	//FT_Outline_Get_BBox(outline, &glyphBox);
	//rect r(glyphBox.xMin, glyphBox.yMin, glyphBox.xMax, glyphBox.yMax);
	//log_msg("Glyph for character '%c' has computed bounds %s", code, r.toString().c_str());

	sh = new DynamicShape();
	sh->beginFill(rgba(255, 255, 255, 255));

	FT_Outline_Funcs walk;
       	walk.move_to = walkMoveTo;
	walk.line_to = walkLineTo;
	walk.conic_to = walkConicTo;
	walk.cubic_to = walkCubicTo;
	walk.shift = 0; // ?
	walk.delta = 0; // ?

#ifdef DEBUG_OUTLINE_DECOMPOSITION 
	log_debug("Decomposing glyph outline for character %u", code);
#endif
	FT_Outline_Decompose(outline, &walk, sh.get());
#ifdef DEBUG_OUTLINE_DECOMPOSITION 
	log_msg("Decomposed glyph for character '%c' has bounds %s", code, sh->get_bound().toString().c_str());
#endif

	return sh.get();
}
#else // ndef(HAVE_FREETYPE2)
boost::intrusive_ptr<shape_character_def>
FreetypeGlyphsProvider::getGlyph(uint16_t, float& advance)
{
	assert(0); // should never be called... 
}
#endif // ndef(HAVE_FREETYPE2)

} // namespace gnash

