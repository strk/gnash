// LineStyle.cpp   Line style types.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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
// Based on public domain work by Thatcher Ulrich <tu@tulrich.com> 2003
// styles.cpp   -- Thatcher Ulrich <tu@tulrich.com> 2003

#include "RunResources.h"
#include "LineStyle.h"
#include "log.h"
#include "SWFStream.h"
#include "smart_ptr.h"
#include "movie_definition.h"
#include "SWF.h"
#include "GnashException.h"
#include "fill_style.h"
#include "GnashNumeric.h"

namespace gnash {

LineStyle::LineStyle()
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
LineStyle::read_morph(SWFStream& in, SWF::TagType t, movie_definition& md,
    const RunResources& r, LineStyle *pOther)
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
    _startCapStyle =  (CapStyle)((flags1 & 0xC0) >> 6);
    _joinStyle     = (JoinStyle)((flags1 & 0x30) >> 4);
    bool has_fill      =   flags1 & (1 << 3);
    _scaleHorizontally = !(flags1 & (1 << 2));
    _scaleVertically   = !(flags1 & (1 << 1));
    _pixelHinting      =   flags1 & (1 << 0);
    _noClose = flags2 & (1 << 2);
    _endCapStyle = (CapStyle) (flags2 & 0x03); 

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
LineStyle::read(SWFStream& in, SWF::TagType t, movie_definition& md,
        const RunResources& r)
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
    _startCapStyle =  (CapStyle)((flags1 & 0xC0) >> 6);
    _joinStyle     = (JoinStyle)((flags1 & 0x30) >> 4);
    bool has_fill      =   flags1 & (1 << 3);
    _scaleHorizontally = !(flags1 & (1 << 2));
    _scaleVertically   = !(flags1 & (1 << 1));
    _pixelHinting      =   flags1 & (1 << 0);
    _noClose = flags2 & (1 << 2);
    _endCapStyle = (CapStyle) (flags2 & 0x03); 

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
LineStyle::set_lerp(const LineStyle& ls1, const LineStyle& ls2, float ratio)
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
