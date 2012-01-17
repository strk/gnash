// Array_as.cpp:  ActionScript array class, for Gnash.
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
//

#include "Array_as.h"

#include <string>
#include <algorithm>
#include <cmath>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/lexical_cast.hpp>

#include "as_value.h"
#include "log.h"
#include "NativeFunction.h" 
#include "as_function.h"
#include "fn_call.h"
#include "Global_as.h"
#include "GnashException.h"
#include "VM.h" 
#include "GnashNumeric.h"
#include "namedStrings.h"

namespace gnash {

// Forward declarations
namespace {
	
    /// Sort flags
    enum SortFlags {
        /// Case-insensitive (z precedes A)
        SORT_CASE_INSENSITIVE = (1<<0), // 1
        /// Descending order (b precedes a)
        SORT_DESCENDING	= (1<<1), // 2
        /// If two or more elements in the array
        /// have identical sort fields, return 0
        /// and don't modify the array.
        /// Otherwise proceed to sort the array.
        SORT_UNIQUE	= (1<<2), // 4
        /// Don't modify the array, rather return
        /// a new array containing indexes into it
        /// in sorted order.
        SORT_RETURN_INDEX = (1<<3), // 8
        /// Numerical sort (9 precedes 10)
        SORT_NUMERIC = (1<<4) // 16
    };

    struct indexed_as_value;

    typedef boost::function2<bool, const as_value&, const as_value&> as_cmp_fn;

    void attachArrayInterface(as_object& proto);
    void attachArrayStatics(as_object& proto);

    as_value join(as_object* array, const std::string& separator);

    as_value array_new(const fn_call& fn);
    as_value array_slice(const fn_call& fn);
    as_value array_concat(const fn_call& fn);
    as_value array_toString(const fn_call& fn);
    as_value array_join(const fn_call& fn);
    as_value array_reverse(const fn_call& fn);
    as_value array_shift(const fn_call& fn);
    as_value array_pop(const fn_call& fn);
    as_value array_unshift(const fn_call& fn);
    as_value array_push(const fn_call& fn);
    as_value array_sortOn(const fn_call& fn);
    as_value array_sort(const fn_call& fn);
    as_value array_splice(const fn_call& fn);

    ObjectURI getKey(const fn_call& fn, size_t i);
    int isIndex(const std::string& name);

    /// Implementation of foreachArray that takes a start and end range.
    template<typename T> void foreachArray(as_object& array, int start,
                           int end, T& pred);

    inline bool int_lt_or_eq (int a) {
        return a <= 0;
    }

    inline bool int_gt (int a) {
        return a > 0;
    }

    void getIndexedElements(as_object& array, std::vector<indexed_as_value>& v);

    void pushIndices(as_object& o, const std::vector<indexed_as_value>& index);

    /// Set the length property of an object only if it is a genuine array.
    void setArrayLength(as_object& o, const int size);

    void resizeArray(as_object& o, const int size);

}

/// Function objects for foreachArray()
namespace {

struct indexed_as_value : public as_value
{
    int vec_index;
    
    indexed_as_value(const as_value& val, int index)
	: as_value(val)
	{
	    vec_index = index;
	}
};

class PushToArray
{
public:
    PushToArray(as_object& obj) : _obj(obj) {}
    void operator()(const as_value& val) {
        callMethod(&_obj, NSV::PROP_PUSH, val);
    }
private:
    as_object& _obj;
};

template<typename T>
class PushToContainer
{
public:
    PushToContainer(T& v) : _v(v) {}
    void operator()(const as_value& val) {
        _v.push_back(val);
    }
private:
    T& _v;
};


class PushToIndexedVector
{
public:
    PushToIndexedVector(std::vector<indexed_as_value>& v) : _v(v), _i(0) {}
    void operator()(const as_value& val) {
        _v.push_back(indexed_as_value(val, _i));
        ++_i;
    }
private:
    std::vector<indexed_as_value>& _v;
    size_t _i;
};

        
/// \brief
/// Attempt to sort the array using given values comparator, avc.
/// If two or more elements in the array are equal, as determined
/// by the equality comparator ave, then the array is not sorted
/// and 0 is returned. Otherwise the array is sorted and returned.
///
/// @param avc
///	boolean functor or function comparing two as_value& objects
///     used to determine sort-order
///
/// @param ave
///	boolean functor or function comparing two as_value& objects
///     used to determine equality
///
template <class AVCMP, class AVEQ>
bool sort(as_object& o, AVCMP avc, AVEQ ave)
{
    // IMPORTANT NOTE
    //
    // As for ISO/IEC 14882:2003 - 23.2.2.4.29 
    // the sort algorithm relies on the assumption
    // that the comparator function implements
    // a Strict Weak Ordering operator:
    // http://www.sgi.com/tech/stl/StrictWeakOrdering.html
    //
    // Invalid comparator can lead to undefined behaviour,
    // including invalid memory access and infinite loops.
    //
    // Pragmatically, it seems that std::list::sort is
    // more robust in this reguard, so we'll sort a list
    // instead of the queue. We want to sort a copy anyway
    // to avoid the comparator changing the original container.

    typedef std::list<as_value> SortContainer;

    SortContainer v;
    PushToContainer<SortContainer> pv(v);
    foreachArray(o, pv);

    const size_t size = v.size(); 

    v.sort(avc);

    if (std::adjacent_find(v.begin(), v.end(), ave) != v.end()) return false;

    VM& vm = getVM(o);

    SortContainer::const_iterator it = v.begin();

    for (size_t i = 0; i < size; ++i) {
        if (i >= v.size()) {
            break;
        }
        o.set_member(arrayKey(vm, i), *it);
        ++it;
    }
    return true;
}


template <class AVCMP>
void
sort(as_object& o, AVCMP avc) 
{

    typedef std::list<as_value> SortContainer;

    SortContainer v;
    PushToContainer<SortContainer> pv(v);
    foreachArray(o, pv);

    const size_t size = v.size(); 

    v.sort(avc);

    VM& vm = getVM(o);

    SortContainer::const_iterator it = v.begin();

    for (size_t i = 0; i < size; ++i) {
        if (it == v.end()) {
            break;
        }
        o.set_member(arrayKey(vm, i), *it);
        ++it;
    }
}

/// \brief
/// Return a new array containing sorted index of this array.
/// If two or more elements in the array are equal, as determined
/// by the equality comparator ave, then 0 is returned instead.
///
/// @param avc
///	boolean functor or function comparing two as_value& objects
///     used to determine sort-order
///
/// @param ave
///	boolean functor or function comparing two as_value& objects
///     used to determine equality
///
template <class AVCMP, class AVEQ>
as_value sortIndexed(as_object& array, AVCMP avc, AVEQ ave)
{
    std::vector<indexed_as_value> v;

    getIndexedElements(array, v);

    std::sort(v.begin(), v.end(), avc);

    if (std::adjacent_find(v.begin(), v.end(), ave) != v.end()) {
        return as_value(0.0);
    }

    as_object* o = getGlobal(array).createArray();
    pushIndices(*o, v);
    return o;
}


/// \brief
/// Return a new array containing sorted index of this array
///
/// @param avc
///	boolean functor or function comparing two as_value& objects
///
template <class AVCMP>
as_object*
sortIndexed(as_object& array, AVCMP avc)
{
    std::vector<indexed_as_value> v;
    getIndexedElements(array, v);
    std::sort(v.begin(), v.end(), avc);
    as_object* o = getGlobal(array).createArray();
    pushIndices(*o, v);
    return o;
}


// simple as_value strict-weak-ordering comparison functors:
// string comparison, ascending (default sort method)
struct as_value_lt
{
    as_value_lt(const fn_call& fn) : _fn(fn) {}

