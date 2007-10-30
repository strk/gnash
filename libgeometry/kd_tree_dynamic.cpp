// kd_tree_dynamic.cpp	-- by Thatcher Ulrich <tu@tulrich.com>

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Utility kd-tree structure, for building kd-trees from triangle
// soup.

// #include <cwctype>
// #include <cwchar>
#include <cstdio>

#include "kd_tree_dynamic.h"
#include "tu_file.h"
#include <cfloat>
#include "hash_wrapper.h"

using namespace std;
using namespace gnash;

static const float	EPSILON = 1e-4f;
static const int	LEAF_FACE_COUNT = 6;
static const int	MAX_SPLIT_PLANES_TESTED = 10;

//#define CARVE_OFF_SPACE
//#define ADHOC_METRIC
#define MACDONALD_AND_BOOTH_METRIC
//#define SORT_VERTICES

// A higher value for MAX_SPLIT_PLANES_TESTED gives faster trees;
// e.g. on one dataset, MSPT=100 gives 10% faster queries than
// MSPT=10.  But the tree building is much slower.

// On one dataset I checked, SORT_VERTICES makes queries ~10% faster.
// On most others, it seemed to make no difference.  It takes extra
// time to do the sort, though.


float	kd_tree_dynamic::face::get_min_coord(int axis, const std::vector<vec3>& verts) const
{
	float	minval = verts[m_vi[0]][axis];
	minval = fmin(minval, verts[m_vi[1]][axis]);
	minval = fmin(minval, verts[m_vi[2]][axis]);
	return minval;
}


float	kd_tree_dynamic::face::get_max_coord(int axis, const std::vector<vec3>& verts) const
{
	float	maxval = verts[m_vi[0]][axis];
	maxval = fmax(maxval, verts[m_vi[1]][axis]);
	maxval = fmax(maxval, verts[m_vi[2]][axis]);
	return maxval;
}


void split_mesh(
	std::vector<vec3>* verts0,
	std::vector<int>* tris0,
	std::vector<vec3>* verts1,
	std::vector<int>* tris1,
	int /* vert_count */,
	const vec3 verts[],
	int triangle_count,
	const int indices[],
	int axis,
	float offset)
// Divide a mesh into two pieces, roughly along the plane [axis]=offset.
// Assign faces to one side or the other based on centroid.
{
	assert(verts0 && tris0 && verts1 && tris1);
	assert(verts0->size() == 0);
	assert(tris0->size() == 0);
	assert(verts1->size() == 0);
	assert(tris1->size() == 0);

	// Remap table from verts array to new verts0/1 arrays.
	hash_wrapper<int, int>	verts_to_verts0;
	hash_wrapper<int, int>	verts_to_verts1;

	// Divide the faces.
	for (int i = 0; i < triangle_count; i++)
	{
		int	index = i * 3;
		int	v[3] = {
			indices[index],
			indices[index + 1],
			indices[index + 2]
		};

		float centroid = (verts[v[0]][axis] + verts[v[1]][axis] + verts[v[2]][axis]) / 3.0f;

		if (centroid < offset)
		{
			// Put this face into verts0/tris0
			for (int ax = 0; ax < 3; ax++)
			{
				int	new_index;
				if (verts_to_verts0.get(v[ax], &new_index))
				{
					// OK.
				}
				else
				{
					// Must add.
					new_index = verts0->size();
					verts_to_verts0.add(v[ax], new_index);
					verts0->push_back(verts[v[ax]]);
				}
				tris0->push_back(new_index);
			}
		}
		else
		{
			// Put this face into verts1/tris1
			for (int ax = 0; ax < 3; ax++)
			{
				int	new_index;
				if (verts_to_verts1.get(v[ax], &new_index))
				{
					// OK.
				}
				else
				{
					// Must add.
					new_index = verts1->size();
					verts_to_verts1.add(v[ax], new_index);
					verts1->push_back(verts[v[ax]]);
				}
				tris1->push_back(new_index);
			}
		}
	}
}


static void	remap_vertex_order(kd_tree_dynamic::node* node, hash_wrapper<int, int>* map_indices_old_to_new, int* new_vertex_count)
// Traverse this tree in depth-first order, and remap the vertex
// indices to go in order.
{
	if (node == NULL) return;

	if (node->m_leaf)
	{
		for (int i = 0, n = node->m_leaf->m_faces.size(); i < n; i++)
		{
			kd_tree_dynamic::face*	f = &node->m_leaf->m_faces[i];
			for (int vi = 0; vi < 3; vi++)
			{
				int	old_index = f->m_vi[vi];
				int	new_index = *new_vertex_count;
				if (map_indices_old_to_new->get(old_index, &new_index))
				{
					// vert is already remapped; use existing mapping.
				}
				else
				{
					// vert is not remapped yet; remap it.
					map_indices_old_to_new->add(old_index, new_index);
					(*new_vertex_count)++;
				}

				// Remap.
				f->m_vi[vi] = new_index;
			}
		}
	}
	else
	{
		remap_vertex_order(node->m_neg, map_indices_old_to_new, new_vertex_count);
		remap_vertex_order(node->m_pos, map_indices_old_to_new, new_vertex_count);
	}
}


