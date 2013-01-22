// string.cpp:  ActionScript "String" class, for Gnash.
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

#include "String_as.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <algorithm>
#include <locale>
#include <stdexcept>

#include "SWFCtype.h"
#include "fn_call.h"
#include "Global_as.h"
#include "as_object.h"
#include "NativeFunction.h" 
#include "log.h"
#include "as_value.h"
#include "GnashException.h"
#include "movie_definition.h" 
#include "VM.h" 
#include "namedStrings.h"
#include "utf8.h"
#include "GnashNumeric.h"
#include "Global_as.h"

namespace gnash {

// Forward declarations
namespace {

    as_value string_concat(const fn_call& fn);
    as_value string_slice(const fn_call& fn);
    as_value string_split(const fn_call& fn);
    as_value string_lastIndexOf(const fn_call& fn);
    as_value string_substr(const fn_call& fn);
    as_value string_substring(const fn_call& fn);
    as_value string_indexOf(const fn_call& fn);
    as_value string_fromCharCode(const fn_call& fn);
    as_value string_charCodeAt(const fn_call& fn);
    as_value string_charAt(const fn_call& fn);
    as_value string_toUpperCase(const fn_call& fn);
    as_value string_toLowerCase(const fn_call& fn);
    as_value string_toString(const fn_call& fn);
    as_value string_valueOf(const fn_call& fn);
    as_value string_oldToLower(const fn_call& fn);
    as_value string_oldToUpper(const fn_call& fn);
    as_value string_ctor(const fn_call& fn);

    size_t validIndex(const std::wstring& subject, int index);
    void attachStringInterface(as_object& o);

    inline bool checkArgs(const fn_call& fn, size_t min, size_t max,
            const std::string& function);

