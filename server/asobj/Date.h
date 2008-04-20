// 
//	Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef __DATE_H__
#define __DATE_H__

#include "as_object.h" // for inheritance
#include "fn_call.h" // for inheritance

namespace gnash {

class Date : public as_object
{
public:
    void setTimeValue(const double& value) { _value = value; }
    double getTimeValue() const { return _value; }

    Date();
    Date(double value);
    
    as_value toString() const;
    
    bool isDateObject() { return true; }
    
private:
    double _value;
};

void registerDateNative(as_object& global);

void date_class_init(as_object& global);

} // end of gnash namespace

// __DATE_H__
#endif

