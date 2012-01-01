// LineStyle.cpp   Line style types.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include "LineStyle.h"

#include "TypesParser.h"
#include "RunResources.h"
#include "log.h"
#include "SWFStream.h"
#include "movie_definition.h"
#include "SWF.h"
#include "GnashException.h"
#include "FillStyle.h"
#include "GnashNumeric.h"

namespace gnash {

namespace {

class GetColor : public boost::static_visitor<rgba>
{
public:
    rgba operator()(const SolidFill& f) const {
        return f.color();
    }
    rgba operator()(const GradientFill&) const {
        return rgba();
    }
    rgba operator()(const BitmapFill&) const {
        return rgba();
    }
};

}

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
    const RunResources& /*r*/, LineStyle *pOther)
{
    if (t == SWF::DEFINEMORPHSHAPE)
    {
        in.ensureBytes(2 + 2);
        m_width = in.read_u16();
        pOther->m_width = in.read_u16();
        m_color = readRGBA(in);
        pOther->m_color = readRGBA(in);
        return;
    }

    assert(t == SWF::DEFINEMORPHSHAPE2 || t == SWF::DEFINEMORPHSHAPE2_);

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
    if (has_fill) {
        OptionalFillPair fp = readFills(in, t, md, true);

        // TODO: store a fill style properly, removing the need for the 
        // visitor.
        m_color = boost::apply_visitor(GetColor(), fp.first.fill);
        pOther->m_color = boost::apply_visitor(GetColor(), fp.second->fill);
    }
    else {
        m_color = readRGBA(in);
        pOther->m_color = readRGBA(in);
    }
}

void
LineStyle::read(SWFStream& in, SWF::TagType t, movie_definition& md,
        const RunResources& /*r*/)
{
    switch (t) {

        default:
            in.ensureBytes(2);
            m_width = in.read_u16();
            m_color = readRGBA(in);
            return;

        case SWF::DEFINESHAPE:
        case SWF::DEFINESHAPE2:
            in.ensureBytes(2);
            m_width = in.read_u16();
            m_color = readRGB(in);
            return;

        case SWF::DEFINESHAPE4:
        case SWF::DEFINESHAPE4_:
        {
            // TODO: Unfinished. Temporary to allow DefineShape4 to work in
            // many cases, but does not work correctly in all cases.
            in.ensureBytes(2+2);
            m_width = in.read_u16();

            const boost::uint8_t flags1 = in.read_u8();
            const boost::uint8_t flags2 = in.read_u8();

            _startCapStyle = (CapStyle)((flags1 & 0xC0) >> 6);
            _joinStyle = (JoinStyle)((flags1 & 0x30) >> 4);
            const bool has_fill  =   flags1 & (1 << 3);
            _scaleHorizontally = !(flags1 & (1 << 2));
            _scaleVertically   = !(flags1 & (1 << 1));
            _pixelHinting      =   flags1 & (1 << 0);
            _noClose = flags2 & (1 << 2);
            _endCapStyle = (CapStyle) (flags2 & 0x03); 

            if (_joinStyle == JOIN_MITER) {
                in.ensureBytes(2);
                _miterLimitFactor = in.read_short_ufixed();
            }
            if (has_fill) {
                // TODO: store a fill style properly, removing the need for the 
                // visitor.
                OptionalFillPair fp = readFills(in, t, md, false);
                m_color = boost::apply_visitor(GetColor(), fp.first.fill);
            }
            else {
                m_color = readRGBA(in);
            }
        }
    }
}

void
LineStyle::set_lerp(const LineStyle& ls1, const LineStyle& ls2, float ratio)
{
    m_width = static_cast<boost::uint16_t>(
        frnd(lerp<float>(ls1.getThickness(), ls2.getThickness(), ratio)));
    m_color = lerp(ls1.get_color(), ls2.get_color(), ratio);
    if ( ls1._scaleVertically != ls2._scaleVertically )
    {
        LOG_ONCE(log_error(_("UNTESTED: Do not know how to interpolate"
            " line styles with different vertical thickness scaling")));
    }
    if ( ls1._scaleHorizontally != ls2._scaleHorizontally )
    {
        LOG_ONCE(log_error(_("UNTESTED: Do not know how to interpolate"
            " line styles with different horizontal thickness scaling")));
    }
}

} // namespace gnash


// Local Variables:
// mode: C++
// End:
