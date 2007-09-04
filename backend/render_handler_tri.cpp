// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

// 
//

/* $Id: render_handler_tri.cpp,v 1.21 2007/09/04 02:12:55 cmusick Exp $ */

#include "render_handler_tri.h"

#ifndef RENDER_HANDLER_H
#error missing includes!
#endif

#ifndef GNASH_RENDER_HANDLER_TRI_H
#error missing includes!
#endif

#include "log.h"

namespace gnash {

// helper function for tri_cache_manager
static int	sort_by_decreasing_error(const void* A, const void* B)
{
  const mesh_set*	a = *(const mesh_set* const *) A;
  const mesh_set*	b = *(const mesh_set* const *) B;
  float atol = a->get_error_tolerance(); 
  float btol = b->get_error_tolerance(); 
  
  if (atol < btol) {
    return 1;
  } else if (atol > btol) {
    return -1;
  } else {
    return 0;
  }
}

//------------------------------------------------------------------------------

mesh_set* tri_cache_manager::search_candidate(float max_error)  {

  for (unsigned int i=0,n=m_cached_meshes.size(); i<n; i++) {
  
    mesh_set* candidate = m_cached_meshes[i];
    float candidate_etol = candidate->get_error_tolerance();
  
    if (max_error > candidate_etol * 3.0f) {
      // Mesh is too high-res; the remaining meshes are higher res,
      // so stop searching and build an appropriately scaled mesg.
      break;
    }
    
    if (max_error > candidate_etol) {
      // found it!
      return candidate;
    }
  
  } // for
  
  // when we come here it means that no candidate could be found... :-(
  return NULL;
  
} // search_candidate
  
  
/// Adds a mesh set to the cache. 
void tri_cache_manager::add(mesh_set* m) {
  m_cached_meshes.push_back(m);
  sort_and_clean_meshes();
} // add


/// Maintain cached meshes. Clean out mesh_sets that haven't been used 
/// recently, and make sure they're sorted from high error to low error.
void tri_cache_manager::sort_and_clean_meshes() {

  // Re-sort.
  if (m_cached_meshes.size() > 0) {
  
    qsort(
      &m_cached_meshes[0],
      m_cached_meshes.size(),
      sizeof(m_cached_meshes[0]),
      sort_by_decreasing_error
    );
  
    // TODO: The cache will grow forever, without any limit. Add code that
    // limits the vector to a certain size. The older Gnash implementation
    // appears to never have removed unused cache objects!
  
  }
  
  // Sort check omitted!

} // sort_and_clean_meshes
  


//------------------------------------------------------------------------------

// Can't initialize this in header, for ISO C++ conformance:
// error: ISO C++ forbids initialization of member constant `s_curve_max_pixel_error'
// of non-integral type `const float'
const float triangulating_render_handler::s_curve_max_pixel_error = 1.0f;


void triangulating_render_handler::draw_glyph(shape_character_def *def,
    const matrix& mat, const rgba& color, float pixel_scale) {
    
  // Make sure m_single_fill_styles contains the desired color
  need_single_fill_style(color);
    
  // A glyph is a notmal character, so draw it normally
  draw_shape_character(def, mat, m_neutral_cxform, pixel_scale, 
    m_single_fill_styles, m_dummy_line_styles);
}


  
  
void triangulating_render_handler::draw_shape_character(shape_character_def *def, 
  const matrix& mat,
  const cxform& cx,
  float pixel_scale,
  const std::vector<fill_style>& fill_styles,
  const std::vector<line_style>& line_styles) {    
  tri_cache_manager* cman = get_cache_of(def);
  
  // Compute the error tolerance in object-space.
  float	max_scale = mat.get_max_scale();
  
  if (fabsf(max_scale) < 1e-6f) {
    // Scale is essentially zero.
    return;
  }    
  
  float	object_space_max_error = 
    20.0f / max_scale / pixel_scale * s_curve_max_pixel_error;
    
    
  // NOTE: gnash_debug_show_paths ommitted
  
  // Try to find a usable mesh set in the cache
  mesh_set *m = cman->search_candidate(object_space_max_error);
  int from_cache = m != NULL;
   
  if (!from_cache) {
    // no cache hit, construct a new mesh to handle this error tolerance.
    m = new mesh_set(def, object_space_max_error * 0.75f);
  };

  // draw the mesh set    
  draw_mesh_set(*m, mat, cx, fill_styles, line_styles, 1.0);
    
  
  if (!from_cache) {
    // add to cache (do this after drawing the mesh!)
    cman->add(m);    
  }
  
} // draw_shape_character



void triangulating_render_handler::draw_mesh_set(const mesh_set& m, 
  const matrix& mat, const cxform& cx,
  const std::vector<fill_style> &fill_styles,
  const std::vector<line_style> &line_styles, float ratio) {
  
  set_matrix(mat);
  set_cxform(cx);
  
  // draw fills
  for (unsigned int i = 0; i < m.m_meshes.size(); i++) {
    const mesh& the_mesh = m.m_meshes[i];
    const fill_style& the_style = fill_styles[i];    
    
    if (!the_mesh.m_triangle_strip.size()) continue; // nothing to draw
    
    apply_fill_style(the_style, 0, ratio);
    draw_mesh_strip(&the_mesh.m_triangle_strip[0], 
      the_mesh.m_triangle_strip.size() / 2);   
  }
  
  // draw outlines
  for (unsigned int i = 0; i < m.m_line_strips.size(); i++)
  {
    int	style = m.m_line_strips[i].get_style();
    const line_strip& strip = m.m_line_strips[i];
    
    assert(strip.m_coords.size() > 1);
    assert((strip.m_coords.size() & 1) == 0);
    apply_line_style(line_styles[style], ratio);
    
    draw_line_strip(&strip.m_coords[0], strip.m_coords.size() >> 1);
  }
    
  
} // draw_mesh_set


void triangulating_render_handler::apply_fill_style(const fill_style& style, 
  int fill_side, float ratio) {

  UNUSED(ratio);
  
  if (style.m_type == SWF::FILL_SOLID)
  {
    // 0x00: solid fill
    fill_style_color(fill_side, style.m_color); 
  }
  else if (style.m_type == SWF::FILL_LINEAR_GRADIENT
    || style.m_type == SWF::FILL_RADIAL_GRADIENT
	|| style.m_type == SWF::FILL_FOCAL_GRADIENT)
  {
    // 0x10: linear gradient fill
    // 0x12: radial gradient fill
    
    style.need_gradient_bitmap();
    
    if (style.m_gradient_bitmap_info != NULL) {
      fill_style_bitmap(
        fill_side,
        style.m_gradient_bitmap_info.get(),
        style.m_gradient_matrix,
        gnash::render_handler::WRAP_CLAMP);
    }
  }
  else if (style.m_type == SWF::FILL_TILED_BITMAP
    || style.m_type == SWF::FILL_CLIPPED_BITMAP
    || style.m_type == SWF::FILL_TILED_BITMAP_HARD
    || style.m_type == SWF::FILL_CLIPPED_BITMAP_HARD)
  {
    
    // bitmap fill (either tiled or clipped)
    
    gnash::bitmap_info*	bi = NULL;
    
    if (style.m_bitmap_character != NULL)	{
    
      bi = style.m_bitmap_character->get_bitmap_info();
      
      if (bi != NULL)	{
      
        gnash::render_handler::bitmap_wrap_mode	
          wmode = gnash::render_handler::WRAP_REPEAT;
          
        if (style.m_type == SWF::FILL_CLIPPED_BITMAP
          || style.m_type == SWF::FILL_CLIPPED_BITMAP_HARD)
        {
          wmode = gnash::render_handler::WRAP_CLAMP;
        }
        fill_style_bitmap(
          fill_side,
          bi,
          style.m_bitmap_matrix,
          wmode);
      }
    }
  }  

} // apply_fill_stype


void triangulating_render_handler::apply_line_style(const line_style& style, 
  float ratio) {
  
  UNUSED(ratio);
  line_style_color(style.m_color);
  line_style_width(style.m_width);  
} // apply_line_style



void	triangulating_render_handler::draw_line_strip(const void* coords, 
  int vertex_count, const rgba& color) {
  
  line_style_color(color);
  line_style_width(1);
  draw_line_strip(coords, vertex_count);
  
}

void  triangulating_render_handler::draw_poly(const point* corners, 
  size_t corner_count, const rgba& fill, const rgba& outline, bool /*masked*/) {
  
  // TODO: The current implementation is only correct when masked==true,
  // ie. it will always be masked.
  
  unsigned int vno=0;
  // Create points array to vertex array 
  int16_t *vertex = new int16_t[(corner_count+1)*2];
  for (unsigned int cno=0; cno<corner_count; cno++) {
    vertex[vno  ] = static_cast<int16_t>(corners[cno].m_x);
    vertex[vno+1] = static_cast<int16_t>(corners[cno].m_y);
    vno+=2;
  }
  // add one more point to close the polygon
  vertex[vno  ] = vertex[0];
  vertex[vno+1] = vertex[1];
  
  // fill the polygon
  if (fill.m_a>0) {
    fill_style_color(0, fill);
    draw_mesh_strip(vertex, corner_count+1);
  }
  
  // draw the polygon outline
  if (outline.m_a>0) {
    line_style_color(outline);
    line_style_width(1.0f);
    draw_line_strip(vertex, corner_count+1);
  } 
  
  delete[] vertex;
  
} // draw_poly




tri_cache_manager* triangulating_render_handler::get_cache_of(character_def* def) {

  if (def->m_render_cache == NULL) {
    def->m_render_cache = new tri_cache_manager;
  }
  
  return (tri_cache_manager*) def->m_render_cache;

} // get_cache_of
 

geometry::Range2d<int>
triangulating_render_handler::world_to_pixel(const rect& worldbounds)
{
  // TODO: verify this is correct
  geometry::Range2d<int> ret(worldbounds.getRange());
  ret.scale(1.0/20.0); // twips to pixels
  return ret;
}

point 
triangulating_render_handler::pixel_to_world(int x, int y)
{
  // TODO: verify this is correct
  return point(PIXELS_TO_TWIPS(x), PIXELS_TO_TWIPS(y));
}

tri_cache_manager::~tri_cache_manager()
{
	for (MeshSetList::iterator i=m_cached_meshes.begin(), e=m_cached_meshes.end();
			i != e; ++i)
	{
		delete *i;
	}
}


} // namespace gnash
