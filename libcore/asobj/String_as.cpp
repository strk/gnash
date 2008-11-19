// string.cpp:  ActionScript "String" class, for Gnash.
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


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

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
#include "namedStrings.h"
#include "utf8.h"
#include "String_as.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <algorithm>
#include <locale>
#include <stdexcept>

namespace gnash {

/// Check the number of arguments, returning false if there
/// aren't enough, or true if there are either enough or too many.
/// Logs an error if the number isn't between min and max.
inline bool checkArgs(const fn_call& fn, size_t min, size_t max,
        const std::string& function)
{

    if (fn.nargs < min) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream os;
            fn.dump_args(os);
                log_aserror(_("%1%(%2%) needs %3% argument(s)"),
                    function, os.str(), min);
            )
         return false;
    }                          
    IF_VERBOSE_ASCODING_ERRORS(
        if (fn.nargs > max)
        {
            std::ostringstream os;
            fn.dump_args(os);
            log_aserror(_("%1%(%2%) has more than %3% argument(s)"),
                function, os.str(), max);
        }
    );
    return true;
}

// Forward declarations

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
static as_value string_oldToLower(const fn_call& fn);
static as_value string_oldToUpper(const fn_call& fn);
static as_value string_ctor(const fn_call& fn);

static void
attachStringInterface(as_object& o)
{
	VM& vm = o.getVM();

	// TODO fill in the rest

	// ASnative(251, 1) - [String.prototype] valueOf
	vm.registerNative(as_object::tostring_method, 251, 1);
	o.init_member("valueOf", vm.getNative(251, 1));

	// ASnative(251, 2) - [String.prototype] toString
	vm.registerNative(string_to_string, 251, 2);
	o.init_member("toString", vm.getNative(251, 2));

	// ASnative(251, 3) - [String.prototype] toUpperCase
	// ASnative(102, 0) - SWF5 to upper.
	vm.registerNative(string_oldToUpper, 102, 0);
	vm.registerNative(string_to_upper_case, 251, 3);
	o.init_member("toUpperCase", vm.getNative(251, 3));

	// ASnative(251, 4) - [String.prototype] toLowerCase
	// ASnative(102, 1) - SWF5 to lower.
	vm.registerNative(string_oldToLower, 102, 1);
	vm.registerNative(string_to_lower_case, 251, 4);
	o.init_member("toLowerCase", vm.getNative(251, 4));

	// ASnative(251, 5) - [String.prototype] charAt
	vm.registerNative(string_char_at, 251, 5);
	o.init_member("charAt", vm.getNative(251, 5));

	// ASnative(251, 6) - [String.prototype] charCodeAt
	vm.registerNative(string_char_code_at, 251, 6);
	o.init_member("charCodeAt", vm.getNative(251, 6));

	// ASnative(251, 7) - [String.prototype] concat
	vm.registerNative(string_concat, 251, 7);
	o.init_member("concat", vm.getNative(251, 7));

	// ASnative(251, 8) - [String.prototype] indexOf
	vm.registerNative(string_index_of, 251, 8);
	o.init_member("indexOf", vm.getNative(251, 8));

	// ASnative(251, 9) - [String.prototype] lastIndexOf
	vm.registerNative(string_last_index_of, 251, 9);
	o.init_member("lastIndexOf", vm.getNative(251, 9));

	// ASnative(251, 10) - [String.prototype] slice
	vm.registerNative(string_slice, 251, 10);
	o.init_member("slice", vm.getNative(251, 10));

	// ASnative(251, 11) - [String.prototype] substring
	vm.registerNative(string_sub_string, 251, 11);
	o.init_member("substring", vm.getNative(251, 11));

	// ASnative(251, 12) - [String.prototype] split
	vm.registerNative(string_split, 251, 12);
	o.init_member("split", vm.getNative(251, 12));

	// ASnative(251, 13) - [String.prototype] substr
	vm.registerNative(string_sub_str, 251, 13);
	o.init_member("substr", vm.getNative(251, 13));
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

class String_as : public as_object
{

public:
    String_as(const std::string& s)
            :
            as_object(getStringInterface()),
            _string(s)
    {
	std::wstring wstr = utf8::decodeCanonicalString(_string, _vm.getSWFVersion());
	init_member(NSV::PROP_LENGTH, wstr.size(), as_prop_flags::dontDelete|as_prop_flags::dontEnum); // can override though
    }


    bool useCustomToString() const { return false; }

    std::string get_text_value() const
    {
        return _string;
    }

    as_value get_primitive_value() const

    {
        return as_value(_string);
    }

    const std::string& str()
    {
        return _string;
    }

private:
    std::string _string;
};

// all the arguments will be converted to string and concatenated.
// This can only be applied to String objects, unlike other methods.
static as_value
string_concat(const fn_call& fn)
{
    boost::intrusive_ptr<String_as> obj = ensureType<String_as>(fn.this_ptr);

    // Make a copy of our string.
    std::string str = obj->str();

    for (unsigned int i = 0; i < fn.nargs; i++) {
        str += fn.arg(i).to_string();
    }

    return as_value(str);
}


static size_t
validIndex(const std::wstring& subject, int index)
{

    if (index < 0) {
        index = subject.size() + index;
    }

    index = utility::clamp<int>(index, 0, subject.size());

    return index;
}

// 1st param: start_index, 2nd param: end_index
static as_value
string_slice(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = ensureType<as_object>(fn.this_ptr);
    as_value val(fn.this_ptr);
    
    const std::string& str = val.to_string();

    /// version to use is the one of the SWF containing caller code.
    /// If callerDef is null, this calls is spontaneous (system-event?)
    /// in which case we should research on which version should drive behaviour.
    /// NOTE: it is unlikely that a system event triggers string_split so
    ///       in most cases a null callerDef means the caller forgot to 
    ///       set the field (ie: a programmatic error)
    if ( ! fn.callerDef )
    {
        log_error("No fn_call::callerDef in string_slice call");
        //abort();
    }
    const int version = fn.callerDef ? fn.callerDef->get_version() : obj->getVM().getSWFVersion();

    std::wstring wstr = utf8::decodeCanonicalString(str, version);

    if (!checkArgs(fn, 1, 2, "String.slice()")) return as_value();

    size_t start = validIndex(wstr, fn.arg(0).to_int());

    size_t end = wstr.length();

    if (fn.nargs >= 2)
    {
    	end = validIndex(wstr, fn.arg(1).to_int());

    } 

    if (end < start) // move out of if ?
    {
            return as_value("");
    }

    size_t retlen = end - start;

    log_debug("start: %d, end: %d, retlen: %d", start, end, retlen);

    return as_value(utf8::encodeCanonicalString(wstr.substr(start, retlen), version));
}

// String.split(delimiter[, limit])
// For SWF5, the following conditions mean that an array with a single
// element containing the entire string is returned:
// 1. No arguments are passed.
// 2. The delimiter is empty.
// 3. The delimiter has more than one character or is undefined and limit is not 0.
// 4. The delimiter is not present in the string and the limit is not 0.
//
// Accordingly, an empty array is returned only when the limit is less
// than 0 and a non-empty delimiter is passed.
//
// For SWF6:
// Full string returned in 1-element array:
// 1. If no arguments are passed.
// 2. If delimiter undefined.
// 3: empty string, non-empty delimiter.
//
// Empty array returned:
// 4. string and delimiter are empty but defined.
// 5. non-empty string, non-empty delimiter; 0 or less elements required.
static as_value
string_split(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = ensureType<as_object>(fn.this_ptr);
    as_value val(fn.this_ptr);
    
    const std::string& str = val.to_string();

    /// version to use is the one of the SWF containing caller code.
    /// If callerDef is null, this calls is spontaneous (system-event?)
    /// in which case we should research on which version should drive behaviour.
    /// NOTE: it is unlikely that a system event triggers string_split so
    ///       in most cases a null callerDef means the caller forgot to 
    ///       set the field (ie: a programmatic error)
    if ( ! fn.callerDef ) log_error("No fn_call::callerDef in string_split call");
    const int version = fn.callerDef ? fn.callerDef->get_version() : obj->getVM().getSWFVersion();
    
    std::wstring wstr = utf8::decodeCanonicalString(str, version);

    boost::intrusive_ptr<Array_as> array(new Array_as());

    if (fn.nargs == 0)
    {
        // Condition 1:
        array->push(str);
        return as_value(array.get());
    }

    const std::wstring& delim = utf8::decodeCanonicalString(fn.arg(0).to_string(), version);
    const size_t delimiterSize = delim.size();

    if ((version < 6 && delimiterSize == 0) ||
        (version >= 6 && fn.arg(0).is_undefined()))
    {
        // Condition 2:
        array->push(str);
        return as_value(array.get());
    }

    size_t max = wstr.size() + 1;

    if (version < 6)
    {
        // SWF5
        if (fn.nargs > 1 && !fn.arg(1).is_undefined())
        {
            int limit = fn.arg(1).to_int();
            if (limit < 1)
            {
                // Return empty array.
                return as_value(array.get());
            }
            max = utility::clamp<size_t>(limit, 0, max);
        }

        if (delimiterSize > 1 || fn.arg(0).is_undefined() || wstr.empty())
        {
            // Condition 3 (plus a shortcut if the string itself
            // is empty).
	        array->push(str);
	        return as_value(array.get());            
        }
    }
    else
    {
        // SWF6+
        if (wstr.empty())
        {
            // If the string itself is empty, SWF6 returns a 0-sized
            // array only if the delimiter is also empty. Otherwise
            // it returns an array with 1 empty element.
            if (delimiterSize) array->push(str);
            return as_value(array.get());
        }

        // If we reach this point, the string is not empty and
        // the delimiter is defined.
        if (fn.nargs > 1 && !fn.arg(1).is_undefined())
        {
	        int limit = fn.arg(1).to_int();
	        if (limit < 1) {
	            // Return empty array if 
	            return as_value(array.get());
	        }
            max = utility::clamp<size_t>(limit, 0, max);
        }

        // If the delimiter is empty, put each character in an
        // array element.
        if ( delim.empty() ) {
            for (size_t i = 0, e = wstr.size(); i < e; ++i) {
                array->push(utf8::encodeCanonicalString(wstr.substr(i, 1), version));
            }
            return as_value(array.get());
        }

    }

    size_t pos = 0, prevpos = 0;
    size_t num = 0;

    while (num < max) {
        pos = wstr.find(delim, pos);

        array->push(utf8::encodeCanonicalString(
               		wstr.substr(prevpos, pos - prevpos),
               		version));
        if (pos == std::wstring::npos) break;
        num++;
        prevpos = pos + delimiterSize;
        pos++;
    }

    return as_value(array.get());
}

/// String.lastIndexOf[string[, pos]]
//
/// Performs a reverse search for the complete search string, optionally
/// starting from pos. Returns -1 if not found.
static as_value
string_last_index_of(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = ensureType<as_object>(fn.this_ptr);
    as_value val(fn.this_ptr);
    
    const std::string& str = val.to_string();

    if (!checkArgs(fn, 1, 2, "String.lastIndexOf()")) return as_value(-1);

    const std::string& toFind = fn.arg(0).to_string();

    int start = str.size();

    if (fn.nargs >= 2) {
        start = fn.arg(1).to_int();
    }
    
    if (start < 0) {
        return as_value(-1);
    }

    size_t found = str.rfind(toFind, start);

    if (found == std::string::npos) {
        return as_value(-1);
    }

    return as_value(found);
}

// String.substr(start[, length]).
// If the second value is absent or undefined, the remainder of the string from
// <start> is returned.
// If start is more than string length or length is 0, empty string is returned.
// If length is negative, the substring is taken from the *end* of the string.
static as_value
string_sub_str(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = ensureType<as_object>(fn.this_ptr);
    as_value val(fn.this_ptr);
    
    const std::string& str = val.to_string();

    int version = obj->getVM().getSWFVersion();

    std::wstring wstr = utf8::decodeCanonicalString(str, version);

    if (!checkArgs(fn, 1, 2, "String.substr()")) return as_value(str);
    
    int start = validIndex(wstr, fn.arg(0).to_int());

    int num = wstr.length();

    if (fn.nargs >= 2 && !fn.arg(1).is_undefined())
    {
        num = fn.arg(1).to_int();
	    if ( num < 0 )
	    {
		    if ( -num <= start ) num = 0;
		    else
		    {
			    num = wstr.length() + num;
			    if ( num < 0 ) return as_value("");
		    }
	    }
    }

    return as_value(utf8::encodeCanonicalString(wstr.substr(start, num), version));
}

// string.substring(start[, end])
// If *either* value is less than 0, 0 is used.
// The values are *then* swapped if end is before start.
// Valid values for the start position are up to string 
// length - 1.
static as_value
string_sub_string(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = ensureType<as_object>(fn.this_ptr);
    as_value val(fn.this_ptr);
    
    const std::string& str = val.to_string();

    int version = obj->getVM().getSWFVersion();

    const std::wstring& wstr = utf8::decodeCanonicalString(str, version);

    if (!checkArgs(fn, 1, 2, "String.substring()")) return as_value(str);

    int start = fn.arg(0).to_int();
    int end = wstr.size();

    if (start < 0) {
        start = 0;
    }

    if (static_cast<unsigned>(start) >= wstr.size()) {
        return as_value("");
    }

    if (fn.nargs >= 2) {
        int num = fn.arg(1).to_int();

        if (num < 0) {
            num = 0;
        }

        end = num;
        
        if (end < start) {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("string.slice() called with end < start"));
            )
            std::swap (end, start);
        }
    }
    
    if (static_cast<unsigned>(end) > wstr.size()) {
        end = wstr.size();
    }
    
    end -= start;
    //log_debug("Start: %d, End: %d", start, end);

    return as_value(utf8::encodeCanonicalString(wstr.substr(start, end), version));
}

