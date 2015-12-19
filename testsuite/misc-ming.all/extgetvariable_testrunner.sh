#!/bin/sh

# 
# extgetvariable_testrunner.sh, container-emulated, automated
#     GetVariable plugin function test generator
# 
# Copyright (C) 2015 Free Software Foundation, Inc.
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
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
# 
# 
# Original author: Nutchanon Wetchasit <Nutchanon.Wetchasit@gmail.com>
# 
# This test runner checks Gnash for:
#  * GetVariable() datatype issues (bug #42395)
#        <https://savannah.gnu.org/bugs/?42395>
#  * Timeline variable declaration issue (bug #45840) in SWF5 environment
#        <https://savannah.gnu.org/bugs/?45840>
# 
# Usage:
#     ./extgetvariable_testrunner.sh <builddir> <srcdir> <swfversion> <swf>
# 
# Generated test runner's exit codes:
#     0         if tester ran completely
#     non-zero  if tester encountered an error
# 
# Note:
#     The generated test file requires a filesystem that supports named pipes.
# 

# Check for generation parameters
while getopts "" name
do
	case $name in
		?)
			echo "Usage: $0 <builddir> <srcdir> <swfversion> <swf>" >&2
			exit 1;;
	esac
done
shift $(($OPTIND - 1))
if [ "$#" -ne 4 ]
then
	echo "Usage: $0 <builddir> <srcdir> <swfversion> <swf>" >&2
	exit 1
fi

# Load generation parameters
top_builddir=$1
shift
top_srcdir=$1
shift
swfversion=$1
shift
swf=$1

# Generate the test runner
echo "#!/bin/sh"
echo

echo "# Environment variables"
env | grep '^GNASH' | while read reply
do
	echo "export \"${reply}\""
done

cat << EOF

# Filenames and constants
LOGFILE=${top_builddir}/testoutlog.\$\$
PIPE2CONTAINER=${top_builddir}/tocontainer.\$\$
PIPE2PLAYER=${top_builddir}/toplayer.\$\$
READTIMEOUT=5

# Test counts
TESTED=0
FAILED=0
PASSED=0

