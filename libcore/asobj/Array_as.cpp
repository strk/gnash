// Array_as.cpp:  ActionScript array class, for Gnash.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "smart_ptr.h" // GNASH_USE_GC
#include "as_value.h"
#include "Array_as.h"
#include "log.h"
#include "builtin_function.h" // for Array class
#include "NativeFunction.h" 
#include "as_function.h" // for sort user-defined comparator
#include "fn_call.h"
#include "Global_as.h"
#include "GnashException.h"
#include "action.h" // for call_method
#include "VM.h" // for PROPNAME, registerNative
#include "Object.h" // for getObjectInterface()
#include "GnashNumeric.h"

#include <string>
#include <algorithm>
#include <cmath>
#include <boost/algorithm/string/case_conv.hpp>

//#define GNASH_DEBUG 

namespace gnash {

typedef boost::function2<bool, const as_value&, const as_value&> as_cmp_fn;

inline string_table::key
arrayKey(string_table& st, size_t i)
{
    std::ostringstream os;
    os << i;
    return st.find(os.str());
}

namespace {

string_table::key getKey(const fn_call& fn, size_t i) {
    string_table& st = getStringTable(fn);
    return arrayKey(st, i);
}

template<typename T>
bool foreachArray(as_object& array, int start, int end, T& pred)
{
    as_value length;
    if (!array.get_member(NSV::PROP_LENGTH, &length)) return false;
    
    const int size = length.to_int();
    if (size < 0) return false;

    if (start < 0) start = size + start;
    if (start >= size) return false;
    start = std::max(start, 0);

    if (end < 0) end = size + end;
    end = std::max(start, end);
    end = std::min<size_t>(end, size);

    assert(start >= 0);
    assert(end >= start);
    assert(size >= end);

    string_table& st = getStringTable(array);

    for (size_t i = start; i < static_cast<size_t>(end); ++i) {
        pred(array.getMember(arrayKey(st, i)));
    }
    return true;
}

class PushToArray
{
public:
    PushToArray(as_object& obj) : _obj(obj) {}
    void operator()(const as_value& val) {
        _obj.callMethod(NSV::PROP_PUSH, val);
    }
private:
    as_object& _obj;
};

class PushToVector
{
public:
    PushToVector(std::vector<as_value>& v) : _v(v) {}
    void operator()(const as_value& val) {
        _v.push_back(val);
    }
private:
    std::vector<as_value>& _v;
};

}

static as_object* getArrayInterface();
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
        ret = call_method(cmp_method, _env, _object, args);

        return (*_zeroCmp)(ret.to_int());
    }
};

// Comparator for sorting on a single array property
class as_value_prop
{
public:
    
    // Note: cmpfn must implement a strict weak ordering
    as_value_prop(string_table::key name, as_cmp_fn cmpfn, const as_object& o)
        :
        _comp(cmpfn),
        _prop(name),
        _obj(o)
    {
    }

    bool operator() (const as_value& a, const as_value& b)
    {
        as_value av, bv;

        // why do we cast ao/bo to objects here ?
        boost::intrusive_ptr<as_object> ao = a.to_object(*getGlobal(_obj));
        boost::intrusive_ptr<as_object> bo = b.to_object(*getGlobal(_obj));
        
        ao->get_member(_prop, &av);
        bo->get_member(_prop, &bv);
        return _comp(av, bv);
    }
private:
    as_cmp_fn _comp;
    string_table::key _prop;
    const as_object& _obj;
};

// Comparator for sorting on multiple array properties
class as_value_multiprop
{
public:
    typedef std::deque<as_cmp_fn> Comps;
    Comps& _cmps;

    typedef std::deque<string_table::key> Props;
    Props& _prps;
    
    const as_object& _obj;

    // Note: all as_cmp_fns in *cmps must implement strict weak ordering
    as_value_multiprop(std::deque<string_table::key>& prps, 
        std::deque<as_cmp_fn>& cmps, const as_object& o)
        :
        _cmps(cmps),
        _prps(prps),
        _obj(o)
    {
    }

    bool operator() (const as_value& a, const as_value& b)
    {
        if ( _cmps.empty() ) return false;

        std::deque<as_cmp_fn>::iterator cmp = _cmps.begin();

        // why do we cast ao/bo to objects here ?
        boost::intrusive_ptr<as_object> ao = a.to_object(*getGlobal(_obj));
        boost::intrusive_ptr<as_object> bo = b.to_object(*getGlobal(_obj));
        
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
        std::deque<as_cmp_fn>& cmps, const as_object& o)
        :
        as_value_multiprop(prps, cmps, o),
        _obj(o)
    {
    }

