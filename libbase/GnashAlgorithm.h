// GnashAlgorithm.h:  Moderately useful functors for generic algorithms
//
//   Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifndef GNASH_ALGORITHM_H
#define GNASH_ALGORITHM_H

#include <algorithm>
#include <boost/checked_delete.hpp>

namespace gnash {

/// Retrieve the second element of a container with std::pairs.
template<typename T>
struct SecondElement
{
    typedef typename T::second_type result_type;

    const result_type& operator()(const T& pair) const {
        return pair.second;
    }
};

/// Return a pointer to a type
template<typename T>
struct CreatePointer
{
    const T* operator()(const T& t) { 
        return &t;
    }
};

/// Recurse to the base type of a pointer.
template<typename T>
struct RemovePointer
{
    typedef T value_type;
};

template<typename T>
struct RemovePointer<T*>
{
    typedef typename RemovePointer<T>::value_type value_type;
};

/// Delete a pointer safely
//
/// Any depth of pointers-to-pointers (up to maximum template recursion) can
/// be passed to this struct. The type of the pointee is deduced and passed
/// to boost::checked_deleter, which ensures that the type is fully known
/// at the point of deletion. It does not, of course, check that the pointer
/// was allocated with new, so this isn't completely idiot-proof.
template<typename T>
struct CheckedDeleter
{
};

template<typename T>
struct CheckedDeleter<T**>
{
    void operator()(T** p) const {
        CheckedDeleter<T*> del;
        dp(*p);
    }
};

template<typename T>
struct CheckedDeleter<T*>
{
    void operator()(T* p) const {
        typename boost::template checked_deleter<
            typename RemovePointer<T>::value_type> del;
        del(p);
    }
};

template<typename T>
void
deleteAllChecked(const T& c)
{
    std::for_each(c.begin(), c.end(),
            CheckedDeleter<typename T::value_type>());
}

} // namespace gnash

#endif

