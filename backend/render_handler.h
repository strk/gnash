// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//

/* $Id: render_handler.h,v 1.18 2006/10/27 14:50:33 alexeev Exp $ */

#ifndef RENDER_HANDLER_H
#define RENDER_HANDLER_H

/// \page render_handler_intro Render handler introduction
///
/// Information for writing new render handlers:
///
/// There are two ways to write a new render handler:
/// - either you write a handler that must only be able to draw line strips and
///   and triangles (with various fill styles, however). This we call a 
///   triangulating render handler.
/// - or your handler can deal with the special fill style Flash uses directly,
///   which does *not* apply to most hardware accelerations. In this case the
///   unaltered original shapes are passed to the renderer, which is responsible
///   of drawing them correctly. 
///
/// For triangulating render handlers, see gnash::triangulating_render_handler
/// (defined in render_handler_tri.h), otherwise read on...
///
/// The most important thing about drawing Flash shapes is to understand how 
/// their fill styles work. 
/// A single Flash character can contain any number shapes that use any number
/// of different fill styles and line styles. The shapes of a character are 
/// defined by a number of "paths". Some important things about paths:
///
/// - A path is a list of connected straight lines and (quadratic bezier) 
///   curves (=edges). Interesting to note is that in the Flash world there are 
///   *no* primitive objects like circles, rectangles or similar. These objects 
///   are always translated to lines and curves (a circle is a set of eight 
///   curves).
///            
/// - All paths together must by definition always build a fully closed shape. 
///   You can't draw a rectangle with three edges, for example, contrary to 
///   most graphics library polygon routines that connect the last anchor to
///   the first. However, a *single* path does *not* have to be closed. The
///   missing parts may be defined by other paths (you will see this makes
///   sense).
/// 
/// - Each path has up to two fill styles and no or one line style. The line
///   style should be obvious. The two fill styles define the fill to the left
///   (fill style zero) and to the right (fill style one) of the path if you
///   think of it like a vector. The fill style is defined by a index to a 
///   list of previously defined fill style definitions. Index 0 means "no 
///   style" and is equal to a fully transparent fill style ("hole", if you 
///   wish).
///
/// - Paths are *never* self-intersecting. 
///   
///  Simple examples to understand this concept:
///
///  - A rectangle that contains another rectangle. Only the area between the 
///    two rectangles is filled (so it looks like a "o"). In this case Flash
///    fill create two paths (one for each rectangle) and one fill style. Assume
///    both paths come in clockwise order, then the outer rectangle will have
///    fillstyle0=0 and fillstyle1=1. The inner rectangle will have 
///    fillstyle0=1 and fillstyle1=0.
///
/// \code
///      +--------------------------------+
///      |XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX|
///      |XXX+------------------------+XXX|
///      |XXX|                        |XXX|
///      |XXX+------------------------+XXX|
///      |XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX|
///      +--------------------------------+
/// \endcode
///
///  - A rectangle is divided vertically in two halves, both having different
///    colors:
///
/// \code
///      +-------A-------+-------B--------+
///      |...............|################|
///      A...............C################B
///      |...............|################|
///      +-------A-------+-------B--------+
/// \endcode
///
///    Flash will probably produce three paths (A,B,C) and two fill styles.
///    Paths "A" and "B" will have just one fill style (fillstyle1 will be 
///    zero) while path "C" (which contains only one straight line!!) will
///    have two fill styles. To be exact the horizontal edges would not even
///    be necessary to render this shape (for a scanline based renderer) but 
///    they are necessary then the character is about to be rotated.
///
/// Now, these are simple examples but complex graphics can be compressed very
/// efficiently this way. Also, this method was most probably intended for a
/// renderer engine that can produce the final character in just one pass 
/// (like the AGG backend does too).    

  
/// \page region_update Detection of updated regions
///
/// (this applies to the whole Gnash playback architecture)
///
/// After advancing the root movie (see Gui::advance_movie) it is checked
/// which region of the stage has been changed visibly (by computing the 
/// bounds around updated characters). This has two advantages:
/// 
/// 1st, it allows a renderer/gui combination to avoid re-rendering of
/// unchanged parts in the scene. When supported by the rendering engine
/// this can be a huge performance gain. The original Flash player does
/// that too, btw. Altough he is able to define multiple smaller regions
/// for one frame. This could be implemented in Gnash, too.
/// 
/// 2nd, it can detect still frames (like a stopped movie). gui.cpp can
/// detect these and completely avoids to call any rendering function.
/// 
/// Of course, the most critical part is detection of changes. There is a 
/// method called set_invalidated() which gets called whenever a critical
/// property of a instance gets updated, like when it changes position, for
/// example. It's really important to always call set_invalidated() whenever 
/// code is added that changes the character instance in a visible way.
/// 
/// Even if no renderer really uses this information it has effects when
/// skipping unchanged frames. If necessary, this feature can be switched
/// off easily in gui.cpp (maybe using a runtime option?).
///
/// Note the updated region is only passed to the GUI, which is itself 
/// responsible of informing the renderer. This is because it's pointless
/// to have a renderer which updates only a small part of the stage when
/// the GUI shows it all since the area aroung the region is undefined.
/// However, there can be a GUI which supports update regions without needing
/// the renderer to do so (for example, to save time during blitting).
/// The GUI can also completely ignore the region information. 
/// 
/// As for the integer/float discussion: I used rect (floats) because all
/// the bounds calculation involves floats anyway and so it's probably
/// faster than converting between ints and floats all the way.


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h" // for DSOEXPORT

