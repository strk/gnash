#include "check.as"

import SimpleInterface;
import ImplementationA;
import ImplementationB;
import BExtendingImplementation;

class SimpleInterfaceTest {
	
	var objectA:SimpleInterface;
	var objectB:SimpleInterface;
	var objectC:SimpleInterface;
	
	function SimpleInterfaceTest(testMethod:String) {
	super(testMethod);
	}
	
	function test_all():Void {
		objectA = new ImplementationA();
		objectB = new ImplementationB();
		objectC = new BExtendingImplementation();
	
		check_equals(100, objectA.doStuff(1, "foo"));
		check_equals(100, objectA.doStuff(1, "foo"));
	
		check_equals("param1 was foo", objectA.doMoreStuff("foo", 1));
	
		check_equals(200, objectB.doStuff(1, "foo"));
	
		check_equals("param2 was 1", objectB.doMoreStuff("foo", 1));
	
		check_equals(objectB.doStuff(1, "foo"), objectC.doStuff(1, "foo"));
	
		check_equals("overriding implementation", objectC.doMoreStuff("foo", 1));
	}

	static function main(mc)
	{
		var myTest = new SimpleInterfaceTest;
		myTest.test_all();

		Dejagnu.done();
	}
}

