

#ifndef GNASH_USER_FUNCTION_H
#define GNASH_USER_FUNCTION_H

#include "as_function.h"

namespace gnash {

class UserFunction : public as_function
{
public:

    virtual size_t registers() const = 0;

protected:

    UserFunction(Global_as& gl) : as_function(gl) {}

    virtual ~UserFunction() = 0;

};

inline UserFunction::~UserFunction() {}

}

#endif
