

#include "DefinitionTag.h"
#include "MovieClip.h"
#include "Movie.h"

namespace gnash {
namespace SWF {

void
DefinitionTag::executeState(MovieClip* m, DisplayList& /*dlist*/) const
{
    m->get_root()->addCharacter(_id);
}


}
}