static as_value
string_index_of(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = ensureType<as_object>(fn.this_ptr);
    as_value val(fn.this_ptr);
 
    /// Do not return before this, because the toString method should always
    /// be called. (TODO: test).   
    const std::string& str = val.to_string();

    if (!checkArgs(fn, 1, 2, "String.indexOf")) return as_value(-1);

    int version = obj->getVM().getSWFVersion();

    const std::wstring& wstr = utf8::decodeCanonicalString(str, version);

    const as_value& tfarg = fn.arg(0); // to find arg
    const std::wstring& toFind = utf8::decodeCanonicalString(tfarg.to_string(), version);

    size_t start = 0;

    if (fn.nargs >= 2)
    {
        const as_value& saval = fn.arg(1); // start arg val
        int start_arg = saval.to_int();
        if ( start_arg > 0 ) start = (size_t) start_arg;
	else
	{
		IF_VERBOSE_ASCODING_ERRORS(
		if ( start_arg < 0 )
		{
			log_aserror("String.indexOf(%s, %s): second argument casts to invalid offset (%d)",
				tfarg, saval, start_arg);
		}
		);
	}
    }

    size_t pos = wstr.find(toFind, start);

    if (pos == std::wstring::npos) {
        return as_value(-1);
    }

    return as_value(pos);
}

