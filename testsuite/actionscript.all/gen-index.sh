#!/bin/sh

echo "<html><head><title>Test units</title></head><body>" > index.html
for t in $@; do 
	echo "<li><a href=$t>$t</a></li>" >> index.html; 
done; 
echo "</body></html>" >> index.html
