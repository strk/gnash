// GnashSmartPtr.h: scoped pointer supporting deleters.
//
//   Copyright (C) 2013
//   Free Software Foundation, Inc.
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

#ifndef GNASH_SCOPED_PTR_H
#define GNASH_SCOPED_PTR_H

#include <boost/utility.hpp> // noncopyable
#include <boost/function.hpp>

namespace gnash {

/// ScopedPtr is very similar to scoped_ptr, but includes the Deleter
/// functionality from shared_ptr. ScopedPtr can be used to implement the RAII
/// pattern for C APIs, which frequently have their own deallocation strategy,
/// when shared_ptr semantics are not desirable.
//
/// ScopedPtr is similar to C++11's unique_ptr, but the deleter is not part of
/// the type.
template <typename T>
class ScopedPtr : public boost::noncopyable
{
private:
    typedef boost::function<void(T* x)> DeleterT;
public:

    /// Construct a ScopedPtr and provide a deleter.
    ///
    /// @ptr the pointer to exclusively manage.
    /// @deleter the deleter to call when this object goes out of scope.
    /// The expression d(ptr) must be well-formed.
    explicit ScopedPtr(T* ptr, DeleterT d)
    : _ptr(ptr),
      _deleter(d)
    {}

    /// Dereferences the managed pointer and returns a reference.
    T& operator*() const 
    {
        return *_ptr;
    }

    /// Dereference the contained pointer.
    T* operator->() const
    {
        return _ptr;
    }

    /// Obtain the contained pointer.
    T* get() const
    {
        return _ptr;
    }

    ~ScopedPtr()
    {
        if (_ptr) {
            _deleter(_ptr);
        }
    }
private:

    T* _ptr;
    DeleterT _deleter;
};

} // namespace gnash

#endif
