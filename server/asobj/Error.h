// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// 
//
//

#ifndef __ERROR_H__
#define __ERROR_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "impl.h"

namespace gnash {
  
class Error {
public:
    Error();
    ~Error();
   void toString();
private:
    bool _message;
    bool _name;
};

class error_as_object : public as_object
{
public:
    Error obj;
};

void error_new(const fn_call& fn);
void error_tostring(const fn_call& fn);

} // end of gnash namespace

// __ERROR_H__
#endif

