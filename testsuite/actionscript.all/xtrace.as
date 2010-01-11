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

#ifndef _XTRACE_AS_
#define _XTRACE_AS_

createTextField("out",300000,0,0,600,800);

// FIXME: _global object isn't recognized
//_global.xtrace = function (msg)

xtrace = function (msg) 
{
	_level0.out.text += msg+"\n";
};


//xtrace("Xtrace working");

#endif // _XTRACE_AS_