/*static*/ void	kd_tree_dynamic::build_trees(
	std::vector<kd_tree_dynamic*>* treelist,
	int vert_count,
	const vec3 verts[],
	int triangle_count,
	const int indices[]
	)
// Build one or more kd trees to represent the given mesh.
{
	if (vert_count >= 65536)
	{
		// Too many verts for one tree; subdivide.
		axial_box	bound;
		compute_actual_bounds(&bound, vert_count, verts);

		int	longest_axis = bound.get_longest_axis();
		float	offset = bound.get_center()[longest_axis];

		std::vector<vec3>	verts0, verts1;
		std::vector<int>	tris0, tris1;
		split_mesh(
			&verts0,
			&tris0,
			&verts1,
			&tris1,
			vert_count,
			verts,
			triangle_count,
			indices,
			longest_axis,
			offset);

		if ((int) verts0.size() >= vert_count || (int) verts1.size() >= vert_count)
		{
			// Trouble: couldn't reduce vert count by
			// splitting.
			abort();
			// log error
			return;
		}

		build_trees(treelist, verts0.size(), &verts0[0], tris0.size() / 3, &tris0[0]);
		build_trees(treelist, verts1.size(), &verts1[0], tris1.size() / 3, &tris1[0]);

		return;
	}

	treelist->push_back(new kd_tree_dynamic(vert_count, verts, triangle_count, indices));
}


kd_tree_dynamic::kd_tree_dynamic(
	int vert_count,
	const vec3 verts[],
	int triangle_count,
	const int indices[])
// Constructor; build the kd-tree from the given triangle soup.
{
	assert(vert_count > 0 && vert_count < 65536);
	assert(triangle_count > 0);

	// Copy the verts.
	m_verts.resize(vert_count);
	memcpy(&m_verts[0], verts, sizeof(verts[0]) * vert_count);

	// Make a mutable array of faces, and also compute our bounds.
	axial_box	bounds(axial_box::INVALID, vec3::flt_max, vec3::minus_flt_max);
	std::vector<face>	faces;
	for (int i = 0; i < triangle_count; i++)
	{
		face	f;
		f.m_vi[0] = indices[i * 3 + 0];
		f.m_vi[1] = indices[i * 3 + 1];
		f.m_vi[2] = indices[i * 3 + 2];
		f.m_flags = 0;	// @@ should be a way to initialize this

		faces.push_back(f);

		// Update bounds.
		bounds.set_enclosing(m_verts[f.m_vi[0]]);
		bounds.set_enclosing(m_verts[f.m_vi[1]]);
		bounds.set_enclosing(m_verts[f.m_vi[2]]);
	}

	m_bound = bounds;

	m_root = build_tree(1, faces.size(), &faces[0], bounds);

#ifdef SORT_VERTICES
	// Sort vertices in the order they first appear in a
	// depth-first traversal of the tree.  Idea is to exploit
	// cache coherency when traversing tree.

	index	map_indices_old_to_new;
	int	new_vertex_count = 0;
	remap_vertex_order(m_root, &map_indices_old_to_new, &new_vertex_count);

	assert(new_vertex_count == m_verts.size());

	// Make the re-ordered vertex buffer.
	std::vector<vec3>	new_verts;
	new_verts.resize(new_vertex_count);
	for (int i = 0; i < m_verts.size(); i++)
	{
		int	new_index = 0;
		bool	found = map_indices_old_to_new.get(i, &new_index);
		assert(found);
		if (found)
		{
			new_verts[new_index] = m_verts[i];
		}
	}

	// Use the new verts.
	m_verts = new_verts;
#endif // SORT_VERTICES
}


kd_tree_dynamic::~kd_tree_dynamic()
// Destructor; make sure to delete the stuff we allocated.
{
	delete m_root;
}


