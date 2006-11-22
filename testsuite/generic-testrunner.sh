#!/bin/sh

top_builddir=$1
shift
testfiles=$@

cat << EOF
#!/bin/sh
for t in ${testfiles}; do
	echo "NOTE: Running test \${t}"
	${top_builddir}/utilities/gprocessor -v \${t} || echo "FAILED: gprocessor returned an error"
done
EOF
