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


#include "ShapeRecord.h"

#include <vector>

#include "TypesParser.h"
#include "utility.h"
#include "SWF.h"
#include "SWFStream.h"
#include "movie_definition.h"
#include "FillStyle.h"
#include "Geometry.h"
#include "GnashNumeric.h"
#include "log.h"

namespace gnash {
namespace SWF {

// Forward declarations
namespace {
    void readFillStyles(ShapeRecord::FillStyles& styles, SWFStream& in,
        SWF::TagType tag, movie_definition& md, const RunResources& /*r*/);
    void readLineStyles(ShapeRecord::LineStyles& styles, SWFStream& in,
        SWF::TagType tag, movie_definition& md, const RunResources& /*r*/);
}

// Functors for path and style manipulation.
namespace {

template<typename T>
class Lerp
{
public:
    Lerp(typename T::const_iterator style1, typename T::const_iterator style2,
            const double ratio)
        :
        _style1(style1),
        _style2(style2),
        _ratio(ratio)
    {}

    void operator()(typename T::value_type& st)
    {
        setLerp(st, *_style1, *_style2, _ratio);
        ++_style1, ++_style2;
    }

private:
    typename T::const_iterator _style1;
    typename T::const_iterator _style2;
    const double _ratio;
};

// Facilities for working with list of paths.
class PathList
{
    typedef SWF::ShapeRecord::Paths Paths;
public:

    PathList(const Paths& paths)
        :
        _paths(paths),
        _currpath(0),
        _curredge(0),
        _nedges(computeNumberOfEdges(_paths))
    {}

    /// Return number of edges in the path list
    size_t size() const
    {
        return _nedges;
    }

    /// Get next edge in the path list.
    //
    /// After last edge in the list has been fetched,
    /// next call to this function will return first
    /// edge again.
    ///
    const Edge& getNextEdge()
    {
        const Edge& ret = _paths[_currpath][_curredge];
        if ( ++_curredge >= _paths[_currpath].size() ) {
            if ( ++_currpath >= _paths.size() ) {
                // this is not really needed,
                // but it's simpler to do so that
                // to make next call fail or abort..
                _currpath = 0;
                _curredge = 0;
            }
        }
        return ret;
    }

    /// Compute total number of edges
    static size_t computeNumberOfEdges(const Paths& paths)
    {
        size_t count=0;
        for (Paths::const_iterator i = paths.begin(), e = paths.end();
                i != e; ++i) {

            count += i->size();
        }
        return count;
    }

private:

    const Paths& _paths;

    size_t _currpath;

    size_t _curredge;

