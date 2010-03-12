// impl.cpp:  Implement ActionScript tags, movie loading, library, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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
//

#include "MovieFactory.h"
#include "smart_ptr.h" // GNASH_USE_GC
#include "IOChannel.h"
#include "utility.h"
#include "fontlib.h"
#include "log.h"
#include "GnashImage.h"
#include "sprite_definition.h"
#include "SWFMovieDefinition.h"
#include "BitmapMovieDefinition.h"
#include "RunResources.h"
#include "URL.h"
#include "StreamProvider.h"
#include "MovieClip.h"
#include "VM.h"
#include "MovieLibrary.h"
#include "gnash.h" // DSOEXPORTS

#ifdef GNASH_USE_GC
#include "GC.h"
#endif

#include <string>
#include <map>
#include <memory> // for auto_ptr
#include <algorithm>

namespace gnash
{

//
// global gnash management
//

// Maximum release of resources.
void  clear()
{
    // Ideally, we should make sure that function properly signals all threads
    // about exiting and giving them a chance to cleanly exit.
    //
    // If we clear shared memory here we're going to leave threads possibly
    // accessing deleted memory, which would trigger a segfault.
    //
    // My experience is that calling exit(), altought it has the same problem,
    // reduces the chances of segfaulting ...
    //
    // We want this fixed anyway as exit()
    // itselt can also trigger segfaults.
    //
    // See task task #6959 and depending items
    //
    log_debug("Any segfault past this message is likely due to improper "
            "threads cleanup.");

    VM::get().clear();

    MovieFactory::movieLibrary.clear();
    fontlib::clear();

#ifdef GNASH_USE_GC 
    GC::get().fuzzyCollect(); // why would this be needed ?

    GC::cleanup();
#endif

}

#ifdef GNASH_USE_GC
/// A GC root used to mark all reachable collectable pointers
class GnashGcRoot : public GcRoot 
{

public:

  GnashGcRoot()
  {
  }

  void markReachableResources() const
  {
    VM::get().markReachableResources();
  }
};
#endif

void gnashInit()
{
#ifdef GNASH_USE_GC
  static GnashGcRoot gcRoot;
  GC::init(gcRoot);
#endif
}

} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
