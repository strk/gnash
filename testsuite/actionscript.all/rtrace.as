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

#ifndef _RTRACE_AS_
#define _RTRACE_AS_

#ifndef USE_RTRACE
// Set the URL to the location of the testreport.php script
// (you should really set USE_RTRACE before including this file though)
#define USE_RTRACE "http://localhost/testreport.php";
#endif

var traced = "";

rtrace = function (msg) 
{
	last_trace_time = getTimer();
	traced += msg+"\n";
};


report = function()
{
	var url = USE_RTRACE;
	getUrl(url, 'report', 'POST');
};

var last_trace_time = getTimer();

//
// getTimer() return milliseconds, so
// we send the report after 1 second of inactivity
// and check for it every 1/2 seconds
//
var interval = setInterval(
	function() {
		if (getTimer() - last_trace_time > 1000 ) { 
			clearInterval(interval);
			report();
		}
	},
500);

#endif // _RTRACE_AS_
