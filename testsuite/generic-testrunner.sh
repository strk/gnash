#!/bin/sh

# 
#   Copyright (C) 2005, 2006, 2009, 2010 Free Software Foundation, Inc.
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

runs=1
advances=0
delay=1 # milliseconds between advances
endtagpat=
endtagexp=""

while getopts r:f:c:C:d: name; do
	case $name in
		r) runs="$OPTARG" ;;
		f) advances="$OPTARG" ;;
		c) endtagpat="$OPTARG" ;;
		C) endtagpat="$OPTARG"; endtagexp=X ;;
		d) delay="$OPTARG" ;;
		?)
		   {
		   echo "Usage: $0 [-r <runs>] [-f <advances>] [-c <string>]  <swf> ..." 
		   echo "   -r <runs>       : allow for <runs> jump-backs" 
		   echo "   -f <advances>   : only advance <advances> times" 
		   echo "   -c <pattern>    : verify that the test ends with a trace matching <pattern>, or print a failure" 
		   echo "   -C <pattern>    : same as -c <pattern> but a failure is expected" 
		   } >&2
		   exit 1;;
	esac
done
shift $(($OPTIND - 1))

top_builddir=$1
shift
testfiles=$@

echo "#!/bin/sh"
echo

echo "# Environment"
env | grep GNASH | while read REPLY; do
	echo "export ${REPLY}"
done

cat << EOF

for t in ${testfiles}; do
	#outlog=\${t}.output.\$$
	outlog=${top_builddir}/testoutlog.\$$
	echo "NOTE: Running test \${t}"
	( 
		exec > \${outlog} 2>&1
		${top_builddir}/utilities/gprocessor -d${delay} -r${runs} -f${advances} -v \${t} || echo "FAILED: gprocessor returned an error while playing '\${t}'"
	)
	cat \${outlog}
	if test "x${endtagpat}" != x; then
		lasttrace=\`grep TRACE \${outlog} | tail -1 | sed 's/.*TRACE: //'\`
		if ! expr "\${lasttrace}" : '${endtagpat}' > /dev/null; then
			echo "${endtagexp}FAILED: consistency check: last trace from run of test \${t} (\${lasttrace}) doesn't match pattern (${endtagpat})"
		else
			echo "${endtagexp}PASSED: consistency check: last trace from run of test \${t} (\${lasttrace}) matches pattern (${endtagpat})"
		fi
	fi
	rm \${outlog}
done
EOF
