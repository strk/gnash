// textformat.cpp:  ActionScript text formatting decorators, for Gnash.
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

#include "log.h"
#include "Object.h" // for getObjectInterface
#include "TextFormat.h"
#include "fn_call.h"
#include "builtin_function.h" // for getter/setter properties
#include "namedStrings.h"
#include "VM.h"
#include "RGBA.h" // for rgba type
#include "StringPredicates.h" // for parseAlignString
#include "smart_ptr.h" // intrusive_ptr

namespace gnash {

// Forward declarations
static as_value textformat_new(const fn_call& fn);
static as_object* getTextFormatInterface();
static void attachTextFormatInterface(as_object& o);


void
registerTextFormatNative(as_object& o)
{
    VM& vm = o.getVM();
    
    //vm.registerNative(110, 0) // [_global] TextFormat
    vm.registerNative(&TextFormat::font_getset, 110, 1);
    vm.registerNative(&TextFormat::font_getset, 110, 2);
    vm.registerNative(&TextFormat::size_getset, 110, 3);
    vm.registerNative(&TextFormat::size_getset, 110, 4);
    vm.registerNative(&TextFormat::color_getset, 110, 5);
    vm.registerNative(&TextFormat::color_getset, 110, 6);
    vm.registerNative(&TextFormat::url_getset, 110, 7);
    vm.registerNative(&TextFormat::url_getset, 110, 8);
    vm.registerNative(&TextFormat::target_getset, 110, 9);
    vm.registerNative(&TextFormat::target_getset, 110, 10);
    vm.registerNative(&TextFormat::bold_getset, 110, 11);
    vm.registerNative(&TextFormat::bold_getset, 110, 12);
    vm.registerNative(&TextFormat::italic_getset, 110, 13);
    vm.registerNative(&TextFormat::italic_getset, 110, 14);
    vm.registerNative(&TextFormat::underline_getset, 110, 15);
    vm.registerNative(&TextFormat::underline_getset, 110, 16);
    vm.registerNative(&TextFormat::align_getset, 110, 17);
    vm.registerNative(&TextFormat::align_getset, 110, 18);
    vm.registerNative(&TextFormat::leftMargin_getset, 110, 19);
    vm.registerNative(&TextFormat::leftMargin_getset, 110, 20);
    vm.registerNative(&TextFormat::rightMargin_getset, 110, 21);
    vm.registerNative(&TextFormat::rightMargin_getset, 110, 22);
    vm.registerNative(&TextFormat::indent_getset, 110, 23);
    vm.registerNative(&TextFormat::indent_getset, 110, 24);
    vm.registerNative(&TextFormat::leading_getset, 110, 25);
    vm.registerNative(&TextFormat::leading_getset, 110, 26);
    vm.registerNative(&TextFormat::blockIndent_getset, 110, 27);
    vm.registerNative(&TextFormat::blockIndent_getset, 110, 28);
    vm.registerNative(&TextFormat::tabStops_getset, 110, 29);
    vm.registerNative(&TextFormat::tabStops_getset, 110, 30);
    vm.registerNative(&TextFormat::bullet_getset, 110, 31);
    vm.registerNative(&TextFormat::bullet_getset, 110, 32);
    vm.registerNative(&TextFormat::getTextExtent_method, 110, 33);

}

TextFormat::TextFormat()
	:
	as_object(getTextFormatInterface()),
	_flags(0),
	_underline(false),
	_bold(false),
	_italic(false),
	_bullet(false),
	_align(edit_text_character_def::ALIGN_LEFT),
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
	//log_debug("%s:", __FUNCTION__);
	init_member("getTextExtent", new builtin_function(TextFormat::getTextExtent_method));
}

