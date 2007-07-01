// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
//

/* $Id: aqua.cpp,v 1.10 2007/07/01 10:54:00 bjacques Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

extern "C"{
#include <unistd.h>
#ifdef HAVE_GETOPT_H
	#include <getopt.h>
#endif
//	extern int getopt(int, char *const *, const char *);
}

#include "gnash.h"
#include "gui.h"
#include "aquasup.h"
#include "log.h"
#include "movie_root.h"

#include "render_handler.h"

#include <Carbon/Carbon.h>

namespace gnash {
	
AquaGui::AquaGui()
: Gui() 
{

}

AquaGui::AquaGui(unsigned long xid, float scale, bool loop, unsigned int depth)
	: Gui(xid, scale, loop, depth)
{
}

AquaGui::~AquaGui()
{
	
}

void AquaGui::setInterval(unsigned int interval)
{
    _interval = interval;
}

bool AquaGui::run()
{
    GNASH_REPORT_FUNCTION;
    return false;
}

void AquaGui::renderBuffer()
{
    //GNASH_REPORT_FUNCTION;

    _glue.render();
}

bool AquaGui::init(int /*argc*/, char *** /*argv*/) /* Self-explainatory */
{
	return true;
}

void AquaGui::setTimeout(unsigned int timeout)
{
    _timeout = timeout;
}

void AquaGui::key_event(int key, bool down)
{
}

bool AquaGui::createWindow(const char* /*title*/, int /*width*/, int /*height*/)
{
	return true;
}

bool AquaGui::createMenu()
{ 
	return true;
}

bool AquaGui::setupEvents()
{
	return true;
}

}