// GnashAlgorithm.h: useful templates and functors for generic algorithms
//
//   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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
#include <boost/intrusive_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

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

/// Retrieve the first element of a container with std::pairs.
template<typename T>
struct FirstElement
{
    typedef typename T::first_type result_type;

    const result_type& operator()(const T& pair) const {
        return pair.first;
    }
};

/// Return a pointer to a type
template<typename T>
struct CreatePointer
{
    typedef T* result_type;
    result_type operator()(T& t) { 
        return &t;
    }
};

/// Erase elements from an associative container based on a predicate
//
/// This removes elements from a container such as a map if they fulfil a
/// particular condition. Because keys of associative container are const,
/// we can't do this using iterators, because we can't write to them.
template<typename Container, typename Predicate>
void EraseIf(Container& c, Predicate p)
{
    typedef typename Container::iterator iterator;

    for (iterator i = c.begin(), e = c.end(); i != e; ) {
        iterator stored = i++;
        if (p(*stored)) c.erase(stored);
    }
}


/// Get the size of an array without passing a pointer by mistake
template<typename T, size_t N>
size_t
arraySize(T(&)[N])
{
    return N;
}


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
    /// Typedef for use in boost::bind.
    typedef typename CheckedDeleter<T*>::result_type result_type;

    void operator()(T** p) const {
        CheckedDeleter<T*>()(*p);
    }
};

template<typename T>
struct CheckedDeleter<T*>
{
    /// Typedef for use in boost::bind.
    typedef void result_type;

    void operator()(T* p) const {
        boost::checked_delete<T>(p);
    }
};


/// Call a functor on the second element of each element in a range.
//
/// @tparam T           An iterator type satisfying the requirements of a
///                     forward iterator
/// @tparam U           The type of the functor op.
/// @param begin        The start of the range to call op on.
/// @param end          The end of the range to call op on.
/// @param op           The function to call on each second element.
template<typename T, typename U>
void
foreachSecond(T begin, T end, U op)
{
    typedef SecondElement<typename std::iterator_traits<T>::value_type> S;
    std::for_each(begin, end, boost::bind(op, boost::bind(S(), _1)));
}

/// Safely call delete on each element in a range.
//
/// This checks that the type is fully known, but cannot check whether the
/// pointer was allocated with new. Pointers allocated with new[] or any other
/// allocation function should never be passed to this function.
//
/// @param begin        The start of the range to call delete on.
/// @param end          The end of the range to call delete on.
template<typename T>
void
deleteChecked(T begin, T end)
{
    typedef typename std::iterator_traits<T>::value_type value_type;
    std::for_each(begin, end, CheckedDeleter<value_type>());
}

} // namespace gnash

#endif

