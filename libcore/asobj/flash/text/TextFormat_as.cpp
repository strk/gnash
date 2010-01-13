// textformat.cpp:  ActionScript text formatting decorators, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#include "log.h"
#include "TextFormat_as.h"
#include "fn_call.h"
#include "Global_as.h"
#include "builtin_function.h" // for getter/setter properties
#include "NativeFunction.h" // for getter/setter properties
#include "namedStrings.h"
#include "VM.h"
#include "RGBA.h" // for rgba type
#include "StringPredicates.h" // for parseAlignString
#include "smart_ptr.h" // intrusive_ptr
#include "GnashNumeric.h"
#include "Array_as.h"


namespace gnash {

// Forward declarations

namespace {
    
    as_value textformat_new(const fn_call& fn);
    void attachTextFormatInterface(as_object& o);
    const char* getAlignString(TextField::TextAlignment a);
	const char* getDisplayString(TextField::TextFormatDisplay a);
	TextField::TextAlignment parseAlignString(const std::string& align);
	TextField::TextFormatDisplay parseDisplayString(const std::string& display);

	as_value textformat_display(const fn_call& fn);
	as_value textformat_bullet(const fn_call& fn);
	as_value textformat_tabStops(const fn_call& fn);
	as_value textformat_blockIndent(const fn_call& fn);
	as_value textformat_leading(const fn_call& fn);
	as_value textformat_indent(const fn_call& fn);
	as_value textformat_rightMargin(const fn_call& fn);
	as_value textformat_leftMargin(const fn_call& fn);
	as_value textformat_align(const fn_call& fn);
	as_value textformat_underline(const fn_call& fn);
	as_value textformat_italic(const fn_call& fn);
	as_value textformat_bold(const fn_call& fn);
	as_value textformat_target(const fn_call& fn);
	as_value textformat_url(const fn_call& fn);
	as_value textformat_color(const fn_call& fn);
	as_value textformat_size(const fn_call& fn);
	as_value textformat_font(const fn_call& fn);
	as_value textformat_getTextExtent(const fn_call& fn);

}

void
TextFormat_as::alignSet(const std::string& align) 
{
    alignSet(parseAlignString(align));
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
    vm.registerNative(textformat_font, 110, 1);
    vm.registerNative(textformat_font, 110, 2);
    vm.registerNative(textformat_size, 110, 3);
    vm.registerNative(textformat_size, 110, 4);
    vm.registerNative(textformat_color, 110, 5);
    vm.registerNative(textformat_color, 110, 6);
    vm.registerNative(textformat_url, 110, 7);
    vm.registerNative(textformat_url, 110, 8);
    vm.registerNative(textformat_target, 110, 9);
    vm.registerNative(textformat_target, 110, 10);
    vm.registerNative(textformat_bold, 110, 11);
    vm.registerNative(textformat_bold, 110, 12);
    vm.registerNative(textformat_italic, 110, 13);
    vm.registerNative(textformat_italic, 110, 14);
    vm.registerNative(textformat_underline, 110, 15);
    vm.registerNative(textformat_underline, 110, 16);
    vm.registerNative(textformat_align, 110, 17);
    vm.registerNative(textformat_align, 110, 18);
    vm.registerNative(textformat_leftMargin, 110, 19);
    vm.registerNative(textformat_leftMargin, 110, 20);
    vm.registerNative(textformat_rightMargin, 110, 21);
    vm.registerNative(textformat_rightMargin, 110, 22);
    vm.registerNative(textformat_indent, 110, 23);
    vm.registerNative(textformat_indent, 110, 24);
    vm.registerNative(textformat_leading, 110, 25);
    vm.registerNative(textformat_leading, 110, 26);
    vm.registerNative(textformat_blockIndent, 110, 27);
    vm.registerNative(textformat_blockIndent, 110, 28);
    vm.registerNative(textformat_tabStops, 110, 29);
    vm.registerNative(textformat_tabStops, 110, 30);
    vm.registerNative(textformat_bullet, 110, 31);
    vm.registerNative(textformat_bullet, 110, 32);
    vm.registerNative(textformat_getTextExtent, 110, 33);
}

TextFormat_as::TextFormat_as()
	:
	_flags(0),
	_underline(false),
	_bold(false),
	_italic(false),
	_bullet(false),
    _display(),
	_align(TextField::ALIGN_LEFT),
	_blockIndent(-1),
	_color(),
	_indent(-1),
	_leading(-1),
	_leftMargin(-1),
	_rightMargin(-1),
	_pointSize(-1),
	_tabStops(),
	_target(),
	_url()
{
}



// extern (used by Global.cpp)
void
textformat_class_init(as_object& global, const ObjectURI& uri)
{

    Global_as& gl = getGlobal(global);
    as_object* proto = gl.createObject();;
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
	        tf->leadingSet(pixelsToTwips(toInt(fn.arg(12))));
	    case 12:
	        tf->indentSet(pixelsToTwips(toInt(fn.arg(11))));
	    case 11:
	        tf->rightMarginSet(pixelsToTwips(toInt(fn.arg(10))));
	    case 10:
	        tf->leftMarginSet(pixelsToTwips(toInt(fn.arg(9))));
	    case 9:
	        tf->alignSet(fn.arg(8).to_string());
	    case 8:
	        tf->targetSet(fn.arg(7).to_string());
	    case 7:
	        tf->urlSet(fn.arg(6).to_string());
	    case 6:
	        tf->underlinedSet(fn.arg(5).to_bool());
	    case 5:
	        tf->italicedSet(fn.arg(4).to_bool());
	    case 4:
	        tf->boldSet(fn.arg(3).to_bool());
	    case 3:
	    {
	        rgba col;
	        col.parseRGB(toInt(fn.arg(2)));
	        tf->colorSet(col);
	    }
	    case 2:
	        tf->sizeSet(pixelsToTwips(toInt(fn.arg(1))));
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
		if ( relay->displayDefined() ) {
            ret.set_string(getDisplayString(relay->display()));
        }
        else ret.set_null();
	}
	else // setter
	{
		relay->displaySet(fn.arg(0).to_string());
	}

