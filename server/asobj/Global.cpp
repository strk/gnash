// Global.cpp:  Global ActionScript class setup, for Gnash.
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

/* $Id: Global.cpp,v 1.89 2008/01/31 15:19:06 bwy Exp $ */

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "as_object.h"
#include "as_prop_flags.h"
#include "as_value.h"
#include "as_function.h" // for function_class_init
#include "array.h"
#include "AsBroadcaster.h"
#include "Boolean.h"
#include "Camera.h"
#include "Color.h"
#include "ContextMenu.h"
#include "CustomActions.h"
#include "Date.h"
#include "Error.h"
#include "Global.h"
#include "gstring.h"
#include "Key.h"
#include "LoadVars.h"
#include "LocalConnection.h"
#include "Microphone.h"
#include "Number.h"
#include "Object.h"
#include "GMath.h"
#include "Mouse.h"
#include "MovieClipLoader.h"
#include "movie_definition.h"
#include "NetConnection.h"
#include "NetStream.h"
#include "Selection.h"
#include "SharedObject.h"
#include "Sound.h"
#include "Stage.h"
#include "System.h"
#include "textformat.h"
#include "TextSnapshot.h"
#include "video_stream_instance.h"
#include "extension.h"
#include "VM.h"
#include "timers.h"
#include "URL.h" // for URL::encode and URL::decode (escape/unescape)
#include "builtin_function.h"
#include "edit_text_character.h"
#include "rc.h"
#include "ClassHierarchy.h"
#include "namedStrings.h"

#include "fn_call.h"
#include "sprite_instance.h"

#include "xml.h"
#include "xmlsocket.h"

#include <limits> // for numeric_limits<double>::quiet_NaN

// Common code to warn and return if a required single arg is not present
// and to warn if there are extra args.
#define ASSERT_FN_ARGS_IS_1						\
    if (fn.nargs < 1) {							\
	IF_VERBOSE_ASCODING_ERRORS(					\
            log_aserror(_("%s needs one argument"), __FUNCTION__);		\
            )								\
         return as_value();							\
    }									\
    IF_VERBOSE_ASCODING_ERRORS(						\
	if (fn.nargs > 1)						\
            log_aserror(_("%s has more than one argument"), __FUNCTION__);	\
    )

using namespace std;

