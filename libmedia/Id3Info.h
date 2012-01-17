// Id3Info.h: Container for Id3Info data
// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
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

#ifndef GNASH_MEDIA_ID3INFO_H
#define GNASH_MEDIA_ID3INFO_H

#include <boost/optional.hpp>
#include <string>

namespace gnash {
namespace media {

/// Contains ID3 data.
//
/// Not all fields must be set.
struct Id3Info
{
    boost::optional<std::string> name;
    boost::optional<std::string> album;
    boost::optional<int> year;
};

/// Set a field of the optional Id3Info.
//
/// The object is created if it doesn't already exist.
///
/// @param member   The member to set, e.g. &Id3Info::name
/// @param val      The value for the member.
/// @param          An Id3Info object.
template<typename T>
inline void
setId3Info(boost::optional<T> Id3Info::*member, T const& val,
        boost::optional<Id3Info>& obj)
{
    if (!obj) obj = Id3Info();
    ((*obj).*member) = val;
}

}
}

#endif