    size_t _nedges;

};

} // anonymous namespace

ShapeRecord::ShapeRecord(SWFStream& in, SWF::TagType tag, movie_definition& m,
        const RunResources& r)
{
    read(in, tag, m, r);
}

ShapeRecord::ShapeRecord()
{
}

ShapeRecord::~ShapeRecord()
{
}

void
ShapeRecord::clear()
{
    _bounds.set_null();
    _subshapes.clear();
}

void
Subshape::addFillStyle(const FillStyle& fs)
{
    _fillStyles.push_back(fs);
}


/// Find the bounds of this subhape, and return them in a rectangle.
SWFRect
Subshape::computeBounds(int swfVersion) const
{
    SWFRect bounds;

    for (unsigned int i = 0; i < _paths.size(); i++) {
        const Path& p = _paths[i];

        unsigned thickness = 0;
        if ( p.m_line ) {
            // For glyph shapes m_line is allowed to be 1
            // while no defined line styles are allowed.
            if (lineStyles().empty()) {
                // This is either a Glyph, for which m_line==1 is valid
                // or a bug in the parser, which we have no way to
                // check at this time
                assert(p.m_line == 1);
            }
            else
            {
                thickness = lineStyles()[p.m_line-1].getThickness();
            }
        }
        p.expandBounds(bounds, thickness, swfVersion);
    }

    return bounds;
}

void
ShapeRecord::setLerp(const ShapeRecord& aa, const ShapeRecord& bb,
        const double ratio)
{
	assert(_subshapes.size() == 1);

    // Update current bounds.
    _bounds.set_lerp(aa.getBounds(), bb.getBounds(), ratio);
    const Subshape& a = aa.subshapes().front();
    const Subshape& b = bb.subshapes().front();

    // fill styles
    const FillStyles::const_iterator fs1 = a.fillStyles().begin();
    const FillStyles::const_iterator fs2 = b.fillStyles().begin();

    std::for_each(_subshapes.front().fillStyles().begin(), _subshapes.front().fillStyles().end(),
            Lerp<FillStyles>(fs1, fs2, ratio));

    // line styles
    const LineStyles::const_iterator ls1 = a.lineStyles().begin();
    const LineStyles::const_iterator ls2 = b.lineStyles().begin();

    std::for_each(_subshapes.front().lineStyles().begin(), _subshapes.front().lineStyles().end(),
            Lerp<LineStyles>(ls1, ls2, ratio));

    // This is used for cases in which number
    // of paths in start shape and end shape are not
    // the same.
    const Path empty_path;
    const Edge empty_edge;

    // shape
    const Paths& paths1 = a.paths();
    const Paths& paths2 = b.paths();
    for (size_t i = 0, k = 0, n = 0; i < _subshapes.front().paths().size(); i++) {
        Path& p = _subshapes.front().paths()[i];
        const Path& p1 = i < paths1.size() ? paths1[i] : empty_path;
        const Path& p2 = n < paths2.size() ? paths2[n] : empty_path;

        const float new_ax = lerp<float>(p1.ap.x, p2.ap.x, ratio);
        const float new_ay = lerp<float>(p1.ap.y, p2.ap.y, ratio);

        p.reset(new_ax, new_ay, p1.getLeftFill(),
                p2.getRightFill(), p1.getLineStyle());

        //  edges;
        const size_t len = p1.size();
        p.m_edges.resize(len);

        for (size_t j=0; j < p.size(); j++) {
            Edge& e = p[j];
            const Edge& e1 = j < p1.size() ? p1[j] : empty_edge;

            const Edge& e2 = k < p2.size() ? p2[k] : empty_edge;

            e.cp.x = static_cast<int>(lerp<float>(e1.cp.x, e2.cp.x, ratio));
            e.cp.y = static_cast<int>(lerp<float>(e1.cp.y, e2.cp.y, ratio));
            e.ap.x = static_cast<int>(lerp<float>(e1.ap.x, e2.ap.x, ratio));
            e.ap.y = static_cast<int>(lerp<float>(e1.ap.y, e2.ap.y, ratio));
            ++k;

            if (p2.size() <= k) {
                k = 0;
                ++n;
            }
        }
    }
}

unsigned
ShapeRecord::readStyleChange(SWFStream& in, size_t num_style_bits, size_t numStyles)
{
    if (!num_style_bits) {
    	return 0;
    }

    in.ensureBits(num_style_bits);
    unsigned style = in.read_uint(num_style_bits);

    if ( style > numStyles ) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("Invalid fill style %1% in "
                           "style change record - %2% defined. "
                           "Set to 0."), style, numStyles);
        );
        style = 0;
    }

    return style;
}

