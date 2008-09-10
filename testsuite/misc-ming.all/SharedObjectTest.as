// 
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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


so1 = SharedObject.getLocal("sol1");

check_equals(so1.data.num, 2);
check_equals(so1.data.str, 'a string');
check_equals(typeof(so1.data.tbool), 'boolean');
check_equals(so1.data.tbool, true);
check_equals(typeof(so1.data.fbool), 'boolean');
check_equals(so1.data.fbool, false);
check_equals(typeof(so1.data.ary), 'object');
check_equals(so1.data.ary.toString(), '1,2,3');
check_equals(typeof(so1.data.obj), 'object');
check_equals(typeof(so1.data.obj.a), 'number');
check_equals(so1.data.obj.a, 10);
check_equals(typeof(so1.data.obj.b), 'string');
check_equals(so1.data.obj.b, '20');
check_equals(typeof(so1.data.obj.c), 'boolean');
check_equals(so1.data.obj.c, true);
check_equals(so1.data.ref, so1.data.obj);

// force writing the sol or the adobe player won't save it
// again. This will also serve as a kind of reference for
// how the sol file was produced in the first place
so1.data.num = 2; 
so1.data.str = 'a string'; 
so1.data.tbool = true;
so1.data.fbool = false;
so1.data.ary = [1,2,3];
so1.data.obj = {a:10,b:'20',c:true};
so1.data.ref = so1.data.obj;

so1.flush();

quit = function()
{
	loadMovie('fscommand:quit', '');
};
note(" Will quit in 1 seconds");
setInterval(quit, 1000);
stop();

totals();
