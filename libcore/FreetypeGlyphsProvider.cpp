// FreetypeGlyphsProvider.cpp:  Freetype glyphs manager
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "FreetypeGlyphsProvider.h"

#include <string>
#include <memory> // for auto_ptr
#include <boost/cstdint.hpp>
#include <boost/format.hpp>

#include "GnashException.h"
#include "ShapeRecord.h"
#include "log.h"
#include "FillStyle.h"

#ifdef USE_FREETYPE 
# include <ft2build.h>
# include FT_OUTLINE_H
# include FT_BBOX_H
// Methods of FT_Outline_Funcs take a 'const FT_Vector*' in 2.2
// and a non-const one in 2.1, so we use an FT_CONST macro to
// support both
# if FREETYPE_MAJOR == 2 && FREETYPE_MINOR < 2
#  define FT_CONST 
# else
#  define FT_CONST const
# endif
#endif

#ifdef HAVE_FONTCONFIG_FONTCONFIG_H
# define HAVE_FONTCONFIG 1
#endif

#ifdef HAVE_FONTCONFIG
# include <fontconfig/fontconfig.h>
# include <fontconfig/fcfreetype.h>
#endif

#ifdef HAIKU_HOST
# include <Path.h>
# include <FindDirectory.h>
#endif

// Define the following to make outline decomposition verbose
//#define DEBUG_OUTLINE_DECOMPOSITION 1

// Define the following to make device font handling verbose
//#define GNASH_DEBUG_DEVICEFONTS 1

namespace gnash {

#ifdef USE_FREETYPE 

/// Outline glyph walker/decomposer, for drawing an FT_Outline to DynamicShape
//
/// See  FT_Outline_Decompose function of freetype2 lib
///
class OutlineWalker
{

public:

    /// Create an outline walker drawing to the given DynamiShape
    //
    /// @param sh
    ///    The DynamicShape to draw to. Externally owned.
    ///
    /// @param scale
    ///    The scale to apply to coordinates.
    ///    This is to match an arbitrary EM 
    ///
    OutlineWalker(SWF::ShapeRecord& sh, float scale)
        :
        _shape(sh),
        _scale(scale),
        _currPath(0),
        _x(0),
        _y(0)
    {
        /// A default fill style is solid white.
        FillStyle f = SolidFill(rgba());
        _subshape.addFillStyle(f);
        _subshape.addPath(Path(_x, _y, 1, 0, 0));
        _currPath = &_subshape.currentPath();
    }

    void finish() 
    {
        _currPath->close();
        _shape.addSubshape(_subshape);
    }

    ~OutlineWalker() {}

    /// Callback function for the move_to member of FT_Outline_Funcs
    static int
    walkMoveTo(FT_CONST FT_Vector* to, void* ptr)
    {
        OutlineWalker* walker = static_cast<OutlineWalker*>(ptr);
        return walker->moveTo(to);
    }

    /// Callback function for the line_to member of FT_Outline_Funcs
    static int
    walkLineTo(FT_CONST FT_Vector* to, void* ptr)
    {
        OutlineWalker* walker = static_cast<OutlineWalker*>(ptr);
        return walker->lineTo(to);
    }

    /// Callback function for the conic_to member of FT_Outline_Funcs
    static int
    walkConicTo(FT_CONST FT_Vector* ctrl, FT_CONST FT_Vector* to, void* ptr)
    {
        OutlineWalker* walker = static_cast<OutlineWalker*>(ptr);
        return walker->conicTo(ctrl, to);
    }

    /// Callback function for the cubic_to member of FT_Outline_Funcs
    //
    /// Transform the cubic curve into a quadratic one an interpolated point
    /// falling in the middle of the two control points.
    ///
    static int
    walkCubicTo(FT_CONST FT_Vector* ctrl1, FT_CONST FT_Vector* ctrl2,
            FT_CONST FT_Vector* to, void* ptr)
    {
        OutlineWalker* walker = static_cast<OutlineWalker*>(ptr);
        return walker->cubicTo(ctrl1, ctrl2, to);
    }

private:
    
    int moveTo(const FT_Vector* to)
    {
#ifdef DEBUG_OUTLINE_DECOMPOSITION 
        log_debug("moveTo: %ld,%ld", to->x, to->y);
#endif
        _x = static_cast<boost::int32_t>(to->x * _scale);
        _y = - static_cast<boost::int32_t>(to->y * _scale);
        _currPath->close();
        _subshape.addPath(Path(_x, _y, 1, 0, 0));
        _currPath = &_subshape.currentPath();
        return 0;
    }

