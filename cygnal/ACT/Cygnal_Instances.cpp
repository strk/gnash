// 
// Copyright (C) 2007 Free Software Foundation, Inc.
//
// This file is part of GNU Cygnal.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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

/**	\file Cygnal_Instance.cpp
 *	\brief Template instances of Scheduling_Queue etc. needed for Cygnal.
 *
 *	Because we define the implementations of template functions outside of its declaration in its header,
 *		it's necessary to manually instantiate the classes actually used.
 *	This instantiation is in a separate file to support unit testing.
 *	Testing uses different template parameters.
 *	Separating definition and instantiation alleviates problems with multiply-defined symbols.
 */

#include "Scheduling_Queue.cpp"

namespace ACT {
	// Explicit template instantiation.
	template Scheduling_Queue< Basic_Scheduled_Item > ;

} // end namespace ACT