    int str_cmp(const as_value& a, const as_value& b) const 
    {
        std::string s = a.to_string(getSWFVersion(_fn));
        return s.compare(b.to_string(getSWFVersion(_fn)));
    }

    int str_nocase_cmp(const as_value& a, const as_value& b) const
    {
        using namespace boost::algorithm;

        std::string c = to_upper_copy(a.to_string(getSWFVersion(_fn)));
        std::string d = to_upper_copy(b.to_string(getSWFVersion(_fn)));
        return c.compare(d);
    }

    bool as_value_numLT(const as_value& a, const as_value& b) const
    {
        if (a.is_undefined()) return false;
        if (b.is_undefined()) return true;
        if (a.is_null()) return false;
        if (b.is_null()) return true;
        const double aval = toNumber(a, getVM(_fn));
        const double bval = toNumber(b, getVM(_fn));
        if (isNaN(aval)) return false;
        if (isNaN(bval)) return true;
        return aval < bval;
    }

    bool as_value_numGT(const as_value& a, const as_value& b) const
    {
        if (b.is_undefined()) return false;
        if (a.is_undefined()) return true;
        if (b.is_null()) return false;
        if (a.is_null()) return true;
        const double aval = toNumber(a, getVM(_fn));
        const double bval = toNumber(b, getVM(_fn));
        if (isNaN(bval)) return false;
        if (isNaN(aval)) return true;
        return aval > bval;
    }

    inline bool as_value_numEQ(const as_value& a, const as_value& b) const
    {
        if (a.is_undefined() && b.is_undefined()) return true;
        if (a.is_null() && b.is_null()) return true;
        double aval = toNumber(a, getVM(_fn));
        double bval = toNumber(b, getVM(_fn));
        if (isNaN(aval) && isNaN(bval)) return true;
        return aval == bval;
    }

