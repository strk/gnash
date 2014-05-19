// gui_gtk.cpp: Wrapper for Gnome ToolKit graphical user interface, for Gnash.
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

#ifdef GUI_GTK
#include "gtksup.h"
#endif

namespace gnash {

#ifdef GUI_GTK
std::unique_ptr<Gui> createGTKGui(unsigned long windowid, float scale, bool do_loop, RunResources& r)
{
	return std::unique_ptr<Gui>(new GtkGui(windowid, scale, do_loop, r));
}
#else // ! GUI_GTK
std::unique_ptr<Gui> createGTKGui(unsigned long , float, bool, RunResourcesfloat , bool , unsigned int )
{
	throw GnashException("Support for GTK gui was not compiled in");
}
#endif // ! GUI_GTK

} // namespace gnash

