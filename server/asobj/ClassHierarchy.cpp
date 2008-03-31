// 
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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
#include "button_character_instance.h"
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
#include "asClass.h"

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
		init_member("constructor", as_function::getFunctionConstructor().get());
	}

	virtual as_value operator()(const fn_call& /*fn*/)
	{
		string_table& st = VM::get().getStringTable();
		log_debug("Loading extension class %s", st.value(mDeclaration.name));

		as_value super;
		if (mDeclaration.super_name)
		{
			// Check to be sure our super exists.
			// This will trigger its instantiation if necessary.
			if (!mTarget->get_member(mDeclaration.super_name, &super))
			{
				// Error here -- doesn't exist.
				log_error("Can't find %s (Superclass of %s)",
					st.value(mDeclaration.super_name),
					st.value(mDeclaration.name));
				super.set_undefined();
				return super;
			}
			if (!super.is_as_function())
			{
				// Error here -- not an object.
				log_error("%s (Superclass of %s) is not a function (%s)",
					st.value(mDeclaration.super_name),
					st.value(mDeclaration.name), super.to_debug_string());
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
			if (mDeclaration.super_name && !us.to_object()->hasOwnProperty(NSV::PROP_uuPROTOuu))
			{
				us.to_object()->set_prototype(super.to_as_function()->getPrototype());
			}
			return us;
		}
		// Error here -- not successful in loading.
		log_error("Could not load class %s", st.value(mDeclaration.name));
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
		// does it make any sense to set a 'constructor' here ??
		//init_member("constructor", this);
		//init_member("constructor", as_function::getFunctionConstructor().get());
	}

	virtual as_value operator()(const fn_call& /*fn*/)
	{
		string_table& st = VM::get().getStringTable();

		log_debug("Loading native class %s", st.value(mDeclaration.name));

		mDeclaration.initializer(*mTarget);
		// Successfully loaded it, now find it, set its proto, and return.
		as_value us;
		if ( mTarget->get_member(mDeclaration.name, &us) )
		{
			as_value super;
			if (mDeclaration.super_name)
			{
				// Check to be sure our super exists.
				// This will trigger its instantiation if necessary.
				if (!mTarget->get_member(mDeclaration.super_name, &super))
				{
					// Error here -- doesn't exist.
					log_error("Can't find %s (Superclass of %s)",
						st.value(mDeclaration.super_name),
						st.value(mDeclaration.name));
					super.set_undefined();
					return super;
				}
				if (!super.is_as_function())
				{
					// Error here -- not an object.
					log_error("%s (Superclass of %s) is not a function (%s)",
						st.value(mDeclaration.super_name),
						st.value(mDeclaration.name), super.to_debug_string());
					super.set_undefined();
					return super;
				}
				assert(super.to_as_function());
			}
			if ( ! us.to_object() )
			{
				log_error("Native class %s is not an object after initialization (%s)",
					st.value(mDeclaration.name), us.to_debug_string());
			}
			if (mDeclaration.super_name && !us.to_object()->hasOwnProperty(NSV::PROP_uuPROTOuu))
			{
				us.to_object()->set_prototype(super.to_as_function()->getPrototype());
			}
		}
		else
		{
			log_error("Native class %s is not found after initialization", 
				st.value(mDeclaration.name));
		}
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

	mGlobalNamespace->stubPrototype(c.name);
	mGlobalNamespace->getClass(c.name)->setDeclared();
	mGlobalNamespace->getClass(c.name)->setSystem();

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
	// For AS2 and below, registering with mGlobal _should_ make it equivalent
	// to being in the global namespace, since everything is global there.
	asNamespace *nso = findNamespace(c.namespace_name);
	if (!nso)
		nso = addNamespace(c.namespace_name);
	nso->stubPrototype(c.name);
	nso->getClass(c.name)->setDeclared();
	nso->getClass(c.name)->setSystem();

	boost::intrusive_ptr<as_function> getter =
		new declare_native_function(c, mGlobal, mExtension);
	boost::intrusive_ptr<as_function> setter =
		new builtin_function(do_nothing);

	return mGlobal->init_destructive_property(c.name,
		*getter, *setter);
}

static ClassHierarchy::nativeClass knownClasses[] =
{
// This makes it clear the difference between "We don't know where the
// class belongs" and "it belongs in the global namespace", even though
// the result is the same.
#define NS_GLOBAL 0
#define NS_UNKNOWN 0
	/* { function_name, name key, super name key, lowest version }, */
//	{ object_class_init, NSV::CLASS_OBJECT, 0, NS_GLOBAL, 5 }, // Object is special
//	{ function_class_init, NSV::CLASS_FUNCTION, NSV::CLASS_OBJECT, NS_GLOBAL, 6 }, // Function is special
//	{ array_class_init, NSV::CLASS_ARRAY, NSV::CLASS_OBJECT, NS_GLOBAL, 5 }, // Array is special
	{ system_class_init, NSV::CLASS_SYSTEM, 0, NSV::NS_FLASH_SYSTEM, 1 },
	{ stage_class_init, NSV::CLASS_STAGE, 0, NSV::NS_FLASH_DISPLAY, 1 },
	{ movieclip_class_init, NSV::CLASS_MOVIE_CLIP, 0, NSV::NS_FLASH_DISPLAY, 3 },
	{ textfield_class_init, NSV::CLASS_TEXT_FIELD, 0, NSV::NS_FLASH_TEXT, 3 },
	{ math_class_init, NSV::CLASS_MATH, 0, NS_GLOBAL, 4 },
	{ boolean_class_init, NSV::CLASS_BOOLEAN, NSV::CLASS_OBJECT, NS_GLOBAL, 5 },
	{ button_class_init, NSV::CLASS_BUTTON, NSV::CLASS_OBJECT, NS_GLOBAL, 5 },
	{ color_class_init, NSV::CLASS_COLOR, NSV::CLASS_OBJECT, NS_GLOBAL, 5 },
	{ selection_class_init, NSV::CLASS_SELECTION, NSV::CLASS_OBJECT, NS_UNKNOWN, 5 },
	{ sound_class_init, NSV::CLASS_SOUND, NSV::CLASS_OBJECT, NSV::NS_FLASH_MEDIA, 5 },
	{ xmlsocket_class_init, NSV::CLASS_X_M_L_SOCKET, NSV::CLASS_OBJECT, NSV::NS_FLASH_NET, 5 },
	{ date_class_init, NSV::CLASS_DATE, NSV::CLASS_OBJECT, NS_GLOBAL, 5 },
	{ xml_class_init, NSV::CLASS_X_M_L, NSV::CLASS_OBJECT, NS_GLOBAL, 5 },
	{ xmlnode_class_init, NSV::CLASS_X_M_L_NODE, NSV::CLASS_OBJECT, NSV::NS_FLASH_XML, 5 },
	{ mouse_class_init, NSV::CLASS_MOUSE, NSV::CLASS_OBJECT, NSV::NS_FLASH_UI, 5 },
	{ number_class_init, NSV::CLASS_NUMBER, NSV::CLASS_OBJECT, NS_GLOBAL, 5 },
//	{ string_class_init, NSV::CLASS_STRING, NSV::CLASS_OBJECT, NS_GLOBAL, 5 }, // string is special
	{ key_class_init, NSV::CLASS_KEY, NSV::CLASS_OBJECT, NS_GLOBAL, 5 },
	{ AsBroadcaster_init, NSV::CLASS_AS_BROADCASTER, NSV::CLASS_OBJECT, NS_GLOBAL, 5 },
	{ textsnapshot_class_init, NSV::CLASS_TEXT_SNAPSHOT, NSV::CLASS_OBJECT, NSV::NS_FLASH_TEXT, 6 },
	{ video_class_init, NSV::CLASS_VIDEO, NSV::CLASS_OBJECT, NSV::NS_FLASH_MEDIA, 6 },
	{ camera_class_init, NSV::CLASS_CAMERA, NSV::CLASS_OBJECT, NSV::NS_FLASH_UI, 6 },
	{ microphone_class_init, NSV::CLASS_MICROPHONE, NSV::CLASS_OBJECT, NSV::NS_FLASH_UI, 6 },
	{ sharedobject_class_init, NSV::CLASS_SHARED_OBJECT, NSV::CLASS_OBJECT, NSV::NS_FLASH_NET, 6 },
	{ loadvars_class_init, NSV::CLASS_LOAD_VARS, NSV::CLASS_OBJECT, NS_GLOBAL, 6 },
	{ customactions_class_init, NSV::CLASS_CUSTOM_ACTIONS, NSV::CLASS_OBJECT, NSV::NS_ADOBE_UTILS, 6 },
	{ netconnection_class_init, NSV::CLASS_NET_CONNECTION, NSV::CLASS_OBJECT, NSV::NS_FLASH_NET, 6 },
	{ netstream_class_init, NSV::CLASS_NET_STREAM, NSV::CLASS_OBJECT, NSV::NS_FLASH_NET, 6 },
	{ contextmenu_class_init, NSV::CLASS_CONTEXT_MENU, NSV::CLASS_OBJECT, NSV::NS_FLASH_UI, 7 },
	{ moviecliploader_class_init, NSV::CLASS_MOVIE_CLIP_LOADER, NSV::CLASS_OBJECT, NS_GLOBAL, 7 },
	{ error_class_init, NSV::CLASS_ERROR, NSV::CLASS_OBJECT, NS_GLOBAL, 7 },
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

void
ClassHierarchy::markReachableResources() const
{
	// TODO
}

void
ClassHierarchy::dump()
{
}

} /* namespace gnash */
