#!/bin/sh

top_builddir=$1
shift
testfiles=$@

cat << EOF
#!/bin/sh
for t in ${testfiles}; do
${top_builddir}/utilities/gprocessor -v \${t}
done
EOF
