
#ifndef GNASH_TRANSFORM_H
#define GNASH_TRANSFORM_H

#include "SWFMatrix.h"
#include "cxform.h"
#include "SWFRect.h"

namespace gnash {

class Transform
{
public:
    Transform(const SWFMatrix& m = SWFMatrix(), const cxform& cx = cxform())
        :
        matrix(m),
        colorTransform(cx)
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
    cxform colorTransform;
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