    inline int getStringVersioned(const fn_call& fn, const as_value& arg,
            std::string& str);

}

String_as::String_as(const std::string& s)
    :
    _string(s)
{
}

void
registerStringNative(as_object& global)
{
    VM& vm = getVM(global);
    vm.registerNative(string_ctor, 251, 0);
    vm.registerNative(string_valueOf, 251, 1);
    vm.registerNative(string_toString, 251, 2);
    vm.registerNative(string_oldToUpper, 102, 0);
    vm.registerNative(string_toUpperCase, 251, 3);
    vm.registerNative(string_oldToLower, 102, 1);
    vm.registerNative(string_toLowerCase, 251, 4);
    vm.registerNative(string_charAt, 251, 5);
    vm.registerNative(string_charCodeAt, 251, 6);
    vm.registerNative(string_concat, 251, 7);
    vm.registerNative(string_indexOf, 251, 8);
    vm.registerNative(string_lastIndexOf, 251, 9);
    vm.registerNative(string_slice, 251, 10);
    vm.registerNative(string_substring, 251, 11);
    vm.registerNative(string_split, 251, 12);
    vm.registerNative(string_substr, 251, 13);
    vm.registerNative(string_fromCharCode, 251, 14);
}

// extern (used by Global.cpp)
void
string_class_init(as_object& where, const ObjectURI& uri)
{
    // This is going to be the global String "class"/"function"
    
    VM& vm = getVM(where);
    Global_as& gl = getGlobal(where);

    as_object* proto = createObject(gl);
    as_object* cl = vm.getNative(251, 0);
    cl->init_member(NSV::PROP_PROTOTYPE, proto);
    proto->init_member(NSV::PROP_CONSTRUCTOR, cl);

    attachStringInterface(*proto);

    cl->init_member("fromCharCode", vm.getNative(251, 14)); 

    const int flags = PropFlags::dontEnum; 
    where.init_member(uri, cl, flags);
}


/// String class interface
namespace {

void
attachStringInterface(as_object& o)
{
    VM& vm = getVM(o);

    o.init_member("valueOf", vm.getNative(251, 1));
    o.init_member("toString", vm.getNative(251, 2));
    o.init_member("toUpperCase", vm.getNative(251, 3));
    o.init_member("toLowerCase", vm.getNative(251, 4));
    o.init_member("charAt", vm.getNative(251, 5));
    o.init_member("charCodeAt", vm.getNative(251, 6));
    o.init_member("concat", vm.getNative(251, 7));
    o.init_member("indexOf", vm.getNative(251, 8));
    o.init_member("lastIndexOf", vm.getNative(251, 9));
    o.init_member("slice", vm.getNative(251, 10));
    o.init_member("substring", vm.getNative(251, 11));
    o.init_member("split", vm.getNative(251, 12));
    o.init_member("substr", vm.getNative(251, 13));
}

// all the arguments will be converted to string and concatenated.
as_value
string_concat(const fn_call& fn)
{
    as_value val(fn.this_ptr);

    std::string str;
    const int version = getStringVersioned(fn, val, str);

    for (size_t i = 0; i < fn.nargs; i++) {
        str += fn.arg(i).to_string(version);
    }

    return as_value(str);
}


// 1st param: start_index, 2nd param: end_index
as_value
string_slice(const fn_call& fn)
{
    as_value val(fn.this_ptr);
    
    std::string str;
    const int version = getStringVersioned(fn, val, str);

    std::wstring wstr = utf8::decodeCanonicalString(str, version);

    if (!checkArgs(fn, 1, 2, "String.slice()")) return as_value();

    size_t start = validIndex(wstr, toInt(fn.arg(0), getVM(fn)));

    size_t end = wstr.length();

    if (fn.nargs >= 2)
    {
        end = validIndex(wstr, toInt(fn.arg(1), getVM(fn)));

    } 

    if (end < start) // move out of if ?
    {
            return as_value("");
    }

    size_t retlen = end - start;

    //log_debug("start: %d, end: %d, retlen: %d", start, end, retlen);

    return as_value(utf8::encodeCanonicalString(
                wstr.substr(start, retlen), version));
}

// String.split(delimiter[, limit])
// For SWF5, the following conditions mean that an array with a single
// element containing the entire string is returned:
// 1. No arguments are passed.
// 2. The delimiter is empty.
// 3. The delimiter has more than one DisplayObject or is undefined and limit is not 0.
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
as_value
string_split(const fn_call& fn)
{
    as_value val(fn.this_ptr);
    
    std::string str;
    const int version = getStringVersioned(fn, val, str);
    
    std::wstring wstr = utf8::decodeCanonicalString(str, version);

    Global_as& gl = getGlobal(fn);
    as_object* array = gl.createArray();

    if (fn.nargs == 0)
    {
        // Condition 1:
        callMethod(array, NSV::PROP_PUSH, str);
        return as_value(array);
    }

    const std::wstring& delim = utf8::decodeCanonicalString(
            fn.arg(0).to_string(), version);
    const size_t delimiterSize = delim.size();

    if ((version < 6 && delimiterSize == 0) ||
        (version >= 6 && fn.arg(0).is_undefined()))
    {
        // Condition 2:
        callMethod(array, NSV::PROP_PUSH, str);
        return as_value(array);
    }

    size_t max = wstr.size() + 1;

    if (version < 6)
    {
        // SWF5
        if (fn.nargs > 1 && !fn.arg(1).is_undefined())
        {
            int limit = toInt(fn.arg(1), getVM(fn));
            if (limit < 1)
            {
                // Return empty array.
                return as_value(array);
            }
            max = clamp<size_t>(limit, 0, max);
        }

        if (delimiterSize > 1 || fn.arg(0).is_undefined() || wstr.empty())
        {
            // Condition 3 (plus a shortcut if the string itself
            // is empty).
            callMethod(array, NSV::PROP_PUSH, str);
            return as_value(array);            
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
            if (delimiterSize) callMethod(array, NSV::PROP_PUSH, str);
            return as_value(array);
        }

        // If we reach this point, the string is not empty and
        // the delimiter is defined.
        if (fn.nargs > 1 && !fn.arg(1).is_undefined())
        {
            int limit = toInt(fn.arg(1), getVM(fn));
            if (limit < 1) {
                // Return empty array if 
                return as_value(array);
            }
            max = clamp<size_t>(limit, 0, max);
        }

        // If the delimiter is empty, put each character in an
        // array element.
        if (delim.empty()) {
            for (size_t i = 0, e = std::min<size_t>(wstr.size(), max);
                    i < e; ++i) {
                callMethod(array, NSV::PROP_PUSH,
                       utf8::encodeCanonicalString(wstr.substr(i, 1), version));
            }
            return as_value(array);
        }

    }

    size_t pos = 0, prevpos = 0;
    size_t num = 0;

    while (num < max) {
        pos = wstr.find(delim, pos);

        callMethod(array, NSV::PROP_PUSH, utf8::encodeCanonicalString(
                       wstr.substr(prevpos, pos - prevpos), version));

        if (pos == std::wstring::npos) break;
        num++;
        prevpos = pos + delimiterSize;
        pos++;
    }

    return as_value(array);
}

/// String.lastIndexOf[string[, pos]]
//
/// Performs a reverse search for the complete search string, optionally
/// starting from pos. Returns -1 if not found.
as_value
string_lastIndexOf(const fn_call& fn)
{
    as_value val(fn.this_ptr);
    
    std::string str;
    const int version = getStringVersioned(fn, val, str);
    const std::wstring& wstr = utf8::decodeCanonicalString(str, version);

    if (!checkArgs(fn, 1, 2, "String.lastIndexOf()")) return as_value(-1);

    const std::wstring& toFind = utf8::decodeCanonicalString(
        fn.arg(0).to_string(version), version);

    int start = str.size();

    if (fn.nargs >= 2) {
        start = toInt(fn.arg(1), getVM(fn));
    }
    
    if (start < 0) {
        return as_value(-1);
    }

    size_t found = wstr.rfind(toFind, start);

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
as_value
string_substr(const fn_call& fn)
{
    as_value val(fn.this_ptr);
    
    std::string str;
    const int version = getStringVersioned(fn, val, str);

    std::wstring wstr = utf8::decodeCanonicalString(str, version);

    if (!checkArgs(fn, 1, 2, "String.substr()")) return as_value(str);
    
    int start = validIndex(wstr, toInt(fn.arg(0), getVM(fn)));

    int num = wstr.length();

    if (fn.nargs >= 2 && !fn.arg(1).is_undefined())
    {
        num = toInt(fn.arg(1), getVM(fn));
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
as_value
string_substring(const fn_call& fn)
{
    as_value val(fn.this_ptr);
    
    std::string str;
    const int version = getStringVersioned(fn, val, str);

    const std::wstring& wstr = utf8::decodeCanonicalString(str, version);

    if (!checkArgs(fn, 1, 2, "String.substring()")) return as_value(str);

    const as_value& s = fn.arg(0);

    int start = toInt(s, getVM(fn));
    int end = wstr.size();

    if (s.is_undefined() || start < 0) {
        start = 0;
    }

    if (static_cast<unsigned>(start) >= wstr.size()) {
        return as_value("");
    }

    if (fn.nargs >= 2 && !fn.arg(1).is_undefined()) {
        int num = toInt(fn.arg(1), getVM(fn));

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

as_value
string_indexOf(const fn_call& fn)
{
    as_value val(fn.this_ptr);
 
    /// Do not return before this, because the toString method should always
    /// be called. (TODO: test).   
    std::string str;
    const int version = getStringVersioned(fn, val, str);

    if (!checkArgs(fn, 1, 2, "String.indexOf")) return as_value(-1);

    const std::wstring& wstr = utf8::decodeCanonicalString(str, version);

    const as_value& tfarg = fn.arg(0); // to find arg
    const std::wstring& toFind =
        utf8::decodeCanonicalString(tfarg.to_string(version),
                version);

    size_t start = 0;

    if (fn.nargs >= 2)
    {
        const as_value& saval = fn.arg(1); // start arg val
        int start_arg = toInt(saval, getVM(fn));
        if (start_arg > 0) start = (size_t) start_arg;
        else {
            IF_VERBOSE_ASCODING_ERRORS(
                if (start_arg < 0) {
                    log_aserror(_("String.indexOf(%s, %s): second argument casts "
                                  "to invalid offset (%d)"), tfarg, saval, start_arg);
                }
            );
        }
    }

    const size_t pos = wstr.find(toFind, start);

    if (pos == std::wstring::npos) {
        return as_value(-1);
    }

    return as_value(pos);
}

// String.fromCharCode(code1[, code2[, code3[, code4[, ...]]]])
// Makes a string out of any number of char codes.
// The string is always UTF8, so SWF5 mangles it.
as_value
string_fromCharCode(const fn_call& fn)
{

    const int version = getSWFVersion(fn);

    if (version == 5)
    {
        std::string str;
        for (unsigned int i = 0; i < fn.nargs; i++)
        {
            // Maximum 65535, as with all DisplayObject codes.
            const boost::uint16_t c = 
                static_cast<boost::uint16_t>(toInt(fn.arg(i), getVM(fn)));
            
            // If more than 255, push 'overflow' byte.
            if (c > 255) {
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
        const boost::uint16_t c = 
            static_cast<boost::uint16_t>(toInt(fn.arg(i), getVM(fn)));
        if (c == 0) break;
        wstr.push_back(c);
    }
    
    return as_value(utf8::encodeCanonicalString(wstr, version));

}

as_value
string_charCodeAt(const fn_call& fn)
{
    as_value val(fn.this_ptr);
    
    std::string str;
    const int version = getStringVersioned(fn, val, str);

    const std::wstring& wstr = utf8::decodeCanonicalString(str, version);

    if (fn.nargs == 0) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("string.charCodeAt needs one argument"));
        )
        as_value rv;
        setNaN(rv);
        return rv;    // Same as for out-of-range arg
    }

    IF_VERBOSE_ASCODING_ERRORS(
        if (fn.nargs > 1) {
            log_aserror(_("string.charCodeAt has more than one argument"));
        }
    )

    size_t index = static_cast<size_t>(toNumber(fn.arg(0), getVM(fn)));

    if (index >= wstr.length()) {
        as_value rv;
        setNaN(rv);
        return rv;
    }

    return as_value(wstr.at(index));
}

as_value
string_charAt(const fn_call& fn)
{
    as_value val(fn.this_ptr);
    
    std::string str;
    const int version = getStringVersioned(fn, val, str);

    if (!checkArgs(fn, 1, 1, "String.charAt()")) return as_value("");

    // to_int() makes this safe from overflows.
    const size_t index = static_cast<size_t>(toInt(fn.arg(0), getVM(fn)));

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

as_value
string_toUpperCase(const fn_call& fn)
{
    as_value val(fn.this_ptr);

    std::string str;
    const int version = getStringVersioned(fn, val, str);

    std::wstring wstr = utf8::decodeCanonicalString(str, version);

#if !defined(__HAIKU__) && !defined(__amigaos4__) && !defined(__ANDROID__)
    static const std::locale swfLocale((std::locale()), new SWFCtype());
    boost::to_upper(wstr, swfLocale);
#else
    size_t l = wstr.size();
    for (size_t i = 0; i < l; ++i) {
        if (wstr[i] >= 'a' && wstr[i] <= 'z') {
            wstr[i] += 'A' - 'a';
        }
    }
#endif

    return as_value(utf8::encodeCanonicalString(wstr, version));

}

as_value
string_toLowerCase(const fn_call& fn)
{
    as_value val(fn.this_ptr);
    
    std::string str;
    const int version = getStringVersioned(fn, val, str);

    std::wstring wstr = utf8::decodeCanonicalString(str, version);

#if !defined(__HAIKU__) && !defined(__amigaos4__) && !defined(__ANDROID__)
    static const std::locale swfLocale((std::locale()), new SWFCtype());
    boost::to_lower(wstr, swfLocale);
#else
    size_t l = wstr.size();
    for (size_t i = 0; i < l; ++i) {
        if (wstr[i] >= 'A' && wstr[i] <= 'Z') {
            wstr[i] -= 'A' - 'a';
        }
    }
#endif

    return as_value(utf8::encodeCanonicalString(wstr, version));
}

as_value
string_oldToLower(const fn_call& fn)
{
    as_value val(fn.this_ptr);

    // This should use the C locale; extended DisplayObjects are
    // left alone. FIXME: SWF5 should garble the output.
    std::string str = boost::to_lower_copy(val.to_string());
    return as_value(str);
}


as_value
string_oldToUpper(const fn_call& fn)
{
    as_value val(fn.this_ptr);

    // This should use the C locale; extended DisplayObjects are
    // left alone. FIXME: SWF5 should garble the output.
    std::string str = boost::to_upper_copy(val.to_string());
    return as_value(str);
}

/// This returns as_value.toString() value of an object. For Strings this is
/// a string, for objects "[type Object]" or "[type Function]", for Booleans
/// "true" or "false", etc.
as_value
string_valueOf(const fn_call& fn)
{
    const int version = getSWFVersion(fn);
    return as_value(fn.this_ptr).to_string(version);
}

as_value
string_toString(const fn_call& fn)
{
    String_as* str = ensure<ThisIsNative<String_as> >(fn);
    return as_value(str->value());
}


as_value
string_ctor(const fn_call& fn)
{
    const int version = getSWFVersion(fn);

    std::string str;

    if (fn.nargs) {
        str = fn.arg(0).to_string(version);
    }

    if (!fn.isInstantiation())
    {
        return as_value(str);
    }
    
    as_object* obj = fn.this_ptr;

    obj->setRelay(new String_as(str));
    std::wstring wstr = utf8::decodeCanonicalString(str, getSWFVersion(fn));
    obj->init_member(NSV::PROP_LENGTH, wstr.size(), as_object::DefaultFlags);

    return as_value();
}
    
inline int
getStringVersioned(const fn_call& fn, const as_value& val, std::string& str)
{

    /// version to use is the one of the SWF containing caller code.
    /// If callerDef is null, this calls is spontaneous (system-event?)
    /// in which case we should research on which version should drive
    /// behaviour.
    /// NOTE: it is unlikely that a system event triggers string_split so
    ///       in most cases a null callerDef means the caller forgot to 
    ///       set the field (ie: a programmatic error)
    if (!fn.callerDef) {
        log_error(_("No fn_call::callerDef in string function call"));
    }

    const int version = fn.callerDef ? fn.callerDef->get_version() :
        getSWFVersion(fn);
    
    str = val.to_string(version);

    return version;


}

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

size_t
validIndex(const std::wstring& subject, int index)
{

    if (index < 0) {
        index = subject.size() + index;
    }

    index = clamp<int>(index, 0, subject.size());

    return index;
}

} // anonymous namespace
} // namespace gnash
