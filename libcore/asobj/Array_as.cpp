// Array_as.cpp:  ActionScript array class, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "smart_ptr.h" // GNASH_USE_GC
#include "as_value.h"
#include "Array_as.h"
#include "log.h"
#include "builtin_function.h" // for Array class
#include "as_function.h" // for sort user-defined comparator
#include "fn_call.h"
#include "GnashException.h"
#include "action.h" // for call_method
#include "VM.h" // for PROPNAME, registerNative
#include "Object.h" // for getObjectInterface()

#include <string>
#include <algorithm>
#include <cmath>
#include <boost/algorithm/string/case_conv.hpp>

//#define GNASH_DEBUG 

namespace gnash {

typedef boost::function2<bool, const as_value&, const as_value&> as_cmp_fn;

static as_object* getArrayInterface();
static void attachArrayProperties(as_object& proto);
static void attachArrayInterface(as_object& proto);
static void attachArrayStatics(as_object& proto);

inline static bool int_lt_or_eq (int a)
{
    return a <= 0;
}

inline static bool int_gt (int a)
{
    return a > 0;
}

// simple as_value strict-weak-ordering comparison functors:

// string comparison, ascending (default sort method)
struct as_value_lt
{
    int _version;

    as_value_lt(int version)
        : _version(version)
    {
    }

    inline int str_cmp(const as_value& a, const as_value& b)
    {
        std::string s = a.to_string_versioned(_version);
        return s.compare(b.to_string_versioned(_version));
    }

    inline int str_nocase_cmp(const as_value& a, const as_value& b)
    {
        using namespace boost::algorithm;

        std::string c = to_upper_copy(a.to_string_versioned(_version));
        std::string d = to_upper_copy(b.to_string_versioned(_version));
        return c.compare(d);
    }

    inline bool as_value_numLT (const as_value& a, const as_value& b)
    {
        if (a.is_undefined()) return false;
        if (b.is_undefined()) return true;
        if (a.is_null()) return false;
        if (b.is_null()) return true;
        double aval = a.to_number();
        double bval = b.to_number();
        if (isNaN(aval)) return false;
        if (isNaN(bval)) return true;
        return aval < bval;
    }

    inline bool as_value_numGT (const as_value& a, const as_value& b)
    {
        if (b.is_undefined()) return false;
        if (a.is_undefined()) return true;
        if (b.is_null()) return false;
        if (a.is_null()) return true;
        double aval = a.to_number();
        double bval = b.to_number();
        if (isNaN(bval)) return false;
        if (isNaN(aval)) return true;
        return aval > bval;
    }

    inline bool as_value_numEQ (const as_value& a, const as_value& b)
    {
        if (a.is_undefined() && b.is_undefined()) return true;
        if (a.is_null() && b.is_null()) return true;
        double aval = a.to_number();
        double bval = b.to_number();
        if (isNaN(aval) && isNaN(bval)) return true;
        return aval == bval;
    }

