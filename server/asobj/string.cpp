// string.cpp:  ActionScript "String" class, for Gnash.
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

/* $Id: string.cpp,v 1.38 2007/10/06 07:08:52 strk Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h"
#include "smart_ptr.h"
#include "fn_call.h"
#include "as_object.h"
#include "builtin_function.h" // need builtin_function
#include "log.h"
#include "array.h"
#include "as_value.h"
#include "GnashException.h"
#include "VM.h" // for registering static GcResources (constructor and prototype)
#include "Object.h" // for getObjectInterface

#include <boost/algorithm/string/case_conv.hpp>

#define ENSURE_FN_ARGS(min, max, rv)                                    \
    if (fn.nargs < min) {                                               \
        IF_VERBOSE_ASCODING_ERRORS(                                     \
            log_aserror(_("%s needs one argument"), __FUNCTION__);         \
            )                                                           \
         return as_value(rv);                                           \
    }                                                                   \
    IF_VERBOSE_ASCODING_ERRORS(                                         \
        if (fn.nargs > max)                                             \
            log_aserror(_("%s has more than one argument"), __FUNCTION__); \
    )



namespace gnash
{

// Forward declarations

static as_value string_get_length(const fn_call& fn);
static as_value string_concat(const fn_call& fn);
static as_value string_slice(const fn_call& fn);
static as_value string_split(const fn_call& fn);
static as_value string_last_index_of(const fn_call& fn);
static as_value string_sub_str(const fn_call& fn);
static as_value string_sub_string(const fn_call& fn);
static as_value string_index_of(const fn_call& fn);
static as_value string_from_char_code(const fn_call& fn);
static as_value string_char_code_at(const fn_call& fn);
static as_value string_char_at(const fn_call& fn);
static as_value string_to_upper_case(const fn_call& fn);
static as_value string_to_lower_case(const fn_call& fn);
static as_value string_to_string(const fn_call& fn);
static as_value string_ctor(const fn_call& fn);

static void
attachStringInterface(as_object& o)
{
    // TODO fill in the rest
    o.init_member("concat", new builtin_function(string_concat));
    o.init_member("slice", new builtin_function(string_slice));
    o.init_member("split", new builtin_function(string_split));
    o.init_member("lastIndexOf", new builtin_function(string_last_index_of));
    o.init_member("substr", new builtin_function(string_sub_str));
    o.init_member("substring", new builtin_function(string_sub_string));
    o.init_member("indexOf", new builtin_function(string_index_of));
    o.init_member("toString", new builtin_function(string_to_string));
    o.init_member("fromCharCode", new builtin_function(string_from_char_code));
    o.init_member("charAt", new builtin_function(string_char_at));
    o.init_member("charCodeAt", new builtin_function(string_char_code_at));
    o.init_member("toUpperCase", new builtin_function(string_to_upper_case));
    o.init_member("toLowerCase", new builtin_function(string_to_lower_case));
    o.init_member("valueOf", new builtin_function(as_object::tostring_method));

    boost::intrusive_ptr<builtin_function> length_getter(new builtin_function(string_get_length));
    o.init_readonly_property("length", *length_getter);

}

static as_object*
getStringInterface()
{
    static boost::intrusive_ptr<as_object> o;

    if ( o == NULL )
    {
        o = new as_object(getObjectInterface());
	VM::get().addStatic(o.get());

        attachStringInterface(*o);
    }

    return o.get();
}

class string_as_object : public as_object
{

public:
    string_as_object()
            :
            as_object(getStringInterface())
    {}

    bool useCustomToString() const { return false; }

    std::string get_text_value() const
    {
        return _string;
    }

    as_value get_primitive_value() const

    {
        return as_value(_string);
    }

    std::string& str()

    {
        return _string;
    }

private:
    std::string _string;
};

static as_value
string_get_length(const fn_call& fn)
{
    boost::intrusive_ptr<string_as_object> obj = ensureType<string_as_object>(fn.this_ptr);

    return as_value(obj->str().size());
}

// all the arguments will be converted to string and concatenated
static as_value
string_concat(const fn_call& fn)
{
    boost::intrusive_ptr<string_as_object> obj = ensureType<string_as_object>(fn.this_ptr);

    // Make a copy of our string.
    std::string str = obj->str();

    for (unsigned int i = 0; i < fn.nargs; i++) {
        str += fn.arg(i).to_string(&(fn.env()));
    }

    return as_value(str);
}


static size_t
valid_index(std::string subject, int index)
{
    int myIndex = index;

    if (myIndex < 0) {
        myIndex = subject.size() + myIndex;
    }

    myIndex = iclamp(myIndex, 0, subject.size());

    return myIndex;
}

// 1st param: start_index, 2nd param: end_index
static as_value
string_slice(const fn_call& fn)
{
    boost::intrusive_ptr<string_as_object> obj = ensureType<string_as_object>(fn.this_ptr);

    // Make a copy.
    std::string str = obj->str();

    ENSURE_FN_ARGS(1, 2, str);

    int start = fn.arg(0).to_number<int>();

    int end = str.size();

    if (fn.nargs >= 2) {
        end = fn.arg(1).to_number<int>();

        if (end < start) {
            // Swap start and end like substring
            swap(&start, &end);
        }

        start = valid_index(str, start);

        end = valid_index(str, end) - start ;
    } else {
        start = valid_index(str, start);
    }

    return as_value(str.substr(start, end));
}

static as_value
string_split(const fn_call& fn)
{
    boost::intrusive_ptr<string_as_object> obj =
        ensureType<string_as_object>(fn.this_ptr);

    std::string str = obj->str();

    as_value val;

    boost::intrusive_ptr<as_array_object> array(new as_array_object());

    int SWFVersion = fn.env().get_version();

    if (fn.nargs == 0)
    {
        val.set_std_string(str);
        array->push(val);

        return as_value(array.get());
    }

    const std::string& delim = fn.arg(0).to_string(&(fn.env()));

    // SWF5 didn't support multichar or empty delimiter
    if ( SWFVersion < 6 )
    {
	    if ( delim.size() != 1 )
	    {
		    val.set_std_string(str);
		    array->push(val);
		    return as_value(array.get());
	    }
    }

    size_t max = str.size();

    if (fn.nargs >= 2)
    {
	int max_in = fn.arg(1).to_number<int>();
	if ( SWFVersion < 6 && max_in < 1 )
	{
		return as_value(array.get());
	}
        max = iclamp((size_t)max_in, 0, str.size());
    }

    if ( str.empty() )
    {
        val.set_std_string(str);
        array->push(val);

        return as_value(array.get());
    }


    //if (delim == "") {
    if ( delim.empty() ) {
        for (unsigned i=0; i <max; i++) {
            val.set_std_string(str.substr(i, i+1));
            array->push(val);
        }

        return as_value(array.get());
    }


    size_t pos = 0, prevpos = 0;
    size_t num = 0;

    while (num < max) {
        pos = str.find(delim, pos);

        if (pos != std::string::npos) {
            val.set_std_string(str.substr(prevpos, pos - prevpos));
            array->push(val);
            num++;
            prevpos = pos + delim.size();
            pos++;
        } else {
            val.set_std_string(str.substr(prevpos));
            array->push(val);
            break;
        }
    }

    return as_value(array.get());
}

static as_value
string_last_index_of(const fn_call& fn)
{
    boost::intrusive_ptr<string_as_object> obj = ensureType<string_as_object>(fn.this_ptr);

    const std::string& str = obj->str();

    ENSURE_FN_ARGS(1, 2, -1);

    const std::string& toFind = fn.arg(0).to_string(&(fn.env()));

    size_t start = str.size();

    if (fn.nargs >= 2) {
        start = fn.arg(1).to_number<size_t>();
    }

    size_t found = str.find_last_of(toFind, start);

    if (found == std::string::npos) {
        return as_value(-1);
    }

    return as_value(found-toFind.size()+1);
}

// 1st param: start_index, 2nd param: length (NOT end_index)
static as_value
string_sub_str(const fn_call& fn)
{
    boost::intrusive_ptr<string_as_object> obj = ensureType<string_as_object>(fn.this_ptr);

    // Make a copy.
    std::string str = obj->str();

    ENSURE_FN_ARGS(1, 2, str);

    int start = valid_index(str, fn.arg(0).to_number<int>());

    int num = str.size();

    if (fn.nargs >= 2) {
        num = fn.arg(1).to_number<int>();
    }

    return as_value(str.substr(start, num));
}

// 1st param: start_index, 2nd param: end_index
// end_index is 1-based.
static as_value
string_sub_string(const fn_call& fn)
{
    boost::intrusive_ptr<string_as_object> obj = ensureType<string_as_object>(fn.this_ptr);

    const std::string& str = obj->str();

    ENSURE_FN_ARGS(1, 2, str);

    int start = fn.arg(0).to_number<int>();

    if (start < 0) {
        start = 0;
    }

    if (static_cast<unsigned>(start) > str.size()) {
        return as_value("");
    }

    int end = str.size();

    if (fn.nargs >= 2) {
        int num = fn.arg(1).to_number<int>();

        if (num < 0) {
            return as_value("");
        }

        if (num > 1 && static_cast<unsigned>(num) < str.size()) {
            end = num;

            if (end < start) {
                IF_VERBOSE_ASCODING_ERRORS(
                    log_aserror(_("string.slice() called with end < start"));
                )
                swap(&end, &start);
            }

            end -= start;
        }

    }

    return as_value(str.substr(start, end));
}

static as_value
string_index_of(const fn_call& fn)
{
    boost::intrusive_ptr<string_as_object> obj = ensureType<string_as_object>(fn.this_ptr);

    const std::string& str = obj->str();

    ENSURE_FN_ARGS(1, 2, -1);

    as_value& tfarg = fn.arg(0); // to find arg
    const std::string& toFind = tfarg.to_string(&(fn.env()));

    size_t start = 0;

    if (fn.nargs >= 2)
    {
        as_value& saval = fn.arg(1); // start arg val
        int start_arg = saval.to_int(fn.env());
        if ( start_arg > 0 ) start = (size_t) start_arg;
	else
	{
		IF_VERBOSE_ASCODING_ERRORS(
		if ( start_arg < 0 )
		{
			log_aserror("String.indexOf(%s, %s): second argument casts to invalid offset (%d)",
				tfarg.to_debug_string().c_str(),
				saval.to_debug_string().c_str(), start_arg);
		}
		);
	}
    }

    size_t pos = str.find(toFind, start);

    if (pos == std::string::npos) {
        return as_value(-1);
    }

    return as_value(pos);
}

static as_value
string_from_char_code(const fn_call& fn)
{
    std::string result;

    // isn't this function supposed to take one argument?

    for (unsigned int i = 0; i < fn.nargs; i++) {
        uint32_t c = fn.arg(i).to_number<uint32_t>();
        result += c;
    }

    return as_value(result);
}

static as_value
string_char_code_at(const fn_call& fn)
{
    boost::intrusive_ptr<string_as_object> obj = ensureType<string_as_object>(fn.this_ptr);

    const std::string& str = obj->str();

    if (fn.nargs == 0) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("string.charCodeAt needs one argument"));
        )
        as_value rv;
        rv.set_nan();
        return rv;	// Same as for out-of-range arg
    }

    IF_VERBOSE_ASCODING_ERRORS(
        if (fn.nargs > 1) {
            log_aserror(_("string.charCodeAt has more than one argument"));
        }
    )

    size_t index = fn.arg(0).to_number<size_t>();

    if (index > str.size()) {
        as_value rv;
        rv.set_nan();
        return rv;
    }

    return as_value(str[index]);
}

static as_value
string_char_at(const fn_call& fn)
{
    boost::intrusive_ptr<string_as_object> obj = ensureType<string_as_object>(fn.this_ptr);
    std::string& str = obj->str();

    ENSURE_FN_ARGS(1, 1, "");

    size_t index = fn.arg(0).to_number<size_t>();

    if (index > str.size()) {
        as_value rv;
        rv.set_nan();
        return rv;
    }

    std::string rv;

    rv.push_back(str[index]);

    return as_value(rv);
}

static as_value
string_to_upper_case(const fn_call& fn)
{
    boost::intrusive_ptr<string_as_object> obj = ensureType<string_as_object>(fn.this_ptr);
    std::string subject = obj->str();

    VM& vm = VM::get();

    boost::to_upper(subject, vm.getLocale());

    return as_value(subject);
}

static as_value
string_to_lower_case(const fn_call& fn)
{
    boost::intrusive_ptr<string_as_object> obj = ensureType<string_as_object>(fn.this_ptr);
    std::string subject = obj->str();

    VM& vm = VM::get();

    boost::to_lower(subject, vm.getLocale());

    return as_value(subject);
}

static as_value
string_to_string(const fn_call& fn)
{
    boost::intrusive_ptr<string_as_object> obj = ensureType<string_as_object>(fn.this_ptr);
    return as_value(obj->str());
}


static as_value
string_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<string_as_object> obj = new string_as_object;

    std::string& str = obj->str();

    if (fn.nargs > 0) {
        str = fn.arg(0).to_string(&(fn.env()));
    }

    // this shouldn't be needed
    //attachStringInterface(*str);

    return as_value(obj.get());
}

static boost::intrusive_ptr<builtin_function>
getStringConstructor()
{
    // This is going to be the global String "class"/"function"

    static boost::intrusive_ptr<builtin_function> cl;

    if ( cl == NULL )
    {
        cl=new builtin_function(&string_ctor, getStringInterface());
	VM::get().addStatic(cl.get());

        // replicate all interface to class, to be able to access
        // all methods as static functions
        attachStringInterface(*cl);

    }

    return cl;
}

// extern (used by Global.cpp)
void string_class_init(as_object& global)
{
    // This is going to be the global String "class"/"function"
    boost::intrusive_ptr<builtin_function> cl = getStringConstructor();

    // Register _global.String
    global.init_member("String", cl.get());
}

boost::intrusive_ptr<as_object>
init_string_instance(const char* val)
{
    boost::intrusive_ptr<builtin_function> cl = getStringConstructor();
    as_environment env;
    env.push(val);
    return cl->constructInstance(env, 1, 0);
}

} // namespace gnash
