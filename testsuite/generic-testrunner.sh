#!/bin/sh

# 
#   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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

while getopts r:f: name; do
	case $name in
		r) runs="$OPTARG" ;;
		f) advances="$OPTARG" ;;
		?) echo "Usage: $0 [-r <runs>] [-f <advances>] <swf> ..." >&2;
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
env | grep GNASH | while read; do
	echo "export ${REPLY}"
done

cat << EOF

for t in ${testfiles}; do
	echo "NOTE: Running test \${t}"
	${top_builddir}/utilities/gprocessor -r${runs} -f${advances} -v \${t} || echo "FAILED: gprocessor returned an error while playing '\${t}'"
done
EOF
