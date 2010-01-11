// exception.as - MTASC testcase for try/catch 
//
//   Copyright (C) 2005, 2006, 2007, 2009, 2010 Free Software
//   Foundation, Inc
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
//
// Original author: Asger Ottar Alstrup <asger@area9.dk>
//

#include "check.as"

// This movie tests exceptions

class Test 
{

	// constructor
	function Test()
	{
		note("Test constructor called");
	}

	function addOneOnFinal(o)
	{
		try {
			return 'try';
		}
		finally {
			o.num += 1;
			return 'finally';
		}
	}

	function throwAndCatchAddingOne(o)
	{
		try {
			throw 'catch';
			return 'try';
		}
		catch (e) {
			return e;
		}
		finally {
			o.num += 1;
		}
	}

	function throwFromCatchAddingOne(o)
	{
		try {
			throw 'catch';
			return 'try';
		}
		catch (e) {
			o.num += 1;
			note("Catch inside, throwing again (not expected to survive after finally)!");
			throw e; // double throw not supported ?
			return 'catch';
		}
		finally {
			return 'finally';
		}
	}

	function throwNested(o)
	{
		try {
			throw 'throw';
		} catch (e) {
			note("Catch inside, throwing again!");
			throw e;
		}
	}

	function returnInTryAndCatch(o)
	{
		try {
			return 'try';
			note ("After return in try");
			o.num += 1;
		}
		catch (e) {
		    note ("Catch after return in try");
		    o.num += 1;
			return 'catch';
		}
	}

	function returnInTryCatchAndFinally(o)
	{
		try {
			return 'try';
			note ("After return in try");
			o.num += 1;
		}
		catch (e) {
		    note ("Catch after return in try");
		    o.num += 1;
			return 'catch';
		}
		finally {
		    note ("Finally after returns in try and catch");
		    o.num += 1;
		    return 'finally';
		}
	}

	function tryCatchAndFinally(o)
	{
		try {
		    o.sequence += "try";
			o.num += 1;
		}
		catch (e) {
		    o.sequence += "catch";
		    o.num += 1;
		}
		finally {
		    o.sequence += "finally";
		    o.num += 1;
		}
	}

	function returnInCatchAndFinally(o)
	{
		try {
			throw 'try'
			note ("After throw in try");
			o.num += 1;
		}
		catch (e) {
		    note ("Catch after throw in try");
			return 'catch';
		    o.num += 1;
		}
		finally {
		    note ("Finally after returns in catch");
		    o.num += 1;
		    return 'finally';
		}
	}


	function test_all()
	{
		var res = 'string';
		try {
			throw(1);
			res = 0;
		} catch (e) {
			res = e;
		}
		check_equals(typeof(res), 'number');
		check_equals(res, 1);

		res = 'string';
		try {
			throw('thrown');
			res = 0;
		} catch(e) {
			res = e;
		}
		finally {
			res += '_finally';
		}
		check_equals(typeof(res), 'string');
		check_equals(res, 'thrown_finally');

		res = 'string';
		try {
			res = 0;
		} catch(e) {
			res = e;
		}
		finally {
			res = 3;
		}
		check_equals(typeof(res), 'number');
		check_equals(res, 3);

		var o = new Object();
		o.num = 1;
		var ret = addOneOnFinal(o);
		check_equals(ret, 'finally');
		check_equals(o.num, 2);

		ret = throwAndCatchAddingOne(o);
		check_equals(ret, 'catch');
		check_equals(o.num, 3);

		try {
			ret = throwAndCatchAddingOne(o);
		} catch (e) {
			ret = 'catch_outside';
		}
		check_equals(ret, 'catch');
		check_equals(o.num, 4);

		try {
			ret = throwFromCatchAddingOne(o);
		} catch (e) {
			note("Catch outside");
			o.num += 1;
			ret += e+'_outside';
		}
		check_equals(ret, 'finally');
		check_equals(o.num, 5);


        try {
            ret = returnInTryAndCatch(o);
        }
        check_equals(ret, "try");
        check_equals(o.num, 5);

        ret = returnInTryCatchAndFinally(o);
        check_equals(ret, "finally");
        check_equals(o.num, 6);
        
        o.sequence = "";
        tryCatchAndFinally(o);
        check_equals(o.sequence, "tryfinally");
        check_equals(o.num, 8);

        ret = returnInCatchAndFinally(o);
        check_equals(o.num, 9);
        check_equals(ret, "finally");

		try {
			throwNested();
		} catch (e) {
			note("Catch outside");
			o.num = e;
		}
		check_equals(o.num, 'throw');
	}

	static function main(mc)
	{
	
		var myTest = new Test;
		myTest.test_all();

        check_totals(23);
        Dejagnu.done();
	}

}
