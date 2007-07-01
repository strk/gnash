// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
//

/* $Id: hash_wrapper.h,v 1.7 2007/07/01 10:54:09 bjacques Exp $ */

#ifndef HASH_WRAPPER_H
#define HASH_WRAPPER_H

#include <map>

template<typename T, typename U>
class hash_wrapper : public std::map<T, U>
{

public:
			
	void add(const T& key, U& mov)
	{
		(*this)[key] = mov;
	}
	
	bool get(const T& key, U* ret)
	{
		typename std::map<T, U>::iterator it = std::map<T, U>::find(key);
		if ( it != this->end() )
		{
			*ret = it->second;
			return true;
		}
		else
		{
			return false;
		}
	}
};

#endif