    bool operator() (const as_value& a, const as_value& b)
    {
        return str_cmp(a, b) < 0;
    }
};

// string comparison, descending
struct as_value_gt : public as_value_lt 
{
    as_value_gt(int version) : as_value_lt(version) {}
    bool operator() (const as_value& a, const as_value& b)
    {
        return str_cmp(a, b) > 0;
    }
};

// string equality
struct as_value_eq : public as_value_lt
{
    as_value_eq(int version) : as_value_lt(version) {}
    bool operator() (const as_value& a, const as_value& b)
    {
        return str_cmp(a, b) == 0;
    }
};

// case-insensitive string comparison, ascending
struct as_value_nocase_lt : public as_value_lt
{
    as_value_nocase_lt(int version) : as_value_lt(version) {}
    bool operator() (const as_value& a, const as_value& b)
    {
        return str_nocase_cmp(a, b) < 0;
    }
};

// case-insensitive string comparison, descending
struct as_value_nocase_gt : public as_value_lt
{
    as_value_nocase_gt(int version) : as_value_lt(version) {}
    bool operator() (const as_value& a, const as_value& b)
    {
        return str_nocase_cmp(a, b) > 0;
    }
};

// case-insensitive string equality
struct as_value_nocase_eq : public as_value_lt
{
    as_value_nocase_eq(int version) : as_value_lt(version) {}
    bool operator() (const as_value& a, const as_value& b)
    {
        return str_nocase_cmp(a, b) == 0;
    }
};

// numeric comparison, ascending
struct as_value_num_lt : public as_value_lt
{
    as_value_num_lt(int version) : as_value_lt(version) {}
    bool operator() (const as_value& a, const as_value& b)
    {
        if (a.is_string() || b.is_string())
            return str_cmp(a, b) < 0;
        return as_value_numLT(a, b);
    }
};

// numeric comparison, descending
struct as_value_num_gt : public as_value_lt
{
    as_value_num_gt(int version) : as_value_lt(version) {}
    bool operator() (const as_value& a, const as_value& b)
    {
        if (a.is_string() || b.is_string())
            return str_cmp(a, b) > 0;
        return as_value_numGT(a, b);
    }
};

// numeric equality
struct as_value_num_eq : public as_value_lt
{
    as_value_num_eq(int version) : as_value_lt(version) {}
    bool operator() (const as_value& a, const as_value& b)
    {
        if (a.is_string() || b.is_string())
            return str_cmp(a, b) == 0;
        return as_value_numEQ(a, b);
    }
};

// case-insensitive numeric comparison, ascending
struct as_value_num_nocase_lt : public as_value_lt
{
    as_value_num_nocase_lt(int version) : as_value_lt(version) {}
    bool operator() (const as_value& a, const as_value& b)
    {
        if (a.is_string() || b.is_string())
            return str_nocase_cmp(a, b) < 0;
        return as_value_numLT(a, b);
    }
};

// case-insensitive numeric comparison, descending
struct as_value_num_nocase_gt : public as_value_lt
{
    as_value_num_nocase_gt(int version) : as_value_lt(version) {}
    bool operator() (const as_value& a, const as_value& b)
    {
        if (a.is_string() || b.is_string())
            return str_nocase_cmp(a, b) > 0;
        return as_value_numGT(a, b);
    }
};

// case-insensitive numeric equality
struct as_value_num_nocase_eq : public as_value_lt
{
    as_value_num_nocase_eq(int version) : as_value_lt(version) {}
    bool operator() (const as_value& a, const as_value& b)
    {
        if (a.is_string() || b.is_string())
            return str_nocase_cmp(a, b) == 0;
        return as_value_numEQ(a, b);
    }
};

// Return basic as_value comparison functor for corresponding sort flag
// Note:
// fUniqueSort and fReturnIndexedArray must first be stripped from the flag
as_cmp_fn
get_basic_cmp(boost::uint8_t flags, int version)
{
    as_cmp_fn f;

    // fUniqueSort and fReturnIndexedArray must be stripped by caller
    assert(flags^Array_as::fUniqueSort);
    assert(flags^Array_as::fReturnIndexedArray);

    switch ( flags )
    {
        case 0: // default string comparison
            f = as_value_lt(version);
            return f;

        case Array_as::fDescending:
            f = as_value_gt(version);
            return f;

        case Array_as::fCaseInsensitive: 
            f = as_value_nocase_lt(version);
            return f;

        case Array_as::fCaseInsensitive | 
                Array_as::fDescending:
            f = as_value_nocase_gt(version);
            return f;

        case Array_as::fNumeric: 
            f = as_value_num_lt(version);
            return f;

        case Array_as::fNumeric | Array_as::fDescending:
            f = as_value_num_gt(version);
            return f;

        case Array_as::fCaseInsensitive | 
                Array_as::fNumeric:
            f = as_value_num_nocase_lt(version);
            return f;

        case Array_as::fCaseInsensitive | 
                Array_as::fNumeric |
                Array_as::fDescending:
            f = as_value_num_nocase_gt(version);
            return f;

        default:
            log_unimpl(_("Unhandled sort flags: %d (0x%X)"), (int)flags, (int)flags);
            f = as_value_lt(version);
            return f;
    }
}

// Return basic as_value equality functor for corresponding sort flag
// Note:
// fUniqueSort and fReturnIndexedArray must first be stripped from the flag
as_cmp_fn
get_basic_eq(boost::uint8_t flags, int version)
{
    as_cmp_fn f;
    flags &= ~(Array_as::fDescending);

    switch ( flags )
    {
        case 0: // default string comparison
            f = as_value_eq(version);
            return f;

        case Array_as::fCaseInsensitive: 
            f = as_value_nocase_eq(version);
            return f;

        case Array_as::fNumeric: 
            f = as_value_num_eq(version);
            return f;

        case Array_as::fCaseInsensitive | 
                Array_as::fNumeric:
            f = as_value_num_nocase_eq(version);
            return f;

        default:
            f = as_value_eq(version);
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
    as_environment& _env;

    as_value_custom(as_function& comparator, bool (*zc)(const int), 
        boost::intrusive_ptr<as_object> this_ptr, as_environment& env)
        :
        _comp(comparator),
        _zeroCmp(zc),
        _env(env)
    {
        _object = this_ptr.get();
    }

    bool operator() (const as_value& a, const as_value& b)
    {
        as_value cmp_method(&_comp);
        as_value ret(0.0);

	    std::auto_ptr<std::vector<as_value> > args (new std::vector<as_value>);
	    args->push_back(b);
	    args->push_back(a);
        ret = call_method(cmp_method, &_env, _object, args);

        return (*_zeroCmp)(ret.to_int());
    }
};

// Comparator for sorting on a single array property
class as_value_prop
{
public:
    as_cmp_fn _comp;
    string_table::key _prop;
    
    // Note: cmpfn must implement a strict weak ordering
    as_value_prop(string_table::key name, 
        as_cmp_fn cmpfn)
        :
        _comp(cmpfn),
        _prop(name)
    {
    }

    bool operator() (const as_value& a, const as_value& b)
    {
        as_value av, bv;

        // why do we cast ao/bo to objects here ?
        boost::intrusive_ptr<as_object> ao = a.to_object();
        boost::intrusive_ptr<as_object> bo = b.to_object();
        
        ao->get_member(_prop, &av);
        bo->get_member(_prop, &bv);
        return _comp(av, bv);
    }
};

// Comparator for sorting on multiple array properties
class as_value_multiprop
{
public:
    typedef std::deque<as_cmp_fn> Comps;
    Comps& _cmps;

    typedef std::deque<string_table::key> Props;
    Props& _prps;

    // Note: all as_cmp_fns in *cmps must implement strict weak ordering
    as_value_multiprop(std::deque<string_table::key>& prps, 
        std::deque<as_cmp_fn>& cmps)
        :
        _cmps(cmps),
        _prps(prps)
    {
    }

    bool operator() (const as_value& a, const as_value& b)
    {
        if ( _cmps.empty() ) return false;

        std::deque<as_cmp_fn>::iterator cmp = _cmps.begin();

        // why do we cast ao/bo to objects here ?
        boost::intrusive_ptr<as_object> ao = a.to_object();
        boost::intrusive_ptr<as_object> bo = b.to_object();
        
        for (Props::iterator pit = _prps.begin(), pend = _prps.end(); pit != pend; ++pit, ++cmp)
        {
            as_value av, bv;

            ao->get_member(*pit, &av);
            bo->get_member(*pit, &bv);

            if ( (*cmp)(av, bv) ) return true;
            if ( (*cmp)(bv, av) ) return false;
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
    as_value_multiprop_eq(std::deque<string_table::key>& prps, 
        std::deque<as_cmp_fn>& cmps)
        : as_value_multiprop(prps, cmps)
    {
    }

    bool operator() (const as_value& a, const as_value& b)
    {
        if ( _cmps.empty() ) return false;

        Comps::const_iterator cmp = _cmps.begin();

        // why do we cast ao/bo to objects here ?
        boost::intrusive_ptr<as_object> ao = a.to_object();
        boost::intrusive_ptr<as_object> bo = b.to_object();

        for (Props::iterator pit = _prps.begin(), pend = _prps.end(); pit != pend; ++pit, ++cmp)
        {
            as_value av, bv;
            ao->get_member(*pit, &av);
            bo->get_member(*pit, &bv);

            if ( !(*cmp)(av, bv) ) return false;
        }
        
        return true;
    }
};

// Convenience function to strip fUniqueSort and fReturnIndexedArray from sort
// flag. Presence of flags recorded in douniq and doindex.
static inline boost::uint8_t
flag_preprocess(boost::uint8_t flgs, bool* douniq, bool* doindex)
{
    *douniq = (flgs & Array_as::fUniqueSort);
    *doindex = (flgs & Array_as::fReturnIndexedArray);
    flgs &= ~(Array_as::fReturnIndexedArray);
    flgs &= ~(Array_as::fUniqueSort);
    return flgs;
}

// Convenience function to process and extract flags from an as_value array
// of flags (as passed to sortOn when sorting on multiple properties)
std::deque<boost::uint8_t> 
get_multi_flags(Array_as::const_iterator itBegin, 
    Array_as::const_iterator itEnd, bool* uniq, bool* index)
{
    Array_as::const_iterator it = itBegin;
    std::deque<boost::uint8_t> flgs;

    // extract fUniqueSort and fReturnIndexedArray from first flag
    if (it != itEnd)
    {
        boost::uint8_t flag = static_cast<boost::uint8_t>((*it++).to_number());
        flag = flag_preprocess(flag, uniq, index);
        flgs.push_back(flag);
    }

    while (it != itEnd)
    {
        boost::uint8_t flag = static_cast<boost::uint8_t>((*it++).to_number());
        flag &= ~(Array_as::fReturnIndexedArray);
        flag &= ~(Array_as::fUniqueSort);
        flgs.push_back(flag);
    }
    return flgs;
}

Array_as::Array_as()
    :
    as_object(getArrayInterface()), // pass Array inheritance
    elements(0)
{
    //IF_VERBOSE_ACTION (
    //log_action("%s: %p", __FUNCTION__, (void*)this);
    //)
    attachArrayProperties(*this);
}

Array_as::Array_as(const Array_as& other)
    :
    as_object(other),
    elements(other.elements)
{
    //IF_VERBOSE_ACTION (
    //log_action("%s: %p", __FUNCTION__, (void*)this);
    //)
}

Array_as::~Array_as() 
{
}

std::deque<indexed_as_value>
Array_as::get_indexed_elements()
{
    std::deque<indexed_as_value> indexed_elements;
    int i = 0;

    for (Array_as::const_iterator it = elements.begin(), e = elements.end();
        it != e; ++it)
    {
        indexed_elements.push_back(indexed_as_value(*it, i++));
    }
    return indexed_elements;
}

Array_as::const_iterator
Array_as::begin()
{
    return elements.begin();
}

Array_as::const_iterator
Array_as::end()
{
    return elements.end();
}

int
Array_as::index_requested(string_table::key name)
{
    const std::string& nameString = _vm.getStringTable().value(name);

    // Anything not in [0-9] makes this an invalid index
    if ( nameString.find_first_not_of("0123456789") != std::string::npos )
    {
        return -1;
    }

    // TODO: do we need all this noise ? atol(3) should do !

    as_value temp;
    temp.set_string(nameString);
    double value = temp.to_number();

    // if we were sent a string that can't convert like "asdf", it returns as NaN. -1 means invalid index
    if (!utility::isFinite(value)) return -1;

    return int(value);
}

void
Array_as::push(const as_value& val)
{
        const ArrayContainer::size_type s = elements.size();
        elements.resize(s+1);
        elements[s] = val;
}

void
Array_as::unshift(const as_value& val)
{
        shiftElementsRight(1);
        elements[0] = val;
}

as_value
Array_as::pop()
{
    // If the array is empty, report an error and return undefined!
    const ArrayContainer::size_type s = elements.size();

    if ( ! s )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("tried to pop element from back of empty array, returning undef"));
        );
        return as_value(); // undefined
    }

    as_value ret = elements[s - 1];
    elements.resize(s - 1);

    return ret;
}

as_value
Array_as::shift()
{
    const ArrayContainer::size_type s = elements.size();

    // If the array is empty, report an error and return undefined!
    if ( ! s )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("tried to shift element from front of empty array, returning undef"));
        );
        return as_value(); // undefined
    }

    as_value ret = elements[0];
    shiftElementsLeft(1);

    return ret;
}

