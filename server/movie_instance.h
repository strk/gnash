// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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

// Stateful live Movie instance 


#ifndef GNASH_MOVIE_INSTANCE_H
#define GNASH_MOVIE_INSTANCE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <vector>

#include "sprite_instance.h" // for inheritance

// Forward declarations
namespace gnash {
	class movie_root; 
	class character; 
	class movie_def_impl;
}

namespace gnash
{

/// Stateful Movie object (a special kind of sprite)
class movie_instance : public sprite_instance
{

public:

	movie_instance(movie_def_impl* def,
		movie_root* r, character* parent);

	virtual ~movie_instance() {}

	virtual void advance(float delta_time);

private:

	movie_def_impl* _def;
};


} // end of namespace gnash

#endif // GNASH_MOVIE_INSTANCE_H
