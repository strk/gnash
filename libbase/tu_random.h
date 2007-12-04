// tu_random.h	-- Thatcher Ulrich 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Pseudorandom number generator.


#ifndef TU_RANDOM_H
#define TU_RANDOM_H


#include "tu_config.h" // needed ?

#include <boost/cstdint.hpp> // for boost::uint32_t used in this file

namespace tu_random
{
	// Global generator.
	DSOEXPORT boost::uint32_t	next_random();
	void	seed_random(boost::uint32_t seed);
	DSOEXPORT float	get_unit_float();

	// In case you need independent generators.  The global
	// generator is just an instance of this.
	const int	SEED_COUNT = 8;
	class generator
	{
	public:
		generator();
		void	seed_random(boost::uint32_t seed);	// not necessary
		boost::uint32_t	next_random();
		float	get_unit_float();

	private:
		boost::uint32_t	Q[SEED_COUNT];
		boost::uint32_t	c;
		boost::uint32_t	i;
	};

}	// end namespace tu_random


#endif // TU_RANDOM_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
