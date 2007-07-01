// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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
//
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Test case for passing parameters trough FlashVars (-P switch)
// and url querystring.
//
// execute the movie by passing:
//	QueryString="?a=a_in_qstring&q=q_in_qstring"
//	FlashVars="?a=a_in_fvars&q=q_in_fvars"
//
// QueryString is what appears embedded in the url,
// FlashVars can be given as an attribute of the <embed> tag
// of (in Gnash) with the -P switch
//
// Example:
//
// gnash -P "FlashVars=?a=a_in_fvars&v=v_in_fvars" "FlashParamTest.swf?a=a_in_qstring&q=q_in_qstring"
//
// See FlashVarsTest.html for a way to test with a plugin
//

note("a="+a);
note("_root.a="+_root.a);
check(_root.hasOwnProperty('a'));
check_equals(_root.a, "a_in_qstring");
_root.a="changed";
check_equals(_root.a, "changed");
check(delete _root.a);

note("q="+q);
note("_root.q="+_root.q);
check(_root.hasOwnProperty('q'));
check_equals(_root.q, "q_in_qstring");

note("v="+v);
note("_root.v="+_root.v);
check(_root.hasOwnProperty('v'));
check_equals(_root.v, "v_in_fvars");

totals();
