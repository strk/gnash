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


top_builddir=$1
shift
t1=$1
shift
t2=$1


echo "#!/bin/sh"
echo

echo "# Environment"
env | grep GNASH | while read REPLY; do
	echo "export ${REPLY}"
done

timeout=30

cat << EOF

outlog1=${top_builddir}/testoutlog1.\$$
outlog2=${top_builddir}/testoutlog2.\$$
(
    echo "Running first process"
    exec >\${outlog1} ${top_builddir}/gui/gnash -v -r0 ${t1} -t ${timeout}
    cat \${outlog1}
    rm \${outlog1}
) &
(
    echo "Running second process"
    exec >\${outlog2} ${top_builddir}/gui/gnash -v -r0 ${t2} -t ${timeout}
    cat \${outlog2}
    rm \${outlog2}
)
EOF
