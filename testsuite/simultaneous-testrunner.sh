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

while getopts c:C: name; do
  case $name in
    c) endtagpat="$OPTARG" ;;
    C) endtagpat="$OPTARG"; endtagexp=X ;;
    ?)
      {
      echo "Usage: $0 [-r <runs>] [-f <advances>] [-c <string>]  <swf> ..."
      echo "   -c <pattern>    : verify that the test ends with a trace "
      echo "                     matching <pattern>, or print a failure"
      echo "   -C <pattern>    : same as -c <pattern> but a failure is "
      echo "                     expected"
      } >&2
      exit 1;;
  esac
done
shift $(($OPTIND - 1))


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

timeout=40
cat << EOF

outlog1=${top_builddir}/testoutlog.\$$
outlog2=${top_builddir}/testoutlog2.\$$

echo "Running first process"
${top_builddir}/gui/gnash -vvv -r0 ${t1} -t ${timeout} | tee \${outlog1} &
echo "Running second process"
${top_builddir}/gui/gnash -vvv -r0 ${t2} -t ${timeout} | tee \${outlog2} &
wait

for log in \${outlog1} \${outlog2}; do
  if test "x${endtagpat}" != x; then
    lasttrace=\`grep TRACE \$log | tail -1 | sed 's/.*TRACE: //'\`
    if ! expr "\${lasttrace}" : '${endtagpat}' > /dev/null; then
      echo "${endtagexp}FAILED: consistency check: last trace from run of test \${t} (\${lasttrace}) doesn't match pattern (${endtagpat})"
    else
      echo "${endtagexp}PASSED: consistency check: last trace from run of test \${t} (\${lasttrace}) matches pattern (${endtagpat})"
    fi
  fi
  rm \$log
done
EOF