kd_tree_dynamic::node*	kd_tree_dynamic::build_tree(int depth, int face_count, face faces[], const axial_box& bounds)
// Recursively build a kd-tree from the given set of faces.  Return
// the root of the tree.
{
	assert(face_count >= 0);

	if (face_count == 0)
	{
		return NULL;
	}

	// Should we make a leaf?
	if (face_count <= LEAF_FACE_COUNT)
	{
		// Make a leaf
		node*	n = new node;
		n->m_leaf = new leaf;
		n->m_leaf->m_faces.resize(face_count);
		memcpy(&(n->m_leaf->m_faces[0]), faces, sizeof(faces[0]) * face_count);

		return n;
	}

	// TODO I believe it may be better to try partitioning planes,
	// which separate the faces according to triangle centroid (or
	// centroid of the bound?), and then compute the actual
	// splitting planes based on the partition.  I think this
	// helps avoid some bad situations with large triangles, where
	// a large tri keeps getting pushed down deeper and deeper.
	//
	// Currently we generate a candidate neg_offset plane
	// directly, and include all triangles fully behind the
	// neg_offset in one child, which may tend to be unbalanced.

	// Find a good splitting plane.
	float	best_split_quality = 0.0f;
	int	best_split_axis = -1;
	float	best_split_neg_offset = 0.0f;
	float	best_split_pos_offset = 0.0f;

	for (int axis = 0; axis < 3; axis++)
	{
		if (bounds.get_extent()[axis] < EPSILON)
		{
			// Don't try to divide 
			continue;
		}

		// Try offsets that correspond to existing face boundaries.
		int	step_size = 1;
		if (face_count > MAX_SPLIT_PLANES_TESTED)
		{
			// For the sake of speed & sanity, only try the bounds
			// of every N faces.
			step_size = face_count / MAX_SPLIT_PLANES_TESTED;
		}
		assert(step_size > 0);

		float	last_offset_tried = -FLT_MAX;
		float	pos_offset = 0;
		for (int i = 0; i < face_count; i += step_size)
		{
			float	neg_offset = faces[i].get_max_coord(axis, m_verts);

			if (fabsf(neg_offset - last_offset_tried) < EPSILON)
			{
				// Already tried this.
				continue;
			}
			
			last_offset_tried = neg_offset;

			// How good is this split?
			float	quality = evaluate_split(depth, face_count, faces, bounds, axis, neg_offset, &pos_offset);
			if (quality > best_split_quality)
			{
				// Best so far.
				best_split_quality = quality;
				best_split_axis = axis;
				best_split_neg_offset = neg_offset;
				best_split_pos_offset = pos_offset;
			}
		}
	}

	if (best_split_axis == -1)
	{
		// Couldn't find any acceptable split!
		// Make a leaf.
		node*	n = new node;
		n->m_leaf = new leaf;
		n->m_leaf->m_faces.resize(face_count);
		memcpy(&(n->m_leaf->m_faces[0]), faces, sizeof(faces[0]) * face_count);

		return n;
	}
	else
	{
		// Make the split.
		int	back_end = 0;
		int	front_end = 0;

		// We use the implicit node bounds, not the actual bounds of
		// the face sets, for computing split quality etc, since that
		// is what the run-time structures have when they are
		// computing query results.

		axial_box	back_bounds(bounds);
		back_bounds.set_axis_max(best_split_axis, best_split_neg_offset);

		axial_box	front_bounds(bounds);
		front_bounds.set_axis_min(best_split_axis, best_split_pos_offset);

		node*	n = new node;
		n->m_axis = best_split_axis;
		n->m_neg_offset = best_split_neg_offset;
		n->m_pos_offset = best_split_pos_offset;

		// Recursively build sub-trees.
		do_split(&back_end, &front_end, face_count, faces, best_split_axis, best_split_neg_offset, best_split_pos_offset);

		n->m_neg = build_tree(depth + 1, back_end, faces + 0, back_bounds);
		n->m_pos = build_tree(depth + 1, front_end - back_end, faces + back_end, front_bounds);

		return n;
	}
}


kd_tree_dynamic::node::node()
// Default constructor, null everything out.
	:
	m_neg(0),
	m_pos(0),
	m_leaf(0),
	m_axis(0),
	m_neg_offset(0.0f),
	m_pos_offset(0.0f)
{
}


kd_tree_dynamic::node::~node()
// Destructor, delete children if any.
{
	delete m_neg;
	delete m_pos;
	delete m_leaf;
}


bool	kd_tree_dynamic::node::is_valid() const
{
	return
		// internal node.
		(m_leaf == 0
		 && m_axis >= 0
		 && m_axis < 3)
		||
		// leaf node
		(m_leaf != 0
		 && m_neg == 0
		 && m_pos == 0)
		;
}


void	kd_tree_dynamic::compute_actual_bounds(axial_box* result, int face_count, face faces[])
// Compute the actual bounding box around the given list of faces.
{
	assert(face_count > 0);

	result->set_min_max_invalid(vec3::flt_max, vec3::minus_flt_max);
	
	for (int i = 0; i < face_count; i++)
	{
		const face&	f = faces[i];

		// Update bounds.
		result->set_enclosing(m_verts[f.m_vi[0]]);
		result->set_enclosing(m_verts[f.m_vi[1]]);
		result->set_enclosing(m_verts[f.m_vi[2]]);
	}
}


