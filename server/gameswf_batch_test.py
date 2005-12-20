#!python

# gameswf_batch_test.py -Thatcher Ulrich <tu@tulrich.com> 2005

# This source code has been donated to the Public Domain.  Do
# whatever you want with it.

# Script for batch regression tests on gameswf.

import string
import sys
import commands
import difflib

GAMESWF = "./gameswf_test_ogl"
BATCH_ARGS = " -r 0 -1 -t 10 -v "

# Test files and their expected output
testlist = [
[ "samples/test_frame1.swf", '''frame 1 actions (1)
onLoad
onEnterFrame 2
onEnterFrame 3
onEnterFrame 4
onEnterFrame 5
onEnterFrame 6
onEnterFrame 7
onEnterFrame 8
onEnterFrame 9
onEnterFrame 10
frame 10 actions pre
frame 5 actions post
onEnterFrame 6
onEnterFrame 7
onEnterFrame 8
onEnterFrame 9
onEnterFrame 10
frame 10 actions pre
frame 1 actions post
frame 1 actions (1)
onEnterFrame 2
onEnterFrame 3
onEnterFrame 4
onEnterFrame 5
onEnterFrame 6
onEnterFrame 7
onEnterFrame 8
onEnterFrame 9
onEnterFrame 10
frame 10 actions pre
frame 15 actions post
frame 15 actions (15)
onEnterFrame 15
onEnterFrame 15
onEnterFrame 15
frame 20 actions (20)
'''.splitlines(1)],

["samples/test_frame2.swf", '''onEnterFrame 2
onEnterFrame 3
onEnterFrame 4
onEnterFrame 5
onEnterFrame 6
onEnterFrame 7
onEnterFrame 8
onEnterFrame 9
onEnterFrame 10
frame 10 actions
'''.splitlines(1)],

["samples/TestFunction2.swf", '''Tulsa nightlife -
filth, gin, a slut.

Arg a
Arg b
[object Object]
[object Object]
Arg a,Arg b
Arg b
modified b!
another test
27
[object Object]
[object Object]
another test,27
27
modified b!
'''.splitlines(1)],

[ "samples/test_action_order2.swf",
'''frame 1 actions
frame 2 actions
'''.splitlines(1)],


[ "samples/test_string.swf",
'''a value
ABC
65
66
67
abc
HELLO
-1
2
4
string
hello
pants
Stringobj
42
42
undefined
[object Object]
myObj
'''.splitlines(1)],

[ "samples/test_undefined_v6.swf",
'''a
undefined
test_field.text

a
a 
a != '' and a != 'undefined'
0

'''.splitlines(1)],

[ "samples/test_undefined_v7.swf",
'''a
undefined
test_field.text
undefined
a
a undefined
a != '' and a != 'undefined'
9
nde
'''.splitlines(1)],

[ "samples/test_basic_types.swf",
'''null
undefined
true
boolean
1---
false
boolean
false
false
2---
true
true
true
true
3---
false
true
false
false
4---
true
false
false
true
5---
number
string
null
undefined
6---
2
number
afalse
number
7---
2
number
falsea
string
8---
false
false
false
false
9---
false
false
false
true
A---
true
true
true
true
B---
false
false
false
false
'''.splitlines(1)],

[ "samples/test_forin_array.swf",
'''true'''.splitlines(1)],

[ "samples/test_ASSetPropFlags.swf",
'''891
******** null - 0 - 0 ******
0
******** ['0'] - 0 - 0 ******
0
******** ['0', '1'] - 0 - 0 ******
0
******** null - 1 - 0 ******
0
******** ['0'] - 1 - 0 ******
595
******** ['0', '1'] - 1 - 0 ******
298
******** null - 2 - 0 ******
891
******** ['0'] - 2 - 0 ******
891
******** ['0', '1'] - 2 - 0 ******
891
******** null - 3 - 0 ******
0
******** ['0'] - 3 - 0 ******
595
******** ['0', '1'] - 3 - 0 ******
298
******** null - 4 - 0 ******
891
******** ['0'] - 4 - 0 ******
891
******** ['0', '1'] - 4 - 0 ******
891
******** null - 5 - 0 ******
0
******** ['0'] - 5 - 0 ******
595
******** ['0', '1'] - 5 - 0 ******
298
******** null - 6 - 0 ******
891
******** ['0'] - 6 - 0 ******
891
******** ['0', '1'] - 6 - 0 ******
891
******** null - 7 - 0 ******
0
******** ['0'] - 7 - 0 ******
595
******** ['0', '1'] - 7 - 0 ******
298
******** null - 0 - 1 ******
6293
******** ['0'] - 0 - 1 ******
891
******** ['0', '1'] - 0 - 1 ******
891
******** null - 1 - 1 ******
0
******** ['0'] - 1 - 1 ******
595
******** ['0', '1'] - 1 - 1 ******
298
******** null - 2 - 1 ******
6293
******** ['0'] - 2 - 1 ******
891
******** ['0', '1'] - 2 - 1 ******
891
******** null - 3 - 1 ******
0
******** ['0'] - 3 - 1 ******
595
******** ['0', '1'] - 3 - 1 ******
298
******** null - 4 - 1 ******
6293
******** ['0'] - 4 - 1 ******
891
******** ['0', '1'] - 4 - 1 ******
891
******** null - 5 - 1 ******
0
******** ['0'] - 5 - 1 ******
595
******** ['0', '1'] - 5 - 1 ******
298
******** null - 6 - 1 ******
6293
******** ['0'] - 6 - 1 ******
891
******** ['0', '1'] - 6 - 1 ******
891
******** null - 7 - 1 ******
0
******** ['0'] - 7 - 1 ******
595
******** ['0', '1'] - 7 - 1 ******
298
******** null - 0 - 0 ******
|a2||b2||c2|
******** ['0'] - 0 - 0 ******
|a2||b2||c2|
******** ['0', '1'] - 0 - 0 ******
|a2||b2||c2|
******** null - 1 - 0 ******
|a2||b2||c2|
******** ['0'] - 1 - 0 ******
|a2||b2||c2|
******** ['0', '1'] - 1 - 0 ******
|a2||b2||c2|
******** null - 2 - 0 ******
|a2||b2||c2|
******** ['0'] - 2 - 0 ******
|a2||b2||c2|
******** ['0', '1'] - 2 - 0 ******
|a2||b2||c2|
******** null - 3 - 0 ******
|a2||b2||c2|
******** ['0'] - 3 - 0 ******
|a2||b2||c2|
******** ['0', '1'] - 3 - 0 ******
|a2||b2||c2|
******** null - 4 - 0 ******
|a1||b1||c1|
******** ['0'] - 4 - 0 ******
|a1||b2||c2|
******** ['0', '1'] - 4 - 0 ******
|a1||b1||c2|
******** null - 5 - 0 ******
|a1||b1||c1|
******** ['0'] - 5 - 0 ******
|a1||b2||c2|
******** ['0', '1'] - 5 - 0 ******
|a1||b1||c2|
******** null - 6 - 0 ******
|a1||b1||c1|
******** ['0'] - 6 - 0 ******
|a1||b2||c2|
******** ['0', '1'] - 6 - 0 ******
|a1||b1||c2|
******** null - 7 - 0 ******
|a1||b1||c1|
******** ['0'] - 7 - 0 ******
|a1||b2||c2|
******** ['0', '1'] - 7 - 0 ******
|a1||b1||c2|
******** null - 0 - 1 ******
|a2||b2||c2|
******** ['0'] - 0 - 1 ******
|a2||b2||c2|
******** ['0', '1'] - 0 - 1 ******
|a2||b2||c2|
******** null - 1 - 1 ******
|a2||b2||c2|
******** ['0'] - 1 - 1 ******
|a2||b2||c2|
******** ['0', '1'] - 1 - 1 ******
|a2||b2||c2|
******** null - 2 - 1 ******
|a2||b2||c2|
******** ['0'] - 2 - 1 ******
|a2||b2||c2|
******** ['0', '1'] - 2 - 1 ******
|a2||b2||c2|
******** null - 3 - 1 ******
|a2||b2||c2|
******** ['0'] - 3 - 1 ******
|a2||b2||c2|
******** ['0', '1'] - 3 - 1 ******
|a2||b2||c2|
******** null - 4 - 1 ******
|a1||b1||c1|
******** ['0'] - 4 - 1 ******
|a1||b2||c2|
******** ['0', '1'] - 4 - 1 ******
|a1||b1||c2|
******** null - 5 - 1 ******
|a1||b1||c1|
******** ['0'] - 5 - 1 ******
|a1||b2||c2|
******** ['0', '1'] - 5 - 1 ******
|a1||b1||c2|
******** null - 6 - 1 ******
|a1||b1||c1|
******** ['0'] - 6 - 1 ******
|a1||b2||c2|
******** ['0', '1'] - 6 - 1 ******
|a1||b1||c2|
******** null - 7 - 1 ******
|a1||b1||c1|
******** ['0'] - 7 - 1 ******
|a1||b2||c2|
******** ['0', '1'] - 7 - 1 ******
|a1||b1||c2|
*************************
firstname
surname
icq
************************* 1
firstname
surname
icq
************************* 2
__constructor__
constructor
__proto__
firstname
surname
icq
************************* 3
__constructor__
constructor
__proto__
firstname
surname
icq
************************* 4
__constructor__
constructor
__proto__
firstname
surname
icq
************************* 5
Guy
************************* 6
Guy
************************* 7
************************* 8
************************* 9
Guy
Watson
71063418
************************* 10
Guy
Watson
71063418
************************* 11
Guy
************************* 12
Guy
************************* 13
Guy
Watson
71063418
*************************
'''.splitlines(1)],

# Add more tests here, in form of [ filename, expected_output ]

]


