// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#ifndef RENDER_HANDLER_H
#define RENDER_HANDLER_H

/// \page render_handler_intro Render handler introduction
///
/// Information for writing new render handlers.
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
/// After advancing the root movie (see gnash::Gui::advance_movie) it is checked
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
/// detect these and completely avoid calling any rendering function.
/// 
/// Of course, the most critical part is detection of changes. There is a 
/// method gnash::character::set_invalidated() which gets called whenever a
/// critical property of a instance gets updated, like when it changes
/// position, for example.
/// It's really important to *always* call set_invalidated() *before* 
/// any call that changes the character instance in a visible way.
/// 
/// Even if no renderer really uses this information it has effects when
/// skipping unchanged frames. If necessary, this feature can be switched
/// off easily in gui.cpp (maybe using a runtime option?).
///
/// Note the updated region is only passed to the gnash::Gui, which is itself 
/// responsible of informing the renderer (see gnash::Gui::set_invalidated_region).
/// This is because it's pointless
/// to have a renderer which updates only a small part of the stage when
/// the GUI shows it all since the area around the region is undefined.
/// However, there can be a GUI which supports update regions without needing
/// the renderer to do so (for example, to save time during blitting).
/// The GUI can also completely ignore the region information. 
///
/// It's also importanto to note that the bounds passed to the GUI are just
/// a hint and the GUI /is/ allowed to further process and alter the information
/// in any way.
/// 
/// As for the integer/float discussion: I used rect (floats) because all
/// the bounds calculation involves floats anyway and so it's probably
/// faster than converting between ints and floats all the way.


#include "dsodefs.h" // for DSOEXPORT

#include "shape_character_def.h"  
#include "generic_character.h"
#include "Range2d.h"


// Forward declarations.
namespace gnash {
    class bitmap_info;
    class rect;
    class rgba;
    class matrix;
    class cxform;

    class shape_character_def;
    class generic_character;

