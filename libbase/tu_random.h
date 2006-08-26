// tu_random.h	-- Thatcher Ulrich 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Pseudorandom number generator.


#ifndef TU_RANDOM_H
#define TU_RANDOM_H


#include "tu_config.h"
#include "tu_types.h"


namespace tu_random
{
	// Global generator.
	uint32_t	next_random();
	void	seed_random(uint32_t seed);
	float	get_unit_float();

	// In case you need independent generators.  The global
	// generator is just an instance of this.
	const int	SEED_COUNT = 8;
	class generator
	{
	public:
		generator();
		void	seed_random(uint32_t seed);	// not necessary
		uint32_t	next_random();
		float	get_unit_float();

	private:
		uint32_t	Q[SEED_COUNT];
		uint32_t	c;
		uint32_t	i;
	};

}	// end namespace tu_random


#endif // TU_RANDOM_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
