#!/bin/sh

# 
#   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#

# This script generates header file and C++ source file templates
# for an ActionScript class. It depends on the doc/C/NOTES file for
# data. It takes a single argument, which is the name of the class,
# like "./gen-files.sh Camera".
#
# This script is only of use to developers, so it's "as is". Your
# mileage may vary.

if test x"$1" = "x"; then
	echo "Usage: $0 <classname>" >&2
	exit 1
fi

asname=$1
lowname=`echo ${asname} | tr '[A-Z]' '[a-z]'`
upname=`echo ${asname}  | tr '[a-z]' '[A-Z]'` 
outname=${asname}.h
srcname=${asname}.cpp

if test -f ${outname}; then
    echo ${outname} exists!
    exit 1;
fi

if test -f ${srcname}; then
    echo ${srcname} exists!
    exit 1;
fi

notes=../../doc/C/NOTES
#methods=`grep "${asname}\..*()" ${notes} | sed -e 's/${asname}\.//g'`
methods=`grep "${asname}\\\\..*()" ${notes}`
#methods=`echo ${methods} | sed -e s/${asname}.//g`
methods=`echo ${methods} | sed -e s/${asname}\\\\.//g`
#props=`grep "${asname}\." ${notes} | grep -v "()" | sed -e 's/${asname}\.//g'`
props=`grep "${asname}\\\\." ${notes} | grep -v "()"`
#props=`echo ${props} | sed -e s/${asname}.//g`
props=`echo ${props} | sed -e s/${asname}\\\\.//g`

#echo $methods
#echo $props

###############################################################
# Start by generating the header file for this class
#
rm -f ${outname}
cat <<EOF>>${outname}
// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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

#ifndef __GNASH_ASOBJ_${upname}_H__
#define __GNASH_ASOBJ_${upname}_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <memory> // for auto_ptr

namespace gnash {

class as_object;

/// Initialize the global ${asname} class
void ${lowname}_class_init(as_object& global);

/// Return a ${asname} instance (in case the core lib needs it)
//std::auto_ptr<as_object> init_${lowname}_instance();
  
} // end of gnash namespace

// __GNASH_ASOBJ_${upname}_H__
#endif

EOF

##############################################################
# now generate the source file
#

# start with the header part
cat <<EOF>>${srcname}
// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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
#include "config.h"
#endif

#include "${asname}.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

EOF
for i in $methods; do
# DO NOT CONVERT CASE, SWF7+ is case-sensitive 
newi=`echo $i | sed -e 's/)//g'` # | tr '[A-Z]' '[a-z]'
cat <<EOF>>${srcname}
static void ${lowname}_${newi}const fn_call& fn);
EOF
done
cat <<EOF>>${srcname}
void ${lowname}_ctor(const fn_call& fn);

static void
attach${asname}Interface(as_object& o)
{
EOF
# now process the methods
for i in $methods; do
    # DO NOT CONVERT CASE, SWF7+ is case-sensitive 
    newi=`echo $i | sed -e 's/()//g'` # | tr '[A-Z]' '[a-z]'
    cat <<EOF>>${srcname}
	o.init_member("${newi}", new builtin_function(${lowname}_${newi}));
EOF
done
cat <<EOF>>${srcname}
}

static as_object*
get${asname}Interface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object();
		attach${asname}Interface(*o);
	}
	return o.get();
}

class ${lowname}_as_object: public as_object
{

public:

	${lowname}_as_object()
		:
		as_object(get${asname}Interface())
	{}

	// override from as_object ?
	//const char* get_text_value() const { return "${asname}"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }
};

EOF

cat <<EOF>>${srcname}
static ${lowname}_as_object* 
ensure_${lowname}(as_object* obj)
{
	${lowname}_as_object* ret = dynamic_cast<${lowname}_as_object*>(obj);
	if ( ! ret )
	{
		throw ActionException("builtin method or gettersetter for ${asname} objects called against non-${asname} instance");
	}
	return ret;
}
EOF
for i in $methods; do
# DO NOT CONVERT CASE, SWF7+ is case-sensitive 
newi=`echo $i | sed -e 's/)//g'` # | tr '[A-Z]' '[a-z]'
cat <<EOF>>${srcname}
static void
${lowname}_${newi}const fn_call& fn)
{
	${lowname}_as_object* ptr = ensure_${lowname}(fn.this_ptr);
	UNUSED(ptr);
	log_warning("%s: unimplemented", __FUNCTION__);
}
EOF
done

cat <<EOF>>${srcname}

void
${lowname}_ctor(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = new ${lowname}_as_object;
	
	fn.result->set_as_object(obj.get()); // will keep alive
}

// extern (used by Global.cpp)
void ${lowname}_class_init(as_object& global)
{
	// This is going to be the global ${asname} "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(&${lowname}_ctor, get${asname}Interface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		attach${asname}Interface(*cl);
		     
	}

	// Register _global.${asname}
	global.init_member("${asname}", cl.get());

}


} // end of gnash namespace

EOF


