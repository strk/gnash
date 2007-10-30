// bsp.cpp	-- by Thatcher Ulrich <tu@tulrich.com> 2001

// This code is in the public domain.

// Code to make a collision-bsp tree out of triangle soup.

#include <cstdio>
#include <cmath>

#ifdef HAVE_MALLOC_H
	#include <malloc.h>
#else
	#include <cstdlib>	
#endif

using namespace std;

#include "utility.h"
#include "bsp.h"

const bool	print_debug = 0;

/*
sort faces by size?  Or order them randomly?;
make root bsp node w/ the first face;
for rest of faces {
	root->add_face( a, b, c, plane( normal, d ) );
}
*/


enum plane_class {
	INSIDE = -1,
	ON = 0,
	OUTSIDE = 1
};


////const float	BSP_SLOP = 1.1923435f;
//const float	BSP_SLOP = 0.1f;

plane_class	classify_point( const plane_info& p, vec3 a, float slop )
// Classify the given point with respect to the given plane; use a tolerance
// of +/- slop to determine when a point is ON the plane.
{
	float	distance = p.normal * a - p.d;

	if ( distance < -slop ) {
		return INSIDE;
	} else if ( distance > slop ) {
		if ( print_debug ) {
			printf( "d = %f, pn = %f %f %f, pd = %f, a = %f %f %f, p*a = %f\n",
					distance, p.normal.get_x(), p.normal.get_y(), p.normal.get_z(), p.d,
					a.get_x(), a.get_y(), a.get_z(),
					p.normal * a
				);	//xxxx
		}
		return OUTSIDE;
	} else {
		return ON;
	}
}


vec3	intersect( const plane_info& p, const vec3& a, const vec3& b )
// Returns the point of intersection between the plane p and the
// line segment defined by a and b.
//
// NB: does not check to ensure that a & b intersects p!
{
	float	da = p.normal * a - p.d;
	float	db = p.normal * b - p.d;

	float	diff = db - da;

	if ( fabs( diff ) < 0.000001f ) {
		// Segment is parallel to plane.  Just pick the midpoint of
		// the segment as the intersection point.
		return ( a + b ) * 0.5f;

	} else {
		// Compute how far along the segment the intersection is.
		float	f = ( 0 - da ) / diff;
//		printf( "f = %f\n", f );//xxxxx
		return a + ( b - a ) * f;
	}
}


bsp_node::bsp_node( const plane_info& p )
// Constructor.  Make a root node using the given plane info.
{
	m_plane = p;
	m_inside = m_outside = 0;
	m_partitioning_plane = true;

	m_face_list = 0;
	m_face_count = 0;
}


bsp_node::~bsp_node()
// Destructor.  Delete our child trees recursively.
{
	if ( m_inside ) {
		delete m_inside;
	}
	if ( m_outside ) {
		delete m_outside;
	}
	if ( m_face_list ) {
		free( m_face_list );	// using malloc/free because of realloc...
	}
}


void	bsp_node::add_partition( const plane_info& p )
// Add a partitioning plane to this bsp tree.  Basically propagates
// the plane and adds it to all leaf nodes.
//
// NB: VERY IMPORTANT: You must add partitioning planes before adding
// faces.  Otherwise volume queries will be wrong.
{
	if ( m_partitioning_plane != true ) {
		abort();
		// Trying to add a partitioning plane to a tree that already
		// contains faces.  Refuse to add the plane.
		return;
	}

	if ( m_inside ) {
		m_inside->add_partition( p );
	} else {
		m_inside = new bsp_node( p );
	}

	if ( m_outside ) {
		m_outside->add_partition( p );
	} else {
		m_outside = new bsp_node( p );
	}
}


