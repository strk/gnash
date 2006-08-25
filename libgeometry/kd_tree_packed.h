// kd_tree_packed.h	-- by Thatcher Ulrich <tu@tulrich.com>

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// packed kd-tree structure for accelerated collision queries
// vs. static triangle soup

#ifndef KD_TREE_PACKED_H
#define KD_TREE_PACKED_H


#include "container.h"
#include "axial_box.h"
#include "collision.h"
#include "geometry.h"


class tu_file;
class kd_tree_dynamic;
class kd_node;


class kd_tree_packed
{
public:
	~kd_tree_packed();

	void	read(tu_file* in);
	void	write(tu_file* out);

	static kd_tree_packed*	build(const kd_tree_dynamic* source_tree);

	// Return true if the ray query hits any of our faces.
	bool	ray_test(const ray_query& query);

	// void	lss_test(....);

	const axial_box&	get_bound() const { return m_bound; }

	// statistics.
	static int	s_ray_test_face_count;
	static int	s_ray_test_leaf_count;
	static int	s_ray_test_node_count;

private:
	kd_tree_packed();

	//struct node_chunk;

	axial_box	m_bound;
	int	m_vert_count;
	vec3*	m_verts;

	int	m_packed_tree_size;
	kd_node*	m_packed_tree;
};


#endif // KD_TREE_PACKED_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
