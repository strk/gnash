// DisplayObjectContainer.h: Container of DisplayObjects.
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


#ifdef HAVE_CONFIG_H
# include "gnashconfig.h" 
#endif

#include "DisplayObjectContainer.h"

#include <utility>

#include "DisplayList.h" 
#include "InteractiveObject.h"
#include "log.h"

namespace gnash {

DisplayObjectContainer::~DisplayObjectContainer()
{
}

#ifdef USE_SWFTREE

namespace {

class MovieInfoVisitor
{

public:
    MovieInfoVisitor(DisplayObject::InfoTree& tr,
            DisplayObject::InfoTree::iterator it)
        :
        _tr(tr),
        _it(it)
    {}

    void operator()(DisplayObject* ch) {
        ch->getMovieInfo(_tr, _it);
    }

private:

    DisplayObject::InfoTree& _tr;
    DisplayObject::InfoTree::iterator _it;

};

} // anonymous namespace

DisplayObject::InfoTree::iterator 
DisplayObjectContainer::getMovieInfo(InfoTree& tr, InfoTree::iterator it)
{
    InfoTree::iterator selfIt = DisplayObject::getMovieInfo(tr, it);
    std::ostringstream os;
    os << _displayList.size();
    InfoTree::iterator localIter = tr.append_child(selfIt,
            std::make_pair(_("Children"), os.str()));            

    MovieInfoVisitor v(tr, localIter);
    _displayList.visitAll(v);

    return selfIt;

}

#endif // USE_SWFTREE

} // namespace gnash
