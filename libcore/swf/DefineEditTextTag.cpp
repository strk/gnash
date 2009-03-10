// 
//   Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include "DefineEditTextTag.h"
#include "TextField.h"
#include "movie_definition.h"
#include "Font.h"
#include "SWFStream.h"

namespace gnash {
namespace SWF {

void
DefineEditTextTag::loader(SWFStream& in, TagType tag, movie_definition& m,
        const RunInfo& /*r*/)
{
	assert(tag == SWF::DEFINEEDITTEXT); // 37

    in.ensureBytes(2);
    boost::uint16_t characterID = in.read_u16();

    std::auto_ptr<DefineEditTextTag> editText(new DefineEditTextTag(in, m));

    m.add_character(characterID, editText.release());
}

character*
DefineEditTextTag::createDisplayObject(character* parent, int id)
{
	// Resolve the font, if possible
	getFont();
	TextField* ch = new TextField(parent, *this, id);

	// This gives an "instance name" to the TextField, but
	// it is not really what we need.
	//
	// First of all the VariableName ("_variableName") is
	// NOT the default name of an instance, rather it is
	// a variable associated with it and can contain path
	// information (ie. we can associate a variable in a different
	// timeline)
	//
	// We actually need to set that variable to an object which
	// is a TextField dinamic variable. The object should take
	// care of updating this TextField text when assigned to
	// and to retrive this TextField text when extracted value from.
	//
	// The DefineEditTextVariableNameTest.swf file under
	// testsuite/misc-ming.all gives an idea of the problem
	// (in particular it shows a case in which VariableName is
	// outside of TextField timeline/scope)
	//
	//ch->set_name(m_variable_name.c_str());

	return ch;
}


void
DefineEditTextTag::read(SWFStream& in, movie_definition& m)
{

	_rect.read(in);

	in.align();
	in.ensureBytes(2);
    
    int flags = in.read_u8();
	_hasText  = flags & (1 << 7);
	_wordWrap = flags & (1 << 6);
	_multiline = flags & (1 << 5);
	_password  = flags & (1 << 4);
	_readOnly  = flags & (1 << 3); 

    bool hasColor = flags & (1 << 2); 
	bool hasMaxChars = flags & (1 << 1); 
	bool hasFont = flags & (1 << 0); 

    flags = in.read_u8();
	// 0: no font class, 1 font class and height, can't be true if has_font was true
	bool hasFontClass = flags & (1 << 7);
	if (hasFontClass && hasFont )
	{
		IF_VERBOSE_MALFORMED_SWF(
		    log_swferror("DefineEditText: hasFontClass can't be true if "
                "hasFont is true, ignoring");
		);
		hasFontClass = false;
	}

	_autoSize = flags & (1 << 6); 
	bool hasLayout = flags & (1 << 5); 
	_noSelect = flags & (1 << 4); 
	_border = flags & (1 << 3);

    // authored as static text (not dynamic text)
	bool wasStatic = flags & (1 << 2);

    // TODO: what is this for?
    UNUSED(wasStatic);

    _html = flags & (1 << 1); 
	_useOutlines = flags & (1 << 0); 

	if (hasFont)
	{
		in.ensureBytes(4);
		_fontID = in.read_u16();
		_font = m.get_font(_fontID);
		if (!_font)
		{
            IF_VERBOSE_MALFORMED_SWF(
			    log_swferror("DefineEditText: tag refers to unknown font "
                    "id %d", _fontID);
			);
		}
        _textHeight = in.read_u16();
	}
	else if (hasFontClass)
	{
		std::string fontClassName;
		in.read_string(fontClassName);
		log_unimpl("Font class support for DefineEditText (%d)",
                fontClassName);
	}
	
	if (hasColor)
	{
		_color.read_rgba(in);
	}

	if (hasMaxChars)
	{
		in.ensureBytes(2);
		_maxChars = in.read_u16();
	}

	if (hasLayout)
	{
		in.ensureBytes(9); //1 + 2 + 2 + 2 + 2
		_alignment = static_cast<TextField::TextAlignment>(in.read_u8());
		_leftMargin = in.read_u16();
		_rightMargin = in.read_u16();
		_indent = in.read_s16();
		_leading = in.read_s16();
	}

	in.read_string(_variableName);

	if (_hasText)
	{
		in.read_string(_defaultText);
	}

	IF_VERBOSE_PARSE (
		log_parse("edit_text_char:\n"
			" varname = %s\n"
			" text = \"%s\"\n"
			" font_id: %d\n"
			" text_height: %d",
			_variableName, _defaultText, _fontID, _textHeight);
	);
}

DefineEditTextTag::DefineEditTextTag(SWFStream& in, movie_definition& m)
    :
	_hasText(true),
	_wordWrap(false),
	_multiline(false),
	_password(false),
	_readOnly(true),
	_autoSize(false),
	_noSelect(false),
	_border(false),
	_html(false),
	_useOutlines(false),
	_fontID(-1),
	_textHeight(1),
	_maxChars(0),
	_alignment(TextField::ALIGN_LEFT),
	_leftMargin(0),
	_rightMargin(0),
	_indent(0),
	_leading(0)
{
	_color.set(0, 0, 0, 255);
    
    // Parse the tag from the stream.
    read(in, m);
}

}
}
