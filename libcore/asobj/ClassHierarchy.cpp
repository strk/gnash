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
#include "Button.h"
#include "array.h"
#include "AsBroadcaster.h"
#include "Accessibility_as.h"
#include "Boolean.h"
#include "Camera.h"
#include "Color.h"
#include "ContextMenu.h"
#include "CustomActions.h"
#include "Date.h"
#include "Error_as.h"
#include "Global.h"
#include "String_as.h"
#include "Key_as.h"
#include "LoadVars_as.h"
#include "LocalConnection.h"
#include "Microphone.h"
#include "Number_as.h"
#include "Object.h"
#include "Math_as.h"
#include "Mouse.h"
#include "MovieClipLoader.h"
#include "movie_definition.h"
#include "NetConnection_as.h"
#include "NetStream_as.h"
#include "Selection_as.h"
#include "SharedObject.h"
#include "Sound.h"
#include "Stage_as.h"
#include "System_as.h"
#include "TextSnapshot_as.h"
#include "TextFormat.h"
#include "Video.h"
#include "extension.h"
#include "VM.h"
#include "timers.h"
#include "URL.h" // for URL::encode and URL::decode (escape/unescape)
#include "builtin_function.h"
#include "TextField.h"
#include "namedStrings.h"
#include "ClassHierarchy.h"
#include "builtin_function.h"
#include "XMLSocket_as.h"
#include "XML_as.h"
#include "XMLNode_as.h"
#include "asClass.h"

namespace gnash {

namespace { // anonymous namespace

static void
addVisibilityFlag(int& flags, int version)
{
    // TODO: more visibility for swf10+?
    if ( version >= 9 ) flags |= as_prop_flags::onlySWF9Up;
    else if ( version >= 8 ) flags |= as_prop_flags::onlySWF8Up;
    else if ( version >= 7 ) flags |= as_prop_flags::onlySWF7Up;
    else if ( version >= 6 ) flags |= as_prop_flags::onlySWF6Up;
}

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
					st.value(mDeclaration.name), super);
				super.set_undefined();
				return super;
			}
		}
		if (mExtension->initModuleWithFunc(mDeclaration.file_name,
			mDeclaration.init_name, *mTarget))
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

	declare_native_function(const ClassHierarchy::nativeClass &c,
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
						st.value(mDeclaration.name), super);
					super.set_undefined();
					return super;
				}
				assert(super.to_as_function());
			}
			if ( ! us.to_object() )
			{
				log_error("Native class %s is not an object after initialization (%s)",
					st.value(mDeclaration.name), us);
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

ClassHierarchy::~ClassHierarchy()
{
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

	int flags=as_prop_flags::dontEnum;
	addVisibilityFlag(flags, c.version);
	return mGlobal->init_destructive_property(c.name, *getter, flags);
}

bool
ClassHierarchy::declareClass(const nativeClass& c)
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

	int flags=as_prop_flags::dontEnum;
	addVisibilityFlag(flags, c.version);
	return mGlobal->init_destructive_property(c.name,
		*getter, flags);
}

