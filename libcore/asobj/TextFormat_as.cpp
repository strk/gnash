// textformat.cpp:  ActionScript text formatting decorators, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include "TextFormat_as.h"

#include <boost/optional.hpp>

#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "NativeFunction.h" 
#include "namedStrings.h"
#include "VM.h"
#include "RGBA.h" 
#include "StringPredicates.h"
#include "GnashNumeric.h"
#include "Array_as.h"
#include "fontlib.h"
#include "Font.h"


namespace gnash {

// Functions and templates for reducing code duplication.
namespace {

/// Set functors may need access to fn_call resources, e.g. SWF version.
//
/// Currently only as_value::to_string() needs this, but in future other
/// conversions may follow.
struct SetBase
{
    SetBase(const fn_call& fn) : _fn(fn) {}
    const fn_call& fn() const {
        return _fn;
    }
private:
    const fn_call& _fn;
};

/// Convert pixels to twips, treating negative values as 0.
struct
PositiveTwips : SetBase
{
    PositiveTwips(const fn_call& fn) : SetBase(fn) {}
    int operator()(const as_value& val) const {
        return pixelsToTwips(std::max<int>(toInt(val, getVM(fn())), 0));
    }
};

/// Convert an argument from a number of pixels into twips.
struct
PixelsToTwips : SetBase
{
    PixelsToTwips(const fn_call& fn) : SetBase(fn) {}
    boost::int32_t operator()(const as_value& val) const {
        return pixelsToTwips(toNumber(val, getVM(fn())));
    }
};

/// Convert the as_value to a boolean.
struct
ToBool : SetBase
{
    ToBool(const fn_call& fn) : SetBase(fn) {}
    bool operator()(const as_value& val) const {
        return toBool(val, getVM(fn()));
    }
};

/// Convert the as_value to a string.
struct
ToString : SetBase
{
    ToString(const fn_call& fn) : SetBase(fn) {}
    std::string operator()(const as_value& val) const {
        return val.to_string(getSWFVersion(fn()));
    }
};


/// Get functors.
//
/// Conversions and processing are done when setting, so these functors should
/// be relatively simple.

/// Do nothing, i.e. return exactly the same argument that was passed.
struct
Nothing
{
    template<typename T> const T& operator()(const T& val) const {
        return val;
    }
};

/// Convert internal twip values to pixel values for ActionScript.
struct
TwipsToPixels
{
    template<typename T> double operator()(const T& t) const {
        return twipsToPixels(t);
    }
};

/// Produce a function to set a TextFormat property
//
/// @tparam T       The type of the Relay (should be TextFormat_as)
/// @tparam U       The return type of the function to be called (as C++ can't
///                 yet work out what it is)
/// @tparam F       The function to call to store the value.
/// @tparam P       A function object to be applied to the argument before
///                 storing the value.
template<typename T, typename U, void(T::*F)(const boost::optional<U>&), typename P>
struct Set
{
    static as_value set(const fn_call& fn) {

        T* relay = ensure<ThisIsNative<T> >(fn);

        if (!fn.nargs) return as_value();

        const as_value& arg = fn.arg(0);
        // Undefined doesn't do anything.

        if (arg.is_undefined() || arg.is_null()) {
            (relay->*F)(boost::optional<U>());
            return as_value();
        }

        // The function P takes care of converting the argument to the
        // required type.
        (relay->*F)(P(fn)(arg));
        return as_value();
    }

};

/// Produce a function to get a TextFormat property
//
/// @tparam T       The type of the Relay (should be TextFormat_as)
/// @tparam U       The return type of the function to be called (as C++ can't
///                 yet work out what it is)
/// @tparam F       The function to call to retrieve the value.
/// @tparam P       A function object to be applied to the argument before
///                 returning the value.
template<typename T, typename U, const boost::optional<U>&(T::*F)() const,
    typename P = Nothing>
struct Get
{
    static as_value get(const fn_call& fn) {
        T* relay = ensure<ThisIsNative<T> >(fn);
        const boost::optional<U>& opt = (relay->*F)();
		if (opt) return as_value(P()(*opt));
		
        as_value null;
        null.set_null();
        return null;
    }
};

/// Function object for Array handling.
class
PushToVector
{
public:
    PushToVector(std::vector<int>& v, const fn_call& fn) : _v(v), _fn(fn) {}
    void operator()(const as_value& val) {
        _v.push_back(toNumber(val, getVM(_fn)));
    }
private:
    std::vector<int>& _v;
    const fn_call& _fn;
};


}

namespace {

