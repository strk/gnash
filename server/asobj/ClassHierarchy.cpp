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
//

#include "as_object.h"
#include "as_prop_flags.h"
#include "as_value.h"
#include "as_function.h" // for function_class_init
#include "array.h"
#include "AsBroadcaster.h"
#include "Boolean.h"
#include "Camera.h"
#include "Color.h"
#include "ContextMenu.h"
#include "CustomActions.h"
#include "Date.h"
#include "Error.h"
#include "Global.h"
#include "gstring.h"
#include "Key.h"
#include "LoadVars.h"
#include "LocalConnection.h"
#include "Microphone.h"
#include "Number.h"
#include "Object.h"
#include "GMath.h"
#include "Mouse.h"
#include "MovieClipLoader.h"
#include "movie_definition.h"
#include "NetConnection.h"
#include "NetStream.h"
#include "Selection.h"
#include "SharedObject.h"
#include "Sound.h"
#include "Stage.h"
#include "System.h"
#include "textformat.h"
#include "TextSnapshot.h"
#include "video_stream_instance.h"
#include "extension.h"
#include "VM.h"
#include "timers.h"
#include "URL.h" // for URL::encode and URL::decode (escape/unescape)
#include "builtin_function.h"
#include "edit_text_character.h"
#include "namedStrings.h"
#include "ClassHierarchy.h"
#include "builtin_function.h"
#include "xmlsocket.h"
#include "xml.h"
#include "xmlnode.h"

namespace gnash {

namespace { // anonymous namespace

class declare_extension_function : public as_function
{
private:
	ClassHierarchy::extensionClass mDeclaration;
	as_object *mTarget;
	Extension *mExtension;

public:
	bool isBuiltin() { return true; }

	declare_extension_function(ClassHierarchy::extensionClass &c,
		as_object *g, Extension* e) :
		as_function(getObjectInterface()),
		mDeclaration(c), mTarget(g), mExtension(e)
	{
		init_member("constructor", this);
	}

	virtual as_value operator()(const fn_call& /*fn*/)
	{
		as_value super;
		if (mDeclaration.super_name)
		{
			// Check to be sure our super exists.
			// This will trigger its instantiation if necessary.
			if (!mTarget->get_member(mDeclaration.super_name, &super))
			{
				// Error here -- doesn't exist.
				// TODO: Log the error.
				super.set_undefined();
				return super;
			}
			if (!super.is_object())
			{
				// Error here -- not an object.
				// TODO: Log the error.
				super.set_undefined();
				return super;
			}
		}
		if (mExtension->initModuleWithFunc(mDeclaration.file_name.c_str(),
			mDeclaration.init_name.c_str(), *mTarget))
		{
			// Successfully loaded it, now find it, set its proto, and return.
			as_value us;
			mTarget->get_member(mDeclaration.name, &us);
			if (0 && mDeclaration.super_name)
				us.to_object()->set_prototype(boost::intrusive_ptr<as_object>(super.to_object()));
			fprintf(stderr, "Loaded ourselves.\n");
			return us;
		}
		// Error here -- not successful in loading.
		super.set_undefined();
		return super;
	}
};

class declare_native_function : public as_function
{
private:
	ClassHierarchy::nativeClass mDeclaration;
	as_object *mTarget;
	Extension *mExtension;

public:
	bool isBuiltin() { return true; }

	declare_native_function(ClassHierarchy::nativeClass &c,
		as_object *g, Extension *e) :
		as_function(getObjectInterface()),
		mDeclaration(c), mTarget(g), mExtension(e)
	{
		init_member("constructor", this);
	}

