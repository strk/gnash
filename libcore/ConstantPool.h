// ConstantPool.h: a parsed ConstantPool 
// 
//   Copyright (C) 2011, 2012 Free Software Foundation, Inc
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

#ifndef GNASH_CONSTANTPOOL_H
#define GNASH_CONSTANTPOOL_H

#include <vector>
#include <iosfwd>

namespace gnash {

class VM;

/// An indexed list of strings (must match the definition in action_buffer.h)
typedef std::vector<const char*> ConstantPool;

std::ostream& operator<<(std::ostream& os, const ConstantPool& p);

class PoolGuard
{
public:

	// @param pool : ConstantPool to set temporarely
	PoolGuard(VM& vm, const ConstantPool* pool);
	~PoolGuard();

private:

    VM& _vm;
    const ConstantPool* _from;

};

} // namespace gnash


#endif 