    bool operator() (const as_value& a, const as_value& b)
    {
        if ( _cmps.empty() ) return false;

        Comps::const_iterator cmp = _cmps.begin();

        // why do we cast ao/bo to objects here ?
        boost::intrusive_ptr<as_object> ao = a.to_object(*getGlobal(_obj));
        boost::intrusive_ptr<as_object> bo = b.to_object(*getGlobal(_obj));

        for (Props::iterator pit = _prps.begin(), pend = _prps.end(); pit != pend; ++pit, ++cmp)
        {
            as_value av, bv;
            ao->get_member(*pit, &av);
            bo->get_member(*pit, &bv);

            if ( !(*cmp)(av, bv) ) return false;
        }
        
        return true;
    }
private:
    const as_object& _obj;
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
    init_member(NSV::PROP_LENGTH, 0.0);
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
    const std::string& nameString = getStringTable(*this).value(name);

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
    if (!isFinite(value)) return -1;

    return int(value);
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

/* virtual public, overriding as_object::set_member */
bool
Array_as::set_member(string_table::key name,
        const as_value& val, string_table::key nsname, bool ifFound)
{
    int index = index_requested(name);

    // if we were sent a valid array index and not a normal member
    if (index >= 0)
    {
        if (size_t(index) >= elements.size())
        {
            // if we're setting index (x), the vector
            // must be size (x+1)
            elements.resize(index+1);
        }

        as_object::set_member(NSV::PROP_LENGTH, elements.size());
        // set the appropriate index and return
        elements[index] = val;
        return true;
    }

    if (name == NSV::PROP_LENGTH) {
        elements.resize(std::max(val.to_int(), 0));
        // Don't return before as_object has set the value!
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
        intIndexes->callMethod(NSV::PROP_PUSH, it->vec_index);
    }
    return intIndexes;
}

static as_value
array_splice(const fn_call& fn)
{
    as_object* array = ensureType<as_object>(fn.this_ptr);
    
    if (fn.nargs < 1) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Array.splice() needs at least 1 argument, "
                    "call ignored"));
        );
        return as_value();
    }
    
    as_value length;
    if (!array->get_member(NSV::PROP_LENGTH, &length)) return as_value();
    
    const int size = length.to_int();
    if (size < 0) return as_value();

    //----------------
    // Get start offset
    //----------------

    int start = fn.arg(0).to_int();
    if (start < 0) start = size + start; 
    start = clamp<int>(start, 0, size);

    // Get length to delete
    size_t remove = size - start;
    
    if (fn.nargs > 1) {
        int remval = fn.arg(1).to_int();
        if (remval < 0) {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("Array.splice(%d,%d): negative length "
                        "given, call ignored"), start, remval);
            );
            return as_value();
        }
        remove = clamp<int>(remval, 0, size - start);
    }

    Global_as* gl = getGlobal(fn);
    as_object* ret = gl->createArray();

    // Copy the original array values for reinsertion. It's not possible
    // to do a simple copy in-place without overwriting values that still
    // need to be shifted. The algorithm could certainly be improved though.
    std::vector<as_value> v;
    PushToVector pv(v);
    foreachArray(*array, pv);

    const size_t newelements = fn.nargs > 2 ? fn.nargs - 2 : 0;
    
    // Push removed elements to the new array.
    for (size_t i = 0; i < remove; ++i) {
        const size_t key = getKey(fn, start + i);
        ret->callMethod(NSV::PROP_PUSH, array->getMember(key));
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
    array->set_member(NSV::PROP_LENGTH, size + newelements - remove);

    return as_value(ret);
}

