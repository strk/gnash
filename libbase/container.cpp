// container.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Some non-inline implementation help for generic containers.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cmath>
#include <cstdio>
#include <cstdarg>

#include "container.h"
#include "utf8.h"
#include "tu_random.h"


#ifdef CONTAINER_UNIT_TEST


// Compile this test case with something like:
//
// gcc container.cpp utf8.cpp tu_random.cpp -g -I.. -DCONTAINER_UNIT_TEST -lstdc++ -o container_test
//
//    or
//
// cl container.cpp utf8.cpp tu_random.cpp -Zi -Od -DCONTAINER_UNIT_TEST -I..


void	test_hash()
{
	// Collect a bunch of random key/value pairs.
	std::vector<uint32_t>	data;
	for (int i = 0; i < 1000; i++)
	{
		data.push_back(tu_random::next_random());
	}

	// Push into hash.
	hash<uint32_t, uint32_t>	h;
	{for (int i = 0; i < data.size() / 2; i++)
	{
		h.add(data[i*2], data[i*2 + 1]);

		// Verify the contents of the hash so far.
		for (int j = 0; j < i; j++)
		{
			uint32_t	key = data[j*2];
			uint32_t	val;
			bool	got = h.get(key, &val);
			assert(got);
			assert(val == data[j*2 + 1]);
		}
	}}

	// Manually copy stuff over to h2, using iterator interface.
	hash<uint32_t, uint32_t>	h2;
	{for (hash<uint32_t, uint32_t>::iterator it = h.begin(); it != h.end(); ++it)
	{
		//printf("first = 0x%X, second = 0x%X\n", it->first, it->second);//xxxxx
		assert(h.get(it->first, NULL) == true);

		h2.add(it->first, it->second);

		uint32_t	val;
		bool	got = h2.get(it->first, &val);
		assert(got);
		assert(val == it->second);
	}}

	// Verify the contents of h2.
	{for (int i = 0; i < data.size() / 2; i++)
	{
		uint32_t	key = data[i*2];
		uint32_t	val;
		bool	got = h.get(key, &val);
		assert(got);
		assert(val == data[i*2 + 1]);
	}}

	h.clear();
	assert(h.size() == 0);

	// Verify that h really is missing the stuff it had before, and h2 really has it.
	{for (hash<uint32_t, uint32_t>::iterator it = h2.begin(); it != h2.end(); ++it)
	{
		assert(h.get(it->first, NULL) == false);
		assert(h2.get(it->first, NULL) == true);
		assert(h.find(it->first) == h.end());
		assert(h2.find(it->first) != h2.end());
	}}
}


//#include <ext/hash_map>
//#include <hash_map>
//#include <map>

void	test_hash_speed()
// Test function for hash performance of adding keys and doing lookup.
{

// Hash type, for doing comparative tests.
//
// tu_hash tests faster than the map and hash_map included with GCC
// 3.3 as well as map and hash_map from MSVC7.  In some cases, several
// times faster.  GCC's hash_map is the closest to tu_hash, but is
// still 33% slower in my tests.

// // tu's hash
#define HASH hash<uint32_t, uint32_t >
#define HASH_ADD(h, k, v) h.add(k, v)

// STL's hash
//#define HASH __gnu_cxx::hash_map<uint32_t, uint32_t>
//#define HASH std::hash_map<uint32_t, uint32_t>
//#define HASH_ADD(h, k, v) h[k] = v

// STL's map
//#define HASH std::map<uint32_t, uint32_t>
//#define HASH_ADD(h, k, v) h[k] = v

//	const int	SIZE = 10000000;
	const int	SIZE = 1000000;

	// Make an array of random numbers.
	std::vector<uint32_t>	numbers;
	numbers.resize(SIZE);

	for (int i = 0, n = numbers.size(); i < n; i++)
	{
		numbers[i] = tu_random::next_random();
	}

	// Uniquify the array.
	HASH	new_index;
	int	next_new_index = 0;
	{for (int i = 0, n = numbers.size(); i < n; i++)
	{
		HASH::iterator	it = new_index.find(numbers[i]);
		if (it == new_index.end())
		{
			// First time this number has been seen.
			HASH_ADD(new_index, numbers[i], next_new_index);
			next_new_index++;
		}
		else
		{
			// This number already appears in the list.
// 			printf("duplicate entry %x, prev new index %d, current array index %d\n",
// 				   numbers[i],
// 				   it->second,
// 				   i);
		}
	}}

	printf("next_new_index = %d\n", next_new_index);
}

#endif // CONTAINER_UNIT_TEST


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