namespace gnash {

static as_value
as_global_trace(const fn_call& fn)
{
    ASSERT_FN_ARGS_IS_1

    // Log our argument.
    //
    // @@ what if we get extra args?
    //
    // @@ Nothing needs special treatment,
    //    as_value::to_string() will take care of everything
    const char* arg0 = fn.arg(0).to_string().c_str();
    log_trace("%s", arg0);
    return as_value();
}


static as_value
as_global_isnan(const fn_call& fn)
{
    ASSERT_FN_ARGS_IS_1

    return as_value( static_cast<bool>(isnan(fn.arg(0).to_number()) ));
}

static as_value
as_global_isfinite(const fn_call& fn)
{
    ASSERT_FN_ARGS_IS_1

    return as_value( (bool)isfinite(fn.arg(0).to_number()) );
}

/// \brief Encode a string to URL-encoded format
/// converting all dodgy characters to %AB hex sequences
//
/// Characters that need escaping are:
/// - ASCII control characters: 0-31 and 127
/// - Non-ASCII chars: 128-255
/// - URL syntax characters: $ & + , / : ; = ? @
/// - Unsafe characters: SPACE " < > # % { } | \ ^ ~ [ ] `
/// Encoding is a % followed by two hexadecimal characters, case insensitive.
/// See RFC1738 http://www.rfc-editor.org/rfc/rfc1738.txt,
/// Section 2.2 "URL Character Encoding Issues"

static as_value
as_global_escape(const fn_call& fn)
{
    ASSERT_FN_ARGS_IS_1

    string input = fn.arg(0).to_string();
    URL::encode(input);
    return as_value(input.c_str());
}

/// \brief Decode a string from URL-encoded format
/// converting all hexadecimal sequences to ASCII characters.
//
/// A sequence to convert is % followed by two case-independent hexadecimal
/// digits, which is replaced by the equivalent ASCII character.
/// See RFC1738 http://www.rfc-editor.org/rfc/rfc1738.txt,
/// Section 2.2 "URL Character Encoding Issues"

static as_value
as_global_unescape(const fn_call& fn)
{
    ASSERT_FN_ARGS_IS_1

    string input = fn.arg(0).to_string();
    URL::decode(input);
    return as_value(input.c_str());
}

static as_value
as_global_parsefloat(const fn_call& fn)
{
    ASSERT_FN_ARGS_IS_1

    float result;
    as_value rv;

    // sscanf will handle the whitespace / unneeded characters etc. automatically
    if (1 == sscanf(fn.arg(0).to_string().c_str(), "%f", &result))
	rv = double(result);
    else
	// if sscanf didn't find anything, return NaN
	rv.set_nan();

    return rv;
}

static as_value
as_global_parseint(const fn_call& fn)
{
    // assert(fn.nargs == 2 || fn.nargs == 1);
    if (fn.nargs < 1) {
	IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("%s needs at least one argument"), __FUNCTION__);
            )
         return as_value();
    }
    IF_VERBOSE_ASCODING_ERRORS(
	if (fn.nargs > 2)
            log_aserror(_("%s has more than two arguments"), __FUNCTION__);
    )

    const std::string& expr = fn.arg(0).to_string();
    
    int base = 10;
    const std::string digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    bool bNegative = false;
  
    std::string::const_iterator it = expr.begin();

    // Skip leading whitespace
    while(*it == ' ' || *it == '\n' || *it == '\t' || *it == '\r')
    {
        ++it;
    }

    // Is the first non-whitespace character a minus?
    if (*it == '-')
    {
        bNegative = true;
	++it;
    }

    // if we were sent a second argument, that's our base
    if (fn.nargs > 1)
    {
	// to_number returns a double. atoi() would be better
	base = (int)(fn.arg(1).to_number());
	
	// Bases from 2 to 36 are valid, otherwise return NaN
        if (base < 2 || base > 36)
        {
	    as_value rv;
	    rv.set_nan();
	    return rv;
        }	

    }

    // If the string starts with '0x':
    else if (expr.end() - it >= 2 &&
    		(*it == '0' && toupper(*(it + 1)) == 'X' ))
    {
        // the base is 16
	base = 16;
        // Move to the digit after the 'x'
	it += 2; 
    }

    // Octal if the string starts with "0" then an octal digit, but
    // *only* if there is no whitespace before it; in that case decimal.
    else if (it - expr.begin() == (bNegative ? 1 : 0))
    {
        if (expr.end() - it >= 2 && 
    		*it == '0' && isdigit(*(it + 1)))
        {
            // And if there are any chars other than 0-7, it's *still* a
            // base 10 number...
            // At least we know where we are in the string, so can use
            // string methods.
	    if (expr.find_first_not_of("01234567", (bNegative ? 1 : 0)) !=
	    	std::string::npos)
	    {
	        base = 10;
	    }
	    else base = 8;

	    // Point the iterator to the first digit after the '0'.
	    ++it;

        }
    }

    // Check to see if the first digit is valid, otherwise 
    // return NaN.
    int digit = digits.find(toupper(*it));

    if (digit >= base)
    {
	    as_value rv;
	    rv.set_nan();
	    return rv;
    }

    // The first digit was valid, so continue from the present position
    // until we reach the end of the string or an invalid character,
    // adding valid characters to our result.
    // Which characters are invalid depends on the base. 
    int result = digit;
    ++it;
    
    while (it != expr.end() && (digit = digits.find(toupper(*it))) < base
    		&& digit >= 0)
    {
	    result = result * base + digit;
	    ++it;
    }

    if (bNegative)
	result = -result;
    
    // Now return the parsed string as an integer.
    return as_value(result);
}

