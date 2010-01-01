#!/bin/sh

# 
#   Copyright (C) 2008, 2009, 2010 Free Software Foundation, Inc.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
# 

#
# This script will copy all the SOL files found in the INPUTSOLDIR
# to the actual runtime sol dir, play a file and then compare saved
# sol files at the end with input ones.
#
# Called w/out arguments will use gprocessor as player, SharedObjectTest.sol/
# as input sol dir and testsuite/tmpSharedObject as runtime sol dir.
#
# You can pass arguments to change player and runtime sol dir for testing
# with the adobe player. Example:
#
# $ ./SharedObjectTestRunner flashplayer '/home/strk/.macromedia/Flash_Player/#SharedObjects/KZNLGTEU/'
#
# The first directory (usually "/home/") is stripped, as the proprietary
# player and now Gnash both do this.
# (as this is what seems to happen)
#
#

TOP_BUILDDIR=@@TOP_BUILDDIR@@
BASEINPUTSOLDIR=@@BASEINPUTSOLDIR@@
PLAYER="${TOP_BUILDDIR}/utilities/gprocessor -v"

INPUTSOLDIR=${BASEINPUTSOLDIR}
SWFTEST=${TOP_BUILDDIR}/testsuite/misc-ming.all/SharedObjectTest.swf
homestrip=`echo ${SWFTEST} | sed -e 's/\/[^\/]*//'`
SOLDIR=${TOP_BUILDDIR}/testsuite/tmpSharedObject/localhost/${homestrip}/

if [ -n "$1" ]; then PLAYER="$1"; fi
if [ -n "$2" ]; then
    SOLDIR="$2/localhost/${homestrip}";
fi

mkdir -p ${SOLDIR} # just in case..

rm -f ${SOLDIR}/* # clean target soldir out
cp -v ${INPUTSOLDIR}/*.sol ${SOLDIR}/
chmod 600 ${SOLDIR}/*.sol
touch ${SOLDIR}/copytime
echo "SWF=${SWFTEST}"
echo "INPUT=${INPUTSOLDIR}"
echo "SOLDIR=${SOLDIR}"
sleep 1

export GNASHRC=${TOP_BUILDDIR}/testsuite/gnashrc
export TZ=GMT

#####################################################
##
## FIRST STEP: test reading well-known .sol file
##
#####################################################

${PLAYER} ${SWFTEST}

#####################################################
##
## SECOND STEP: test writing
##
##   test that written sol files
##   are bytewise equal to input ones
##
#####################################################

cd ${SOLDIR}; find . -cnewer ${SOLDIR}/copytime | sed 's@^./@@' > ${SOLDIR}/newfiles; cd -

for f in ${INPUTSOLDIR}/*.sol; do
	solname=`basename ${f}`
	if ! grep -w ${solname} ${SOLDIR}/newfiles; then
		echo "FAILED: ${solname} wasn't saved"
		continue
	fi
	if ! cmp ${INPUTSOLDIR}/${solname} ${SOLDIR}/${solname}; then
		echo "FAILED: ! cmp ${SOLDIR}/${solname} ${INPUTSOLDIR}/${solname}"
		continue
	fi
	echo "PASSED: SharedObject ${solname} matches input"
done

#####################################################
##
## THIRD STEP: test re-reading just-written sol files
##
#####################################################

# ( temporarly disabled )
${PLAYER} ${SWFTEST}
