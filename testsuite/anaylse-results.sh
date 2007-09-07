#!/bin/sh


# case "$1" in
#     totals)
# 	totals ;;
#     *)
# 	exit ;;
# esac

total_fail=0;
total_pass=0;
total_xfail=0;
total_xpass=0;
total_untested=0;
total_unresolved=0;

echo
echo "[Test Results Summary]"
echo

# TODO1: always find in top-level dir instead (taking a parameter) ?
# TODO2: increment -maxdepth in case we add subdirs to our testsuites ?

#for dir in `find . -maxdepth 1 -type d | egrep -v ".libs|.deps" | grep "./" | sort`; do
suitefail=
suitexpass=
for dir in `find . -type d | egrep -v ".libs|.deps" | grep "./" | sort`; do
    if test ! -f "${dir}/testrun.sum" ; then
	continue
    fi
    nofail=`grep -c "^FAIL: "   ${dir}/testrun.sum`
    nopass=`grep -c "^PASS: "   ${dir}/testrun.sum`
    noxfail=`grep -c "^XFAIL: " ${dir}/testrun.sum`
    noxpass=`grep -c "^XPASS: " ${dir}/testrun.sum`
    nounresolved=`grep -c "^UNRESOLVED: " ${dir}/testrun.sum`
    nountested=`grep -c "^UNTESTED: " ${dir}/testrun.sum`
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

	if test ${total_fail} -gt 0; then
		echo "Unexpected failures follow:"
		for s in ${suitefail}; do
			echo -n "	${s}: "
			grep -w FAIL ${s}/testrun.sum;
		done
		echo
	fi

	if test ${total_xpass} -gt 0; then
		echo "Unexpected successes follow:"
		for s in ${suitexpass}; do
			echo -n "	${s}: "
			grep -w FAIL ${s}/testrun.sum; done
		echo
	fi

	exit 1
else
	exit 0
fi

#
# Look for regressions
#
