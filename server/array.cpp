// array.cpp:  ActionScript array class, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
#include "config.h"
#endif

#include "as_value.h"
#include "array.h"
#include "log.h"
#include "builtin_function.h" // for Array class
#include "as_function.h" // for sort user-defined comparator
#include "fn_call.h"
#include "GnashException.h"
#include "action.h" // for call_method
#include "VM.h" // for PROPNAME macro to work

#include <string>
#include <algorithm>
#include <memory> // for auto_ptr
#include <boost/algorithm/string/case_conv.hpp>

//#define GNASH_DEBUG 


namespace gnash {

typedef bool (* as_cmp_fn) (const as_value&, const as_value&);

static as_object* getArrayInterface();
static void attachArrayProperties(as_object& proto);
static void attachArrayInterface(as_object& proto);

inline static bool int_lt_or_eq (int a)
{
	return a <= 0;
}

inline static bool int_gt (int a)
{
	return a > 0;
}

// simple as_value strict-weak-ordering comparison functions:
inline static bool
as_value_lt (const as_value& a, const as_value& b)
{
	return a.to_string().compare(b.to_string()) < 0;
}

inline static bool
as_value_gt (const as_value& a, const as_value& b)
{
	return a.to_string().compare(b.to_string()) > 0;
}

inline static bool
as_value_eq (const as_value& a, const as_value& b)
{
	return a.to_string().compare(b.to_string()) == 0;
}

inline static int
as_value_StrNoCaseCmp (const as_value& a, const as_value& b)
{
	using namespace boost::algorithm;

	std::string strA = to_upper_copy(a.to_string());
	std::string strB = to_upper_copy(b.to_string());

	return strA.compare(strB);
}

inline static bool
as_value_nocase_lt (const as_value& a, const as_value& b)
{
	return as_value_StrNoCaseCmp(a, b) < 0;
}

inline static bool
as_value_nocase_gt (const as_value& a, const as_value& b)
{
	return as_value_StrNoCaseCmp(a, b) > 0;
}

inline static bool
as_value_nocase_eq (const as_value& a, const as_value& b)
{
	return as_value_StrNoCaseCmp(a, b) == 0;
}

inline static bool
as_value_numLT (const as_value& a, const as_value& b)
{
	if (a.is_undefined()) return false;
	if (b.is_undefined()) return true;
	if (a.is_null()) return false;
	if (b.is_null()) return true;
	double aval = a.to_number();
	double bval = b.to_number();
	if (isnan(aval)) return false;
	if (isnan(bval)) return true;
	return aval < bval;
}

inline static bool
as_value_numGT (const as_value& a, const as_value& b)
{
	if (b.is_undefined()) return false;
	if (a.is_undefined()) return true;
	if (b.is_null()) return false;
	if (a.is_null()) return true;
	double aval = a.to_number();
	double bval = b.to_number();
	if (isnan(bval)) return false;
	if (isnan(aval)) return true;
	return aval > bval;
}

inline static bool
as_value_numEQ (const as_value& a, const as_value& b)
{
	if (a.is_undefined() && b.is_undefined()) return true;
	if (a.is_null() && b.is_null()) return true;
	double aval = a.to_number();
	double bval = b.to_number();
	if (isnan(aval) && isnan(bval)) return true;
	return aval == bval;
}

inline static bool
as_value_num_lt (const as_value& a, const as_value& b)
{
	if (a.is_string() || b.is_string())
		return as_value_lt(a, b);
	return as_value_numLT(a, b);
}

inline static bool
as_value_num_gt (const as_value& a, const as_value& b)
{
	if (a.is_string() || b.is_string())
		return as_value_gt(a, b);
	return as_value_numGT(a, b);
}

inline static bool
as_value_num_eq (const as_value& a, const as_value& b)
{
	if (a.is_string() || b.is_string())
		return as_value_eq(a, b);
	return as_value_numEQ(a, b);
}

inline static bool
as_value_num_nocase_lt (const as_value& a, const as_value& b)
{
	if (a.is_string() || b.is_string())
		return as_value_nocase_lt(a, b);
	return as_value_numLT(a, b);
}

inline static bool
as_value_num_nocase_gt (const as_value& a, const as_value& b)
{
	if (a.is_string() || b.is_string())
		return as_value_nocase_gt(a, b);
	return as_value_numGT(a, b);
}

inline static bool
as_value_num_nocase_eq (const as_value& a, const as_value& b)
{
	if (a.is_string() || b.is_string())
		return as_value_nocase_eq(a, b);
	return as_value_numEQ(a, b);
}

// Return basic as_value comparison function for corresponding sort flag
// Note:
// fUniqueSort and fReturnIndexedArray must first be stripped from the flag
static as_cmp_fn
get_basic_cmp(uint8_t flags)
{
	switch ( flags )
	{
		case 0: 
			return &as_value_lt;

		case as_array_object::fDescending:
			return &as_value_gt;

		case as_array_object::fCaseInsensitive: 
			return &as_value_nocase_lt;

		case as_array_object::fCaseInsensitive | 
				as_array_object::fDescending:
			return &as_value_nocase_gt;

		case as_array_object::fNumeric: 
			return &as_value_num_lt;

		case as_array_object::fNumeric | as_array_object::fDescending:
			return &as_value_num_gt;

		case as_array_object::fCaseInsensitive | 
				as_array_object::fNumeric:
			return &as_value_num_nocase_lt;

		case as_array_object::fCaseInsensitive | 
				as_array_object::fNumeric |
				as_array_object::fDescending:
			return &as_value_num_nocase_gt;

		default:
			log_error(_("Unhandled sort flags: %d (0x%X)"), flags, flags);
			return &as_value_lt;
	}
}

// Return basic as_value equality function for corresponding sort flag
// Note:
// fUniqueSort and fReturnIndexedArray must first be stripped from the flag
static as_cmp_fn
get_basic_eq(uint8_t flags)
{
	flags &= ~(as_array_object::fDescending);
	switch ( flags )
	{
		case 0: // default string comparison
			return &as_value_eq;

		case as_array_object::fCaseInsensitive: 
			return &as_value_nocase_eq;

		case as_array_object::fNumeric: 
			return &as_value_num_eq;

		case as_array_object::fCaseInsensitive | 
				as_array_object::fNumeric:
			return &as_value_num_nocase_eq;

		default:
			return &as_value_eq;
	}
}

// Custom (ActionScript) comparator 
class as_value_custom
{
public:
	as_function& _comp;
	as_object* _object;
	bool (*_zeroCmp)(const int);

