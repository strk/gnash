// kd_tree_packed.cpp	-- by Thatcher Ulrich <tu@tulrich.com>

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// packed kd-tree structure for accelerated collision queries
// vs. static triangle soup


#include "kd_tree_packed.h"

#include "tu_file.h"
#include "tu_types.h"
#include "utility.h"
#include "axial_box.h"
#include "kd_tree_dynamic.h"

using namespace std;

#if 0


struct ray_test_result;
struct ray_test_query;


class kd_tree_packed::node_chunk
{
public:
	void	ray_test(
		ray_test_result* result,
		const ray_test_query& query,
		const axial_box& node_bound,
		int node_idx = 0) const;

	void	get_child(const node_chunk** child, int* child_idx, int idx, int which_child) const;

private:
	struct leaf;

	// Kudos to Christer Ericson for the sweet packed node union.
	union node
	{
		float	m_plane;	// axis is packed into low 2 bits.  01==x, 02==y, 03==z
		int	m_leaf_offset;	// if low 2 bits == 00 then this is an offset into the leaf array

		// @@ can unions have member functions?  MSVC says yes...

		bool	is_leaf() const { return (m_leaf_offset & 3) == 00; }
		const leaf*	get_leaf() const { return (const leaf*) (((const char*) this) + m_leaf_offset); }

		int	get_axis() const
		{
			assert(is_leaf() == false);
			return (m_leaf_offset & 3) - 1;
		}

		float	get_plane_offset() const
		{
			// @@ should we mask off bottom bits?  Or leave them alone?
			return m_plane;
		}
	};

	node	m_nodes[13];		// interior nodes
	int	m_first_child_offset;	// offset of first child chunk, relative to the head of this struct
	int	m_child_offset_bits;	// 1 bit for each existing breadth-first child
};


void	kd_tree_packed::node_chunk::ray_test(
	ray_test_result* result,
	const ray_test_query& query,
	const axial_box& node_bound,
	int node_idx /* = 0 */) const
// Ray test against the node_idx'th node in this chunk.
{
	assert(node_idx >= 0 && node_idx < 13);

	const node&	n = m_nodes[node_idx];
	if (n.is_leaf())
	{
//x		n.get_leaf()->ray_test(result, query);
	}
	else
	{
		int	axis = n.get_axis();
		float	plane_offset = n.get_plane_offset();

		const node_chunk*	chunk;
		int	child_idx;

		// crossing node always gets queried.
		get_child(&chunk, &child_idx, node_idx, 0);
		chunk->ray_test(result, query, node_bound, child_idx);

		// cut the ray at plane_offset

		axial_box	child_box(node_bound);

		// if query reaches front child
		child_box.set_axis_min(axis, plane_offset);
		get_child(&chunk, &child_idx, node_idx, 1);
		chunk->ray_test(result, query, child_box, child_idx);
		
		// if query reaches back child
		child_box = node_bound;
		child_box.set_axis_max(axis, plane_offset);
		get_child(&chunk, &child_idx, node_idx, 2);
		chunk->ray_test(result, query, child_box, child_idx);
	}
}



#endif // 0



//
// straightforward impl
//


struct kd_face
{
	uint16_t	m_vi[3];	// vert indices

//	void	local_assert() { compiler_assert(sizeof(struct kd_face) == 6); }
};


struct kd_leaf
{
	uint8_t	m_flags;	// low two bits == 0b11
	uint8_t	m_face_count;

	kd_face*	get_face(int index)
	{
		assert(index >= 0 && index < m_face_count);

		return (kd_face*) (((uint8_t*) this) + sizeof(struct kd_leaf) + sizeof(kd_face) * index);
	}

//	void	local_assert() { compiler_assert(sizeof(struct kd_leaf) == 2); }
};


// TODO I believe it may be better for cache usage to store the leaf
// array separately from the kd tree nodes.  The reasoning is, when
// traversing, the two child nodes of a parent can be adjacent in
// memory, so if the first child is rejected, the second child is
// (likely) already in cache.
//
// The node layout changes: siblings are always stored together, so
// the child offset points to both children and the neg child is not
// immediately after the parent.  IIRC this is what both Opcode and
// OpenRT do, and they both have clues.
//
// (Also, obviously, need to try aligning this class.)

class kd_node
{
public:
	uint8_t	m_flags[4];
	// low two bits == axis, if axis == 3 then this is leaf.
	// 0x04 is set if the neg child is present.
	// 0x08 is set if the pos child is present
	// m_flags[1..3] is the offset to the pos child, little-endian.

