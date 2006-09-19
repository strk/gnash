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
<html>
<head><title>Test units ebedded</title></head>
<body onLoad='javascript:window.open("","report")'>

<p>
Testcases are (hopefully) being run by your browser and plugin.
In a few seconds a new window should pop up pointing to the logger
application. If nothing happens please
<a href='https://savannah.gnu.org/bugs/?func=additem&group=gnash'>
file a bug report
</a>.
</p>
EOF

for t in $@; do 
	echo "<embed src=$t width=1 height=1>"
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