// ASSetPropFlags function
static as_value
as_global_assetpropflags(const fn_call& fn)
{
    int version = VM::get().getSWFVersion();

    //log_msg(_("ASSetPropFlags called with %d args"), fn.nargs);

    // Check the arguments
    // assert(fn.nargs == 3 || fn.nargs == 4);
    // assert((version == 5) ? (fn.nargs == 3) : true);

    if (fn.nargs < 3) {
	IF_VERBOSE_ASCODING_ERRORS(	
            log_aserror(_("%s needs at least three arguments"), __FUNCTION__);
            )
         return as_value();
    }
    IF_VERBOSE_ASCODING_ERRORS(
	if (fn.nargs > 4)
            log_aserror(_("%s has more than four arguments"), __FUNCTION__);
	if (version == 5 && fn.nargs == 4)
            log_aserror(_("%s has four arguments in a SWF version 5 movie"), __FUNCTION__);
    )
		
    // ASSetPropFlags(obj, props, n, allowFalse=false)

    // object
    boost::intrusive_ptr<as_object> obj = fn.arg(0).to_object();
    if ( ! obj )
    {
		log_error(_("Invalid call to ASSetPropFlags: "
			"object argument is not an object: %s"),
			fn.arg(0).to_string().c_str());
		return as_value();
    }

    // list of child names

    as_value& props = fn.arg(1);

    // a number which represents three bitwise flags which
    // are used to determine whether the list of child names should be hidden,
    // un-hidden, protected from over-write, un-protected from over-write,
    // protected from deletion and un-protected from deletion
    int set_true = int(fn.arg(2).to_number()) & as_prop_flags::as_prop_flags_mask;

    // Is another integer bitmask that works like set_true,
    // except it sets the attributes to false. The
    // set_false bitmask is applied before set_true is applied

    // ASSetPropFlags was exposed in Flash 5, however the fourth argument 'set_false'
    // was not required as it always defaulted to the value '~0'. 
    int set_false = (fn.nargs == 3 ? 
		     (version == 5 ? ~0 : 0) : int(fn.arg(3).to_number()))
	& as_prop_flags::as_prop_flags_mask;

	obj->setPropFlags(props, set_false, set_true);

    return as_value();
}

// ASNative function
// See: http://osflash.org/flashcoders/undocumented/asnative?s=asnative
static as_value
as_global_asnative(const fn_call& fn)
{
	as_value ret;

	if (fn.nargs < 2)
	{
		IF_VERBOSE_ASCODING_ERRORS(	
		log_aserror(_("ASNative(%s): needs at least two arguments"), fn.dump_args().c_str());
		)
		return ret;
	}

	int sx = fn.arg(0).to_int();
	int sy = fn.arg(1).to_int();

	if ( sx < 0 )
	{
		IF_VERBOSE_ASCODING_ERRORS(	
		log_aserror(_("ASNative(%s): first arg must be >= 0"), fn.dump_args().c_str());
		)
		return ret;
	}
	if ( sy < 0 )
	{
		IF_VERBOSE_ASCODING_ERRORS(	
		log_aserror(_("ASNative(%s): second arg must be >= 0"), fn.dump_args().c_str());
		)
		return ret;
	}

	unsigned x = (unsigned)sx;
	unsigned y = (unsigned)sy;

	VM& vm = VM::get();
	as_function* fun = vm.getNative(x, y);
	if ( ! fun ) return ret;
	ret.set_as_function(fun);
	return ret;
		
}

// ASSetNative function
// TODO: find dox 
static as_value
as_global_assetnative(const fn_call& /*fn*/)
{
	log_unimpl("ASSetNative");
	return as_value();
}

// ASSetNativeAccessor function
// TODO: find dox 
static as_value
as_global_assetnativeaccessor(const fn_call& /*fn*/)
{
	log_unimpl("ASSetNativeAccessor");
	return as_value();
}

// ASconstructor function
// TODO: find dox 
static as_value
as_global_asconstructor(const fn_call& /*fn*/)
{
	log_unimpl("ASconstructor");
	return as_value();
}

// updateAfterEvent function
static as_value
as_global_updateAfterEvent(const fn_call& /*fn*/)
{
	static bool warned=false;
	if ( ! warned )
	{
		log_unimpl("updateAfterEvent()");
		warned=true;
	}
	return as_value();
}