	float	m_neg_offset;
	float	m_pos_offset;

	kd_leaf*	get_leaf() { assert(is_leaf()); return (kd_leaf*) this; }
	bool	is_leaf() const { return (m_flags[0] & 3) == 3; }
	int	get_axis() const { return (m_flags[0] & 3); }

	bool	has_neg_child() const { return (m_flags[0] & 4) ? true : false; }
	bool	has_pos_child() const { return (m_flags[0] & 8) ? true : false; }

	kd_node*	get_neg_child()
	{
		if (has_neg_child())
		{
			// Neg child follows immediately.
			return (kd_node*) (((uint8_t*) this) + sizeof(kd_node));
		}
		else return NULL;
	}

	kd_node*	get_pos_child()
	{
		if (has_pos_child())
		{
			unsigned int offset = (m_flags[1]) + (m_flags[2] << 8) + (m_flags[3] << 16);
			assert(offset >= sizeof(kd_node));	// sanity check

			return (kd_node*) (((uint8_t*) this) + offset);
		}
		else return NULL;
	}


	void	local_assert() { compiler_assert(sizeof(kd_node) == 12); }
};


kd_tree_packed::kd_tree_packed()
	:
	m_vert_count(0),
	m_verts(0),
	m_packed_tree_size(0),
	m_packed_tree(0)
{
}


kd_tree_packed::~kd_tree_packed()
{
	if (m_verts)
	{
		tu_free(m_verts, sizeof(vec3) * m_vert_count);
	}

	if (m_packed_tree)
	{
		tu_free(m_packed_tree, m_packed_tree_size);
	}
}


static void	write_packed_data(tu_file* out, const kd_tree_dynamic::node* source)
// Write kd tree data in the form of packed node & leaf structs.
{
	if (source->m_leaf)
	{
		// leaf case.
		assert(source->m_neg == NULL);
		assert(source->m_pos == NULL);

		kd_tree_dynamic::leaf*	sl = source->m_leaf;
		assert(sl);

		kd_leaf	l;
		l.m_flags = 3;	// mark this as a leaf
		if (sl->m_faces.size() > 255)
		{
			// problem.
			abort();
			l.m_face_count = 255;
		}
		else
		{
			l.m_face_count = sl->m_faces.size();
		}
		out->write_bytes(&l, sizeof(l));

		// write faces.
		for (int i = 0; i < l.m_face_count; i++)
		{
			kd_face	f;
			f.m_vi[0] = sl->m_faces[i].m_vi[0];
			f.m_vi[1] = sl->m_faces[i].m_vi[1];
			f.m_vi[2] = sl->m_faces[i].m_vi[2];
			out->write_bytes(&f, sizeof(f));
		}
	}
	else
	{
		// node case.

		kd_node	n;

		n.m_neg_offset = source->m_neg_offset;
		n.m_pos_offset = source->m_pos_offset;

		n.m_flags[0] = source->m_axis;
		n.m_flags[1] = 0;
		n.m_flags[2] = 0;
		n.m_flags[3] = 0;
		if (source->m_neg)
		{
			n.m_flags[0] |= 4;	// neg child present.
		}
		if (source->m_pos)
		{
			n.m_flags[0] |= 8;	// pos child present.
		}

		// Remember where our flags are, so we can come back
		// and overwrite the pos child offset.
		int	flags_start = out->get_position();

		// Write node data.
		out->write_bytes(&n, sizeof(n));
		
		// Neg child data.
		if (source->m_neg)
		{
			write_packed_data(out, source->m_neg);
		}

		// Pos child data.
		if (source->m_pos)
		{
			int	pos_child_start = out->get_position();
			int	pos_child_delta = pos_child_start - flags_start;
			if (pos_child_delta >= (1 << 24))
			{
				// Problem.
				abort();
			}
			else
			{
				n.m_flags[1] = (pos_child_delta      ) & 0x0FF;
				n.m_flags[2] = (pos_child_delta >> 8 ) & 0x0FF;
				n.m_flags[3] = (pos_child_delta >> 16) & 0x0FF;
				out->set_position(flags_start);
				out->write_bytes(&n.m_flags[0], 4);
				out->set_position(pos_child_start);

				// Write pos child.
				write_packed_data(out, source->m_pos);
			}
		}
	}
}


