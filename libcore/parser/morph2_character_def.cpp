// morph2_character_def.cpp:   Load and render morphing shapes, for Gnash.
//
//   Copyright (C) 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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


// Based on the public domain morph2.cpp of:
// Thatcher Ulrich <tu@tulrich.com>, Mike Shaver <shaver@off.net> 2003,
// Vitalij Alexeev <tishka92@mail.ru> 2004.

#include "morph2_character_def.h"
#include "SWFStream.h"
#include "render.h"
#include "movie_definition.h"
#include "MovieClip.h"


namespace gnash {

/// Functors for path and style manipulation.
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
        st.set_lerp(*_style1, *_style2, _ratio);
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
    typedef shape_character_def::Paths Paths;
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
	const edge& getNextEdge()
	{
		const edge& ret = _paths[_currpath][_curredge];
        if ( ++_curredge >= _paths[_currpath].size() )
		{
			if ( ++_currpath >= _paths.size() )
			{
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

morph2_character_def::morph2_character_def()
    :
    m_shape1(new shape_character_def),
    m_shape2(new shape_character_def),
    m_last_ratio(-1.0f)
{
}


morph2_character_def::~morph2_character_def()
{
}

void
morph2_character_def::display(character* inst)
{

    const double ratio = inst->get_ratio() / 65535.0;

    // bounds
    rect new_bound;
    new_bound.set_lerp(m_shape1->get_bound(), m_shape2->get_bound(), ratio);
    set_bound(new_bound);

    // fill styles
    const FillStyles::const_iterator fs1 = m_shape1->fillStyles().begin();
    const FillStyles::const_iterator fs2 = m_shape2->fillStyles().begin();

    std::for_each(_fill_styles.begin(), _fill_styles.end(),
            Lerp<FillStyles>(fs1, fs2, ratio));

    // line styles
    const LineStyles::const_iterator ls1 = m_shape1->lineStyles().begin();
    const LineStyles::const_iterator ls2 = m_shape2->lineStyles().begin();

    std::for_each(_line_styles.begin(), _line_styles.end(),
            Lerp<LineStyles>(ls1, ls2, ratio));

    // This is used for cases in which number
    // of paths in start shape and end shape are not
    // the same.
    path empty_path;
    edge empty_edge;

    // shape
    const Paths& paths1 = m_shape1->get_paths();
    const Paths& paths2 = m_shape2->get_paths();
    for (size_t i = 0, k = 0, n = 0; i < _paths.size(); i++)
    {
        path& p = _paths[i];
        const path& p1 = i < paths1.size() ? paths1[i] : empty_path;
        const path& p2 = n < paths2.size() ? paths2[n] : empty_path;

        const float new_ax = utility::flerp(p1.ap.x, p2.ap.x, ratio);
        const float new_ay = utility::flerp(p1.ap.y, p2.ap.y, ratio);

        p.reset(new_ax, new_ay, p1.getLeftFill(),
                p2.getRightFill(), p1.getLineStyle());

        //  edges;
        const size_t len = p1.size();
        p.m_edges.resize(len);

        for (size_t j=0; j < p.size(); j++)
        {
            edge& e = p[j];
            const edge& e1 = j < p1.size() ? p1[j] : empty_edge;
            const edge& e2 = k < p2.size() ? p2[k] : empty_edge;

            e.cp.x = static_cast<int>(utility::flerp(e1.cp.x, e2.cp.x, ratio));
            e.cp.y = static_cast<int>(utility::flerp(e1.cp.y, e2.cp.y, ratio));
            e.ap.x = static_cast<int>(utility::flerp(e1.ap.x, e2.ap.x, ratio));
            e.ap.y = static_cast<int>(utility::flerp(e1.ap.y, e2.ap.y, ratio));
            ++k;
            if (p2.size() <= k)
            {
                k = 0;
                ++n;
            }
        }
    }

    //  display
    render::draw_shape_character(this, inst);

}


void morph2_character_def::read(SWFStream& in, SWF::TagType tag,
        movie_definition& md)
{
    assert(tag == SWF::DEFINEMORPHSHAPE
        || tag == SWF::DEFINEMORPHSHAPE2
        || tag == SWF::DEFINEMORPHSHAPE2_);

    rect bound1, bound2;
    bound1.read(in);
    bound2.read(in);

    if (tag == SWF::DEFINEMORPHSHAPE2 ||
            tag == SWF::DEFINEMORPHSHAPE2_)
    {
        // TODO: Use these values.
        rect inner_bound1, inner_bound2;
        inner_bound1.read(in);
        inner_bound2.read(in);
        // This should be used -- first 6 bits reserved, then
        // 'non-scaling' stroke, then 'scaling' stroke -- these can be
        // used to optimize morphing.
        
        in.ensureBytes(1);
        static_cast<void>(in.read_u8());
    }

    in.ensureBytes(4);
    offset = in.read_u32();

    // Next line will throw ParserException on malformed SWF
    fill_style_count = in.read_variable_count();
    int i;
    fill_style fs1, fs2;
    for (i = 0; i < fill_style_count; ++i)
    {
        fs1.read(in, tag, md, &fs2);
        m_shape1->addFillStyle(fs1);
        m_shape2->addFillStyle(fs2);
    }

    line_style_count = in.read_variable_count();
    line_style ls1, ls2;
    for (i = 0; i < line_style_count; ++i)
    {
        ls1.read_morph(in, tag, md, &ls2);
        m_shape1->addLineStyle(ls1);
        m_shape2->addLineStyle(ls2);
    }

    m_shape1->read(in, tag, false, md);
    in.align();
    m_shape2->read(in, tag, false, md);

    // Set bounds as read in *this* tags rather then
    // the one computed from shape_character_def parser
    // (does it make sense ?)
    m_shape1->set_bound(bound1);
    m_shape2->set_bound(bound2);

    const shape_character_def::FillStyles& s1Fills = 
        m_shape1->fillStyles();

    const shape_character_def::LineStyles& s1Lines = 
        m_shape1->lineStyles();

    assert(s1Fills.size() == m_shape2->fillStyles().size());
    assert(s1Lines.size() == m_shape2->lineStyles().size());

    // setup array size
    _fill_styles.resize(s1Fills.size());
    unsigned int k;
    for (k = 0; k < _fill_styles.size(); k++)
    {
        fill_style& fs = _fill_styles[k];
        const fill_style& fs1 = s1Fills[k];
        fs.m_gradients.resize(fs1.m_gradients.size());
    }

    _line_styles.resize(s1Lines.size());
    _paths.resize(m_shape1->get_paths().size());

    unsigned edges_count1 = PathList::computeNumberOfEdges(
            m_shape1->get_paths());
    unsigned edges_count2 = PathList::computeNumberOfEdges(
            m_shape2->get_paths());

    IF_VERBOSE_PARSE(
      log_parse("morph: "
          "startShape(paths:%d, edges:%u), "
          "endShape(paths:%d, edges:%u)",
          m_shape1->get_paths().size(), edges_count1,
          m_shape2->get_paths().size(), edges_count2);
    );

    IF_VERBOSE_MALFORMED_SWF(
        // It is perfectly legal to have a different number of paths,
        // edges count should be the same instead
        if ( edges_count1 != edges_count2 )
        {
            log_swferror(_("Different number of edges "
                "in start (%u) and end (%u) shapes "
                "of a morph"),
                edges_count1, edges_count1);
        }

    );

}

} // namespace gnash

// Local Variables:
// mode: C++
// c-basic-offset: 8
// tab-width: 8
// indent-tabs-mode: t
// End:
