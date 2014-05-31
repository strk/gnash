// Transform.h:  Transform information for rendering.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#ifndef GNASH_TRANSFORM_H
#define GNASH_TRANSFORM_H

#include "SWFMatrix.h"
#include "SWFCxForm.h"
#include <utility>

namespace gnash {

/// The Transform class expresses a stage in a cumulative transformation
//
/// All DisplayObjects have a color transform and a matrix, which is
/// concatenated with its parent's transform to produce the actual transform.
class Transform
{
public:

    /// Construct a Transform
    //
    /// Any arguments not supplied are identity transformations.
    explicit Transform(SWFMatrix m = SWFMatrix(),
            SWFCxForm cx = SWFCxForm())
        :
        matrix(std::move(m)),
        colorTransform(std::move(cx))
    {}

    Transform(const Transform& other)
        :
        matrix(other.matrix),
        colorTransform(other.colorTransform)
    {}

    Transform& operator*=(const Transform& other) {
        matrix.concatenate(other.matrix);
        colorTransform.concatenate(other.colorTransform);
        return *this;
    }

    SWFMatrix matrix;
    SWFCxForm colorTransform;
};

/// Concatenate two transforms.
inline Transform
operator*(const Transform& a, const Transform& b)
{
    Transform ret(a);
    ret *= b;
    return ret;
}

} // namespace gnash

#endif
