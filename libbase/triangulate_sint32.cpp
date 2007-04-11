// triangulate_sint32.cpp	-- Thatcher Ulrich 2004

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Code to triangulate arbitrary 2D polygonal regions.
//
// Instantiate our templated algo from triangulate_inst.h

/* $Id: triangulate_sint32.cpp,v 1.6 2007/04/11 17:54:21 bjacques Exp $ */

#include "triangulate_impl.h"

using namespace std;

namespace triangulate
{
	// Version using int32_t coords
	void	compute(
		std::vector<int32_t>* result,	// trilist
		int path_count,
		const std::vector<int32_t> paths[],
		int debug_halt_step /* = -1 */,
		std::vector<int32_t>* debug_remaining_loop /* = NULL */)
	{
		compute_triangulation<int32_t>(result, path_count, paths, debug_halt_step, debug_remaining_loop);
	}
}
