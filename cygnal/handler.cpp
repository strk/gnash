// 
//   Copyright (C) 2008, 2009 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <algorithm>
#include <string>
#include <deque>
#include <list>
#include <map>
#include <vector>

#include "log.h"
#include "network.h"
#include "buffer.h"
#include "utility.h"
#include "dsodefs.h" //For DSOEXPORT.
#include "handler.h"

#include "rtmp.h"
#include "http.h"

using namespace gnash;
using namespace std;
using namespace boost;

namespace cygnal
{

map<int, Handler *> DSOEXPORT handlers;

Handler::Handler()
    : _in_fd(0)
{
//    GNASH_REPORT_FUNCTION;
}

Handler::~Handler()
{
//    GNASH_REPORT_FUNCTION;
}

bool
Handler::sync(int in_fd)
{
//    GNASH_REPORT_FUNCTION;

}

// Dump internal data.
void
Handler::dump()
{
//    GNASH_REPORT_FUNCTION;
}

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

