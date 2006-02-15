#!/bin/sh

## generate index.html
echo "<html><head><title>Test units</title></head><body>" > index.html
for t in $@; do 
	echo "<li><a href=$t>$t</a></li>" >> index.html; 
done; 
echo "</body></html>" >> index.html

## generate list for the wiki
echo > index.wiki
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
	} >> index.wiki
done; 
