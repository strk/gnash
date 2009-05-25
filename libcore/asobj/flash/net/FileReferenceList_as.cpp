// FileReferenceList_as.cpp:  ActionScript "FileReferenceList" class, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "FileReferenceList_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "Object.h" // for AS inheritance
#include "VM.h" // for addStatics

#include <sstream>

namespace gnash {

static as_value FileReferenceList_addListener(const fn_call& fn);
static as_value FileReferenceList_browse(const fn_call& fn);
static as_value FileReferenceList_removeListener(const fn_call& fn);
static as_value FileReferenceList_fileList_getset(const fn_call& fn);


as_value FileReferenceList_ctor(const fn_call& fn);

static void
attachFileReferenceListInterface(as_object& o)
{
    o.init_member("addListener", new builtin_function(FileReferenceList_addListener));
    o.init_member("browse", new builtin_function(FileReferenceList_browse));
    o.init_member("removeListener", new builtin_function(FileReferenceList_removeListener));
    o.init_property("fileList", FileReferenceList_fileList_getset, FileReferenceList_fileList_getset);
}

static void
attachFileReferenceListStaticProperties(as_object& /*o*/)
{
   
}

static as_object*
getFileReferenceListInterface()
{
	static boost::intrusive_ptr<as_object> o;

	if ( ! o )
	{
		// TODO: check if this class should inherit from Object
		//       or from a different class
		o = new as_object(getObjectInterface());
		VM::get().addStatic(o.get());

		attachFileReferenceListInterface(*o);

	}

	return o.get();
}

class FileReferenceList_as: public as_object
{

public:

	FileReferenceList_as()
		:
		as_object(getFileReferenceListInterface())
	{}

	// override from as_object ?
	//std::string get_text_value() const { return "FileReferenceList"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }
};


static as_value
FileReferenceList_addListener(const fn_call& fn)
{
	boost::intrusive_ptr<FileReferenceList_as> ptr = ensureType<FileReferenceList_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
FileReferenceList_browse(const fn_call& fn)
{
	boost::intrusive_ptr<FileReferenceList_as> ptr = ensureType<FileReferenceList_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
FileReferenceList_removeListener(const fn_call& fn)
{
	boost::intrusive_ptr<FileReferenceList_as> ptr = ensureType<FileReferenceList_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
FileReferenceList_fileList_getset(const fn_call& fn)
{
	boost::intrusive_ptr<FileReferenceList_as> ptr = ensureType<FileReferenceList_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}



as_value
FileReferenceList_ctor(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = new FileReferenceList_as;

	if ( fn.nargs )
	{
		std::stringstream ss;
		fn.dump_args(ss);
		LOG_ONCE( log_unimpl("FileReferenceList(%s): %s", ss.str(), _("arguments discarded")) );
	}

	return as_value(obj.get()); // will keep alive
}

// extern 
void filereferencelist_class_init(as_object& where)
{
	// This is going to be the FileReferenceList "class"/"function"
	// in the 'where' package
	boost::intrusive_ptr<builtin_function> cl;
	cl=new builtin_function(&FileReferenceList_ctor, getFileReferenceListInterface());
	attachFileReferenceListStaticProperties(*cl);

	// Register _global.FileReferenceList
	where.init_member("FileReferenceList", cl.get());
}

} // end of gnash namespace
