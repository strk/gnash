// 
//     Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//     Foundation, Inc
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef RENDER_HANDLER_H
#define RENDER_HANDLER_H

/// \page Renderer_intro Render handler introduction
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
///    fill create two paths (one for each rectangle) and one fill style.
///    Assume both paths come in clockwise order, then the outer rectangle
///    will have fillstyle0=0 and fillstyle1=1. The inner rectangle will have 
///    fillstyle0=1 and fillstyle1=0.
///
/// \code
///            +--------------------------------+
///            |XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX|
///            |XXX+------------------------+XXX|
///            |XXX|                        |XXX|
///            |XXX+------------------------+XXX|
///            |XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX|
///            +--------------------------------+
/// \endcode
///
///  - A rectangle is divided vertically in two halves, both having different
///    colors:
///
/// \code
///            +-------A-------+-------B--------+
///            |...............|################|
///            A...............C################B
///            |...............|################|
///            +-------A-------+-------B--------+
/// \endcode
///
///        Flash will probably produce three paths (A,B,C) and two fill styles.
///        Paths "A" and "B" will have just one fill style (fillstyle1 will be 
///        zero) while path "C" (which contains only one straight line!!) will
///        have two fill styles. To be exact the horizontal edges would not even
///        be necessary to render this shape (for a scanline based renderer) but 
///        they are necessary then the character is about to be rotated.
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
/// As for the integer/float discussion: I used SWFRect (floats) because all
/// the bounds calculation involves floats anyway and so it's probably
/// faster than converting between ints and floats all the way.


#include <vector>
#include <boost/noncopyable.hpp>

#include "dsodefs.h" // for DSOEXPORT

#include "boost/shared_array.hpp"
#include "boost/scoped_ptr.hpp"
#include "GnashEnums.h" 
#include "Range2d.h"
#include "Point2d.h"
#include "RGBA.h"
#include "log.h"
#include "snappingrange.h"
#include "SWFRect.h"

// Forward declarations.
namespace gnash {
    class IOChannel;
    class CachedBitmap;
    class rgba;
    class Transform;
    class SWFMatrix;
    class FillStyle;
    class LineStyle;
    class Shape;
    class MorphShape;

    // XXX: GnashImageProxy (delayed image rendering)
    class GnashVaapiImageProxy;

    namespace SWF {
        class ShapeRecord;
    }
    namespace image {
        class GnashImage;
    }
}

namespace gnash {

/// Base class for render handlers.
//
/// You must define a subclass of Renderer, and pass an
/// instance to the core (RunResources) *before* any SWF parsing begins.
///
/// For more info see page \ref Renderer_intro.
class DSOEXPORT Renderer : boost::noncopyable
{
public:

    Renderer(): _quality(QUALITY_HIGH) { }
    
    virtual ~Renderer() {}

    /// Return a description of this renderer.
    virtual std::string description() const = 0;

    /// ==================================================================
    /// Interfaces for adjusting renderer output.
    /// ==================================================================

    /// Sets the x/y scale for the movie    
    virtual void set_scale(float /*xscale*/, float /*yscale*/) {} 

    /// Sets the x/y offset for the movie in pixels. This applies to all
    /// graphics drawn except the background, which must be drawn for the
    /// entire canvas, regardless of the translation.
    virtual void set_translation(float /*xoff*/, float /*yoff*/) {}

    void setQuality(Quality q) { _quality = q; }
        
    /// ==================================================================
    /// Caching utitilies for core.
    /// ==================================================================
    
    /// \brief
    /// Given an image, returns a pointer to a CachedBitmap class
    /// that can later be passed to FillStyleX_bitmap(), to set a
    /// bitmap fill style.
    virtual CachedBitmap *
        createCachedBitmap(std::auto_ptr<image::GnashImage> im) = 0;


    /// ==================================================================
    /// Rendering Interface.
    /// ==================================================================

