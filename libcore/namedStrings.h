// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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

/// All known lower-case names are grouped at the beginning
/// of the enum, followed by INTERNAL_HIGHEST_LOWERCASE.
/// WARNING: putting mixed-case names before
///          INTERNAL_HIGHEST_LOWERCASE introduces bugs !
/// (putting lower-case names after INTERNAL_HIGHEST_LOWERCASE
///  is fine, except for performance)
///
enum NamedStrings {
        PROP_A = 1,
        PROP_ALIGN,
        PROP_ARGUMENTS,
        PROP_B,
        PROP_BOLD,
        PROP_C,
        PROP_CALLEE,
        PROP_CALLER,
        PROP_COLOR,
        PROP_CONCAT,
        PROP_CONSTRUCTOR,
        PROP_D,
        PROP_DATA,
        PROP_DECODE,
        PROP_E,
        PROP_ENABLED,
        PROP_ESCAPE,
        PROP_G,
        PROP_H,
        PROP_HEIGHT,
        PROP_INDENT,
        PROP_ITALIC,
        PROP_LEADING,
        PROP_LEFT_MARGIN,
        PROP_LENGTH,
        PROP_LOADED,
        PROP_METH,
        PROP_PROTOTYPE,
        PROP_PUSH,
        PROP_R,
        PROP_SIZE,
        PROP_SPLICE,
        PROP_STATUS,
        PROP_SUPER,
        PROP_TARGET,
        PROP_TEXT,
        PROP_THIS,
        PROP_TX,
        PROP_TY,
        PROP_uALPHA,
        PROP_uCURRENTFRAME,
        PROP_uDROPTARGET,
        PROP_uFOCUSRECT,
        PROP_uFRAMESLOADED,
        PROP_uGLOBAL,
        PROP_uHEIGHT,
        PROP_uHIGHQUALITY,
        PROP_uLISTENERS,
        PROP_uNAME,
        PROP_UNDERLINE,
        PROP_uPARENT,
        PROP_uQUALITY,
        PROP_uROOT,
        PROP_uROTATION,
        PROP_uSOUNDBUFTIME,
        PROP_uTARGET,
        PROP_uTOTALFRAMES,
        PROP_uuCONSTRUCTORuu,
        PROP_uuPROTOuu,
        PROP_uuRESOLVE,
        PROP_uURL,
        PROP_uVISIBLE,
        PROP_uWIDTH,
        PROP_uX,
        PROP_uXMOUSE,
        PROP_uXSCALE,
        PROP_uY,
        PROP_uYMOUSE,
        PROP_uYSCALE,
        PROP_W,
        PROP_WIDTH,
        PROP_X,
        PROP_Y,
        INTERNAL_HIGHEST_LOWERCASE,