    as_value textformat_new(const fn_call& fn);
    void attachTextFormatInterface(as_object& o);
    const char* getAlignString(TextField::TextAlignment a);
	const char* getDisplayString(TextField::TextFormatDisplay a);
	TextField::TextFormatDisplay parseDisplayString(const std::string& display);

    /// Align works a bit differently, so is currently not a template.
    as_value textformat_align(const fn_call& fn);

    /// Display is never null, so not a template.
	as_value textformat_display(const fn_call& fn);
	as_value textformat_tabStops(const fn_call& fn);
	as_value textformat_color(const fn_call& fn);
	as_value textformat_getTextExtent(const fn_call& fn);

}

TextFormat_as::TextFormat_as()
    :
    _display(TextField::TEXTFORMAT_BLOCK)
{
}



void
TextFormat_as::alignSet(const std::string& align) 
{
    StringNoCaseEqual cmp;
    if (cmp(align, "left")) alignSet(TextField::ALIGN_LEFT);
    if (cmp(align, "center")) alignSet(TextField::ALIGN_CENTER);
    if (cmp(align, "right")) alignSet(TextField::ALIGN_RIGHT);
    if (cmp(align, "justify")) alignSet(TextField::ALIGN_JUSTIFY);
}

void
TextFormat_as::displaySet(const std::string& display)
{
	displaySet(parseDisplayString(display));
}

void
registerTextFormatNative(as_object& o)
{
    VM& vm = getVM(o);
    
    // TODO: find out native accessors for kerning, letterSpacing.
    // TODO: these functions are probably split into getters and setters
    // instead of one function for both. Needs testing.
    vm.registerNative(textformat_new, 110, 0);
    
    vm.registerNative(
            Get<const TextFormat_as, std::string, &TextFormat_as::font>::get,
            110, 1);
    vm.registerNative(
            Set<TextFormat_as, std::string, &TextFormat_as::fontSet,
            ToString>::set, 
            110, 2);
    
    vm.registerNative(
            Get<const TextFormat_as, boost::uint16_t,
            &TextFormat_as::size, TwipsToPixels>::get,
            110, 3);
    vm.registerNative(
            Set<TextFormat_as, boost::uint16_t, &TextFormat_as::sizeSet,
            PixelsToTwips>::set, 
            110, 4);
    
    vm.registerNative(textformat_color, 110, 5);
    vm.registerNative(textformat_color, 110, 6);
    
    vm.registerNative(
            Get<const TextFormat_as, std::string, &TextFormat_as::url>::get,
            110, 7);
    vm.registerNative(
            Set<TextFormat_as, std::string, &TextFormat_as::urlSet,
            ToString>::set, 
            110, 8);
    
    vm.registerNative(
            Get<const TextFormat_as, std::string, &TextFormat_as::target>::get,
            110, 9);
    vm.registerNative(
            Set<TextFormat_as, std::string, &TextFormat_as::targetSet,
            ToString>::set, 
            110, 10);
    
    vm.registerNative(
            Get<const TextFormat_as, bool, &TextFormat_as::bold>::get,
            110, 11);
    vm.registerNative(
            Set<TextFormat_as, bool, &TextFormat_as::boldSet,
            ToBool>::set, 
            110, 12);
    
    vm.registerNative(
            Get<const TextFormat_as, bool, &TextFormat_as::italic>::get,
            110, 13);
    vm.registerNative(
            Set<TextFormat_as, bool, &TextFormat_as::italicSet,
            ToBool>::set, 
            110, 14);
    
    vm.registerNative(
            Get<const TextFormat_as, bool, &TextFormat_as::underlined>::get,
            110, 15);
    vm.registerNative(
            Set<TextFormat_as, bool, &TextFormat_as::underlinedSet,
            ToBool>::set, 
            110, 16);
    
    vm.registerNative(textformat_align, 110, 17);
    vm.registerNative(textformat_align, 110, 18);

    vm.registerNative(
            Get<const TextFormat_as, boost::uint16_t,
            &TextFormat_as::leftMargin, TwipsToPixels>::get,
            110, 19);
    vm.registerNative(
            Set<TextFormat_as, boost::uint16_t, &TextFormat_as::leftMarginSet,
            PositiveTwips>::set, 
            110, 20);

    vm.registerNative(
            Get<const TextFormat_as, boost::uint16_t,
            &TextFormat_as::rightMargin, TwipsToPixels>::get,
            110, 21);
    vm.registerNative(
            Set<TextFormat_as, boost::uint16_t, &TextFormat_as::rightMarginSet,
            PositiveTwips>::set, 
            110, 22);

    vm.registerNative(
            Get<const TextFormat_as, boost::uint16_t,
            &TextFormat_as::indent,
            TwipsToPixels>::get, 110, 23);
    vm.registerNative(
            Set<TextFormat_as, boost::uint16_t, &TextFormat_as::indentSet,
            PositiveTwips>::set, 
            110, 24);
    
    vm.registerNative(
            Get<const TextFormat_as, boost::uint16_t,
            &TextFormat_as::leading,
            TwipsToPixels>::get, 110, 25);
    vm.registerNative(
            Set<TextFormat_as, boost::uint16_t, &TextFormat_as::leadingSet,
            PositiveTwips>::set, 
            110, 26);

    vm.registerNative(
            Get<const TextFormat_as, boost::uint32_t,
            &TextFormat_as::blockIndent,
            TwipsToPixels>::get, 110, 27);
    vm.registerNative(
            Set<TextFormat_as, boost::uint32_t, &TextFormat_as::blockIndentSet,
            PositiveTwips>::set,
            110, 28);

    vm.registerNative(textformat_tabStops, 110, 29);
    vm.registerNative(textformat_tabStops, 110, 30);

    vm.registerNative(
            Get<const TextFormat_as, bool, &TextFormat_as::bullet,
            Nothing>::get, 110, 31);
    vm.registerNative(
            Set<TextFormat_as, bool, &TextFormat_as::bulletSet,
            ToBool>::set, 
            110, 32);

    vm.registerNative(textformat_getTextExtent, 110, 33);
}

// extern (used by Global.cpp)
void
textformat_class_init(as_object& global, const ObjectURI& uri)
{

    Global_as& gl = getGlobal(global);
    as_object* proto = createObject(gl);;
    as_object* cl = gl.createClass(&textformat_new, proto);

	global.init_member(uri, cl, as_object::DefaultFlags);

}


namespace {

/// new TextFormat([font, [size, [color, [bold, [italic, [underline, [url, [target, [align,[leftMargin, [rightMargin, [indent, [leading]]]]]]]]]]]]])
//
/// This is a native function responsible for:
/// 1. attaching properties to TextFormat.prototype
/// 2. adding a getTextExtent member to the constructed object
/// 3. attaching the TextFormat_as relay object.
/// 4. setting the appropriate native properties of TextFormat_as
as_value
textformat_new(const fn_call& fn)
{

    as_object* obj = ensure<ValidThis>(fn);

    std::auto_ptr<TextFormat_as> tf(new TextFormat_as);

	const size_t args = fn.nargs;
	
	switch (args)
	{
	    default:
	        log_error(_("Too many args (%d) passed to TextFormat"), args);
	    case 13:
	        tf->leadingSet(pixelsToTwips(toInt(fn.arg(12), getVM(fn))));
	    case 12:
	        tf->indentSet(pixelsToTwips(toInt(fn.arg(11), getVM(fn))));
	    case 11:
	        tf->rightMarginSet(pixelsToTwips(toInt(fn.arg(10), getVM(fn))));
	    case 10:
	        tf->leftMarginSet(pixelsToTwips(toInt(fn.arg(9), getVM(fn))));
	    case 9:
	        tf->alignSet(fn.arg(8).to_string());
	    case 8:
	        tf->targetSet(fn.arg(7).to_string());
	    case 7:
	        tf->urlSet(fn.arg(6).to_string());
	    case 6:
	        tf->underlinedSet(toBool(fn.arg(5), getVM(fn)));
	    case 5:
	        tf->italicSet(toBool(fn.arg(4), getVM(fn)));
	    case 4:
	        tf->boldSet(toBool(fn.arg(3), getVM(fn)));
	    case 3:
	    {
	        rgba col;
	        col.parseRGB(toInt(fn.arg(2), getVM(fn)));
	        tf->colorSet(col);
	    }
	    case 2:
	        tf->sizeSet(pixelsToTwips(toInt(fn.arg(1), getVM(fn))));
	    case 1:
	        tf->fontSet(fn.arg(0).to_string());
	        break;
	    case 0:
	        // What happens here?
	        break;
	}
	
    obj->setRelay(tf.release());
    as_object* proto = obj->get_prototype();
    if (proto) attachTextFormatInterface(*proto);

    const int flags = 0;
    
    // This is a weird function with no children.
    VM& vm = getVM(fn);
    NativeFunction* gte = vm.getNative(110, 33);
    gte->clearProperties();
	obj->init_member("getTextExtent", gte, flags);

	return as_value();
}

as_value
textformat_display(const fn_call& fn)
{
    TextFormat_as* relay = ensure<ThisIsNative<TextFormat_as> >(fn);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
        ret.set_string(getDisplayString(relay->display()));
	}
	else // setter
	{
		relay->displaySet(fn.arg(0).to_string());
	}

