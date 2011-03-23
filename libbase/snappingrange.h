// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
// 

#ifndef GNASH_SNAPPINGRANGE_H
#define GNASH_SNAPPINGRANGE_H

#include "Range2d.h"

#include <vector>
#include <iterator>
#include <algorithm>
#include <ostream>
#include <boost/cstdint.hpp>

namespace gnash {

namespace geometry {

/// \brief
/// Snapping range class. Can hold a number of 2D ranges and combines 
/// ranges that come very close. This class is used for multiple invalidated
/// bounds calculation.
//
/// Additionally to merge intersecting ranges this class also "snaps" ranges
/// together which "probably" would be rendered in one single step. When the
/// area of the potential merged range is not much greater than the sum of
/// two single range areas, then these two ranges are merged together to one
/// single range. The "snap factor" defines how much bigger the merged area
/// can become to be a snap candidate (it's the maximum allowed factor that
/// the merged area can be greater than the sum).
///
/// Optimally the factor 1.0 would be best, but multiple snapping ranges also
/// increase rendering preprocessing overhead and the factor tries to equalize
/// this. The default value is usually fine for most situations.
///
/// The factor method makes sure that very close, but also very different 
/// shapes, don't get merged, like in the following example:
///
///                    +---+
///                    |   |        
///                    |   |        
///                    |   |        
///                    |   |        
///                    |   |
///                    +---+
///     +-----------------------------------+
///     |                                   |
///     +-----------------------------------+        
///
/// Merging these two ranges would create a much bigger range which in some
/// situations means that rendering is notably slower (for example, when 
/// there is a scaled bitmap behind these shapes).

// Forward declarations.
namespace {

    /// returns true when two ranges should be merged together
    template<typename T> inline bool snaptest(
            const geometry::Range2d<T>& range1,
            const geometry::Range2d<T>& range2, const float snapFactor);
}

template <typename T>
class SnappingRanges2d
{
public:
    typedef geometry::Range2d<T> RangeType;
    typedef std::vector<RangeType> RangeList; 
    typedef typename RangeList::size_type size_type;    

    template<typename U> friend std::ostream& operator<<(std::ostream& os,
                    const SnappingRanges2d<U>& r);
    
    SnappingRanges2d() 
        :
        _snapFactor(1.3f),
        _singleMode(false),
        _rangesLimit(50),
        _combineCounter(0)
    {
    }

    /// Templated copy constructor, for casting between range types
    template <typename U>
    SnappingRanges2d(const SnappingRanges2d<U>& from)
        :
        _snapFactor(from.getSnapFactor()), 
        _singleMode(from.getSingleMode()),
        _rangesLimit(from.getRangeCountLimit()),
        _combineCounter(0)
    {
        if (from.isWorld()) setWorld();
        else if (from.isNull()) setNull();
        else {
            // TODO: can we safely assume that the 'from' parameter was
            //             finalized ?

            // TODO: use visitor pattern !
            for (size_type i = 0, e = from.size(); i != e; ++i) {
                const Range2d<U>& r = from.getRange(i);
                RangeType rc(r);
                add(rc);
            }
        }
    }
    
    /// Sets the snapping factor (which must be > 1.0). Higher factors make
    /// the ranges more attractive for snapping. A good value is usually 1.3.
    void setSnapFactor(const float factor) {
        assert(factor > 1.0f);
        _snapFactor = factor;
    }
    
    float getSnapFactor() const {
        return _snapFactor;
    }
    
    /// if mode==true, then the snapping ranges will act like a normal Range2d
    void setSingleMode(const bool mode) {
        _singleMode = mode;
    }
    
    bool getSingleMode() const {
        return _singleMode;
    }    
    
    /// Sets the maximum number of ranges allowed (to avoid lots of small
    /// ranges)
    void setRangeCountLimit(const size_type limit) {
        _rangesLimit = limit;
    }
    
    size_type getRangeCountLimit() const {
        return _rangesLimit;
    }
    
    /// Copy the snapping settings from another ranges list, without
    /// copying the ranges itself
    void inheritConfig(const SnappingRanges2d<T>& from) {
        _snapFactor = from._snapFactor;
        _singleMode = from._singleMode;
    }
    
