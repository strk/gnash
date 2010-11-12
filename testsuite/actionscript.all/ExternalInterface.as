// 
//   Copyright (C) 2008, 2009, 2010 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
//
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
// Test case for ExternalInterface ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

#include "check.as"

flash.system.Security.allowDomain("localhost");

ASSetPropFlags (_global, "flash", 0, 5248);
EI = flash.external.ExternalInterface;

#if OUTPUT_VERSION < 6 // {

check_equals(typeof(EI), 'undefined');

#else // OUTPUT_VERSION >= 6 }{

check_equals(typeof(EI), 'function');
check(EI.hasOwnProperty('addCallback'));
check(EI.hasOwnProperty("_argumentsToAS"));
check(EI.hasOwnProperty("_argumentsToXML"));
check(EI.hasOwnProperty("_arrayToAS"));
check(EI.hasOwnProperty("_arrayToJS"));
check(EI.hasOwnProperty("_arrayToXML"));
check(EI.hasOwnProperty('available'));
check(EI.hasOwnProperty('call'));
check(EI.hasOwnProperty("_callIn"));
check(EI.hasOwnProperty("_callOut"));
check(EI.hasOwnProperty("_escapeXML"));
check(EI.hasOwnProperty("_evalJS"));
check(EI.hasOwnProperty("_initJS"));
check(EI.hasOwnProperty("_jsQuoteString"));
check(EI.hasOwnProperty("_objectID"));
check(EI.hasOwnProperty("_objectToAS"));
check(EI.hasOwnProperty("_objectToJS"));
check(EI.hasOwnProperty("_objectToXML"));
check(EI.hasOwnProperty("_toAS"));
check(EI.hasOwnProperty("_toJS"));
check(EI.hasOwnProperty("_toXML"));
check(EI.hasOwnProperty("_unescapeXML"));
check(EI.hasOwnProperty("prototype"));
check(!EI.hasOwnProperty("marshallExceptions"));
check(!EI.hasOwnProperty("objectID"));
// Empty prototype...
var s = ''; for (var i in EI.prototype) s += i; check_equals(s, '');

#if OUTPUT_VERSION < 8 // {

xcheck_equals(typeof(EI.addCallback), 'undefined');
xcheck_equals(typeof(EI._argumentsToAS), 'undefined');
xcheck_equals(typeof(EI._argumentsToXML), 'undefined');
xcheck_equals(typeof(EI._arrayToAS), 'undefined');
xcheck_equals(typeof(EI._arrayToJS), 'undefined');
xcheck_equals(typeof(EI._arrayToXML), 'undefined');
check_equals(typeof(EI.available), 'undefined');
check_equals(typeof(EI.call), 'function');
xcheck_equals(typeof(EI._callIn), 'undefined');
check_equals(typeof(EI._callOut), 'undefined');
check_equals(typeof(EI._escapeXML), 'undefined');
check_equals(typeof(EI._evalJS), 'undefined');
check_equals(typeof(EI._initJS), 'undefined');
check_equals(typeof(EI._jsQuoteString), 'undefined');
check_equals(typeof(EI._objectID), 'undefined');
xcheck_equals(typeof(EI._objectToAS), 'undefined');
xcheck_equals(typeof(EI._objectToJS), 'undefined');
xcheck_equals(typeof(EI._objectToXML), 'undefined');
xcheck_equals(typeof(EI._toAS), 'undefined');
xcheck_equals(typeof(EI._toJS), 'undefined');
xcheck_equals(typeof(EI._toXML), 'undefined');
check_equals(typeof(EI._unescapeXML), 'undefined');

#else //  OUTPUT_VERSION >= 8  }{

check_equals(typeof(EI.addCallback), 'function');
check_equals(typeof(EI._argumentsToAS), 'function');
check_equals(typeof(EI._argumentsToXML), 'function');
check_equals(typeof(EI._arrayToAS), 'function');
check_equals(typeof(EI._arrayToJS), 'function');
check_equals(typeof(EI._arrayToXML), 'function');
check_equals(typeof(EI.available), 'boolean');
check_equals(typeof(EI.call), 'function');
check_equals(typeof(EI._callIn), 'function');
check_equals(typeof(EI._callOut), 'function');
check_equals(typeof(EI._escapeXML), 'function');
check_equals(typeof(EI._evalJS), 'function');
check_equals(typeof(EI._initJS), 'function');
check_equals(typeof(EI._jsQuoteString), 'function');
check_equals(typeof(EI._objectID), 'function');
check_equals(typeof(EI._objectToAS), 'function');
check_equals(typeof(EI._objectToJS), 'function');
check_equals(typeof(EI._objectToXML), 'function');
check_equals(typeof(EI._toAS), 'function');
check_equals(typeof(EI._toJS), 'function');
check_equals(typeof(EI._toXML), 'function');
check_equals(typeof(EI._unescapeXML), 'function');

// Create a test function for the callback
function TestASMethod (msg) {
    note("TestASMethod called! " + msg);
    return "I am here!";
}