// String.fromCharCode(code1[, code2[, code3[, code4[, ...]]]])
// Makes a string out of any number of char codes.
// The string is always UTF8, so SWF5 mangles it.
static as_value
string_from_char_code(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = ensureType<as_object>(fn.this_ptr);

    const int version = obj->getVM().getSWFVersion();

    if (version == 5)
    {
        std::string str;
        for (unsigned int i = 0; i < fn.nargs; i++)
        {
            // Maximum 65535, as with all character codes.
            boost::uint16_t c = static_cast<boost::uint16_t>(fn.arg(i).to_int());
            
            // If more than 255, push 'overflow' byte.
            if (c > 255)
            {
                str.push_back(static_cast<unsigned char>(c >> 8));
            }

            // 0 terminates the string, but mustn't be pushed or it
            // will break concatenation.
            if (static_cast<unsigned char>(c) == 0) break;
            str.push_back(static_cast<unsigned char>(c));
        }    
        return as_value(str);
    }

    std::wstring wstr;

    for (unsigned int i = 0; i < fn.nargs; i++)
    {
        boost::uint16_t c = static_cast<boost::uint16_t>(fn.arg(i).to_int());
        if (c == 0) break;
        wstr.push_back(c);
    }
    
    return as_value(utf8::encodeCanonicalString(wstr, version));

}