# check_equals(\$op1, \$op2, \$msg)
# Equality checker and counter
check_equals() {
	if [ "\$1" = "\$2" ]
	then
		echo "PASSED: \$3"
		PASSED=\`expr "\$PASSED" + 1\`
	else
		echo "FAILED: \$3 (\"\$1\" != \"\$2\")"
		FAILED=\`expr "\$FAILED" + 1\`
	fi
	TESTED=\`expr "\$TESTED" + 1\`
}

# xcheck_equals(\$op1, \$op2, \$msg)
# Equality checker and counter (for expected failure)
xcheck_equals() {
	if [ "\$1" = "\$2" ]
	then
		echo "XPASSED: \$3"
		PASSED=\`expr "\$PASSED" + 1\`
	else
		echo "XFAILED: \$3 (\"\$1\" != \"\$2\")"
		FAILED=\`expr "\$FAILED" + 1\`
	fi
	TESTED=\`expr "\$TESTED" + 1\`
}

# check_totals(\$op, \$msg)
# Test count checker
check_totals() {
	check_equals "\$TESTED" "\$1" "\$2"
}

# check_error(\$bool, \$msg)
# Assert \$bool is 0; if not, flag error in the test, and exit
check_error() {
	if [ "\$1" -ne 0 ]
	then
		echo "ERROR: \$2" >&2
		exit 1
	fi
}

# read_timeout(\$varname, \$timeout)
# Read one line from standard input, with a specified timeout (in seconds)
read_timeout() {
	trap 'trap - USR1; return 142' USR1
	(sleep "\$2" && kill -USR1 "\$\$" > /dev/null 2>&1) &
	TIMEOUTPID=\$!
	read "\$1"
	READERROR=\$?
	kill "\$TIMEOUTPID" > /dev/null 2>&1
	trap - USR1
	return \$READERROR
}

# Create required named pipes
if [ \! -p "\$PIPE2CONTAINER" ]
then
	mkfifo "\$PIPE2CONTAINER"
	check_error "\$?" "Failed to create a named pipe: \$PIPE2CONTAINER"
fi
if [ \! -p "\$PIPE2PLAYER" ]
then
	mkfifo "\$PIPE2PLAYER"
	check_error "\$?" "Failed to create a named pipe: \$PIPE2PLAYER"
fi

# Open player-to-host pipe
exec 3<> "\$PIPE2CONTAINER"
check_error \$? "Failed to open a named pipe: \$PIPE2CONTAINER"

# Open host-to-player pipe
exec 4<> "\$PIPE2PLAYER"
check_error \$? "Failed to open a named pipe: \$PIPE2PLAYER"

# Start player
"${top_builddir}/gui/gnash" -r 0 -vv -F 3:4 "${swf}" > "\$LOGFILE" 2>&1 &
GNASHPID=\$!

# Wait until the SWF code finish running, by loop-checking logfile
STARTCOUNTDOWN=\$READTIMEOUT
while [ \$STARTCOUNTDOWN -gt 0 ]
do
	if grep "TRACE: ENDOFTEST" "\$LOGFILE" 2>&1 > /dev/null
	then
		break
	fi
	sleep 1
	STARTCOUNTDOWN=\`expr \$STARTCOUNTDOWN - 1\`
done

[ \$STARTCOUNTDOWN -ne 0 ]
check_equals \$? 0 "Gnash-side ActionScript code should be successfully run"

# Call string-returning GetVariable() on string variable
echo '<invoke name="GetVariable" returntype="xml"><arguments><string>string_variable</string></arguments></invoke>' >&4

# Read for value statement
read_timeout LINE \$READTIMEOUT <&3
check_equals "\$LINE" '<string>This is a string</string>' "Gnash should return a correct value from GetVariable call on string"

# Call string-returning GetVariable() on integer variable
echo '<invoke name="GetVariable" returntype="xml"><arguments><string>integer_variable</string></arguments></invoke>' >&4

# Read for return value statement
read_timeout LINE \$READTIMEOUT <&3
check_equals "\$LINE" '<string>9876</string>' "Gnash should return a correct value from GetVariable call on integer"

# Call string-returning GetVariable() on floating-point variable
echo '<invoke name="GetVariable" returntype="xml"><arguments><string>float_variable</string></arguments></invoke>' >&4

# Read for return value statement
read_timeout LINE \$READTIMEOUT <&3
check_equals "\$LINE" '<string>9876.5432</string>' "Gnash should return a correct value from GetVariable call on floating point"

# Call string-returning GetVariable() on positive infinite
# floating-point variable
echo '<invoke name="GetVariable" returntype="xml"><arguments><string>infinite_variable</string></arguments></invoke>' >&4

# Read for return value statement
read_timeout LINE \$READTIMEOUT <&3
check_equals "\$LINE" '<string>Infinity</string>' "Gnash should return a correct value from GetVariable call on infinity floating point"

# Call string-returning GetVariable() on negative infinite
# floating-point variable
echo '<invoke name="GetVariable" returntype="xml"><arguments><string>neginfinite_variable</string></arguments></invoke>' >&4

# Read for return value statement
read_timeout LINE \$READTIMEOUT <&3
check_equals "\$LINE" '<string>-Infinity</string>' "Gnash should return a correct value from GetVariable call on negative infinity floating point"

# Call string-returning GetVariable() on non-number floating-point variable
echo '<invoke name="GetVariable" returntype="xml"><arguments><string>nan_variable</string></arguments></invoke>' >&4

# Read for return value statement
read_timeout LINE \$READTIMEOUT <&3
check_equals "\$LINE" '<string>NaN</string>' "Gnash should return a correct value from GetVariable call on non-number floating point"

# Call string-returning GetVariable() on boolean variable
echo '<invoke name="GetVariable" returntype="xml"><arguments><string>boolean_variable</string></arguments></invoke>' >&4

# Read for return value statement
read_timeout LINE \$READTIMEOUT <&3
check_equals "\$LINE" '<string>true</string>' "Gnash should return a correct value from GetVariable call on boolean"

# Call string-returning GetVariable() on null variable
echo '<invoke name="GetVariable" returntype="xml"><arguments><string>null_variable</string></arguments></invoke>' >&4

# Read for return value statement
read_timeout LINE \$READTIMEOUT <&3
check_equals "\$LINE" '<string>null</string>' "Gnash should return a correct value from GetVariable call on null"

# Call string-returning GetVariable() on unassigned variable
echo '<invoke name="GetVariable" returntype="xml"><arguments><string>unassigned_variable</string></arguments></invoke>' >&4

# Read for return value statement
read_timeout LINE \$READTIMEOUT <&3
if [ "${swfversion}" -gt 5 ]
then
	check_equals "\$LINE" '<string>undefined</string>' "Gnash should return a correct value from GetVariable call on unassigned variable"
else
	xcheck_equals "\$LINE" '<string></string>' "Gnash should return a correct value from GetVariable call on unassigned variable"
fi

# Call string-returning GetVariable() on variable with undefined value
echo '<invoke name="GetVariable" returntype="xml"><arguments><string>undefined_variable</string></arguments></invoke>' >&4

# Read for return value statement
read_timeout LINE \$READTIMEOUT <&3
if [ "${swfversion}" -gt 5 ]
then
	check_equals "\$LINE" '<string>undefined</string>' "Gnash should return a correct value from GetVariable call on variable with undefined value"
else
	xcheck_equals "\$LINE" '<string></string>' "Gnash should return a correct value from GetVariable call on variable with undefined value"
fi

# Call string-returning GetVariable() on non-existent variable
echo '<invoke name="GetVariable" returntype="xml"><arguments><string>nonexistent_variable</string></arguments></invoke>' >&4

# Read for return value statement
read_timeout LINE \$READTIMEOUT <&3
check_equals "\$LINE" '<null/>' "Gnash should return a correct value from GetVariable call on non-existent variable"

# Call string-returning GetVariable() on string array variable
echo '<invoke name="GetVariable" returntype="xml"><arguments><string>array_variable</string></arguments></invoke>' >&4

# Read for return value statement
read_timeout LINE \$READTIMEOUT <&3
check_equals "\$LINE" '<string>The,quick,brown,fox,jumps,over,the,lazy,dog</string>' "Gnash should return a correct value from GetVariable call on array variable"

# Call string-returning GetVariable() on object variable
echo '<invoke name="GetVariable" returntype="xml"><arguments><string>object_variable</string></arguments></invoke>' >&4

# Read for return value statement
read_timeout LINE \$READTIMEOUT <&3
check_equals "\$LINE" '<string>[object Object]</string>' "Gnash should return a correct value from GetVariable call on object variable"

# Call string-returning GetVariable() on object variable
# with custom toString() method
echo '<invoke name="GetVariable" returntype="xml"><arguments><string>object_variable_customstring</string></arguments></invoke>' >&4

# Read for return value statement
read_timeout LINE \$READTIMEOUT <&3
check_equals "\$LINE" '<string>This is a custom Object.toString()</string>' "Gnash should return a correct value from GetVariable call on object variable with custom toString()"

# Call string-returning GetVariable() on function variable
echo '<invoke name="GetVariable" returntype="xml"><arguments><string>function_variable</string></arguments></invoke>' >&4

# Read for return value statement
read_timeout LINE \$READTIMEOUT <&3
check_equals "\$LINE" '<string>[type Function]</string>' "Gnash should return a correct value from GetVariable call on function variable"

# Close pipes
exec 3<&-
exec 4<&-

# Force Gnash to exit
kill \$GNASHPID
wait \$GNASHPID

# Check for total number of test run
check_totals "16" "There should be 16 tests run"

# Remove temporary files
rm "\$LOGFILE"
rm "\$PIPE2CONTAINER"
rm "\$PIPE2PLAYER"
EOF