    bool operator()(const as_value& a, const as_value& b) const
    {
        return str_cmp(a, b) < 0;
    }
private:
    const fn_call& _fn;
};

// string comparison, descending
struct as_value_gt : public as_value_lt 
{
    as_value_gt(const fn_call& fn) : as_value_lt(fn) {}
    bool operator()(const as_value& a, const as_value& b) const {
        return str_cmp(a, b) > 0;
    }
};

// string equality
struct as_value_eq : public as_value_lt
{
    as_value_eq(const fn_call& fn) : as_value_lt(fn) {}
    bool operator()(const as_value& a, const as_value& b) const
    {
        return str_cmp(a, b) == 0;
    }
};

// case-insensitive string comparison, ascending
struct as_value_nocase_lt : public as_value_lt
{
    as_value_nocase_lt(const fn_call& fn) : as_value_lt(fn) {}
    bool operator()(const as_value& a, const as_value& b) const
    {
        return str_nocase_cmp(a, b) < 0;
    }
};

// case-insensitive string comparison, descending
struct as_value_nocase_gt : public as_value_lt
{
    as_value_nocase_gt(const fn_call& fn) : as_value_lt(fn) {}
    bool operator()(const as_value& a, const as_value& b) const
    {
        return str_nocase_cmp(a, b) > 0;
    }
};

// case-insensitive string equality
struct as_value_nocase_eq : public as_value_lt
{
    as_value_nocase_eq(const fn_call& fn) : as_value_lt(fn) {}
    bool operator()(const as_value& a, const as_value& b) const
    {
        return str_nocase_cmp(a, b) == 0;
    }
};

// numeric comparison, ascending
struct as_value_num_lt : public as_value_lt
{
    as_value_num_lt(const fn_call& fn) : as_value_lt(fn) {}
    bool operator()(const as_value& a, const as_value& b) const
    {
        if (a.is_string() || b.is_string()) return str_cmp(a, b) < 0;
        return as_value_numLT(a, b);
    }
};

// numeric comparison, descending
struct as_value_num_gt : public as_value_lt
{
    as_value_num_gt(const fn_call& fn) : as_value_lt(fn) {}
    bool operator()(const as_value& a, const as_value& b) const
    {
        if (a.is_string() || b.is_string()) return str_cmp(a, b) > 0;
        return as_value_numGT(a, b);
    }
};

// numeric equality
struct as_value_num_eq : public as_value_lt
{
    as_value_num_eq(const fn_call& fn) : as_value_lt(fn) {}
    bool operator()(const as_value& a, const as_value& b) const
    {
        if (a.is_string() || b.is_string()) return str_cmp(a, b) == 0;
        return as_value_numEQ(a, b);
    }
};

// case-insensitive numeric comparison, ascending
struct as_value_num_nocase_lt : public as_value_lt
{
    as_value_num_nocase_lt(const fn_call& fn) : as_value_lt(fn) {}
    bool operator()(const as_value& a, const as_value& b) const
    {
        if (a.is_string() || b.is_string()) return str_nocase_cmp(a, b) < 0;
        return as_value_numLT(a, b);
    }
};

// case-insensitive numeric comparison, descending
struct as_value_num_nocase_gt : public as_value_lt
{
    as_value_num_nocase_gt(const fn_call& fn) : as_value_lt(fn) {}
    bool operator()(const as_value& a, const as_value& b) const
    {
        if (a.is_string() || b.is_string()) return str_nocase_cmp(a, b) > 0;
        return as_value_numGT(a, b);
    }
};

// case-insensitive numeric equality
struct as_value_num_nocase_eq : public as_value_lt
{
    as_value_num_nocase_eq(const fn_call& fn) : as_value_lt(fn) {}
    bool operator() (const as_value& a, const as_value& b) const
    {
        if (a.is_string() || b.is_string()) return str_nocase_cmp(a, b) == 0;
        return as_value_numEQ(a, b);
    }
};

// Return basic as_value comparison functor for corresponding sort flag
// Note:
// SORT_UNIQUE and SORT_RETURN_INDEX must first be stripped from the flag
as_cmp_fn
get_basic_cmp(boost::uint8_t flags, const fn_call& fn)
{
    as_cmp_fn f;

    // SORT_UNIQUE and SORT_RETURN_INDEX must be stripped by caller
    assert(flags^SORT_UNIQUE);
    assert(flags^SORT_RETURN_INDEX);

    switch ( flags )
    {
        case 0: // default string comparison
            f = as_value_lt(fn);
            return f;

        case SORT_DESCENDING:
            f = as_value_gt(fn);
            return f;

        case SORT_CASE_INSENSITIVE: 
            f = as_value_nocase_lt(fn);
            return f;

        case SORT_CASE_INSENSITIVE | 
                SORT_DESCENDING:
            f = as_value_nocase_gt(fn);
            return f;

        case SORT_NUMERIC: 
            f = as_value_num_lt(fn);
            return f;

        case SORT_NUMERIC | SORT_DESCENDING:
            f = as_value_num_gt(fn);
            return f;

        case SORT_CASE_INSENSITIVE | 
                SORT_NUMERIC:
            f = as_value_num_nocase_lt(fn);
            return f;

        case SORT_CASE_INSENSITIVE | 
                SORT_NUMERIC |
                SORT_DESCENDING:
            f = as_value_num_nocase_gt(fn);
            return f;

        default:
            log_unimpl(_("Unhandled sort flags: %d (0x%X)"), +flags, +flags);
            f = as_value_lt(fn);
            return f;
    }
}

// Return basic as_value equality functor for corresponding sort flag
// Note:
// SORT_UNIQUE and SORT_RETURN_INDEX must first be stripped from the flag
as_cmp_fn
get_basic_eq(boost::uint8_t flags, const fn_call& fn)
{
    as_cmp_fn f;
    flags &= ~(SORT_DESCENDING);

    switch (flags)
    {
        case 0: // default string comparison
            f = as_value_eq(fn);
            return f;

        case SORT_CASE_INSENSITIVE: 
            f = as_value_nocase_eq(fn);
            return f;

        case SORT_NUMERIC: 
            f = as_value_num_eq(fn);
            return f;

        case SORT_CASE_INSENSITIVE | 
                SORT_NUMERIC:
            f = as_value_num_nocase_eq(fn);
            return f;

        default:
            f = as_value_eq(fn);
            return f;
    }
}

// Custom (ActionScript) comparator 
class as_value_custom
{
public:
    as_function& _comp;
    as_object* _object;
    bool (*_zeroCmp)(const int);
    const as_environment& _env;

    as_value_custom(as_function& comparator, bool (*zc)(const int), 
            as_object* this_ptr, const as_environment& env)
        :
        _comp(comparator),
        _zeroCmp(zc),
        _env(env)
    {
        _object = this_ptr;
    }

    bool operator() (const as_value& a, const as_value& b)
    {
        as_value cmp_method(&_comp);
        as_value ret(0.0);
        fn_call::Args args;
        args += b, a;
        ret = invoke(cmp_method, _env, _object, args);

        return (*_zeroCmp)(toInt(ret, getVM(_env)));
    }
};

// Comparator for sorting on a single array property
class as_value_prop
{
public:
    
