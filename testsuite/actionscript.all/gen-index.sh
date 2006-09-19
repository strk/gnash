#!/bin/sh

## generate index.html
{
echo "<html><head><title>Test units</title></head><body>" 
echo "<a href=embed.html>all tests in a single page</a>"
echo "<ul>"
for t in $@; do 
	echo "<li><a href=$t>$t</a></li>" 
done; 
echo "</body></html>" 
} > index.html

## generate embed.html
{

cat << EOF
<html><head><title>Test units ebedded</title></head><body>

<p>If you see any line printed in any box (apart from player and testcase
version) that means there are failures. If you're using the proprietary
flash player this also means the testcases are bogus, so we really
want to fix them. So, please copy & paste the full box text and file a
<a href='https://savannah.gnu.org/bugs/?func=additem&group=gnash'>bug
report</a> in that case.</p>
EOF

for t in $@; do 
	echo "<embed src=$t>"
done; 
echo "</body></html>" 
} > embed.html

## generate list for the wiki
{
for t in $@; do 
	testname=`basename $t .swf`
	{
	echo "== $testname =="
	echo 
	echo "[http://www.gnu.org/software/gnash/testcases/v5/$t SWF5] "
	echo "[http://www.gnu.org/software/gnash/testcases/v6/$t SWF6] "
	echo "[http://www.gnu.org/software/gnash/testcases/v7/$t SWF7] "
	echo "[http://www.gnu.org/software/gnash/testcases/v8/$t SWF8]"
	echo 
	} 
done; 
} > index.wiki