static as_value
string_char_code_at(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = ensureType<as_object>(fn.this_ptr);
    as_value val(fn.this_ptr);
    
    const std::string& str = val.to_string();

    int version = obj->getVM().getSWFVersion();

    const std::wstring& wstr = utf8::decodeCanonicalString(str, version);

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

    size_t index = static_cast<size_t>(fn.arg(0).to_number());

    if (index >= wstr.length()) {
        as_value rv;
        rv.set_nan();
        return rv;
    }

    return as_value(wstr.at(index));
}

static as_value
string_char_at(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = ensureType<as_object>(fn.this_ptr);
    as_value val(fn.this_ptr);
    
    const std::string& str = val.to_string();

    const int version = obj->getVM().getSWFVersion();

    if (!checkArgs(fn, 1, 1, "String.charAt()")) return as_value("");

    // to_int() makes this safe from overflows.
    const size_t index = static_cast<size_t>(fn.arg(0).to_int());

    size_t currentIndex = 0;

    std::string::const_iterator it = str.begin(), e = str.end();

    while (boost::uint32_t code = utf8::decodeNextUnicodeCharacter(it, e))
    {
        if (currentIndex == index)
        {
            if (version == 5)
            {
                return as_value(utf8::encodeLatin1Character(code));
            }
            return as_value(utf8::encodeUnicodeCharacter(code));
        }
        ++currentIndex;
    }

    // We've reached the end without finding the index
    return as_value("");
}

