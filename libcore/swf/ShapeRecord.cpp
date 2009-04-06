
#include "ShapeRecord.h"
#include "swf.h"
#include "SWFStream.h"
#include "movie_definition.h"
#include "Geometry.h"

#include <vector>

namespace gnash {
namespace SWF {

// Forward declarations
namespace {
    void readFillStyles(ShapeRecord::FillStyles& styles, SWFStream& in,
        SWF::TagType tag, movie_definition& md);
    void readLineStyles(ShapeRecord::LineStyles& styles, SWFStream& in,
        SWF::TagType tag, movie_definition& md);
    void computeBounds(rect& bounds, const ShapeRecord::Paths& paths,
        const ShapeRecord::LineStyles& lineStyles, int swfVersion);
}

ShapeRecord::ShapeRecord(SWFStream& in, SWF::TagType tag, movie_definition& m)
{
    read(in, tag, m);
}

void
ShapeRecord::read(SWFStream& in, SWF::TagType tag, movie_definition& m)
{

    /// What should this be?
    //
    // False: DEFINEFONT DEFINEFONT2, DEFINEMORPHSHAPE DEFINEMORPHSHAPE2
    const bool styleInfo = 
        (tag == SWF::DEFINESHAPE || tag == SWF::DEFINESHAPE2);

    if (styleInfo)
    {
        _bounds.read(in);
    
        IF_VERBOSE_PARSE(
            std::string b = _bounds.toString();
            log_parse(_("  bound rect: %s"), b);
        );
    
        // TODO: Store and use these. Unfinished.
        if (tag == SWF::DEFINESHAPE4 || tag == SWF::DEFINESHAPE4_)
        {
            rect tbound;
            tbound.read(in);
            in.ensureBytes(1);
            static_cast<void>(in.read_u8());
            LOG_ONCE(log_unimpl("DEFINESHAPE4 edge boundaries and scales"));
        }
    
        readFillStyles(_fillStyles, in, tag, m);
        readLineStyles(_lineStyles, in, tag, m);
    }

    if (tag == SWF::DEFINEFONT || tag == SWF::DEFINEFONT2 )
    {
        assert(!styleInfo);
    }
    
    // Use read_u8 to force alignment.
    in.ensureBytes(1);
    boost::uint8_t num_bits = in.read_u8();
    int num_fill_bits = (num_bits & 0xF0) >> 4;
    int num_line_bits = (num_bits & 0x0F);
    
    IF_VERBOSE_PARSE(
        log_parse(_("  shape_character_def read: nfillbits = %d, "
                "nlinebits = %d"), num_fill_bits, num_line_bits);
    );
    
    if ( !num_fill_bits && !num_line_bits )
    {
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
                tag == SWF::DEFINEFONT3)
        {
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
    int fill_base = 0;
    int line_base = 0;
    int   x = 0, y = 0;
    Path  current_path;

#define SHAPE_LOG 0
    
    // SHAPERECORDS
    for (;;)
    {
        in.ensureBits(1);
        bool isEdgeRecord = in.read_bit();
        if (!isEdgeRecord)
        {
            // Parse the record.
            in.ensureBits(5);
            int flags = in.read_uint(5);
            if (flags == SHAPE_END)
            {  
                // Store the current path if any.
                if (! current_path.empty())
                {
                    _paths.push_back(current_path);
                    current_path.m_edges.resize(0);
                }
                break;
            }
            if (flags & SHAPE_MOVE)
            {  
                // Store the current path if any, and prepare a fresh one.
                if (! current_path.empty())
                {
                    _paths.push_back(current_path);
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
            if ((flags & SHAPE_FILLSTYLE0_CHANGE) && num_fill_bits > 0)
            {
                // fill_style_0_change = 1;
                if (! current_path.empty())
                {
                    _paths.push_back(current_path);
                    current_path.m_edges.resize(0);
                    current_path.ap.x = x;
                    current_path.ap.y = y;
                }
                in.ensureBits(num_fill_bits);
                unsigned style = in.read_uint(num_fill_bits);
                if (style > 0)
                {
                    style += fill_base;
                }
    
                if (tag == SWF::DEFINEFONT || tag == SWF::DEFINEFONT2)
                {
                    if ( style > 1 )          // 0:hide 1:renderer
                    {
                        IF_VERBOSE_MALFORMED_SWF(
                             log_swferror(_("Invalid fill style %d in "
                                     "fillStyle0Change record for font tag "
                                     "(0 or 1 valid). Set to 0."), style);
                        );
                        style = 0;
                    }
                }
                else
                {
                    // 1-based index
                    if ( style > _fillStyles.size() )
                    {
                        IF_VERBOSE_MALFORMED_SWF(
                             log_swferror(_("Invalid fill style %d in "
                                     "fillStyle0Change record - %d defined. "
                                     "Set to 0."), style, _fillStyles.size());
                        );
                        style = 0;
                    }
                }
    
                current_path.setLeftFill(style);
#if SHAPE_LOG
                IF_VERBOSE_PARSE(
                     log_parse(_("  Shape read: fill0 (left) = %d"),
                         current_path.getLeftFill());
                );
#endif
            }
            if ((flags & SHAPE_FILLSTYLE1_CHANGE) && num_fill_bits > 0)
            {
                // fill_style_1_change = 1;
                if (! current_path.empty())
                {
                    _paths.push_back(current_path);
                    current_path.m_edges.resize(0);
                    current_path.ap.x = x;
                    current_path.ap.y = y;
                }
                in.ensureBits(num_fill_bits);
                unsigned style = in.read_uint(num_fill_bits);
                if (style > 0)
                {
                    style += fill_base;
                }
    
                if (tag == SWF::DEFINEFONT || tag == SWF::DEFINEFONT2)
                {
                    if ( style > 1 )          // 0:hide 1:renderer
                    {
                        IF_VERBOSE_MALFORMED_SWF(
                             log_swferror(_("Invalid fill style %d in "
                                     "fillStyle1Change record for font tag "
                                     "(0 or 1 valid). Set to 0."), style);
                        );
                        style = 0;
                    }
                }
                else
                {
                    // 1-based index
                    if ( style > _fillStyles.size() )
                    {
                        IF_VERBOSE_MALFORMED_SWF(
                            log_swferror(_("Invalid fill style %d in "
                                    "fillStyle1Change record - %d defined. "
                                    "Set to 0."), style, _fillStyles.size());
                        );
                        style = 0;
                    }
                }
                current_path.setRightFill(style);
#if SHAPE_LOG
                IF_VERBOSE_PARSE (
                    log_parse(_("  Shape read: fill1 (right) = %d"),
                        current_path.getRightFill());
                );
#endif
            }
            if ((flags & SHAPE_LINESTYLE_CHANGE) && num_line_bits > 0)
            {
                // line_style_change = 1;
                if (! current_path.empty())
                {
                    _paths.push_back(current_path);
                    current_path.m_edges.resize(0);
                    current_path.ap.x = x;
                    current_path.ap.y = y;
                }
                in.ensureBits(num_line_bits);
                unsigned style = in.read_uint(num_line_bits);
                if (style > 0)
                {
                    style += line_base;
                }
                if (tag == SWF::DEFINEFONT || tag == SWF::DEFINEFONT2)
                {
                    if ( style > 1 )          // 0:hide 1:renderer
                    {
                        IF_VERBOSE_MALFORMED_SWF(
                            log_swferror(_("Invalid line style %d in "
                                    "lineStyleChange record for font tag "
                                    "(0 or 1 valid). Set to 0."), style);
                        );
                        style = 0;
                    }
                }
                else {
                    // 1-based index
                    if (style > _lineStyles.size()) {
                        IF_VERBOSE_MALFORMED_SWF(
                            log_swferror(_("Invalid fill style %d in "
                                    "lineStyleChange record - %d defined. "
                                    "Set to 0."), style, _lineStyles.size());
                        );
                        style = 0;
                    }
                }
                current_path.setLineStyle(style);
#if SHAPE_LOG
                IF_VERBOSE_PARSE(
                    log_parse(_("  Shape_read: line = %d"), 
                        current_path.getLineStyle());
                )
#endif
            }
            if (flags & SHAPE_HAS_NEW_STYLES)
            {
                if (!styleInfo)
                {
                    IF_VERBOSE_MALFORMED_SWF(
                         log_swferror("Unexpected HasNewStyle flag in tag "
                             "%d shape record", tag);
                    );
                    continue;
                }
                IF_VERBOSE_PARSE (
                    log_parse(_("  Shape read: more fill styles"));
                );
    
                // Store the current path if any.
                if (! current_path.empty())
                {
                    _paths.push_back(current_path);
                    current_path.clear();
                }
    
                // Tack on an empty path signalling a new shape.
                // @@ need better understanding of whether this is correct??!?!!
                // @@ i.e., we should just start a whole new shape here, right?
                _paths.push_back(Path());
                _paths.back().m_new_shape = true;
    
                fill_base = _fillStyles.size();
                line_base = _lineStyles.size();
                readFillStyles(_fillStyles, in, tag, m);
                readLineStyles(_lineStyles, in, tag, m);
    
                in.ensureBits(8);
                num_fill_bits = in.read_uint(4);
                num_line_bits = in.read_uint(4);
            }
        }
        else
        {
            // EDGERECORD
            in.ensureBits(1);
            bool edge_flag = in.read_bit();
            if (edge_flag == 0)
            {
                in.ensureBits(4);
                int num_bits = 2 + in.read_uint(4);
                // curved edge
                in.ensureBits(4 * num_bits);
                int cx = x + in.read_sint(num_bits);
                int cy = y + in.read_sint(num_bits);
                int ax = cx + in.read_sint(num_bits);
                int ay = cy + in.read_sint(num_bits);
    
#if SHAPE_LOG
                IF_VERBOSE_PARSE (
                    log_parse(_("  Shape read: curved edge   = "
                            "%d %d - %d %d - %d %d"), x, y, cx, cy, ax, ay);
                );
#endif
                current_path.m_edges.push_back(Edge(cx, cy, ax, ay));
                x = ax;
                y = ay;
            }
            else
            {
                // straight edge
                in.ensureBits(5);
                int num_bits = 2 + in.read_uint(4);
                bool  line_flag = in.read_bit();
                int dx = 0, dy = 0;
                if (line_flag)
                {
                    // General line.
                    in.ensureBits(2 * num_bits);
                    dx = in.read_sint(num_bits);
                    dy = in.read_sint(num_bits);
                }
                else
                {
                    in.ensureBits(1);
                    bool vert_flag = in.read_bit();
                    if (vert_flag == 0)
                    {
                        // Horizontal line.
                        in.ensureBits(num_bits);
                        dx = in.read_sint(num_bits);
                    }
                    else
                    {
                        // Vertical line.
                        in.ensureBits(num_bits);
                        dy = in.read_sint(num_bits);
                    }
                }
    
#if SHAPE_LOG
                IF_VERBOSE_PARSE (
                     log_parse(_("  Shape_read: straight edge = "
                             "%d %d - %d %d"), x, y, x + dx, y + dy);
                );
#endif
                current_path.m_edges.push_back(Edge(x + dx, y + dy,
                            x + dx, y + dy));
                x += dx;
                y += dy;
            }
        }
    }
    
    if (!styleInfo)
    {
        // TODO: performance would be improved by computing
        //       the bounds as edges are parsed.
        computeBounds(_bounds, _paths, _lineStyles, m.get_version());
    }
#ifdef GNASH_DEBUG_SHAPE_BOUNDS
    else
    {
        rect computedBounds;
        computeBounds(computedBounds, _paths, _lineStyles, m->get_version());
        if ( computedBounds != m_bounds )
        {
            log_debug("Shape DisplayObject read for tag %d contained embedded "
                    "bounds %s, while we computed bounds %s",
                    tag, m_bound, computedBounds);
        }
    }
#endif
}

namespace {

// Read fill styles, and push them onto the given style array.
void
readFillStyles(ShapeRecord::FillStyles& styles, SWFStream& in,
                 SWF::TagType tag, movie_definition& m)
{
    in.ensureBytes(1);
    boost::uint16_t fill_style_count = in.read_u8();
    if (tag != SWF::DEFINESHAPE)
    {
        if (fill_style_count == 0xFF)
        {
            in.ensureBytes(2);
            fill_style_count = in.read_u16();
        }
    }

    IF_VERBOSE_PARSE (
        log_parse(_("  readFillStyles: count = %u"), fill_style_count);
    );

    // Read the styles.
    styles.reserve(styles.size()+fill_style_count);
    for (boost::uint16_t i = 0; i < fill_style_count; ++i)
    {
        // TODO: add a fill_style constructor directly reading from stream
        fill_style fs;
        fs.read(in, tag, m);
        styles.push_back(fs);
    }
}

// Read line styles and push them onto the back of the given array.
void
readLineStyles(ShapeRecord::LineStyles& styles, SWFStream& in,
        SWF::TagType tag, movie_definition& md)
{
    in.ensureBytes(1);
    int line_style_count = in.read_u8();

    IF_VERBOSE_PARSE(
        log_parse(_("  readLineStyles: count = %d"), line_style_count);
    );

    if (line_style_count == 0xFF)
    {
        in.ensureBytes(2);
        line_style_count = in.read_u16();
        IF_VERBOSE_PARSE(
            log_parse(_("  readLineStyles: count2 = %d"), line_style_count);
        );
    }

    // Read the styles.
    for (int i = 0; i < line_style_count; i++)
    {
        styles.resize(styles.size() + 1);
        styles.back().read(in, tag, md);
    }
}

// Find the bounds of this shape, and store them in the given rectangle.
void
computeBounds(rect& bounds, const ShapeRecord::Paths& paths,
        const ShapeRecord::LineStyles& lineStyles, int swfVersion)
{
    bounds.set_null();

    for (unsigned int i = 0; i < paths.size(); i++)
    {
        const Path& p = paths[i];

        unsigned thickness = 0;
        if ( p.m_line )
        {
            // For glyph shapes m_line is allowed to be 1
            // while no defined line styles are allowed.
            if (lineStyles.empty()) {
                // This is either a Glyph, for which m_line==1 is valid
                // or a bug in the parser, which we have no way to
                // check at this time
                assert(p.m_line == 1);
            }
            else
            {
                thickness = lineStyles[p.m_line-1].getThickness();
            }
        }
        p.expandBounds(bounds, thickness, swfVersion);
    }
}


} // anonymous namespace

} // namespace SWF
} // namespace gnash