/// new TextFormat([font, [size, [color, [bold, [italic, [underline, [url, [target, [align,[leftMargin, [rightMargin, [indent, [leading]]]]]]]]]]]]])
static as_value
textformat_new(const fn_call& fn)
{
	//GNASH_REPORT_FUNCTION;

	boost::intrusive_ptr<TextFormat> tf = new TextFormat;
	
	const unsigned int args = fn.nargs;
	
	switch (args)
	{
	    default:
	        log_error(_("Too many args (%d) passed to TextFormat"), args);
	    case 13:
	        tf->leadingSet(PIXELS_TO_TWIPS(fn.arg(12).to_int()));
	    case 12:
	        tf->indentSet(PIXELS_TO_TWIPS(fn.arg(11).to_int()));
	    case 11:
	        tf->rightMarginSet(PIXELS_TO_TWIPS(fn.arg(10).to_int()));
	    case 10:
	        tf->leftMarginSet(PIXELS_TO_TWIPS(fn.arg(9).to_int()));
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
	        tf->sizeSet(PIXELS_TO_TWIPS(fn.arg(1).to_int()));
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
TextFormat::display_getset(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl("TextField.display") );
	return as_value();
}

as_value
TextFormat::bullet_getset(const fn_call& fn)
{
    // Has the right return values, but not properly implemented
	LOG_ONCE( log_unimpl("TextFormat.bullet") );

	boost::intrusive_ptr<TextFormat> ptr = ensureType<TextFormat>(fn.this_ptr);

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
TextFormat::tabStops_getset(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl("TextField.tabStops") );
	return as_value();
}

as_value
TextFormat::blockIndent_getset(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat> ptr = ensureType<TextFormat>(fn.this_ptr);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( ptr->blockIndentDefined() ) ret.set_double(TWIPS_TO_PIXELS(ptr->blockIndent()));
		else ret.set_null();
	}
	else // setter
	{
		ptr->blockIndentSet(PIXELS_TO_TWIPS(fn.arg(0).to_int()));
	}

	return ret;
}

as_value
TextFormat::leading_getset(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat> ptr = ensureType<TextFormat>(fn.this_ptr);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( ptr->leadingDefined() ) ret.set_double(TWIPS_TO_PIXELS(ptr->leading()));
		else ret.set_null();
	}
	else // setter
	{
		ptr->leadingSet(PIXELS_TO_TWIPS(fn.arg(0).to_int()));
	}

	return ret;
}

as_value
TextFormat::indent_getset(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat> ptr = ensureType<TextFormat>(fn.this_ptr);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( ptr->indentDefined() ) ret.set_double(TWIPS_TO_PIXELS(ptr->indent()));
		else ret.set_null();
	}
	else // setter
	{
		ptr->indentSet(PIXELS_TO_TWIPS(fn.arg(0).to_int()));
	}

	return ret;
}

as_value
TextFormat::rightMargin_getset(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat> ptr = ensureType<TextFormat>(fn.this_ptr);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( ptr->rightMarginDefined() ) ret.set_double(TWIPS_TO_PIXELS(ptr->rightMargin()));
		else ret.set_null();
	}
	else // setter
	{
		ptr->rightMarginSet(PIXELS_TO_TWIPS(fn.arg(0).to_int()));
	}

	return ret;
}

as_value
TextFormat::leftMargin_getset(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat> ptr = ensureType<TextFormat>(fn.this_ptr);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( ptr->leftMarginDefined() ) ret.set_double(TWIPS_TO_PIXELS(ptr->leftMargin()));
		else ret.set_null();
	}
	else // setter
	{
		ptr->leftMarginSet(PIXELS_TO_TWIPS(fn.arg(0).to_int()));
	}

	return ret;
}

as_value
TextFormat::align_getset(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat> ptr = ensureType<TextFormat>(fn.this_ptr);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( ptr->alignDefined() ) ret.set_string(ptr->getAlignString(ptr->align()));
		else ret.set_null();
	}
	else // setter
	{
		ptr->alignSet(fn.arg(0).to_string());
	}

	return ret;
}