void
Array_as::reverse()
{
    const ArrayContainer::size_type s = elements.size();
    if ( s < 2 ) return; // nothing to do (CHECKME: might be a single hole!)

    // We create another container, as we want to fill the gaps
    // There could likely be an in-place version for this, but
    // filling the gaps would need more care
    ArrayContainer newelements(s);

    for (size_t i = 0, n = s - 1; i < s; ++i, --n)
    {
        newelements[i] = elements[n];
    }

    elements = newelements;
}

std::string
Array_as::join(const std::string& separator, as_environment*) const
{
    // TODO - confirm this is the right format!
    // Reportedly, flash version 7 on linux, and Flash 8 on IE look like
    // "(1,2,3)" and "1,2,3" respectively - which should we mimic?
    // Using no parentheses until confirmed for sure
    //
    // We should change output based on SWF version --strk 2006-04-28

    std::string temp;

    const ArrayContainer::size_type s = elements.size();

    if ( s ) 
    {
        int swfversion = _vm.getSWFVersion();

        for (size_t i = 0; i < s; ++i)
        {
            if ( i ) temp += separator;
            temp += elements[i].to_string_versioned(swfversion);
        }
    }

    return temp;

}

void
Array_as::concat(const Array_as& other)
{
    for (ArrayContainer::size_type i = 0, e = other.size(); i < e; i++)
    {
        push(other.at(i));
    }
}

