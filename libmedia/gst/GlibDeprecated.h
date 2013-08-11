/* 
 *   Copyright (C) 2013 Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */ 

#ifndef GLIB_DEPRECATED_H
#define GLIB_DEPRECATED_H

// deprecated since 2.32: g_value_array_get_nth, g_value_array_free
// gstreamer devs keep using GValueArray deprecated in favour of GArray
// http://lists.freedesktop.org/archives/gstreamer-devel/2012-October/037539.html
#define GLIB_DISABLE_DEPRECATION_WARNINGS
#include <glib-object.h>
#undef GLIB_DISABLE_DEPRECATION_WARNINGS

#endif
