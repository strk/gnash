// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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

#ifndef GNASH_SWF_DEFINEEDITTEXTTAG_H
#define GNASH_SWF_DEFINEEDITTEXTTAG_H

#include "SWFRect.h"
#include "DefinitionTag.h"
#include "SWF.h" // for TagType definition
#include "RGBA.h"
#include "TextField.h"

#include <boost/intrusive_ptr.hpp>
#include <string>
#include <boost/cstdint.hpp> // for boost::uint16_t and friends
 

// Forward declarations
namespace gnash {
	class SWFStream;
	class movie_definition;
	class RunResources;
    class Font;
}

namespace gnash {
namespace SWF {

/// SWF Tag DefineEditText (37).
//
///
/// Virtual control tag for syncing streaming sound to playhead
///
/// Gnash will register instances of this ControlTag in the 
/// frame containing blocks of a streaming sound, which is
/// occurrences of SWF Tag StreamSoundBlock (19).
///
/// The tag will then be used to start playing the specific block
/// in sync with the frame playhead.
///
class DefineEditTextTag : public DefinitionTag
{

public:

    ~DefineEditTextTag() {}

	/// Load an SWF::DEFINEEDITTEXT (37) tag.
	static void loader(SWFStream& in, TagType tag, movie_definition& m,
            const RunResources& r);

    const SWFRect& bounds() const { return _rect; }

    DisplayObject* createDisplayObject(Global_as& gl,
            DisplayObject* parent) const;

	/// Return a reference to the default text associated
	/// with this EditText definition.
	const std::string& defaultText() const {
		return _defaultText;
	}

	/// Return a reference to the "VariableName" associated
	/// with this EditText definition. The variable name
	/// is allowed to contain path information and should
	/// be used to provide an 'alias' to the 'text' member
	/// of instances.
	const std::string& variableName() const {
		return _variableName;
	}

	/// Return the maximum length of text this widget can hold.
	//
	/// If zero, the text length is unlimited.
	///
	unsigned int maxChars() const {
		return _maxChars;
	}

	/// Get right margin in twips
	boost::uint16_t rightMargin() const {
		return _rightMargin;
	}

	/// Get left margin in twips
	boost::uint16_t leftMargin() const {
		return _leftMargin;
	}

	/// Get indentation in  twips
	boost::uint16_t indent() const {
		return _indent;
	}

	/// Get height of font  in twips.
	// @@ what if has_font is false ??
	boost::uint16_t textHeight() const {
		return _textHeight;
	}

	/// Get color of the text
	const rgba& color() const {
		return _color;
	}

	/// \brief
	/// Get extra space between lines (in twips).
	//
	/// This is in addition to default font line spacing.
	boost::uint16_t leading() const {
		return _leading;
	}

    bool multiline() const {
        return _multiline;
    }

    bool password() const {
        return _password;
    }

	/// Get text alignment
    TextField::TextAlignment alignment() const {
		return _alignment;
	}

	/// Is border requested ?
	bool border() const {
		return _border;
	}

    bool autoSize() const {
        return _autoSize;
    }

	/// Word wrap requested ?
	bool wordWrap() const {
		return _wordWrap;
	}

	/// Has text defined ?
	bool hasText() const {
		return _hasText;
	}

	bool readOnly() const
	{
		return _readOnly;
	}

	bool noSelect() const 
	{
	     return _noSelect;
	}
	
	/// Return true if HTML was allowed by definition
	bool html() const { return _html; }

	/// Return true if this DisplayObject definition requested use of device fonts
	// 
	/// Used by TextFielf constructor to set its default.
	///
	bool getUseEmbeddedGlyphs() const 
	{
		return _useOutlines;
	}

    boost::intrusive_ptr<Font> getFont() const
    {
        return _font;
    }

private:

    /// Construct a DefineEditTextTag.
    //
    /// This should only be used from the loader() function.
	DefineEditTextTag(SWFStream& in, movie_definition& m, boost::uint16_t id);

    /// Read a tag from the SWFStream.
    void read(SWFStream& in, movie_definition& m);

	SWFRect _rect;

	std::string _variableName;

    // For an SWF-defined textfield we'll read
    // this from the tag. Dynamic textfields should
    // behave as always having text by default (not tested).
	bool _hasText;
	bool _wordWrap;
	bool _multiline;

    /// show asterisks instead of actual DisplayObjects
	bool _password;
	bool _readOnly;
	/// resize our bound to fit the text
	bool _autoSize;
	bool _noSelect;

	/// forces white background and black border.
	/// silly, but sometimes used
	bool _border;

	/// Allowed HTML (from Alexis SWF Reference).
	//
	/// <a href=url target=targ>...</a> -- hyperlink
	/// <b>...</b> -- bold
	/// <br> -- line break
	/// <font face=name size=[+|-][0-9]+ color=#RRGGBB>...</font>  -- font change; size in TWIPS
	/// <i>...</i> -- italic
	/// <li>...</li> -- list item
	/// <p>...</p> -- paragraph
	/// <tab> -- insert tab
	/// <TEXTFORMAT>  </TEXTFORMAT>
	///   [ BLOCKINDENT=[0-9]+ ]
	///   [ INDENT=[0-9]+ ]
	///   [ LEADING=[0-9]+ ]
	///   [ LEFTMARGIN=[0-9]+ ]
	///   [ RIGHTMARGIN=[0-9]+ ]
	///   [ TABSTOPS=[0-9]+{,[0-9]+} ]
	///
	/// Change the different parameters as indicated. The
	/// sizes are all in TWIPs. There can be multiple
	/// positions for the tab stops. These are seperated by
	/// commas.
	/// <U>...</U> -- underline
	///
	bool _html;

	/// \brief
	/// When true, use specified SWF internal font (embed fonts)
	/// Otherwise, use specified device font (or a default one if m_font_id is -1)
	///
	/// Also known as USE_GLYPH (from Ming)
	///
    // For an SWF-defined textfield we'll read
    // this from the tag. Dynamic textfields should
	// use device fonts by default (so not use outline ones)
	bool _useOutlines;

	int	_fontID;
	boost::intrusive_ptr<Font> _font;

	/// height of font text, in twips
    // TODO: initialize to a meaningful value (see MovieClip::add_textfield)
    //       and make sure get_font_height is not called for rendering purposes
    //       (instead call a method of TextField) (?)
	boost::uint16_t _textHeight;

	/// Text color
	rgba _color;

	/// Maximum length of text this widget can display (number of chars?)
	//
	/// If zero, the text length is unlimited.
	///
	unsigned int _maxChars;

    TextField::TextAlignment _alignment;
	
	/// extra space between box's left border and text (in twips)
	boost::uint16_t _leftMargin;

	/// extra space between box's right border and text (in twips)
	boost::uint16_t _rightMargin;

	/// how much to indent the first line of multiline text (in twips)
	boost::uint16_t	_indent;

	/// \brief
	/// Extra space between lines
	/// (in addition to default font line spacing)
	boost::uint16_t	_leading;

	/// The default text to be displayed
	std::string	_defaultText;
};


} // namespace gnash::SWF
} // namespace gnash

#endif