static const ClassHierarchy::nativeClass knownClasses[] =
{
// This makes clear the difference between "We don't know where the
// class belongs" and "it belongs in the global namespace", even though
// the result is the same.
    #define NS_GLOBAL 0
    #define NS_UNKNOWN 0

//  { function_name, name key, super name key, lowest version },
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
	{ XML_as::init, NSV::CLASS_X_M_L, NSV::CLASS_OBJECT, NS_GLOBAL, 5 },
	{ XMLNode_as::init, NSV::CLASS_X_M_L_NODE, NSV::CLASS_OBJECT,
        NSV::NS_FLASH_XML, 5 },
	{ mouse_class_init, NSV::CLASS_MOUSE, NSV::CLASS_OBJECT, NSV::NS_FLASH_UI, 5 },
	{ number_class_init, NSV::CLASS_NUMBER, NSV::CLASS_OBJECT, NS_GLOBAL, 5 },
	{ textformat_class_init, NSV::CLASS_TEXT_FORMAT, NSV::CLASS_OBJECT, NS_GLOBAL, 5 },
	{ key_class_init, NSV::CLASS_KEY, NSV::CLASS_OBJECT, NS_GLOBAL, 5 },
	{ AsBroadcaster_init, NSV::CLASS_AS_BROADCASTER, NSV::CLASS_OBJECT, NS_GLOBAL, 5 },
	{ textsnapshot_class_init, NSV::CLASS_TEXT_SNAPSHOT, NSV::CLASS_OBJECT, NSV::NS_FLASH_TEXT, 6 },
	{ video_class_init, NSV::CLASS_VIDEO, NSV::CLASS_OBJECT, NSV::NS_FLASH_MEDIA, 6 },
	{ camera_class_init, NSV::CLASS_CAMERA, NSV::CLASS_OBJECT, NSV::NS_FLASH_UI, 6 },
	{ microphone_class_init, NSV::CLASS_MICROPHONE, NSV::CLASS_OBJECT, NSV::NS_FLASH_UI, 6 },
	{ sharedobject_class_init, NSV::CLASS_SHARED_OBJECT, NSV::CLASS_OBJECT, NSV::NS_FLASH_NET, 6 },
	{ loadvars_class_init, NSV::CLASS_LOAD_VARS, NSV::CLASS_OBJECT, NS_GLOBAL, 6 },
	{ localconnection_class_init, NSV::CLASS_LOCAL_CONNECTION, NSV::CLASS_OBJECT, NS_GLOBAL, 6 }, // FIXME: not global ?
	{ customactions_class_init, NSV::CLASS_CUSTOM_ACTIONS, NSV::CLASS_OBJECT, NSV::NS_ADOBE_UTILS, 6 },
	{ netconnection_class_init, NSV::CLASS_NET_CONNECTION, NSV::CLASS_OBJECT, NSV::NS_FLASH_NET, 6 },
	{ netstream_class_init, NSV::CLASS_NET_STREAM, NSV::CLASS_OBJECT, NSV::NS_FLASH_NET, 6 },
	{ contextmenu_class_init, NSV::CLASS_CONTEXT_MENU, NSV::CLASS_OBJECT, NSV::NS_FLASH_UI, 7 },
	{ moviecliploader_class_init, NSV::CLASS_MOVIE_CLIP_LOADER, NSV::CLASS_OBJECT, NS_GLOBAL, 7 },
	{ Error_class_init, NSV::CLASS_ERROR, NSV::CLASS_OBJECT, NS_GLOBAL, 5 },
	{ Accessibility_class_init, NSV::CLASS_ACCESSIBILITY, NSV::CLASS_OBJECT, 
        NS_GLOBAL, 5 }
// These classes are all implicitly constructed; that is, it is not
// necessary for the class name to be used to construct the class, so
// they must always be available.
//	{ object_class_init, NSV::CLASS_OBJECT, 0, NS_GLOBAL, 5 }
//	{ function_class_init, NSV::CLASS_FUNCTION, NSV::CLASS_OBJECT,
//	NS_GLOBAL, 6 }
//	{ array_class_init, NSV::CLASS_ARRAY, NSV::CLASS_OBJECT, NS_GLOBAL, 5 }
//	{ string_class_init, NSV::CLASS_STRING, NSV::CLASS_OBJECT, NS_GLOBAL, 5 }

};

void
ClassHierarchy::massDeclare(int /*version*/) // drop version...
{
	// Natives get declared first. It doesn't make any sense for a native
	// to depend on an extension, but it does make sense the other way
	// around.
	const size_t size = sizeof (knownClasses) / sizeof (nativeClass);
	for (size_t i = 0; i < size; ++i)
	{
		const nativeClass& c = knownClasses[i];
		//if (c.version > version) continue;

		if ( ! declareClass(c) )
		{
			log_error("Could not declare class %s", c);
		}
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

std::ostream& operator << (std::ostream& os, const ClassHierarchy::nativeClass& c)
{
	string_table& st = VM::get().getStringTable();

	os << "("
		<< " name:" << st.value(c.name)
		<< " super:" << st.value(c.super_name)
		<< " namespace:" << st.value(c.namespace_name)
		<< " version:" << c.version
		<< ")";

	return os;
}

std::ostream& operator << (std::ostream& os, const ClassHierarchy::extensionClass& c)
{
	string_table& st = VM::get().getStringTable();

	os << "(file:" << c.file_name
		<< " init:" << c.init_name
		<< " name:" << st.value(c.name)
		<< " super:" << st.value(c.super_name)
		<< " namespace:" << st.value(c.namespace_name)
		<< " version:" << c.version
		<< ")";

	return os;
}

} /* namespace gnash */
