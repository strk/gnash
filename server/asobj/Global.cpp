// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

// Implementation of the Global ActionScript Object

#include "as_object.h"
#include "array.h"
#include "Boolean.h"
#include "Camera.h"
#include "Color.h"
#include "ContextMenu.h"
#include "CustomActions.h"
#include "Date.h"
#include "Error.h"
#include "as_function.h" // for action_init
#include "Global.h"
#include "gstring.h"
#include "Key.h"
#include "LoadVars.h"
#include "LocalConnection.h"
#include "Microphone.h"
#include "GMath.h"
#include "Mouse.h"
#include "MovieClipLoader.h"
#include "MovieClip.h" 
#include "movie_definition.h"
#include "NetConnection.h"
#include "NetStream.h"
#include "Selection.h"
#include "SharedObject.h"
#include "smart_ptr.h"
#include "ASSound.h"
#include "Stage.h"
#include "System.h"
#include "textformat.h"
#include "TextSnapshot.h"
#include "Video.h"

#include <fn_call.h>
#include <sprite_instance.h>

#ifdef HAVE_LIBXML
#include "xml.h"
#include "xmlsocket.h"
#endif

using namespace std;

namespace gnash {

void
as_global_trace(const fn_call& fn)
{
    assert(fn.nargs >= 1);

// @@ NOTHING should get special treatment,
//    as_value::to_string() will take care of everything
#if 0
    // Special case for objects: try the toString() method.
    if (fn.arg(0).get_type() == as_value::OBJECT)
	{
	    as_object* obj = fn.arg(0).to_object();
	    assert(obj);

	    as_value method;
	    if (obj->get_member("toString", &method)
		&& method.is_function())
		{
		    as_value result = call_method0(method, fn.env, obj);
		    log_msg("%s\n", result.to_string());

		    return;
		}
	}
#endif

    // Log our argument.
    //
    // @@ what if we get extra args?
    //
    // @@ Array gets special treatment.
    // @@ NOTHING should get special treatment,
    //    as_value::to_string() will take care of everything
    const char* arg0 = fn.arg(0).to_string();
    log_msg("%s\n", arg0);
}


static void
as_global_object_ctor(const fn_call& fn)
    // Constructor for ActionScript class Object.
{
    as_object *new_obj;

    if ( fn.nargs == 0 )
	{
	    new_obj = new as_object();
	}
    else if ( fn.nargs == 1 ) // copy constructor
	{
	    as_object *src_obj = fn.arg(0).to_object();
	    new_obj = new as_object(src_obj);
	}
    else
	{
	    dbglogfile << "Too many args to Object constructor" << endl;
	    new_obj = new as_object();
	}

    fn.result->set_as_object(new_obj);
}

static void
as_global_isnan(const fn_call& fn)
{
    assert(fn.nargs == 1);

    fn.result->set_bool(fn.arg(0).is_nan());
}

static void
as_global_isfinite(const fn_call& fn)
{
    assert(fn.nargs == 1);

    fn.result->set_bool(fn.arg(0).is_finite());
}

static void
as_global_unescape(const fn_call& fn)
{
    assert(fn.nargs == 1);

    string input = fn.arg(0).to_string();
    string insertst;
    int hexcode;

    for (unsigned int i=0;i<input.length();)
	{
	    if ((input.length() > i + 2) && input[i] == '%' &&
		isxdigit(input[i+1]) && isxdigit(input[i+2]))
		{
		    input[i+1] = toupper(input[i+1]);
		    input[i+2] = toupper(input[i+2]);
		    if (isdigit(input[i+1]))
			hexcode = (input[i+1] - '0') * 16;
		    else
			hexcode = (input[i+1] - 'A' + 10) * 16;

		    if (isdigit(input[i+2]))
			hexcode += (input[i+2] - '0');
		    else
			hexcode += (input[i+2] - 'A' + 10);

		    input.erase(i,3);
				
		    switch (hexcode)
			{
			  case 0x20: // space
			      insertst = ' ';
			      break;
			  case 0x22: // "
			      insertst = '\"';
			      break;
			  case 0x23: // #
			      insertst = '#';
			      break;
			  case 0x24: // $
			      insertst = '$';
			      break;
			  case 0x25: // %
			      insertst = '%';
			      break;
			  case 0x26: // &
			      insertst = '&';
			      break;
			  case 0x2B: // +
			      insertst = '+';
			      break;
			  case 0x2C: // ,
			      insertst = ',';
			      break;
			  case 0x2F: // /
			      insertst = '/';
			      break;
			  case 0x3A: // :
			      insertst = ':';
			      break;
			  case 0x3B: // ;
			      insertst = ';';
			      break;
			  case 0x3C: // <
			      insertst = '<';
			      break;
			  case 0x3D: // =
			      insertst = '=';
			      break;
			  case 0x3E: // >
			      insertst = '>';
			      break;
			  case 0x3F: // ?
			      insertst = '?';
			      break;
			  case 0x40: // @
			      insertst = '@';
			      break;
			  case 0x5B: // [
			      insertst = '[';
			      break;
			  case 0x5C: // \ (backslash)
			      insertst = '\\';
			      break;
			  case 0x5D: // ]
			      insertst = ']';
			      break;
			  case 0x5E: // ^
			      insertst = '^';
			      break;
			  case 0x60: // `
			      insertst = '`';
			      break;
			  case 0x7B: // {
			      insertst = '{';
			      break;
			  case 0x7C: // |
			      insertst = '|';
			      break;
			  case 0x7D: // }
			      insertst = '}';
			      break;
			  case 0x7E: // ~
			      insertst = '~';
			      break;
			  default:
			      log_action("ERROR: unescape() function reached "
							  "unknown hexcode %d, aborting unescape()\n",hexcode);
			      fn.result->set_string(fn.arg(0).to_string());
			      return;
			}
		    input.insert(i,insertst);
		}
	    else
		i++;
	}
    fn.result->set_string(input.c_str());
}

static void
as_global_parsefloat(const fn_call& fn)
{
    assert(fn.nargs == 1);

    float result;

    // sscanf will handle the whitespace / unneeded characters etc. automatically
    if (1 == sscanf(fn.arg(0).to_string(), "%f", &result))
	fn.result->set_double(double(result));
    else
	// if sscanf didn't find anything, return NaN
	fn.result->set_nan();
}

static void
as_global_parseint(const fn_call& fn)
{
    assert(fn.nargs == 2 || fn.nargs == 1);

    // Make sure our argument is the correct type
    if (fn.nargs > 1)
	fn.arg(1).convert_to_number();

    // Set up some variables
    const string digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char *input = new char[strlen(fn.arg(0).to_string())+1];
    strcpy(input,fn.arg(0).to_string());
    double base;
    int result = 0, i;
    bool bNegative;

    // Skip leading whitespace
    while ((input[0] == ' ') || (input[0] == 0x9))
	input++;

    if (input[0] == '-')
	{
	    bNegative = true;
	    input++;
	}
    else
	bNegative = false;

    // Convert the string to uppercase
    for (i=0;i<int(strlen(input));i++)
	input[i] = toupper(input[i]);

    // if we were sent a second argument, that's our base
    if (fn.nargs > 1)
	{
	    base = fn.arg(1).to_number();
	}
    // if the string starts with "0x" then a hex digit
    else if (strlen(input) > 2 && input[0] == '0' && input[1] == 'X'
	     && (isdigit(input[2]) || (input[2] >= 'A' && input[2] <= 'F')))
	{
	    base = 16.0;	// the base is 16
	    input = input + 2; // skip the leading "0x"
	}
    // if the string starts with "0" then an octal digit
    else if (strlen(input) > 1 && input[0] == '0' &&
	     (input[1] >= '0' && input[1] <= '7'))
	{
	    base = 8.0;
	    input++; // skip the leading '0'
	}
    else
	// default base is 10
	base = 10.0;

    assert (base >= 2 && base <= 36);

    int numdigits = 0;

    // Start at the beginning, see how many valid digits we have
    // in the base we're dealing with
    while (numdigits < int(strlen(input)) 
	   && int(digits.find(input[numdigits])) < base
	   && digits.find(input[numdigits]) != std::string::npos)
	numdigits++;

    // If we didn't get any digits, we should return NaN
    if (numdigits == 0)
	{
	    fn.result->set_nan();
	    return;
	}

    for (i=0;i<numdigits;i++)
	{
		result += digits.find(input[i]) * (int)pow(base, numdigits - i - 1);
	}

    if (bNegative)
	result = -result;
    
    delete [] input;
    
    // Now return the parsed string
    fn.result->set_int(result);
}

// ASSetPropFlags function
static void
as_global_assetpropflags(const fn_call& fn)
{
	int version = fn.env->get_version();

	log_msg("ASSetPropFlags called with %d args", fn.nargs);

	// Check the arguments
	assert(fn.nargs == 3 || fn.nargs == 4);
	assert((version == 5) ? (fn.nargs == 3) : true);
		
	// ASSetPropFlags(obj, props, n, allowFalse=false)

	// object
	as_object* obj = fn.arg(0).to_object();
	if ( ! obj )
	{
		log_warning("Invalid call to ASSetPropFlags: "
			"object argument is not an object: %s",
			fn.arg(0).to_string());
		return;
	}

	// list of child names

	as_object* props = fn.arg(1).to_object();
	if (props == NULL)
	{
		// second argument can either be an array or
		// a comma-delimited string.
		// see: http://www.flashguru.co.uk/assetpropflags/
		log_error("ASSetPropFlags unimplemented for non-array prop"
			" argument (%s)", fn.arg(1).to_string());

		return; // be nice, dont' abort

		// tulrich: this fires in test_ASSetPropFlags -- is it correct?
		assert(fn.arg(1).get_type() == as_value::NULLTYPE);
	}

    // a number which represents three bitwise flags which
    // are used to determine whether the list of child names should be hidden,
    // un-hidden, protected from over-write, un-protected from over-write,
    // protected from deletion and un-protected from deletion
    int set_true = int(fn.arg(2).to_number()) & as_prop_flags::as_prop_flags_mask;

    // Is another integer bitmask that works like set_true,
    // except it sets the attributes to false. The
    // set_false bitmask is applied before set_true is applied

    // ASSetPropFlags was exposed in Flash 5, however the fourth argument 'set_false'
    // was not required as it always defaulted to the value '~0'. 
    int set_false = (fn.nargs == 3 ? 
		     (version == 5 ? ~0 : 0) : int(fn.arg(3).to_number()))
	& as_prop_flags::as_prop_flags_mask;

    // Evan: it seems that if set_true == 0 and set_false == 0, this function
    // acts as if the parameters where (object, null, 0x1, 0) ...
    if (set_false == 0 && set_true == 0)
	{
	    props = NULL;
	    set_false = 0;
	    set_true = 0x1;
	}

    if (props == NULL)
	{
	    // Take all the members of the object

	    as_object* object = obj;

	    stringi_hash<as_member>::const_iterator it = object->m_members.begin();
	    while (it != object->m_members.end())
		{
		    as_member member = it->second;

		    as_prop_flags f = member.get_member_flags();
		    //const int oldflags = 
		    f.get_flags();
		    //const int newflags =
		    f.set_flags(set_true, set_false);
		    member.set_member_flags(f);

		    object->m_members[it->first] = member;

		    ++it;
		}

	    if (object->m_prototype != NULL)
		{
		    const as_object* prototype = object->m_prototype;

		    it = prototype->m_members.begin();
		    while (it != prototype->m_members.end())
			{
			    as_member member = it->second;

			    as_prop_flags f = member.get_member_flags();
			    //const int oldflags =
			    f.get_flags();
			    //const int newflags = 
			    f.set_flags(set_true, set_false);
			    member.set_member_flags(f);

			    object->m_members[it->first] = member;

			    ++it;
			}
		}
	}
	else
	{
	    as_object* object = obj;
	    as_object* object_props = props;

	    stringi_hash<as_member>::iterator it = object_props->m_members.begin();
	    while(it != object_props->m_members.end())
		{
		    const tu_stringi key = (it->second).get_member_value().to_string();
		    stringi_hash<as_member>::iterator it2 = object->m_members.find(key);

		    if (it2 != object->m_members.end())
			{
			    as_member member = it2->second;

			    as_prop_flags f = member.get_member_flags();
			    //const int oldflags =
			    f.get_flags();
			    //const int newflags =
			    f.set_flags(set_true, set_false);
			    member.set_member_flags(f);

			    object->m_members[it->second.get_member_value().to_string()] = member;
			}

		    ++it;
		}
	}
}

Global::Global()
	:
	as_object()
{
	set_member("trace", as_value(as_global_trace));
	set_member("Object", as_value(as_global_object_ctor));
	set_member("Sound", as_value(sound_new));

	set_member("TextFormat", as_value(textformat_new));
#ifdef HAVE_LIBXML
	set_member("XML", as_value(xml_new));
	set_member("XMLNode", as_value(xmlnode_new));
	//set_member("XML", as_value(xmlsocket_xml_new));
	set_member("XMLSocket", as_value(xmlsocket_new));
#endif // HAVE_LIBXML
	set_member("MovieClipLoader", as_value(moviecliploader_new));
	//set_member("String", as_value(string_ctor));
	// This next set are all the unimplemented classes whose
	// code was machine generated.
	set_member("Boolean", as_value(boolean_new));
	set_member("Camera", as_value(camera_new));
	set_member("Color", as_value(color_new));
	set_member("ContextMenu", as_value(contextmenu_new));
	set_member("CustomActions", as_value(customactions_new));
	set_member("Date", as_value(date_new));
	set_member("Error", as_value(error_new));
	set_member("LoadVars", as_value(loadvars_new));
	set_member("LocalConnection", as_value(localconnection_new));
	set_member("Microphone", as_value(microphone_new));
	set_member("Mouse", as_value(mouse_new));
	set_member("NetConnection", as_value(netconnection_new));
	set_member("NetStream", as_value(netstream_new));
	set_member("Selection", as_value(selection_new));
	set_member("SharedObject", as_value(sharedobject_new));
	set_member("Stage", as_value(stage_new));
	set_member("System", as_value(system_new));
	set_member("TextSnapshot", as_value(textsnapshot_new));
	set_member("Video", as_value(video_new));
	// ASSetPropFlags
	set_member("ASSetPropFlags", as_global_assetpropflags);
	// unescape
	set_member("unescape", as_global_unescape);
	// parseFloat
	set_member("parseFloat", as_global_parsefloat);
	// parseInt
	set_member("parseInt", as_global_parseint);
	// isNan
	set_member("isNan", as_global_isnan);
	// isFinite
	set_member("isFinite", as_global_isfinite);

	string_class_init(*this); 
	array_class_init(*this);
	function_init(this);
	movieclip_init(this);
	math_init(this);
	key_init(this);
	system_init(this);
}

} // namespace gnash
