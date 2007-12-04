// tu_random.cpp	-- Thatcher Ulrich 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Pseudorandom number generator.


#include "tu_random.h"

#ifdef TEST_TU_RANDOM
	#include <cstdio>
#endif

namespace tu_random
{
	// Global generator.
	static generator	s_generator;

	boost::uint32_t	next_random()
	{
		return s_generator.next_random();
	}

	void	seed_random(boost::uint32_t seed)
	{
		s_generator.seed_random(seed);
	}

	float	get_unit_float()
	{
		return s_generator.get_unit_float();
	}


	// PRNG code adapted from the complimentary-multiply-with-carry
	// code in the article: George Marsaglia, "Seeds for Random Number
	// Generators", Communications of the ACM, May 2003, Vol 46 No 5,
	// pp90-93.
	//
	// The article says:
	//
	// "Any one of the choices for seed table size and multiplier will
	// provide a RNG that has passed extensive tests of randomness,
	// particularly those in [3], yet is simple and fast --
	// approximately 30 million random 32-bit integers per second on a
	// 850MHz PC.  The period is a*b^n, where a is the multiplier, n
	// the size of the seed table and b=2^32-1.  (a is chosen so that
	// b is a primitive root of the prime a*b^n + 1.)"
	//
	// [3] Marsaglia, G., Zaman, A., and Tsang, W.  Toward a universal
	// random number generator.  _Statistics and Probability Letters
	// 8_ (1990), 35-39.

//	const boost::uint64_t	a = 123471786;	// for SEED_COUNT=1024
//	const boost::uint64_t	a = 123554632;	// for SEED_COUNT=512
//	const boost::uint64_t	a = 8001634;	// for SEED_COUNT=255
//	const boost::uint64_t	a = 8007626;	// for SEED_COUNT=128
//	const boost::uint64_t	a = 647535442;	// for SEED_COUNT=64
//	const boost::uint64_t	a = 547416522;	// for SEED_COUNT=32
//	const boost::uint64_t	a = 487198574;	// for SEED_COUNT=16
	const boost::uint64_t	a = 716514398;	// for SEED_COUNT=8


	generator::generator()
		:
		c(362436),
		i(SEED_COUNT - 1)
	{
		seed_random(987654321);
	}


	void	generator::seed_random(boost::uint32_t seed)
	{
		// Simple pseudo-random to reseed the seeds.
		// Suggested by the above article.
		boost::uint32_t	j = seed;
		for (int i = 0; i < SEED_COUNT; i++)
		{
			j = j ^ (j << 13);
			j = j ^ (j >> 17);
			j = j ^ (j << 5);
			Q[i] = j;
		}
	}


	boost::uint32_t	generator::next_random()
	// Return the next pseudo-random number in the sequence.
	{
		boost::uint64_t	t;
		boost::uint32_t	x;

		//static boost::uint32_t	c = 362436;
		//static boost::uint32_t	i = SEED_COUNT - 1;
		const boost::uint32_t	r = 0xFFFFFFFE;

		i = (i+1) & (SEED_COUNT - 1);
		t = a * Q[i] + c;
		c = (boost::uint32_t) (t >> 32);
		x = (boost::uint32_t) (t + c);
		if (x < c)
		{
			x++;
			c++;
		}
		
		boost::uint32_t	val = r - x;
		Q[i] = val;
		return val;
	}

	
	float	generator::get_unit_float()
	{
		boost::uint32_t	r = next_random();

		// 24 bits of precision.
		return float(r >> 8) / (16777216.0f - 1.0f);
	}

}	// end namespace tu_random


#ifdef TEST_TU_RANDOM

// Compile with e.g.:
//
//  gcc -o tu_random_test tu_random.cpp -I.. -g -DTEST_TU_RANDOM -lstdc++
//
// Generate a test file of random numbers for DIEHARD.
int	main()
{
	const int	COUNT = 15000000 / 4;	// number of 4-byte words; DIEHARD needs ~80M bits

	for (int i = 0; i < COUNT; i++)
	{
		boost::uint32_t	val = tu_random::next_random();
		fwrite(&val, sizeof(val), 1, stdout);
	}
}


#endif // TEST_TU_RANDOM


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