    int lineTo(const FT_Vector* to)
    {
#ifdef DEBUG_OUTLINE_DECOMPOSITION 
        log_debug("lineTo: %ld,%ld", to->x, to->y);
#endif
        _x = static_cast<boost::int32_t>(to->x * _scale);
        _y = - static_cast<boost::int32_t>(to->y * _scale);
        _currPath->drawLineTo(_x, _y);
        expandBounds(_x, _y);
        return 0;
    }

    int conicTo(const FT_Vector* ctrl, const FT_Vector* to)
    {
#ifdef DEBUG_OUTLINE_DECOMPOSITION 
        log_debug("conicTo: %ld,%ld %ld,%ld", ctrl->x, ctrl->y, to->x, to->y);
#endif
        boost::int32_t x1 = static_cast<boost::int32_t>(ctrl->x * _scale);
        boost::int32_t y1 = static_cast<boost::int32_t>(ctrl->y * _scale);
        _x = static_cast<boost::int32_t>(to->x * _scale);
        _y = - static_cast<boost::int32_t>(to->y * _scale);
        _currPath->drawCurveTo(x1, -y1, _x, _y);
        expandBounds(x1, -y1, _x, _y);
        return 0;
    }

    int
    cubicTo(const FT_Vector* ctrl1, const FT_Vector* ctrl2, const FT_Vector* to)
    {
#ifdef DEBUG_OUTLINE_DECOMPOSITION 
        log_debug("cubicTo: %ld,%ld %ld,%ld %ld,%ld", ctrl1->x,
                ctrl1->y, ctrl2->x, ctrl2->y, to->x, to->y);
#endif
        float x = ctrl1->x + ( (ctrl2->x - ctrl1->x) * 0.5 );
        float y = ctrl1->y + ( (ctrl2->y - ctrl1->y) * 0.5 );
        boost::int32_t x1 = static_cast<boost::int32_t>(x * _scale);
        boost::int32_t y1 = static_cast<boost::int32_t>(y * _scale);
        _x = static_cast<boost::int32_t>(to->x * _scale);
        _y = - static_cast<boost::int32_t>(to->y * _scale);
        
        _currPath->drawCurveTo(x1, -y1, _x, _y);
        expandBounds(x1, -y1, _x, _y);
        return 0;
    }
    
    void expandBounds(int x, int y) {
        SWFRect bounds = _shape.getBounds();
        if (_currPath->size() == 1) _currPath->expandBounds(bounds, 0, 6);
        else {
            bounds.expand_to_circle(x, y, 0);
        }
        _shape.setBounds(bounds);
    }

    void expandBounds(int ax, int ay, int cx, int cy) {
        SWFRect bounds = _shape.getBounds();
        if (_currPath->size() == 1) _currPath->expandBounds(bounds, 0, 6);
        else {
            bounds.expand_to_circle(ax, ay, 0);
            bounds.expand_to_circle(cx, cy, 0);
        }
        _shape.setBounds(bounds);
    }

    SWF::Subshape _subshape;
    SWF::ShapeRecord& _shape;

    const float _scale;

    Path* _currPath;