Global::Global(VM& vm, ClassHierarchy *ch)
	:
	as_object()
{

	//-------------------------------------------------
	// Unclassified - TODO: move to appropriate section
	//
	// WARNING: this approach seems to be bogus, in 
	//          that the proprietary player seems to 
	//          always provide all the core classes it
	//          supports, reguardless of target SWF version.
	//          The only difference seems to be in actual
	//          usability of them. For example some will
	//          be available [ typeof(Name) == 'function' ]
	//          but not instanciatable.
	//-------------------------------------------------

	// ASSetPropFlags
	init_member("ASSetPropFlags", new builtin_function(as_global_assetpropflags));
	init_member("ASnative", new builtin_function(as_global_asnative));
	init_member("ASSetNative", new builtin_function(as_global_assetnative));
	init_member("ASSetNativeAccessor", new builtin_function(as_global_assetnativeaccessor));
	init_member("ASconstructor", new builtin_function(as_global_asconstructor));
	init_member("updateAfterEvent", new builtin_function(as_global_updateAfterEvent));

	// Defined in timers.h
	init_member("setInterval", new builtin_function(timer_setinterval));
	init_member("clearInterval", new builtin_function(timer_clearinterval));
	init_member("setTimeout", new builtin_function(timer_settimeout));
	init_member("clearTimeout", new builtin_function(timer_clearinterval));

	ch->setGlobal(this);

// If extensions aren't used, then no extensions will be loaded.
#ifdef USE_EXTENSIONS
	ch->setExtension(&et);
#endif

	ch->massDeclare(vm.getSWFVersion());

	if (vm.getSWFVersion() >= 5)
	{
		object_class_init(*this);
		ch->getGlobalNs()->stubPrototype(NSV::CLASS_OBJECT);
		ch->getGlobalNs()->getClass(NSV::CLASS_OBJECT)->setDeclared();
		array_class_init(*this);
		ch->getGlobalNs()->stubPrototype(NSV::CLASS_ARRAY);
		ch->getGlobalNs()->getClass(NSV::CLASS_ARRAY)->setDeclared();
		string_class_init(*this);
		ch->getGlobalNs()->stubPrototype(NSV::CLASS_STRING);
		ch->getGlobalNs()->getClass(NSV::CLASS_STRING)->setDeclared();
	}
	if (vm.getSWFVersion() >= 6)
	{
		function_class_init(*this);
		ch->getGlobalNs()->stubPrototype(NSV::CLASS_FUNCTION);
		ch->getGlobalNs()->getClass(NSV::CLASS_FUNCTION)->setDeclared();
	}
	
	if ( vm.getSWFVersion() < 3 ) goto extscan;
	//-----------------------
	// SWF3
	//-----------------------

	if ( vm.getSWFVersion() < 4 ) goto extscan;
	//-----------------------
	// SWF4
	//-----------------------

	init_member("trace", new builtin_function(as_global_trace));

	if ( vm.getSWFVersion() < 5 ) goto extscan;
	//-----------------------
	// SWF5
	//-----------------------

	init_member("escape", new builtin_function(as_global_escape));
	init_member("unescape", new builtin_function(as_global_unescape));
	init_member("parseFloat", new builtin_function(as_global_parsefloat));
	init_member("parseInt", new builtin_function(as_global_parseint));
	init_member("isNaN", new builtin_function(as_global_isnan));
	init_member("isFinite", new builtin_function(as_global_isfinite));

	// NaN and Infinity should only be in _global since SWF6,
	// but this is just because SWF5 or lower did not have a "_global"
	// reference at all, most likely.
	init_member("NaN", as_value(NAN));
	init_member("Infinity", as_value(INFINITY));

	color_class_init(*this);

	if ( vm.getSWFVersion() < 6 ) goto extscan;
	//-----------------------
	// SWF6
	//-----------------------
	init_member("LocalConnection", new builtin_function(localconnection_new));
	init_member("TextFormat", new builtin_function(textformat_new));

	if ( vm.getSWFVersion() < 7 ) goto extscan;
	//-----------------------
	// SWF7
	//-----------------------

	if ( vm.getSWFVersion() < 8 ) goto extscan;
	//-----------------------
	// SWF8
	//-----------------------

extscan: 

	//-----------------------
	// Extensions
	//-----------------------
        // Scan the plugin directories for all plugins, and load them now.
        // FIXME: this should actually be done dynamically, and only
        // if a plugin defines a class that a movie actually wants to
        // use.
#ifdef USE_EXTENSIONS

	if ( RcInitFile::getDefaultInstance().enableExtensions() )
	{
		log_security("Extensions enabled, scanning plugin dir for load");
		//et.scanDir("/usr/local/lib/gnash/plugins");
		et.scanAndLoad(*this);
	}
	else
	{
		log_security("Extensions disabled");
	}
#else
	return;
#endif
}

} // namespace gnash
