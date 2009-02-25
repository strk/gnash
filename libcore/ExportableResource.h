// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifndef GNASH_RESOURCE_H
#define GNASH_RESOURCE_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "ref_counted.h" // for inheritance
#include "smart_ptr.h"

namespace gnash {

/// A class for SWF resources that may be exported
//
/// They are: character_def, sound_sample, and font.
/// These may be held in the export map of a SWFMovieDefinition.
class ExportableResource : public ref_counted
{
protected:
    ExportableResource() {}
	virtual ~ExportableResource() {}
	
};


} // namespace gnash

#endif // GNASH_RESOURCE_H
