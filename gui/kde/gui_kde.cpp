// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include "gui.h"
#include "GnashException.h"

#ifdef GUI_KDE3
#include "kdesup.h"
#endif

namespace gnash {

#ifdef GUI_KDE3
std::auto_ptr<Gui> createKDEGui(unsigned long windowid, float scale, bool do_loop, RunResources& r)
{
	return std::auto_ptr<Gui>(new KdeGui(windowid, scale, do_loop, r));
}
#else // ! GUI_KDE3
std::auto_ptr<Gui> createKDEGui(unsigned long , float, bool, RunResourcesfloat , bool , unsigned int )
{
	throw GnashException("Support for KDE gui was not compiled in");
}
#endif // ! GUI_KDE3

} // namespace gnash