	as_value_custom(as_function& comparator, bool (*zc)(const int), 
		boost::intrusive_ptr<as_object> this_ptr)
		:
		_comp(comparator),
		_zeroCmp(zc)
	{
		_object = this_ptr.get();
	}

	bool operator() (const as_value& a, const as_value& b)
	{
		as_value cmp_method(&_comp);
		as_environment env;
		as_value ret(0);

		env.push(a);
		env.push(b);
		ret = call_method(cmp_method, &env, _object, 2, 1);
		return (*_zeroCmp)((int)ret.to_number());
	}
};

// Comparator for sorting on a single array property
class as_value_prop
{
public:
	as_cmp_fn _comp;
	const std::string& _prop;
	
	// Note: cmpfn must implement a strict weak ordering
	as_value_prop(const std::string& name, 
		as_cmp_fn cmpfn)
		:
		_comp(cmpfn),
		_prop(name)
	{
	}

	bool operator() (const as_value& a, const as_value& b)
	{
		as_value av, bv;
		boost::intrusive_ptr<as_object> ao = a.to_object();
		boost::intrusive_ptr<as_object> bo = b.to_object();
		
		ao->get_member(_prop, &av);
		bo->get_member(_prop, &bv);
		return (*_comp)(av, bv);
	}
};


// Comparator for sorting on multiple array properties
class as_value_multiprop
{
public:
	const as_cmp_fn* _cmps;
	std::deque<std::string>::const_iterator _pBegin, _pEnd, pit;

	// Note: all as_cmp_fns in *cmps must implement strict weak ordering
	as_value_multiprop(std::deque<std::string>::const_iterator pBegin,
		std::deque<std::string>::const_iterator pEnd,
		as_cmp_fn* cmps)
		:
		_pBegin(pBegin), _pEnd(pEnd)
	{
		_cmps = cmps;
	}

