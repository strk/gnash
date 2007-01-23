#!/bin/sh

# 
#   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
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

while getopts r: name; do
	case $name in
		r) runs="$OPTARG" ;;
		?) echo "Usage: $0 [-r <runs>] <swf> ..." >&2;
		   exit 1;;
	esac
done
shift $(($OPTIND - 1))

top_builddir=$1
shift
testfiles=$@

cat << EOF
#!/bin/sh
for t in ${testfiles}; do
	echo "NOTE: Running test \${t}"
	${top_builddir}/utilities/gprocessor -r${runs} -v \${t} || echo "FAILED: gprocessor returned an error while playing '\${t}'"
done
EOF