#include "shape_character_def.h"  
#include "generic_character.h"    


// Forward declarations.
namespace gnash {
	class bitmap_info;
	class rect;
	class rgba;
	class matrix;
	class cxform;
	
	class shape_character_def;
	class generic_character;
}
// @@ forward decl to avoid including base/image.h; TODO change the
// render_handler interface to not depend on these classes at all.
namespace image { class image_base; class rgb; class rgba; }

namespace gnash {

/*
// Base class for renderer general cache objects
class render_cache_object
{
  
}
*/


class DSOEXPORT render_cache_manager
{
public:
  //Virtual dtor, removes compiler warning.
  virtual ~render_cache_manager(){}
  /// Clears the cache completely (necessary for runtime shapes / drawing API)
  virtual void clear() 
  {
    // TODO: Make this abstract to force real implementation!!
    // nop
  } 
};


/// Base class for render handlers.
//
/// You must define a subclass of render_handler, and pass an
/// instance to set_render_handler() *before* any SWF parsing begins.
///
/// For more info see page \ref render_handler_intro.
/// 
///
class DSOEXPORT render_handler
{
public:
	virtual ~render_handler() {}

	// Your handler should return these with a ref-count of 0.  (@@ is that the right policy?)

	/// \brief
	///  Create a bitmap_info so that it contains an alpha texture
	/// with the given data (1 byte per texel).
	//
	/// Munges *data (in order to make mipmaps)!!
	///
	virtual bitmap_info*	create_bitmap_info_alpha(int w, int h, unsigned char* data) = 0;

	/// \brief
	/// Given an image, returns a pointer to a bitmap_info class
	/// that can later be passed to fill_styleX_bitmap(), to set a
	/// bitmap fill style.
	virtual bitmap_info*	create_bitmap_info_rgb(image::rgb* im) = 0;

	/// \brief
	/// Given an image, returns a pointer to a bitmap_info class
	/// that can later be passed to fill_style_bitmap(), to set a
	/// bitmap fill style.
	//
	/// This version takes an image with an alpha channel.
	///
	virtual bitmap_info*	create_bitmap_info_rgba(image::rgba* im) = 0;

	/// Delete the given bitmap info class.
	virtual void	delete_bitmap_info(bitmap_info* bi) = 0;

	virtual YUV_video* create_YUV_video(int width, int height) = 0;	
	virtual void delete_YUV_video(YUV_video* yuv) = 0;

	/// Sets the update region (called prior to begin_display).
	//
	/// It is not required for all renderers.
	/// Parameters are world coordinates.
	///
	/// For more info see page \ref region_update.
	///
	virtual void set_invalidated_region(const rect /*bounds*/) {    
		// implementation is optional    
	}
	
  /// Converts world coordinates to pixel coordinates
  virtual void world_to_pixel(int *x, int *y, const float world_x, 
    const float world_y) = 0;  
		
	/// Bracket the displaying of a frame from a movie.
	//
	/// Fill the background color, and set up default
	/// transforms, etc.
	///
	virtual void	begin_display(
		rgba background_color,
		int viewport_x0, int viewport_y0,
		int viewport_width, int viewport_height,
		float x0, float x1, float y0, float y1) = 0;

	virtual void	end_display() = 0;
		
	/// Geometric transforms for mesh and line_strip rendering.
	virtual void	set_matrix(const matrix& m) = 0;

	/// Color transforms for mesh and line_strip rendering.
	virtual void	set_cxform(const cxform& cx) = 0;
		