    boost::int32_t _x, _y;

};

// statics
FT_Library FreetypeGlyphsProvider::m_lib = 0;
boost::mutex FreetypeGlyphsProvider::m_lib_mutex;

// static private
void
FreetypeGlyphsProvider::init()
{
    boost::mutex::scoped_lock lock(m_lib_mutex);

    if (m_lib) return; 

    const int error = FT_Init_FreeType(&m_lib);
    if (error) {
        boost::format err = boost::format(_("Can't init FreeType! Error "
                    "= %d")) % error;
        throw GnashException(err.str());
    }
}

// static private
void
FreetypeGlyphsProvider::close()
{
    const int error = FT_Done_FreeType(m_lib);
    if (error) {
        log_error(_("Can't close FreeType! Error = %d"), error);
    }
}

// private
bool
FreetypeGlyphsProvider::getFontFilename(const std::string &name,
   bool bold, bool italic, std::string& filename)
{

#ifdef HAIKU_HOST
    // only returns a default
    BPath bp;
    if (B_OK != find_directory(B_BEOS_FONTS_DIRECTORY, &bp)) {
        log_error(_("Failed to find fonts directory, using hard-coded "
                    "font filename \"%s\""), DEFAULT_FONTFILE);
        filename = DEFAULT_FONTFILE;
        return true;
    }
    
    bp.Append("ttfonts/DejaVuSans.ttf");
    filename = bp.Path();
    return true;
#endif
    
#ifdef HAVE_FONTCONFIG
    if (!FcInit ()) {        
        log_error(_("Can't init fontconfig library, using hard-"
                "coded font filename \"%s\""), DEFAULT_FONTFILE);
        filename = DEFAULT_FONTFILE;
        return true;
        //return false;
    }
    
    FcResult result;
    
    FcPattern* pat = FcNameParse((const FcChar8*)name.c_str());
    
    FcConfigSubstitute (0, pat, FcMatchPattern);

    if (italic) {
        FcPatternAddInteger (pat, FC_SLANT, FC_SLANT_ITALIC);
    }

    if (bold) {
        FcPatternAddInteger (pat, FC_WEIGHT, FC_WEIGHT_BOLD);
    }

    FcDefaultSubstitute (pat);

    FcPattern   *match;
    match = FcFontMatch (0, pat, &result);
    FcPatternDestroy (pat);

    FcFontSet* fs = NULL;
    if (match) {
        fs = FcFontSetCreate ();
        FcFontSetAdd (fs, match);
    }

    if ( fs ) {
#ifdef GNASH_DEBUG_DEVICEFONTS
        log_debug("Found %d fonts matching the family %s (using "
                    "first)", fs->nfont, name);
#endif

        for (int j = 0; j < fs->nfont; j++) {
            FcChar8 *file;
            if (FcPatternGetString (fs->fonts[j], FC_FILE, 0, &file) != FcResultMatch) {
#ifdef GNASH_DEBUG_DEVICEFONTS
                log_debug("Matching font %d has unknown filename, skipping",
                          j);
#endif
        continue;
            }

            filename = (char *)file;
            FcFontSetDestroy(fs);

#ifdef GNASH_DEBUG_DEVICEFONTS
            log_debug("Loading font from file %d", filename);
#endif
            return true;

        }

        FcFontSetDestroy(fs);
    }

    log_error(_("No device font matches the name '%s', using hard-coded"
                " font filename"), name);
    filename = DEFAULT_FONTFILE;
    return true;
#else
    log_error(_("Font filename matching not implemented (no fontconfig"
                " support built-in), using hard-coded font filename"),
            DEFAULT_FONTFILE);
    filename = DEFAULT_FONTFILE;
    return true;
#endif
}

#endif // USE_FREETYPE 

#ifdef USE_FREETYPE 
// static
std::auto_ptr<FreetypeGlyphsProvider>
FreetypeGlyphsProvider::createFace(const std::string& name, bool bold, bool italic)
{

    std::auto_ptr<FreetypeGlyphsProvider> ret;

    try { 
        ret.reset(new FreetypeGlyphsProvider(name, bold, italic));
    }
    catch (const GnashException& ge) {
        log_error(ge.what());
        assert(! ret.get());
    }

    return ret;

}
#else // ndef USE_FREETYPE 
std::auto_ptr<FreetypeGlyphsProvider>
FreetypeGlyphsProvider::createFace(const std::string&, bool, bool)
{
    log_error(_("Freetype not supported"));
    return std::auto_ptr<FreetypeGlyphsProvider>(NULL);
}
#endif 
	
unsigned short
FreetypeGlyphsProvider::unitsPerEM() const
{
    assert(_face);
    return _face->units_per_EM;
}

float
FreetypeGlyphsProvider::descent() const
{
    assert(_face);
    return std::abs(_face->descender);
}

float
FreetypeGlyphsProvider::ascent() const
{
    assert(_face);
    return _face->ascender;
}

#ifdef USE_FREETYPE 
FreetypeGlyphsProvider::FreetypeGlyphsProvider(const std::string& name,
        bool bold, bool italic)
    :
    _face(NULL)
{

    if (m_lib == NULL)
    {
        init();
    }

    std::string filename;
    if (getFontFilename(name, bold, italic, filename) == false)
    {
        boost::format msg = boost::format(_("Can't find font file "
                       "for font '%s'")) % name;
        throw GnashException(msg.str());
    }

    int error = FT_New_Face(m_lib, filename.c_str(), 0, &_face);
    switch (error)
    {
        case 0:
            break;

        case FT_Err_Unknown_File_Format:
        {
            boost::format msg = boost::format(_("Font file '%s' "
                        "has bad format")) % filename;
            throw GnashException(msg.str());
            break;
        }

        default:
        {
            // TODO: return a better error message !
            boost::format msg = boost::format(_("Some error "
                        "opening font '%s'"))
                               % filename;
            throw GnashException(msg.str());
            break;
        }
    }

    // We want an EM of unitsPerEM, so if units_per_EM is different
    // we will scale 
    scale = (float)unitsPerEM()/_face->units_per_EM;

#ifdef GNASH_DEBUG_DEVICEFONTS
    log_debug("EM square for font '%s' is %d, scale is this %g",
              name, _face->units_per_EM, scale);
#endif
}
#else // ndef(USE_FREETYPE)
FreetypeGlyphsProvider::FreetypeGlyphsProvider(const std::string&, bool, bool)
{
    abort(); // should never be called
}
#endif // ndef USE_FREETYPE 

#ifdef USE_FREETYPE
std::auto_ptr<SWF::ShapeRecord>
FreetypeGlyphsProvider::getGlyph(boost::uint16_t code, float& advance)
{
    std::auto_ptr<SWF::ShapeRecord> glyph;

    FT_Error error = FT_Load_Char(_face, code, FT_LOAD_NO_BITMAP | 
                                                FT_LOAD_NO_SCALE);

    if (error) {
        log_error(_("Error loading freetype outline glyph for char '%c' "
                    "(error: %d)"), code, error);
        return glyph;
    }

    // Scale advance by current scale, to match expected output coordinate space
    advance = _face->glyph->metrics.horiAdvance * scale;
#ifdef GNASH_DEBUG_DEVICEFONTS 
    log_debug("Advance value for glyph '%c' is %g (horiAdvance:%ld, "
                "scale:%g)", code, advance, 
              _face->glyph->metrics.horiAdvance, scale);
#endif

    if ( _face->glyph->format != FT_GLYPH_FORMAT_OUTLINE )
    {
        unsigned long gf = _face->glyph->format;
        log_unimpl(_("FT_Load_Char() returned a glyph format != "
                     "FT_GLYPH_FORMAT_OUTLINE (%c%c%c%c)"),
            static_cast<char>((gf>>24)&0xff),
            static_cast<char>((gf>>16)&0xff),
            static_cast<char>((gf>>8)&0xff),
            static_cast<char>(gf&0xff));
        return glyph;
    }

    FT_Outline* outline = &(_face->glyph->outline);

    FT_Outline_Funcs walk;
    walk.move_to = OutlineWalker::walkMoveTo;
    walk.line_to = OutlineWalker::walkLineTo;
    walk.conic_to = OutlineWalker::walkConicTo;
    walk.cubic_to = OutlineWalker::walkCubicTo;
    walk.shift = 0; // ?
    walk.delta = 0; // ?

#ifdef DEBUG_OUTLINE_DECOMPOSITION 
    log_debug("Decomposing glyph outline for DisplayObject %u", code);
#endif
    
    glyph.reset(new SWF::ShapeRecord);

    OutlineWalker walker(*glyph, scale);

    FT_Outline_Decompose(outline, &walk, &walker);
#ifdef DEBUG_OUTLINE_DECOMPOSITION 
#if 0
    // Don't use VM::get if this is to be re-enabled.
    SWFRect bound; sh->compute_bound(&bound, VM::get().getSWFVersion());
    log_debug("Decomposed glyph for DisplayObject '%c' has bounds %s",
            code, bound.toString());
#endif
#endif

    walker.finish();

    return glyph;
}
#else // ndef(USE_FREETYPE)

std::auto_ptr<SWF::ShapeRecord>
FreetypeGlyphsProvider::getGlyph(boost::uint16_t, float& advance)
{
    abort(); // should never be called... 
}
#endif

FreetypeGlyphsProvider::~FreetypeGlyphsProvider()
{
#ifdef USE_FREETYPE 
    if (_face) {
        if (FT_Done_Face(_face) != 0) {
            log_error(_("Could not release FT face resources"));
        }
    }
#endif
}

} // namespace gnash

