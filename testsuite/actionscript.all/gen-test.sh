#!/bin/sh

# 
#   Copyright (C) 2005, 2006, 2009, 2010 Free Software Foundation, Inc.
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
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

# This script generates a Ming style ".as" source file, which can be
# compiled using makeswf, and then run through Gnash. The test cases
# use the "trace()" function in Flash, so these messages are only
# visible when using the "-v" option to gnash. To run the resultant
# test case, invoke it like this "gnash -v -1 -r 0 Camera.swf". The
# "-r 0" turns off the renderer, so we don't get windows popping up
# for test cases that are purely text driven. The "-1" options says
# run this file only once, instead of the default, which is to play
# the movie indefinetly. The "-v" options has Gnash print the messages
# from trace(). For now, all these generated test cases do is test for
# the existance of a class and it's methods.

# This script depends on the doc/C/NOTES file for data. It takes a
# single argument, which is the name of the class, like "./gen-test.sh
# Camera". 
#
# This script is only of use to developers, so it's "as is". Your
# mileage may vary.

asname=$1
lowname=`echo ${asname} | tr '[A-Z]' '[a-z]'`
upname=`echo ${asname}  | tr '[a-z]' '[A-Z]'` 
outname=${asname}.as
asobjname="${lowname}Obj";

if test -f ${outname}; then
    echo ${outname} exists!
    exit 1;
fi

notes=../../doc/C/NOTES
#methods=`grep "${asname}\..*()" ${notes} | sed -e 's/${asname}\.//g'`
methods=`grep "${asname}\..*()" ${notes}`
methods=`echo ${methods} | sed -e s/${asname}.//g`
#props=`grep "${asname}\." ${notes} | grep -v "()" | sed -e 's/${asname}\.//g'`
props=`grep "${asname}\." ${notes} | grep -v "()"`
props=`echo ${props} | sed -e s/${asname}.//g`

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
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License

// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

// Test case for ${asname} ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

#include "check.as"

var ${asobjname} = new ${asname};

// test the ${asname} constuctor
check (${asobjname} != undefined);

EOF

for i in $methods; do
newi=`echo $i | sed -e 's/)//g' | sed -e 's/(//g' -e 's/)//g'`
cat <<EOF>>${outname}
// test the ${asname}::${newi} method
check (${asobjname}.$newi != undefined);
EOF
done