def run_batch_test(testfile, expected_output):
  '''Run gameswf on a test file, and compare its output to the given expected output.
  Return an error code and a report string summarizing the results'''

  report = "";
  success = True;

  [status, output] = commands.getstatusoutput(GAMESWF + BATCH_ARGS + testfile)

  # Clean up the output.
  output = output.splitlines(1)
  output = map(fix_string, output)  # lose trailing newlines, avoid DOS/Unix confusion

  expected_output = map(fix_string, expected_output)    # lose trailing newlines, avoid DOS/Unix confusion

  if (status != 0):
    success = False;
    report += format_header_line(testfile, "[failed]")
    report += "  command returned status code " + str(status) + "\n"
    report += "  command output:\n"
    report += "    " + string.join(output, "    ")
  else:
    # Let's show the difference between expected and actual output
    difference = list(difflib.unified_diff(expected_output, output, "expected", "actual"))
    if (len(difference) == 0):
      report += format_header_line(testfile, "[OK]")
    else:
      success = False;
      report += format_header_line(testfile, "[failed]")
      report += "    " + string.join(difference, "    ")

  return success, report


def format_header_line(test_name, result_tag):
  '''Make a nice aligned summary line for the test'''
  padding = 70 - len(test_name)
  return test_name + ("." * padding) + result_tag + "\n"


def fix_string(s):
  '''strip trailing whitespace, add consistent newline'''
  return (string.rstrip(s) + '\n')


def do_tests():
  success_count = 0
  failed_count = 0
  report = ""

  for test in testlist:
    [success, rep] = run_batch_test(test[0], test[1])
    if success:
      success_count += 1
    else:
      failed_count += 1
    report += rep

  sys.stdout.writelines("Test results: " + str(success_count) + "/" + str(success_count + failed_count) + "\n")
  sys.stdout.writelines(report)


# main
report = do_tests()
sys.exit(0)