std::string
Array_as::toString(as_environment* env) const
{
    return join(",", env);
}

unsigned int
Array_as::size() const
{
    return elements.size();
}

as_value
Array_as::at(unsigned int index) const
{
    if ( index > elements.size()-1 ) return as_value();
    else return elements[index];
}

boost::intrusive_ptr<Array_as>
Array_as::slice(unsigned int start, unsigned int one_past_end)
{
    assert(one_past_end >= start);
    assert(one_past_end <= size());
    assert(start <= size());

    boost::intrusive_ptr<Array_as> newarray(new Array_as);

#ifdef GNASH_DEBUG
    log_debug(_("Array.slice(%u, %u) called"), start, one_past_end);
#endif

    size_t newsize = one_past_end - start;
    newarray->elements.resize(newsize);

    // maybe there's a standard algorithm for this ?
    for (unsigned int i=start; i<one_past_end; ++i)
    {
        newarray->elements[i-start] = elements[i];
    }

    return newarray;

}

bool
Array_as::removeFirst(const as_value& v)
{
    for (iterator it = elements.begin(), e = elements.end(); it != e; ++it)
    {
        if ( v.equals(*it) )
        {
            splice(it.index(), 1);
            return true;
        }
    }
    return false;
}

/* virtual public, overriding as_object::get_member */
bool
Array_as::get_member(string_table::key name, as_value *val,
    string_table::key nsname)
{
    // an index has been requested
    int index = index_requested(name);

    if ( index >= 0 ) // a valid index was requested
    {
        size_t i = index;
        const_iterator it = elements.find(i);
        if ( it != elements.end() && it.index() == i )
        {
            *val = *it;
            return true;
        }
    }

    return as_object::get_member(name, val, nsname);
}

bool
Array_as::hasOwnProperty(string_table::key name, string_table::key nsname)
{
    // an index has been requested
    int index = index_requested(name);

    if ( index >= 0 ) // a valid index was requested
    {
        size_t i = index;
        const_iterator it = elements.find(i);
        if ( it != elements.end() && it.index() == i )
        {
            return true;
        }
    }

    return as_object::hasOwnProperty(name, nsname);
}

std::pair<bool,bool> 
Array_as::delProperty(string_table::key name, string_table::key nsname)
{
    // an index has been requested
    int index = index_requested(name);

    if ( index >= 0 ) // a valid index was requested
    {
        size_t i = index;
        const_iterator it = elements.find(i);
        if ( it != elements.end() && it.index() == i )
        {
            elements.erase_element(i);
            return std::make_pair(true, true);
        }
    }

    return as_object::delProperty(name, nsname);
}

void
Array_as::resize(unsigned int newsize)
{
    elements.resize(newsize);
}

void
Array_as::set_indexed(unsigned int index,
    const as_value& val)
{
    if (index >= elements.size())
    {
        // make sure the vector is large enough.
        elements.resize(index + 1);
    }

    elements[index] = val;
    return;
}

/* virtual public, overriding as_object::set_member */
bool
Array_as::set_member(string_table::key name,
        const as_value& val, string_table::key nsname, bool ifFound)
{
    int index = index_requested(name);

    // if we were sent a valid array index and not a normal member
    if (index >= 0)
    {
        if ( size_t(index) >= elements.size() )
        {
            // if we're setting index (x), the vector
            // must be size (x+1)
            elements.resize(index+1);
        }

        // set the appropriate index and return
        elements[index] = val;
        return true;
    }


    return as_object::set_member(name,val, nsname, ifFound);
}

Array_as*
Array_as::get_indices(std::deque<indexed_as_value> elems)
{
    Array_as* intIndexes = new Array_as();

    for (std::deque<indexed_as_value>::const_iterator it = elems.begin();
        it != elems.end(); ++it)
    {
        intIndexes->push(as_value(it->vec_index));
    }
    return intIndexes;
}

static as_value
array_splice(const fn_call& fn)
{
    boost::intrusive_ptr<Array_as> array = ensureType<Array_as>(fn.this_ptr);

#ifdef GNASH_DEBUG
    std::stringstream ss;
    fn.dump_args(ss);
    log_debug(_("Array(%s).splice(%s) called"), array->toString(), ss.str());
#endif

    if (fn.nargs < 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Array.splice() needs at least 1 argument, call ignored"));
        );
        return as_value();
    }

    unsigned origlen = array->size();

    //----------------
    // Get start offset
    //----------------
    unsigned startoffset;
    int start = fn.arg(0).to_int();
    if ( start < 0 ) start = array->size()+start; // start is negative, so + means -abs()
    startoffset = utility::clamp<int>(start, 0, origlen);