// ExternalInterface::available is always false when run standalone,
// and true if running under a browser and has allowable access to the
// resource. So we can't have a test case for this property that works
// both standalone and online Instead we do depend on it being correct
// as the ExternalInterface::call() tests require this test case to
// be run under a browser.

// ::call() calls JavaScript functions in the browser, not in flash,
// so we can't test it when running standalone. So this will always
// return null unless run from a browser

// This adds a callback that the brower can call from Javascript. This is
// a bit of a bogus test case, as addCallback() doesn't return anything, but
// we need to add a method anyway to test being called by Javascript.
ret = EI.addCallback("TestASMethod", null, TestASMethod);
check_equals(typeof(ret), 'boolean');
check_equals(ret, EI.available); // returns true if available...
ret = EI.call("TestJSMethod", "test");
if ( EI.available ) {
    check_equals(typeof(ret), 'undefined');
} else {
    check_equals(typeof(ret), 'null');
}

// A native class
nc = Mouse;

// Try instantiating.
r = new EI;
// You get an object
check_equals(typeof(r), 'object');
check(r instanceOf EI);

// All methods are class statics, not inherited
check_equals(typeof(r.addCallback), 'undefined');
check_equals(typeof(r._argumentsToAS), 'undefined');
check_equals(typeof(r._argumentsToXML), 'undefined');
check_equals(typeof(r._arrayToAS), 'undefined');
check_equals(typeof(r._arrayToJS), 'undefined');
check_equals(typeof(r._arrayToXML), 'undefined');
check_equals(typeof(r.available), 'undefined');
check_equals(typeof(r.call), 'undefined');
check_equals(typeof(r._callIn), 'undefined');
check_equals(typeof(r._callOut), 'undefined');
check_equals(typeof(r._escapeXML), 'undefined');
check_equals(typeof(r._evalJS), 'undefined');
check_equals(typeof(r._initJS), 'undefined');
check_equals(typeof(r._jsQuoteString), 'undefined');
check_equals(typeof(r._objectID), 'undefined');
check_equals(typeof(r._objectToAS), 'undefined');
check_equals(typeof(r._objectToJS), 'undefined');
check_equals(typeof(r._objectToXML), 'undefined');
check_equals(typeof(r._toAS), 'undefined');
check_equals(typeof(r._toJS), 'undefined');
check_equals(typeof(r._toXML), 'undefined');
check_equals(typeof(r._unescapeXML), 'undefined');

xml = EI._objectToXML(nc);
check_equals (xml, '<object></object>');

// An object
o = { a: 1, b: "string" };

xml = EI._objectToXML(o);
check_equals (xml, '<object><property id="a"><number>1</number></property><property id="b"><string>string</string></property></object>');

xml = EI._objectToXML(o, 1, 2, 3);
check_equals (xml, '<object><property id="a"><number>1</number></property><property id="b"><string>string</string></property></object>');

// An object with an object reference

o = { o: o };
xml = EI._objectToXML(o);
check_equals (xml, '<object><property id="o"><object><property id="a"><number>1</number></property><property id="b"><string>string</string></property></object></property></object>');

// An object with circular references
// (the proprietary player hangs here, we jus don't want to segfault)
// oc = {};
// oc.cr = oc;
// xml = EI._objectToXML(oc);
// check_equals (xml, '');

// An undefined

xml = EI._objectToXML(undefined);
check_equals (xml, '<object></object>');

xml = EI._objectToXML(null);
check_equals (xml, '<object></object>');

xml = EI._objectToXML(6);
check_equals (xml, '<object></object>');

xml = EI._objectToXML();
check_equals (xml, '<object></object>');

// An Array
anArray = [ 12, "tr", 1 ];
anArray.length = 4;

xml = EI._arrayToXML(anArray);
check_equals (xml,
'<array><property id="0"><number>12</number></property><property id="1"><string>tr</string></property><property id="2"><number>1</number></property><property id="3"><undefined/></property></array>'
);

xml = EI._argumentsToXML(anArray);
check_equals (xml, 
'<arguments><string>tr</string><number>1</number><undefined/></arguments>'
);
xml = EI._argumentsToXML();
check_equals (xml, '<arguments></arguments>');
xml = EI._argumentsToXML(['single']);
check_equals (xml, '<arguments></arguments>');
xml = EI._argumentsToXML('one');
check_equals (xml, '<arguments><undefined/><undefined/></arguments>');
xml = EI._argumentsToXML(1,2,3);
check_equals (xml, '<arguments></arguments>');

// It uses the length property...
str = "hi";
ret = flash.external.ExternalInterface._arrayToXML(str);
check_equals(ret, '<array><property id="0"><undefined/></property><property id="1"><undefined/></property></array>');

// xml = EI._toXML(o);
// if (xml == '<object><property id="a"><number>1</number></property><property id="b"><string>string</string></property></object>') {
//     pass("ExternalInterface::_toXML(object)");
// } else {
//     fail("ExternalInterface::_toXML(object)");
// }