    // @@ forward decl to avoid including base/image.h; TODO change the
    // render_handler interface to not depend on these classes at all.
    namespace image { class ImageBase; class rgb; class rgba; }
}

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
  enum antialias_method
  {
    ANTIALIAS_DEFAULT, // Whatever the renderer prefers
    ANTIALIAS_NONE,    // Disabled

    ANTIALIAS_MULTISAMPLE, // OpenGL ARB_multisample extension
    ANTIALIAS_ACCUM,       // OpenGL accumulation buffer

    ANTIALIAS_GRAY        // Cairo single-color antialiasing
  };
 
  virtual ~render_handler() {}

  // Your handler should return these with a ref-count of 0.  (@@ is that the right policy?)

  /// \brief
  /// Given an image, returns a pointer to a bitmap_info class
  /// that can later be passed to fill_styleX_bitmap(), to set a
  /// bitmap fill style.
  virtual bitmap_info*  create_bitmap_info_rgb(image::ImageRGB* im) = 0;

  /// \brief
  /// Given an image, returns a pointer to a bitmap_info class
  /// that can later be passed to fill_style_bitmap(), to set a
  /// bitmap fill style.
  //
  /// This version takes an image with an alpha channel.
  ///
  virtual bitmap_info*  create_bitmap_info_rgba(image::ImageRGBA* im) = 0;

  /// Delete the given bitmap info class.
  virtual void  delete_bitmap_info(bitmap_info* bi) = 0;

  /// The different video frame formats
  enum video_frame_format
  {
    NONE,
    YUV,
    RGB
  };

  /// Returns the format the current renderer wants videoframes in.
  virtual int videoFrameFormat() = 0;
  
  /// Draws a video frame. 
  //
  /// The frame has already been decoded and is available in the format
  /// specified by videoFrameFormat().    
  ///         
  /// @param frame The RGB or YUV video buffer frame.
  ///   Ownership of the buffer is left to the caller.
  ///
  /// @param mat The matrix with world coordinates used to retrieve the x
  ///   and y coordinate of the video object. The scaling of the matrix only
  ///   refers to the Flash instance, *not* to the video inside that instance.
  ///   When a video object is placed on the stage and the loaded video is
  ///   smaller, then the matrix is still an "identity matrix". However, if
  ///   the video object is scaled via ActionScript, for example, then the
  ///   matrix will change. This means the renderer has to find the correct
  ///   scaling for the video inside the bounds.                                
  ///
  /// @param bounds The minX/minY fields of this rect are always zero. 
  ///   The width and height determine the size of the Flash video instance
  ///   on the stage (in TWIPS) prior to matrix transformations.         
  ///
  virtual void drawVideoFrame(image::ImageBase* frame, const matrix* mat, const rect* bounds) = 0;

  /// Sets the update region (called prior to begin_display).
  //
  /// The renderer 
  /// might do clipping and leave the region outside these bounds unchanged,
  /// but he is allowed to change them if that makes sense. After rendering
  /// a frame the area outside the invalidated region can be undefined and 
  /// is not used. 
  ///
  /// It is not required for all renderers.
  /// Parameters are world coordinates (TWIPS).
  ///
  /// For more info see page \ref region_update.
  ///
  virtual void set_invalidated_region(const rect& /*bounds*/) {    
    // implementation is optional    
  }

  virtual void set_invalidated_regions(const InvalidatedRanges& /*ranges*/) {    
    // implementation is optional    
  }
  
  /// Converts world coordinates to pixel coordinates
  virtual geometry::Range2d<int> world_to_pixel(const rect& worldbounds) = 0;
  
  /// Converts pixel coordinates to world coordinates (TWIPS)
  virtual point pixel_to_world(int x, int y) = 0;
  
  virtual geometry::Range2d<float> pixel_to_world(const geometry::Range2d<int>& pixelbounds)
  {
    point topleft     = pixel_to_world(pixelbounds.getMinX(), pixelbounds.getMinY());
    point bottomright = pixel_to_world(pixelbounds.getMaxX(), pixelbounds.getMaxY());
    
    return geometry::Range2d<float> (topleft.x, topleft.y, 
      bottomright.x, bottomright.y);
  }
  
  virtual geometry::Range2d<int> world_to_pixel(const geometry::Range2d<float>& worldbounds)
  {
      if ((worldbounds.isNull() || worldbounds.isWorld()))  
        return worldbounds;

    return world_to_pixel(rect(worldbounds.getMinX(), worldbounds.getMinY(),
                               worldbounds.getMaxX(), worldbounds.getMaxY()));  
  }
  
    
  /// Bracket the displaying of a frame from a movie.
  //
  /// Set up to render a full frame from a movie and fills the
  /// background. Sets up necessary transforms, to scale the
  /// movie to fit within the given dimensions.  Call
  /// end_display() when you're done.
  ///
  /// The rectangle (viewport_x0, viewport_y0, viewport_x0 +
  /// viewport_width, viewport_y0 + viewport_height) defines the
  /// window coordinates taken up by the movie.
  ///
  /// The rectangle (x0, y0, x1, y1) defines the pixel
  /// coordinates of the movie that correspond to the viewport
  /// bounds.
  ///
  virtual void  begin_display(
    const rgba& background_color,
    int viewport_x0, int viewport_y0,
    int viewport_width, int viewport_height,
    float x0, float x1, float y0, float y1) = 0;

  virtual void  end_display() = 0;
    
  /// Draw a line-strip directly, using a thin, solid line.
  //
  /// Can be used to draw empty boxes and cursors.
  ///
  /// @coords an array of 16-bit signed integer coordinates. Even indices
  ///         (and 0) are x coordinates, while uneven ones are y coordinates.
  ///
  /// @vertex_count the number of x-y coordinates (vertices).
  ///
  /// @color the color to be used to draw the line strip.
  ///
  /// @mat the matrix to be used to transform the vertices.
  virtual void  draw_line_strip(const boost::int16_t* coords, int vertex_count,
      const rgba& color, const matrix& mat) = 0;
    
  /// Draw a simple, solid filled polygon with a thin (~1 pixel) outline.
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
  /// When masked==false, then any potential mask currently active will be
  /// ignored, otherwise it is respected.
  ///
  virtual void  draw_poly(const point* corners, size_t corner_count, 
    const rgba& fill, const rgba& outline, const matrix& mat, bool masked) = 0;
    
    
  /// Set line and fill styles for mesh & line_strip rendering.
  enum bitmap_wrap_mode
  {
    WRAP_REPEAT,
    WRAP_CLAMP
  };
    
  virtual void  set_antialiased(bool enable) = 0;
    
  ///@{ Masks
  ///
  /// Masks are defined by drawing calls enclosed by begin_submit_mask()
  /// and end_submit_mask(). Between these two calls, no drawing is to
  /// occur. The shapes rendered between the two calls define the
  /// visible region of the mask. Graphics that are irrelevant in the
  /// context of a mask (lines and fill styles, for example) should be
  /// ignored. After use, disable_mask() is called to remove the mask.
  ///
  /// Masks may be nested. That is, end_submit_mask() may be followed
  /// by a call to begin_submit_mask(). The resulting mask shall be an
  /// intersection of the previously created mask. disable_mask() shall
  /// result in the disabling or destruction of the last created mask.
  virtual void begin_submit_mask() = 0;
  virtual void end_submit_mask() = 0;
  virtual void disable_mask() = 0;
  ///@}
  
  /// \brief
  /// Draws the given character definition with the stateful properties
  /// of the given instance.
  //
  /// Normally this does not need to be re-implemented in 
  /// render handler implementations. Instead, see the version without
  /// character instance.
  ///
  virtual void draw_shape_character(shape_character_def *def, 
    character *inst)
  {
    // check if the character needs to be rendered at all
    rect cur_bounds;

    cur_bounds.expand_to_transformed_rect(inst->get_world_matrix(), 
        def->get_bound());
        
    if (!bounds_in_clipping_area(cur_bounds))
    {
        return; // no need to draw
    }    

    // TODO: I don't like that there is a draw_shape_character() version with
    // arbitrary fill and line styles as this may break caching...

    // render the character
    draw_shape_character(def, 
        inst->get_world_matrix(), 
        inst->get_world_cxform(),
        def->get_fill_styles(),
        def->get_line_styles());
  }
  
  /// \brief
  /// Checks if the given bounds are (partially) in the current drawing clipping
  /// area.
  //
  /// A render handler implementing invalidated bounds should implement
  /// this method to avoid rendering of characters that are not visible anyway.
  /// By default this method always returns true, which will ensure correct
  /// rendering. If possible, it should be re-implemented by the renderer 
  /// handler for better performance.
  /// 'bounds' contains TWIPS coordinates.
  ///
  /// TODO: Take a Range2d<T> rather then a gnash::rect ?
  ///       Would T==int be good ? TWIPS as integer types ?
  ///
  /// See also gnash::renderer::bounds_in_clipping_area
  ///
  virtual bool bounds_in_clipping_area(const rect& bounds) {
    return bounds_in_clipping_area(bounds.getRange());
  }
  
  virtual bool bounds_in_clipping_area(const InvalidatedRanges& ranges)
  {
    for (unsigned int rno=0; rno<ranges.size(); rno++) 
    {
      if (bounds_in_clipping_area(ranges.getRange(rno)))
        return true;
    }
        
    return false;
  }
  
  virtual bool bounds_in_clipping_area(const geometry::Range2d<float>& /*bounds*/) {
    return true;
  }

  /// \brief
  /// Draws the given character definition with the given transformations and
  /// styles. 
  virtual void draw_shape_character(shape_character_def *def, 
    const matrix& mat,
    const cxform& cx,
    const std::vector<fill_style>& fill_styles,
    const std::vector<line_style>& line_styles) = 0;
    
  /// \brief
  /// Draws a glyph (font character).
  //
  /// Glyphs are defined just like shape characters with the difference that
  /// they do not have any fill or line styles.
  /// Instead, the shape must be drawn using the given color (solid fill).
  /// Please note that although the glyph paths may indicate subshapes,
  /// the renderer is to ignore that information.
  /// 
  /// @param def
  ///
  /// @param mat
  ///
  /// @param color
  virtual void draw_glyph(shape_character_def *def, const matrix& mat,
    const rgba& color) = 0;

  /// This function returns the color at any position in the stage. It is used
  /// for automatic testing only, it should not be used for anything else!
  /// x and y are pixel coordinates (<0 won't make any sense) and the color of 
  /// the nearest pixel is returned.
  /// The function returns false when the coordinates are outside the 
  /// main frame buffer.
  virtual bool getPixel(rgba& /*color_return*/, int /*x*/, int /*y*/)
  {

    log_debug("getPixel() not implemented for this renderer");
    abort();    
    return false; // avoid compiler warning    
  }
  
  
  /// Returns the average RGB color for a square block on the stage. The 
  /// width and height of the block is defined by "radius" and x/y refer
  /// to the center of the block. radius==1 equals getPixel() and radius==0
  /// is illegal. For even "radius" values, the center point is not exactly
  /// defined. 
  /// The function returns false when at least one pixel of the block was
  /// outside the main frame buffer. In that case the value in color_return
  /// is undefined.
  /// This implementation is provided for simplicity. Renderers should
  /// implement a specialized version for better performance.
  virtual bool getAveragePixel(rgba& color_return, int x, int y, 
    unsigned int radius)
  {
  
    assert(radius>0); 
  
    // optimization:
    if (radius==1)
      return getPixel(color_return, x, y);
  
    unsigned int r=0, g=0, b=0, a=0;
    
    x -= radius/2;
    y -= radius/2;
    
    int xe = x+radius;
    int ye = y+radius;

    rgba pixel;
    
    for (int yp=y; yp<ye; yp++)
    for (int xp=x; xp<xe; xp++)
    {
      if (!getPixel(pixel, xp, yp))
        return false;
        
      r += pixel.m_r;      
      g += pixel.m_g;      
      b += pixel.m_b;      
      a += pixel.m_a;      
    }
    
    int pcount = radius*radius; 
    color_return.m_r = r / pcount; 
    color_return.m_g = g / pcount; 
    color_return.m_b = b / pcount; 
    color_return.m_a = a / pcount; 
    
    return true;
  }
  
  
  /// \brief
  /// Initializes the renderer for off-screen rendering used by the  
  /// testsuite.
  ///
  /// This is a special function used for testcases ONLY. It is used by
  /// MovieTester to prepare the renderer for off-screen rendering 
  /// without any GUI. The renderer is responsible to do all required
  /// steps so that rendering is possible after the call. This may mean
  /// that the renderer allocates memory for the given stage size.
  /// 
  /// The function returns false when the renderer is not able to do
  /// off-screen rendering (default).
  ///
  /// Note the function may be called again afterwards, resizing the stage.
  /// Any number of calls to this function is possible and the renderer
  /// is responsible to resize any buffer instead of wasting memory. 
  ///
  /// @param width stage width in pixels
  ///
  /// @param height stage height in pixels
  virtual bool initTestBuffer(unsigned /*width*/, unsigned /*height*/)
  {
    return false;
  }

  /// Return color depth (bits per pixel) or 0 if unknown/unimplemented.
  //
  /// Default implementation returns 0 (unknown).
  ///
  /// TODO: this should be a pure abstract function, just don't want
  ///       to scan ogl and cairo backend for an implementation *now*
  ///       but would be needed for automated testing... Quinn, can you help ?
  ///
  virtual unsigned int getBitsPerPixel() const
  {
    return 0;
  }
  
  /// Sets the x/y scale for the movie  
  virtual void set_scale(float /*xscale*/, float /*yscale*/) {
    // nop
  }

  /// Sets the x/y offset for the movie  
  virtual void set_translation(float /*xoff*/, float /*yoff*/) {
    // nop
  }

protected:

  /// Cached fill style list with just one entry used for font rendering
  std::vector<fill_style> m_single_fill_styles;

  /// Dummy line styles list without entries (do not add anything!!)
  std::vector<line_style> m_dummy_line_styles;

  /// Dummy, neutral color transformation (do not change!!)
  cxform m_neutral_cxform;

  /// Sets m_single_fill_styles to one solid fill with the given color 
  void need_single_fill_style(const rgba& color)
  {

    if (m_single_fill_styles.size() == 0)
    { 
      fill_style dummy;
      m_single_fill_styles.push_back(dummy);
    }

    m_single_fill_styles[0].set_color(color);

  } 

}; // class render_handler



} // namespace gnash

#endif // RENDER_HANDLER_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
