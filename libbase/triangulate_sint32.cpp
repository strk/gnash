// triangulate_sint32.cpp	-- Thatcher Ulrich 2004

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Code to triangulate arbitrary 2D polygonal regions.
//
// Instantiate our templated algo from triangulate_inst.h

/* $Id: triangulate_sint32.cpp,v 1.5 2007/02/10 18:33:03 nihilus Exp $ */

#include "triangulate_impl.h"

using namespace std;

namespace triangulate
{
	// Version using sint32 coords
	void	compute(
		std::vector<sint32>* result,	// trilist
		int path_count,
		const std::vector<sint32> paths[],
		int debug_halt_step /* = -1 */,
		std::vector<sint32>* debug_remaining_loop /* = NULL */)
	{
		compute_triangulation<sint32>(result, path_count, paths, debug_halt_step, debug_remaining_loop);
	}
}