    /// Draws a video frame.  
    //
    /// The frame has already been decoded and is available in RGB format only. 
    ///         
    /// @param frame The RGB video buffer frame.
    ///   Ownership of the buffer is left to the caller.
    ///
    /// @param mat The SWFMatrix with world coordinates used to retrieve the x
    ///   and y coordinate of the video object. The scaling of the SWFMatrix
    ///   only refers to the Flash instance, *not* to the video inside that
    ///   instance. When a video object is placed on the stage and the loaded
    ///   video is smaller, then the SWFMatrix is still an "identity
    ///   matrix". However, if the video object is scaled via ActionScript,
    ///   for example, then the SWFMatrix will change. This means the
    ///   renderer has to find the correct scaling for the video inside the
    ///   bounds.                                
    ///
    /// @param bounds The minX/minY fields of this SWFRect are always zero. 
    ///   The width and height determine the size of the Flash video instance
    ///   on the stage (in TWIPS) prior to SWFMatrix transformations.         
    ///
    virtual void drawVideoFrame(image::GnashImage* frame,
            const Transform& xform, const SWFRect* bounds, bool smooth) = 0;

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
    /// @mat the SWFMatrix to be used to transform the vertices.
    virtual void drawLine(const std::vector<point>& coords,
            const rgba& color, const SWFMatrix& mat) = 0;
        
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
    virtual void drawPoly(const point* corners, size_t corner_count, 
        const rgba& fill, const rgba& outline, const SWFMatrix& mat,
        bool masked) = 0;
        
    virtual void drawShape(const SWF::ShapeRecord& shape,
            const Transform& xform) = 0;
        
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
    virtual void drawGlyph(const SWF::ShapeRecord& rec, const rgba& color,
           const SWFMatrix& mat) = 0;

    /// Draw the current rendering buffer to an image file.
    //
    /// Although this can be done at any time during the rendering cycle
    /// without harmful side effects, it's advisable only to do it when 
    /// between advance() calls, when the frame is fully renderered.
    //
    /// @param io       The IOChannel to write to.
    /// @param type     The type of image output required (PNG, JPEG, GIF).
    ///                 Note that not all FileTypes are images: rendering
    ///                 to an FLV will not work.
    virtual void renderToImage(boost::shared_ptr<IOChannel> /*io*/,
        FileType /*type*/, int /*quality*/) const {

        log_debug(_("Rendering to image not implemented for this "
            "renderer"));
    }
        

    /// ==================================================================
    /// Prepare drawing area and other utilities
    /// ==================================================================
    
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
    virtual void set_invalidated_regions(const InvalidatedRanges& /*ranges*/)
    {        
    }

    /// ==================================================================
    /// Machinery for delayed images rendering (e.g. Xv with YV12 or VAAPI)
    /// ==================================================================

    ///@{
    typedef boost::shared_ptr<GnashVaapiImageProxy> RenderImage;
    typedef std::vector<RenderImage> RenderImages;

    // Get first render image
    virtual RenderImages::iterator getFirstRenderImage()
            { return _render_images.begin(); }
    virtual RenderImages::const_iterator getFirstRenderImage() const
            { return _render_images.begin(); }

    // Get last render image
    virtual RenderImages::iterator getLastRenderImage()
            { return _render_images.end(); }
    virtual RenderImages::const_iterator getLastRenderImage() const
            { return _render_images.end(); }
    
    ///@}
        
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
    
    /// ==================================================================
    /// Interface for querying the renderer.
    /// ==================================================================
    
    /// Converts world coordinates to pixel coordinates
    virtual geometry::Range2d<int> world_to_pixel(const SWFRect& worldbounds) = 0;
    
    /// Converts pixel coordinates to world coordinates (TWIPS)
    virtual point pixel_to_world(int x, int y) = 0;
    
    virtual geometry::Range2d<int> pixel_to_world(
                    const geometry::Range2d<int>& pixelbounds)
    {
        point topleft = pixel_to_world(
                        pixelbounds.getMinX(), pixelbounds.getMinY());
        point bottomright = pixel_to_world(
                        pixelbounds.getMaxX(), pixelbounds.getMaxY());
        
        return geometry::Range2d<int> (topleft.x, topleft.y, 
            bottomright.x, bottomright.y);
    }
    
    virtual geometry::Range2d<int> world_to_pixel(
                    const geometry::Range2d<int>& worldbounds)
    {
        if ((worldbounds.isNull() || worldbounds.isWorld())) return worldbounds;

	// We always get compiler warnings on casting floats to int
	// here, so we cast it ourselves to get rid of the warning
	// message. Note that in both cases this rounds the float to
	// an integer by dropping the decimal part.
        return world_to_pixel(SWFRect(static_cast<int>(worldbounds.getMinX()),
				   static_cast<int>(worldbounds.getMinY()),
				   static_cast<int>(worldbounds.getMaxX()),
				   static_cast<int>(worldbounds.getMaxY())));
    }
        