    // Note: cmpfn must implement a strict weak ordering
    as_value_prop(ObjectURI name, as_cmp_fn cmpfn, const as_object& o)
        :
        _comp(cmpfn),
        _prop(name),
        _obj(o)
    {
    }

    bool operator()(const as_value& a, const as_value& b) const {

        // why do we cast ao/bo to objects here ?
        as_object* ao = toObject(a, getVM(_obj));
        as_object* bo = toObject(b, getVM(_obj));

        assert(ao);
        assert(bo);
        
        const as_value& av = getOwnProperty(*ao, _prop);
        const as_value& bv = getOwnProperty(*bo, _prop);

        return _comp(av, bv);
    }
private:
    as_cmp_fn _comp;
    ObjectURI _prop;
    const as_object& _obj;
};

// Comparator for sorting on multiple array properties
class as_value_multiprop
{
public:
    typedef std::vector<as_cmp_fn> Comps;
    Comps& _cmps;

    typedef std::vector<ObjectURI> Props;
    Props& _prps;
    
    const as_object& _obj;

    // Note: all as_cmp_fns in *cmps must implement strict weak ordering
    as_value_multiprop(std::vector<ObjectURI>& prps, 
        std::vector<as_cmp_fn>& cmps, const as_object& o)
        :
        _cmps(cmps),
        _prps(prps),
        _obj(o)
    {
    }

    bool operator() (const as_value& a, const as_value& b)
    {
        if (_cmps.empty()) return false;

        std::vector<as_cmp_fn>::iterator cmp = _cmps.begin();

        // why do we cast ao/bo to objects here ?
        as_object* ao = toObject(a, getVM(_obj));
        as_object* bo = toObject(b, getVM(_obj));

        // TODO: this may not be correct, but it is better than accessing
        // null pointers.
        if (!ao || !bo) return false;
        
        for (Props::iterator pit = _prps.begin(), pend = _prps.end();
                pit != pend; ++pit, ++cmp) {
            
            const as_value& av = getOwnProperty(*ao, *pit);
            const as_value& bv = getOwnProperty(*bo, *pit);

            if ((*cmp)(av, bv)) return true;
            if ((*cmp)(bv, av)) return false;
            // Note: for loop finishes only if a == b for
            // each requested comparison
            // (since *cmp(av,bv) == *cmp(bv,av) == false)
        }
        
        return false;
    }
};

class as_value_multiprop_eq : public as_value_multiprop
{
public:
    as_value_multiprop_eq(std::vector<ObjectURI>& prps, 
        std::vector<as_cmp_fn>& cmps, const as_object& o)
        :
        as_value_multiprop(prps, cmps, o),
        _obj(o)
    {
    }

