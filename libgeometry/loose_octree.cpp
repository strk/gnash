// loose_octree.cpp	-- by Thatcher Ulrich <tu@tulrich.com>

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// bounding-volume hierarchy for doing spatial queries on objects.


#ifndef SPATIAL_CONTAINER_H
#define SPATIAL_CONTAINER_H


#include "geometry.h"

// sp_tag is used as a link node for the spatial_container.
// 
// @@ questions: best to keep this separate from the actual object
// storage?  Or is there an advantage to allowing contained objects to
// derive from this tag type (probably not -- prevents putting objects
// in multiple containers, and isn't too intuitive).
//

struct sp_tag {
	vec3	center;
	float	extent;	// half-size of an edge for cubes, radius for spheres.
	void*	payload;

	sp_tag	*next;
	sp_tag	*previous;
};


struct spatial_container_result_callback {
	virtual void	result_callback( const sp_tag& result );
};


class spatial_container_void {

	spatial_container_void( base_cell_extent, max_depth );
  
#if EXAMPLES
	// with the iterator pattern, I think the result callback may be useless...
	// you just do this sort of thing:
	//
	for ( spatial_container_void::iterator i( container, center, extent );
		  ! i.is_finished();
		  i.advance() )
	{
		i.get_current()->payload...;
	}

	// define a spatial_query class, templated on type of container...?  dunno if that's possible.

	foreach_spatial_in_cube( type, var, container, center, extent, hint ) {
		if ( square_length( var->GetPosition(), center ) < extent * extent ) {
			var->apply_effect();
		}
	}

	foreach_spatial_in_frustum( type /* from container? or useful for casting. */, var, container, frustum, hint ) {
		var->Render();
	}

	ray_test( &result, ray, );

#endif // EXAMPLES


	class cube_iterator {
	public:
		// void	advance();
		// sp_tag*	get_current();
		sp_tag*	get_next();	// return 0 when finished?
		bool	is_finished();

	private:
		vec3	query_center;
		float	query_extent;
		// int	direction_hint;
		// int	query_outcode;	// @@ figure this out.

		node*	current_node;
		sp_tag*	current_tag;
	};


//	class ray_iterator {
//	};


//	class frustum_iterator {
//	};

};


#endif // SPATIAL_CONTAINER_H

