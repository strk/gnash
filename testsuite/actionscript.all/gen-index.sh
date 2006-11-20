#!/bin/sh

## generate index.html
{
echo "<html><head><title>Gnash - ActionScript tests</title></head><body>" 
echo "<h1>Gnash - ActionScript tests</h1>"
echo "<h2>All tests in a single SWF movie</h2>"
echo "<p>"
echo "<ul>"
echo "<li><a href=alltests.swf>alltests.swf</a></li>"
echo "</ul>"
echo "</p>"
echo "<p>"
echo "This test is likely to overflow the 'visual traces' window."
echo "If this is the case (you can't see the final report about number"
echo "of succeeded / failed tests) consider running the single tests "
echo "instead. We'll need to fix this."
echo "</p>"
echo "<h2>One SWF for each test</H2>"
echo "<p>"
echo "<ul>"
for t in $@; do 
	if test "$t" = "alltests.swf"; then
		continue # we list it as first
	fi
	echo "<li><a href=$t>$t</a></li>" 
done; 
echo "</p>"
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