    bool operator()(const as_value& a, const as_value& b) const {
        if (_cmps.empty()) return false;

        Comps::const_iterator cmp = _cmps.begin();

        // why do we cast ao/bo to objects here ?
        as_object* ao = toObject(a, getVM(_obj));
        as_object* bo = toObject(b, getVM(_obj));

        for (Props::iterator pit = _prps.begin(), pend = _prps.end();
                pit != pend; ++pit, ++cmp)
        {
            const as_value& av = getOwnProperty(*ao, *pit);
            const as_value& bv = getOwnProperty(*bo, *pit);

            if (!(*cmp)(av, bv)) return false;
        }
        
        return true;
    }
private:
    const as_object& _obj;
};

// Convenience function to strip SORT_UNIQUE and SORT_RETURN_INDEX from sort
// flag. Presence of flags recorded in douniq and doindex.
inline boost::uint8_t
flag_preprocess(boost::uint8_t flgs, bool* douniq, bool* doindex)
{
    *douniq = (flgs & SORT_UNIQUE);
    *doindex = (flgs & SORT_RETURN_INDEX);
    flgs &= ~(SORT_RETURN_INDEX);
    flgs &= ~(SORT_UNIQUE);
    return flgs;
}


class GetKeys
{
public:
    GetKeys(std::vector<ObjectURI>& v, VM& vm, int version)
        :
        _v(v),
        _vm(vm),
        _version(version)
    {}
    void operator()(const as_value& val) {
        _v.push_back(getURI(_vm, val.to_string(_version)));
    }
private:
    std::vector<ObjectURI>& _v;
    VM& _vm;
    const int _version;
};

/// Functor to extract flags from an array-like object.
//
/// I don't know how accurate this code is. It was copied from the previous
/// implementation but without using Array_as.
class GetMultiFlags
{
public:
    GetMultiFlags(std::vector<boost::uint8_t>& v, const fn_call& fn)
        :
        _v(v),
        _i(0),
        _uniq(false),
        _index(false),
        _fn(fn)
    {}
    void operator()(const as_value& val) {
        // extract SORT_UNIQUE and SORT_RETURN_INDEX from first flag
        if (!_i) {
            boost::uint8_t flag =
                static_cast<boost::uint8_t>(toNumber(val, getVM(_fn)));
            flag = flag_preprocess(flag, &_uniq, &_index);
            _v.push_back(flag);
            ++_i;
            return;
        }
        boost::uint8_t flag = 
                static_cast<boost::uint8_t>(toNumber(val, getVM(_fn)));
        flag &= ~(SORT_RETURN_INDEX);
        flag &= ~(SORT_UNIQUE);
        _v.push_back(flag);
        ++_i;
    }
    bool unique() const { return _uniq; }
    bool index() const { return _index; }

private:
    std::vector<boost::uint8_t>& _v;
    size_t _i;
    bool _uniq;
    bool _index;
    const fn_call& _fn;
};

}

bool
IsStrictArray::accept(const ObjectURI& uri, const as_value& /*val*/)
{
    // We ignore namespace.
    if (isIndex(uri.toString(_st.getStringTable())) >= 0) return true;
    _strict = false;
    return false;
}

void
checkArrayLength(as_object& array, const ObjectURI& uri, const as_value& val)
{
    // TODO: check if we should really be doing
    //       case-sensitive comparison here!
    const bool caseless = true;
    ObjectURI::CaseEquals eq(getStringTable(array), caseless);
    if (eq(uri, getURI(getVM(array), NSV::PROP_LENGTH))) {
        resizeArray(array, toInt(val, getVM(array)));
        return;
    }

    const int index = isIndex(uri.toString(getStringTable(array)));

    // if we were sent a valid array index
    if (index >= 0) {
        if (static_cast<size_t>(index) >= arrayLength(array)) {
            setArrayLength(array, index + 1);
        }
    }
}

size_t
arrayLength(as_object& array)
{
    // Only the length property of the array object itself counts.
    const as_value& length = getOwnProperty(array, NSV::PROP_LENGTH);
    if (length.is_undefined()) return 0;
    
    const int size = toInt(length, getVM(array));
    if (size < 0) return 0;
    return size;
}

void
registerArrayNative(as_object& global)
{
    VM& vm = getVM(global);
    vm.registerNative(array_new, 252, 0);
    vm.registerNative(array_push, 252, 1);
    vm.registerNative(array_pop, 252, 2);
    vm.registerNative(array_concat, 252, 3);
    vm.registerNative(array_shift, 252, 4);
    vm.registerNative(array_unshift, 252, 5);
    vm.registerNative(array_slice, 252, 6);
    vm.registerNative(array_join, 252, 7);
    vm.registerNative(array_splice, 252, 8);
    vm.registerNative(array_toString, 252, 9);
    vm.registerNative(array_sort, 252, 10);
    vm.registerNative(array_reverse, 252, 11);
    vm.registerNative(array_sortOn, 252, 12);
}

void
array_class_init(as_object& where, const ObjectURI& uri)
{
    // This is going to be the global Array "class"/"function"
    Global_as& gl = getGlobal(where);

    as_object* proto = createObject(gl);

    VM& vm = getVM(where);
    as_object* cl = vm.getNative(252, 0);

    cl->init_member(NSV::PROP_PROTOTYPE, proto);
    proto->init_member(NSV::PROP_CONSTRUCTOR, cl);

    attachArrayInterface(*proto);
    attachArrayStatics(*cl);

    const int flags = PropFlags::dontEnum; 
    where.init_member(uri, cl, flags);
}

// Used by foreachArray, declared in Array_as.h
ObjectURI
arrayKey(VM& vm, size_t i)
{
    // TODO: tell getURI that the string is already lowercase!
    return getURI(vm, boost::lexical_cast<std::string>(i), true);
}

namespace {

void
attachArrayStatics(as_object& proto)
{
    const int flags = 0; // these are not protected
    proto.init_member("CASEINSENSITIVE", SORT_CASE_INSENSITIVE, flags);
    proto.init_member("DESCENDING", SORT_DESCENDING, flags);
    proto.init_member("UNIQUESORT", SORT_UNIQUE, flags);
    proto.init_member("RETURNINDEXEDARRAY", SORT_RETURN_INDEX, flags);
    proto.init_member("NUMERIC", SORT_NUMERIC, flags);
}

void
attachArrayInterface(as_object& proto)
{
    VM& vm = getVM(proto);

    proto.init_member("push", vm.getNative(252, 1));
    proto.init_member("pop", vm.getNative(252, 2));
    proto.init_member("concat", vm.getNative(252, 3));
    proto.init_member("shift", vm.getNative(252, 4));
    proto.init_member("unshift", vm.getNative(252, 5));
    proto.init_member("slice", vm.getNative(252, 6));
    proto.init_member("join", vm.getNative(252, 7));
    proto.init_member("splice", vm.getNative(252, 8));
    proto.init_member("toString", vm.getNative(252, 9));
    proto.init_member("sort", vm.getNative(252, 10));
    proto.init_member("reverse", vm.getNative(252, 11));
    proto.init_member("sortOn", vm.getNative(252, 12));
}

as_value
array_splice(const fn_call& fn)
{
    as_object* array = ensure<ValidThis>(fn);
    
    if (fn.nargs < 1) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Array.splice() needs at least 1 argument, "
                    "call ignored"));
        );
        return as_value();
    }
    
    const size_t size = arrayLength(*array);

    //----------------
    // Get start offset
    //----------------

    int start = toInt(fn.arg(0), getVM(fn));
    if (start < 0) start = size + start; 
    start = clamp<int>(start, 0, size);

    // Get length to delete
    size_t remove = size - start;
    
    if (fn.nargs > 1) {
        int remval = toInt(fn.arg(1), getVM(fn));
        if (remval < 0) {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("Array.splice(%d,%d): negative length "
                        "given, call ignored"), start, remval);
            );
            return as_value();
        }
        remove = clamp<int>(remval, 0, size - start);
    }

    Global_as& gl = getGlobal(fn);
    as_object* ret = gl.createArray();

    // Copy the original array values for reinsertion. It's not possible
    // to do a simple copy in-place without overwriting values that still
    // need to be shifted. The algorithm could certainly be improved though.
    typedef std::vector<as_value> TempContainer;
    TempContainer v;
    PushToContainer<TempContainer> pv(v);
    foreachArray(*array, pv);

    const size_t newelements = fn.nargs > 2 ? fn.nargs - 2 : 0;
    
    // Push removed elements to the new array.
    ObjectURI propPush = getURI(getVM(fn), NSV::PROP_PUSH);
    for (size_t i = 0; i < remove; ++i) {
        const ObjectURI& key = getKey(fn, start + i);
        callMethod(ret, propPush, getOwnProperty(*array, key));
    }

    // Shift elements in 'this' array by simple assignment, not delete
    // and readd.
    for (size_t i = 0; i < static_cast<size_t>(size - remove); ++i) {
        const bool started = (i >= static_cast<size_t>(start));
        const size_t index = started ? i + remove : i;
        const size_t target = started ? i + newelements : i;
        array->set_member(getKey(fn, target), v[index]);
    }

    // Insert the replacement elements in the gap we left.
    for (size_t i = 0; i < newelements; ++i) {
        array->set_member(getKey(fn, start + i), fn.arg(i + 2));
    }
    
    // This one is correct!
    ObjectURI propLen = getURI(getVM(fn), NSV::PROP_LENGTH);
    array->set_member(propLen, size + newelements - remove);

    return as_value(ret);
}