/*static*/ void	kd_tree_dynamic::compute_actual_bounds(axial_box* result, int vert_count, const vec3 verts[])
// Compute the actual bounding box around the given list of faces.
{
	assert(vert_count > 0);

	result->set_min_max_invalid(vec3::flt_max, vec3::minus_flt_max);
	
	for (int i = 0; i < vert_count; i++)
	{
		// Update bounds.
		result->set_enclosing(verts[i]);
	}
}


static int	classify_coord(float coord, float offset)
{
	if (coord < offset /* - EPSILON */)
	{
		return -1;
	}
	else if (coord > offset /* + EPSILON */)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


void	kd_tree_dynamic::do_split(
	int* back_end,
	int* front_end,
	int face_count,
	face faces[],
	int axis,
	float neg_offset,
	float pos_offset)
// Classify the given faces as either negative or positive.  The faces
// within faces[] are shuffled.  On exit, the faces[] array has the
// following segments:
//
// [0, *neg_end-1]              -- the faces behind the plane axis=neg_offset
// [neg_end, face_count-1]      -- the faces in front of axis=pos_offset (i.e. everything else)
//
// pos_offset must be placed so that it catches everything not on the
// negative side; this routine asserts against that.
{
	// We do an in-place sort.  During sorting, faces[] is divided
	// into three segments: at the beginning are the front faces,
	// in the middle are the unsorted faces, and at the end are
	// the back faces.  when we sort a face, we swap it into the
	// next position for either the back or front face segment.
	int	back_faces_end = 0;
	int	front_faces_start = face_count;
	//int	next_face = 0;
	
	while (back_faces_end < front_faces_start)
	{
		const face&	f = faces[back_faces_end];

		int	result = classify_face(f, axis, neg_offset);
		if (result == -1)
		{
			// Behind.  Leave this face where it is, and
			// bump back_faces_end so it's now in the back
			// faces segment.
			back_faces_end++;
		}
		else
		{
			// In front.
			assert(f.get_min_coord(axis, m_verts) >= pos_offset);	// should not have any crossing faces!

			// Swap this face up to the beginning of the front faces.
			front_faces_start--;
			swap(&faces[back_faces_end], &faces[front_faces_start]);
		}
	}

	*back_end = back_faces_end;
	*front_end = face_count;
	assert(*back_end <= *front_end);
	assert(*front_end == face_count);

#if 0
	std::vector<face>	back_faces;
	std::vector<face>	front_faces;

	for (int i = 0; i < face_count; i++)
	{
		const face&	f = faces[i];

		int	result = classify_face(f, axis, neg_offset);
		if (result == -1)
		{
			// Behind.
			back_faces.push_back(f);
		}
		else
		{
			assert(f.get_min_coord(axis, m_verts) >= pos_offset);	// should not have any crossing faces!

			front_faces.push_back(f);
		}
	}

	assert(back_faces.size() + front_faces.size() == face_count);

	*back_end = back_faces.size();
	if (back_faces.size() > 0)
	{
		memcpy(&(faces[0]), &(back_faces[0]), back_faces.size() * sizeof(faces[0]));
	}

	*front_end = *back_end + front_faces.size();
	if (front_faces.size() > 0)
	{
		memcpy(&faces[*back_end], &front_faces[0], front_faces.size() * sizeof(faces[0]));
	}

	assert(*back_end <= *front_end);
	assert(*front_end == face_count);
#endif // 0
}


float	kd_tree_dynamic::evaluate_split(
	int /* depth */,
	int face_count,
	face faces[],
	const axial_box& bounds,
	int axis,
	float neg_offset,
	float* pos_offset)
// Compute the "value" of splitting the given set of faces, bounded by
// the given box, along the plane [axis]=offset.  A value of 0 means
// that a split is possible, but has no value.  A negative value means
// that the split is not valid at all.  Positive values indicate
// increasing goodness.
//
// *pos_offset is computed based on the minimum coord of the faces
// that don't fit behind the neg_offset.  Could be greater or less
// than neg_offset.
//
// This is kinda heuristicy -- it's where the "special sauce" comes
// in.
{
	// Count the faces that will end up in the groups
	// back,front.
	int	back_count = 0;
	int	front_count = 0;

	*pos_offset = bounds.get_max()[axis];

	for (int i = 0; i < face_count; i++)
	{
		const face&	f = faces[i];

		int	result = classify_face(f, axis, neg_offset);
		if (result == -1)
		{
			// Neg.
			back_count++;
		}
		else
		{
			// Pos.
			front_count++;

			// Update *pos_offset so it contains this face.
			float	mincoord = f.get_min_coord(axis, m_verts);
			if (mincoord < *pos_offset)
			{
				*pos_offset = mincoord;
				assert(mincoord >= bounds.get_min()[axis]);
			}
		}
	}

	if ((back_count == 0 && *pos_offset - EPSILON <= bounds.get_min()[axis])
	    || (front_count == 0 && neg_offset + EPSILON >= bounds.get_max()[axis]))
	{
		// No faces are separated by this split; this split is
		// entirely useless.
		return -1;
	}

	//float	center = bounds.get_center().get(axis);
	//float	extent = bounds.get_extent().get(axis);

	axial_box	back_bounds(bounds);
	back_bounds.set_axis_max(axis, neg_offset);
	axial_box	front_bounds(bounds);
	front_bounds.set_axis_min(axis, *pos_offset);

// Probably not a win.
#ifdef CARVE_OFF_SPACE
	// Special case: if the plane carves off space at one side or the
	// other, without orphaning any faces, then we reward a large
	// empty space.
	float	space_quality = 0.0f;
	if (front_count == 0)
	{
		// All the faces are in back -- reward a bigger empty front volume.
		return space_quality = back_count * front_bounds.get_surface_area();
	}
	else if (back_count == 0)
	{
		// All the faces are in front.
		return space_quality = front_count * back_bounds.get_surface_area();
	}
#endif // CARVE_OFF_SPACE


// My ad-hoc metric
#ifdef ADHOC_METRIC
	// compute a figure for how close to the center this splitting
	// plane is.  Normalize in [0,1].
	float	volume_balance = 1.0f - fabsf(center - (neg_offset + *pos_offset) / 2) / extent;

	// Compute a figure for how well we balance the faces.  0 == bad,
	// 1 == good.
	float	face_balance = 1.0f - (fabsf(float(front_count - back_count))) / face_count;

	float	split_quality = bounds.get_surface_area() * volume_balance * face_balance;

	return split_quality;
#endif // ADHOC_METRIC


#ifdef MACDONALD_AND_BOOTH_METRIC
	// MacDonald and Booth's metric, as quoted by Havran, endorsed by
	// Ville Miettinen and Atman Binstock:

	float	cost_back = back_bounds.get_surface_area() * (back_count);
	float	cost_front = front_bounds.get_surface_area() * (front_count);

	float	havran_cost = cost_back + cost_front;

	float	parent_cost = bounds.get_surface_area() * face_count;

	// We need to turn the cost into a quality, so subtract it from a
	// big number.
	return  parent_cost - havran_cost;

#endif // MACDONALD_AND_BOOTH_METRIC
}


int	kd_tree_dynamic::classify_face(const face& f, int axis, float offset)
// Return -1 if the face is entirely behind the plane [axis]=offset
// Return 0 if the face spans the plane.
// Return 1 if the face is entirely in front of the plane.
//
// "behind" means on the negative side, "in front" means on the
// positive side.
{
	assert(axis >= 0 && axis < 3);

	bool	has_front_vert = false;
	bool	has_back_vert = false;

	for (int i = 0; i < 3; i++)
	{
		float	coord = m_verts[f.m_vi[i]].get(axis);
		int	cr = classify_coord(coord, offset);

		if (cr == -1)
		{
			has_back_vert = true;
		}
		else if (cr == 1)
		{
			has_front_vert = true;
		}
	}

	if (has_front_vert && has_back_vert)
	{
		return 0;	// crossing.
	}
	else if (has_front_vert)
	{
		return 1;	// all verts in front.
	}
	else if (has_back_vert)
	{
		return -1;	// all verts in back.
	}
	else
	{
		// Face is ON the plane.
		return 0;	// call it "crossing".
	}
}


void	kd_tree_dynamic::clip_faces(std::vector<face>* faces, int axis, float offset)
// Clip the given faces against the plane [axis]=offset.  Update the
// *faces array with the newly clipped faces; add faces and verts as
// necessary.
{
	int	original_face_count = faces->size();

	for (int i = 0; i < original_face_count; i++)
	{
		face	f = (*faces)[i];

		if (classify_face(f, axis, offset) == 0)
		{
			// Crossing face, probably needs to be clipped.

			int	vr[3];
			vr[0] = classify_coord(m_verts[f.m_vi[0]].get(axis), offset);
			vr[1] = classify_coord(m_verts[f.m_vi[1]].get(axis), offset);
			vr[2] = classify_coord(m_verts[f.m_vi[2]].get(axis), offset);

			// Sort...
			if (vr[0] > vr[1])
			{
				swap(&vr[0], &vr[1]);
				swap(&f.m_vi[0], &f.m_vi[1]);
			}
			if (vr[1] > vr[2])
			{
				swap(&vr[1], &vr[2]);
				swap(&f.m_vi[1], &f.m_vi[2]);
			}
			if (vr[0] > vr[1])
			{
				swap(&vr[0], &vr[1]);
				swap(&f.m_vi[0], &f.m_vi[1]);
			}

			if (vr[0] == 0 || vr[2] == 0)
			{
				// Face doesn't actually cross; no need to clip.
				continue;
			}
			
			const vec3	v[3] = {
				m_verts[f.m_vi[0]],
				m_verts[f.m_vi[1]],
				m_verts[f.m_vi[2]]
			};

			// Different cases.
			if (vr[1] == 0)
			{
				// Middle vert is on the plane; make two triangles.

				// One new vert.
				float	lerper = (offset - v[0].get(axis)) / (v[2].get(axis) - v[0].get(axis));
				vec3	new_vert = v[0] * (1 - lerper) + v[2] * lerper;
				new_vert.set(axis, offset);	// make damn sure
				assert(new_vert.checknan() == false);

				int	new_vi = m_verts.size();
				m_verts.push_back(new_vert);

				// New faces.
				face	f0 = f;
				f0.m_vi[2] = new_vi;
				(*faces)[i] = f0;	// replace original face

				assert(classify_face(f0, axis, offset) <= 0);
				
				face	f1 = f;
				f1.m_vi[0] = new_vi;
				faces->push_back(f1);	// add a face

				assert(classify_face(f1, axis, offset) >= 0);
			}
			else if (vr[1] < 0)
			{
				// Middle vert is behind the plane.
				// Make two tris behind, one in front.

				// Two new verts.
				float	lerper0 = (offset - v[0].get(axis)) / (v[2].get(axis) - v[0].get(axis));
				vec3	new_vert0 = v[0] * (1 - lerper0) + v[2] * lerper0;
				new_vert0.set(axis, offset);	// make damn sure
				assert(new_vert0.checknan() == false);
				int	new_vi0 = m_verts.size();
				m_verts.push_back(new_vert0);

				float	lerper1 = (offset - v[1].get(axis)) / (v[2].get(axis) - v[1].get(axis));
				vec3	new_vert1 = v[1] * (1 - lerper1) + v[2] * lerper1;
				new_vert1.set(axis, offset);	// make damn sure
				assert(new_vert1.checknan() == false);
				int	new_vi1 = m_verts.size();
				m_verts.push_back(new_vert1);

				// New faces.
				face	f0 = f;
				f0.m_vi[2] = new_vi0;
				(*faces)[i] = f0;

				assert(classify_face(f0, axis, offset) <= 0);

				face	f1 = f;
				f1.m_vi[0] = new_vi0;
				f1.m_vi[2] = new_vi1;
				faces->push_back(f1);

				assert(classify_face(f1, axis, offset) <= 0);

				face	f2 = f;
				f2.m_vi[0] = new_vi0;
				f2.m_vi[1] = new_vi1;
				faces->push_back(f2);

				assert(classify_face(f2, axis, offset) >= 0);
			}
			else if (vr[1] > 0)
			{
				// Middle vert is in front of the plane.
				// Make on tri behind, two in front.

				// Two new verts.
				float	lerper1 = (offset - v[0].get(axis)) / (v[1].get(axis) - v[0].get(axis));
				vec3	new_vert1 = v[0] * (1 - lerper1) + v[1] * lerper1;
				new_vert1.set(axis, offset);	// make damn sure
				assert(new_vert1.checknan() == false);
				int	new_vi1 = m_verts.size();
				m_verts.push_back(new_vert1);

				float	lerper2 = (offset - v[0].get(axis)) / (v[2].get(axis) - v[0].get(axis));
				vec3	new_vert2 = v[0] * (1 - lerper2) + v[2] * lerper2;
				new_vert2.set(axis, offset);	// make damn sure
				assert(new_vert2.checknan() == false);
				int	new_vi2 = m_verts.size();
				m_verts.push_back(new_vert2);

				// New faces.
				face	f0 = f;
				f0.m_vi[1] = new_vi1;
				f0.m_vi[2] = new_vi2;
				(*faces)[i] = f0;

				assert(classify_face(f0, axis, offset) <= 0);

				face	f1 = f;
				f1.m_vi[0] = new_vi1;
				f1.m_vi[2] = new_vi2;
				faces->push_back(f1);

				assert(classify_face(f1, axis, offset) >= 0);

				face	f2 = f;
				f2.m_vi[0] = new_vi2;
				faces->push_back(f2);

				assert(classify_face(f2, axis, offset) >= 0);
			}
		}
		
	}
}


void	kd_tree_dynamic::dump(tu_file* out) const
// Dump some debug info.
{
	node*	n = m_root;

	if (n) n->dump(out, 0);
}


void	kd_tree_dynamic::node::dump(tu_file* out, int depth) const
{
	for (int i = 0; i < depth; i++) { out->write_byte(' '); }

	if (m_leaf)
	{
		int	face_count = m_leaf->m_faces.size();
		char	c = ("0123456789X")[iclamp(face_count, 0, 10)];
		out->write_byte(c);
		out->write_byte('\n');
	}
	else
	{
		out->write_byte('+');
		out->write_byte('\n');
		if (m_neg)
		{
			m_neg->dump(out, depth + 1);
		}
		if (m_pos)
		{
			m_pos->dump(out, depth + 1);
		}
	}
}


#include "postscript.h"


static const int	X_SIZE = 612;
static const int	Y_SIZE = 792;
static const int	MARGIN = 20;


class kd_diagram_dump_info
{
public:
	postscript*	m_ps;
	int	m_depth;
	int	m_max_depth;
	std::vector<int>	m_width;	// width of the tree at each level
	std::vector<int>	m_max_width;
	std::vector<int>	m_count;	// width so far, during drawing

	// Some stats.
	int	m_leaf_count;
	int	m_node_count;
	int	m_face_count;
	int	m_max_faces_in_leaf;
	int	m_null_children;
	int	m_depth_times_faces;

	kd_diagram_dump_info()
		:
		m_ps(0),
		m_depth(0),
		m_max_depth(0),
		m_leaf_count(0),
		m_node_count(0),
		m_face_count(0),
		m_max_faces_in_leaf(0),
		m_null_children(0),
		m_depth_times_faces(0)
	{
	}

	void	get_node_coords(int* x, int* y)
	{
		float	h_spacing = (X_SIZE - MARGIN*2) / float(m_max_width.back());
		float	adjust = 1.0f;
		if (m_width[m_depth] > 1) adjust = (m_max_width[m_depth] + 1) / float(m_width[m_depth] + 1);

		*x = int(X_SIZE/2 + (m_count[m_depth] - m_width[m_depth] / 2) * h_spacing * adjust);
		*y = Y_SIZE - MARGIN - m_depth * (Y_SIZE - MARGIN*2) / (m_max_depth + 1);
	}

	void	update_stats(kd_tree_dynamic::node* n)
	// Add this node's stats to our totals.
	{
		if (n == 0)
		{
			m_null_children++;
		}
		else if (n->m_leaf)
		{
			m_leaf_count++;

			assert(n->m_leaf);
			int	faces = n->m_leaf->m_faces.size();
			m_face_count += faces;
			if (faces > m_max_faces_in_leaf) m_max_faces_in_leaf = faces;

			m_depth_times_faces += (m_depth + 1) * faces;
		}
		else
		{
			m_node_count++;
		}
	}

	void	diagram_stats()
	// Print some textual stats to the given Postscript stream.
	{
		float	x = MARGIN;
		float	y = Y_SIZE - MARGIN;
		const float	LINE = 10;
		y -= LINE; m_ps->printf(x, y, "Loose KD-Tree");
#ifdef MACDONALD_AND_BOOTH_METRIC
		y -= LINE; m_ps->printf(x, y, "using MacDonald and Booth metric");
#endif
#ifdef ADHOC_METRIC
		y -= LINE; m_ps->printf(x, y, "using ad-hoc metric");
#endif
#ifdef CARVE_OFF_SPACE
		y -= LINE; m_ps->printf(x, y, "using carve-off-space heuristic");
#endif
		y -= LINE; m_ps->printf(x, y, "leaf face count limit: %d", LEAF_FACE_COUNT);
		y -= LINE; m_ps->printf(x, y, "face ct: %d", m_face_count);
		y -= LINE; m_ps->printf(x, y, "leaf ct: %d", m_leaf_count);
		y -= LINE; m_ps->printf(x, y, "node ct: %d", m_node_count);
		y -= LINE; m_ps->printf(x, y, "null ct: %d", m_null_children);
		y -= LINE; m_ps->printf(x, y, "worst leaf: %d faces", m_max_faces_in_leaf);
		y -= LINE; m_ps->printf(x, y, "max depth: %d", m_max_depth + 1);
		y -= LINE; m_ps->printf(x, y, "avg face depth: %3.2f", m_depth_times_faces / float(m_face_count));
	}
};


static void	node_traverse(kd_diagram_dump_info* inf, kd_tree_dynamic::node* n)
// Traverse the tree, updating inf->m_width.  That's helpful for
// formatting the diagram.
{
	inf->update_stats(n);

	if (inf->m_depth > inf->m_max_depth)
	{
		inf->m_max_depth = inf->m_depth;
	}

	while ((int) inf->m_width.size() <= inf->m_max_depth)
	{
		inf->m_width.push_back(0);
	}

	inf->m_width[inf->m_depth]++;	// count this node.

	if (n && n->m_leaf == 0)
	{
		// Count children.
		inf->m_depth++;
		node_traverse(inf, n->m_neg);
		node_traverse(inf, n->m_pos);
		inf->m_depth--;

		assert(inf->m_depth >= 0);
	}
}


static void	node_diagram(kd_diagram_dump_info* inf, kd_tree_dynamic::node* n, int parent_x, int parent_y)
// Emit Postscript drawing commands to diagram this node in the tree.
{
	// Diagram this node.
	int	x, y;
	inf->get_node_coords(&x, &y);

	// Line to parent.
	inf->m_ps->line((float) x, (float) y, (float) parent_x, (float) parent_y);

	if (n == 0)
	{
		// NULL --> show a circle w/ slash
		inf->m_ps->circle((float) x, (float) y, 1);
		inf->m_ps->line((float) x + 1, (float) y + 1, (float) x - 1, (float) y - 1);
	}
	else if (n->m_leaf)
	{
		// Leaf.  Draw concentric circles.
		int	face_count = n->m_leaf->m_faces.size();
		for (int i = 0; i < face_count + 1; i++)
		{
			inf->m_ps->circle((float) x, (float) y, 2 + i * 1.0f);
		}
	}
	else
	{
		// Internal node.

		// draw disk
		inf->m_ps->disk((float) x, (float) y, 1);

		// draw children.
		inf->m_depth++;
		node_diagram(inf, n->m_neg, x, y);
		node_diagram(inf, n->m_pos, x, y);
		inf->m_depth--;

		assert(inf->m_depth >= 0);
	}

	// count this node.
	inf->m_count[inf->m_depth]++;
}


void	kd_tree_dynamic::diagram_dump(tu_file* out) const
// Generate a Postscript schematic diagram of the tree.
{
	postscript*	ps = new postscript(out, "kd-tree diagram");
	
	kd_diagram_dump_info	inf;
	inf.m_ps = ps;
	inf.m_depth = 0;

	node_traverse(&inf, m_root);

	while ((int) inf.m_count.size() <= inf.m_max_depth)
	{
		inf.m_count.push_back(0);
	}

	int	max_width = 1;
	for (int i = 0; i <= inf.m_max_depth; i++)
	{
		if (inf.m_width[i] > max_width)
		{
			max_width = inf.m_width[i];
		}
		inf.m_max_width.push_back(max_width);
	}

	inf.diagram_stats();

	int	root_x = 0, root_y = 0;
	inf.get_node_coords(&root_x, &root_y);

	node_diagram(&inf, m_root, root_x, root_y);

	delete ps;
}


static void	mesh_node_dump(
	postscript* ps,
	int axis,
	kd_tree_dynamic::node* node,
	const axial_box& bound,
	const std::vector<vec3>& verts)
// Draw faces under node, projected onto given axis plane.  Scale to fit paper.
{
	if (node == NULL) return;

	if (node->m_leaf)
	{
		// Draw faces.
		for (int i = 0, n = node->m_leaf->m_faces.size(); i < n; i++)
		{
			vec3	v[3] = {
				verts[node->m_leaf->m_faces[i].m_vi[0]],
				verts[node->m_leaf->m_faces[i].m_vi[1]],
				verts[node->m_leaf->m_faces[i].m_vi[2]]
			};

			float	x[3], y[3];
			int	axis1 = (axis + 1) % 3;
			int	axis2 = (axis + 2) % 3;
			for (int vert = 0; vert < 3; vert++)
			{
				x[vert] = (v[vert][axis1] - bound.get_min()[axis1]) / bound.get_size()[axis1];
				y[vert] = (v[vert][axis2] - bound.get_min()[axis2]) / bound.get_size()[axis2];

				x[vert] = flerp(float(MARGIN), float(X_SIZE - MARGIN), x[vert]);
				y[vert] = flerp(float(MARGIN), float(Y_SIZE - MARGIN), y[vert]);
			}

			// Draw triangle.
			ps->line(x[0], y[0], x[1], y[1]);
			ps->line(x[1], y[1], x[2], y[2]);
			ps->line(x[2], y[2], x[0], y[0]);
		}

		return;
	}
	
	mesh_node_dump(ps, axis, node->m_neg, bound, verts);
	mesh_node_dump(ps, axis, node->m_pos, bound, verts);
}


void	kd_tree_dynamic::mesh_diagram_dump(tu_file* out, int axis) const
// Generate a Postscript schematic diagram of the mesh, orthogonal to
// the given axis.
{
	postscript*	ps = new postscript(out, "kd-tree diagram");
	
	mesh_node_dump(ps, axis, m_root, get_bound(), m_verts);

	delete ps;
}


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