static as_value
string_to_upper_case(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = ensureType<as_object>(fn.this_ptr);
    as_value val(fn.this_ptr);

    VM& vm = obj->getVM();
    const int version = vm.getSWFVersion();

    std::wstring wstr = utf8::decodeCanonicalString(val.to_string(), version);

    // If this is the C locale, the conversion will be wrong.
    // Most other locales are correct. FIXME: get this to
    // work regardless of user's current settings.
    std::locale currentLocale;
    try
    {
        currentLocale = std::locale("");
    }
    catch (std::runtime_error& e)
    {
        currentLocale = std::locale::classic();
    }

    if (currentLocale == std::locale::classic())
    {
        LOG_ONCE(
            log_error(_("Your locale probably can't convert non-ascii "
            "characters to upper case. Using a UTF8 locale may fix this."));
        );
    }

    boost::to_upper(wstr, currentLocale);

    return as_value(utf8::encodeCanonicalString(wstr, version));

}

static as_value
string_to_lower_case(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = ensureType<as_object>(fn.this_ptr);
    as_value val(fn.this_ptr);
    
    VM& vm = obj->getVM();
    const int version = vm.getSWFVersion();

    std::wstring wstr = utf8::decodeCanonicalString(val.to_string(), version);

    // If this is the C locale, the conversion will be wrong.
    // Most other locales are correct. FIXME: get this to
    // work regardless of user's current settings.
    std::locale currentLocale;
    try
    {
        currentLocale = std::locale("");
    }
    catch (std::runtime_error& e)
    {
        currentLocale = std::locale::classic();
    }

    if (currentLocale == std::locale::classic())
    {
        LOG_ONCE( 
            log_error(_("Your locale probably can't convert non-ascii "
                "characters to lower case. Using a UTF8 locale may fix this"));
        );
    }

    boost::to_lower(wstr, currentLocale);

    return as_value(utf8::encodeCanonicalString(wstr, version));
}