#ifdef GNASH_DEBUG
    if ( startoffset != start )
        log_debug(_("Array.splice: start:%d became %u"), start, startoffset);
#endif

    //----------------
    // Get length
    //----------------
    unsigned len = origlen - start;
    if (fn.nargs > 1)
    {
        int lenval = fn.arg(1).to_int();
        if ( lenval < 0 )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Array.splice(%d,%d): negative length given, call ignored"),
                start, lenval);
            );
            return as_value();
        }
        len = utility::clamp<int>(lenval, 0, origlen-startoffset);
    }

    //----------------
    // Get replacement
    //----------------
    std::vector<as_value> replace;
    for (unsigned i=2; i<fn.nargs; ++i)
    {
        replace.push_back(fn.arg(i));
    }

    Array_as* ret = new Array_as();
    array->splice(startoffset, len, &replace, ret);

    return as_value(ret);
}

static as_value
array_sort(const fn_call& fn)
{
    boost::intrusive_ptr<Array_as> array = 
        ensureType<Array_as>(fn.this_ptr);
    
    const int version = array->getVM().getSWFVersion();
    
    if (!fn.nargs)
    {
        array->sort(as_value_lt(version));
        return as_value(array.get());
    }
    
    if (fn.arg(0).is_undefined()) return as_value();

    boost::uint8_t flags = 0;

    if ( fn.nargs == 1 && fn.arg(0).is_number() )
    {
        flags=static_cast<boost::uint8_t>(fn.arg(0).to_number());
    }
    else if (fn.arg(0).is_as_function())
    {

        // Get comparison function
        as_function* as_func = fn.arg(0).to_as_function();

        assert(as_func);

        bool (*icmp)(int);
    
        if (fn.nargs == 2 && fn.arg(1).is_number()) {
            flags=static_cast<boost::uint8_t>(fn.arg(1).to_number());
        }

        if (flags & Array_as::fDescending) icmp = &int_lt_or_eq;
        else icmp = &int_gt;

        as_environment& env = fn.env();

        as_value_custom avc = 
            as_value_custom(*as_func, icmp, fn.this_ptr, env);

        if ((flags & Array_as::fReturnIndexedArray))
        {
            return as_value(array->sort_indexed(avc));
        }

        array->sort(avc);
        return as_value(array.get());
        // note: custom AS function sorting apparently ignores the 
        // UniqueSort flag which is why it is also ignored here
    }
    else
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Sort called with invalid arguments."));
        )
        return as_value(array.get());
    }
    bool do_unique, do_index;
    flags = flag_preprocess(flags, &do_unique, &do_index);
    as_cmp_fn comp = get_basic_cmp(flags, version);

    if (do_unique)
    {
        as_cmp_fn eq =
            get_basic_eq(flags, version);
        if (do_index) return array->sort_indexed(comp, eq);
        return array->sort(comp, eq);
    }
    if (do_index) return as_value(array->sort_indexed(comp));
    array->sort(comp);
    return as_value(array.get());
}

static as_value
array_sortOn(const fn_call& fn)
{
    boost::intrusive_ptr<Array_as> array = 
        ensureType<Array_as>(fn.this_ptr);

    bool do_unique = false, do_index = false;
    boost::uint8_t flags = 0;

    VM& vm = array->getVM();
    int version = vm.getSWFVersion();
    string_table& st = vm.getStringTable();

    // cases: sortOn("prop) and sortOn("prop", Array.FLAG)
    if ( fn.nargs > 0 && fn.arg(0).is_string() )
    {
        string_table::key propField = st.find(PROPNAME(fn.arg(0).to_string_versioned(version)));

        if ( fn.nargs > 1 && fn.arg(1).is_number() )
        {
            flags = static_cast<boost::uint8_t>(fn.arg(1).to_number());
            flags = flag_preprocess(flags, &do_unique, &do_index);
        }
        as_value_prop avc = as_value_prop(propField, 
                    get_basic_cmp(flags, version));
        if (do_unique)
        {
            as_value_prop ave = as_value_prop(propField, 
                get_basic_eq(flags, version));
            if (do_index)
                return array->sort_indexed(avc, ave);
            return array->sort(avc, ave);
        }
        if (do_index)
            return as_value(array->sort_indexed(avc));
        array->sort(avc);
        return as_value(array.get());
    }

    // case: sortOn(["prop1", "prop2"] ...)
    if (fn.nargs > 0 && fn.arg(0).is_object() ) 
    {
        boost::intrusive_ptr<Array_as> props = 
            ensureType<Array_as>(fn.arg(0).to_object());
        std::deque<string_table::key> prp;
        unsigned int optnum = props->size();
        std::deque<as_cmp_fn> cmp;
        std::deque<as_cmp_fn> eq;

        for (Array_as::const_iterator it = props->begin();
            it != props->end(); ++it)
        {
            string_table::key s = st.find(PROPNAME((*it).to_string_versioned(version)));
            prp.push_back(s);
        }
        
        // case: sortOn(["prop1", "prop2"])
        if (fn.nargs == 1)
        {
            // assign each cmp function to the standard cmp fn
            as_cmp_fn c = get_basic_cmp(0, version);
            cmp.assign(optnum, c);
        }
        // case: sortOn(["prop1", "prop2"], [Array.FLAG1, Array.FLAG2])
        else if ( fn.arg(1).is_object() )
        {
            boost::intrusive_ptr<Array_as> farray = 
                ensureType<Array_as>(fn.arg(1).to_object());
            if (farray->size() == optnum)
            {
                Array_as::const_iterator 
                    fBegin = farray->begin(),
                    fEnd = farray->end();

                std::deque<boost::uint8_t> flgs = 
                    get_multi_flags(fBegin, fEnd, 
                        &do_unique, &do_index);

                std::deque<boost::uint8_t>::const_iterator it = 
                    flgs.begin();

                while (it != flgs.end())
                    cmp.push_back(get_basic_cmp(*it++, version));

                if (do_unique)
                {
                    it = flgs.begin();
                    while (it != flgs.end())
                        eq.push_back(get_basic_eq(*it++, version));
                }
            }
            else
            {
                as_cmp_fn c = get_basic_cmp(0, version);
                cmp.assign(optnum, c);
            }
        }
        // case: sortOn(["prop1", "prop2"], Array.FLAG)
        else if ( fn.arg(1).is_number() )
        {
            boost::uint8_t flags = 
                static_cast<boost::uint8_t>(fn.arg(1).to_number());
            flags = flag_preprocess(flags, &do_unique, &do_index);
            as_cmp_fn c = get_basic_cmp(flags, version);

            cmp.assign(optnum, c);
            
            if (do_unique)
            {
                as_cmp_fn e = get_basic_eq(flags, version);
                eq.assign(optnum, e);
            }
        }

        as_value_multiprop avc = 
            as_value_multiprop(prp, cmp);

        if (do_unique)
        {
            as_value_multiprop_eq ave = 
                as_value_multiprop_eq(prp, eq);
            if (do_index)
                return array->sort_indexed(avc, ave);
            return array->sort(avc, ave);
        }
        if (do_index)
            return as_value(array->sort_indexed(avc));
        array->sort(avc);
        return as_value(array.get());

    }
    IF_VERBOSE_ASCODING_ERRORS(
    log_aserror(_("SortOn called with invalid arguments."));
    )
    if (fn.nargs == 0 )
        return as_value();

    return as_value(array.get());
}

