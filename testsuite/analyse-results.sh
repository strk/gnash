#!/bin/sh

mode=$1

total_fail=0;
total_pass=0;
total_xfail=0;
total_xpass=0;
total_untested=0;
total_unresolved=0;

echo
echo "[Test Results Summary]"
echo

suitefail=
suitexpass=
for fn in `find testsuite extensions -name testrun.sum | sort`; do
    dir=`dirname $fn`
    nofail=`grep -c "^FAIL: "   ${fn}`
    nopass=`grep -c "^PASS: "   ${fn}`
    noxfail=`grep -c "^XFAIL: " ${fn}`
    noxpass=`grep -c "^XPASS: " ${fn}`
    nounresolved=`grep -c "^UNRESOLVED: " ${fn}`
    nountested=`grep -c "^UNTESTED: " ${fn}`
    echo -n "Test suite $dir: "
    someprint=0
    if test $nofail -gt 0; then
	suitefail="${suitefail} ${dir}"
	echo -n "$nofail real failures"
	total_fail=`expr $total_fail + $nofail`
	someprint=1
    fi
    if test $noxpass -gt 0; then
	suitexpass="${suitexpass} ${dir}"
	if test $someprint -gt 0; then echo -n ", "; fi
	echo -n "$noxpass unexpected successes"
	total_xpass=`expr $total_xpass + $noxpass`
	someprint=1
    fi
    if test $nopass -gt 0; then
	if test $someprint -gt 0; then echo -n ", "; fi
	echo -n "$nopass passes"
	total_pass=`expr $total_pass + $nopass`
	someprint=1
    fi
    if test $noxfail -gt 0; then
	if test $someprint -gt 0; then echo -n ", "; fi
	echo -n "${noxfail} expected failure"
	total_xfail=`expr $total_xfail + $noxfail`
	someprint=1
    fi
    if test ${nounresolved} -gt 0; then
	if test $someprint -gt 0; then echo -n ", "; fi
	echo -n "${nounresolved} unresolved"
	total_unresolved=`expr ${total_unresolved} + ${nounresolved}`
	someprint=1
    fi
    if test ${nountested} -gt 0; then
	if test $someprint -gt 0; then echo -n ", "; fi
	echo -n "${nountested} untested"
	total_untested=`expr ${total_untested} + ${nountested}`
	someprint=1
    fi
    if test ${nofail} -gt 0 || test ${noxpass} -gt 0; then echo -n " *"; fi
    echo
done

echo
echo "Test Result Totals:"
if test ${total_pass} -gt 0; then
    echo "	Total passes: $total_pass"
fi
if test ${total_unresolved} -gt 0; then
    echo "	Total unresolved: $total_unresolved"
fi
if test ${total_xfail} -gt 0; then
    echo "	Total expected failures: ${total_xfail}"
fi
if test ${total_untested} -gt 0; then
    echo "	Total untested: ${total_untested}"
fi
if test ${total_fail} -gt 0; then
    echo "	* Total real failures: $total_fail"
fi
if test ${total_xpass} -gt 0; then
    echo "	--> Total unexpected successes: ${total_xpass}"
fi

echo

# For now, return a failure if any XPASS or FAIL occurred
if test ${total_fail} -gt 0 || test ${total_xpass} -gt 0; then

	rc=1
	timing=$(dirname $0)/timingissues
	for t in ${suitefail}; do
		testruns="${testruns} ${t}/testrun.sum"
	done
	failures="grep -hw FAIL ${testruns}"

	if test ${total_fail} -gt 0; then
		echo "Unexpected failures follow:"
		for s in ${suitefail}; do
			echo " --=[ ${s} ]=-- "
			grep -w FAIL ${s}/testrun.sum
		done
		echo
	fi

	if test ${total_xpass} -gt 0; then
		echo "Unexpected successes follow:"
		for s in ${suitexpass}; do
			echo " --=[ ${s} ]=-- "
			grep -w XPASS ${s}/testrun.sum
		done
		echo
	fi

	if test `$failures | grep -cf $timing` -gt 0; then
		if test `$failures | grep -cvf $timing` -eq 0; then
			echo "All failures are time-related. Exiting 0."
			echo "See http://wiki.gnashdev.org/PredictableLoading"
			echo
			rc=0
		else
			echo "Time-related failures follow:"
			$failures | grep -f $timing
			echo
			echo "See http://wiki.gnashdev.org/PredictableLoading"
			echo
			echo "Non time-related failures follow:"
			$failures | grep -vf $timing
			echo
		fi
	fi

	if [ "$mode" = "verbose" ]; then
		if test ${total_fail} -gt 0; then
			echo "Verbose mode enabled. Displaying following files:"
			echo
			swfdecfail=$($failures | sed -n 's/^.*in \(.*.trace-gnash\).*in \(.*.log\).*$/\1\n\2/p')
			for log in $swfdecfail; do
				echo " testsuite/swfdec/$log"
				logfiles="${logfiles} testsuite/swfdec/$log"
			done
			for s in ${suitefail}; do
				echo " ${s}/testrun.log"
				logfiles="${logfiles} ${s}/testrun.log"
			done
			echo
			for logfile in ${logfiles}; do
				echo "= = = = = = = [ ${logfile} file - BEGIN ] = = = = = = ="
				cat ${logfile}
				echo "= = = = = = = [ ${logfile} file - END   ] = = = = = = ="
				echo
			done
		fi
	fi

	exit $rc
else
	exit 0
fi

#
# Look for regressions
#
