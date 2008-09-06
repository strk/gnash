//
//   Copyright (C) 2008 Free Software Foundation, Inc.
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

#include <algorithm> // for_each
#include <boost/checked_delete.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/range.hpp> // begin, end, range_value.

/// Function template used to delete all pointers in a pointer container.
//
/// @param c The pointer container to be deleted.
///          The contained type must be a complete type and must be
///          a pointer type. The container class itself may be of any
///          standard type or a C array of pointer.
template<typename Container>
void
delete_ptr_container(Container& c)
{
  using namespace boost;

  typedef typename range_value<Container>::type ContainedPtrType;
  typedef typename remove_pointer<ContainedPtrType>::type ContainedValType;

  std::for_each (begin(c), end(c), checked_deleter< ContainedValType >());
}