	return ret;
}

as_value
textformat_bullet(const fn_call& fn)
{
    TextFormat_as* relay = ensure<ThisIsNative<TextFormat_as> >(fn);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( relay->bulletDefined() ) ret.set_bool(relay->bullet());
		else ret.set_null();
	}
	else // setter
	{
	    // Boolean
		relay->bulletSet(fn.arg(0).to_bool());
	}

	return ret;
}

class PushToVector
{
public:
    PushToVector(std::vector<int>& v) : _v(v) {}
    void operator()(const as_value& val) {
        _v.push_back(val.to_number());
    }
private:
    std::vector<int>& _v;
};

as_value
textformat_tabStops(const fn_call& fn)
{
    TextFormat_as* relay = ensure<ThisIsNative<TextFormat_as> >(fn);
	
    if (!fn.nargs) {
		LOG_ONCE( log_unimpl("Getter for textformat_tabStops") );
        as_value null;
        null.set_null();
        return null;
	}
	
    as_object* arg = fn.arg(0).to_object(getGlobal(fn));
    if (!arg) return as_value();

	std::vector<int> tabStops;

    PushToVector pv(tabStops);
    foreachArray(*arg, pv);

    relay->tabStopsSet(tabStops);
	
	return as_value();
}

as_value
textformat_blockIndent(const fn_call& fn)
{
    TextFormat_as* relay = ensure<ThisIsNative<TextFormat_as> >(fn);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if (relay->blockIndentDefined()) {
            ret.set_double(twipsToPixels(relay->blockIndent()));
        }
		else ret.set_null();
	}
	else // setter
	{
		relay->blockIndentSet(pixelsToTwips(toInt(fn.arg(0))));
	}

	return ret;
}

as_value
textformat_leading(const fn_call& fn)
{
    TextFormat_as* relay = ensure<ThisIsNative<TextFormat_as> >(fn);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( relay->leadingDefined() ) ret.set_double(twipsToPixels(relay->leading()));
		else ret.set_null();
	}
	else // setter
	{
		relay->leadingSet(pixelsToTwips(toInt(fn.arg(0))));
	}

	return ret;
}

as_value
textformat_indent(const fn_call& fn)
{
    TextFormat_as* relay = ensure<ThisIsNative<TextFormat_as> >(fn);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( relay->indentDefined() ) ret.set_double(twipsToPixels(relay->indent()));
		else ret.set_null();
	}
	else // setter
	{
		relay->indentSet(pixelsToTwips(toInt(fn.arg(0))));
	}

	return ret;
}

as_value
textformat_rightMargin(const fn_call& fn)
{
    TextFormat_as* relay = ensure<ThisIsNative<TextFormat_as> >(fn);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( relay->rightMarginDefined() ) ret.set_double(twipsToPixels(relay->rightMargin()));
		else ret.set_null();
	}
	else // setter
	{
		relay->rightMarginSet(pixelsToTwips(toInt(fn.arg(0))));
	}

	return ret;
}