	/// Draw a line-strip directly, using a thin, solid line. 
	//
	/// Can be used to draw empty boxes and cursors.
	virtual void	draw_line_strip(const void* coords, int vertex_count,
    const rgba color) = 0;
    
  /// Draw a simple, solid filled polygon (no outline).
  //
  /// This can't be used for 
  /// Flash shapes but is intended for internal drawings like bounding boxes 
  /// (editable text fields) and similar. The polygon should not contain 
  /// self-intersections. If you do not wish a outline or a fill, then simply 
  /// set the alpha value to zero.
  ///
  /// The polygon need NOT be closed (ie: this function will automatically
  /// add an additional vertex to close it.
  ///
  virtual void  draw_poly(const point* corners, size_t corner_count, 
    const rgba fill, const rgba outline) = 0;
    
		
	/// Set line and fill styles for mesh & line_strip rendering.
	enum bitmap_wrap_mode
	{
		WRAP_REPEAT,
		WRAP_CLAMP
	};
		
		
	/// Special function to draw a rectangular bitmap.
	//
	/// Intended for textured glyph rendering.  Ignores
	/// current transforms.
	///
	virtual void	draw_bitmap(
		const matrix&		m,
		const bitmap_info*	bi,
		const rect&		coords,
		const rect&		uv_coords,
		rgba			color) = 0;
		
	virtual void	set_antialiased(bool enable) = 0;
		
	virtual void begin_submit_mask() = 0;
	virtual void end_submit_mask() = 0;
	virtual void disable_mask() = 0;
	
	/// \brief
	/// Draws the given character definition with the stateful properties
	/// of the given instance.
	//
	/// Normally this does not need to be re-implemented in 
	/// render handler implementations. Instead, see the version without
	/// character instance.
	///
	virtual void draw_shape_character(shape_character_def *def, 
    character *inst) {
    
    // TODO: I don't like that there is a draw_shape_character() version with
    // arbitrary fill and line styles as this may break caching...
  
    draw_shape_character(def, 
      inst->get_world_matrix(), 
      inst->get_world_cxform(),
      inst->get_parent()->get_pixel_scale(),
      def->get_fill_styles(),
      def->get_line_styles());

  }

  /// \brief
  /// Draws the given character definition with the given transformations and
  /// styles. 
	virtual void draw_shape_character(shape_character_def *def, 
    const matrix& mat,
    const cxform& cx,
    float pixel_scale,
    const std::vector<fill_style>& fill_styles,
    const std::vector<line_style>& line_styles) = 0;
    
  /// \brief
  /// Draws a glyph (font character).
  //
  /// Glyphs are defined just like shape characters with the difference that
  /// they do not have any fill or line styles.
  /// Instead, the shape must be drawn using the given color (solid fill). 
  /// 
  /// @param def
  ///
  /// @param mat
  ///
  /// @param color
  ///
  /// @param pixel_scale
  ///
  virtual void draw_glyph(shape_character_def *def,
    const matrix& mat,
    rgba color,
    float pixel_scale) = 0;
    
  /// The render handler can choose if it wishes to use textured glyphs 
  /// (pre-computed bitmaps which are used for small text sizes) or if 
  /// draw_glyph() should be used in any case. When glyph textures are not
  /// desired, then draw_bitmap() is never called in the *current* version.  
  virtual bool allow_glyph_textures() = 0;
    
    
  /// This function returns the color at any position in the stage. It is used
  /// for automatic testing only, it should not be used for anything else!
  /// world_x and world_y are world coordinates (twips) and the color of the
  /// nearest pixel is returned.
  virtual void get_pixel(rgba& /*color_return*/, float /*world_x*/, 
    float /*world_y*/) {
    
    log_msg("get_pixel() not implemented for this renderer");
    assert(0);    
    
  }
    
    
protected:

  // Cached fill style list with just one entry used for font rendering
  std::vector<fill_style>	m_single_fill_styles;
  
  // Dummy line styles list without entries (do not add anything!!)
  std::vector<line_style>	m_dummy_line_styles;
  
  // Dummy, neutral color transformation (do not change!!)
  cxform m_neutral_cxform;
  
  // Sets m_single_fill_styles to one solid fill with the given color 
  void need_single_fill_style(const rgba& color){
  
    if (m_single_fill_styles.size() == 0) {  
      fill_style dummy;
    
      m_single_fill_styles.push_back(dummy);
    }
    
    m_single_fill_styles[0].set_color(color);
  
  } //need_single_fill_style

}; // class render_handler



}	// namespace gnash

#endif // RENDER_HANDLER_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
