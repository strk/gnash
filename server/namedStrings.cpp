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

#include "namedStrings.h"
#include "string_table.h"

namespace gnash {
namespace NSV { // Named String Values

// Load up our pre-known names
static string_table::svt preload_names[] =
{
	{ "addListener", NSV::PROP_ADD_LISTENER },
	{ "align", NSV::PROP_ALIGN },
	{ "_alpha", NSV::PROP_uALPHA },
	{ "blockIndent", NSV::PROP_BLOCK_INDENT },
	{ "bold", NSV::PROP_BOLD },
	{ "broadcastMessage", NSV::PROP_BROADCAST_MESSAGE },
	{ "bullet", NSV::PROP_BULLET },
	{ "callee", NSV::PROP_CALLEE },
	{ "color", NSV::PROP_COLOR },
	{ "constructor", NSV::PROP_CONSTRUCTOR },
	{ "__constructor__", NSV::PROP_uuCONSTRUCTORuu },
	{ "_currentframe", NSV::PROP_uCURRENTFRAME },
	{ "_droptarget", NSV::PROP_uDROPTARGET },
	{ "enabled", NSV::PROP_ENABLED },
	{ "_focusrect", NSV::PROP_uFOCUSRECT },
	{ "_framesloaded", NSV::PROP_uFRAMESLOADED },
	{ "_height", NSV::PROP_uHEIGHT },
	{ "_highquality", NSV::PROP_uHIGHQUALITY },
	{ "htmlText", NSV::PROP_HTML_TEXT },
	{ "indent", NSV::PROP_INDENT },
	{ "italic", NSV::PROP_ITALIC },
	{ "leading", NSV::PROP_LEADING },
	{ "left_margin", NSV::PROP_LEFT_MARGIN },
	{ "length", NSV::PROP_LENGTH },
	{ "_listeners", NSV::PROP_uLISTENERS },
	{ "loaded", NSV::PROP_LOADED },
	{ "_name", NSV::PROP_uNAME },
	{ "onLoad", NSV::PROP_ON_LOAD },
	{ "onResize", NSV::PROP_ON_RESIZE },
	{ "onRollOut", NSV::PROP_ON_ROLL_OUT },
	{ "onRollOver", NSV::PROP_ON_ROLL_OVER },
	{ "onSelect", NSV::PROP_ON_SELECT },
	{ "onStatus", NSV::PROP_ON_STATUS },
	{ "_parent", NSV::PROP_uPARENT },
	{ "__proto__", NSV::PROP_uuPROTOuu },
	{ "prototype", NSV::PROP_PROTOTYPE },
	{ "push", NSV::PROP_PUSH },
	{ "removeListener", NSV::PROP_REMOVE_LISTENER },
	{ "rightMargin", NSV::PROP_RIGHT_MARGIN },
	{ "_rotation", NSV::PROP_uROTATION },
	{ "scaleMode", NSV::PROP_SCALE_MODE },
	{ "size", NSV::PROP_SIZE },
	{ "_soundbuftime", NSV::PROP_uSOUNDBUFTIME },
	{ "splice", NSV::PROP_SPLICE },
	{ "Stage", NSV::PROP_iSTAGE },
	{ "status", NSV::PROP_STATUS },
	{ "super", NSV::PROP_SUPER },
	{ "_target", NSV::PROP_uTARGET },
	{ "text", NSV::PROP_TEXT },
	{ "textColor", NSV::PROP_TEXT_COLOR },
	{ "textWidth", NSV::PROP_TEXT_WIDTH },
	{ "toString", NSV::PROP_TO_STRING },
	{ "_totalframes", NSV::PROP_uTOTALFRAMES },
	{ "underline", NSV::PROP_UNDERLINE },
	{ "_url", NSV::PROP_uURL },
	{ "valueOf", NSV::PROP_VALUE_OF },
	{ "_visible", NSV::PROP_uVISIBLE },
	{ "_width", NSV::PROP_uWIDTH },
	{ "x", NSV::PROP_X },
	{ "_x", NSV::PROP_uX },
	{ "_xmouse", NSV::PROP_uXMOUSE },
	{ "_xscale", NSV::PROP_uXSCALE },
	{ "y", NSV::PROP_Y },
	{ "_y", NSV::PROP_uY },
	{ "_ymouse", NSV::PROP_uYMOUSE },
	{ "_yscale", NSV::PROP_uYSCALE },
	{ "System", NSV::CLASS_SYSTEM },
//	{ "Stage", NSV::CLASS_STAGE }, // Identical to PROP_iSTAGE
	{ "MovieClip", NSV::CLASS_MOVIE_CLIP },
	{ "TextField", NSV::CLASS_TEXT_FIELD },
	{ "Math", NSV::CLASS_MATH },
	{ "Boolean", NSV::CLASS_BOOLEAN },
	{ "Color", NSV::CLASS_COLOR },
	{ "Selection", NSV::CLASS_SELECTION },
	{ "Sound", NSV::CLASS_SOUND },
	{ "XMLSocket", NSV::CLASS_X_M_L_SOCKET },
	{ "Date", NSV::CLASS_DATE },
	{ "XML", NSV::CLASS_X_M_L },
	{ "XMLNode", NSV::CLASS_X_M_L_NODE },
	{ "Mouse", NSV::CLASS_MOUSE },
	{ "Object", NSV::CLASS_OBJECT },
	{ "String", NSV::CLASS_STRING },
	{ "Number", NSV::CLASS_NUMBER },
	{ "Array", NSV::CLASS_ARRAY },
	{ "Key", NSV::CLASS_KEY },
	{ "AsBroadcaster", NSV::CLASS_AS_BROADCASTER },
	{ "Function", NSV::CLASS_FUNCTION },
	{ "TextSnapshot", NSV::CLASS_TEXT_SNAPSHOT },
	{ "Video", NSV::CLASS_VIDEO },
	{ "Camera", NSV::CLASS_CAMERA },
	{ "Microphone", NSV::CLASS_MICROPHONE },
	{ "SharedObject", NSV::CLASS_SHARED_OBJECT },
	{ "LoadVars", NSV::CLASS_LOAD_VARS },
	{ "CustomActions", NSV::CLASS_CUSTOM_ACTIONS },
	{ "NetConnection", NSV::CLASS_NET_CONNECTION },
	{ "NetStream", NSV::CLASS_NET_STREAM },
	{ "ContextMenu", NSV::CLASS_CONTEXT_MENU },
	{ "MovieClipLoader", NSV::CLASS_MOVIE_CLIP_LOADER },
	{ "Error", NSV::CLASS_ERROR },
	{ "flash.display", NSV::NS_FLASH_DISPLAY },
	{ "flash.text", NSV::NS_FLASH_TEXT },
	{ "flash.geom", NSV::NS_FLASH_GEOM },
	{ "flash.net", NSV::NS_FLASH_NET },
	{ "flash.system", NSV::NS_FLASH_SYSTEM },
	{ "flash.utils", NSV::NS_FLASH_UTILS },
	{ "flash.events", NSV::NS_FLASH_EVENTS },
	{ "flash.accessibility", NSV::NS_FLASH_ACCESSIBILITY },
	{ "flash.media", NSV::NS_FLASH_MEDIA },
	{ "flash.xml", NSV::NS_FLASH_XML },
	{ "flash.ui", NSV::NS_FLASH_UI },
	{ "adobe.utils", NSV::NS_ADOBE_UTILS },
	{ "", NSV::INTERNAL_TYPE },
	{ "", NSV::INTERNAL_STACK_PARENT },
	{ "", NSV::INTERNAL_INTERFACES }
};

void load_strings(string_table *table, int version)
{
	if (version < 7)
		table->lower_next_group();

	table->insert_group(preload_names,
		sizeof (preload_names) / sizeof (string_table::svt));
}

} // namespace NSV
} // namespace gnash
