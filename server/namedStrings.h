// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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

// A file to contain all of the different strings for which we want compile time
// known string table keys.
#ifndef GNASH_NAMED_STRINGS_H
#define GNASH_NAMED_STRINGS_H

namespace gnash {

class string_table; // Forward

/// Named String Values
//
/// These are enumerations of the strings which should have known string
/// table values.  They are the first strings added to the string table,
/// and the key will be equal to the enumeration.
///
/// Lowercase letters in the enum value signal the format of the string
/// literals associated with these enums.
/// i: Initial capital for groups which are normally initial lower case.
/// For example: PROP_iSTAGE is "Stage";
/// u: An underscore
/// For example: PROP_uuPROTOuu is "__proto__"
/// _: The next letter is capitalized
/// For example: PROP_ON_LOAD is "onLoad"
///
/// Names beginning with PROP_ have a lowercase initial letter
/// Names beginning with CLASS_ have an uppercase initial letter
/// Names beginning with NS_ have a lowercase initial letter and _ are
///  . instead of uppercase.
///
/// Names beginning with INTERNAL_ have no named string -- they can only
/// be used if you know their key value already.
namespace NSV {

typedef enum {
		PROP_ADD_LISTENER = 1,
		PROP_ALIGN,
		PROP_uALPHA,
		PROP_BLOCK_INDENT,
		PROP_BOLD,
		PROP_BROADCAST_MESSAGE,
		PROP_BULLET,
		PROP_CALLEE,
		PROP_COLOR,
		PROP_CONSTRUCTOR,
		PROP_uuCONSTRUCTORuu,
		PROP_uCURRENTFRAME,
		PROP_uDROPTARGET,
		PROP_ENABLED,
		PROP_uFOCUSRECT,
		PROP_uFRAMESLOADED,
		PROP_uHEIGHT,
		PROP_uHIGHQUALITY,
		PROP_HTML_TEXT,
		PROP_INDENT,
		PROP_ITALIC,
		PROP_LEADING,
		PROP_LEFT_MARGIN,
		PROP_LENGTH,
		PROP_uLISTENERS,
		PROP_LOADED,
		PROP_uNAME,
		PROP_ON_LOAD,
		PROP_ON_RESIZE,
		PROP_ON_ROLL_OUT,
		PROP_ON_ROLL_OVER,
		PROP_ON_SELECT,
		PROP_ON_STATUS,
		PROP_ON_META_DATA,
		PROP_uPARENT,
		PROP_uROOT,
		PROP_uGLOBAL,
		PROP_uuPROTOuu,
		PROP_PROTOTYPE,
		PROP_PUSH,
		PROP_REMOVE_LISTENER,
		PROP_RIGHT_MARGIN,
		PROP_uROTATION,
		PROP_SCALE_MODE,
		PROP_SIZE,
		PROP_uSOUNDBUFTIME,
		PROP_SPLICE,
		PROP_SUPER,
		PROP_iSTAGE,
		CLASS_STAGE = PROP_iSTAGE,
		PROP_STATUS,
		PROP_uTARGET,
		PROP_TEXT,
		PROP_TEXT_COLOR,
		PROP_TEXT_WIDTH,
		PROP_TO_STRING,
		PROP_uTOTALFRAMES,
		PROP_UNDERLINE,
		PROP_uURL,
		PROP_VALUE_OF,
		PROP_uVISIBLE,
		PROP_uWIDTH,
		PROP_X,
		PROP_uX,
		PROP_uXMOUSE,
		PROP_uXSCALE,
		PROP_Y,
		PROP_uY,
		PROP_uYMOUSE,
		PROP_uYSCALE,
		CLASS_SYSTEM,
		CLASS_MOVIE_CLIP,
		CLASS_TEXT_FIELD,
		CLASS_MATH,
		CLASS_BOOLEAN,
		CLASS_COLOR,
		CLASS_SELECTION,
		CLASS_SOUND,
		CLASS_X_M_L_SOCKET,
		CLASS_DATE,
		CLASS_X_M_L,
		CLASS_X_M_L_NODE,
		CLASS_MOUSE,
		CLASS_OBJECT,
		CLASS_NUMBER,
		CLASS_STRING,
		CLASS_ARRAY,
		CLASS_KEY,
		CLASS_AS_BROADCASTER,
		CLASS_FUNCTION,
		CLASS_TEXT_SNAPSHOT,
		CLASS_VIDEO,
		CLASS_CAMERA,
		CLASS_MICROPHONE,
		CLASS_SHARED_OBJECT,
		CLASS_LOAD_VARS,
		CLASS_CUSTOM_ACTIONS,
		CLASS_NET_CONNECTION,
		CLASS_NET_STREAM,
		CLASS_CONTEXT_MENU,
		CLASS_MOVIE_CLIP_LOADER,
		CLASS_ERROR,
		NS_FLASH_DISPLAY,
		NS_FLASH_TEXT,
		NS_FLASH_GEOM,
		NS_FLASH_NET,
		NS_FLASH_SYSTEM,
		NS_FLASH_UTILS,
		NS_FLASH_EVENTS,
		NS_FLASH_ACCESSIBILITY,
		NS_FLASH_MEDIA,
		NS_FLASH_XML,
		NS_FLASH_UI,
		NS_ADOBE_UTILS,
		INTERNAL_TYPE, // The type name
		INTERNAL_STACK_PARENT, // Any public property is unsafe
		INTERNAL_INTERFACES
	} named_strings;

/// Load the prenamed strings.
/// version controls case
void load_strings(string_table *table, int version);

} // namespace NSV
} // namespace gnash

#endif // GNASH_NAMED_STRINGS_H

