// Shape.cpp:  Mouse/Character handling, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include "MorphShape.h"
#include "GnashNumeric.h"
#include "VM.h"
#include "fill_style.h"
#include "Geometry.h"

namespace gnash
{

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
    const Edge& getNextEdge()
    {
        const Edge& ret = _paths[_currpath][_curredge];
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


MorphShape::MorphShape(morph_character_def* def, DisplayObject* parent, int id)
    :
    DisplayObject(parent, id),
    _def(def),
    _fillStyles(_def->shape1().fillStyles()),
    _lineStyles(_def->shape1().lineStyles()),
    _paths(_def->shape1().paths()),
    _bounds(def->shape1().get_bound())
{
}

void
MorphShape::stagePlacementCallback(as_object* initObj)
{
    assert(!initObj);
    if (get_ratio()) morph();
    _vm.getRoot().addLiveChar(this);
}

bool
MorphShape::pointInShape(boost::int32_t x, boost::int32_t y) const
{
    SWFMatrix wm = getWorldMatrix();
    SWFMatrix wm_inverse = wm.invert();
    point lp(x, y);
    wm_inverse.transform(lp);
    
    // FIXME: if the shape contains non-scaled strokes
    //        we can't rely on boundary itself for a quick
    //        way out. Bounds supposedly already include
    //        thickness, so we might keep a flag telling us
    //        whether *non_scaled* strokes are present
    //        and if not still use the boundary check.
    // NOTE: just skipping this test breaks a corner-case
    //       in DrawingApiTest (kind of a fill-leakage making
    //       the collision detection find you inside a self-crossing
    //       shape).
    if (!_bounds.point_test(lp.x, lp.y)) return false;

    return geometry::pointTest(_paths, _lineStyles, lp.x, lp.y, wm);
}

void  
MorphShape::display()
{
    _def->display(*this); 
    clear_invalidated();
}

rect
MorphShape::getBounds() const {
    return _bounds;
}

void
MorphShape::morph()
{
    
    const double ratio = get_ratio() / 65535.0;

    const shape_character_def& shape1 = _def->shape1();
    const shape_character_def& shape2 = _def->shape2();

    // bounds
    _bounds.set_lerp(shape1.get_bound(), shape2.get_bound(), ratio);

    // fill styles
    const FillStyles::const_iterator fs1 = shape1.fillStyles().begin();
    const FillStyles::const_iterator fs2 = shape2.fillStyles().begin();

    std::for_each(_fillStyles.begin(), _fillStyles.end(),
            Lerp<FillStyles>(fs1, fs2, ratio));

    // line styles
    const LineStyles::const_iterator ls1 = shape1.lineStyles().begin();
    const LineStyles::const_iterator ls2 = shape2.lineStyles().begin();

    std::for_each(_lineStyles.begin(), _lineStyles.end(),
            Lerp<LineStyles>(ls1, ls2, ratio));

    // This is used for cases in which number
    // of paths in start shape and end shape are not
    // the same.
    const Path empty_path;
    const Edge empty_edge;

    // shape
    const Paths& paths1 = shape1.paths();
    const Paths& paths2 = shape2.paths();
    for (size_t i = 0, k = 0, n = 0; i < _paths.size(); i++)
    {
        Path& p = _paths[i];
        const Path& p1 = i < paths1.size() ? paths1[i] : empty_path;
        const Path& p2 = n < paths2.size() ? paths2[n] : empty_path;

        const float new_ax = flerp(p1.ap.x, p2.ap.x, ratio);
        const float new_ay = flerp(p1.ap.y, p2.ap.y, ratio);

        p.reset(new_ax, new_ay, p1.getLeftFill(),
                p2.getRightFill(), p1.getLineStyle());

        //  edges;
        const size_t len = p1.size();
        p.m_edges.resize(len);

        for (size_t j=0; j < p.size(); j++)
        {
            Edge& e = p[j];
            const Edge& e1 = j < p1.size() ? p1[j] : empty_edge;

            const Edge& e2 = k < p2.size() ? p2[k] : empty_edge;

            e.cp.x = static_cast<int>(flerp(e1.cp.x, e2.cp.x, ratio));
            e.cp.y = static_cast<int>(flerp(e1.cp.y, e2.cp.y, ratio));
            e.ap.x = static_cast<int>(flerp(e1.ap.x, e2.ap.x, ratio));
            e.ap.y = static_cast<int>(flerp(e1.ap.y, e2.ap.y, ratio));
            ++k;

            if (p2.size() <= k) {
                k = 0;
                ++n;
            }
        }
    }
}

void
MorphShape::advance()
{
    set_invalidated();
    morph();
}

} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
