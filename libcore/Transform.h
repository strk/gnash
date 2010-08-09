
#ifndef GNASH_TRANSFORM_H
#define GNASH_TRANSFORM_H

#include "SWFMatrix.h"
#include "cxform.h"
#include "SWFRect.h"

namespace gnash {

class Transform
{
public:
    Transform(const SWFMatrix& m = SWFMatrix(), const cxform& cx = cxform(),
            const SWFRect& r = SWFRect())
        :
        matrix(m),
        colorTransform(cx),
        rect(r)
    {}

    SWFMatrix matrix;
    cxform colorTransform;
    SWFRect rect;
};

}

#endif
