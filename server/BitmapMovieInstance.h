// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// 
//

/* $Id: BitmapMovieInstance.h,v 1.2 2007/02/14 09:03:53 strk Exp $ */

#ifndef GNASH_BITMAPMOVIEINSTANCE_H
#define GNASH_BITMAPMOVIEINSTANCE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "movie_instance.h" // for inheritance

// Forward declarations
namespace gnash
{
	class BitmapMovieDefinition;
}

namespace gnash
{


/// Instance of a BitmapMovieDefinition
class BitmapMovieInstance : public movie_instance
{

public:

	BitmapMovieInstance(BitmapMovieDefinition* def); 

	virtual ~BitmapMovieInstance() {}

};

} // end of namespace gnash

#endif // GNASH_BITMAPMOVIEINSTANCE_H