// Callback to push values to the back of an array
static as_value
array_push(const fn_call& fn)
{
    boost::intrusive_ptr<Array_as> array = ensureType<Array_as>(fn.this_ptr);

        IF_VERBOSE_ACTION (
    log_action(_("calling array push, pushing %d values onto back of array"),fn.nargs);
        );

    for (unsigned int i=0;i<fn.nargs;i++)
        array->push(fn.arg(i));

    return as_value(array->size());
}

// Callback to push values to the front of an array
static as_value
array_unshift(const fn_call& fn)
{
    boost::intrusive_ptr<Array_as> array = ensureType<Array_as>(fn.this_ptr);

        IF_VERBOSE_ACTION (
    log_action(_("calling array unshift, pushing %d values onto front of array"), fn.nargs);
        );

    for (int i=fn.nargs-1; i>=0; i--)
        array->unshift(fn.arg(i));

    return as_value(array->size());
}

// Callback to pop a value from the back of an array
static as_value
array_pop(const fn_call& fn)
{
    boost::intrusive_ptr<Array_as> array = ensureType<Array_as>(fn.this_ptr);

    // Get our index, log, then return result
    as_value rv = array->pop();

    IF_VERBOSE_ACTION (
    log_action(_("calling array pop, result:%s, new array size:%d"),
        rv, array->size());
    );
        return rv;
}

// Callback to pop a value from the front of an array
static as_value
array_shift(const fn_call& fn)
{
    boost::intrusive_ptr<Array_as> array = ensureType<Array_as>(fn.this_ptr);

    // Get our index, log, then return result
    as_value rv = array->shift();

    IF_VERBOSE_ACTION (
    log_action(_("calling array shift, result:%s, new array size:%d"),
        rv, array->size());
    );
    return rv;
}

// Callback to reverse the position of the elements in an array
static as_value
array_reverse(const fn_call& fn)
{
    boost::intrusive_ptr<Array_as> array = ensureType<Array_as>(fn.this_ptr);

    array->reverse();

    as_value rv(array.get()); 

    IF_VERBOSE_ACTION (
    log_action(_("called array reverse, result:%s, new array size:%d"),
        rv, array->size());
    );
    return rv;
}

// Callback to convert array to a string with optional custom separator (default ',')
static as_value
array_join(const fn_call& fn)
{
    boost::intrusive_ptr<Array_as> array = ensureType<Array_as>(fn.this_ptr);

    std::string separator = ",";
    int version = array->getVM().getSWFVersion();
    as_environment* env = &(fn.env());

    if (fn.nargs > 0)
    {
        separator = fn.arg(0).to_string_versioned(version);
    }

    std::string ret = array->join(separator, env);

    return as_value(ret);
}

// Callback to convert array to a string
// TODO CHECKME: rely on Object.toString  ? (
static as_value
array_to_string(const fn_call& fn)
{
    boost::intrusive_ptr<Array_as> array = ensureType<Array_as>(fn.this_ptr);

    std::string ret = array->toString();

        IF_VERBOSE_ACTION
        (
    log_action(_("array_to_string called, nargs = %d, "
            "this_ptr = %p"),
            fn.nargs, (void*)fn.this_ptr.get());
    log_action(_("to_string result is: %s"), ret);
        );

    return as_value(ret);
}