	return ret;
}

as_value
textformat_tabStops(const fn_call& fn)
{
    TextFormat_as* relay = ensure<ThisIsNative<TextFormat_as> >(fn);
	
    if (!fn.nargs) {
        LOG_ONCE(log_unimpl(_("Getter for textformat_tabStops")));
        as_value null;
        null.set_null();
        return null;
	}
	
    as_object* arg = toObject(fn.arg(0), getVM(fn));
    if (!arg) return as_value();

	std::vector<int> tabStops;

    PushToVector pv(tabStops, fn);
    foreachArray(*arg, pv);

    relay->tabStopsSet(tabStops);
	
	return as_value();
}

as_value
textformat_color(const fn_call& fn)
{
    TextFormat_as* relay = ensure<ThisIsNative<TextFormat_as> >(fn);

	as_value ret;

	if (fn.nargs == 0) {
		if (relay->color()) ret.set_double(relay->color()->toRGB());
		else ret.set_null();
	}
	else {
		rgba newcolor;
		newcolor.parseRGB(toInt(fn.arg(0), getVM(fn)));
		relay->colorSet(newcolor);
	}

	return ret;
}

as_value
textformat_align(const fn_call& fn)
{
    TextFormat_as* relay = ensure<ThisIsNative<TextFormat_as> >(fn);

    as_value ret;

    if (fn.nargs == 0) {
        if (relay->align()) {
            ret.set_string(getAlignString(*relay->align()));
        }
        else ret.set_null();
    }
    else // setter
    {
        relay->alignSet(fn.arg(0).to_string());
    }

    return ret;
}


/// Return various dimensions of a theoretical run of text
//
/// The TextFormat's format values are used to calculate what the dimensions
/// of a TextField would be if it contained the given text.
//
/// This may never apply to embedded fonts. There is no way to instruct the
/// function to use embedded fonts, so it makes sense if it always chooses
/// the device font.
//
/// TODO: this duplicates other functionality in TextField; ideally both
/// should be fixed, tested, and merged.
as_value
textformat_getTextExtent(const fn_call& fn)
{
    TextFormat_as* relay = ensure<ThisIsNative<TextFormat_as> >(fn);
    
    if (!fn.nargs) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("TextFormat.getTextExtent requires at least one"
                          "argument"));
        );
        return as_value();
    }

    const int version = getSWFVersion(fn);
    const std::string& s = fn.arg(0).to_string(version);

    const bool limitWidth = (fn.nargs > 1);
    
    // Everything must be in twips here.
    const double tfw = limitWidth ?
        pixelsToTwips(toNumber(fn.arg(1), getVM(fn))) : 0;

    const bool bold = relay->bold() ? *relay->bold() : false;
    const bool italic = relay->italic() ? *relay->italic() : false;
    const double size = relay->size() ? *relay->size() : 240;

    // Note: currently leading is never defined for device fonts, and since
    // getTextExtent currently only takes account of device fonts we don't
    // need it.

    Font* f = relay->font() ?
        fontlib::get_font(*relay->font(), bold, italic) :
        fontlib::get_default_font().get();
    
    // Whether to use embedded fonts if required.
    const bool em = false;

    /// Advance, descent, ascent given according to square of 1024.
    //
    /// An ascent of 1024 is equal to the whole size of the character, so
    /// 240 twips for a size 12.
    const double scale = size / static_cast<double>(f->unitsPerEM(em));

    // If the text is empty, size is 0. Otherwise we start with the font
    // size.
    double height = s.empty() ? 0 : size;
    double width = 0;
    double curr = 0;
    
    const double ascent = f->ascent(em) * scale;
    const double descent = f->descent(em) * scale;

    for (std::string::const_iterator it = s.begin(), e = s.end();
            it != e; ++it) {

        const int index = f->get_glyph_index(*it, em);
        const double advance = f->get_advance(index, em) * scale;
        if (limitWidth && (curr + advance > tfw)) {
            curr = 0;
            height += size;
        }
        curr += advance;
        width = std::max(width, curr);

    }

    Global_as& gl = getGlobal(fn);
    as_object* obj = new as_object(gl);

    obj->init_member("textFieldHeight", twipsToPixels(height) + 4);
    obj->init_member("textFieldWidth",
            limitWidth ? twipsToPixels(tfw) : twipsToPixels(width) + 4);
    obj->init_member("width", twipsToPixels(width));
    obj->init_member("height", twipsToPixels(height));
    obj->init_member("ascent", twipsToPixels(ascent));
    obj->init_member("descent", twipsToPixels(descent));

    return as_value(obj);

}


