// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "check.h"
#include "Range2d.h"
#include <iostream>
#include <sstream>
#include <cassert>

using namespace std;
using namespace gnash;
using namespace gnash::geometry;

int
main(int /*argc*/, char** /*argv*/)
{

	//
	// Test NULL range construction
	//

	Range2d<int> nullIntRange1;
	check(   nullIntRange1.isNull() );
	check( ! nullIntRange1.isWorld() );
	check( ! nullIntRange1.isFinite() );

	Range2d<int> nullIntRange2(nullRange);
	check_equals( nullIntRange1, nullIntRange2 );
	check( nullIntRange1 == nullIntRange2 );

	//
	// Test WORLD range construction
	//

	Range2d<int> worldIntRange1(worldRange);
	check( ! worldIntRange1.isNull() );
	check(   worldIntRange1.isWorld() );
	check( ! worldIntRange1.isFinite() );
	check( nullIntRange1 != worldIntRange1 );

	//
	// Test FINITE range construction
	//

	Range2d<int> fIntRange1(0, 0, 10, 10);

	check( ! fIntRange1.isNull() );
	check( ! fIntRange1.isWorld() );
	check(   fIntRange1.isFinite() );
	check( fIntRange1 != worldIntRange1 );
	check( fIntRange1 != nullIntRange1 );

	//
	// Test expandToCircle()
	//
	
	Range2d<float> circleRange;
    circleRange.expandToCircle(50.0, 100.0, 10.0);
    check_equals( circleRange, Range2d<float>(40.0, 90.0, 60.0, 110.0));  


	//
	// Test growBy()
	//

	check_equals(
		Range2d<int>(0, 0, 1, 2).growBy(2),
		Range2d<int>(-2, -2, 3, 4) );

	// hit the numeric limits on each side
	unsigned uupbound = std::numeric_limits<unsigned>::max();
	// overflow xmin
	check_equals(
		Range2d<unsigned>(0, 3, 1, 6).growBy(2),
		Range2d<unsigned>(worldRange) );
	// overflow ymin
	check_equals(
		Range2d<unsigned>(3, 1, 7, 8).growBy(2),
		Range2d<unsigned int>(worldRange) );
	// overflow xmax
	check_equals(
		Range2d<unsigned>(10, 10, uupbound-1, 20).growBy(2),
		Range2d<unsigned int>(worldRange) );
	// overflow ymax
	check_equals(
		Range2d<unsigned>(10, 10, 20, uupbound-1).growBy(2),
		Range2d<unsigned int>(worldRange) );
	// overflow both direction so that min is still < max as a result
	// (this is tricky)
	check_equals(
		Range2d<unsigned>(1, 1,
			uupbound-1, uupbound-1).growBy(uupbound),
		Range2d<unsigned int>(worldRange) );

	//
	// Test shinkBy()
	//

	check_equals(
		Range2d<int>(0, 0, 10, 20).shrinkBy(2),
		Range2d<int>(2, 2, 8, 18) );

	// Collapse horizontally
	check_equals(
		Range2d<int>(0, 3, 1, 6).shrinkBy(2),
		Range2d<int>(nullRange) );
	// Collapse vertically
	check_equals(
		Range2d<int>(0, 3, 10, 6).shrinkBy(8),
		Range2d<int>(nullRange) );

	//
	// Test scale()
	//

	check_equals(
		Range2d<int>(0, 3, 10, 6).scale(2),
		Range2d<int>(0, 6, 20, 12) );

	check_equals(
		Range2d<int>(0, 3, 10, 6).scale(2, 3),
		Range2d<int>(0, 9, 20, 18) );

	check_equals(
		Range2d<int>(-1, 3, 10, 5).scale(3, .5),
		Range2d<int>(-3, 1, 30, 3) );

	check_equals(
		Range2d<unsigned int>(1, 3, 10, 5).scale(3, .5),
		Range2d<unsigned int>(3, 1, 30, 3) );

	check_equals(
		Range2d<float>(-1, 3, 10, 5).scale(3, .5),
		Range2d<float>(-3, 1.5, 30, 2.5) );

	check_equals( // tolerate 1/1000 floating point drift
		round(Range2d<float>(0, 0, 40, 80).scale(1.2, 1.0/20).width()*1000)/1000,
		48);

	check_equals(
		Range2d<float>(0, 0, 40, 80).scale(1.2, 1.0/20).height(),
		4);

	//
	// Test range Union 
	//

	Range2d<int> uIntNW1 = Union(nullIntRange1, worldIntRange1);
	Range2d<int> uIntNW2 = Union(worldIntRange1, nullIntRange1);

	// union is transitive
	check_equals( uIntNW1, uIntNW2 );

	// union of anything with world is world
	check_equals( uIntNW1, worldIntRange1 );

	Range2d<int> uIntNF1 = Union(nullIntRange1, fIntRange1);
	Range2d<int> uIntNF2 = Union(fIntRange1, nullIntRange1);

	// union is transitive
	check_equals( uIntNF1, uIntNF2 );

	// union of anything with null is no-op
	check_equals( uIntNF1, fIntRange1 );

	Range2d<int> uIntWF1 = Union(worldIntRange1, fIntRange1);
	Range2d<int> uIntWF2 = Union(fIntRange1, worldIntRange1);

	// union is transitive
	check_equals( uIntWF1, uIntWF2 );

	// union of anything with world is world
	check_equals( uIntWF1, worldIntRange1 );

	check_equals(
		Union( Range2d<int>(0, 0, 10, 10),
			Range2d<int>(-10, -10, -5, -5)),
		Range2d<int>(-10, -10, 10, 10)
	);

	// disjoint
	check_equals(
		Union( Range2d<int>(0, 0, 10, 10),
			Range2d<int>(-10, -10, 5, 5)),
		Range2d<int>(-10, -10, 10, 10)
	);

	// overlapping
	check_equals(
		Union( Range2d<int>(0, 0, 10, 10),
			Range2d<int>(-10, -10, 5, 5)),
		Range2d<int>(-10, -10, 10, 10)
	);
	check_equals(
		Union( Range2d<float>(-0.2, -0.3, 0.7, 0.8),
			Range2d<float>(-0.1, -0.1, 0.8, 0.9)),
		Range2d<float>(-0.2, -0.3, 0.8, 0.9)
	);

	// inscribed
	check_equals(
		Union( Range2d<int>(0, 0, 10, 10),
			Range2d<int>(2, 2, 5, 5)),
		Range2d<int>(0, 0, 10, 10)
	);
	check_equals(
		Union( Range2d<unsigned short>(2, 2, 8, 9),
			Range2d<unsigned short>(0, 1, 9, 10)),
		Range2d<unsigned short>(0, 1, 9, 10)
	);

	//
	// Test Intersection  / Intersects
	//

	Range2d<int> iIntNW1 = Intersection(nullIntRange1, worldIntRange1);
	Range2d<int> iIntNW2 = Intersection(worldIntRange1, nullIntRange1);

	// intersection is transitive
	check_equals( iIntNW1, iIntNW2 );

	// intersection of anything with null is null
	check_equals( iIntNW1, nullIntRange1 );
	check_equals( Intersect(nullIntRange1, worldIntRange1), false );

	// disjoint ranges
	check_equals(
		Intersection( Range2d<int>(0, 0, 10, 10),
			Range2d<int>(-10, -10, -2, -3)),
		Range2d<int>() // NULL range !
	);
	check_equals(
		Intersection( Range2d<double>(-100.4, 50.5, -60.4, 60),
			Range2d<double>(-80, -10, -70, -9)),
		Range2d<double>() // NULL range !
	);
	check_equals(
		Intersect( Range2d<double>(-100.4, 50.5, -60.4, 60),
			Range2d<double>(-80, -10, -70, -9)),
		false
	);

	// overlapping ranges
	check_equals(
		Intersection( Range2d<int>(0, 0, 10, 10),
			Range2d<int>(-10, -10, 5, 5) ),
		Range2d<int>(0, 0, 5, 5) 
	);
	check_equals(
		Intersect( Range2d<int>(0, 0, 10, 10),
			Range2d<int>(-10, -10, 5, 5)),
		true
	);

	// inscribed ranges
	check_equals(
		Intersection( Range2d<unsigned short>(0, 0, 10, 10),
			Range2d<unsigned short>(2, 2, 5, 5)),
		Range2d<unsigned short>(2, 2, 5, 5) 
	);
	check_equals(
		Intersect( Range2d<unsigned short>(0, 0, 10, 10),
			Range2d<unsigned short>(2, 2, 5, 5)),
		true
	);

	//
	// Test Range2d<float> to Range2d<int> cast
	//
	
	check_equals( (Range2d<int>)Range2d<float>(-0.1, 0.1, 10.1, 10.2),
			Range2d<int>(-1, 0, 11, 11) );


	//
	// Test Contains
	//

	// Null ranges don't contain anything, not even themselves
	check( ! nullIntRange1.contains(10, 10) );
	check( ! nullIntRange1.contains(nullIntRange1) );
	check( ! nullIntRange1.contains(worldIntRange1) );

	// World ranges contain everything except null ranges
	check( worldIntRange1.contains(10, -190) );
	check( worldIntRange1.contains(worldIntRange1) );
	check( ! worldIntRange1.contains(nullIntRange1) );

	// Self-containment
	check( fIntRange1.contains(fIntRange1) );

	cout << "fIntRange1 == " << fIntRange1 << endl;

	// Boundary overlaps
	check( fIntRange1.contains(0, 0) );
	check( fIntRange1.contains(10, 5) );
	check( fIntRange1.contains(5, 10) );
	check( fIntRange1.contains(5, 0) );
	check( fIntRange1.contains(Range2d<int>(0, 0, 2, 2)) );
	check( fIntRange1.contains(Range2d<int>(0, 4, 2, 6)) );
	check( fIntRange1.contains(Range2d<int>(0, 8, 2, 10)) );
	check( fIntRange1.contains(Range2d<int>(4, 8, 6, 10)) );
	check( fIntRange1.contains(Range2d<int>(8, 8, 10, 10)) );
	check( fIntRange1.contains(Range2d<int>(8, 4, 10, 6)) );
	check( fIntRange1.contains(Range2d<int>(8, 0, 10, 2)) );
	check( fIntRange1.contains(Range2d<int>(4, 0, 6, 2)) );

	// Strict containment
	check( fIntRange1.contains(Range2d<int>(2, 2, 4, 4)) );
	check( fIntRange1.contains(5, 5) );

	// Intersection (partial overlap)
	check( ! fIntRange1.contains(Range2d<int>(-2, 0, 2, 2)) );
	check( ! fIntRange1.contains(Range2d<int>(8, 8, 10, 11)) );
	check( ! fIntRange1.contains(Range2d<int>(8, 8, 11, 11)) );
	

}