	bool operator() (const as_value& a, const as_value& b)
	{
		const as_cmp_fn* cmp = _cmps;
		
		for (pit = _pBegin; pit != _pEnd; ++pit, ++cmp)
		{
			as_value av, bv;
			boost::intrusive_ptr<as_object> ao = a.to_object();
			boost::intrusive_ptr<as_object> bo = b.to_object();
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
	as_value_multiprop_eq(std::deque<std::string>::const_iterator pBegin,
		std::deque<std::string>::const_iterator pEnd,
		as_cmp_fn* eq)
		: as_value_multiprop(pBegin, pEnd, eq)
	{
	}

	bool operator() (const as_value& a, const as_value& b)
	{
		const as_cmp_fn* cmp = _cmps;
		
		for (pit = _pBegin; pit != _pEnd; ++pit, ++cmp)
		{
			as_value av, bv;
			boost::intrusive_ptr<as_object> ao = a.to_object();
			boost::intrusive_ptr<as_object> bo = b.to_object();
			ao->get_member(*pit, &av);
			bo->get_member(*pit, &bv);

			if ( !(*cmp)(av, bv) ) return false;
		}
		
		return true;
	}
};

// Convenience function to strip fUniqueSort and fReturnIndexedArray from sort
// flag. Presence of flags recorded in douniq and doindex.
static inline uint8_t
flag_preprocess(uint8_t flgs, bool* douniq, bool* doindex)
{
	*douniq = (flgs & as_array_object::fUniqueSort);
	*doindex = (flgs & as_array_object::fReturnIndexedArray);
	flgs &= ~(as_array_object::fReturnIndexedArray);
	flgs &= ~(as_array_object::fUniqueSort);
	return flgs;
}

// Convenience function to process and extract flags from an as_value array
// of flags (as passed to sortOn when sorting on multiple properties)
static void
get_multi_flags(std::deque<as_value>::const_iterator itBegin, 
	std::deque<as_value>::const_iterator itEnd, 
	uint8_t* flgs, bool* uniq, bool* index)
{
	std::deque<as_value>::const_iterator it = itBegin;
	int i = 0;

	// extract fUniqueSort and fReturnIndexedArray from first flag
	if (it != itEnd)
	{
		uint8_t flag = static_cast<uint8_t>((*it).to_number());
		flag = flag_preprocess(flag, uniq, index);
		flgs[i++] = flag;
		++it;
	}

	while (it != itEnd)
	{
		uint8_t flag = static_cast<uint8_t>((*it).to_number());
		flag &= ~(as_array_object::fReturnIndexedArray);
		flag &= ~(as_array_object::fUniqueSort);
		flgs[i++] = flag;
		++it;
	}
}

as_array_object::as_array_object()
	:
	as_object(getArrayInterface()), // pass Array inheritance
	elements(0)
{
	//IF_VERBOSE_ACTION (
	//log_action("%s: %p", __FUNCTION__, (void*)this);
	//)
	attachArrayProperties(*this);
}

as_array_object::as_array_object(const as_array_object& other)
	:
	as_object(other),
	elements(other.elements)
{
    //IF_VERBOSE_ACTION (
    //log_action("%s: %p", __FUNCTION__, (void*)this);
    //)
}

as_array_object::~as_array_object() 
{
}

std::deque<indexed_as_value>
as_array_object::get_indexed_elements()
{
	std::deque<indexed_as_value> indexed_elements;
	int i = 0;

	for (std::deque<as_value>::const_iterator it = elements.begin();
		it != elements.end(); ++it)
	{
		indexed_elements.push_back(indexed_as_value(*it, i++));
	}
	return indexed_elements;
}

std::deque<as_value>::const_iterator
as_array_object::begin()
{
	return elements.begin();
}

std::deque<as_value>::const_iterator
as_array_object::end()
{
	return elements.end();
}

int
as_array_object::index_requested(const std::string& name)
{
	as_value temp;
	temp.set_string(name);
	double value = temp.to_number();

	// if we were sent a string that can't convert like "asdf", it returns as NaN. -1 means invalid index
	if (isnan(value)) return -1;

	// TODO / WARNING: because to_number returns a double and we're
	// converting to an int,
	// I want to make sure we're above any "grey area" when we we round down
	// by adding a little to the number before we round it.
	// We don't want to accidentally look to index-1!
	return int(value + 0.01);
}

void
as_array_object::push(const as_value& val)
{
	elements.push_back(val);
}

void
as_array_object::unshift(const as_value& val)
{
	elements.push_front(val);
}

as_value
as_array_object::pop()
{
	// If the array is empty, report an error and return undefined!
	if ( elements.empty() ) 
	{
	    log_error(_("tried to pop element from back of empty array, returning undef"));
		return as_value(); // undefined
	}

	as_value ret = elements.back();
	elements.pop_back();

	return ret;
}

as_value
as_array_object::shift()
{
	// If the array is empty, report an error and return undefined!
	if ( elements.empty() ) 
	{
		log_error(_("tried to shift element from front of empty array, returning undef"));
		return as_value(); // undefined
	}

	as_value ret = elements.front();
	elements.pop_front();

	return ret;
}

void
as_array_object::reverse()
{
	// Reverse the deque elements
	std::reverse(elements.begin(), elements.end());
}

std::string
as_array_object::join(const std::string& separator, as_environment* env) const
{
	// TODO - confirm this is the right format!
	// Reportedly, flash version 7 on linux, and Flash 8 on IE look like
	// "(1,2,3)" and "1,2,3" respectively - which should we mimic?
	// Using no parentheses until confirmed for sure
	//
	// We should change output based on SWF version --strk 2006-04-28

	std::string temp;
	//std::string temp = "("; // SWF > 7

	if ( ! elements.empty() ) 
	{
		std::deque<as_value>::const_iterator
			it=elements.begin(),
			itEnd=elements.end();

		// print first element w/out separator prefix
		temp += (*it++).to_string(env);

		// print subsequent elements with separator prefix
		while ( it != itEnd )
		{
			temp += separator + (*it++).to_string(env);
		}
	}

	// temp += ")"; // SWF > 7

	return temp;

}

void
as_array_object::concat(const as_array_object& other)
{
	elements.insert(elements.end(), other.elements.begin(),
		other.elements.end());
}

std::string
as_array_object::toString(as_environment* env) const
{
	return join(",", env);
}

unsigned int
as_array_object::size() const
{
	return elements.size();
}

as_value
as_array_object::at(unsigned int index)
{
	if ( index > elements.size()-1 )
	{
		return as_value();
	}
	else
	{
		return elements[index];
	}
}

std::auto_ptr<as_array_object>
as_array_object::slice(unsigned int start, unsigned int one_past_end)
{
	assert(one_past_end >= start);
	assert(one_past_end <= size());
	assert(start <= size());

	std::auto_ptr<as_array_object> newarray(new as_array_object);

#ifdef GNASH_DEBUG
	log_msg(_("Array.slice(%u, %u) called"), start, one_past_end);
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

std::auto_ptr<as_array_object>
as_array_object::splice(unsigned start, unsigned len,
		const std::vector<as_value>& replace)
{
	assert(len <= size()-start);
	assert(start <= size());

#ifdef GNASH_DEBUG
	std::stringstream ss;
	ss << "Array.splice(" << start << ", " << len << ", ";
	std::ostream_iterator<as_value> ostrIter(ss, "," ) ;
	std::copy(replace.begin(), replace.end(), ostrIter);
        ss << ") called";
	log_msg("%s", ss.str().c_str());
	log_msg(_("Current array is %s"), toString().c_str());
#endif

	container::iterator itStart = elements.begin()+start;
	container::iterator itEnd = itStart+len;

	// This will be returned...
	std::auto_ptr<as_array_object> ret(new as_array_object);
	
	// If something has to be removed do it and assign
	// to the returned object
	if ( itStart != itEnd )
	{
		ret->elements.assign(itStart, itEnd);

		elements.erase(itStart, itEnd);
	}

	// Now insert the new stuff, if needed
	if ( ! replace.empty() )
	{
		container::iterator itStart = elements.begin()+start;
		elements.insert(itStart, replace.begin(), replace.end());
	}

	return ret;
}

/* virtual public, overriding as_object::get_member */
bool
as_array_object::get_member(const std::string& name, as_value *val)
{
	// an index has been requested
	int index = index_requested(name);
	if ( index >= 0 && (unsigned int)index < elements.size() )
	{
		*val = elements[index];
		return true;
	}

	return get_member_default(name, val);
}

void
as_array_object::resize(unsigned int newsize)
{
	elements.resize(newsize);
}

/* virtual public, overriding as_object::set_member */
void
as_array_object::set_member(const std::string& name,
		const as_value& val )
{
	int index = index_requested(name);

	// if we were sent a valid array index and not a normal member
	if (index >= 0)
	{
		if (index >= int(elements.size()))
		{
			// if we're setting index (x), the vector
			// must be size (x+1)
			elements.resize(index+1);
		}

		// set the appropriate index and return
		elements[index] = val;
		return;
	}

	as_object::set_member_default(name,val);
}

as_array_object*
as_array_object::get_indices(std::deque<indexed_as_value> elems)
{
	as_array_object* intIndexes = new as_array_object();

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
	boost::intrusive_ptr<as_array_object> array = ensureType<as_array_object>(fn.this_ptr);

#ifdef GNASH_DEBUG
	std::stringstream ss;
	fn.dump_args(ss);
	log_msg(_("Array(%s).splice(%s) called"), array->toString().c_str(), ss.str().c_str());
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
	int start = fn.arg(0).to_number<int>(&(fn.env()));
	if ( start < 0 ) start = array->size()+start; // start is negative, so + means -abs()
	startoffset = iclamp(start, 0, origlen);
#ifdef GNASH_DEBUG
	if ( startoffset != start )
		log_msg(_("Array.splice: start:%d became %u"), start, startoffset);
#endif

	//----------------
	// Get length
	//----------------
	unsigned len = origlen - start;
	if (fn.nargs > 1)
	{
		int lenval = fn.arg(1).to_number<int>(&(fn.env()));
		if ( lenval < 0 )
		{
			IF_VERBOSE_ASCODING_ERRORS(
			log_aserror(_("Array.splice(%d,%d): negative length given, call ignored"),
				start, lenval);
			);
			return as_value();
		}
		len = iclamp(lenval, 0, origlen-startoffset);
	}

	//----------------
	// Get replacement
	//----------------
	std::vector<as_value> replace;
	for (unsigned i=2; i<fn.nargs; ++i)
	{
		replace.push_back(fn.arg(i));
	}

	std::auto_ptr<as_array_object> spliced ( array->splice(startoffset, len, replace) );

	boost::intrusive_ptr<as_object> ret = spliced.release();

	return as_value(ret);
}

static as_value
array_sort(const fn_call& fn)
{
	boost::intrusive_ptr<as_array_object> array = 
		ensureType<as_array_object>(fn.this_ptr);
	uint8_t flags = 0;

	if ( fn.nargs == 0 )
	{
		array->sort(&as_value_lt);
		return as_value((boost::intrusive_ptr<as_object>)array);
	}
	else if ( fn.nargs == 1 && fn.arg(0).is_number() )
	{
		flags=static_cast<uint8_t>(fn.arg(0).to_number());
	}
	else if ( fn.arg(0).is_as_function() )
	{
		// Get comparison function
		as_function* as_func = fn.arg(0).to_as_function();
		bool (*icmp)(int);
	
		if ( fn.nargs == 2 && fn.arg(1).is_number() )
			flags=static_cast<uint8_t>(fn.arg(1).to_number());
		
		if (flags & as_array_object::fDescending) icmp = &int_lt_or_eq;
		else icmp = &int_gt;

		as_value_custom avc = 
			as_value_custom(*as_func, icmp, fn.this_ptr);

		if ( (flags & as_array_object::fReturnIndexedArray) )
			return as_value(array->sort_indexed(&avc));
		array->sort(&avc);
		return as_value((boost::intrusive_ptr<as_object>)array);
		// note: custom AS function sorting apparently ignores the 
		// UniqueSort flag which is why it is also ignored here
	}
	else
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Sort called with invalid arguments."));
		)
		return as_value();
	}
	bool do_unique, do_index;
	flags = flag_preprocess(flags, &do_unique, &do_index);
	as_cmp_fn comp = get_basic_cmp(flags);

	if (do_unique)
	{
		as_cmp_fn eq =
			get_basic_eq(flags);
		if (do_index) return array->sort_indexed(comp, eq);
		return array->sort(comp, eq);
	}
	if (do_index) return as_value(array->sort_indexed(comp));
	array->sort(comp);
	return as_value((boost::intrusive_ptr<as_object>)array);
}

static as_value
array_sortOn(const fn_call& fn)
{
	boost::intrusive_ptr<as_array_object> array = 
		ensureType<as_array_object>(fn.this_ptr);

	// case: sortOn("prop")
	if ( fn.nargs == 1 && fn.arg(0).is_string() )
	{
		std::string propField = PROPNAME(fn.arg(0).to_string());
		as_value_prop avc = as_value_prop(propField, &as_value_lt);
		array->sort(&avc);
		return as_value((boost::intrusive_ptr<as_object>)array);
	}

	// case: sortOn("prop", Array.FLAG)
	bool do_unique = false, do_index = false;
	if ( fn.nargs == 2 && fn.arg(0).is_string() )
	{
		std::string propField = PROPNAME(fn.arg(0).to_string());
		if ( fn.arg(1).is_number() )
		{
			uint8_t flags = 
				static_cast<uint8_t>(fn.arg(1).to_number());
			flags = flag_preprocess(flags, &do_unique, &do_index);
			as_value_prop avc = as_value_prop(propField, 
						get_basic_cmp(flags));
			if (do_unique)
			{
				as_value_prop ave = as_value_prop(propField, 
					get_basic_eq(flags));
				if (do_index)
					return array->sort_indexed(&avc, &ave);
				return array->sort(&avc, &ave);
			}
			if (do_index)
				return as_value(array->sort_indexed(&avc));
			array->sort(&avc);
			return as_value((boost::intrusive_ptr<as_object>)array);
		}
	}

	// case: sortOn(["prop1", "prop2"] ...)
	if (fn.nargs > 0 && fn.arg(0).is_object() ) 
	{
		boost::intrusive_ptr<as_array_object> props = 
			ensureType<as_array_object>(fn.arg(0).to_object());
		std::deque<std::string> prp;
		unsigned int optnum = props->size();
		as_cmp_fn cmp[optnum];
		as_cmp_fn eq[optnum];

		for (std::deque<as_value>::const_iterator it = props->begin();
			it != props->end(); ++it)
		{
			std::string s = PROPNAME((*it).to_string());
			prp.push_back(s);
		}
		
		// case: sortOn(["prop1", "prop2"])
		if (fn.nargs == 1)
		{	// assign each cmp function to the standard cmp fn
			for (unsigned int i = 0; i < optnum; i++)
				cmp[i] = &as_value_lt;
		}
		// case: sortOn(["prop1", "prop2"], [Array.FLAG1, Array.FLAG2])
		else if ( fn.arg(1).is_object() )
		{
			boost::intrusive_ptr<as_array_object> farray = 
				ensureType<as_array_object>(fn.arg(1).to_object());
			if (farray->size() == optnum)
			{
				uint8_t flgs[optnum];
				std::deque<as_value>::const_iterator 
					fBegin = farray->begin(),
					fEnd = farray->end();

				get_multi_flags(fBegin, fEnd, flgs, 
					&do_unique, &do_index);

				for (unsigned int i = 0; i < optnum; i++)
					cmp[i] = get_basic_cmp(flgs[i]);

				if (do_unique)
				{
					for (unsigned int i = 0; i < optnum; i++)
						eq[i] = get_basic_eq(flgs[i]);
				}
			}
			else
			{
				for (unsigned int i = 0; i < optnum; i++)
					cmp[i] = &as_value_lt;
			}
		}
		// case: sortOn(["prop1", "prop2"], Array.FLAG)
		else if ( fn.arg(1).is_number() )
		{
			uint8_t flags = 
				static_cast<uint8_t>(fn.arg(1).to_number());
			flag_preprocess(flags, &do_unique, &do_index);
			as_cmp_fn c = get_basic_cmp(flags);

			for (unsigned int i = 0; i < optnum; i++)
				cmp[i] = c;
			
			if (do_unique)
			{
				as_cmp_fn e = get_basic_eq(flags);
				for (unsigned int i = 0; i < optnum; i++)
					eq[i] = e;
			}
		}

		as_value_multiprop avc = 
			as_value_multiprop(prp.begin(), prp.end(), cmp);

		if (do_unique)
		{
			as_value_multiprop_eq ave = 
				as_value_multiprop_eq(prp.begin(), prp.end(), eq);
			if (do_index)
				return array->sort_indexed(&avc, &ave);
			return array->sort(&avc, &ave);
		}
		if (do_index)
			return as_value(array->sort_indexed(&avc));
		array->sort(&avc);
		return as_value((boost::intrusive_ptr<as_object>)array);

	}
	IF_VERBOSE_ASCODING_ERRORS(
	log_aserror(_("SortOn called with invalid arguments."));
	)
	return as_value();
}

// Callback to push values to the back of an array
static as_value
array_push(const fn_call& fn)
{
	boost::intrusive_ptr<as_array_object> array = ensureType<as_array_object>(fn.this_ptr);

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
	boost::intrusive_ptr<as_array_object> array = ensureType<as_array_object>(fn.this_ptr);

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
	boost::intrusive_ptr<as_array_object> array = ensureType<as_array_object>(fn.this_ptr);

	// Get our index, log, then return result
	as_value rv = array->pop();

	IF_VERBOSE_ACTION (
	log_action(_("calling array pop, result:%s, new array size:%d"),
		rv.to_string().c_str(), array->size());
	);
        return rv;
}

// Callback to pop a value from the front of an array
static as_value
array_shift(const fn_call& fn)
{
	boost::intrusive_ptr<as_array_object> array = ensureType<as_array_object>(fn.this_ptr);

	// Get our index, log, then return result
	as_value rv = array->shift();

	IF_VERBOSE_ACTION (
	log_action(_("calling array shift, result:%s, new array size:%d"),
		rv.to_string().c_str(), array->size());
	);
	return rv;
}

// Callback to reverse the position of the elements in an array
static as_value
array_reverse(const fn_call& fn)
{
	boost::intrusive_ptr<as_array_object> array = ensureType<as_array_object>(fn.this_ptr);

	array->reverse();

	as_value rv(array.get()); 

	IF_VERBOSE_ACTION (
	log_action(_("called array reverse, result:%s, new array size:%d"),
		rv.to_string().c_str(), array->size());
	);
	return rv;
}

// Callback to convert array to a string with optional custom separator (default ',')
static as_value
array_join(const fn_call& fn)
{
	boost::intrusive_ptr<as_array_object> array = ensureType<as_array_object>(fn.this_ptr);

	std::string separator = ",";

	if (fn.nargs > 0)
		separator = fn.arg(0).to_string();

	std::string ret = array->join(separator, &(fn.env()));

	return as_value(ret.c_str());
}

static as_value
array_size(const fn_call& fn)
{
	boost::intrusive_ptr<as_array_object> array = ensureType<as_array_object>(fn.this_ptr);

	return as_value(array->size());
}

// Callback to convert array to a string
// TODO CHECKME: rely on Object.toString  ? (
static as_value
array_to_string(const fn_call& fn)
{
	boost::intrusive_ptr<as_array_object> array = ensureType<as_array_object>(fn.this_ptr);

	std::string ret = array->toString(&(fn.env()));

		IF_VERBOSE_ACTION
		(
	log_action(_("array_to_string called, nargs = %d, "
			"this_ptr = %p"),
			fn.nargs, (void*)fn.this_ptr.get());
	log_action(_("to_string result is: %s"), ret.c_str());
		);

	return as_value(ret.c_str());
}

/// concatenates the elements specified in the parameters with
/// the elements in my_array, and creates a new array. If the
/// value parameters specify an array, the elements of that
/// array are concatenated, rather than the array itself. The
/// array my_array is left unchanged.
static as_value
array_concat(const fn_call& fn)
{
	boost::intrusive_ptr<as_array_object> array = ensureType<as_array_object>(fn.this_ptr);

	// use copy ctor
	as_array_object* newarray = new as_array_object(*array);

	for (unsigned int i=0; i<fn.nargs; i++)
	{
		// Array args get concatenated by elements
		boost::intrusive_ptr<as_array_object> other = boost::dynamic_pointer_cast<as_array_object>(fn.arg(i).to_object());
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
	boost::intrusive_ptr<as_array_object> array = ensureType<as_array_object>(fn.this_ptr);

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
		as_array_object* newarray = new as_array_object(*array);
		return as_value(newarray);
	}


	startindex = int(fn.arg(0).to_number());

	// if the index is negative, it means "places from the end"
	// where -1 is the last element
	if (startindex < 0) startindex = startindex + arraysize;

	// if we sent at least two arguments, setup endindex
	if (fn.nargs >= 2)
	{
		endindex = int(fn.arg(1).to_number());

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

	std::auto_ptr<as_array_object> newarray(array->slice(
		startindex, endindex));

	return as_value(newarray.release());		
}

static as_value
array_length(const fn_call& fn)
{
	boost::intrusive_ptr<as_array_object> array = ensureType<as_array_object>(fn.this_ptr);

	if ( fn.nargs ) // setter
	{
		array->resize(unsigned(fn.arg(0).to_number(&(fn.env()))));
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

	boost::intrusive_ptr<as_array_object>	ao = new as_array_object;

	if (fn.nargs == 0)
	{
		// Empty array.
	}
	else if (fn.nargs == 1 && fn.arg(0).is_number() )
	{
		// Create an empty array with the given number of undefined elements.
		//
		as_value index_number, undef_value;

		undef_value.set_undefined();
		for (int i = 0; i < int(fn.arg(0).to_number()); i++)
		{
			index_number.set_int(i);
			ao->set_member(index_number.to_string().c_str(), undef_value);
		}
	}
	else
	{
		// Use the arguments as initializers.
		as_value	index_number;
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
	boost::intrusive_ptr<builtin_function> gettersetter;

	gettersetter = new builtin_function(&array_length, NULL);
	proto.init_property("length", *gettersetter, *gettersetter);
}

static void
attachArrayInterface(as_object& proto)
{

	proto.init_member("join", new builtin_function(array_join));
	proto.init_member("concat", new builtin_function(array_concat));
	proto.init_member("slice", new builtin_function(array_slice));
	proto.init_member("push", new builtin_function(array_push));
	proto.init_member("unshift", new builtin_function(array_unshift));
	proto.init_member("pop", new builtin_function(array_pop));
	proto.init_member("shift", new builtin_function(array_shift));
	proto.init_member("splice", new builtin_function(array_splice));
	proto.init_member("sort", new builtin_function(array_sort));
	proto.init_member("size", new builtin_function(array_size));
	proto.init_member("sortOn", new builtin_function(array_sortOn));
	proto.init_member("reverse", new builtin_function(array_reverse));
	proto.init_member("toString", new builtin_function(array_to_string));

	proto.init_member("CASEINSENSITIVE", as_array_object::fCaseInsensitive);
	proto.init_member("DESCENDING", as_array_object::fDescending);
	proto.init_member("UNIQUESORT", as_array_object::fUniqueSort);
	proto.init_member("RETURNINDEXEDARRAY", as_array_object::fReturnIndexedArray);
	proto.init_member("NUMERIC", as_array_object::fNumeric);
}

static as_object*
getArrayInterface()
{
	static boost::intrusive_ptr<as_object> proto = NULL;
	if ( proto == NULL )
	{
		proto = new as_object();
		attachArrayInterface(*proto);
	}
	return proto.get();
}

// this registers the "Array" member on a "Global"
// object. "Array" is a constructor, thus an object
// with .prototype full of exported functions + 
// 'constructor'
//
void
array_class_init(as_object& glob)
{
	// This is going to be the global Array "class"/"function"
	static boost::intrusive_ptr<as_function> ar=NULL;

	if ( ar == NULL )
	{
		ar = new builtin_function(&array_new, getArrayInterface());

		// We replicate interface to the Array class itself
		attachArrayInterface(*ar);
	}

	// Register _global.Array
	glob.init_member("Array", ar.get());
}

#ifdef GNASH_USE_GC
void
as_array_object::markReachableResources() const
{
	for (container::const_iterator i=elements.begin(), e=elements.end(); i!=e; ++i)
	{
		i->setReachable();
	}
	markAsObjectReachable();
}
#endif // GNASH_USE_GC

} // end of gnash namespace


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

