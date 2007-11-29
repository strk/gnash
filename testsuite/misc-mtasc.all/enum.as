#include "check.as"

// This movie tests ActionEnum2

class Test 
{

	// constructor
	function Test()
	{
		note("Test constructor called");
	}

	function enumerate(obj, enum)
	{
		var enumlen = 0;
		for (var i in obj) {
			enum[i] = obj[i];
			++enumlen;
		}
		return enumlen;
	}

	function dump(obj)
	{
		for (var i in obj) {
			note("DUMP: obj["+i+"] = "+obj[i]);
		}
	}

	function test_all()
	{
		var res;
		var len;
		var a = new Object;
		a.m1 = 20;

		dump(a);

		res = new Array;
		len = enumerate(a, res);
		check_equals(len, 1);
		check_equals(res['m1'], 20);

		res = new Array;
		len = enumerate(null, res);
		check_equals(len, 0);
		check_equals(res.length, 0);

		res = new Array;
		len = enumerate(undefined, res);
		check_equals(len, 0);
		check_equals(res.length, 0);

		res = new Array;
		dump(""); // becomes a string, most likely
		len = enumerate("", res);
		check_equals(len, 0);
		check_equals(res.length, 0);

		String.prototype.addedMember = 3;

		res = new Array;
		len = enumerate(new String, res);
		check_equals(len, 1);
		check_equals(res.length, 0);

		// enum2 doesn't convert the empty string to a String object!
		res = new Array;
		len = enumerate("", res);
		check_equals(len, 0);
		check_equals(res.length, 0);
	}

	static function main(mc)
	{
		var myTest = new Test;
		myTest.test_all();

		check_totals(12);
                Dejagnu.done();
	}

}
