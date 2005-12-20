// bsp.h	-- by Thatcher Ulrich <tu@tulrich.com> 2001

// This code is in the public domain.

// Code to make a collision-bsp tree out of triangle soup.


#ifndef BSP_H
#define BSP_H


#include "geometry.h"


class bsp_node {
// Class for representing polygonal volumes using a bsp tree.  Can
// build a tree using the constructor and bsp_node::add_face().  Can
// test points and rays against the resulting volume.
public:
	bsp_node( const plane_info& p );
	~bsp_node();

	void	add_face( const vec3& a, const vec3& b, const vec3& c, const plane_info& p, int face_index, float plane_slop = 0.001f );
	void	add_partition( const plane_info& p );

	bool	ray_cast( collision_info* result, const vec3& p0, const vec3& dir, float distance, bool (*face_test_callback)( const vec3& normal, const vec3& pt, int face_index ) );
	bool	test_point( const vec3& a );

	// need some utility functions for setting the child nodes?

private:
	void	add_inside_face( const vec3& a, const vec3& b, const vec3& c, const plane_info& p, int face_index, float slop );
	void	add_outside_face( const vec3& a, const vec3& b, const vec3& c, const plane_info& p, int face_index, float slop );
	void	insert_into_face_list( int face_index );

	plane_info	m_plane;
	bsp_node*	m_inside;
	bsp_node*	m_outside;
	bool	m_partitioning_plane;	// true if this plane is strictly for partitioning; i.e. has no faces associated with it.

	int*	m_face_list;
	int	m_face_count;
};


#endif // BSP_H