/// concatenates the elements specified in the parameters with
/// the elements in my_array, and creates a new array. If the
/// value parameters specify an array, the elements of that
/// array are concatenated, rather than the array itself. The
/// array my_array is left unchanged.
static as_value
array_concat(const fn_call& fn)
{
    boost::intrusive_ptr<Array_as> array = ensureType<Array_as>(fn.this_ptr);

    // use copy ctor
    Array_as* newarray = new Array_as();

    for (size_t i=0, e=array->size(); i<e; i++)
        newarray->push(array->at(i));

    for (unsigned int i=0; i<fn.nargs; i++)
    {
        // Array args get concatenated by elements
        boost::intrusive_ptr<Array_as> other = boost::dynamic_pointer_cast<Array_as>(fn.arg(i).to_object());
        if ( other )
        {
            newarray->concat(*other);
        }
        else
        {
            newarray->push(fn.arg(i));
        }
    }

    return as_value(newarray);        
}

// Callback to slice part of an array to a new array
// without changing the original
static as_value
array_slice(const fn_call& fn)
{
    boost::intrusive_ptr<Array_as> array = ensureType<Array_as>(fn.this_ptr);

    // start and end index of the part we're slicing
    int startindex, endindex;
    unsigned int arraysize = array->size();

    if (fn.nargs > 2)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("More than 2 arguments to Array.slice, "
            "and I don't know what to do with them.  "
            "Ignoring them"));
        );
    }

    // They passed no arguments: simply duplicate the array
    // and return the new one
    if (fn.nargs < 1)
    {
        Array_as* newarray = new Array_as(*array);
        return as_value(newarray);
    }


    startindex = fn.arg(0).to_int();

    // if the index is negative, it means "places from the end"
    // where -1 is the last element
    if (startindex < 0) startindex = startindex + arraysize;

    // if we sent at least two arguments, setup endindex
    if (fn.nargs >= 2)
    {
        endindex = fn.arg(1).to_int();

        // if the index is negative, it means
        // "places from the end" where -1 is the last element
        if (endindex < 0) endindex = endindex + arraysize;
    }
    else
    {
        // They didn't specify where to end,
        // so choose the end of the array
        endindex = arraysize;
    }

    if ( startindex < 0 ) startindex = 0;
    else if ( static_cast<size_t>(startindex) > arraysize ) startindex = arraysize;

    if ( endindex < startindex ) endindex = startindex;
    else if ( static_cast<size_t>(endindex)  > arraysize ) endindex = arraysize;

    boost::intrusive_ptr<Array_as> newarray(array->slice(
        startindex, endindex));

    return as_value(newarray.get());        
}

static as_value
array_length(const fn_call& fn)
{
    boost::intrusive_ptr<Array_as> array = ensureType<Array_as>(fn.this_ptr);

    if ( fn.nargs ) // setter
    {
        int length = fn.arg(0).to_int();
        if ( length < 0 ) // TODO: set a max limit too ?
        {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror("Attempt to set Array.length to a negative value %d", length);
            )
            length = 0;
        }

        array->resize(length);
        return as_value();
    }
    else // getter
    {
        return as_value(array->size());
    }
}

as_value
array_new(const fn_call& fn)
{
    IF_VERBOSE_ACTION (
    log_action(_("array_new called, nargs = %d"), fn.nargs);
    );

    boost::intrusive_ptr<Array_as> ao = new Array_as;

    if (fn.nargs == 0)
    {
        // Empty array.
    }
    else if (fn.nargs == 1 && fn.arg(0).is_number() )
    {
        // TODO: limit max size !!
        int newSize = fn.arg(0).to_int();
        if ( newSize < 0 ) newSize = 0;
        else ao->resize(newSize);
    }
    else
    {
        // Use the arguments as initializers.
        as_value    index_number;
        for (unsigned int i = 0; i < fn.nargs; i++)
        {
            ao->push(fn.arg(i));
        }
    }

    IF_VERBOSE_ACTION (
    log_action(_("array_new setting object %p in result"), (void*)ao.get());
    );

    return as_value(ao.get());
    //return as_value(ao);
}

static void
attachArrayProperties(as_object& proto)
{
    proto.init_property(NSV::PROP_LENGTH, &array_length, &array_length);
}

static void
attachArrayStatics(as_object& proto)
{
    int flags = 0; // these are not protected
    proto.init_member("CASEINSENSITIVE", Array_as::fCaseInsensitive, flags);
    proto.init_member("DESCENDING", Array_as::fDescending, flags);
    proto.init_member("UNIQUESORT", Array_as::fUniqueSort, flags);
    proto.init_member("RETURNINDEXEDARRAY", Array_as::fReturnIndexedArray, flags);
    proto.init_member("NUMERIC", Array_as::fNumeric, flags);
}

static void
attachArrayInterface(as_object& proto)
{
    VM& vm = proto.getVM();

    // Array.push
    vm.registerNative(array_push, 252, 1);
    proto.init_member("push", vm.getNative(252, 1));

    // Array.pop
    vm.registerNative(array_pop, 252, 2);
    proto.init_member("pop", vm.getNative(252, 2));

    // Array.concat
    vm.registerNative(array_concat, 252, 3);
    proto.init_member("concat", vm.getNative(252, 3));

    // Array.shift
    vm.registerNative(array_shift, 252, 4);
    proto.init_member("shift", vm.getNative(252, 4));

    // Array.unshift
    vm.registerNative(array_unshift, 252, 5);
    proto.init_member("unshift", vm.getNative(252, 5));

    // Array.slice
    vm.registerNative(array_slice, 252, 6);
    proto.init_member("slice", vm.getNative(252, 6));

    // Array.join
    vm.registerNative(array_join, 252, 7);
    proto.init_member("join", vm.getNative(252, 7));

    // Array.splice
    vm.registerNative(array_splice, 252, 8);
    proto.init_member("splice", vm.getNative(252, 8));

    // Array.toString
    vm.registerNative(array_to_string, 252, 9);
    proto.init_member("toString", vm.getNative(252, 9));

    // Array.sort
    vm.registerNative(array_sort, 252, 10);
    proto.init_member("sort", vm.getNative(252, 10));

    // Array.reverse
    vm.registerNative(array_reverse, 252, 11);
    proto.init_member("reverse", vm.getNative(252, 11));

    // Array.sortOn
    vm.registerNative(array_sortOn, 252, 12);
    proto.init_member("sortOn", vm.getNative(252, 12));

}