void
ShapeRecord::read(SWFStream& in, SWF::TagType tag, movie_definition& m,
        const RunResources& r)
{

    /// TODO: is this correct?
    const bool styleInfo = (tag == SWF::DEFINESHAPE ||
                            tag == SWF::DEFINESHAPE2 ||
                            tag == SWF::DEFINESHAPE3 ||
                            tag == SWF::DEFINESHAPE4 ||
                            tag == SWF::DEFINESHAPE4_);

    Subshape subshape;
    if (!_subshapes.empty()) {
    	// This is a little naughty. In case we're reading DEFINEMORPH, we'll
    	// have been provided with styles, which are now copied....
    	subshape = _subshapes.front();
    	_subshapes.clear();
    }

    if (styleInfo) {
        _bounds = readRect(in);
    
        IF_VERBOSE_PARSE(
            std::string b = _bounds.toString();
            log_parse(_("  bound SWFRect: %s"), b);
        );
    
        // TODO: Store and use these. Unfinished.
        if (tag == SWF::DEFINESHAPE4 || tag == SWF::DEFINESHAPE4_) {
            const SWFRect tbound = readRect(in);
            UNUSED(tbound);
            in.ensureBytes(1);
            static_cast<void>(in.read_u8());
            LOG_ONCE(log_unimpl("DEFINESHAPE4 edge boundaries and scales"));
        }
    
        readFillStyles(subshape.fillStyles(), in, tag, m, r);
        readLineStyles(subshape.lineStyles(), in, tag, m, r);
    }

    if (tag == SWF::DEFINEFONT || tag == SWF::DEFINEFONT2 ) {
        assert(!styleInfo);
    }
    
    // Use read_u8 to force alignment.
    in.ensureBytes(1);
    boost::uint8_t num_bits = in.read_u8();
    int num_fill_bits = (num_bits & 0xF0) >> 4;
    int num_line_bits = (num_bits & 0x0F);
    
    IF_VERBOSE_PARSE(
        log_parse(_("  ShapeRecord(%s): fillbits %d, linebits %d"),
            tag, num_fill_bits, num_line_bits);
    );
    
    if ( !num_fill_bits && !num_line_bits ) {
        /// When reading font glyphs it happens to read 1 byte
        /// past end boundary of a glyph due to fill/line bits being
        /// zero.
        ///
        /// Generally returning here seems to break morphs:
        ///  http://savannah.gnu.org/bugs/?21747
        /// And other normal shapes:
        ///  http://savannah.gnu.org/bugs/?21923
        ///  http://savannah.gnu.org/bugs/?22000
        ///
        /// So for now we only return if NOT reading a morph shape.
        /// Pretty ugly... till next bug report.
        ///
        ///
        if (tag == SWF::DEFINEFONT || tag == SWF::DEFINEFONT2 || 
                tag == SWF::DEFINEFONT3) {
            log_debug("Skipping glyph read, being fill and line bits zero. "
                    "SWF tag is %d.", tag);
            return;
        }
    }
    
    // These are state variables that keep the
    // current position & style of the shape
    // outline, and vary as we read the edge data.
    //
    // At the moment we just store each edge with
    // the full necessary info to render it, which
    // is simple but not optimally efficient.
    int   x = 0, y = 0;
    Path  current_path;

#define SHAPE_LOG 0
    
    // SHAPERECORDS
    for (;;) {
        in.ensureBits(1);
        bool isEdgeRecord = in.read_bit();
        if (!isEdgeRecord) {
            // Parse the record.
            in.ensureBits(5);
            int flags = in.read_uint(5);
            if (flags == SHAPE_END) {  
                // Store the current path if any.
                if (! current_path.empty()) {
                    subshape.paths().push_back(current_path);
                    current_path.m_edges.resize(0);
                    _subshapes.push_back(subshape);
                    subshape.clear();
                }
                break;
            }
            if (flags & SHAPE_MOVE) {  
                // Store the current path if any, and prepare a fresh one.
                if (! current_path.empty()) {
                    subshape.paths().push_back(current_path);
                    current_path.m_edges.resize(0);
                }
                in.ensureBits(5);
                int num_move_bits = in.read_uint(5);
                in.ensureBits(2 * num_move_bits);
                int move_x = in.read_sint(num_move_bits);
                int move_y = in.read_sint(num_move_bits);
    
                x = move_x;
                y = move_y;
    
                // Set the beginning of the path.
                current_path.ap.x = x;
                current_path.ap.y = y;
    
#if SHAPE_LOG
                IF_VERBOSE_PARSE(
                    log_parse(_("  Shape read: moveto %d %d"), x, y);
                );
#endif
            }
            if ((flags & SHAPE_FILLSTYLE0_CHANGE) && num_fill_bits > 0) {
                // FillStyle_0_change = 1;
                if (! current_path.empty()) {
                    subshape.paths().push_back(current_path);
                    current_path.m_edges.resize(0);
                    current_path.ap.x = x;
                    current_path.ap.y = y;
                }

                unsigned style = readStyleChange(in, num_fill_bits,
                    (tag == SWF::DEFINEFONT || tag == SWF::DEFINEFONT2) ? 1 : subshape.fillStyles().size());

                current_path.setLeftFill(style);
#if SHAPE_LOG
                IF_VERBOSE_PARSE(
                     log_parse(_("  Shape read: fill0 (left) = %d"),
                         current_path.getLeftFill());
                );
#endif
            }
            if ((flags & SHAPE_FILLSTYLE1_CHANGE) && num_fill_bits > 0) {
                // FillStyle_1_change = 1;
                if (! current_path.empty()) {
                    subshape.paths().push_back(current_path);
                    current_path.m_edges.resize(0);
                    current_path.ap.x = x;
                    current_path.ap.y = y;
                }
                unsigned style = readStyleChange(in, num_fill_bits,
                    (tag == SWF::DEFINEFONT || tag == SWF::DEFINEFONT2) ? 1 : subshape.fillStyles().size());

                current_path.setRightFill(style);
#if SHAPE_LOG
                IF_VERBOSE_PARSE (
                    log_parse(_("  Shape read: fill1 (right) = %d"),
                        current_path.getRightFill());
                );
#endif
            }
            if ((flags & SHAPE_LINESTYLE_CHANGE) && num_line_bits > 0) {
                // line_style_change = 1;
                if (! current_path.empty()) {
                    subshape.paths().push_back(current_path);
                    current_path.m_edges.resize(0);
                    current_path.ap.x = x;
                    current_path.ap.y = y;
                }
                unsigned style = readStyleChange(in, num_line_bits,
                    (tag == SWF::DEFINEFONT || tag == SWF::DEFINEFONT2) ? 1 : subshape.lineStyles().size());

                current_path.setLineStyle(style);
#if SHAPE_LOG
                IF_VERBOSE_PARSE(
                    log_parse(_("ShapeRecord: line %d"), 
                        current_path.getLineStyle());
                )
#endif
            }
            if (flags & SHAPE_HAS_NEW_STYLES) {
                if (!styleInfo) {
                    IF_VERBOSE_MALFORMED_SWF(
                         log_swferror("Unexpected HasNewStyle flag in tag "
                             "%d shape record", tag);
                    );
                    continue;
                }
                IF_VERBOSE_PARSE (
                    log_parse(_("ShapeRecord: more fill styles"));
                );
    
                // Store the current path if any.
                if (! current_path.empty()) {
                    subshape.paths().push_back(current_path);
                    current_path.clear();
                }
    
                _subshapes.push_back(subshape);
                subshape.clear();
    
                readFillStyles(subshape.fillStyles(), in, tag, m, r);
                readLineStyles(subshape.lineStyles(), in, tag, m, r);
    
                in.ensureBits(8);
                num_fill_bits = in.read_uint(4);
                num_line_bits = in.read_uint(4);
            }
        } else {
            // EDGERECORD
            in.ensureBits(1);
            bool straight_edge = in.read_bit();
            int num_bits = 2 + in.read_uint(4);
            if (!straight_edge) {
                in.ensureBits(4);

                // curved edge
                in.ensureBits(4 * num_bits);
                int cx = x + in.read_sint(num_bits);
                int cy = y + in.read_sint(num_bits);
                int ax = cx + in.read_sint(num_bits);
                int ay = cy + in.read_sint(num_bits);
    
#if SHAPE_LOG
                IF_VERBOSE_PARSE (
                    log_parse(_("ShapeRecord: curved edge "
                            "%d %d - %d %d - %d %d"), x, y, cx, cy, ax, ay);
                );
#endif
                current_path.drawCurveTo(cx, cy, ax, ay);
                x = ax;
                y = ay;
            } else {
                // straight edge
                in.ensureBits(5);
                bool line_flag = in.read_bit();
                int dx = 0, dy = 0;
                if (line_flag)
                {
                    // General line.
                    in.ensureBits(2 * num_bits);
                    dx = in.read_sint(num_bits);
                    dy = in.read_sint(num_bits);
                } else {
                    in.ensureBits(1);
                    bool vert_flag = in.read_bit();
                    if (!vert_flag) {
                        // Horizontal line.
                        in.ensureBits(num_bits);
                        dx = in.read_sint(num_bits);
                    } else {
                        // Vertical line.
                        in.ensureBits(num_bits);
                        dy = in.read_sint(num_bits);
                    }
                }
    
#if SHAPE_LOG
                IF_VERBOSE_PARSE (
                     log_parse(_("ShapeRecord: straight edge "
                             "%d %d - %d %d"), x, y, x + dx, y + dy);
                );
#endif
                current_path.drawLineTo(x + dx, y + dy);

                x += dx;
                y += dy;
            }
        }
    }
    
    if (!styleInfo) {
        // TODO: performance would be improved by computing
        //       the bounds as edges are parsed.
    	_bounds.set_null();
    	for (Subshapes::const_iterator it = _subshapes.begin(),
    			end = _subshapes.end(); it != end; ++it) {
            SWFRect bounds = it->computeBounds(m.get_version());
            _bounds.expand_to_rect(bounds);
    	}
    }

#ifdef GNASH_DEBUG_SHAPE_BOUNDS
    else {
        SWFRect computedBounds;
        computeBounds(computedBounds, _paths, _lineStyles, m.get_version());
        if ( computedBounds != _bounds )
        {
            log_debug("Shape object read for tag %d contained embedded "
                    "bounds %s, while we computed bounds %s",
                    tag, _bounds, computedBounds);
        }
    }
#endif
}

