// Mike Carlson's test program for actionscript arrays
// (initialization, 
// Jan. 17th, 2006

//class arrayTest {

//	static function main() {
		var a;
		a=[551,"asdf",1.2];
		trace("TRACE:var a = [551,asdf,1.2]");
		trace("TRACE:checking a.length() (should be 3):"+a.length());
		trace("TRACE:checking a[2] (should be 1.2):"+a[2]);
		trace("TRACE:executing a.pop() (result should be 1.2):"+a.pop());
		trace("TRACE:checking a[2] (should be undefined):"+a[2]);
		trace("TRACE:checking a[1] (should be asdf):"+a[1]);
		a[1] = a[0];
		trace("TRACE:setting a[1] to a[0]");
		trace("TRACE:checking a[1] (should be 551):"+a[1]);
		trace("TRACE:setting a[0] to 200");
		a[0] = 200;
		trace("TRACE:checking a[0] (should be 200):"+a[0]);
		trace("TRACE:checking a.length() (should be 2):"+a.length());
		trace("TRACE:executing a.push(7,8,9)");
		a.push(7,8,9);
		trace("TRACE:checking a.length() (should be 5):"+a.length());
		trace("TRACE:checking a[0],a[4],a[5] (should be 200,9,undefined):"+a[0]+","+a[4]+","+a[5]);

		var b = new Array();

		trace("TRACE:Initializing variable b as new Array()");
		trace("TRACE:checking b.length() (should be 0):"+b.length());
		trace("TRACE:checking b[0] (should be undefined):"+b[0]);
		trace("TRACE:checking b[500] (should be undefined):"+b[500]);
		b.push(8.112);
		trace("TRACE:executing b.push(8.112)");
		trace("TRACE:checking b[0] (should be 8.112):"+b[0]);
		trace("TRACE:checking b.length() (should be 1):"+b.length());

		var c = new Array(8);

		trace("TRACE:Initializing variable c as new Array(8)");
		trace("TRACE:checking c.length() (should be 8):"+c.length());
		trace("TRACE:checking c[5] (should be undefined):"+c[5]);
		trace("TRACE:checking c[8] (should be undefined):"+c[8]);
		c[1000] = 500;
		trace("TRACE:setting c[1000] = 500");
		trace("TRACE:checking c[999] (should be undefined):"+c[999]);
		trace("TRACE:checking c[1000] (should be 500):"+c[1000]);
		trace("TRACE:checking c[1001] (should be undefined):"+c[1001]);
		trace("TRACE:checking c[1005] (should be undefined):"+c[1005]);
		trace("TRACE:checking c.length() (should be 1001, to accomodate the value at c[1000]):"+c.length());

		var d = new Array(5.2,7,"qwerty",3);

		trace("TRACE:Initializating variable d as new Array(5.2,7,qwerty,3)");
		trace("TRACE:checking d[0] (should be 5.2):"+d[0]);
		trace("TRACE:checking d[2] (should be qwerty):"+d[2]);
		trace("TRACE:checking d[1] (should be 7):"+d[1]);
		trace("TRACE:checking d[3] (should be 3):"+d[3]);
		trace("TRACE:checking d[4] (should be undefined):"+d[4]);
		trace("TRACE:checking d.length() (should be 4):"+d.length());
		d.pop();
		trace("TRACE:executing d.pop()");
		d.pop();
		trace("TRACE:executing d.pop()");
		d.pop();
		trace("TRACE:executing d.pop()");
		trace("TRACE:checking d[1] (should be undefined):"+d[1]);
		d.push("cha");
		trace("TRACE:executing d.push(cha)");
		trace("TRACE:checking d[1] (should be cha):"+d[1]);
		trace("TRACE:checking d.length() (should be 2):"+d.length());
		trace("TRACE:checking d[0] (should be 5.2):"+d[0]);
//	}
//}


// $Log: array.as,v $
// Revision 1.1  2006/02/01 11:43:16  strk
// Added generic rule to build .swf from .as using makeswf (Ming).
// Changed array.as source to avoid ActionScript2 constructs (class).
// Added initial version of a movieclip AS class tester.
//
