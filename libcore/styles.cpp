// styles.cpp   -- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Fill and line style types.


#include "styles.h"
#include "log.h"
#include "render.h"
#include "SWFStream.h"
#include "movie_definition.h"
#include "SWF.h"
#include "GnashException.h"
#include "fill_style.h"
#include "GnashNumeric.h"

namespace gnash {


//
// line_style
//

    
line_style::line_style()
    :
    m_width(0),
    m_color(),
    _scaleVertically(true),
    _scaleHorizontally(true),
    _pixelHinting(false),
    _noClose(false),
    _startCapStyle(CAP_ROUND),
    _endCapStyle(CAP_ROUND),
    _joinStyle(JOIN_ROUND),
    _miterLimitFactor(1.0f)
{
}

void
line_style::read_morph(SWFStream& in, SWF::TagType t, movie_definition& md,
    const RunInfo& r, line_style *pOther)
{
    if (t == SWF::DEFINEMORPHSHAPE)
    {
        in.ensureBytes(2 + 2);
        m_width = in.read_u16();
        pOther->m_width = in.read_u16();
        m_color.read(in, t);
        pOther->m_color.read(in, t);
        return;
    }

    // MorphShape 2 from here down.
    in.ensureBytes(4 + 2);

    m_width = in.read_u16();
    pOther->m_width = in.read_u16();

    int flags1 = in.read_u8();
    int flags2 = in.read_u8();
    _startCapStyle =  (cap_style_e)((flags1 & 0xC0) >> 6);
    _joinStyle     = (join_style_e)((flags1 & 0x30) >> 4);
    bool has_fill      =   flags1 & (1 << 3);
    _scaleHorizontally = !(flags1 & (1 << 2));
    _scaleVertically   = !(flags1 & (1 << 1));
    _pixelHinting      =   flags1 & (1 << 0);
    _noClose = flags2 & (1 << 2);
    _endCapStyle = (cap_style_e) (flags2 & 0x03); 

    if (_joinStyle == JOIN_MITER)  
    {
        in.ensureBytes(2);
        _miterLimitFactor = in.read_short_ufixed();
    }
    if (has_fill)
    {
        // read fill styles for strokes.
        // TODO: don't throw away this information, should be passed to renderer.
        fill_style f, g;
        f.read(in, t, md, r, &g);
        m_color = f.get_color();
        pOther->m_color = g.get_color();
    }
    else
    {
        m_color.read(in, t);
        pOther->m_color.read(in, t);
    }
}

void
line_style::read(SWFStream& in, SWF::TagType t, movie_definition& md,
        const RunInfo& r)
{
    if (!(t == SWF::DEFINESHAPE4 || t == SWF::DEFINESHAPE4_))
    {
        in.ensureBytes(2);
        m_width = in.read_u16();
        m_color.read(in, t);
        return;
    }

    // TODO: Unfinished. Temporary to allow DefineShape4 to work in many
    // cases, but does not work correctly in all cases.
    in.ensureBytes(2+2);
    m_width = in.read_u16();

    int flags1 = in.read_u8();
    int flags2 = in.read_u8();
    _startCapStyle =  (cap_style_e)((flags1 & 0xC0) >> 6);
    _joinStyle     = (join_style_e)((flags1 & 0x30) >> 4);
    bool has_fill      =   flags1 & (1 << 3);
    _scaleHorizontally = !(flags1 & (1 << 2));
    _scaleVertically   = !(flags1 & (1 << 1));
    _pixelHinting      =   flags1 & (1 << 0);
    _noClose = flags2 & (1 << 2);
    _endCapStyle = (cap_style_e) (flags2 & 0x03); 

    if (_joinStyle == JOIN_MITER) 
    {
        in.ensureBytes(2);
        _miterLimitFactor = in.read_short_ufixed();
    }
    if (has_fill)
    {
        // read fill styles for strokes.
        // TODO: don't throw away this information, should be passed to renderer.
        fill_style f;
        f.read(in, t, md, r);
        m_color = f.get_color();
    }
    else
    {
        m_color.read(in, t);
    }
}

void
line_style::set_lerp(const line_style& ls1, const line_style& ls2, float ratio)
{
    m_width = static_cast<boost::uint16_t>(
        frnd(flerp(ls1.getThickness(), ls2.getThickness(), ratio)));
    m_color.set_lerp(ls1.get_color(), ls2.get_color(), ratio);
    if ( ls1._scaleVertically != ls2._scaleVertically )
    {
        LOG_ONCE( log_error("UNTESTED: Dunno how to interpolate line styles with different vertical thickness scaling") );
    }
    if ( ls1._scaleHorizontally != ls2._scaleHorizontally )
    {
        LOG_ONCE( log_error("UNTESTED: Dunno how to interpolate line styles with different horizontal thickness scaling") );
    }
}

// end of namespace
}


// Local Variables:
// mode: C++
// End:
