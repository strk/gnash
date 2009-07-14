// textformat.cpp:  ActionScript text formatting decorators, for Gnash.
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

#include "log.h"
#include "Object.h" // for getObjectInterface
#include "TextFormat_as.h"
#include "fn_call.h"
#include "builtin_function.h" // for getter/setter properties
#include "namedStrings.h"
#include "VM.h"
#include "RGBA.h" // for rgba type
#include "StringPredicates.h" // for parseAlignString
#include "smart_ptr.h" // intrusive_ptr
#include "GnashNumeric.h"

namespace gnash {

// Forward declarations

namespace {
    
    as_value textformat_new(const fn_call& fn);
    as_object* getTextFormatInterface();
    void attachTextFormatInterface(as_object& o);
    const char* getAlignString(TextField::TextAlignment a);
	TextField::TextAlignment parseAlignString(const std::string& align);

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
TextFormat_as::registerNative(as_object& o)
{
    VM& vm = getVM(o);
    
    //vm.registerNative(110, 0) // [_global] TextFormat
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
	as_object(getTextFormatInterface()),
	_flags(0),
	_underline(false),
	_bold(false),
	_italic(false),
	_bullet(false),
	_align(TextField::ALIGN_LEFT),
	_blockIndent(-1),
	_color(),
	_indent(-1),
	_leading(-1),
	_leftMargin(-1),
	_rightMargin(-1),
	_pointSize(-1),
	_tabStops(-1),
	_target()
{
	init_member("getTextExtent", new builtin_function(
                textformat_getTextExtent));
}


// extern (used by Global.cpp)
void
TextFormat_as::init(as_object& global)
{
	// This is going to be the global Color "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(&textformat_new, getTextFormatInterface());
	}

	// Register _global.Color
	global.init_member("TextFormat", cl.get());

}


namespace {

/// new TextFormat([font, [size, [color, [bold, [italic, [underline, [url, [target, [align,[leftMargin, [rightMargin, [indent, [leading]]]]]]]]]]]]])
as_value
textformat_new(const fn_call& fn)
{

	boost::intrusive_ptr<TextFormat_as> tf = new TextFormat_as;
	
	const unsigned int args = fn.nargs;
	
	switch (args)
	{
	    default:
	        log_error(_("Too many args (%d) passed to TextFormat"), args);
	    case 13:
	        tf->leadingSet(pixelsToTwips(fn.arg(12).to_int()));
	    case 12:
	        tf->indentSet(pixelsToTwips(fn.arg(11).to_int()));
	    case 11:
	        tf->rightMarginSet(pixelsToTwips(fn.arg(10).to_int()));
	    case 10:
	        tf->leftMarginSet(pixelsToTwips(fn.arg(9).to_int()));
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
	        col.parseRGB(fn.arg(2).to_int());
	        tf->colorSet(col);
	    }
	    case 2:
	        tf->sizeSet(pixelsToTwips(fn.arg(1).to_int()));
	    case 1:
	        tf->fontSet(fn.arg(0).to_string());
	        break;
	    case 0:
	        // What happens here?
	        break;
	}
	
	return as_value(tf.get());
}

as_value
textformat_display(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl("TextFormat.display") );
	return as_value();
}

as_value
textformat_bullet(const fn_call& fn)
{
    // Has the right return values, but not properly implemented
	LOG_ONCE( log_unimpl("TextFormat.bullet") );

	boost::intrusive_ptr<TextFormat_as> ptr = ensureType<TextFormat_as>(fn.this_ptr);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( ptr->bulletDefined() ) ret.set_bool(ptr->bullet());
		else ret.set_null();
	}
	else // setter
	{
	    // Boolean
		ptr->bulletSet(fn.arg(0).to_bool());
	}

	return ret;
}

as_value
textformat_tabStops(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl("TextFormat.tabStops") );
	return as_value();
}

as_value
textformat_blockIndent(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat_as> ptr = ensureType<TextFormat_as>(fn.this_ptr);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( ptr->blockIndentDefined() ) ret.set_double(twipsToPixels(ptr->blockIndent()));
		else ret.set_null();
	}
	else // setter
	{
		ptr->blockIndentSet(pixelsToTwips(fn.arg(0).to_int()));
	}

	return ret;
}

as_value
textformat_leading(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat_as> ptr = ensureType<TextFormat_as>(fn.this_ptr);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( ptr->leadingDefined() ) ret.set_double(twipsToPixels(ptr->leading()));
		else ret.set_null();
	}
	else // setter
	{
		ptr->leadingSet(pixelsToTwips(fn.arg(0).to_int()));
	}

	return ret;
}

as_value
textformat_indent(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat_as> ptr = ensureType<TextFormat_as>(fn.this_ptr);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( ptr->indentDefined() ) ret.set_double(twipsToPixels(ptr->indent()));
		else ret.set_null();
	}
	else // setter
	{
		ptr->indentSet(pixelsToTwips(fn.arg(0).to_int()));
	}

	return ret;
}

