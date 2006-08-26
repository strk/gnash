#!/bin/sh

# 
#   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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

notes=../doc/C/NOTES
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

#ifndef __${upname}_H__
#define __${upname}_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "impl.h"
#include "as_object.h" // for inheritance

namespace gnash {
  
class ${asname} {
public:
    ${asname}();
    ~${asname}();
EOF

for i in $methods; do
cat <<EOF>>${outname}
   void ${i};
EOF
done

# trailer
cat <<EOF>>${outname}
private:
EOF

# write the properties as variables
for i in $props; do

cat <<EOF>>${outname}
    bool _$i;
EOF
done

cat <<EOF>>${outname}
};

struct ${lowname}_as_object : public as_object
{
    ${asname} obj;
};

EOF

for i in $methods; do
newi=`echo $i | sed -e 's/)//g' | tr '[A-Z]' '[a-z]'`
cat <<EOF>>${outname}
void ${lowname}_${newi}const fn_call& fn);
EOF
done

cat <<EOF>>${outname}

} // end of gnash namespace

// __${upname}_H__
#endif

EOF

##############################################################
# now generate the source file
#

# start with the header part
cat <<EOF>>${srcname}
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "${asname}.h"
#include "fn_call.h"

namespace gnash {

${asname}::${asname}() {
}

${asname}::~${asname}() {
}

EOF

# now process the methods
#newi=`echo $i | sed -e 's/)//g'`
for i in $methods; do
    cat <<EOF>>${srcname}

void
${asname}::${i}
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
EOF
done

cat <<EOF>>${srcname}
void
${lowname}_new(const fn_call& fn)
{
    ${lowname}_as_object *${lowname}_obj = new ${lowname}_as_object;

EOF

# do the methods as callbacks
for i in $methods; do
    newi=`echo $i | sed -e 's/()//g' | tr '[A-Z]' '[a-z]'`
    cat <<EOF>>${srcname}
    ${lowname}_obj->set_member("${newi}", &${lowname}_${newi});
EOF
done

# close the function definition
cat <<EOF>>${srcname}

    fn.result->set_as_object(${lowname}_obj);
}
EOF

for i in $methods; do
newi=`echo $i | sed -e 's/)//g' | tr '[A-Z]' '[a-z]'`
cat <<EOF>>${srcname}
void ${lowname}_${newi}const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
EOF
done

cat <<EOF>>${srcname}

} // end of gnash namespace

EOF