void
attachTextFormatInterface(as_object& o)
{
	int flags = 0;

    NativeFunction* get;
    NativeFunction* set;

    VM& vm = getVM(o);

    get = vm.getNative(110, 1);
    set = vm.getNative(110, 2);
	o.init_property("font", *get, *set, flags);
    get = vm.getNative(110, 3);
    set = vm.getNative(110, 4);
	o.init_property("size", *get, *set, flags);
    get = vm.getNative(110, 5);
    set = vm.getNative(110, 6);
	o.init_property("color", *get, *set, flags);
    get = vm.getNative(110, 7);
    set = vm.getNative(110, 8);
	o.init_property("url", *get, *set, flags);
    get = vm.getNative(110, 9);
    set = vm.getNative(110, 10);
	o.init_property("target", *get, *set, flags);
    get = vm.getNative(110, 11);
    set = vm.getNative(110, 12);
	o.init_property("bold", *get, *set, flags);
    get = vm.getNative(110, 13);
    set = vm.getNative(110, 14);
	o.init_property("italic", *get, *set, flags);
    get = vm.getNative(110, 15);
    set = vm.getNative(110, 16);
	o.init_property("underline", *get, *set, flags);
    get = vm.getNative(110, 17);
    set = vm.getNative(110, 18);
	o.init_property("align", *get, *set, flags);
    get = vm.getNative(110, 19);
    set = vm.getNative(110, 20);
	o.init_property("leftMargin", *get, *set, flags);
    get = vm.getNative(110, 21);
    set = vm.getNative(110, 22);
	o.init_property("rightMargin", *get, *set, flags);
    get = vm.getNative(110, 23);
    set = vm.getNative(110, 24);
	o.init_property("indent", *get, *set, flags);
    get = vm.getNative(110, 25);
    set = vm.getNative(110, 26);
	o.init_property("leading", *get, *set, flags);
    get = vm.getNative(110, 27);
    set = vm.getNative(110, 28);
	o.init_property("blockIndent", *get, *set, flags);
    get = vm.getNative(110, 29);
    set = vm.getNative(110, 30);
	o.init_property("tabStops", *get, *set, flags);
    get = vm.getNative(110, 31);
    set = vm.getNative(110, 32);
	o.init_property("bullet", *get, *set, flags);

    o.init_property("display", textformat_display, textformat_display, flags);
}