    /// Merge two ranges based on snaptest.
    struct ExpandToIfSnap
    {
    public:
        ExpandToIfSnap(const RangeType& rt, const float snapFactor)
            :
            _rt(rt),
            _snapFactor(snapFactor)
        {}
        
        bool operator()(RangeType& r) {
            if (snaptest(r, _rt, _snapFactor)) {
                r.expandTo(_rt);
                return false;
            }
            return true;
        }
    private:
        const RangeType& _rt;
        const float _snapFactor;
    };

    class Scale
    {
    public:
        Scale(const float scale) : _scale(scale) {}
        void operator()(RangeType& r) {
            r.scale(_scale);
        }
    private:
        const float _scale;
    };

    class GrowBy
    {
    public:
        GrowBy(const float factor) : _factor(factor) {}
        void operator()(RangeType& r) {
            r.growBy(_factor);
        }
    private:
        const float _factor;
    };

    class AddTo
    {
    public:
        AddTo(SnappingRanges2d<T>& us) : _this(us) {}
        void operator()(const RangeType& r) {
            _this.add(r);
        }
    private:
        SnappingRanges2d<T>& _this;
    };
    
    class IntersectsRange
    {
    public:
        IntersectsRange(const RangeType& range) : _range(range) {}
        bool operator()(const RangeType& us) {
            return us.intersects(_range);
        }
    private:
        const RangeType& _range;
    };
    
    class ContainsPoint
    {
    public:
        ContainsPoint(const T x, const T y) : _x(x), _y(y) {}
        bool operator()(const RangeType& us) {
            return us.contains(_x, _y);
        }
    private:
        const T _x, _y;
    };
    
    class ContainsRange
    {
    public:
        ContainsRange(const RangeType& range) : _range(range) {}
        bool operator()(const RangeType& us) {
            return us.contains(_range);
        }
    private:
        const RangeType& _range;
    };


    /// Add a Range to the set, merging when possible and appropriate
    void add(const RangeType& range) {
        if (range.isWorld()) {
            setWorld();
            return;
        }
        
        if (range.isNull()) return;
        
        if (_singleMode) {
            if (_ranges.empty()) _ranges.resize(1);
            _ranges[0].expandTo(range);
            return;
        }
        
        ExpandToIfSnap exp(range, _snapFactor);
        if (visit(exp)) return;

        // reached this point we need a new range 
        _ranges.push_back(range);
        
        combineRangesLazy();
    }
    
    /// combines two snapping ranges
    void add(const SnappingRanges2d<T>& other) {
        const RangeList& rl = other._ranges;
        std::for_each(rl.begin(), rl.end(), AddTo(*this));
    }

    /// Grows all ranges by the specified amount 
    void growBy(const T amount) {
    
        if (isWorld() || isNull()) return;
        
        std::for_each(_ranges.begin(), _ranges.end(), GrowBy(amount));
        combineRangesLazy();
    }

    /// Scale all ranges by the specified factor
    void scale(const float factor) {
    
        if (isWorld() || isNull()) return;
        
        std::for_each(_ranges.begin(), _ranges.end(), Scale(factor));
        combineRangesLazy();
    }
    
    /// Resets to NULL range
    void setNull() {
        _ranges.clear();
    }
    
    /// Resets to one range with world flags
    void setWorld() {
        if (isWorld()) return;
        _ranges.resize(1);
        _ranges[0].setWorld();
    }
    
    /// Returns true, when the ranges equal world range
    bool isWorld() const {
        return ((size()==1) && (_ranges.front().isWorld()));
    }
    
    /// Returns true, when there is no range
    bool isNull() const {
        return _ranges.empty();
    }
    
    /// Returns the number of ranges in the list
    size_type size() const {
        finalize();
        return _ranges.size();
    }
    
    /// Returns the range at the specified index
    const RangeType& getRange(size_type index) const {
        finalize();
        assert(index<size());
        return _ranges[index];
    }
    
    /// Return a range that surrounds *all* added ranges. This is used mainly
    /// for compatibilty issues. 
    RangeType getFullArea() const {
        RangeType range;
        
        range.setNull();
        
        int rcount = _ranges.size();
        
        for (int rno=0; rno<rcount; rno++) 
            range.expandTo(_ranges[rno]);
        
        return range;     
    }
    

