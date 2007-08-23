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

/* $Id: render_handler_tri.h,v 1.15 2007/08/23 09:53:03 udog Exp $ */

#ifndef GNASH_RENDER_HANDLER_TRI_H
#define GNASH_RENDER_HANDLER_TRI_H


// Original version by Udo Giacomozzi, Indunet GmbH (www.indunet.it)


#include "gnash.h"
#include "types.h"
#include "image.h"
#include "utility.h"
#include "log.h"

#include "render_handler.h"

#ifndef RENDER_HANDLER_H
#error missing includes!
#endif

namespace gnash
{

class DSOEXPORT tri_cache_manager : public render_cache_manager
{
private:
  /// A list of pre-computed meshes. Each mesh set was computed with a certain
  /// error tolerance. The error tolerance defines the number of straight lines 
  /// that are used to reconstruct a curve. As this approximation becomes
  /// visible when magnifing a character, different mesh sets are cached.  
  /// Of course, more edge lines produce more triangles which slows down 
  /// rendering. So we try to use a mesh set with the minimal triangle amount
  /// that's still acceptable when viewed.
  /// To draw these meshes they still have to be transformed using the desired
  /// matrix!
  ///
  /// This instance owns the mesh_sets
  ///
  typedef std::vector <mesh_set*> MeshSetList;
  MeshSetList m_cached_meshes;
  
public:  

  /// Searches a mesh set with a error tolerance below max_error.
  /// Returns NULL when no candidate could be found.
  /// Ownership is NOT transferred
  mesh_set* search_candidate(float max_error);
    
  /// Adds a mesh set to the cache. 
  //
  /// Transfer ownerhips of the mesh_set.
  /// TODO: take an auto_ptr
  ///
  void add(mesh_set* m);
    
  /// Maintain cached meshes. Clean out mesh_sets that haven't been used 
  /// recently, and make sure they're sorted from high error to low error.
  void sort_and_clean_meshes();  

  /// Drop all cached meshes
  virtual ~tri_cache_manager();
  
}; // class tri_cache_manager




/// \brief
/// This is the base class for all triangle-based renderers (hardware 
/// accelerated ones, for example).
//
/// It shields the renderer from the special
/// Flash edges that have two fill styles - one for each side. Also, Flash 
/// shapes do not contain single, closed polygons. 
/// The triangulating_render_handler contains algorithms that transform the
/// shapes to simplified triangle sets ("mesh sets") that are relatively easy
/// to render.
///
class triangulating_render_handler : public render_handler 
{
public:
  
  // --- reimplemented methods -------------------------------------------------  
  // The implementation of this virtual method takes care that the shape is
  // translated to a mesh set. Triangulating backends do not need to declare
  // their own draw_shape_character. 
  
  /// This method is required by all render handlers. The triangulating render
  /// handler has already a common implementation that uses a tesselator and
  /// generates mesh sets that can be used by simpler handlers.  
  /*void draw_shape_character(shape_character_def *def, 
    character *inst);*/
    
  /// Alterante version of draw_shape_character() which does not depend on any
  /// character instance and accepts arbitrary line and fill styles. 
	void draw_shape_character(shape_character_def *def, 
    const matrix& mat,
    const cxform& cx,
    float pixel_scale,
    const std::vector<fill_style>& fill_styles,
    const std::vector<line_style>& line_styles);    

  /// Triangulating render handlers do not need to support this special version
  /// of draw_line_strip().     
  void	draw_line_strip(const void* coords, int vertex_count, const rgba& color);
  
  /// The given polygon is translated to a mesh strip by this class.
  void  draw_poly(const point* corners, size_t corner_count, const rgba& fill, 
    const rgba& outline, bool masked);
    
  /// The glyph is drawn just like a normal shape character.
  virtual void draw_glyph(shape_character_def *def,
    const matrix& mat, const rgba& color, float pixel_scale);
    
  /// Older backends always used glyph textures, so any triangulating render
  /// handler activates glyph textures by default.
  bool allow_glyph_textures() { return true; }

  /// Translates world coordinates to pixel coordinates
  geometry::Range2d<int> world_to_pixel(const rect& worldbounds);
  
  point pixel_to_world(int x, int y);
    
  // --- helper methods --------------------------------------------------------
  // These methods are called by draw_shape_character() and normally don't need
  // to be accessed by other classes. They are here to make the class more
  // readable and flexible.
  // Note: The old methods mesh_set::display(), fill_style::apply() etc. have 
  // been removed to disallow them completely since these methods rely on 
  // triangular renderers which may not be available. This is the reason why
  // the methods have been re-implemented inside triangulating_render_handler. 
  
  void draw_mesh_set(const mesh_set& m, const matrix& mat, const cxform& cx,
    const std::vector<fill_style> &fill_styles,
    const std::vector<line_style> &line_styles, float ratio);
    
  void apply_fill_style(const fill_style& style, int fill_side, float ratio);
  void apply_line_style(const line_style& style, float ratio);
  
  
  
  // --- low level graphic methods ---------------------------------------------
  // These methods were defined in the older render_handler design. They must be
  // implemented by the real handlers as they are defined abstract.   
  
  /// Disables the current fill style (so that it is transparent) for the 
  /// given fill side (which is always zero?).
	virtual void	fill_style_disable(int fill_side) = 0;
	
	/// Sets the fill style to the given solid color. 
	virtual void	fill_style_color(int fill_side, const rgba& color) = 0;
	
	/// Sets the fill style to the given bitmap. This is also used for gradients
	/// which are transformed to a bitmap prior to this call. 
	virtual void	fill_style_bitmap(int fill_side, const bitmap_info* bi, 
    const matrix& m, bitmap_wrap_mode wm) = 0;
  
  /// Disables the current line style
	virtual void	line_style_disable() = 0;
	
	/// Sets the stroke color for subsequent draw_line_strip() calls 
	virtual void	line_style_color(const rgba& color) = 0;
	
	/// Sets the stroke width for subsequent draw_line_strip() calls. When 
  /// width==1.0 a "hairline" should be drawn. 
	virtual void	line_style_width(float width) = 0;
	
	/// Draws the given line strip using the current transformation matrix.
  virtual void	draw_line_strip(const void* coords, int vertex_count) = 0;
	
	/// Draws the given mesh strip (trianges) using the current transformation 
  /// matrix.
  virtual void	draw_mesh_strip(const void* coords, int vertex_count) = 0; 

    

protected:
  static const float	s_curve_max_pixel_error; //  = 1.0f;
    
  
  /// Returns the cache manager instance of the given character definition.
  /// Allocates a new manager if necessary.
  tri_cache_manager* get_cache_of(character_def* def);
  

}; // class render_handler_impl

} // namespace gnash

#endif // GNASH_RENDER_HANDLER_IMPL_H