TextField::TextFormatDisplay
parseDisplayString(const std::string& display)
{
	StringNoCaseEqual cmp;
	if (cmp(display, "inline")) return TextField::TEXTFORMAT_INLINE;
	if (cmp(display, "block")) return TextField::TEXTFORMAT_BLOCK;
	
    // Is this correct? We have to return something here...
	log_debug("Invalid display string %s ", display);
    return TextField::TEXTFORMAT_BLOCK;
}

const char* 
getAlignString(TextField::TextAlignment a)
{
	switch (a)
	{
		case TextField::ALIGN_LEFT:
			return "left";
		case TextField::ALIGN_CENTER:
			return "center";
		case TextField::ALIGN_RIGHT:
			return "right";
		case TextField::ALIGN_JUSTIFY:
			return "justify";
		default:
                    log_error(_("Uknown alignment value: %d, take as left"), a);
			return "left";
	}
}

const char*
getDisplayString(TextField::TextFormatDisplay a) 
{
	switch (a)
	{
		case TextField::TEXTFORMAT_INLINE:
			return "inline";
		case TextField::TEXTFORMAT_BLOCK:
			return "block";
		default:
                    log_error(_("Unknown display value: %d "), a);
            return "";
	}
}
	

} // anonymous namespace
} // end of gnash namespace
