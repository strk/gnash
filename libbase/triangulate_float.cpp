// triangulate_float.cpp	-- Thatcher Ulrich 2004

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Code to triangulate arbitrary 2D polygonal regions.
//
// Instantiate our templated algo from triangulate_inst.h

/* $Id: triangulate_float.cpp,v 1.5 2007/02/10 18:33:03 nihilus Exp $ */

#include "triangulate_impl.h"

using namespace std;

namespace triangulate
{
	// Version using float coords
	void	compute(
		std::vector<float>* result,	// trilist
		int path_count,
		const std::vector<float> paths[],
		int debug_halt_step /* = -1 */,
		std::vector<float>* debug_remaining_loop /* = NULL */)
	{
		compute_triangulation<float>(result, path_count, paths, debug_halt_step, debug_remaining_loop);
	}
}



#ifdef TEST_TRIANGULATE_FLOAT

// Compile test with something like:
//
// gcc -o triangulate_test -I../ triangulate_float.cpp tu_random.cpp -DTEST_TRIANGULATE_FLOAT -lstdc++
//
// or
//
// cl -Od -Zi -o triangulate_test.exe -I../ triangulate_float.cpp tu_random.cpp -DTEST_TRIANGULATE_FLOAT


void	test_square()
// A very minimal, easy test.
{
	std::vector<float>	result;
	std::vector<std::vector<float> >	paths;

	// Make a square.
	paths.resize(1);
	paths[0].push_back(0);
	paths[0].push_back(0);
	paths[0].push_back(1);
	paths[0].push_back(0);
	paths[0].push_back(1);
	paths[0].push_back(1);
	paths[0].push_back(0);
	paths[0].push_back(1);

	// Triangulate.
	triangulate::compute(&result, paths.size(), &paths[0]);

	// Dump.
	for (int i = 0; i < result.size(); i++)
	{
		printf("%f\n", result[i]);
	}
}


int	main()
{
	test_square();

	return 0;
}


#endif // TEST_TRIANGULATE_FLOAT

