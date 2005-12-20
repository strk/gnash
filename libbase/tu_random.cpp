// tu_random.cpp	-- Thatcher Ulrich 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Pseudorandom number generator.


#include "tu_random.h"


namespace tu_random
{
	// Global generator.
	static generator	s_generator;

	Uint32	next_random()
	{
		return s_generator.next_random();
	}

	void	seed_random(Uint32 seed)
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

//	const Uint64	a = 123471786;	// for SEED_COUNT=1024
//	const Uint64	a = 123554632;	// for SEED_COUNT=512
//	const Uint64	a = 8001634;	// for SEED_COUNT=255
//	const Uint64	a = 8007626;	// for SEED_COUNT=128
//	const Uint64	a = 647535442;	// for SEED_COUNT=64
//	const Uint64	a = 547416522;	// for SEED_COUNT=32
//	const Uint64	a = 487198574;	// for SEED_COUNT=16
	const Uint64	a = 716514398;	// for SEED_COUNT=8


	generator::generator()
		:
		c(362436),
		i(SEED_COUNT - 1)
	{
		seed_random(987654321);
	}


	void	generator::seed_random(Uint32 seed)
	{
		// Simple pseudo-random to reseed the seeds.
		// Suggested by the above article.
		Uint32	j = seed;
		for (int i = 0; i < SEED_COUNT; i++)
		{
			j = j ^ (j << 13);
			j = j ^ (j >> 17);
			j = j ^ (j << 5);
			Q[i] = j;
		}
	}


	Uint32	generator::next_random()
	// Return the next pseudo-random number in the sequence.
	{
		Uint64	t;
		Uint32	x;

		//static Uint32	c = 362436;
		//static Uint32	i = SEED_COUNT - 1;
		const Uint32	r = 0xFFFFFFFE;

		i = (i+1) & (SEED_COUNT - 1);
		t = a * Q[i] + c;
		c = (Uint32) (t >> 32);
		x = (Uint32) (t + c);
		if (x < c)
		{
			x++;
			c++;
		}
		
		Uint32	val = r - x;
		Q[i] = val;
		return val;
	}

	
	float	generator::get_unit_float()
	{
		Uint32	r = next_random();

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
		Uint32	val = tu_random::next_random();
		fwrite(&val, sizeof(val), 1, stdout);
	}
}


#endif // TEST_TU_RANDOM


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