void	bsp_node::add_face( const vec3& a, const vec3& b, const vec3& c, const plane_info& p, int face_index, float slop )
// Inserts the given triangle into this node's bsp tree.  face_index
// is a reference to the original face, which is passed back during
// ray_cast() if the caller gives a pointer to a face_test_callback
// function.
//
// The slop parameter determines how far a vertex may be from a
// splitting plane and still be considered "on" the plane.
//
// @@ slop could possibly be replaced by using a fraction of the
// longest face edge length.
{
	// Classify the three verts of the triangle w/r/t this node's plane.
	plane_class	ca = classify_point( m_plane, a, slop );
	plane_class	cb = classify_point( m_plane, b, slop );
	plane_class	cc = classify_point( m_plane, c, slop );

	if ( print_debug ) {
//x		printf( "ca = %d, cb = %d, cc = %d\n", ca, cb, cc );//xxx
	}

	if ( ca == ON && cb == ON && cc == ON ) {
		// All points are in this node's plane.

		if ( p.normal * m_plane.normal < 0 ) {
			// Face's plane and our node plane are opposite each other.
			// Add the face on the outside of this plane.
			add_outside_face( a, b, c, p, face_index, slop );
		} else {
			// This face fits in this node.  Add the face index to our list.
			if ( print_debug ) printf( "o\n" );//xxxxx

			insert_into_face_list( face_index );
		}
		return;

	} else if ( ( ca && cb && ( ca != cb ) )
				|| ( cb && cc && ( cb != cc ) )
				|| ( cc && ca && ( cc != ca ) ) )
	{
//		printf( "*\n" );
//		return;	//xxxxxxxx

		// This triangle straddles the plane.

		// Make references to the verts, so we can sort them.
		const vec3*	pa = &a;
		const vec3*	pb = &b;
		const vec3*	pc = &c;

		// Sort the vert references so that *pa is the most inside, and *pc is the most outside.
		const vec3*	t;
		plane_class	ct;
		if ( ca > cb ) {
			t = pa; pa = pb; pb = t;	// swap( &a, &b );
			ct = ca; ca = cb; cb = ct;
		}
		if ( cb > cc ) {
			t = pb; pb = pc; pc = t;	// swap( &b, &c );
			ct = cb; cb = cc; cc = ct;
		}
		if ( ca > cb ) {
			t = pa; pa = pb; pb = t;	// swap( &a, &b );
			ct = ca; ca = cb; cb = ct;
		}

		if ( cb == INSIDE ) {
			if ( print_debug ) printf( "^" );//xxxx

			// a and b are INSIDE the plane, c is OUTSIDE.
			vec3	d = intersect( m_plane, *pa, *pc );
			vec3	e = intersect( m_plane, *pb, *pc );

			add_inside_face( *pa, *pb, d, p, face_index, slop );
			add_inside_face( *pb, d, e, p, face_index, slop );
			add_outside_face( d, e, *pc, p, face_index, slop );

		} else if ( cb == ON ) {
			if ( print_debug ) printf( "<" );//xxxx

			// a is INSIDE, b is ON, c is OUTSIDE.
			vec3	d = intersect( m_plane, *pa, *pc );

			add_inside_face( *pa, *pb, d, p, face_index, slop );
			add_outside_face( *pb, d, *pc, p, face_index, slop );

		} else {
			if ( print_debug ) printf( "V: " );//xxxx

			// a is INSIDE, b and c are OUTSIDE.
			vec3	d = intersect( m_plane, *pa, *pb );
			vec3	e = intersect( m_plane, *pa, *pc );

			add_inside_face( *pa, d, e, p, face_index, slop );
			add_outside_face( d, e, *pb, p, face_index, slop );
			add_outside_face( e, *pb, *pc, p, face_index, slop );
		}

	} else {
		// This triangle is fully on one side of the plane or the other.
		if ( ca == INSIDE || cb == INSIDE || cc == INSIDE ) {
			add_inside_face( a, b, c, p, face_index, slop );

		} else {
			add_outside_face( a, b, c, p, face_index, slop );
		}
	}
}


void	bsp_node::add_inside_face( const vec3& a, const vec3& b, const vec3& c, const plane_info& p, int face_index, float slop )
// Adds the given triangle with the specified plane info to the inside
// half of this node.  Creates a new inside node if necessary.
{
	if ( print_debug ) printf( "/" );//xxxx
	if ( m_inside ) {
		m_inside->add_face( a, b, c, p, face_index, slop );
	} else {
		if ( print_debug ) printf( "x\n" );//xxxxx
		m_inside = new bsp_node( p );
		m_inside->insert_into_face_list( face_index );
	}
}


void	bsp_node::add_outside_face( const vec3& a, const vec3& b, const vec3& c, const plane_info& p, int face_index, float slop )
// Adds the given triangle with the specified plane info to the outside
// half-space of this node.  Creates a new outside node if necessary.
{
	if ( print_debug ) printf( "\\" );//xxxx
	if ( m_outside ) {
		m_outside->add_face( a, b, c, p, face_index, slop );
	} else {
		if ( print_debug ) printf( "y\n" );//xxxxx
		m_outside = new bsp_node( p );
		m_outside->insert_into_face_list( face_index );
	}
}


void	bsp_node::insert_into_face_list( int face_index )
// Adds the given face index into our array of face lists.
{
	m_face_count++;

	// Make storage for the new face index.
	if ( m_face_list ) {
		m_face_list = (int*) realloc( m_face_list, m_face_count * sizeof( face_index ) );
	} else {
		m_face_list = (int*) malloc( m_face_count * sizeof( face_index ) );
	}

	m_face_list[ m_face_count - 1 ] = face_index;

	// Mark this node as having geometry.
	m_partitioning_plane = false;
}


