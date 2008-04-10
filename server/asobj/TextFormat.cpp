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
#include "server/types.h" // for PIXELS_TO_TWIPS
#include "StringPredicates.h" // for parseAlignString

#define ONCE(x) { static bool warned=false; if (!warned) { warned=true; x; } }

namespace gnash {

static as_value textformat_new(const fn_call& fn);
static as_object* getTextFormatInterface();
static void attachTextFormatInterface(as_object& o);


TextFormat::TextFormat()
	:
	as_object(getTextFormatInterface()),
	_underline(false),
	_bold(false),
	_italic(false),
	_bullet(false),
	_block_indent(-1),
	_color(0),
	_indent(-1),
	_leading(-1),
	_left_margin(-1),
	_right_margin(-1),
	_point_size(-1),
	_tab_stops(-1),
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
	if ( fn.nargs > 0  ) {	tf->fontSet(fn.arg(0).to_string());
	if ( fn.nargs > 1  ) {	tf->sizeSet(fn.arg(1).to_int());
	if ( fn.nargs > 2  ) {	tf->colorSet(fn.arg(2).to_int()); // TODO: check this...
	if ( fn.nargs > 3  ) {	tf->boldSet(fn.arg(3).to_bool()); 
	if ( fn.nargs > 4  ) {	tf->italicedSet(fn.arg(4).to_bool()); 
	if ( fn.nargs > 5  ) {	tf->underlinedSet(fn.arg(5).to_bool()); 
	if ( fn.nargs > 6  ) {	tf->urlSet(fn.arg(6).to_string()); 
	if ( fn.nargs > 7  ) {	tf->targetSet(fn.arg(7).to_string()); 
	if ( fn.nargs > 8  ) {	tf->alignSet(fn.arg(8).to_string());
	if ( fn.nargs > 9  ) {	tf->leftMarginSet(fn.arg(9).to_int());
	if ( fn.nargs > 10 ) {	tf->rightMarginSet(fn.arg(10).to_int());
	if ( fn.nargs > 11 ) {	tf->indentSet(fn.arg(11).to_int());
	if ( fn.nargs > 12 ) {	tf->leadingSet(fn.arg(12).to_int());
	}}}}}}}}}}}}}

	return as_value(tf.get());
}

as_value
TextFormat::display_getset(const fn_call& /*fn*/)
{
	ONCE( log_unimpl("TextField.display") );
	return as_value();
}

as_value
TextFormat::bullet_getset(const fn_call& /*fn*/)
{
	ONCE( log_unimpl("TextField.bullet") );
	return as_value();
}

as_value
TextFormat::tabStops_getset(const fn_call& /*fn*/)
{
	ONCE( log_unimpl("TextField.tabStops") );
	return as_value();
}

as_value
TextFormat::blockIndent_getset(const fn_call& /*fn*/)
{
	ONCE( log_unimpl("TextField.blockIndent") );
	return as_value();
}

as_value
TextFormat::leading_getset(const fn_call& /*fn*/)
{
	ONCE( log_unimpl("TextField.leading") );
	return as_value();
}

as_value
TextFormat::indent_getset(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat> ptr = ensureType<TextFormat>(fn.this_ptr);

	if ( fn.nargs == 0 ) // getter
	{
		return as_value(TWIPS_TO_PIXELS(ptr->indent()));
	}
	else // setter
	{
		ptr->indentSet(PIXELS_TO_TWIPS(fn.arg(0).to_int()));
	}

	return as_value();
}

as_value
TextFormat::rightMargin_getset(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat> ptr = ensureType<TextFormat>(fn.this_ptr);

	if ( fn.nargs == 0 ) // getter
	{
		return as_value(TWIPS_TO_PIXELS(ptr->rightMargin()));
	}
	else // setter
	{
		ptr->rightMarginSet(PIXELS_TO_TWIPS(fn.arg(0).to_int()));
	}

	return as_value();
}

as_value
TextFormat::leftMargin_getset(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat> ptr = ensureType<TextFormat>(fn.this_ptr);

	if ( fn.nargs == 0 ) // getter
	{
		return as_value(TWIPS_TO_PIXELS(ptr->leftMargin()));
	}
	else // setter
	{
		ptr->leftMarginSet(PIXELS_TO_TWIPS(fn.arg(0).to_int()));
	}

	return as_value();
}

as_value
TextFormat::align_getset(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat> ptr = ensureType<TextFormat>(fn.this_ptr);

	if ( fn.nargs == 0 ) // getter
	{
		return as_value(ptr->getAlignString(ptr->align()));
	}
	else // setter
	{
		ptr->alignSet(fn.arg(0).to_string());
	}

	return as_value();
}

as_value
TextFormat::underline_getset(const fn_call& /*fn*/)
{
	ONCE( log_unimpl("TextField.underline") );
	return as_value();
}

as_value
TextFormat::italic_getset(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat> ptr = ensureType<TextFormat>(fn.this_ptr);

	if ( fn.nargs == 0 ) // getter
	{
		return as_value(ptr->italiced());
	}
	else // setter
	{
		ptr->italicedSet(fn.arg(0).to_bool());
	}

	return as_value();
}

as_value
TextFormat::bold_getset(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat> ptr = ensureType<TextFormat>(fn.this_ptr);

	if ( fn.nargs == 0 ) // getter
	{
		return as_value(ptr->bold());
	}
	else // setter
	{
		ptr->boldSet(fn.arg(0).to_bool());
	}

	return as_value();
}

as_value
TextFormat::target_getset(const fn_call& /*fn*/)
{
	ONCE( log_unimpl("TextField.target") );
	return as_value();
}

as_value
TextFormat::url_getset(const fn_call& /*fn*/)
{
	ONCE( log_unimpl("TextField.url") );
	return as_value();
}

as_value
TextFormat::color_getset(const fn_call& /*fn*/)
{
	ONCE( log_unimpl("TextField.color") );
	return as_value();
}

as_value
TextFormat::size_getset(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat> ptr = ensureType<TextFormat>(fn.this_ptr);

	if ( fn.nargs == 0 ) // getter
	{
		return as_value(TWIPS_TO_PIXELS(ptr->size()));
	}
	else // setter
	{
		ptr->sizeSet(PIXELS_TO_TWIPS(fn.arg(0).to_int()));
		ONCE( log_debug("TextField.size setter TESTING") );

	}

	return as_value();
}

as_value
TextFormat::font_getset(const fn_call& fn)
{
	boost::intrusive_ptr<TextFormat> ptr = ensureType<TextFormat>(fn.this_ptr);

	if ( fn.nargs == 0 ) // getter
	{
		return as_value(ptr->font());
	}
	else // setter
	{
		ptr->fontSet(fn.arg(0).to_string());
	}

	return as_value();
}

as_value
TextFormat::getTextExtent_method(const fn_call& /*fn*/)
{
	ONCE( log_unimpl("TextField.getTextExtent") );
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
	if ( cmp(align, "right") ) return edit_text_character_def::ALIGN_RIGHT;
	else if ( cmp(align, "center") ) return edit_text_character_def::ALIGN_CENTER;
	else if ( cmp(align, "right") ) return edit_text_character_def::ALIGN_RIGHT;
	else if ( cmp(align, "justify") ) return edit_text_character_def::ALIGN_JUSTIFY;
	else
	{
		log_debug("Invalid align string %s, take as left", align);
		return edit_text_character_def::ALIGN_JUSTIFY;
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