	virtual as_value operator()(const fn_call& /*fn*/)
	{
		as_value super;
		if (mDeclaration.super_name)
		{
			// Check to be sure our super exists.
			// This will trigger its instantiation if necessary.
			if (!mTarget->get_member(mDeclaration.super_name, &super))
			{
				// Error here -- doesn't exist.
				// TODO: Log the error.
				super.set_undefined();
				return super;
			}
			if (!super.is_object())
			{
				// Error here -- not an object.
				// TODO: Log the error.
				super.set_undefined();
				return super;
			}
		}
		mDeclaration.initializer(*mTarget);
		// Successfully loaded it, now find it, set its proto, and return.
		as_value us;
		mTarget->get_member(mDeclaration.name, &us);
		if (0 && mDeclaration.super_name)
			us.to_object()->set_prototype(boost::intrusive_ptr<as_object>(super.to_object()));
		return us;
	}
};

} // end anonymous namespace

static
as_value do_nothing(const fn_call&)
{
	return as_value();
}

bool
ClassHierarchy::declareClass(extensionClass& c)
{
	if (mExtension == NULL)
		return false; // Extensions can't be loaded.

	boost::intrusive_ptr<as_function> getter =
		new declare_extension_function(c, mGlobal, mExtension);
	boost::intrusive_ptr<as_function> setter =
		new builtin_function(do_nothing);

	return mGlobal->init_destructive_property(c.name,
		*getter, *setter);
}

bool
ClassHierarchy::declareClass(nativeClass& c)
{
	boost::intrusive_ptr<as_function> getter =
		new declare_native_function(c, mGlobal, mExtension);
	boost::intrusive_ptr<as_function> setter =
		new builtin_function(do_nothing);

	return mGlobal->init_destructive_property(c.name,
		*getter, *setter);
}

static ClassHierarchy::nativeClass knownClasses[] =
{
	/* { function_name, name key, super name key, lowest version }, */
//	{ object_class_init, NSV::CLASS_OBJECT, 0, 5 }, // Object is special
//	{ function_class_init, NSV::CLASS_FUNCTION, NSV::CLASS_OBJECT, 6 }, // Function is special
//	{ array_class_init, NSV::CLASS_ARRAY, NSV::CLASS_OBJECT, 5 }, // Array is special
	{ system_class_init, NSV::CLASS_SYSTEM, 0, 1 },
	{ stage_class_init, NSV::CLASS_STAGE, 0, 1 },
	{ movieclip_class_init, NSV::CLASS_MOVIE_CLIP, 0, 3 },
	{ textfield_class_init, NSV::CLASS_TEXT_FIELD, 0, 3 },
	{ math_class_init, NSV::CLASS_MATH, 0, 4 },
	{ boolean_class_init, NSV::CLASS_BOOLEAN, NSV::CLASS_OBJECT, 5 },
	{ color_class_init, NSV::CLASS_COLOR, NSV::CLASS_OBJECT, 5 },
	{ selection_class_init, NSV::CLASS_SELECTION, NSV::CLASS_OBJECT, 5 },
	{ sound_class_init, NSV::CLASS_SOUND, NSV::CLASS_OBJECT, 5 },
	{ xmlsocket_class_init, NSV::CLASS_X_M_L_SOCKET, NSV::CLASS_OBJECT, 5 },
	{ date_class_init, NSV::CLASS_DATE, NSV::CLASS_OBJECT, 5 },
	{ xml_class_init, NSV::CLASS_X_M_L, NSV::CLASS_OBJECT, 5 },
	{ xmlnode_class_init, NSV::CLASS_X_M_L_NODE, NSV::CLASS_OBJECT, 5 },
	{ mouse_class_init, NSV::CLASS_MOUSE, NSV::CLASS_OBJECT, 5 },
	{ number_class_init, NSV::CLASS_NUMBER, NSV::CLASS_OBJECT, 5 },
	{ string_class_init, NSV::CLASS_STRING, NSV::CLASS_OBJECT, 5 },
	{ key_class_init, NSV::CLASS_KEY, NSV::CLASS_OBJECT, 5 },
	{ AsBroadcaster_init, NSV::CLASS_AS_BROADCASTER, NSV::CLASS_OBJECT, 5 },
	{ textsnapshot_class_init, NSV::CLASS_TEXT_SNAPSHOT, NSV::CLASS_OBJECT, 6 },
	{ video_class_init, NSV::CLASS_VIDEO, NSV::CLASS_OBJECT, 6 },
	{ camera_class_init, NSV::CLASS_CAMERA, NSV::CLASS_OBJECT, 6 },
	{ microphone_class_init, NSV::CLASS_MICROPHONE, NSV::CLASS_OBJECT, 6 },
	{ sharedobject_class_init, NSV::CLASS_SHARED_OBJECT, NSV::CLASS_OBJECT, 6 },
	{ loadvars_class_init, NSV::CLASS_LOAD_VARS, NSV::CLASS_OBJECT, 6 },
	{ customactions_class_init, NSV::CLASS_CUSTOM_ACTIONS, NSV::CLASS_OBJECT, 6 },
	{ netconnection_class_init, NSV::CLASS_NET_CONNECTION, NSV::CLASS_OBJECT, 7 },
	{ netstream_class_init, NSV::CLASS_NET_STREAM, NSV::CLASS_OBJECT, 7 },
	{ contextmenu_class_init, NSV::CLASS_CONTEXT_MENU, NSV::CLASS_OBJECT, 7 },
	{ moviecliploader_class_init, NSV::CLASS_MOVIE_CLIP_LOADER, NSV::CLASS_OBJECT, 7 },
	{ error_class_init, NSV::CLASS_ERROR, NSV::CLASS_OBJECT, 7 },
};

void
ClassHierarchy::massDeclare(int version)
{
	// Natives get declared first. It doesn't make any sense for a native
	// to depend on an extension, but it does make sense the other way
	// around.
	unsigned int size = sizeof (knownClasses) / sizeof (nativeClass);
	for (unsigned int i = 0; i < size; ++i)
	{
		nativeClass& c = knownClasses[i];
		if (c.version > version)
			continue;
		declareClass(c);
	}

	if (mExtension != NULL)
	{
		/* Load extensions here */
	}
}

} /* namespace gnash */