        PROP_ADD_LISTENER,
        PROP_AS_NATIVE,
        PROP_AS_SET_PROP_FLAGS,
        PROP_BLOCK_INDENT,
        PROP_BROADCAST_MESSAGE,
        PROP_BULLET,
        PROP_uBYTES_TOTAL,
        PROP_uBYTES_LOADED,
        PROP_CONTENT_TYPE,
        PROP_FOCUS_ENABLED,
        PROP_HTML_TEXT,
        PROP_MATRIX_TYPE,
        PROP_ON_CLOSE,
        PROP_ON_CONNECT,
        PROP_ON_CONSTRUCT,
        PROP_ON_DATA,
        PROP_ON_DRAG_OUT,
        PROP_ON_DRAG_OVER,
        PROP_ON_ENTER_FRAME,
        PROP_ON_FULLSCREEN,
        PROP_ON_INITIALIZE,
        PROP_ON_KEY_DOWN,
        PROP_ON_KEY_PRESS,
        PROP_ON_KEY_UP,
        PROP_ON_KILL_FOCUS,
        PROP_ON_LOAD,
        PROP_ON_LOAD_ERROR,
        PROP_ON_LOAD_INIT,
        PROP_ON_LOAD_PROGRESS,
        PROP_ON_LOAD_START,
        PROP_ON_META_DATA,
        PROP_ON_MOUSE_DOWN,
        PROP_ON_MOUSE_MOVE,
        PROP_ON_MOUSE_UP,
        PROP_ON_PRESS,
        PROP_ON_RELEASE,
        PROP_ON_RELEASE_OUTSIDE,
        PROP_ON_RESIZE,
        PROP_ON_RESULT,
        PROP_ON_ROLL_OUT,
        PROP_ON_ROLL_OVER,
        PROP_ON_SELECT,
        PROP_ON_SET_FOCUS,
        PROP_ON_SOUND_COMPLETE,
        PROP_ON_STATUS,
        PROP_ON_TIMER,
        PROP_ON_UNLOAD,
        PROP_ON_XML,
        PROP_PARSE_XML,
        PROP_REMOVE_LISTENER,
        PROP_RIGHT_MARGIN,
        PROP_SCALE_MODE,
        PROP_TEXT_COLOR,
        PROP_TEXT_HEIGHT,
        PROP_TEXT_WIDTH,
        PROP_TO_LOWER_CASE,
        PROP_TO_STRING,
        PROP_uCUSTOM_HEADERS,
        PROP_USEHANDCURSOR,
        PROP_VALUE_OF,
        PROP_ON_SYNC,
        CLASS_ACCESSIBILITY,
        CLASS_ANTIALIASTYPE,
        CLASS_ARRAY,
        CLASS_AS_BROADCASTER,
        CLASS_BITMAP,
        CLASS_BOOLEAN,
        CLASS_BUTTON,
        CLASS_CAMERA,
        CLASS_COLOR,
        CLASS_CONTEXTMENU,
        CLASS_CONTEXTMENUITEM,
        CLASS_CSMTEXTSETTINGS,
        CLASS_DATE,
        CLASS_DISPLAYOBJECT,
        CLASS_DISPLAYOBJECTCONTAINER,
        CLASS_ERROR,
        CLASS_EVENT,
        CLASS_EVENTDISPATCHER,
        CLASS_FONT,
        CLASS_FONTSTYLE,
        CLASS_FUNCTION,
        CLASS_GRIDFITTYPE,
        CLASS_INT,
        CLASS_INTERACTIVEOBJECT,
        CLASS_KEY,
        CLASS_KEYBOARD,
        CLASS_LOAD_VARS,
        CLASS_LOCALCONNECTION,
        CLASS_MATH,
        CLASS_MICROPHONE,
        CLASS_MOUSE,
        CLASS_MOVIE_CLIP,
        CLASS_MOVIE_CLIP_LOADER,
        CLASS_NAMESPACE,
        CLASS_NET_CONNECTION,
        CLASS_NET_STREAM,
        CLASS_NUMBER,
        CLASS_OBJECT,
        CLASS_QNAME,
        CLASS_SELECTION,
        CLASS_SHAPE,
        CLASS_SHARED_OBJECT,
        CLASS_SIMPLE_BUTTON,
        CLASS_SOUND,
        CLASS_SPRITE,
        CLASS_STAGE,
        CLASS_STATICTEXT,
        CLASS_STRING,
        CLASS_STYLESHEET,
        CLASS_SYSTEM,
        CLASS_TEXTCOLORTYPE,
        CLASS_TEXTDISPLAYMODE,
        CLASS_TEXT_FIELD,
        CLASS_TEXTFIELDTYPE,
        CLASS_TEXT_FORMAT,
        CLASS_TEXTFORMATALIGN,
        CLASS_TEXT_SNAPSHOT,
        CLASS_TEXTFIELDAUTOSIZE,
        CLASS_TEXTLINEMETRICS,
        CLASS_TEXTRENDERER,
        CLASS_VIDEO,
        CLASS_XML,
        CLASS_XML_DOCUMENT,
        CLASS_XMLNODE,
        CLASS_XMLSOCKET,
        NS_ADOBE_UTILS,
        NS_FLASH_ACCESSIBILITY,
        NS_FLASH_DISPLAY,
        NS_FLASH_EVENTS,
        NS_FLASH_ERRORS,
        NS_FLASH_GEOM,
        NS_FLASH_MEDIA,
        NS_FLASH_NET,
        NS_FLASH_SYSTEM,
        NS_FLASH_TEXT,
        NS_FLASH_UI,
        NS_FLASH_UTILS,
        NS_FLASH_XML,
        INTERNAL_INTERFACES,
        INTERNAL_STACK_PARENT, // Any public property is unsafe
        INTERNAL_TYPE // The type name
    };

/// Load the prenamed strings.
void loadStrings(string_table &table);

} // namespace NSV
} // namespace gnash

#endif // GNASH_NAMED_STRINGS_H