as_value
array_sort(const fn_call& fn)
{
    as_object* array = ensure<ValidThis>(fn);
    
    if (!fn.nargs) {
        sort(*array, as_value_lt(fn));
        return as_value(array);
    }
    
    if (fn.arg(0).is_undefined()) return as_value();

    boost::uint8_t flags = 0;

    if (fn.nargs == 1 && fn.arg(0).is_number()) {
        flags = static_cast<boost::uint8_t>(toNumber(fn.arg(0), getVM(fn)));
    }
    else if (fn.arg(0).is_function()) {
        // Get comparison function
        as_function* as_func = fn.arg(0).to_function();

        assert(as_func);

        bool (*icmp)(int);
    
        if (fn.nargs == 2 && fn.arg(1).is_number()) {
            flags=static_cast<boost::uint8_t>(toNumber(fn.arg(1), getVM(fn)));
        }

        if (flags & SORT_DESCENDING) icmp = &int_lt_or_eq;
        else icmp = &int_gt;

        const as_environment& env = fn.env();

        as_value_custom avc = 
            as_value_custom(*as_func, icmp, fn.this_ptr, env);

        if ((flags & SORT_RETURN_INDEX)) {
            return sortIndexed(*array, avc);
        }

        sort(*array, avc);
        return as_value(array);
        // note: custom AS function sorting apparently ignores the 
        // UniqueSort flag which is why it is also ignored here
    }
    else
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Sort called with invalid arguments."));
        )
        return as_value(array);
    }

    bool do_unique, do_index;
    flags = flag_preprocess(flags, &do_unique, &do_index);
    as_cmp_fn comp = get_basic_cmp(flags, fn);

    if (do_unique) {
        as_cmp_fn eq = get_basic_eq(flags, fn);
        if (do_index) return sortIndexed(*array, comp, eq);
        return sort(*array, comp, eq) ? as_value(array) : as_value(0.0);
    }
    if (do_index) return sortIndexed(*array, comp);
    sort(*array, comp);
    return as_value(array);
}

as_value
array_sortOn(const fn_call& fn)
{
    as_object* array = ensure<ValidThis>(fn);

    bool do_unique = false, do_index = false;
    boost::uint8_t flags = 0;

    const int version = getSWFVersion(fn);
    VM& vm = getVM(fn);

    if (fn.nargs == 0) return as_value();

    // cases: sortOn("prop) and sortOn("prop", Array.FLAG)
    if (fn.arg(0).is_string()) 
    {
        ObjectURI propField =
            getURI(vm, fn.arg(0).to_string(version));

        if (fn.nargs > 1 && fn.arg(1).is_number()) {
            flags = static_cast<boost::uint8_t>(toNumber(fn.arg(1), getVM(fn)));
            flags = flag_preprocess(flags, &do_unique, &do_index);
        }

        as_value_prop avc(propField, get_basic_cmp(flags, fn),
                getGlobal(fn));

        if (do_unique) {
            as_value_prop ave(propField, get_basic_eq(flags, fn), 
                    getGlobal(fn));
            if (do_index)
                return sortIndexed(*array, avc, ave);
            return sort(*array, avc, ave) ? as_value(array) : as_value(0.0);
        }
        
        if (do_index) {
            return sortIndexed(*array, avc);
        }

        sort(*array, avc);
        return as_value(array);
    }

    // case: sortOn(["prop1", "prop2"] ...)
    if (fn.arg(0).is_object()) 
    {
        as_object* props = toObject(fn.arg(0), getVM(fn));
        assert(props);

        std::vector<ObjectURI> prp;
        GetKeys gk(prp, vm, version);
        foreachArray(*props, gk);
        
        std::vector<as_cmp_fn> cmp;
        std::vector<as_cmp_fn> eq;
        
        // Will be the same as arrayLength(*props);
        const size_t optnum = prp.size();
        
        // case: sortOn(["prop1", "prop2"])
        if (fn.nargs == 1) {
            // assign each cmp function to the standard cmp fn
            as_cmp_fn c = get_basic_cmp(0, fn);
            cmp.assign(optnum, c);
        }
        // case: sortOn(["prop1", "prop2"], [Array.FLAG1, Array.FLAG2])
        else if (fn.arg(1).is_object()) {

            as_object* farray = toObject(fn.arg(1), getVM(fn));

            // Only an array will do for this case.
            if (farray->array() && arrayLength(*farray) == optnum) {

                std::vector<boost::uint8_t> flgs;
                GetMultiFlags mf(flgs, fn);
                foreachArray(*farray, mf);
                do_unique = mf.unique();
                do_index = mf.index();
                
                std::vector<boost::uint8_t>::const_iterator it = 
                    flgs.begin();

                while (it != flgs.end()) {
                    cmp.push_back(get_basic_cmp(*it++, fn));
                }

                if (do_unique) {
                    it = flgs.begin();
                    while (it != flgs.end())
                        eq.push_back(get_basic_eq(*it++, fn));
                }
            }
            else {
                as_cmp_fn c = get_basic_cmp(0, fn);
                cmp.assign(optnum, c);
            }
        }
        // case: sortOn(["prop1", "prop2"], Array.FLAG)
        else {
            boost::uint8_t flags = 
                static_cast<boost::uint8_t>(toInt(fn.arg(1), getVM(fn)));
            flags = flag_preprocess(flags, &do_unique, &do_index);
            as_cmp_fn c = get_basic_cmp(flags, fn);

            cmp.assign(optnum, c);
            
            if (do_unique) {
                as_cmp_fn e = get_basic_eq(flags, fn);
                eq.assign(optnum, e);
            }
        }
        as_value_multiprop avc(prp, cmp, getGlobal(fn));

        if (do_unique) {
            as_value_multiprop_eq ave(prp, eq, getGlobal(fn));
            if (do_index) return sortIndexed(*array, avc, ave);
            return sort(*array, avc, ave) ? as_value(array) : as_value(0.0);
        }
        if (do_index) return sortIndexed(*array, avc);
        sort(*array, avc);
        return as_value(array);

    }

    IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("SortOn called with invalid arguments."));
    )

    return as_value(array);
}