/*static*/ kd_tree_packed*	kd_tree_packed::build(const kd_tree_dynamic* source)
{
	tu_file	buf(tu_file::memory_buffer);

	assert(source->get_root());

	write_packed_data(&buf, source->get_root());

	kd_tree_packed*	kd = new kd_tree_packed;

	kd->m_bound = source->get_bound();

	// Copy vert data.
	kd->m_vert_count = source->get_verts().size();
	assert(kd->m_vert_count < 65536);	// we use 16-bit indices for verts
	kd->m_verts = (vec3*) tu_malloc(kd->m_vert_count * sizeof(vec3));
	memcpy(kd->m_verts, &source->get_verts()[0], sizeof(vec3) * kd->m_vert_count);

	// Copy packed tree data.
	kd->m_packed_tree_size = buf.get_position();
	kd->m_packed_tree = (kd_node*) tu_malloc(kd->m_packed_tree_size);
	buf.set_position(0);
	buf.read_bytes(kd->m_packed_tree, kd->m_packed_tree_size);

	return kd;
}


class kd_ray_query_info
{
public:
	kd_ray_query_info(const ray_query& query, const vec3* verts, int vert_count)
		:
		m_query(query),
		m_vert_count(vert_count),
		m_verts(verts)
	{
	}

//data:
	ray_query	m_query;
	int	m_vert_count;
	const vec3*	m_verts;
	
	// other helpful precomputed data?
};


// cbloom code
static inline float triple_product(const vec3 &a, const vec3 &b, const vec3 &c)
// a * (b cross c)
{
	// return Epsilon_ijk A_i B_j C_k
	const float t =
		a.x * (b.y * c.z - b.z * c.y) +
		a.y * (b.z * c.x - b.x * c.z) +
		a.z * (b.x * c.y - b.y * c.x);
	return t;
}


// For crack prevention.  This expands the triangle bounds slightly;
// smaller triangles get more expansion.  Needs to be sized big enough
// so that we don't get cracks on edges between large triangles, but
// small enough so that small triangles don't get expanded too much.
static double	INTERSECT_EPSILON = 1e-4;


// Does not do any checking of t value (time ray crosses plane)
// Haines-Moller with cbloom tweaks
static bool intersect_triangle(
	const vec3& orig, const vec3& dir,
	const vec3& vert0, const vec3& /* vert1 */, const vec3& /* vert2 */,
	const vec3& edge1, const vec3& edge2)
{
	/* begin calculating determinant - also used to calculate U parameter */
	vec3 pvec;
	pvec.set_cross(dir, edge2);

	/* if determinant is near zero, ray lies in plane of triangle */
	const float det = fabsf( edge1 * pvec ); // = vec3U::TripleProduct(dir,edge1,edge2);

	/* calculate distance from vert0 to ray origin */
	const vec3 tvec = orig - vert0;

	/* calculate U parameter and test bounds */
	const float u = tvec * pvec; // = vec3U::TripleProduct(dir,edge2,tvec);
	if ( u < -INTERSECT_EPSILON || u > det + INTERSECT_EPSILON)
		return false;

	/* prepare to test V parameter */

	/* calculate V parameter and test bounds */
	const float v = triple_product(dir,tvec,edge1);
	if ( v < -INTERSECT_EPSILON || u + v > det + INTERSECT_EPSILON)
		return false;

	return true;
}


// Statistics.
int	kd_tree_packed::s_ray_test_face_count = 0;
int	kd_tree_packed::s_ray_test_leaf_count = 0;
int	kd_tree_packed::s_ray_test_node_count = 0;


static bool	ray_test_face(const kd_ray_query_info& qi, kd_face* face)
{
	kd_tree_packed::s_ray_test_face_count++;	// stats

	assert(face->m_vi[0] < qi.m_vert_count);
	assert(face->m_vi[1] < qi.m_vert_count);
	assert(face->m_vi[2] < qi.m_vert_count);

	const vec3&	v0 = qi.m_verts[face->m_vi[0]];
	const vec3&	v1 = qi.m_verts[face->m_vi[1]];
	const vec3&	v2 = qi.m_verts[face->m_vi[2]];

	vec3	edge1(v1); edge1 -= v0;
	vec3	edge2(v2); edge2 -= v0;
	vec3	unscaled_normal;
	unscaled_normal.set_cross(edge1, edge2);

	float	plane_d = v0 * unscaled_normal;

	if (qi.m_query.m_start * unscaled_normal < plane_d)
	{
		// ray can't hit face; it starts behind the face.
		return false;
	}

	if (qi.m_query.m_end * unscaled_normal > 0)
	{
		// ray doesn't reach the face.
		return false;
	}

	// Ray crosses plane of triangle; see if it crosses inside the triangle bounds.
	return intersect_triangle(qi.m_query.m_start, qi.m_query.m_dir, v0, v1, v2, edge1, edge2);
}


