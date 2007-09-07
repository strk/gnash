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
    if test $nofail -gt 0; then
	echo -n " $nofail real failures"
	total_fail=`expr $total_fail + $nofail`
    fi
    if test $noxpass -gt 0; then
	if test $nofail -gt 0; then
	    echo -n ", $noxpass unexpected successes"
	else
	    echo -n " $noxpass unexpected successes"
	fi
	total_xpass=`expr $total_xpass + $noxpass`
    fi
    if test $nopass -gt 0; then
	if test $noxpass -gt 0; then
	    echo -n ", $nopass passes"
	else
	    echo -n "$nopass passes"
	fi
	total_pass=`expr $total_pass + $nopass`
    fi
    if test $noxfail -gt 0; then
	if test $nofail -gt 0 -o $nopass -gt 0; then
	    echo -n ", ${noxfail} expected failures"
	else
	    echo -n "${noxfail} expected failure"
	fi
	total_xfail=`expr $total_xfail + $noxfail`
    fi
    if test ${nounresolved} -gt 0; then
	if test $nofail -gt 0 -o $nopass -gt 0 -o $noxfail -gt 0; then
	    echo -n ", ${nounresolved} unresolved"
	else
	    echo -n "${nounresolved} unresolved"
	fi
	total_unresolved=`expr ${total_unresolved} + ${nounresolved}`
    fi
    if test ${nountested} -gt 0; then
	if test $nofail -gt 0 -o $nopass -gt 0 -o $noxfail -gt 0 -o $nounresolved -gt 0; then
	    echo -n ", ${nountested} untested"
	else
	    echo -n "${nountested} untested"
	fi
	total_untested=`expr ${total_untested} + ${nountested}`
    fi
    echo
done

echo
echo "Test Result Totals:"
if test ${total_fail} -gt 0; then
    echo "	Total real failures: $total_fail"
fi
if test ${total_pass} -gt 0; then
    echo "	Total passes: $total_pass"
fi
if test ${total_unresolved} -gt 0; then
    echo "	Total unresolved: $total_unresolved"
fi
if test ${total_xfail} -gt 0; then
    echo "	Total expected failures: ${total_xfail}"
fi
if test ${total_xpass} -gt 0; then
    echo "	Total unexpected successes: ${total_xpass}"
fi
if test ${total_untested} -gt 0; then
    echo "	Total untested: ${total_untested}"
fi

echo

#
# Look for regressions
#
