// cull.cpp	-- by Thatcher Ulrich <tu@tulrich.com> 16 June 2001

// This source code has been donated to the Public Domain.


#include "cull.h"
#include "tu_math.h"
#include "utility.h"


namespace cull {


result_info	compute_box_visibility(const vec3& center, const vec3& extent, const plane_info frustum[6], result_info in)
// Returns a visibility code indicating the culling status of the
// given axis-aligned box.  The result_info passed in should indicate
// which planes might cull the box, by setting the corresponding
// bit in in.active_planes.
{
//	if (in.culled) return in;	// This check should be done by caller.

	// Check the box against each active frustum plane.
	int	bit = 1;
	for (int i = 0; i < 6; i++, bit <<= 1) {
		if ((bit & in.active_planes) == 0) {
			// This plane is already known to not cull the box.
			continue;
		}

		const plane_info&	p = frustum[i];
		// Check box against this plane.
		float	d = p.normal * center - p.d;
		float	extent_toward_plane = fabsf(extent.get_x() * p.normal.get_x())
			+ fabsf(extent.get_y() * p.normal.get_y())
			+ fabsf(extent.get_z() * p.normal.get_z());
		if (d < 0) {
			if (-d > extent_toward_plane) {
				// Box is culled by plane; it's not visible.
				return result_info(true, 0);
			} // else this plane is ambiguous so leave it active.
		} else {
			if (d > extent_toward_plane) {
				// Box is accepted by this plane, so
				// deactivate it, since neither this
				// box or any contained part of it can
				// ever be culled by this plane.
				in.active_planes &= ~bit;
				if (in.active_planes == 0) {
					// This box is definitively inside all the culling
					// planes, so there's no need to continue.
					return in;
				}
			} // else this plane is ambigious so leave it active.
		}
	}

	return in;	// Box not definitively culled.  Return updated active plane flags.
}


} // end namespace cull

