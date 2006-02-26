// kd_tree_dynamic.h	-- by Thatcher Ulrich <tu@tulrich.com>

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Data structure for building a kd-tree from a triangle mesh.


#ifndef KD_TREE_DYNAMIC_H
#define KD_TREE_DYNAMIC_H


#include "container.h"
#include "geometry.h"
#include "axial_box.h"


class tu_file;
struct kd_diagram_dump_info;


struct kd_tree_dynamic
{
	// Build tree(s) from the given mesh.
	static void	build_trees(
		std::vector<kd_tree_dynamic*>* treelist,
		int vert_count,
		const vec3 verts[],
		int triangle_count,
		const int indices[]
		);

	// vert count must be under 64K
	kd_tree_dynamic(
		int vert_count,
		const vec3 verts[],
		int triangle_count,
		const int indices[]
		);
	~kd_tree_dynamic();

	struct face
	{
		Uint16	m_vi[3];	// indices of verts
		Uint16	m_flags;

		float	get_min_coord(int axis, const std::vector<vec3>& verts) const;
		float	get_max_coord(int axis, const std::vector<vec3>& verts) const;
	};

	struct leaf
	{
		std::vector<face>	m_faces;
	};

	// Internal node.  Not too tidy; would use unions etc. if it were
	// important.
	//
	// This is a "loose" kdtree in the sense that there are two
	// independent splitting planes on the chosen axis.  This
	// ensures that all faces can be classified onto one side or
	// the other.  Almost the same as a binary AABB tree.
	//
	//                   |   |
	//    +--------------+---+----------------+
	//    |     /   /  \ |   |/       --   \  |
	//    |    /   /    \+---/     \        \ |
	//    | |  --- \     |  /|--\   \    /    |
	//    | |       \ -- | / |   \      /  ---|
	//    +--------------+---+----------------+
	//                   |   |
	//  axis *--> +     neg pos
	//
	// So the idea here is that the neg node contains all faces
	// that are strictly on the negative side of the neg_offset,
	// and the pos node has all the rest of the faces, and the
	// pos_offset is placed so that all the pos node faces are
	// strictly on the positive side of pos_offset.
	//
	// Note that the pos and neg nodes could overlap, or could be
	// disjoint.
	struct node
	{
		node*	m_neg;
		node*	m_pos;
		leaf*	m_leaf;
		int	m_axis;	// split axis: 0 = x, 1 = y, 2 = z
		float	m_neg_offset;	// where the back split occurs
		float	m_pos_offset;	// where the front split occurs

		node();
		~node();
		bool	is_valid() const;
		void	dump(tu_file* out, int depth) const;
	};

	const std::vector<vec3>&	get_verts() const { return m_verts; }
	const node*	get_root() const { return m_root; }
	const axial_box&	get_bound() const { return m_bound; }

	// For debugging/evaluating.
	void	dump(tu_file* out) const;
	void	diagram_dump(tu_file* out) const;	// make a Postscript diagram.
	void	mesh_diagram_dump(tu_file* out, int axis) const;	// make a Postscript diagram of the mesh data.

private:
	static void	compute_actual_bounds(axial_box* result, int vert_count, const vec3 verts[]);

	void	compute_actual_bounds(axial_box* result, int face_count, face faces[]);
	node*	build_tree(int depth, int face_count, face faces[], const axial_box& bounds);

	void	do_split(
		int* neg_end,
		int* pos_end,
		int face_count,
		face faces[],
		int axis,
		float neg_offset,
		float pos_offset);

	float	evaluate_split(
		int depth,
		int face_count,
		face faces[],
		const axial_box& bounds,
		int axis,
		float neg_offset,
		float* pos_offset);

	int	classify_face(const face& f, int axis, float offset);

	// Utility, for  testing a clipping  non-loose kdtree.  Duping
	// is probably much preferable to clipping though.
	void	clip_faces(std::vector<face>* faces, int axis, float offset);

	std::vector<vec3>	m_verts;
	node*	m_root;
	axial_box	m_bound;
};


#endif // KD_TREE_DYNAMIC_H


// Local Variables:
// mode: C++
// c-basic-offset: 8
// tab-width: 8
// indent-tabs-mode: t
// End:
