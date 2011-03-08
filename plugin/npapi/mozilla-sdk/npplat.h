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

#ifndef npplat_h_
#define npplat_h_

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#ifdef HAVE_NPUPP
#include "npupp.h"
#else
#include "npapi.h"
#include "npfunctions.h"
#endif

#ifdef XP_WIN
#include "windows.h"
#endif

#ifdef XP_UNIX
#include <stdio.h>
#endif

#ifdef XP_MAC
#include <Carbon/Carbon.h>
#endif

#ifndef HIBYTE
#define HIBYTE(i) (i >> 8)
#endif

#ifndef LOBYTE
#define LOBYTE(i) (i & 0xff)
#endif

#endif // npplat_h_

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