static as_value
string_oldToLower(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = ensureType<as_object>(fn.this_ptr);
    as_value val(fn.this_ptr);

    // This should use the C locale; extended characters are
    // left alone. FIXME: SWF5 should garble the output.
    std::string str = boost::to_lower_copy(val.to_string());
    return as_value(str);
}


static as_value
string_oldToUpper(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = ensureType<as_object>(fn.this_ptr);
    as_value val(fn.this_ptr);

    // This should use the C locale; extended characters are
    // left alone. FIXME: SWF5 should garble the output.
    std::string str = boost::to_upper_copy(val.to_string());
    return as_value(str);
}


static as_value
string_to_string(const fn_call& fn)
{
    boost::intrusive_ptr<String_as> obj 
	   = ensureType<String_as>(fn.this_ptr);
    return as_value(obj->str());
}


static as_value
string_ctor(const fn_call& fn)
{
	std::string str;
	
	if (fn.nargs )
	{
		str = fn.arg(0).to_string();
	}

	if ( ! fn.isInstantiation() )
	{
		return as_value(str);
	}
	
	boost::intrusive_ptr<String_as> obj = new String_as(str);

	return as_value(obj.get());
}

static boost::intrusive_ptr<builtin_function>
getStringConstructor()
{
    // This is going to be the global String "class"/"function"

    static boost::intrusive_ptr<builtin_function> cl;

    if ( cl == NULL )
    {
	VM& vm = VM::get();

        cl=new builtin_function(&string_ctor, getStringInterface());
	vm.addStatic(cl.get());

	// ASnative(251, 14) - [String] fromCharCode 
	vm.registerNative(string_from_char_code, 251, 14);
	cl->init_member("fromCharCode", vm.getNative(251, 14)); 

    }

    return cl;
}

// extern (used by Global.cpp)
void string_class_init(as_object& global)
{
    // This is going to be the global String "class"/"function"
    boost::intrusive_ptr<builtin_function> cl = getStringConstructor();

    // Register _global.String
    // TODO: register as ASnative(251, 0)
    // TODO: register as ASnative(3, 0) for SWF5 ?
    global.init_member("String", cl.get());
}

boost::intrusive_ptr<as_object>
init_string_instance(const std::string& val)
{
	// TODO: get VM from the environment ?
	VM& vm = VM::get();

	// TODO: get the environment passed in !!
	as_environment env(vm);

	int swfVersion = vm.getSWFVersion();

	boost::intrusive_ptr<as_function> cl;

	if ( swfVersion < 6 )
	{
		cl = getStringConstructor();
	}
	else
	{
		as_object* global = vm.getGlobal();
		as_value clval;
		if ( ! global->get_member(NSV::CLASS_STRING, &clval) )
		{
			log_debug("UNTESTED: String instantiation requested but _global doesn't contain a 'String' symbol. Returning the NULL object.");
			return cl;
			//cl = getStringConstructor();
		}
		else if ( ! clval.is_function() )
		{
			log_debug("UNTESTED: String instantiation requested but _global.String is not a function (%s). Returning the NULL object.",
				clval);
			return cl;
			//cl = getStringConstructor();
		}
		else
		{
			cl = clval.to_as_function();
			assert(cl);
		}
	}

	std::auto_ptr< std::vector<as_value> > args ( new std::vector<as_value> );
	args->push_back(val);
	boost::intrusive_ptr<as_object> ret = cl->constructInstance(env, args);

	return ret;
}

} // namespace gnash
