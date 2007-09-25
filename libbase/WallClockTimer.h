// WallClockTimer.h:  Wall clock timer, for Gnash.
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

#ifndef WALLCLOCKTIMER_H
#define WALLCLOCKTIMER_H 1

#include "tu_config.h" // for DSOEXPORT

#include <boost/cstdint.hpp>

namespace gnash {

/// General-purpose wall-clock timer.  
class WallClockTimer
{

public:

	/// Construct a wall-clock timer.
	WallClockTimer();

	/// \brief
	/// Return time elapsed in milliseconds since construction
	/// or last call to ::restart()
	DSOEXPORT uint32_t elapsed() const;

	/// Restart the timer
	DSOEXPORT void restart();

private:

	uint64_t startTimer;

};

} // namespace gnash


#endif // WALLCLOCKTIMER_H