static bool	ray_test_leaf(const kd_ray_query_info& qi, kd_leaf* leaf)
// Return true if ray subset hits any of our faces.
{
	kd_tree_packed::s_ray_test_leaf_count++;	// stats

	for (int i = 0, n = leaf->m_face_count; i < n; i++)
	{
		kd_face*	face = leaf->get_face(i);
		if (ray_test_face(qi, face))
		{
			return true;
		}
	}

	return false;
}


static bool	ray_test_node(const kd_ray_query_info& qi, float t_min, float t_max, kd_node* node)
// Return true if the ray (limited to the subset [t_min,t_max], where
// t is in [0,1]) hits any of our faces.
{
	assert(node);

	if (node->is_leaf())
	{
		// Test the faces.
		return ray_test_leaf(qi, node->get_leaf());
	}

	// Node check.

	kd_tree_packed::s_ray_test_node_count++;	// stats

	int	axis = node->get_axis();
		
	if (qi.m_query.m_inv_dir[axis] == 0)
	{
		// Query is effectively parallel to splitting plane.

		// Does query possibly touch the neg child?
		kd_node*	neg_child = node->get_neg_child();
		if (neg_child
		    && qi.m_query.m_start[axis] <= node->m_neg_offset)
		{
			// Check neg child.
			if (ray_test_node(qi, t_min, t_max, neg_child))
			{
				return true;
			}
		}

		// Does query possibly touch the pos child?
		kd_node*	pos_child = node->get_pos_child();
		if (pos_child
		    && qi.m_query.m_start[axis] >= node->m_pos_offset)
		{
			// Check pos child.
			if (ray_test_node(qi, t_min, t_max, pos_child))
			{
				return true;
			}
		}

		return false;
	}

	// Ordinary node check.

	if (qi.m_query.m_dir[axis] > 0)
	{
		// Neg child is the near side.

		// Handle neg child.
		kd_node*	neg_child = node->get_neg_child();
		if (neg_child)
		{
			float	t_exit_neg = (node->m_neg_offset - qi.m_query.m_start[axis])
				* qi.m_query.m_inv_displacement[axis];

			if (t_exit_neg >= t_min)
			{
				if (ray_test_node(qi, t_min, fmin(t_exit_neg, t_max), neg_child))
				{
					return true;
				}
			}
		}

		// Handle pos child.
		kd_node*	pos_child = node->get_pos_child();
		if (pos_child)
		{
			float	t_enter_pos = (node->m_pos_offset - qi.m_query.m_start[axis])
				* qi.m_query.m_inv_displacement[axis];

			if (t_enter_pos <= t_max)
			{
				if (ray_test_node(qi, fmax(t_enter_pos, t_min), t_max, pos_child))
				{
					return true;
				}
			}
		}
	}
	else
	{
		// Pos child is the near side.

		// Handle neg child.
		kd_node*	neg_child = node->get_neg_child();
		if (neg_child)
		{
			float	t_enter_neg = (node->m_neg_offset - qi.m_query.m_start[axis])
				* qi.m_query.m_inv_displacement[axis];

			if (t_enter_neg <= t_max)
			{
				if (ray_test_node(qi, fmax(t_enter_neg, t_min), t_max, neg_child))
				{
					return true;
				}
			}
		}

		// Handle pos child.
		kd_node*	pos_child = node->get_pos_child();
		if (pos_child)
		{
			float	t_exit_pos = (node->m_pos_offset - qi.m_query.m_start[axis])
				* qi.m_query.m_inv_displacement[axis];

			if (t_exit_pos >= t_min)
			{
				if (ray_test_node(qi, t_min, fmin(t_exit_pos, t_max), pos_child))
				{
					return true;
				}
			}
		}
	}

	return false;
}


bool	kd_tree_packed::ray_test(const ray_query& query)
// Return true if the ray query hits any of our faces.
{
	assert(m_packed_tree);
	assert(m_verts);

	kd_ray_query_info	qi(query, m_verts, m_vert_count);

	// @@ Check (and trim?) against bounding box.

	return ray_test_node(qi, 0, 1, m_packed_tree);
}



// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
