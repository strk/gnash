#!/bin/sh


## generate list for the wiki
{
for t in $@; do 
	testname=`basename $t .swf`
	{
	echo "== $testname =="
	echo 
	echo "Source: http://git.sv.gnu.org/gitweb/?p=gnash.git;a=blob;f=testsuite/actionscript.all/${testname}.as;hb=HEAD"
	echo 
	echo "Targets: "
	echo "[http://gnashdev.org/testcases/v5/${testname}-v5.swf SWF5] "
	echo "[http://gnashdev.org/testcases/v6/${testname}-v6.swf SWF6] "
	echo "[http://gnashdev.org/testcases/v7/${testname}-v7.swf SWF7] "
	echo "[http://gnashdev.org/testcases/v8/${testname}-v8.swf SWF8]"
	echo 
	} 
done; 
} > index.wiki
