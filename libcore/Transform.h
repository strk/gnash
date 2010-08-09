
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

    /// Concatenate a Transform.
    //
    /// This produces a new Transform.
    Transform concatenate(const Transform& other) const
    {
        Transform ret(*this);
        ret.matrix.concatenate(other.matrix);
        ret.colorTransform.concatenate(other.colorTransform);
        return ret;
    }

    SWFMatrix matrix;
    cxform colorTransform;
};

}

#endif