static as_object*
getArrayInterface()
{
    static boost::intrusive_ptr<as_object> proto = NULL;
    if ( proto == NULL )
    {
        proto = new as_object(getObjectInterface());
        proto->getVM().addStatic(proto.get());

        attachArrayInterface(*proto);
    }
    return proto.get();
}

static as_function*
getArrayConstructor(VM& vm)
{
    // This is going to be the global Array "class"/"function"
    static as_function* ar=0;

    if ( ar == NULL )
    {
        vm.registerNative(array_new, 252, 0);
        ar = new builtin_function(&array_new, getArrayInterface());
        vm.addStatic(ar);

        // Attach static members
        attachArrayStatics(*ar);
    }

    return ar;
}

// this registers the "Array" member on a "Global"
// object. "Array" is a constructor, thus an object
// with .prototype full of exported functions + 
// 'constructor'
//
void
array_class_init(as_object& glob)
{
    // Register _global.Array
    int flags = as_prop_flags::dontEnum; // |as_prop_flags::onlySWF5Up; 
    glob.init_member("Array", getArrayConstructor(glob.getVM()), flags);
}

void
Array_as::enumerateNonProperties(as_environment& env) const
{
    std::stringstream ss; 
    for (const_iterator it = elements.begin(),
        itEnd = elements.end(); it != itEnd; ++it)
    {
        int idx = it.index();
        // enumerated values need to be strings, not numbers
        ss.str(""); ss << idx;
        env.push(as_value(ss.str()));
    }
}

void
Array_as::shiftElementsLeft(unsigned int count)
{
    ArrayContainer& v = elements;

    if ( count >= v.size() )
    {
	// NOTE: v.clear() would NOT set size to 0 !!
        v.resize(0);
        return;
    }

    for (unsigned int i=0; i<count; ++i) v.erase_element(i);

    for (iterator i=v.begin(), e=v.end(); i!=e; ++i)
    {
        int currentIndex = i.index();
        int newIndex = currentIndex-count;
        v[newIndex] = *i;
    }
    v.resize(v.size()-count);
}

void
Array_as::shiftElementsRight(unsigned int count)
{
    ArrayContainer& v = elements;

    v.resize(v.size()+count);
        for (ArrayContainer::reverse_iterator i=v.rbegin(), e=v.rend(); i!=e; ++i)
    {
        int currentIndex = i.index();
        int newIndex = currentIndex+count;
        v[newIndex] = *i;
    }
    while (count--) v.erase_element(count);
}

void
Array_as::splice(unsigned int start, unsigned int count, const std::vector<as_value>* replace, Array_as* receive)
{
    size_t sz = elements.size();

    assert ( start <= sz );
    assert ( start+count <= sz );

    size_t newsize = sz-count;
    if ( replace ) newsize += replace->size();
    ArrayContainer newelements(newsize);

    size_t ni=0;

    // add initial portion
    for (size_t i=0; i<start; ++i )
    {
        newelements[ni++] = elements[i];
    }

    // add replacement, if any
    if ( replace )
    {
        for (size_t i=0, e=replace->size(); i<e; ++i) 
            newelements[ni++] = replace->at(i);
    }    

    // add final portion
    for (size_t i=start+count; i<sz; ++i )
        newelements[ni++] = elements[i];

    // push trimmed data to the copy array, if any
    if ( receive )
    {
        for (size_t i=start; i<start+count; ++i )
            receive->push(elements[i]);
    }

    elements = newelements;
}

#ifdef GNASH_USE_GC
void
Array_as::markReachableResources() const
{
    for (const_iterator i=elements.begin(), e=elements.end(); i!=e; ++i)
    {
        (*i).setReachable();
    }
    markAsObjectReachable();
}
#endif // GNASH_USE_GC

void
Array_as::visitPropertyValues(AbstractPropertyVisitor& visitor) const
{
    std::stringstream ss; 
    string_table& st = getVM().getStringTable();
    for (const_iterator i=elements.begin(), ie=elements.end(); i!=ie; ++i)
    {
        int idx = i.index();
        ss.str(""); ss << idx;
        string_table::key k = st.find(ss.str());
        visitor.accept(k, *i);
    }

    // visit proper properties
    as_object::visitPropertyValues(visitor);
}

void
Array_as::visitNonHiddenPropertyValues(AbstractPropertyVisitor& visitor) const
{
    std::stringstream ss; 
    string_table& st = getVM().getStringTable();
    for (const_iterator i=elements.begin(), ie=elements.end(); i!=ie; ++i)
    {
        // TODO: skip hidden ones
        int idx = i.index();
        ss.str(""); ss << idx;
        string_table::key k = st.find(ss.str());
        visitor.accept(k, *i);
    }

    // visit proper properties
    as_object::visitNonHiddenPropertyValues(visitor);
}

bool
Array_as::isStrict() const
{
    if ( hasNonHiddenProperties() ) return false;
    return true;
}

} // end of gnash namespace


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