static as_value
array_sort(const fn_call& fn)
{
    boost::intrusive_ptr<Array_as> array = 
        ensureType<Array_as>(fn.this_ptr);
    
    const int version = getSWFVersion(*array);
    
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
    else if (fn.arg(0).is_function())
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

        const as_environment& env = fn.env();

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

    int version = getSWFVersion(fn);
    string_table& st = getStringTable(fn);

    // cases: sortOn("prop) and sortOn("prop", Array.FLAG)
    if ( fn.nargs > 0 && fn.arg(0).is_string() )
    {
        string_table::key propField = st.find(fn.arg(0).to_string_versioned(version));

        if ( fn.nargs > 1 && fn.arg(1).is_number() )
        {
            flags = static_cast<boost::uint8_t>(fn.arg(1).to_number());
            flags = flag_preprocess(flags, &do_unique, &do_index);
        }
        as_value_prop avc(propField, get_basic_cmp(flags, version),
                *getGlobal(fn));
        if (do_unique)
        {
            as_value_prop ave(propField, get_basic_eq(flags, version), 
                    *getGlobal(fn));
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
            ensureType<Array_as>(fn.arg(0).to_object(*getGlobal(fn)));
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
                ensureType<Array_as>(fn.arg(1).to_object(*getGlobal(fn)));
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

        as_value_multiprop avc(prp, cmp, *getGlobal(fn));

        if (do_unique)
        {
            as_value_multiprop_eq ave(prp, eq, *getGlobal(fn));
            if (do_index) return array->sort_indexed(avc, ave);
            return array->sort(avc, ave);
        }
        if (do_index) return as_value(array->sort_indexed(avc));
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
as_value
array_push(const fn_call& fn)
{
    as_object* array = ensureType<as_object>(fn.this_ptr);
 
    if (!fn.nargs) return as_value();

    const size_t shift = fn.nargs;

    as_value length;
    if (!array->get_member(NSV::PROP_LENGTH, &length)) return as_value();
    
    const int size = length.to_int();
    if (size < 0) return as_value();

    for (size_t i = 0; i < fn.nargs; ++i) {
        array->set_member(getKey(fn, size + i), fn.arg(i));
    }
    
    // TODO: this is wrong, but Gnash relies on it.
    array->set_member(NSV::PROP_LENGTH, size + shift);

    return as_value(size + shift);
}

// Callback to push values to the front of an array
static as_value
array_unshift(const fn_call& fn)
{

    as_object* array = ensureType<as_object>(fn.this_ptr);
 
    if (!fn.nargs) return as_value();

    const size_t shift = fn.nargs;

    as_value length;
    if (!array->get_member(NSV::PROP_LENGTH, &length)) return as_value();
    
    const int size = length.to_int();
    if (size < 0) return as_value();

    string_table& st = getStringTable(fn);
    as_value ret = array->getMember(st.find("0"));
    
    for (size_t i = size + shift - 1; i >= shift ; --i) {
        const string_table::key nextkey = getKey(fn, i - shift);
        const string_table::key currentkey = getKey(fn, i);
        array->delProperty(currentkey);
        array->set_member(currentkey, array->getMember(nextkey));
    }

    for (size_t i = shift; i > 0; --i) {
        const size_t index = i - 1;
        array->set_member(getKey(fn, index), fn.arg(index));
    }
 
    // TODO: this is wrong, but Gnash relies on it.
    array->set_member(NSV::PROP_LENGTH, size + shift);

    return as_value(size + shift);
}

// Callback to pop a value from the back of an array
static as_value
array_pop(const fn_call& fn)
{

    as_object* array = ensureType<as_object>(fn.this_ptr);

    as_value length;
    if (!array->get_member(NSV::PROP_LENGTH, &length)) return as_value();
    
    const int size = length.to_int();
    if (size < 1) return as_value();

    const string_table::key ind = getKey(fn, size - 1);
    as_value ret = array->getMember(ind);
    array->delProperty(ind);
    
    // TODO: this is wrong, but Gnash relies on it.
    array->set_member(NSV::PROP_LENGTH, size - 1);

    return ret;
}

// Callback to pop a value from the front of an array
static as_value
array_shift(const fn_call& fn)
{
    as_object* array = ensureType<as_object>(fn.this_ptr);

    as_value length;
    if (!array->get_member(NSV::PROP_LENGTH, &length)) return as_value();
    
    const int size = length.to_int();

    // An array with no elements has nothing to return.
    if (size < 1) return as_value();

    as_value ret = array->getMember(getKey(fn, 0));

    for (size_t i = 0; i < static_cast<size_t>(size - 1); ++i) {
        const string_table::key nextkey = getKey(fn, i + 1);
        const string_table::key currentkey = getKey(fn, i);
        array->delProperty(currentkey);
        array->set_member(currentkey, array->getMember(nextkey));
    }
    
    // TODO: this is wrong, but Gnash relies on it.
    array->set_member(NSV::PROP_LENGTH, size - 1);

    return ret;
}

// Callback to reverse the position of the elements in an array
static as_value
array_reverse(const fn_call& fn)
{
    as_object* array = ensureType<as_object>(fn.this_ptr);

    as_value length;
    if (!array->get_member(NSV::PROP_LENGTH, &length)) return as_value();
    
    const int size = length.to_int();

    // An array with 0 or 1 elements has nothing to reverse.
    if (size < 2) return as_value();

    for (size_t i = 0; i < static_cast<size_t>(size) / 2; ++i) {
        const string_table::key bottomkey = getKey(fn, i);
        const string_table::key topkey = getKey(fn, size - i - 1);
        const as_value top = array->getMember(topkey);
        const as_value bottom = array->getMember(bottomkey);
        array->delProperty(topkey);
        array->delProperty(bottomkey);
        array->set_member(bottomkey, top);
        array->set_member(topkey, bottom);
    }

    return array;
}

as_value
join(as_object* array, const std::string& separator)
{
    as_value length;
    if (!array->get_member(NSV::PROP_LENGTH, &length)) return as_value("");

    const double size = length.to_int();
    if (size < 0) return as_value("");

    std::string s;

    string_table& st = getStringTable(*array);
    const int version = getSWFVersion(*array);

    for (size_t i = 0; i < size; ++i) {
        std::ostringstream os;
        os << i;
        if (i) s += separator;
        as_value el;
        array->get_member(st.find(os.str()), &el);
        s += el.to_string_versioned(version);
    }
    return as_value(s);
}

as_value
array_join(const fn_call& fn)
{
    as_object* array = ensureType<as_object>(fn.this_ptr);

    const int version = getSWFVersion(fn);
    const std::string separator =
        fn.nargs ? fn.arg(0).to_string_versioned(version) : ",";

    return join(array, separator);
}

// Callback to convert array to a string
as_value
array_toString(const fn_call& fn)
{
    as_object* array = ensureType<as_object>(fn.this_ptr);
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
    boost::intrusive_ptr<Array_as> array = ensureType<Array_as>(fn.this_ptr);

    // use copy ctor
    Array_as* newarray = new Array_as();

    PushToArray push(*newarray);
    if (!foreachArray(*array, push)) return as_value();

    for (size_t i = 0; i < fn.nargs; ++i) {

        // Array args get concatenated by elements
        // The type is checked using instanceOf.
        const as_value& arg = fn.arg(i);

        Global_as* gl = getGlobal(fn);
        as_object* other = arg.to_object(*gl);

        if (other) {
            
            // If it's not an array, we want to carry on and add it as an
            // object.
            if (other->instanceOf(getClassConstructor(fn, "Array"))) {
                // Do we care if it has no length property?
                foreachArray(*other, push);
                continue;
            }
        }
        newarray->callMethod(NSV::PROP_PUSH, fn.arg(i));
    }

    return as_value(newarray);        
}

// Callback to slice part of an array to a new array
// without changing the original
as_value
array_slice(const fn_call& fn)
{
    as_object* array = ensureType<as_object>(fn.this_ptr);

    if (fn.nargs > 2) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("More than 2 arguments to Array.slice, "
            "and I don't know what to do with them.  "
            "Ignoring them"));
        );
    }

    int startindex = fn.nargs ? fn.arg(0).to_int() : 0;

    // if we sent at least two arguments, setup endindex
    int endindex = fn.nargs > 1 ? fn.arg(1).to_int() :
        std::numeric_limits<int>::max();

    Global_as* gl = getGlobal(fn);
    as_object* newarray = gl->createArray();

    PushToArray push(*newarray);

    foreachArray(*array, startindex, endindex, push);

    return as_value(newarray);        
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
        ao->set_member(NSV::PROP_LENGTH, newSize);
    }
    else
    {
        // Use the arguments as initializers.
        as_value    index_number;
        for (unsigned int i = 0; i < fn.nargs; i++)
        {
            ao->callMethod(NSV::PROP_PUSH, fn.arg(i));
        }
    }

    IF_VERBOSE_ACTION (
    log_action(_("array_new setting object %p in result"), (void*)ao.get());
    );

    return as_value(ao.get());
    //return as_value(ao);
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

static as_object*
getArrayInterface()
{
    static boost::intrusive_ptr<as_object> proto = NULL;
    if ( proto == NULL )
    {
        proto = new as_object(getObjectInterface());
        getVM(*proto).addStatic(proto.get());

        attachArrayInterface(*proto);
    }
    return proto.get();
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

// this registers the "Array" member on a "Global"
// object. "Array" is a constructor, thus an object
// with .prototype full of exported functions + 
// 'constructor'
//
void
array_class_init(as_object& where, const ObjectURI& uri)
{
    static as_object* cl = 0;

    if (cl == NULL) {

        // This is going to be the global Array "class"/"function"
        VM& vm = getVM(where);

        as_object* proto = getArrayInterface();
        cl = vm.getNative(252, 0);
        cl->init_member(NSV::PROP_PROTOTYPE, proto);
        proto->init_member(NSV::PROP_CONSTRUCTOR, cl);

        // Attach static members
        attachArrayStatics(*cl);
    }

    const int flags = PropFlags::dontEnum; 
    where.init_member(getName(uri), cl, flags, getNamespace(uri));
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
    string_table& st = getStringTable(*this);
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
    string_table& st = getStringTable(*this);
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