    /// Returns true if any of the ranges intersect the given range
    //
    /// Note that a NULL range doesn't intersect anything
    /// and a WORLD range intersects everything except a NULL Range.
    ///
    bool intersects(const RangeType& r) const {
    
        finalize();
        return std::find_if(_ranges.begin(), _ranges.end(), IntersectsRange(r))
            != _ranges.end();
    }
    
    /// Returns true if any of the ranges contains the point
    bool contains(T x, T y) const {
    
        finalize();
        return std::find_if(_ranges.begin(), _ranges.end(), ContainsPoint(x, y))
            != _ranges.end();
    }

    /// Returns true if any of the ranges contains the range
    //
    /// Note that a NULL range is not contained in any range and
    /// a WORLD range is onluy contained in another WORLD range.
    ///
    bool contains(const RangeType& r) const {
    
        finalize();
        return std::find_if(_ranges.begin(), _ranges.end(), ContainsRange(r))
            != _ranges.end();
    }

    /// \brief
    /// Returns true if all ranges in the given SnappingRanges2d 
    /// are contained in at least one of the ranges composing this
    /// one.
    ///
    /// Note that a NULL range is not contained in any range and
    /// a WORLD range is onluy contained in another WORLD range.
    ///
    bool contains(const SnappingRanges2d<T>& o) const
    {
    
        finalize();
        // o.finalize(); // should I finalize the other range too ?

        // Null range set doesn't contain and isn't contained by anything
        if ( isNull() ) return false;
        if ( o.isNull() ) return false;

        // World range contains everything (except null ranges)
        if ( isWorld() ) return true;

        // This snappingrange is neither NULL nor WORLD
        // The other can still be WORLD, but in that case the
        // first iteration would return false
        //
        /// TODO: use a visitor !
        for (unsigned rno=0, rcount=o.size(); rno<rcount; rno++) 
        {
            RangeType r = o.getRange(rno);
            if ( ! contains(r) )
            {
                return false;
            }
        }
            
        return true;
    
    }
    
    
    /// Intersect this ranges list with the given ranges list,
    /// updating the current ranges list.
    /// Note this is currently a relatively expensive operation    
    /// for complex lists.
    ///
    void intersect(const SnappingRanges2d<T>& o) 
    {
        if (o.isNull()) {
            setNull();
            return;
        }
        
        if (o.isWorld()) return;
        
        // We create a new ranges set for each range in "o" and
        // then update ourselves with the *union* of these ranges.
        // Anybody knows a better method (in terms of efficieny) ?    
     
        std::vector<SnappingRanges2d<T> > list;
        list.reserve(o.size());
    
        //TODO: use a visitor !
        for (unsigned rno=0, rcount=o.size(); rno<rcount; rno++) {
            
            // add a copy of ourselves to the list
            list.push_back(*this);
            
            // intersect that copy with the single range
            list.back().intersect(o.getRange(rno));
            
        } 
        
        // update ourselves with the union of the "list"
        setNull();
        for (size_type lno=0, lcount=list.size(); lno<lcount; lno++) {
            add(list[lno]);
        }
                            
    }
    
    
    /// Intersects this ranges list with the given single range,
    /// updating the current ranges list.
    void intersect(const RangeType& r) 
    {
    
        finalize();

        if (isWorld()) {            // world intersection with X = X
            setNull();    
            add(r);
            return;
        }
        
        if (isNull()) return; // NULL will always remain NULL
        
        if (r.isNull()) {         // X intersection with NULL = NULL
            setNull();
            return;
        }
        
        if (r.isWorld()) return;    // X intersection with WORLD = X
        
        // TODO: use a vector (remember to walk in reverse dir.)
        for (int rno=_ranges.size()-1; rno>=0; rno--) {     
        
            RangeType newrange = Intersection(_ranges[rno], r);
            
            if (newrange.isNull())
                _ranges.erase(_ranges.begin() + rno);
            else             
                _ranges[rno] = newrange;
        }
    }
    
