
#include "movie_root.h"
#include "as_object.h"

namespace gnash {


/// Destructor of ActiveRelay needs definition of movie_root.
ActiveRelay::~ActiveRelay()
{
    getRoot(*_owner).removeAdvanceCallback(this);
}


void
ActiveRelay::setReachable()
{
    markReachableResources();
    _owner->setReachable();
}

}
