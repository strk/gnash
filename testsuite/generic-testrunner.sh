#!/bin/sh

top_builddir=$1
testfile=$2

cat << EOF
#!/bin/sh
${top_builddir}/utilities/gprocessor -v ${testfile}
EOF