    /// Combines known ranges. Previously merged ranges may have come close
    /// to other ranges. Algorithm could be optimized. 
    void combineRanges() const {
    
        // makes no sense in single mode
        if (_singleMode) return;
    
        bool restart = true;
        
        _combineCounter = 0;
        
        while (restart) {
        
            int rcount = _ranges.size();

            restart=false;
        
            for (int i=0; i<rcount; i++) {
            
                for (int j=i+1; j<rcount; j++) {
                
                    if (snaptest(_ranges[i], _ranges[j], _snapFactor)) {
                        // merge i + j
                        _ranges[i].expandTo(_ranges[j]);
                        
                        _ranges.erase(_ranges.begin() + j);
                        
                        restart=true; // restart from beginning
                        break;
                        
                    } 
                } 
                
                if (restart) break;
            } 
        } 
        
        // limit number of ranges
        if (_ranges.size() > _rangesLimit) {
        
            // We found way too much ranges, so reduce to just one single range.
            // We could also double the factor and try again, but that probably
            // won't make much difference, so we avoid the trouble...
            
            RangeType single = getFullArea();            
            _ranges.resize(1);
            _ranges[0] = single;
        
        }
    
    }
    
    /// Visit the current Ranges set
    //
    /// Visitor functor will be invoked
    /// for each RangeType in the current set.
    /// 
    /// The visitor functor will 
    /// receive a RangeType reference; must return true if
    /// it wants next item or true to exit the loop.
    ///
    /// @return false if the visitor reached the end.
    template<class V> inline bool visit(V& visitor) const
    {
        typename RangeList::iterator it, e;
        for (it = _ranges.begin(), e = _ranges.end(); it != e; ++it) {
            if (!visitor(*it)) break;
        }
        return it != _ranges.end();
    }

    /// Visit the current Ranges set
    //
    /// Visitor functor will be invoked inconditionally
    /// for each RangeType in the current set.
    /// 
    /// The visitor functor will receive a RangeType reference.
    ///
    template<class V> inline void visitAll(V& visitor) const
    {
        for_each(_ranges.begin(), _ranges.end(), visitor);
    }
    
private:

    
    /// Calls combineRanges() once in a while, but not always. Avoids too many
    /// combineRanges() checks, which could slow down everything.
    void combineRangesLazy() {
        const size_type max = 5;
        ++_combineCounter;
        if (_combineCounter > max) combineRanges();
    }
            
    void finalize() const {
        if (_combineCounter > 0) combineRanges();
    } 
        
    /// The current Ranges list.
    //
    /// Mutable due to lazy finalization. This isn't very nice, but better than
    /// const_cast.
    mutable RangeList _ranges;

    /// snapping factor - see setSnapFactor() 
    float _snapFactor;
    
    /// if set, only a single, outer range is maintained (extended). 
    bool _singleMode;
    
    /// maximum number of ranges allowed
    size_type _rangesLimit;     
    
    /// Counter used in finalizing ranges.
    mutable size_type _combineCounter;
        
};

template <class T>
std::ostream&
operator<< (std::ostream& os, const SnappingRanges2d<T>& r)
{
    if ( r.isNull() ) return os << "NULL";
    if ( r.isWorld() ) return os << "WORLD";

    typedef typename SnappingRanges2d<T>::RangeList R;

    const R& ranges = r._ranges;

    std::copy(ranges.begin(), ranges.end(),
            std::ostream_iterator<typename R::value_type>(os, ","));

    return os;
}
    
namespace {

template<typename T>
inline bool snaptest(const geometry::Range2d<T>& range1,
        const geometry::Range2d<T>& range2, const float snapFactor)
{

    // when they intersect anyway, they should of course be merged! 
    // TODO: not really, a "+" style ranges list might be worth to 
    // remain unmerged (but needs special handling, i.e. create three
    // ranges out of two)...
    if (range1.intersects(range2)) return true;
        
    geometry::Range2d<T> temp = range1;
    temp.expandTo(range2);
    
    return (range1.getArea() + range2.getArea()) * snapFactor >
        temp.getArea();

} 
    
} // anonymous namespace
} // namespace geometry

/// Standard snapping 2d ranges type for invalidated bounds calculation    
typedef geometry::SnappingRanges2d<boost::int32_t> InvalidatedRanges;

} //namespace gnash

#endif