as_value
TextFormat::underline_getset(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat> ptr = ensureType<TextFormat>(fn.this_ptr);

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
TextFormat::italic_getset(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat> ptr = ensureType<TextFormat>(fn.this_ptr);

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
TextFormat::bold_getset(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat> ptr = ensureType<TextFormat>(fn.this_ptr);

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
TextFormat::target_getset(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl("TextField.target") );
	return as_value();
}

as_value
TextFormat::url_getset(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl("TextField.url") );
	return as_value();
}

as_value
TextFormat::color_getset(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat> ptr = ensureType<TextFormat>(fn.this_ptr);

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
TextFormat::size_getset(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat> ptr = ensureType<TextFormat>(fn.this_ptr);

	as_value ret;

	if ( fn.nargs == 0 ) // getter
	{
		if ( ptr->sizeDefined() ) ret.set_double(TWIPS_TO_PIXELS(ptr->size()));
		else ret.set_null();
	}
	else // setter
	{
		ptr->sizeSet(PIXELS_TO_TWIPS(fn.arg(0).to_int()));
	}

	return ret;
}

as_value
TextFormat::font_getset(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat> ptr = ensureType<TextFormat>(fn.this_ptr);

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
TextFormat::getTextExtent_method(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl("TextField.getTextExtent") );
	return as_value();
}

static void
attachTextFormatInterface(as_object& o)
{
	int flags = 0; // for sure we want to enum, dunno about deleting yet

	o.init_property("display", &TextFormat::display_getset, &TextFormat::display_getset, flags);
	o.init_property("bullet", &TextFormat::bullet_getset, &TextFormat::bullet_getset, flags);
	o.init_property("tabStops", &TextFormat::tabStops_getset, &TextFormat::tabStops_getset, flags);
	o.init_property("blockIndent", &TextFormat::blockIndent_getset, &TextFormat::blockIndent_getset, flags);
	o.init_property("leading", &TextFormat::leading_getset, &TextFormat::leading_getset, flags);
	o.init_property("indent", &TextFormat::indent_getset, &TextFormat::indent_getset, flags);
	o.init_property("rightMargin", &TextFormat::rightMargin_getset, &TextFormat::rightMargin_getset, flags);
	o.init_property("leftMargin", &TextFormat::leftMargin_getset, &TextFormat::leftMargin_getset, flags);
	o.init_property("align", &TextFormat::align_getset, &TextFormat::align_getset, flags);
	o.init_property("underline", &TextFormat::underline_getset, &TextFormat::underline_getset, flags);
	o.init_property("italic", &TextFormat::italic_getset, &TextFormat::italic_getset, flags);
	o.init_property("bold", &TextFormat::bold_getset, &TextFormat::bold_getset, flags);
	o.init_property("target", &TextFormat::target_getset, &TextFormat::target_getset, flags);
	o.init_property("url", &TextFormat::url_getset, &TextFormat::url_getset, flags);
	o.init_property("color", &TextFormat::color_getset, &TextFormat::color_getset, flags);
	o.init_property("size", &TextFormat::size_getset, &TextFormat::size_getset, flags);
	o.init_property("font", &TextFormat::font_getset, &TextFormat::font_getset, flags);
}

static as_object*
getTextFormatInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object(getObjectInterface());
		attachTextFormatInterface(*o);
	}
	return o.get();
}

// extern (used by Global.cpp)
void textformat_class_init(as_object& global)
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

edit_text_character_def::alignment
TextFormat::parseAlignString(const std::string& align)
{
	StringNoCaseEqual cmp;
	if ( cmp(align, "left") ) return edit_text_character_def::ALIGN_LEFT;
	else if ( cmp(align, "center") ) return edit_text_character_def::ALIGN_CENTER;
	else if ( cmp(align, "right") ) return edit_text_character_def::ALIGN_RIGHT;
	else if ( cmp(align, "justify") ) return edit_text_character_def::ALIGN_JUSTIFY;
	else
	{
		log_debug("Invalid align string %s, take as left", align);
		return edit_text_character_def::ALIGN_LEFT;
	}
}

const char* 
TextFormat::getAlignString(edit_text_character_def::alignment a)
{
	switch (a)
	{
		case edit_text_character_def::ALIGN_LEFT:
			return "left";
		case edit_text_character_def::ALIGN_CENTER:
			return "center";
		case edit_text_character_def::ALIGN_RIGHT:
			return "right";
		case edit_text_character_def::ALIGN_JUSTIFY:
			return "justify";
		default:
			log_error("Uknown alignment value: %d, take as left", a);
			return "left";
	}
}


} // end of gnash namespace
