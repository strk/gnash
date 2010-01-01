#!/bin/sh
#
#   Copyright (C) 2009, 2010 Free Software Foundation, Inc.
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

# Top level path
# Add some newlines to make sed;ing and grep'ing easier.
namespace="`basename $PWD`"
class=`basename $1 | sed -e "s:.html::"`
#sed -e "s/<div/\n<div/g" -e "s/<t/\n<t/g" -e "s/<a /\n<a /g" -e "s/&nbsp;/\n&nbsp;/g" http://livedocs.adobe.com/flash/9.0/ActionScriptLangRefV3/flash/media/Camera.html > tmp.html
			 
sed -e "s/<div/\n<div/g" -e "s/<t/\n<t/g" -e "s/<a /\n<a /g" -e "s/&nbsp;/\n&nbsp;/g" $1 > tmp.html
dos2unix -q tmp.html

# extract the properties from the web page
props="`sed -e '1,/MainContent/ d' -e '/Method Detail/,$d' tmp.html | grep "detailHeaderName" | sed -e 's:^.*detailHeaderName">::' -e 's:<.*::' | tr -s '\n' ' '`"
# extract the methods from the web page
methods="`sed -e '1,/methodDetail/ d' -e '/(summaryTableTitle|propertyDetail|eventDetail|ExamplesSummary|Event)/,$d' tmp.html | grep "detailHeaderName" | sed -e 's:^.*detailHeaderName">::' -e 's:<.*::'  | tr -s '\n' ' '`"
# extract the methods from the web page
events="`sed -e '1,/eventDetail/ d' -e '/(propertyDetail|Property Detail)/,$d' tmp.html | grep "detailHeaderName" | sed -e 's:^.*detailHeaderName">::' -e 's:<.*::'  | tr -s '\n' ' '`"

# ignore the package-detail files
if test x"${class}" = x"package-detail" -o x"${class}" = x"package"; then
  exit
fi

# display what we found
echo "Class: $class"
props_blanks=`echo $props | tr -d ' '`
if test  x"$props_blanks" != x; then
  echo "    Properties: \"$props\""
fi
meth_blanks=`echo $methods | tr -d ' '`
if test  x"$meth_blanks" != x; then
  echo "    Methods: \"$methods\""
fi


asname=${class}
lowname=`echo ${asname} | tr '[A-Z]' '[a-z]'`
upname=`echo ${asname}  | tr '[a-z]' '[A-Z]'`
outname=${asname}_as3.h
srcname=${asname}_as3.cpp

if test -f ${outname}; then
    echo ${outname} exists!
#    mv -f ${outname} ${outname}.orig
    exit 1;
fi

if test -f ${srcname}; then
    echo ${srcname} exists!
#    mv -f ${srcname} ${outname}.orig
    exit 1;
fi

###############################################################
# Start by generating the header file for this class
#
rm -f ${outname}
cat <<EOF>>${outname}
// ${outname}:  ActionScript 3 "${asname}" class, for Gnash.
//
//   Copyright (C) 2009, 2010 Free Software Foundation, Inc.
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

#ifndef GNASH_ASOBJ3_${upname}_H
#define GNASH_ASOBJ3_${upname}_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "fn_call.h"

// Forward declarations
class as_object;

namespace gnash {
namespace {
    as_object* get${asname}Interface();
}

class ${asname}_as3: public as_object
{

public:

    ${asname}_as3()
        :
        as_object(get${asname}Interface())
    {}

};

/// Initialize the global ${asname} class
void ${lowname}_class_init(as_object& global);

} // gnash namespace

// GNASH_ASOBJ3_${upname}_H
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

EOF
##############################################################
# now generate the source file
#

# start with the header part
cat <<EOF>>${srcname}
// ${srcname}:  ActionScript "${asname}" class, for Gnash.
//
//   Copyright (C) 2009, 2010 Free Software Foundation, Inc.
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

#include "${namespace}/${asname}_as3.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
EOF
for i in $methods; do
# DO NOT CONVERT CASE, SWF7+ is case-sensitive
newi=`echo $i | sed -e 's/)//g'` # | tr '[A-Z]' '[a-z]'
cat <<EOF>>${srcname}
    as_value ${lowname}_${newi}(const fn_call& fn);
EOF
done
cat <<EOF>>${srcname}
    as_value ${lowname}_ctor(const fn_call& fn);
    void attach${asname}Interface(as_object& o);
    void attach${asname}StaticInterface(as_object& o);
    as_object* get${asname}Interface();

}

// extern (used by Global.cpp)
void ${lowname}_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&${lowname}_ctor, get${asname}Interface());
        attach${asname}StaticInterface(*cl);
    }

    // Register _global.${asname}
    global.init_member("${asname}", cl.get());
}

namespace {

void
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

void
attach${asname}StaticInterface(as_object& o)
{

}

as_object*
get${asname}Interface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attach${asname}Interface(*o);
    }
    return o.get();
}
EOF

for i in $methods; do
# DO NOT CONVERT CASE, SWF7+ is case-sensitive
newi=`echo $i | sed -e 's/)//g'` # | tr '[A-Z]' '[a-z]'
cat <<EOF>>${srcname}

as_value
${lowname}_${newi}(const fn_call& fn)
{
    boost::intrusive_ptr<${asname}_as3> ptr =
        ensureType<${asname}_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}
EOF
done

cat <<EOF>>${srcname}

as_value
${lowname}_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new ${asname}_as3;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

EOF

rm -f tmp.html