// xml = EI._toXML("Hello World!");
// if (xml == "<string>Hello World!</string>") {
//     pass("ExternalInterface::_toXML(string)");
// } else {
//     fail("ExternalInterface::_toXML(string)");
// }

xml = EI._toXML(123.456);
check_equals (xml , "<number>123.456</number>");

// A native object
no = new XML;

xml = EI._objectToXML(no);
xcheck_equals (xml, '<object><property id="namespaceURI"><null/></property><property id="localName"><null/></property><property id="prefix"><null/></property><property id="previousSibling"><null/></property><property id="parentNode"><null/></property><property id="nodeValue"><null/></property><property id="nodeType"><number>1</number></property><property id="nodeName"><null/></property><property id="nextSibling"><null/></property><property id="lastChild"><null/></property><property id="firstChild"><null/></property><property id="childNodes"><array></array></property><property id="attributes"><null/></property><property id="getPrefixForNamespace"><null/></property><property id="getNamespaceForPrefix"><null/></property><property id="toString"><null/></property><property id="hasChildNodes"><null/></property><property id="appendChild"><null/></property><property id="insertBefore"><null/></property><property id="removeNode"><null/></property><property id="cloneNode"><null/></property><property id="xmlDecl"><undefined/></property><property id="status"><number>0</number></property><property id="loaded"><undefined/></property><property id="ignoreWhite"><false/></property><property id="docTypeDecl"><undefined/></property><property id="contentType"><string>application/x-www-form-urlencoded</string></property><property id="addRequestHeader"><null/></property><property id="getBytesTotal"><null/></property><property id="getBytesLoaded"><null/></property><property id="onData"><null/></property><property id="onLoad"><null/></property><property id="sendAndLoad"><null/></property><property id="send"><null/></property><property id="load"><null/></property><property id="parseXML"><null/></property><property id="createTextNode"><null/></property><property id="createElement"><null/></property></object>');

// xcheck_equals(EI._objectToJS(o), '({a:1,b:"string"})');

// xcheck_equals(EI._objectToAS(no).toString(), '[object Object]');

// check_equals(EI._objectID(o), null);

// escape / unescape
rin = "& ß+ü < << <>''\"";
rout = "&amp; ß+ü &lt; &lt;&lt; &lt;&gt;&apos;&apos;&quot;";
ret = EI._escapeXML(rin);
check_equals(ret, rout);

// It doesn't escape html entities.
rin = "&amp; ß+ü &nbsp; &lt; &lt;&lt; &lt;&gt;&apos;&apos;&quot;";
rout = "& ß+ü .. < << <>''\"";
ret  = EI._unescapeXML(rin);
// This test will until fail until we can figure out the best way to
// match the converted strings. Testing in GDB show the result is correct,
// so this is mainly a test case problem.
xcheck_equals (ret, "& ß+ü &nbsp; < << <>''\"");

// This isn't how _toAS works:
val = EI._toAS("<number>34.56</number>");
check_equals (typeof(val), 'undefined');

val = EI._toAS("<string>Hello World!</string>");
check_equals (typeof(val), 'undefined');

val = EI._toAS("<null/>");
check_equals (typeof(val), 'undefined');

val = EI._toAS("<true/>");
check_equals (typeof(val), 'undefined');

val = EI._toAS("<false/>");
check_equals (typeof(val), 'undefined');

// This is how it works:
// Note that it's really designed for XMLNodes, but it's quite happy with
// the faked nodes we're giving it here.

o = {};
o.nodeName = "false";
check_equals(EI._toAS(o), false);

o = {};
o.nodeName = "true";
check_equals(EI._toAS(o), true);

o = {};
o.nodeName = "number";
check_equals(EI._toAS(o).toString(), "NaN");
o.firstChild = new String("45");
check_equals(EI._toAS(o), 45);

e = {};
e.toString = function() { return "23"; };
o.firstChild = e;
check_equals(EI._toAS(o), 23);

// This way doesn't work.
o = {};
o.nodeName = "boolean";
check_equals(EI._toAS(o), undefined);
o.firstChild = new String("false");
check_equals(EI._toAS(o), undefined);

o = {};
o.nodeName = "null";
check_equals(EI._toAS(o), null);

o = {};
o.nodeName = "class";
o.firstChild = new String("foo");
check_equals(EI._toAS(o), undefined);

o.firstChild = new String("Date");
check_equals(typeof(EI._toAS(o)), "function");

val = EI._objectToAS('<object><property id="b"><string>string</string></property><property id="a"><number>1</number></property></object>');
xcheck_equals (typeOf(val), 'object');

// Check what happens with addCallback

// It doesn't add the callback as a member of ExternalInterface.
o = {};
EI.addCallback("func1", o);
check_equals(EI.func1, undefined);

#endif  // version > 7 }

#endif // OUTPUT_VERSION >= 6 }


#if OUTPUT_VERSION < 6 // {
	check_totals(1);
#elif OUTPUT_VERSION < 8 // }{
	check_totals(49);
#else // SWF8+ }{
	check_totals(112);
# endif // }