    /// \brief
    /// Checks if the given bounds are (partially) in the current drawing
    /// clipping area.
    //
    /// A render handler implementing invalidated bounds should implement
    /// this method to avoid rendering of characters that are not visible
    /// anyway.
    /// By default this method always returns true, which will ensure correct
    /// rendering. If possible, it should be re-implemented by the renderer 
    /// handler for better performance.
    /// 'bounds' contains TWIPS coordinates.
    ///
    /// TODO: Take a Range2d<T> rather then a gnash::SWFRect ?
    ///             Would T==int be good ? TWIPS as integer types ?
    ///
    /// See also gnash::renderer::bounds_in_clipping_area
    ///
    virtual bool bounds_in_clipping_area(const SWFRect& bounds) {
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
    
    virtual bool bounds_in_clipping_area(
            const geometry::Range2d<int>& /*bounds*/)
    {
        return true;
    }

#ifdef USE_TESTSUITE

        
    /// ==================================================================
    /// Interfaces for testing only. Disabled when the testsuite isn't built.
    /// ==================================================================


    /// This function returns the color at any position in the stage. It is used
    /// for automatic testing only, it should not be used for anything else!
    /// x and y are pixel coordinates (<0 won't make any sense) and the color of 
    /// the nearest pixel is returned.
    /// The function returns false when the coordinates are outside the 
    /// main frame buffer.
    virtual bool getPixel(rgba& /*color_return*/, int /*x*/, int /*y*/) const {

        log_debug("getPixel() not implemented for this renderer");
        abort();        
        return false; // avoid compiler warning        
    }

    void addRenderImage(boost::shared_ptr<GnashVaapiImageProxy> image) {
        _render_images.push_back(image);
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
        unsigned int radius) const
    {
    
        assert(radius>0); 
    
        // optimization:
        if (radius==1) return getPixel(color_return, x, y);
    
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
    virtual bool initTestBuffer(unsigned /*width*/, unsigned /*height*/) {
        return false;
    }

    /// Return color depth (bits per pixel) or 0 if unknown/unimplemented.
    //
    /// Default implementation returns 0 (unknown).
    ///
    /// TODO: this should be a pure abstract function, just don't want
    ///     to scan ogl and cairo backend for an implementation *now*
    ///     but would be needed for automated testing... Quinn, can you help ?
    virtual unsigned int getBitsPerPixel() const {
        return 0;
    }
    
#endif

    class External 
    {
    public:
        /// Prepare the renderer for external rendering
        //
        /// Note that all arguments except the background colour are useless
        /// outside the ogl renderer.
        External(Renderer& r, const rgba& c, int w = 0, int h = 0,
                float x0 = 0, float x1 = 0, float y0 = 0, float y1 = 0)
            :
            _r(r)
        {
            _r.begin_display(c, w, h, x0, x1, y0, y1);
        }

        ~External() {
            _r.end_display();
        }

    private:
        Renderer& _r;
    };
    
    class Internal 
    {
    public:

        /// Prepare the renderer for internal rendering
        Internal(Renderer& r, image::GnashImage& im)
            :
            _r(r),
            _ext(_r.startInternalRender(im))
        {
        }

        Renderer* renderer() const {
            return _ext;
        }

        ~Internal() {
            _r.endInternalRender();
        }

    private:
        Renderer& _r;
        Renderer* _ext;
    };

protected:

    /// Kept in parallel with movie_root's setting.
    Quality _quality;

    // Delayed imaged to render
    RenderImages _render_images;

private:

    /// Bracket the displaying of a frame from a movie.
    //
    /// Set up to render a full frame from a movie and fills the
    /// background. Sets up necessary transforms, to scale the
    /// movie to fit within the given dimensions.    Call
    /// end_display() when you're done.
    //
    /// Most of the arguments are only for the ogl renderer. See documentation
    /// in that class. Do not use these arguments for new renderers!
    virtual void begin_display(const rgba& background_color, 
                    int viewport_width, int viewport_height,
                    float x0, float x1, float y0, float y1) = 0;

    virtual void end_display() = 0;

    /// Setup the renderer to draw to an internal buffer.
    //
    /// Implementations are free to return a new renderer if they choose.
    //
    /// @return         0 if the renderer does not support this.
    virtual Renderer* startInternalRender(image::GnashImage& buffer) = 0;

    /// Finish internal rendering.
    //
    /// Any rendering after this function has been called must apply to the
    /// external buffer.
    virtual void endInternalRender() = 0;

}; 

} // namespace gnash

#endif 


// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
