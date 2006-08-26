// cull.h	-- by Thatcher Ulrich <tu@tulrich.com> 16 June 2001

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Routines for frustum culling.


#ifndef CULL_H
#define CULL_H


#include "geometry.h"


namespace cull {

	class result_info {
	public:
		bool	culled;	// true when the volume is not visible
		uint8_t	active_planes;	// one bit per frustum plane
		
		result_info(bool c = false, uint8_t a = 0x3f) : culled(c), active_planes(a) {}
	};
	
	
	result_info	compute_box_visibility(const vec3& center, const vec3& extent,
					       const plane_info frustum[6], result_info in);
}


#endif // CULL_H
