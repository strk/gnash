// ExternalInterface.hx:  ActionScript 3 "ExternalInterface" class, for Gnash 
//internal testing.
//
//
//   Copyright (C) 2009, 2010 Free Software Foundation, Inc.
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

// This test case must be processed by CPP before compiling to include the
//  DejaGnu.hx header file for the testing framework support.


//in the AS2 ExternalInterface.as file, the existance of these methods were
//checked
//"_argumentsToXML"
//"_argumentsToAS"
//"_unescapeXML"
//"_toXML"
//"_toJS"
//"_toAS"
//"_objectToXML"
//"_objectToJS"
//"_objectToAS"
//"_objectID"
//"_jsQuoteString"
//"_initJS"
//"_evalJS"
//"_escapeXML"
//"_callOut"
//"_callIn"
//"_arrayToXML"
//"_arrayToJS"
//"_arrayToAS"
//"_addCallback"

//this bug testing code was also there:
// An object
o = { a: 1, b: "string" };

// An object with circular references
oc = {};
oc.a = {};
oc.a.a = oc.b;
oc.b = oc.a;

// A native class
nc = Mouse;

// A native object
no = new XML;

// An Array
a = [ 12, 34, "tr", 1, 2, 3, 4 ];

// Try instantiating.
r = new EI;
// You get an object
check_equals(typeof(r), "object");
check(r instanceOf EI);
// But it doesn't do much.
check_equals(r._toXML(o), undefined);


#if OUTPUT_VERSION > 7

// All methods are static.

// _objectTo*
xcheck_equals(EI._objectToXML(o), '<object><property id="a"><number>1</number></property><property id="b"><string>string</string></property></object>');
xcheck_equals(EI._objectToXML(nc), '<object></object>');
xcheck_equals(EI._objectToXML(no), '<object><property id="namespaceURI"><null/></property><property id="localName"><null/></property><property id="prefix"><null/></property><property id="previousSibling"><null/></property><property id="parentNode"><null/></property><property id="nodeValue"><null/></property><property id="nodeType"><number>1</number></property><property id="nodeName"><null/></property><property id="nextSibling"><null/></property><property id="lastChild"><null/></property><property id="firstChild"><null/></property><property id="childNodes"><array></array></property><property id="attributes"><null/></property><property id="getPrefixForNamespace"><null/></property><property id="getNamespaceForPrefix"><null/></property><property id="toString"><null/></property><property id="hasChildNodes"><null/></property><property id="appendChild"><null/></property><property id="insertBefore"><null/></property><property id="removeNode"><null/></property><property id="cloneNode"><null/></property><property id="xmlDecl"><undefined/></property><property id="status"><number>0</number></property><property id="loaded"><undefined/></property><property id="ignoreWhite"><false/></property><property id="docTypeDecl"><undefined/></property><property id="contentType"><string>application/x-www-form-urlencoded</string></property><property id="addRequestHeader"><null/></property><property id="getBytesTotal"><null/></property><property id="getBytesLoaded"><null/></property><property id="onData"><null/></property><property id="onLoad"><null/></property><property id="sendAndLoad"><null/></property><property id="send"><null/></property><property id="load"><null/></property><property id="parseXML"><null/></property><property id="createTextNode"><null/></property><property id="createElement"><null/></property></object>');
xcheck_equals(EI._objectToXML(undefined), '<object></object>');
xcheck_equals(EI._objectToXML(6), '<object></object>');
xcheck_equals(EI._objectToXML(oc), '<object><property id="b"><object><property id="a"><undefined/></property></object></property><property id="a"><object><property id="a"><undefined/></property></object></property></object>');

xcheck_equals(EI._objectToJS(o), '({a:1,b:"string"})');

xcheck_equals(EI._objectToAS(no).toString(), '[object Object]');

check_equals(EI._objectID(o), null);

// _arrayTo*
xcheck_equals(EI._arrayToXML(a), '<array><property id="0"><number>12</number></property><property id="1"><number>34</number></property><property id="2"><string>tr</string></property><property id="3"><number>1</number></property><property id="4"><number>2</number></property><property id="5"><number>3</number></property><property id="6"><number>4</number></property></array>');
xcheck_equals(EI._arrayToJS(a), '[12,34,"tr",1,2,3,4]');

// _to*

xcheck_equals(EI._toXML("string"), '<string>string</string>');
xcheck_equals(EI._toXML(4), '<number>4</number>');

xcheck_equals(EI._toXML(o), '<object><property id="a"><number>1</number></property><property id="b"><string>string</string></property></object>');

// escape / unescape
r = "& ß+ü < << <>''\"";
xcheck_equals(EI._escapeXML(r), "&amp; ß+ü &lt; &lt;&lt; &lt;&gt;&apos;&apos;&quot;");

// It doesn't escape html entities.
r = "&amp; ß+ü &nbsp; &lt; &lt;&lt; &lt;&gt;&apos;&apos;&quot;";
xcheck_equals(EI._unescapeXML(r), "& ß+ü &nbsp; < << <>''\"");

