// ConstantPool.h: a parsed ConstantPool 
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include <vector>
#include <iostream>

#include "ConstantPool.h"
#include "VM.h"


namespace gnash {

std::ostream& 
operator<<(std::ostream& os, const ConstantPool& p) {
    for (size_t i=0; i<p.size(); ++i) {
        if (i) os <<  ", ";
        os << i << ':' << p[i];
    }
    return os;
}


PoolGuard::PoolGuard(VM& vm, const ConstantPool* pool)
    :
    _vm(vm),
    _from(_vm.getConstantPool())
{
    _vm.setConstantPool(pool);
}


PoolGuard::~PoolGuard() 
{
    _vm.setConstantPool(_from);
}


} // namespace gnash