as_value
textformat_rightMargin(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat_as> ptr = ensureType<TextFormat_as>(fn.this_ptr);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( ptr->rightMarginDefined() ) ret.set_double(twipsToPixels(ptr->rightMargin()));
		else ret.set_null();
	}
	else // setter
	{
		ptr->rightMarginSet(pixelsToTwips(fn.arg(0).to_int()));
	}

	return ret;
}

as_value
textformat_leftMargin(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat_as> ptr =
        ensureType<TextFormat_as>(fn.this_ptr);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if (ptr->leftMarginDefined()) {
            ret.set_double(twipsToPixels(ptr->leftMargin()));
        }
		else ret.set_null();
	}
	else // setter
	{
		ptr->leftMarginSet(pixelsToTwips(fn.arg(0).to_int()));
	}

	return ret;
}

as_value
textformat_align(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat_as> ptr =
        ensureType<TextFormat_as>(fn.this_ptr);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( ptr->alignDefined() ) {
            ret.set_string(getAlignString(ptr->align()));
        }
        else ret.set_null();
	}
	else // setter
	{
		ptr->alignSet(fn.arg(0).to_string());
	}

	return ret;
}

as_value
textformat_underline(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat_as> ptr =
        ensureType<TextFormat_as>(fn.this_ptr);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( ptr->underlinedDefined() ) ret.set_bool(ptr->underlined());
		else ret.set_null();
	}
	else // setter
	{
		ptr->underlinedSet(fn.arg(0).to_bool());
	}

	return ret;
}

as_value
textformat_italic(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat_as> ptr =
        ensureType<TextFormat_as>(fn.this_ptr);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( ptr->italicedDefined() ) ret.set_bool(ptr->italiced());
		else ret.set_null();
	}
	else // setter
	{
		ptr->italicedSet(fn.arg(0).to_bool());
	}

	return ret;
}

as_value
textformat_bold(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat_as> ptr =
        ensureType<TextFormat_as>(fn.this_ptr);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( ptr->boldDefined() ) ret.set_bool(ptr->bold());
		else ret.set_null();
	}
	else // setter
	{
		ptr->boldSet(fn.arg(0).to_bool());
	}

	return ret;
}

as_value
textformat_target(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl("TextFormat.target") );
	return as_value();
}

as_value
textformat_url(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl("TextFormat.url") );
	return as_value();
}

as_value
textformat_color(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat_as> ptr =
        ensureType<TextFormat_as>(fn.this_ptr);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( ptr->colorDefined() )  ret.set_double(ptr->color().toRGB());
		else ret.set_null();
	}
	else // setter
	{
		rgba newcolor;
		newcolor.parseRGB(fn.arg(0).to_int());
		ptr->colorSet(newcolor);
	}

	return ret;
}

as_value
textformat_size(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat_as> ptr =
        ensureType<TextFormat_as>(fn.this_ptr);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( ptr->sizeDefined() ) ret.set_double(twipsToPixels(ptr->size()));
		else ret.set_null();
	}
	else // setter
	{
		ptr->sizeSet(pixelsToTwips(fn.arg(0).to_int()));
	}

	return ret;
}

as_value
textformat_font(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat_as> ptr = 
        ensureType<TextFormat_as>(fn.this_ptr);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( ptr->fontDefined() ) ret.set_string(ptr->font());
		else ret.set_null();
	}
	else // setter
	{
		ptr->fontSet(fn.arg(0).to_string());
	}

	return ret;
}


as_value
textformat_getTextExtent(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl("TextFormat.getTextExtent") );
	return as_value();
}


void
attachTextFormatInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
	int flags = 0; // for sure we want to enum, dunno about deleting yet

	o.init_property("display", textformat_display, textformat_display, flags);
	o.init_property("bullet", textformat_bullet, textformat_bullet, flags);
	o.init_property("tabStops", textformat_tabStops, 
            textformat_tabStops, flags);
	o.init_property("blockIndent", textformat_blockIndent, 
            textformat_blockIndent, flags);
	o.init_property("leading", textformat_leading, textformat_leading, flags);
	o.init_property("indent", textformat_indent, textformat_indent, flags);
	o.init_property("rightMargin", textformat_rightMargin, 
            textformat_rightMargin, flags);
	o.init_property("leftMargin", textformat_leftMargin, 
            textformat_leftMargin, flags);
	o.init_property("align", textformat_align, textformat_align, flags);
	o.init_property("underline", textformat_underline, 
            textformat_underline, flags);
	o.init_property("italic", textformat_italic, textformat_italic, flags);
	o.init_property("bold", textformat_bold, textformat_bold, flags);
	o.init_property("target", textformat_target, textformat_target, flags);
	o.init_property("url", textformat_url, textformat_url, flags);
	o.init_property("color", textformat_color, textformat_color, flags);
	o.init_property("size", textformat_size, textformat_size, flags);
	o.init_property("font", textformat_font, textformat_font, flags);
}

as_object*
getTextFormatInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if (!o) {
		o = new as_object(getObjectInterface());
        VM::get().addStatic(o.get());
		attachTextFormatInterface(*o);
	}
	return o.get();
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

} // anonymous namespace
} // end of gnash namespace
