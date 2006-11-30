#!/bin/sh


## generate list for the wiki
{
for t in $@; do 
	testname=`basename $t .swf`
	{
	echo "== $testname =="
	echo 
	echo "Source: http://cvs.savannah.gnu.org/viewcvs/gnash/testsuite/actionscript.all/${testname}.as?root=gnash"
	echo 
	echo "Targets: "
	echo "[http://gnash.lulu.com/testcases/v5/${testname}-v5.swf SWF5] "
	echo "[http://gnash.lulu.com/testcases/v6/${testname}-v6.swf SWF6] "
	echo "[http://gnash.lulu.com/testcases/v7/${testname}-v7.swf SWF7] "
	echo "[http://gnash.lulu.com/testcases/v8/${testname}-v8.swf SWF8]"
	echo 
	} 
done; 
} > index.wiki