as_value
textformat_leftMargin(const fn_call& fn)
{
    TextFormat_as* relay = ensure<ThisIsNative<TextFormat_as> >(fn);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if (relay->leftMarginDefined()) {
            ret.set_double(twipsToPixels(relay->leftMargin()));
        }
		else ret.set_null();
	}
	else // setter
	{
		relay->leftMarginSet(pixelsToTwips(toInt(fn.arg(0))));
	}

	return ret;
}

as_value
textformat_align(const fn_call& fn)
{
    TextFormat_as* relay = ensure<ThisIsNative<TextFormat_as> >(fn);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( relay->alignDefined() ) {
            ret.set_string(getAlignString(relay->align()));
        }
        else ret.set_null();
	}
	else // setter
	{
		relay->alignSet(fn.arg(0).to_string());
	}

	return ret;
}

as_value
textformat_underline(const fn_call& fn)
{
    TextFormat_as* relay = ensure<ThisIsNative<TextFormat_as> >(fn);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( relay->underlinedDefined() ) ret.set_bool(relay->underlined());
		else ret.set_null();
	}
	else // setter
	{
		relay->underlinedSet(fn.arg(0).to_bool());
	}

	return ret;
}

as_value
textformat_italic(const fn_call& fn)
{
    TextFormat_as* relay = ensure<ThisIsNative<TextFormat_as> >(fn);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( relay->italicedDefined() ) ret.set_bool(relay->italiced());
		else ret.set_null();
	}
	else // setter
	{
		relay->italicedSet(fn.arg(0).to_bool());
	}

	return ret;
}

as_value
textformat_bold(const fn_call& fn)
{
    TextFormat_as* relay = ensure<ThisIsNative<TextFormat_as> >(fn);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( relay->boldDefined() ) ret.set_bool(relay->bold());
		else ret.set_null();
	}
	else // setter
	{
		relay->boldSet(fn.arg(0).to_bool());
	}

	return ret;
}

as_value
textformat_target(const fn_call& fn)
{
    TextFormat_as* relay = ensure<ThisIsNative<TextFormat_as> >(fn);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( relay->targetDefined() ) ret.set_string(relay->target());
		else ret.set_null();
	}
	else // setter
	{
		relay->targetSet(fn.arg(0).to_string());
	}

	return ret;
}

as_value
textformat_url(const fn_call& fn)
{
    TextFormat_as* relay = ensure<ThisIsNative<TextFormat_as> >(fn);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( relay->urlDefined() ) ret.set_string(relay->url());
		else ret.set_null();
	}
	else // setter
	{
		relay->urlSet(fn.arg(0).to_string());
	}

	return ret;
}

as_value
textformat_color(const fn_call& fn)
{
    TextFormat_as* relay = ensure<ThisIsNative<TextFormat_as> >(fn);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( relay->colorDefined() )  ret.set_double(relay->color().toRGB());
		else ret.set_null();
	}
	else // setter
	{
		rgba newcolor;
		newcolor.parseRGB(toInt(fn.arg(0)));
		relay->colorSet(newcolor);
	}

	return ret;
}

as_value
textformat_size(const fn_call& fn)
{
    TextFormat_as* relay = ensure<ThisIsNative<TextFormat_as> >(fn);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( relay->sizeDefined() ) ret.set_double(twipsToPixels(relay->size()));
		else ret.set_null();
	}
	else // setter
	{
		relay->sizeSet(pixelsToTwips(toInt(fn.arg(0))));
	}

	return ret;
}

as_value
textformat_font(const fn_call& fn)
{
    TextFormat_as* relay = ensure<ThisIsNative<TextFormat_as> >(fn);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if (relay->fontDefined()) ret.set_string(relay->font());
		else ret.set_null();
	}
	else // setter
	{
		relay->fontSet(fn.arg(0).to_string());
	}

	return ret;
}


as_value
textformat_getTextExtent(const fn_call& fn)
{
    TextFormat_as* relay = ensure<ThisIsNative<TextFormat_as> >(fn);
    UNUSED(relay);
	LOG_ONCE( log_unimpl("TextFormat.getTextExtent") );
	return as_value();
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


TextField::TextAlignment
parseAlignString(const std::string& align)
{
	StringNoCaseEqual cmp;
	if ( cmp(align, "left") ) return TextField::ALIGN_LEFT;
    if ( cmp(align, "center") ) return TextField::ALIGN_CENTER;
	if ( cmp(align, "right") ) return TextField::ALIGN_RIGHT;
	if ( cmp(align, "justify") ) return TextField::ALIGN_JUSTIFY;
	
	log_debug("Invalid align string %s, taking as left", align);
	return TextField::ALIGN_LEFT;
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
			log_error("Uknown alignment value: %d, take as left", a);
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
			log_error("Unknown display value: %d ", a);
            return "";
	}
}
	

} // anonymous namespace
} // end of gnash namespace