bool	bsp_node::ray_cast( collision_info* result, const vec3& p0, const vec3& dir, float distance, bool (*face_test_callback)( const vec3& normal, const vec3& pt, int face_index ) )
// Cast the specified ray through this BSP tree.  Finds the nearest
// hit, if any.  If a hit is found, the results are put in *result and
// returns true; otherwise returns false and leaves *result alone.
//
// If face_test_callback is not null, then use it to verify hits.
// This function will call face_test_callback() with candidate
// intersection points and a face_index.  The candidate point is
// guaranteed to be in the plane of the face, so the callback just
// needs to determine if the point is within the triangle, and return
// true if it is.
{
	float	dot = dir * m_plane.normal;

	float	d0 = p0 * m_plane.normal - m_plane.d;
	float	delta_n = dot * distance;
	float	d1 = d0 + delta_n;

	bool	parallel = fabs( dot ) < 0.000001f;

//	printf( "%f %f %f %f\n", d0, d1, dot, distance );	//xxxxx

	if ( parallel == false ) {
		// The ray might cross our plane.

		float	hit_distance = -d0 / dot;
//		printf( "hd = %f\n", hit_distance );//xxxxx

		if ( d0 > 0 && d1 <= 0 ) {
			//
			// Ray crosses from the outside to the inside of this half-space.
			//
//			printf( "+" );	//xxxxxx

			// Check the first half of the ray against the outside.
			if ( m_outside
				 && m_outside->ray_cast( result, p0, dir, hit_distance, face_test_callback ) )
			{
				return true;
			}

			vec3	hit_point = p0 + dir * hit_distance;

			if ( m_partitioning_plane == false ) {
				// If the crossing point is on the inside of our inside
				// tree, then we have a potential hit.
//				if ( m_inside == 0
//					 || m_inside->test_point( hit_point ) )
//				{
					// Check the faces for a hit.
					int	i;
					for ( i = 0; i < m_face_count; i++ ) {
						if ( face_test_callback == 0
							 || face_test_callback( m_plane.normal, hit_point, m_face_list[ i ] ) )
						{
							result->point = hit_point;
							result->normal = m_plane.normal;
							// get face properties from callback?
							return true;
						}
					}
//				}
			}

			// No hits so far... check the inside portion of the ray.
			return m_inside &&
				m_inside->ray_cast( result, hit_point, dir, distance - hit_distance, face_test_callback );
		}

		if ( d0 <= 0 && d1 > 0 ) {
			//
			// Ray crosses from the inside to the outside of this half-space.
			//

			// Check the first half of the ray against the inside.
			if ( m_inside
				 && m_inside->ray_cast( result, p0, dir, hit_distance, face_test_callback ) )
			{
				return true;
			}

			vec3	hit_point = p0 + dir * hit_distance;

			// If no hit, check the second half against the outside.
			return m_outside
				&& m_outside->ray_cast( result, hit_point, dir, distance - hit_distance, face_test_callback );
		}
	}

	//
	// Ray does not cross our plane.  Check which side of the plane
	// the ray is on, and only check that side.
	//

//	printf( "O\n" );	//xxxx

	if ( d0 <= 0 ) {
//		printf( "/" );//xxx
		return m_inside
			&& m_inside->ray_cast( result, p0, dir, distance, face_test_callback );
	} else {
//		printf( "\\" );//xxx
		return m_outside
			&& m_outside->ray_cast( result, p0, dir, distance, face_test_callback );
	}
}


bool	bsp_node::test_point( const vec3& a )
// Return true if the given point is inside our volume; false otherwise.
{
	float	d = m_plane.normal * a - m_plane.d;

	if ( d <= 0 ) {
		// Point is inside this node.

		if ( m_inside == 0 && m_partitioning_plane == true ) {
			// Point is in an empty partitioning volume.  Assume we're
			// outside any volume.
			//
			// @@ This is not strictly correct, because we could be
			// inside a volume defined by faces which entirely enclose
			// the partitioning volume, without intersecting it.  Such
			// volumes will have hollow cells inside them defined by
			// the partitioning planes.  However, this shouldn't be a
			// practical problem for ray tracing, because valid test
			// points will be on a real volume plane.
			//
			// There is one potential failure case: if a ray hits a
			// big volume very near a point where the corner or edge
			// of an internal partitioning volume touches the big
			// volume, it's conceivable that the raycast will miss due
			// to the test_point() call returning false for that
			// point.  Hm.  One solution would be to mark leaf
			// partitioning planes that are inside such volumes with
			// an "inside" flag; in which case we'd return "true"
			// here, and everything would be hunky-dory.
			return false;
		}

		return m_inside == 0
			|| m_inside->test_point( a );
	} else {
		// Point is on the outside of this node...
		return m_outside
			&& m_outside->test_point( a );
	}
}