namespace {

// Read fill styles, and push them onto the given style array.
void
readFillStyles(ShapeRecord::FillStyles& styles, SWFStream& in,
         SWF::TagType tag, movie_definition& m, const RunResources& /*r*/)
{
    in.ensureBytes(1);
    boost::uint16_t fillcount = in.read_u8();
    if (tag != SWF::DEFINESHAPE) {
        if (fillcount == 0xff) {
            in.ensureBytes(2);
            fillcount = in.read_u16();
        }
    }

    IF_VERBOSE_PARSE(
        log_parse(_("  fill styles: %1%"), fillcount);
    );

    // Read the styles.
    styles.reserve(styles.size() + fillcount);
    for (boost::uint16_t i = 0; i < fillcount; ++i) {
        OptionalFillPair fp = readFills(in, tag, m, false);
        styles.push_back(fp.first);
        IF_VERBOSE_PARSE(
            log_parse(_("  Read fill: %1%"), fp.first);
        );
    }
}

// Read line styles and push them onto the back of the given array.
void
readLineStyles(ShapeRecord::LineStyles& styles, SWFStream& in,
        SWF::TagType tag, movie_definition& md, const RunResources& r)
{
    in.ensureBytes(1);
    int line_style_count = in.read_u8();

    IF_VERBOSE_PARSE(
        log_parse(_("  readLineStyles: count = %d"), line_style_count);
    );

    if (line_style_count == 0xFF) {
        in.ensureBytes(2);
        line_style_count = in.read_u16();
        IF_VERBOSE_PARSE(
            log_parse(_("  readLineStyles: count2 = %d"), line_style_count);
        );
    }

    // Read the styles.
    for (int i = 0; i < line_style_count; i++) {
        styles.resize(styles.size() + 1);
        styles.back().read(in, tag, md, r);
    }
}



} // anonymous namespace

std::ostream&
operator<<(std::ostream& o, const ShapeRecord& sh)
{
    o << boost::format("Shape Record: bounds %1%") % sh.getBounds();


    for (ShapeRecord::Subshapes::const_iterator it = sh.subshapes().begin(),
         end = sh.subshapes().end(); it != end; ++it) {

        const ShapeRecord::FillStyles& fills = it->fillStyles();
        std::copy(fills.begin(), fills.end(),
                std::ostream_iterator<FillStyle>(o, ","));
	}

    return o;
}

} // namespace SWF
} // namespace gnash