// Callback to push values to the back of an array
as_value
array_push(const fn_call& fn)
{
    as_object* array = ensure<ValidThis>(fn);
 
    if (!fn.nargs) return as_value();

    const size_t shift = fn.nargs;

    const size_t size = arrayLength(*array);

    for (size_t i = 0; i < shift; ++i) {
        array->set_member(getKey(fn, size + i), fn.arg(i));
    }
 
    return as_value(size + shift);
}

// Callback to push values to the front of an array
as_value
array_unshift(const fn_call& fn)
{

    as_object* array = ensure<ValidThis>(fn);
 
    if (!fn.nargs) return as_value();

    const size_t shift = fn.nargs;

    const size_t size = arrayLength(*array);

    for (size_t i = size + shift - 1; i >= shift ; --i) {
        const ObjectURI nextkey = getKey(fn, i - shift);
        const ObjectURI currentkey = getKey(fn, i);
        array->delProperty(currentkey);
        array->set_member(currentkey, getOwnProperty(*array, nextkey));
    }

    for (size_t i = shift; i > 0; --i) {
        const size_t index = i - 1;
        array->set_member(getKey(fn, index), fn.arg(index));
    }
 
    setArrayLength(*array, size + shift);

    return as_value(size + shift);
}

// Callback to pop a value from the back of an array
as_value
array_pop(const fn_call& fn)
{

    as_object* array = ensure<ValidThis>(fn);

    const size_t size = arrayLength(*array);
    if (size < 1) return as_value();

    const ObjectURI ind = getKey(fn, size - 1);
    as_value ret = getOwnProperty(*array, ind);
    array->delProperty(ind);
    
    setArrayLength(*array, size - 1);

    return ret;
}

// Callback to pop a value from the front of an array
as_value
array_shift(const fn_call& fn)
{
    as_object* array = ensure<ValidThis>(fn);

    const size_t size = arrayLength(*array);
    // An array with no elements has nothing to return.
    if (size < 1) return as_value();

    as_value ret = getOwnProperty(*array, getKey(fn, 0));

    for (size_t i = 0; i < static_cast<size_t>(size - 1); ++i) {
        const ObjectURI nextkey = getKey(fn, i + 1);
        const ObjectURI currentkey = getKey(fn, i);
        array->delProperty(currentkey);
        array->set_member(currentkey, getOwnProperty(*array, nextkey));
    }
    
    setArrayLength(*array, size - 1);

    return ret;
}

// Callback to reverse the position of the elements in an array
as_value
array_reverse(const fn_call& fn)
{
    as_object* array = ensure<ValidThis>(fn);

    const size_t size = arrayLength(*array);
    // An array with 0 or 1 elements has nothing to reverse.
    if (size < 2) return as_value();

    for (size_t i = 0; i < static_cast<size_t>(size) / 2; ++i) {
        const ObjectURI bottomkey = getKey(fn, i);
        const ObjectURI topkey = getKey(fn, size - i - 1);
        const as_value top = getOwnProperty(*array, topkey);
        const as_value bottom = getOwnProperty(*array, bottomkey);
        array->delProperty(topkey);
        array->delProperty(bottomkey);
        array->set_member(bottomkey, top);
        array->set_member(topkey, bottom);
    }

    return array;
}
as_value
array_join(const fn_call& fn)
{
    as_object* array = ensure<ValidThis>(fn);

    const int version = getSWFVersion(fn);
    const std::string separator =
        fn.nargs ? fn.arg(0).to_string(version) : ",";

    return join(array, separator);
}

// Callback to convert array to a string
as_value
array_toString(const fn_call& fn)
{
    as_object* array = ensure<ValidThis>(fn);
    return join(array, ",");
}

/// concatenates the elements specified in the parameters with
/// the elements in my_array, and creates a new array. If the
/// value parameters specify an array, the elements of that
/// array are concatenated, rather than the array itself. The
/// array my_array is left unchanged.
as_value
array_concat(const fn_call& fn)
{
    as_object* array = ensure<ValidThis>(fn);

    Global_as& gl = getGlobal(fn);
    as_object* newarray = gl.createArray();

    PushToArray push(*newarray);
    foreachArray(*array, push);

    ObjectURI propPush = getURI(getVM(fn), NSV::PROP_PUSH);
    for (size_t i = 0; i < fn.nargs; ++i) {

        // Array args get concatenated by elements
        // The type is checked using instanceOf.
        const as_value& arg = fn.arg(i);

        as_object* other = toObject(arg, getVM(fn));

        if (other) {
            // If it's not an array, we want to carry on and add it as an
            // object.
            if (other->instanceOf(getClassConstructor(fn, "Array"))) {
                // Do we care if it has no length property?
                foreachArray(*other, push);
                continue;
            }
        }
        callMethod(newarray, propPush, fn.arg(i));
    }

    return as_value(newarray);        
}

// Callback to slice part of an array to a new array
// without changing the original
as_value
array_slice(const fn_call& fn)
{
    as_object* array = ensure<ValidThis>(fn);

    if (fn.nargs > 2) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("More than 2 arguments to Array.slice, "
            "and I don't know what to do with them.  "
            "Ignoring them"));
        );
    }

    int startindex = fn.nargs ? toInt(fn.arg(0), getVM(fn)) : 0;

    // if we sent at least two arguments, setup endindex
    int endindex = fn.nargs > 1 ? toInt(fn.arg(1), getVM(fn)) :
        std::numeric_limits<int>::max();

    Global_as& gl = getGlobal(fn);
    as_object* newarray = gl.createArray();

    PushToArray push(*newarray);

    foreachArray(*array, startindex, endindex, push);

    return as_value(newarray);        
}

as_value
array_new(const fn_call& fn)
{

    as_object* ao = fn.isInstantiation() ? ensure<ValidThis>(fn) :
                                           getGlobal(fn).createArray();

    ao->setRelay(0);
    ao->setArray();
    ao->init_member(NSV::PROP_LENGTH, 0.0);

    if (!fn.nargs) {
        return as_value(ao);
    }

    if (fn.nargs == 1 && fn.arg(0).is_number()) {
        const int newSize = std::max(toInt(fn.arg(0), getVM(fn)), 0);
        if (newSize) {
            ao->set_member(NSV::PROP_LENGTH, newSize);
        }
        return as_value(ao);
    }

    // Use the arguments as initializers.
    for (size_t i = 0; i < fn.nargs; i++) {
        callMethod(ao, NSV::PROP_PUSH, fn.arg(i));
    }

    return as_value(ao);
}

as_value
join(as_object* array, const std::string& separator)
{
    const size_t size = arrayLength(*array);
    as_value length;
    if (size < 1) return as_value("");

    std::string s;

    VM& vm = getVM(*array);
    const int version = getSWFVersion(*array);

    for (size_t i = 0; i < size; ++i) {
        if (i) s += separator;
        const std::string& index = boost::lexical_cast<std::string>(i);
        const as_value& el = getOwnProperty(*array, getURI(vm, index));
        s += el.to_string(version);
    }
    return as_value(s);
}

ObjectURI
getKey(const fn_call& fn, size_t i)
{
    VM& vm = getVM(fn);
    return arrayKey(vm, i);
}

template<typename T>
void foreachArray(as_object& array, int start, int end, T& pred)
{
    const int size = arrayLength(array);
    if (!size) return;

    if (start < 0) start = size + start;
    if (start >= size) return;
    start = std::max(start, 0);

    if (end < 0) end = size + end;
    end = std::max(start, end);
    end = std::min<size_t>(end, size);

    assert(start >= 0);
    assert(end >= start);
    assert(size >= end);

    VM& vm = getVM(array);

    for (size_t i = start; i < static_cast<size_t>(end); ++i) {
        pred(getOwnProperty(array, arrayKey(vm, i)));
    }
}

void
pushIndices(as_object& o, const std::vector<indexed_as_value>& elems)
{
    for (std::vector<indexed_as_value>::const_iterator it = elems.begin();
        it != elems.end(); ++it) {
        callMethod(&o, NSV::PROP_PUSH, it->vec_index);
    }
}

void
getIndexedElements(as_object& array, std::vector<indexed_as_value>& v)
{
    PushToIndexedVector pv(v);
    foreachArray(array, pv);
}

void
resizeArray(as_object& o, const int size)
{
    // Only positive indices are deleted.
    size_t realSize = std::max(size, 0);

    const size_t currentSize = arrayLength(o);
    if (realSize < currentSize) {
        VM& vm = getVM(o);
        for (size_t i = realSize; i < currentSize; ++i) {
            o.delProperty(arrayKey(vm, i));
        }
    }
}

void
setArrayLength(as_object& array, const int size)
{
    if (!array.array()) return;

    resizeArray(array, size);

    array.set_member(NSV::PROP_LENGTH, size);
}

int
isIndex(const std::string& nameString)
{
    try {
        return boost::lexical_cast<int>(nameString);
    }
    catch (boost::bad_lexical_cast& e) {
        return -1;
    }
}

} // anonymous namespace

} // end of gnash namespace


// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:

