// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

// Original version by Udo Giacomozzi and Hannes Mayr, 
// INDUNET GmbH (www.indunet.it)


/// A Renderer that uses the Anti-Grain Geometry Toolkit (antigrain.com)
/// and renders directly to a buffer (for example to the framebuffer). This 
/// backend is *completely* independent of any hardware. It can be used for
/// rendering to the Linux FrameBuffer device, or be blitted inside a 
/// window (regardless of what operating system). It should also be no problem
/// to render into a file...
/// This file uses *very* heavily templates and is optimized mainly for speed,
/// meaning that the compiler generates very, very, very much specialized 
/// code. That's good for speed but bloats up the resulting machine code. 

/*

Status
------

  outlines:
    solid             COMPLETE
    patterns          don't exist (they're converted at compile time by Flash!)
    widths            COMPLETE
    colors, alpha     COMPLETE
    cap styles        DONE, but end cap style is ignored
    join styles       COMPLETE
    no-close flag     COMPLETE        
  
    
  fills:
    solid fills       COMPLETE
    linear gradients  COMPLETE
    radial gradients  COMPLETE
    focal gradients   COMPLETE
    ext. spread modes COMPLETE
    linear RGB mode   COMPLETE
    bitmaps, tiled    COMPLETE
    bitmaps, clipped  COMPLETE
    bitmaps, smooth   COMPLETE
    bitmaps, hard     COMPLETE    
    color xform       COMPLETE
    
  fonts               COMPLETE
    
  masks               COMPLETE
  
  caching             NONE IMPLEMENTED
  
  video               COMPLETE
  
  Currently the renderer should be able to render everything correctly.
  
  
What could and should be /optimized/
------------------------------------  
  
  - EASY: Do not even start rendering shapes that are abviously out of the
    invalidated bounds!  
    
  - The alpha mask buffers (masks) are allocated and freed for each mask which
    results in many large-size buffer allocations during a second. Maybe this
    should be optimized.
    
  - Matrix-transformed paths (generated before drawing a shape) should be cached
    and re-used to avoid recalculation of the same coordinates.
    
  - Characters (or sprites) may be cached as bitmaps with alpha channel (RGBA).
    The mechanism could be automatically activated when the same character is
    being rendered the 3rd or 5th time in a row with the same transformations
    (regardless of the instance itself!). It's not a good idea to always
    render into a bitmap buffer because this eats up memory and adds an 
    additional pass in rendering (blitting the bitmap buffer). This may be
    tricky to implement anyway.
    
  - Masks are a very good candidate for bitmap caching as they do not change
    that often. With other words, the alpha mask should not be discarded after
    rendering and should be reused if possible.
    
  - there are also a few TODO comments in the code!
  
  
AGG resources
-------------
  http://www.antigrain.com/    
  http://haiku-os.org/node/86

*/

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "Renderer_agg.h" 

#include <vector>
#include <cmath>
#include <math.h> // We use round()!
#include <climits>
#include <functional>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wsign-compare"
#include <agg_rendering_buffer.h>
#include <agg_renderer_base.h>
#include <agg_pixfmt_gray.h>
#include <agg_pixfmt_rgb_packed.h>
#include <agg_color_rgba.h>
#include <agg_color_gray.h>
#include <agg_ellipse.h>
#include <agg_conv_transform.h>
#include <agg_trans_affine.h>
#include <agg_scanline_u.h>
#include <agg_scanline_bin.h>
#include <agg_scanline_p.h>
#include <agg_renderer_scanline.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_rasterizer_compound_aa.h>
#include <agg_span_allocator.h>
#include <agg_path_storage.h>
#include <agg_conv_curve.h>
#include <agg_conv_stroke.h>
#include <agg_renderer_primitives.h>
#include <agg_image_accessors.h>
#include <agg_alpha_mask_u8.h>
#pragma GCC diagnostic pop

#include "Renderer_agg_style.h"

#include "GnashEnums.h"
#include "CachedBitmap.h"
#include "RGBA.h"
#include "GnashImage.h"
#include "log.h"
#include "Range2d.h"
#include "swf/ShapeRecord.h" 
#include "GnashNumeric.h"
#include "SWFCxForm.h"
#include "FillStyle.h"
#include "Transform.h"

#ifdef HAVE_VA_VA_H
#include "GnashVaapiImage.h"
#include "GnashVaapiImageProxy.h"
#endif

#include "Renderer_agg_bitmap.h"

// Print a debugging warning when rendering of a whole character
// is skipped 
//#define GNASH_WARN_WHOLE_CHARACTER_SKIP


#include <boost/numeric/conversion/converter.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace gnash {

namespace {

class AlphaMask;

typedef std::vector<agg::path_storage> AggPaths;
typedef std::vector<geometry::Range2d<int> > ClipBounds;
typedef boost::ptr_vector<AlphaMask> AlphaMasks;
typedef std::vector<Path> GnashPaths;

// Note: this is here in case ::round doesn't exist. However, it's not
// advisable to check using ifdefs (as previously), because ::round is
// generally a function not a macro!
template<typename T>
T round(T t)
{
    return ::round(t);
}

template <class Rasterizer>
inline void applyClipBox(Rasterizer& ras, const geometry::Range2d<int>& bounds)
{
    assert(bounds.isFinite());
    ras.clip_box(static_cast<double>(bounds.getMinX()),
            static_cast<double>(bounds.getMinY()),
            static_cast<double>(bounds.getMaxX() + 1), 
            static_cast<double>(bounds.getMaxY() + 1)
            );  
}

/// Analyzes a set of paths to detect real presence of fills and/or outlines
/// TODO: This should be something the character tells us and should be 
/// cached. 
void
analyzePaths(const GnashPaths &paths, bool& have_shape,
    bool& have_outline)
{

    have_shape = false;
    have_outline = false;

    const int pcount = paths.size();

    for (int pno=0; pno<pcount; ++pno) {

        const Path &the_path = paths[pno];

        if ((the_path.m_fill0 > 0) || (the_path.m_fill1 > 0)) {
            have_shape = true;
            if (have_outline) return; // have both
        }

        if (the_path.m_line > 0) {
            have_outline = true;
            if (have_shape) return; // have both
        }
    }
}

class EdgeToPath
{

public:
    EdgeToPath(AggPaths::value_type& path, double shift = 0)
        :
        _path(path),
        _shift(shift)
    {}

    void operator()(const Edge& edge)
    {
        if (edge.straight()) {
            _path.line_to(twipsToPixels(edge.ap.x) + _shift, 
                          twipsToPixels(edge.ap.y) + _shift);
        }
        else {
            _path.curve3(twipsToPixels(edge.cp.x) + _shift, 
                     twipsToPixels(edge.cp.y) + _shift,
                     twipsToPixels(edge.ap.x) + _shift, 
                     twipsToPixels(edge.ap.y) + _shift);             
        }
    }

private:
    agg::path_storage& _path;
    const double _shift;
};

/// In-place transformation of Gnash paths to AGG paths.
class GnashToAggPath
{
public:

    GnashToAggPath(AggPaths& dest, double shift = 0)
        :
        _dest(dest),
        _it(_dest.begin()),
        _shift(shift)
    {
    }

    void operator()(const Path& in)
    {
        agg::path_storage& p = *_it;

        p.move_to(twipsToPixels(in.ap.x) + _shift, 
                  twipsToPixels(in.ap.y) + _shift);

        std::for_each(in.m_edges.begin(), in.m_edges.end(),
                EdgeToPath(p, _shift));
        ++_it;
    }

private:
    AggPaths& _dest;
    AggPaths::iterator _it;
    const double _shift;

};


/// Transposes Gnash paths to AGG paths, which can be used for both outlines
/// and shapes. Subshapes are ignored (ie. all paths are converted). Converts 
/// TWIPS to pixels on the fly.
inline void
buildPaths(AggPaths& dest, const GnashPaths& paths) 
{
    dest.resize(paths.size());
    std::for_each(paths.begin(), paths.end(), GnashToAggPath(dest, 0.05));
} 

// --- ALPHA MASK BUFFER CONTAINER ---------------------------------------------
// How masks are implemented: A mask is basically a full alpha buffer. Each 
// pixel in the alpha buffer defines the fraction of color values that are
// copied to the main buffer. The alpha mask buffer has 256 alpha levels per
// pixel, which is good as it allows anti-aliased masks. A full size buffer
// is allocated for each mask even if the invalidated bounds may be much 
// smaller. The advantage of this is that the alpha mask adaptor does not need
// to do any clipping which results in better performance.
// Masks can be nested, which means the intersection of all masks should be 
// visible (logical AND). To allow this we hold a stack of alpha masks and the 
// topmost mask is used itself as a mask to draw any new mask. When rendering 
// visible shapes only the topmost mask must be used and when a mask should not 
// be used anymore it's simply discarded so that the next mask becomes active 
// again.
// To be exact, Flash is a bit restrictive regarding to what can be a mask
// (dynamic text, shapes, ...) but our renderer can build a mask from
// anything we can draw otherwise (except lines, which are excluded 
// explicitly).    

class AlphaMask 
{

    typedef agg::renderer_base<agg::pixfmt_gray8> Renderer;
    typedef agg::alpha_mask_gray8 Mask;

public:

    AlphaMask(int width, int height)
        :
        _rbuf(0, width, height, width),
        _pixf(_rbuf),
        _rbase(_pixf),
        _amask(_rbuf),
        _buffer(new boost::uint8_t[width * height]())
    {
        _rbuf.attach(_buffer.get(), width, height, width);
    }
    
    void clear(const geometry::Range2d<int>& region)
    {
        if (region.isNull()) return;
        assert(region.isFinite());

        const agg::gray8 black(0);
                
        // region can't be world as it should be intersected with 
        // the visible SWFRect
        assert(!region.isWorld());

        unsigned int left = region.getMinX();
        unsigned int width = region.width() + 1;

        const unsigned int max_y = region.getMaxY();
        for (unsigned int y=region.getMinY(); y <= max_y; ++y) 
        {
             _pixf.copy_hline(left, y, width, black);
        }
    }
    
    Renderer& get_rbase() {
        return _rbase;
    }
    
    const Mask& getMask() const {
        return _amask;
    }    
    
private:

    // agg class to access the buffer
    agg::rendering_buffer _rbuf;
    
    // pixel access
    agg::pixfmt_gray8 _pixf;    
    
    // renderer base
    Renderer _rbase;
    
    // alpha mask
    Mask _amask;
    
    // in-memory buffer
    std::unique_ptr<boost::uint8_t[]> _buffer;
    
};

/// Class for rendering lines.
template<typename PixelFormat>
class LineRenderer
{
public:
    typedef agg::renderer_base<PixelFormat> BaseRenderer;
    typedef agg::renderer_scanline_aa_solid<BaseRenderer> Renderer;
    typedef agg::rasterizer_scanline_aa<> Rasterizer;
    typedef agg::conv_stroke<agg::path_storage> Stroke;

    LineRenderer(const ClipBounds& clipbounds, BaseRenderer& baseRenderer)
        :
        _clipbounds(clipbounds),
        _renderer(baseRenderer)
    {}

    template<typename ScanLine>
    void render(ScanLine& sl, Stroke& stroke, const rgba& color)
    {
        for (ClipBounds::const_iterator i = _clipbounds.begin(),
                e = _clipbounds.end(); i != e; ++i) {

            const ClipBounds::value_type& bounds = *i;
              
            applyClipBox<Rasterizer> (_ras, bounds);

            // The vectorial pipeline
            _ras.add_path(stroke);

            // Set the color and render the scanlines
            _renderer.color(agg::rgba8_pre(color.m_r, color.m_g, 
                        color.m_b, color.m_a));

            agg::render_scanlines(_ras, sl, _renderer);

        }
    }

private:

    const ClipBounds& _clipbounds;
    Rasterizer _ras;
    Renderer _renderer;

};

/// Class for rendering empty video frames.
//
/// Templated functions are used to allow using different types,
/// particularly for high and low quality rendering. 
//
/// At present, this is bound to the renderer's ClipBounds. In future it
/// may be useful to pass BlendMode, Cxform, and custom clipbounds as well
/// as a caller-provided rendering buffer. This also applies to the 
/// rest of the renderer API.
//
/// @param PixelFormat The format to render to.
template <typename PixelFormat>
class EmptyVideoRenderer
{

public:

    /// Render the pixels using this renderer
    typedef typename agg::renderer_base<PixelFormat> Renderer;
    typedef agg::rasterizer_scanline_aa<> Rasterizer;

    EmptyVideoRenderer(const ClipBounds& clipbounds)
        :
        _clipbounds(clipbounds)
    {}

    void render(agg::path_storage& path, Renderer& rbase,
        const AlphaMasks& masks)
    {
        if (masks.empty()) {
            // No mask active
            agg::scanline_p8 sl;
            renderScanlines(path, rbase, sl);
        }
        else {
            // Untested.
            typedef agg::scanline_u8_am<agg::alpha_mask_gray8> Scanline;
            Scanline sl(masks.back().getMask());
            renderScanlines(path, rbase, sl);
        }
    } 

private:

    template<typename Scanline>
    void renderScanlines(agg::path_storage& path, Renderer& rbase,
            Scanline& sl)
    {
        Rasterizer ras;
        agg::renderer_scanline_aa_solid<Renderer> ren_sl(rbase);

        for (ClipBounds::const_iterator i = _clipbounds.begin(),
            e = _clipbounds.end(); i != e; ++i)
        {
            const ClipBounds::value_type& cb = *i;
            applyClipBox<Rasterizer>(ras, cb);
            ras.add_path(path);

            const agg::rgba8 col(agg::argb8_packed(0));
            ren_sl.color(col);

            // we don't want premultiplied alpha (so, don't use blend functions)
            //agg::render_scanlines(ras, sl, ren_sl);
            if (ras.rewind_scanlines()) {
                sl.reset(ras.min_x(), ras.max_x());
                ren_sl.prepare();
                while (ras.sweep_scanline(sl)) {
                    //ren_sl.render(sl);
                    const int y = sl.y();
                    unsigned int num_spans = sl.num_spans();
                    typename Scanline::const_iterator span = sl.begin();

                    for (;;) {
                        const int x = span->x;
                        assert(span->len > 0); // XXX: check span->len < 0 case!
                        if (span->len > 0)
                            rbase.copy_hline(x, y, (unsigned)span->len, col);
                        else
                            rbase.copy_hline(x, y,
                                             (unsigned)(x - span->len - 1),
                                             col);
                        if (--num_spans == 0)
                            break;
                        ++span;
                    }
                }
            }
        }
    }

    const ClipBounds& _clipbounds;
};    

/// Class for rendering video frames.
//
/// Templated functions are used to allow using different types,
/// particularly for high and low quality rendering. 
//
/// At present, this is bound to the renderer's ClipBounds. In future it
/// may be useful to pass BlendMode, Cxform, and custom clipbounds as well
/// as a caller-provided rendering buffer. This also applies to the 
/// rest of the renderer API.
//
/// @param SourceFormat     The format of the video frame to be rendered
/// @param PixelFormat      The format to render to.
template <typename PixelFormat, typename SourceFormat = agg::pixfmt_rgb24_pre>
class VideoRenderer
{

public:

    /// Fixed types for video frame rendering.

    /// Render the pixels using this renderer
    typedef typename agg::renderer_base<PixelFormat> Renderer;
    typedef agg::span_interpolator_linear<> Interpolator;
    typedef agg::span_allocator<agg::rgba8> SpanAllocator;
    typedef agg::rasterizer_scanline_aa<> Rasterizer;
    
    // cloning image accessor is used to avoid disturbing pixels at
    // the edges for rotated video. 
    typedef agg::image_accessor_clone<SourceFormat> Accessor;

    /// Types used for different quality.
    //
    /// This (affects scaling) is only presently used when smoothing is
    /// requested in high quality.
    typedef agg::span_image_filter_rgb_nn<Accessor, Interpolator>
        LowQualityFilter;

    typedef agg::span_image_filter_rgb_bilinear<Accessor, Interpolator>
        HighQualityFilter;

    typedef agg::trans_affine Matrix;

    VideoRenderer(const ClipBounds& clipbounds, image::GnashImage& frame,
            Matrix& mat, Quality quality, bool smooth)
        :
        _buf(frame.begin(), frame.width(), frame.height(),
                frame.stride()),
        _pixf(_buf),
        _accessor(_pixf),
        _interpolator(mat),
        _clipbounds(clipbounds),
        _quality(quality),
        _smoothing(smooth)
    {}

    void render(agg::path_storage& path, Renderer& rbase,
            const AlphaMasks& masks)
    {
        switch (_quality)
        {
            case QUALITY_BEST:
            case QUALITY_HIGH:
                if (_smoothing) {
                    renderFrame<HighQualityFilter>(path, rbase, masks);
                }
                else renderFrame<LowQualityFilter>(path, rbase, masks);
                break;
            case QUALITY_MEDIUM:
            case QUALITY_LOW:
                // FIXME: Should this be still lower quality?
                renderFrame<LowQualityFilter>(path, rbase, masks);
                break;
        }
    }
    
private:

    /// Render a frame with or without alpha masks active.
    template<typename SpanGenerator>
    void renderFrame(agg::path_storage& path, Renderer& rbase,
            const AlphaMasks& masks)
    {
        SpanGenerator sg(_accessor, _interpolator);
        if (masks.empty()) {
            // No mask active
            agg::scanline_u8 sl;
            renderScanlines(path, rbase, sl, sg);
        }
        else {
            // Untested.
            typedef agg::scanline_u8_am<agg::alpha_mask_gray8> Scanline;
            Scanline sl(masks.back().getMask());
            renderScanlines(path, rbase, sl, sg);
        }
    } 

    template<typename Scanline, typename SpanGenerator>
    void renderScanlines(agg::path_storage& path, Renderer& rbase,
            Scanline& sl, SpanGenerator& sg)
    {
        Rasterizer _ras;
        for (ClipBounds::const_iterator i = _clipbounds.begin(),
            e = _clipbounds.end(); i != e; ++i)
        {
            const ClipBounds::value_type& cb = *i;
            applyClipBox<Rasterizer> (_ras, cb);

            _ras.add_path(path);

            agg::render_scanlines_aa(_ras, sl, rbase, _sa, sg);
        }
    }
    
    // rendering buffer is used to access the frame pixels here        
    agg::rendering_buffer _buf;

    const SourceFormat _pixf;
    
    Accessor _accessor;
         
    Interpolator _interpolator;
    
    SpanAllocator _sa;

    const ClipBounds& _clipbounds;

    /// Quality of renderering
    const Quality _quality;

    /// Whether smoothing is required.
    bool _smoothing;
};    
  

            
}


// --- RENDER HANDLER ----------------------------------------------------------
// The class is implemented using templates so that it supports any kind of
// pixel format. LUT (look up tables) are not supported, however.

// Real AGG handler
template <class PixelFormat>
class Renderer_agg : public Renderer_agg_base
{
  
public:

    std::string description() const {
        // TODO: make an effort to express pixel format
        return "AGG";
    }

    // Given an image, returns a pointer to a bitmap_info class
    // that can later be passed to FillStyleX_bitmap(), to set a
    // bitmap fill style.
    gnash::CachedBitmap* createCachedBitmap(std::unique_ptr<image::GnashImage> im)
    {        
        return new agg_bitmap_info(std::move(im));
    }

    virtual void renderToImage(std::shared_ptr<IOChannel> io,
            FileType type, int quality) const
    {
        image::ImageRGBA im(xres, yres);
        for (int x = 0; x < xres; ++x) {
            for (int y = 0; y < yres; ++y) {
                typename PixelFormat::color_type t = m_pixf->pixel(x, y);
                im.setPixel(x, y, t.r, t.g, t.b, t.a);
            }
        }
        
        image::Output::writeImageData(type, io, im, quality);
    }

    template<typename SourceFormat, typename Matrix>
    void renderVideo(image::GnashImage& frame, Matrix& img_mtx,
            agg::path_storage path, bool smooth)
    {

        // renderer base for the stage buffer (not the frame image!)
        renderer_base& rbase = *m_rbase;

        VideoRenderer<PixelFormat, SourceFormat> vr(_clipbounds, frame,
                img_mtx, _quality, smooth);

        // If smoothing is requested and _quality is set to HIGH or BEST,
        // use high-quality interpolation.
        vr.render(path, rbase, _alphaMasks);
    }

    void renderEmptyVideo(agg::path_storage path)
    {
        // renderer base for the stage buffer (not the frame image!)
        renderer_base& rbase = *m_rbase;

        EmptyVideoRenderer<PixelFormat> vr(_clipbounds);
        vr.render(path, rbase, _alphaMasks);
    }

    void drawVideoFrame(image::GnashImage* frame, const Transform& xform,
        const SWFRect* bounds, bool smooth)
    {
    
        // NOTE: Assuming that the source image is RGB 8:8:8
        // TODO: keep heavy instances alive accross frames for performance!
        // TODO: Maybe implement specialization for 1:1 scaled videos
        SWFMatrix mat = stage_matrix;
        mat.concatenate(xform.matrix);
        
        // compute video scaling relative to video object size
        double vscaleX = bounds->width() /
            static_cast<double>(frame->width());
        
        double vscaleY = bounds->height() /
            static_cast<double>(frame->height());
        
        // convert Gnash SWFMatrix to AGG SWFMatrix and scale down to
        // pixel coordinates while we're at it
        agg::trans_affine mtx(mat.a() / 65536.0, mat.b() / 65536.0, 
            mat.c() / 65536.0, mat.d() / 65536.0, mat.tx(), mat.ty());        
        
        // invert SWFMatrix since this is used for the image source
        mtx.invert();
        
        // Apply video scale
        mtx *= agg::trans_affine_scaling(1.0 / vscaleX, 1.0 / vscaleY);
        
        // make a path for the video outline
        point a, b, c, d;
        mat.transform(&a, point(bounds->get_x_min(), bounds->get_y_min()));
        mat.transform(&b, point(bounds->get_x_max(), bounds->get_y_min()));
        mat.transform(&c, point(bounds->get_x_max(), bounds->get_y_max()));
        mat.transform(&d, point(bounds->get_x_min(), bounds->get_y_max()));

        agg::path_storage path;
        path.move_to(a.x, a.y);
        path.line_to(b.x, b.y);
        path.line_to(c.x, c.y);
        path.line_to(d.x, d.y);
        path.line_to(a.x, a.y);

#ifdef HAVE_VA_VA_H
        if (frame->location() == image::GNASH_IMAGE_GPU) {
            RenderImage image;
            image.reset(new GnashVaapiImageProxy(
                            static_cast<GnashVaapiImage *>(frame),
                            a.x, a.y, c.x - a.x, c.y - a.y));
            _render_images.push_back(image);

            // clear video region with transparent color
            renderEmptyVideo(path);
            return;
        }
#endif

        switch (frame->type())
        {
            case image::TYPE_RGBA:
                renderVideo<agg::pixfmt_rgba32_pre>(*frame, mtx, path, smooth);
                break;
            case image::TYPE_RGB:
                renderVideo<agg::pixfmt_rgb24_pre>(*frame, mtx, path, smooth);
                break;
            default:
                log_error(_("Can't render this type of frame"));
                break;
        }

    } 

  // Constructor
  Renderer_agg(int bits_per_pixel)
      :
      xres(1),
      yres(1),
      bpp(bits_per_pixel),
      scale_set(false),
      m_drawing_mask(false)
  {
    // TODO: we really don't want to set the scale here as the core should
    // tell us the right values before rendering anything. However this is
    // currently difficult to implement. Removing the next call will
    // lead to an assertion failure in begin_display() because we check
    // whether the scale is known there.
    set_scale(1.0f, 1.0f);
  }   

  /// Initializes the rendering buffer. The memory pointed by "mem" is not
  /// owned by the renderer and init_buffer() may be called multiple times
  /// when the buffer size changes, for example. However, bits_per_pixel must
  /// remain the same. 
  /// rowstride is the size, in bytes, of one row.
  /// This method *must* be called prior to any other method of the class!
  void init_buffer(unsigned char *mem, int /*size*/, int x, int y, int rowstride)
  {
        assert(x > 0);
        assert(y > 0);

    xres    = x;
    yres    = y;
    
    m_rbuf.attach(mem, xres, yres, rowstride);

    // allocate pixel format accessor and renderer_base
    m_pixf.reset(new PixelFormat(m_rbuf));
    m_rbase.reset(new renderer_base(*m_pixf));  
    
    // by default allow drawing everywhere
    set_invalidated_region_world();
  }
  

  void begin_display(const gnash::rgba& bg,
      int /*viewport_width*/, int /*viewport_height*/,
      float /*x0*/, float /*x1*/, float /*y0*/, float /*y1*/)
  {
    assert(m_pixf.get());
    
    assert(scale_set);

    // Render images list is cleared here because the GUI may want
    // them for display after ::end_display()
    _render_images.clear();

    // clear the stage using the background color
    if ( ! _clipbounds.empty() )
    {
        const agg::rgba8& col = agg::rgba8_pre(bg.m_r, bg.m_g, bg.m_b, bg.m_a);
        for (ClipBounds::const_iterator i = _clipbounds.begin(),
                e = _clipbounds.end(); i!= e; ++i) 
        {
            clear_framebuffer(*i, col);
        }
    }
    
    // reset status variables
    m_drawing_mask = false;
  }
  
 
    virtual Renderer* startInternalRender(image::GnashImage& im) {
    
        std::unique_ptr<Renderer_agg_base> in;
    
        switch (im.type()) {
            case image::TYPE_RGB:
                in.reset(new Renderer_agg<typename RGB::PixelFormat>(24));
                break;
            case image::TYPE_RGBA:
                in.reset(new Renderer_agg<typename RGBA::PixelFormat>(32));
                break;
            default:
                std::abort();
        }
 
        const size_t width = im.width();
        const size_t height = im.height();
        const size_t stride = width * (im.type() == image::TYPE_RGBA ? 4 : 3);

        in->init_buffer(im.begin(), width * height, width, height, stride);
        _external.reset(in.release());
        return _external.get();
    }

    virtual void endInternalRender() {
        _external.reset();
    }

    // renderer_base.clear() does no clipping which clears the
    // whole framebuffer even if we update just a small portion
    // of the screen. The result would be still correct, but slower. 
    // This function clears only a certain portion of the screen, while /not/ 
    // being notably slower for a fullscreen clear. 
    void clear_framebuffer(const geometry::Range2d<int>& region,
        const agg::rgba8& color)
    {
        assert(region.isFinite());

        // add 1 to width since we have still to draw a pixel when 
        // getMinX==getMaxX     
        unsigned int width=region.width()+1;

        // <Udo> Note: We don't need to check for width/height anymore because
        // Range2d will take care that getMinX <= getMaxX and it's okay when
        // region.width()==0 because in that case getMinX==getMaxX and we have
        // still a pixel to draw. 

        const unsigned int left=region.getMinX();

        for (unsigned int y=region.getMinY(), maxy=region.getMaxY();
            y<=maxy; ++y) {
              m_pixf->copy_hline(left, y, width, color);
        }
    }

    // Clean up after rendering a frame. 
    void end_display()
    {
        if (m_drawing_mask) {
            log_debug("Warning: rendering ended while drawing a mask");
        }

        while (! _alphaMasks.empty()) {
            log_debug("Warning: rendering ended while masks "
                        "were still active");
            disable_mask();      
        }
    }

    // Draw the line strip formed by the sequence of points.
    void drawLine(const std::vector<point>& coords, const rgba& color,
            const SWFMatrix& line_mat)
    {

        assert(m_pixf.get());
        
        if (_clipbounds.empty()) return;
        if (coords.empty()) return;

        SWFMatrix mat = stage_matrix;
        mat.concatenate(line_mat);    

        LineRenderer<PixelFormat> lr(_clipbounds, *m_rbase);

        // -- create path --
        agg::path_storage path;

        typename LineRenderer<PixelFormat>::Stroke stroke(path);
        stroke.width(1);
        stroke.line_cap(agg::round_cap);
        stroke.line_join(agg::round_join);

        typedef std::vector<point> Points;
        
        // We've asserted that it has at least one element.
        Points::const_iterator i = coords.begin();

        point pnt;

        mat.transform(&pnt, *i);
        path.move_to(pnt.x, pnt.y);

        ++i;

        for (const Points::const_iterator e = coords.end(); i != e; ++i)
        {
            mat.transform(&pnt, *i);
            path.line_to(pnt.x, pnt.y);
        }

        if (_alphaMasks.empty()) {
            // No mask active
            agg::scanline_p8 sl;      
            lr.render(sl, stroke, color);
        }
        else {
            // Mask is active!
            typedef agg::scanline_u8_am<agg::alpha_mask_gray8> sl_type;
            sl_type sl(_alphaMasks.back().getMask());      
            lr.render(sl, stroke, color);
        }

    } 


    void begin_submit_mask()
    {
        // Set flag so that rendering of shapes is simplified (only solid fill) 
        m_drawing_mask = true;

        _alphaMasks.push_back(new AlphaMask(xres, yres));
        AlphaMask& new_mask = _alphaMasks.back();

        for (ClipBounds::const_iterator i = _clipbounds.begin(), 
                e = _clipbounds.end(); i != e; ++i) {
            new_mask.clear(*i);
        }

    }

    void end_submit_mask()
    {
        m_drawing_mask = false;
    }

    void disable_mask()
    {
        assert(!_alphaMasks.empty());
        _alphaMasks.pop_back();
    }
  

  void drawGlyph(const SWF::ShapeRecord& shape, const rgba& color,
          const SWFMatrix& mat) 
  {
    if (shape.subshapes().empty()) return;
    assert(shape.subshapes().size() == 1);
    
    // select relevant clipping bounds
    if (shape.getBounds().is_null()) {
        return;
    } 
    select_clipbounds(shape.getBounds(), mat);
    
    if (_clipbounds_selected.empty()) return; 
      
    GnashPaths paths;
    apply_matrix_to_path(shape.subshapes().front().paths(), paths, mat);

    // If it's a mask, we don't need the rest.
    if (m_drawing_mask) {
      draw_mask_shape(paths, false);
      return;
    }

    // convert gnash paths to agg paths.
    AggPaths agg_paths;    
    buildPaths(agg_paths, paths);
 
    std::vector<FillStyle> v(1, FillStyle(SolidFill(color)));

    // prepare style handler
    StyleHandler sh;
    build_agg_styles(sh, v, mat, SWFCxForm());
    
    draw_shape(paths, agg_paths, sh, false);
    
    // NOTE: Do not use even-odd filling rule for glyphs!
    
    // clear clipping ranges to ease debugging
    _clipbounds_selected.clear();
  }


  /// Fills _clipbounds_selected with pointers to _clipbounds members who
  /// intersect with the given character (transformed by mat). This avoids
  /// rendering of characters outside a particular clipping range.
  /// "_clipbounds_selected" is used by draw_shape() and draw_outline() and
  /// *must* be initialized prior to using those function.
  void select_clipbounds(const SWFRect& objectBounds, const SWFMatrix& source_mat)
  {
    
    SWFMatrix mat = stage_matrix;
    mat.concatenate(source_mat);
  
    _clipbounds_selected.clear();
    _clipbounds_selected.reserve(_clipbounds.size());

    if (objectBounds.is_null()) {
      log_debug("Warning: select_clipbounds encountered a character "
                  "definition with null bounds");
      return;
    }   

    SWFRect bounds;    
    bounds.set_null();
    bounds.expand_to_transformed_rect(mat, objectBounds);
    
    assert(bounds.getRange().isFinite());
    
    const int count = _clipbounds.size();
    for (int cno=0; cno<count; ++cno) {
          
      if (_clipbounds[cno].intersects(bounds.getRange())) 
        _clipbounds_selected.push_back(&_clipbounds[cno]);

    }  
  }
  
  void select_all_clipbounds() {
  
    if (_clipbounds_selected.size() == _clipbounds.size()) return; 
  
    _clipbounds_selected.clear();
    _clipbounds_selected.reserve(_clipbounds.size());
    
    for (ClipBounds::iterator i = _clipbounds.begin(),
            e = _clipbounds.end(); i != e; ++i)
    {
      _clipbounds_selected.push_back(&(*i));
    }
  }

    void drawShape(const SWF::ShapeRecord& shape, const Transform& xform)
    {
        // check if the character needs to be rendered at all
        SWFRect cur_bounds;

        cur_bounds.expand_to_transformed_rect(xform.matrix, shape.getBounds());
                
        if (!bounds_in_clipping_area(cur_bounds.getRange()))
        {
            return; // no need to draw
        }

        for (SWF::ShapeRecord::Subshapes::const_iterator it = shape.subshapes().begin(),
             end = shape.subshapes().end(); it != end; ++it ) {

            const SWF::ShapeRecord::FillStyles& fillStyles = it->fillStyles();
            const SWF::ShapeRecord::LineStyles& lineStyles = it->lineStyles();
            const SWF::ShapeRecord::Paths& paths = it->paths();

            // select ranges
            select_clipbounds(shape.getBounds(), xform.matrix);

            // render the DisplayObject's subshape.
            drawShape(fillStyles, lineStyles, paths, xform.matrix,
                      xform.colorTransform);
        }
    }

    void drawShape(const std::vector<FillStyle>& FillStyles,
        const std::vector<LineStyle>& line_styles,
        const std::vector<Path>& objpaths, const SWFMatrix& mat,
        const SWFCxForm& cx)
    {

        bool have_shape, have_outline;

        analyzePaths(objpaths, have_shape, have_outline);

        if (!have_shape && !have_outline) {
            // Early return for invisible character.
            return; 
        }

        GnashPaths paths;
        apply_matrix_to_path(objpaths, paths, mat);

        // Masks apparently do not use agg_paths, so return
        // early
        if (m_drawing_mask) {

            // Shape is drawn inside a mask, skip sub-shapes handling and
            // outlines
            draw_mask_shape(paths, false); 
            return;
        }

        AggPaths agg_paths;    
        AggPaths agg_paths_rounded;    

        // Flash only aligns outlines. Probably this is done at rendering
        // level.
        if (have_outline) {
            buildPaths_rounded(agg_paths_rounded, paths, line_styles);     
        }

        if (have_shape) {
            buildPaths(agg_paths, paths);
        }
        

        if (_clipbounds_selected.empty()) {
#ifdef GNASH_WARN_WHOLE_CHARACTER_SKIP
            log_debug("Warning: AGG renderer skipping a whole character");
#endif
            return; 
        }

        // prepare fill styles
        StyleHandler sh;
        if (have_shape) build_agg_styles(sh, FillStyles, mat, cx);


            if (have_shape) {
                draw_shape(paths, agg_paths, sh, true);        
            }
            if (have_outline)            {
                draw_outlines(paths, agg_paths_rounded,
                        line_styles, cx, mat);
            }

        // Clear selected clipbounds to ease debugging 
        _clipbounds_selected.clear();
    }

    /// Takes a path and translates it using the given SWFMatrix. The new path
    /// is stored in paths_out. Both paths_in and paths_out are expected to
    /// be in TWIPS.
    void apply_matrix_to_path(const GnashPaths &paths_in, 
          GnashPaths& paths_out, const SWFMatrix &source_mat) 
    {

        SWFMatrix mat;
        // make sure paths_out is also in TWIPS to keep accuracy.
        mat.concatenate_scale(20.0,  20.0);
        mat.concatenate(stage_matrix);
        mat.concatenate(source_mat);

        // Copy paths for in-place transform
        paths_out = paths_in;

        /// Transform all the paths using the matrix.
        std::for_each(paths_out.begin(), paths_out.end(), 
                std::bind(&Path::transform, std::placeholders::_1,
		    std::ref(mat)));
    } 

  // Version of buildPaths that uses rounded coordinates (pixel hinting)
  // for line styles that want it.  
  // This is used for outlines which are aligned to the pixel grid to avoid
  // anti-aliasing problems (a perfect horizontal line being drawn over two
  // lines and looking blurry). The proprietary player does this too.  
  //
  // Not all points are aligned, only those lines that:
  //   - are straight
  //   - are pure horizontal or vertical
  // Also, single segments of a path may be aligned or not depending on 
  // the segment properties (this matches MM player behaviour)
  //
  // This function - in contrast to buildPaths() - also checks noClose 
  // flag and automatically closes polygons.
  //
  // TODO: Flash never aligns lines that are wider than 1 pixel on *screen*,
  // but we currently don't check the width.  
  void buildPaths_rounded(AggPaths& dest, 
    const GnashPaths& paths, const std::vector<LineStyle>& line_styles)
  {

    const float subpixel_offset = 0.5f;
    
    const size_t pcount = paths.size();

    dest.resize(pcount);    
    
    for (size_t pno=0; pno<pcount; ++pno) {
      
      const Path& this_path = paths[pno];
      agg::path_storage& new_path = dest[pno];
      
      bool hinting=false, closed=false, hairline=false;
      
      if (this_path.m_line) {
        const LineStyle& lstyle = line_styles[this_path.m_line-1];
        
        hinting = lstyle.doPixelHinting();
        closed = this_path.isClosed() && !lstyle.noClose();
        
        // check if this line is a hairline ON SCREEN
        // TODO: we currently only check for hairlines per definiton, not
        // for thin lines that become hair lines due to scaling
        if (lstyle.getThickness()<=20)
          hairline = true;
      }
      
      float prev_ax = twipsToPixels(this_path.ap.x);
      float prev_ay = twipsToPixels(this_path.ap.y);  
      bool prev_align_x = true;
      bool prev_align_y = true;
      
      size_t ecount = this_path.m_edges.size();

      // avoid extra edge when doing implicit close later
      if (closed && ecount && 
        this_path.m_edges.back().straight()) --ecount;  
      
      for (size_t eno=0; eno<ecount; ++eno) {
        
        const Edge& this_edge = this_path.m_edges[eno];
        
        float this_ax = twipsToPixels(this_edge.ap.x);  
        float this_ay = twipsToPixels(this_edge.ap.y);  
        
        if (hinting || this_edge.straight()) {
        
          // candidate for alignment?
          bool align_x = hinting || (hairline && (prev_ax == this_ax));
          bool align_y = hinting || (hairline && (prev_ay == this_ay));
          
          if (align_x) 
            this_ax = round(this_ax);
          
          if (align_y)
            this_ay = round(this_ay);
          
          // first line?
          if (eno==0) {
          
            if (align_x) 
              prev_ax = round(prev_ax);
              
            if (align_y)
              prev_ay = round(prev_ay);
              
            new_path.move_to(prev_ax + subpixel_offset, 
              prev_ay + subpixel_offset);
            
          } else {
          
            // not the first line, but the previous anchor point
            // might belong to a curve and thus may not be aligned.
            // We need to have both anchors of this new line to be
            // aligned, so it may be neccesary to add a line
            if ((align_x && !prev_align_x) || (align_y && !prev_align_y)) {
            
              if (align_x) 
                prev_ax = round(prev_ax);
                
              if (align_y)
                prev_ay = round(prev_ay);
                
              new_path.line_to(prev_ax + subpixel_offset, 
                prev_ay + subpixel_offset);
              
            }
            
            // TODO: (minor flaw) Flash player never aligns anchor points
            // of curves, even if they are attached to straight vertical
            // or horizontal lines. It can be seen easily with rounded
            // rectangles, where the curves are never aligned and all 
            // straight lines are. AGG backend will align the curve anchor
            // point that follows the straight line. It's not a big problem
            // but it's not exact...
          
          }
        
          new_path.line_to(this_ax + subpixel_offset, 
            this_ay + subpixel_offset);
          
          prev_align_x = align_x;
          prev_align_y = align_y;  
          
          
        } else {
          
          // first line?
          if (eno==0) 
            new_path.move_to(prev_ax, prev_ay);
        
          // never align curves!
          new_path.curve3(
            twipsToPixels(this_edge.cp.x) + subpixel_offset, 
            twipsToPixels(this_edge.cp.y) + subpixel_offset,
            this_ax + subpixel_offset, 
            this_ay + subpixel_offset);
            
          prev_align_x = false;
          prev_align_y = false;  
            
        }
        
        prev_ax = this_ax;
        prev_ay = this_ay;    
        
      } //for
      
      if (closed)
        new_path.close_polygon();
    
    }
  } //buildPaths_rounded

    // Initializes the internal styles class for AGG renderer
    void build_agg_styles(StyleHandler& sh,
        const std::vector<FillStyle>& FillStyles,
        const SWFMatrix& fillstyle_matrix, const SWFCxForm& cx) {
    
        SWFMatrix inv_stage_matrix = stage_matrix;
        inv_stage_matrix.invert();
    
        const size_t fcount = FillStyles.size();

        for (size_t fno = 0; fno < fcount; ++fno) {
            const AddStyles st(stage_matrix, fillstyle_matrix, cx, sh,
                    _quality);
            boost::apply_visitor(st, FillStyles[fno].fill);
        } 
    } 
  

  /// Draws the given path using the given fill style and color transform.
  //
  /// Normally, Flash shapes are drawn using even-odd filling rule. However,
  /// for glyphs non-zero filling rule should be used (even_odd=0).
  /// Note the paths have already been transformed by the SWFMatrix and 
  /// 'subshape_id' defines which sub-shape should be drawn (-1 means all 
  /// subshapes).
  ///
  /// Note the *coordinates* in "paths" are not used because they are 
  /// already prepared in agg_paths. The (nearly ambiguous) "path" parameter
  /// is used to access other properties like fill styles and subshapes.   
  ///
  /// @param subshape_id
  ///    Defines which subshape to draw. -1 means all subshapes.
  ///
  void draw_shape(const GnashPaths &paths,
    const AggPaths& agg_paths,  
    StyleHandler& sh, bool even_odd) {
    
    if (_alphaMasks.empty()) {
    
      // No mask active, use normal scanline renderer
      
      typedef agg::scanline_u8 scanline_type;
      
      scanline_type sl;
      
      draw_shape_impl<scanline_type> (paths, agg_paths, 
        sh, even_odd, sl);
        
    } else {
    
      // Mask is active, use alpha mask scanline renderer
      
      typedef agg::scanline_u8_am<agg::alpha_mask_gray8> scanline_type;
      
      scanline_type sl(_alphaMasks.back().getMask());
      
      draw_shape_impl<scanline_type> (paths, agg_paths, 
        sh, even_odd, sl);
        
    }
    
  }
   
  /// Template for draw_shape(). Two different scanline types are suppored, 
  /// one with and one without an alpha mask. This makes drawing without masks
  /// much faster.  
  template <class scanline_type>
  void draw_shape_impl(const GnashPaths &paths,
    const AggPaths& agg_paths,
    StyleHandler& sh, bool even_odd, scanline_type& sl) {
    /*
    Fortunately, AGG provides a rasterizer that fits perfectly to the flash
    data model. So we just have to feed AGG with all data and we're done. :-)
    This is also far better than recomposing the polygons as the rasterizer
    can do everything in one pass and it is also better for adjacent edges
    (anti aliasing).
    Thank to Maxim Shemanarev for providing us such a great tool with AGG...
    */
    
    assert(m_pixf.get());
    
    assert(!m_drawing_mask);
    
    if ( _clipbounds.empty() ) return;

    // Target renderer
    renderer_base& rbase = *m_rbase;

    typedef agg::rasterizer_compound_aa<agg::rasterizer_sl_clip_int> ras_type;
    ras_type rasc;  // flash-like renderer

    agg::renderer_scanline_aa_solid<
      agg::renderer_base<PixelFormat> > ren_sl(rbase); // solid fills
    agg::span_allocator<agg::rgba8> alloc;  // span allocator (?)
    

    // activate even-odd filling rule
    if (even_odd)
      rasc.filling_rule(agg::fill_even_odd);
    else
      rasc.filling_rule(agg::fill_non_zero);
      
    
    for (unsigned int cno=0; cno<_clipbounds_selected.size(); ++cno) {
    
      const geometry::Range2d<int>* bounds = _clipbounds_selected[cno];
      
      applyClipBox<ras_type> (rasc, *bounds);
      
      // push paths to AGG
      const size_t pcount = paths.size();
  
      for (size_t pno=0; pno<pcount; ++pno) {
          
        const Path &this_path_gnash = paths[pno];
        agg::path_storage &this_path_agg = 
          const_cast<agg::path_storage&>(agg_paths[pno]);
        
        agg::conv_curve<agg::path_storage> curve(this_path_agg);        

        if ((this_path_gnash.m_fill0==0) && (this_path_gnash.m_fill1==0)) {
          // Skip this path as it contains no fill style
          continue;
        } 
        
        // Tell the rasterizer which styles the following path will use.
        // The good thing is, that it already supports two fill styles out of
        // the box. 
        // Flash uses value "0" for "no fill", whereas AGG uses "-1" for that. 
        rasc.styles(this_path_gnash.m_fill0-1, this_path_gnash.m_fill1-1);
                
        // add path to the compound rasterizer
        rasc.add_path(curve);
      
      }

      agg::render_scanlines_compound_layered(rasc, sl, rbase, alloc, sh);
    }
    
  } // draw_shape_impl




  // very similar to draw_shape but used for generating masks. There are no
  // fill styles nor subshapes and such. Just render plain solid shapes.
  void draw_mask_shape(const GnashPaths& paths, bool even_odd)
  {

    const AlphaMasks::size_type mask_count = _alphaMasks.size();
    
    if (mask_count < 2) {
    
      // This is the first level mask
      
      typedef agg::scanline_u8 scanline_type;
      
      scanline_type sl;
      
      draw_mask_shape_impl(paths, even_odd, sl);
        
    }
    else {
    
      // Woohoo! We're drawing a nested mask! Use the previous mask while 
      // drawing the new one, the result will be the intersection.
      
      typedef agg::scanline_u8_am<agg::alpha_mask_gray8> scanline_type;
      
      scanline_type sl(_alphaMasks[mask_count - 2].getMask());
      
      draw_mask_shape_impl(paths, even_odd, sl);
        
    }
    
  }
  
  
  template <class scanline_type>
  void draw_mask_shape_impl(const GnashPaths& paths, bool even_odd,
    scanline_type& sl) {
    
    typedef agg::pixfmt_gray8 pixfmt;
    typedef agg::renderer_base<pixfmt> renderer_base;
    
    assert(!_alphaMasks.empty());
    
    // dummy style handler
    typedef agg_mask_style_handler sh_type;
    sh_type sh;                   
       
    // compound rasterizer used for flash shapes
    typedef agg::rasterizer_compound_aa<agg::rasterizer_sl_clip_int> rasc_type;
    rasc_type rasc;
    

    // activate even-odd filling rule
    if (even_odd) rasc.filling_rule(agg::fill_even_odd);
    else rasc.filling_rule(agg::fill_non_zero);
      
    // push paths to AGG
    agg::path_storage path; 
    agg::conv_curve<agg::path_storage> curve(path);

    for (size_t pno=0, pcount=paths.size(); pno < pcount; ++pno) {

      const Path& this_path = paths[pno];

      path.remove_all();
      
      // reduce everything to just one fill style!
      rasc.styles(this_path.m_fill0==0 ? -1 : 0,
                  this_path.m_fill1==0 ? -1 : 0);
                  
      // starting point of path
      path.move_to(twipsToPixels(this_path.ap.x), 
                   twipsToPixels(this_path.ap.y));
    
      // Add all edges to the path.
      std::for_each(this_path.m_edges.begin(), this_path.m_edges.end(),
              EdgeToPath(path));
      
      // add to rasterizer
      rasc.add_path(curve);
    
    } // for path
    
    // renderer base
    renderer_base& rbase = _alphaMasks.back().get_rbase();
    
    // span allocator
    typedef agg::span_allocator<agg::gray8> alloc_type;
    alloc_type alloc; 
      
    
    // now render that thing!
    agg::render_scanlines_compound_layered (rasc, sl, rbase, alloc, sh);
        
  } // draw_mask_shape



  /// Just like draw_shapes() except that it draws an outline.
  void draw_outlines(const GnashPaths &paths,
    const AggPaths& agg_paths,
    const std::vector<LineStyle> &line_styles, const SWFCxForm& cx,
    const SWFMatrix& linestyle_matrix) {
    
    if (_alphaMasks.empty()) {
    
      // No mask active, use normal scanline renderer
      
      typedef agg::scanline_u8 scanline_type;
      
      scanline_type sl;
      
      draw_outlines_impl<scanline_type> (paths, agg_paths, 
        line_styles, cx, linestyle_matrix, sl);
        
    } else {
    
      // Mask is active, use alpha mask scanline renderer
      
      typedef agg::scanline_u8_am<agg::alpha_mask_gray8> scanline_type;
      
      scanline_type sl(_alphaMasks.back().getMask());
      
      draw_outlines_impl<scanline_type> (paths, agg_paths,
        line_styles, cx, linestyle_matrix, sl);
        
    }
    
  } //draw_outlines


  /// Template for draw_outlines(), see draw_shapes_impl().
  template <class scanline_type>
  void draw_outlines_impl(const GnashPaths &paths,
    const AggPaths& agg_paths,
    const std::vector<LineStyle> &line_styles, const SWFCxForm& cx, 
    const SWFMatrix& linestyle_matrix, scanline_type& sl) {
    
    assert(m_pixf.get());

    // Flash ignores lines in mask /definitions/ 
    if (m_drawing_mask) return;    
    
    if ( _clipbounds.empty() ) return;

    // TODO: While walking the paths for filling them, remember when a path
    // has a line style associated, so that we avoid walking the paths again
    // when there really are no outlines to draw...
    
    // use avg between x and y scale
    const float stroke_scale = (std::abs(linestyle_matrix.get_x_scale()) + 
       std::abs(linestyle_matrix.get_y_scale())) / 2.0f * get_stroke_scale();
    
    
    // AGG stuff
    typedef agg::rasterizer_scanline_aa<> ras_type; 
    ras_type ras;  // anti alias

    renderer_base& rbase = *m_rbase;

    agg::renderer_scanline_aa_solid<
      agg::renderer_base<PixelFormat> > ren_sl(rbase); // solid fills
      
    
    for (unsigned int cno=0; cno<_clipbounds_selected.size(); ++cno) {
    
      const geometry::Range2d<int>* bounds = _clipbounds_selected[cno];
          
      applyClipBox<ras_type> (ras, *bounds);
      
      for (size_t pno=0, pcount=paths.size(); pno<pcount; ++pno) {

        const Path& this_path_gnash = paths[pno];

        agg::path_storage &this_path_agg = 
          const_cast<agg::path_storage&>(agg_paths[pno]);
        
        if (this_path_gnash.m_line==0) {
          // Skip this path as it contains no line style
          continue;
        } 
        
        agg::conv_curve< agg::path_storage > curve(this_path_agg); // to render curves
        agg::conv_stroke< agg::conv_curve < agg::path_storage > > 
          stroke(curve);  // to get an outline

        const LineStyle& lstyle = line_styles[this_path_gnash.m_line-1];
          
        int thickness = lstyle.getThickness();
        if (!thickness) stroke.width(1); // hairline
        else if ( (!lstyle.scaleThicknessVertically()) && (!lstyle.scaleThicknessHorizontally()) )
        {
          stroke.width(twipsToPixels(thickness));
        }
        else
        {
          if ((!lstyle.scaleThicknessVertically()) ||
                  (!lstyle.scaleThicknessHorizontally()))
          {
             LOG_ONCE(log_unimpl(_("Unidirectionally scaled strokes in "
				   "AGG renderer (we'll scale by the "
				   "scalable one)")) );
          }
          stroke.width(std::max(1.0f, thickness*stroke_scale));
        }
        
        // TODO: support endCapStyle
        
        // TODO: When lstyle.noClose==0 and the start and end point matches,
        // then render a real join instead of the caps.

        switch (lstyle.startCapStyle()) {
          case CAP_NONE   : stroke.line_cap(agg::butt_cap); break; 
          case CAP_SQUARE : stroke.line_cap(agg::square_cap); break;          
          default : case CAP_ROUND : stroke.line_cap(agg::round_cap); 
        }
        
        switch (lstyle.joinStyle()) {
          case JOIN_BEVEL : stroke.line_join(agg::bevel_join); break;
          case JOIN_MITER : stroke.line_join(agg::miter_join); break;
          default : case JOIN_ROUND : stroke.line_join(agg::round_join);
        }
        
        stroke.miter_limit(lstyle.miterLimitFactor());
                
        ras.reset();
        ras.add_path(stroke);
        
        rgba color = cx.transform(lstyle.get_color());
        ren_sl.color(agg::rgba8_pre(color.m_r, color.m_g, color.m_b, color.m_a));       
                
        agg::render_scanlines(ras, sl, ren_sl);
        
      }
    
    
    }
      
  } // draw_outlines_impl


  
  /// Draws the given polygon.
  template <class scanline_type>
  void draw_poly_impl(const point* corners, size_t corner_count, const rgba& fill, 
    const rgba& outline, scanline_type& sl, const SWFMatrix& poly_mat) {
    
    assert(m_pixf.get());

    if (corner_count<1) return;
    
    if ( _clipbounds.empty() ) return;
    
    SWFMatrix mat = stage_matrix;
    mat.concatenate(poly_mat);
    
    typedef agg::rasterizer_scanline_aa<> ras_type;
    renderer_base& rbase = *m_rbase;

    ras_type ras;
    agg::renderer_scanline_aa_solid<
      agg::renderer_base<PixelFormat> > ren_sl(rbase);
      
    // -- create path --
    agg::path_storage path;
    point pnt, origin;
    
    
    // Note: The coordinates are rounded and 0.5 is added to snap them to the 
    // center of the pixel. This avoids blurring caused by anti-aliasing.
    
    // The default conversion of the boost converter is truncation.
    boost::numeric::converter<int,float> truncator;

    mat.transform(&origin, 
      point(truncator(corners[0].x), truncator(corners[0].y)));
    path.move_to(truncator(origin.x)+0.5, truncator(origin.y)+0.5);
    
    for (unsigned int i=1; i<corner_count; ++i) {
    
      mat.transform(&pnt, point(corners[i].x, corners[i].y));
        
      path.line_to(truncator(pnt.x)+0.5, truncator(pnt.y)+0.5);
    }
    
    // close polygon
    path.line_to(truncator(origin.x)+0.5, truncator(origin.y)+0.5);
    
    
    
    // -- render --
      
    // iterate through clipping bounds
    for (unsigned int cno=0; cno<_clipbounds.size(); ++cno) {
    
      const ClipBounds::value_type& bounds = _clipbounds[cno];         
      applyClipBox<ras_type> (ras, bounds);     
            
      
      // fill polygon
      if (fill.m_a>0) {
        ras.add_path(path);
        ren_sl.color(agg::rgba8_pre(fill.m_r, fill.m_g, fill.m_b, fill.m_a));
        
        agg::render_scanlines(ras, sl, ren_sl);
      }
      
      // draw outline
      if (outline.m_a>0) {
        agg::conv_stroke<agg::path_storage> stroke(path);
        
        stroke.width(1);
        
        ren_sl.color(agg::rgba8_pre(outline.m_r, outline.m_g, outline.m_b, outline.m_a));
        
        ras.add_path(stroke);
        
        agg::render_scanlines(ras, sl, ren_sl);
      }
    }
    
  } //draw_poly_impl
  
  
  void draw_poly(const std::vector<point>& corners, const rgba& fill, 
    const rgba& outline, const SWFMatrix& mat, bool masked) {
    
    if (masked && !_alphaMasks.empty()) {
    
      // apply mask
      
      typedef agg::scanline_u8_am<agg::alpha_mask_gray8> sl_type; 
      
      sl_type sl(_alphaMasks.back().getMask());
         
      draw_poly_impl<sl_type>(&corners.front(), corners.size(), fill, outline, sl, mat);       
    
    } else {
    
      // no mask
      
      typedef agg::scanline_p8 sl_type; // packed scanline (faster for solid fills)
      
      sl_type sl;
         
      draw_poly_impl<sl_type>(&corners.front(), corners.size(), fill, outline, sl, mat);
    
    }
    
  }


  inline float get_stroke_scale() {
    return (stage_matrix.get_x_scale() + stage_matrix.get_y_scale()) / 2.0f;
  }                      
  
  inline void world_to_pixel(int& x, int& y,
    float world_x, float world_y) const
  {
    // negative pixels seems ok here... we don't 
    // clip to valid range, use world_to_pixel(SWFRect&)
    // and Intersect() against valid range instead.
    point p(world_x, world_y);
    stage_matrix.transform(p);
    x = (int)p.x;
    y = (int)p.y;
  }

  geometry::Range2d<int> world_to_pixel(const SWFRect& wb) const
  {
      using namespace gnash::geometry;

    if ( wb.is_null() ) return Range2d<int>(nullRange);
    if ( wb.is_world() ) return Range2d<int>(worldRange);

    int xmin, ymin, xmax, ymax;

    world_to_pixel(xmin, ymin, wb.get_x_min(), wb.get_y_min());
    world_to_pixel(xmax, ymax, wb.get_x_max(), wb.get_y_max());

    return Range2d<int>(xmin, ymin, xmax, ymax);
  }
  
  point 
  pixel_to_world(int x, int y) const
  {
    point p(x, y);
    SWFMatrix mat = stage_matrix;
    mat.invert().transform(p);
    return p;    
  };
  
  void set_invalidated_region_world() {
    InvalidatedRanges ranges;
    ranges.setWorld();
    set_invalidated_regions(ranges);
  }
  
  virtual void set_invalidated_regions(const InvalidatedRanges& ranges) {
    using gnash::geometry::Range2d;
    
    int count=0;

    _clipbounds_selected.clear();
    _clipbounds.clear();    

    // TODO: cache 'visiblerect' and maintain in sync with
    //       xres/yres.
    Range2d<int> visiblerect;
    if ( xres && yres ) visiblerect = Range2d<int>(0, 0, xres-1, yres-1);
    
    for (size_t rno=0; rno<ranges.size(); ++rno) {
    
      const Range2d<int>& range = ranges.getRange(rno);

      Range2d<int> pixbounds = Renderer::world_to_pixel(range);
      
      geometry::Range2d<int> bounds = Intersection(pixbounds, visiblerect);
      
      if (bounds.isNull()) continue; // out of screen
      
      assert(bounds.isFinite());
      
      _clipbounds.push_back(bounds);
      
      ++count;
    }
    //log_debug(_("%d inv. bounds in frame"), count);
    
  }
  
  
  virtual bool bounds_in_clipping_area(const geometry::Range2d<int>& bounds)
    const {

    using gnash::geometry::Range2d;
  
    Range2d<int> pixbounds = Renderer::world_to_pixel(bounds);
    
    for (unsigned int cno=0; cno<_clipbounds.size(); ++cno) {  
      if (Intersect(pixbounds, _clipbounds[cno]))
        return true;
    }
    return false;
  }

  bool getPixel(rgba& color_return, int x, int y) const {
  
    if ((x<0) || (y<0) || (x>=xres) || (y>=yres))
      return false;

    agg::rgba8 color = m_pixf->pixel(x, y);    
    
    color_return.m_r = color.r;
    color_return.m_g = color.g;
    color_return.m_b = color.b;
    color_return.m_a = color.a;
    
    return true;
  }
  
  void set_scale(float new_xscale, float new_yscale) {
    
    scale_set=true;
    stage_matrix.set_identity();
    stage_matrix.set_scale(new_xscale/20.0f, new_yscale/20.0f);
  }

  void set_translation(float xoff, float yoff) {
    stage_matrix.set_translation(xoff, yoff);
  }

  virtual unsigned int getBytesPerPixel() const {
    return bpp/8;
  }  
  
private:  // private variables
    
    typedef agg::renderer_base<PixelFormat> renderer_base;

    // renderer base
    std::unique_ptr<renderer_base> m_rbase;
 
    // An external renderer.   
    std::unique_ptr<Renderer> _external;

    int xres;
    int yres;
    int bpp;  // bits per pixel
    gnash::SWFMatrix stage_matrix;  // conversion from TWIPS to pixels
    bool scale_set;

    agg::rendering_buffer m_rbuf;  

    std::unique_ptr<PixelFormat> m_pixf;

    /// clipping rectangle
    ClipBounds _clipbounds;
    std::vector< geometry::Range2d<int>* > _clipbounds_selected;

    // this flag is set while a mask is drawn
    bool m_drawing_mask; 

    // Alpha mask stack
    AlphaMasks _alphaMasks;
    
    /// Cached fill style list with just one entry used for font rendering
    std::vector<FillStyle> m_single_FillStyles;


};



// detect the endianess of the host (would prefer to NOT have this function
// here)
bool is_little_endian_host() {

  union {
    boost::uint16_t word;
    struct {
      boost::uint8_t b1;
      boost::uint8_t b2;
    } s;
  } u;
    
  u.s.b1 = 1;
  u.s.b2 = 2;
  
  return u.word == 0x0201;

}


DSOEXPORT Renderer_agg_base*  create_Renderer_agg(const char *pixelformat)
{

  if (!pixelformat) return NULL;

  if (is_little_endian_host())
    log_debug("Framebuffer pixel format is %s (little-endian host)", pixelformat);
  else
    log_debug("Framebuffer pixel format is %s (big-endian host)", pixelformat);
  
#ifdef PIXELFORMAT_RGB555  
  if (!strcmp(pixelformat, "RGB555"))
    return new Renderer_agg<agg::pixfmt_rgb555_pre> (16); // yep, 16!
  
  else
#endif   
#ifdef PIXELFORMAT_RGB565  
  if (!strcmp(pixelformat, "RGB565") || !strcmp(pixelformat, "RGBA16"))
    return new Renderer_agg<agg::pixfmt_rgb565_pre> (16); 
  else 
#endif   
#ifdef PIXELFORMAT_RGB24  
  if (!strcmp(pixelformat, "RGB24"))
    return new Renderer_agg<agg::pixfmt_rgb24_pre> (24);    
  else 
#endif   
#ifdef PIXELFORMAT_BGR24  
  if (!strcmp(pixelformat, "BGR24"))
    return new Renderer_agg<agg::pixfmt_bgr24_pre> (24);
  else 
#endif   
#ifdef PIXELFORMAT_RGBA32 
  if (!strcmp(pixelformat, "RGBA32"))
    return new Renderer_agg<agg::pixfmt_rgba32_pre> (32);
  else 
#endif   
#ifdef PIXELFORMAT_BGRA32  
  if (!strcmp(pixelformat, "BGRA32"))
    return new Renderer_agg<agg::pixfmt_bgra32_pre> (32);
#endif   
#ifdef PIXELFORMAT_RGBA32 
  if (!strcmp(pixelformat, "ARGB32"))
    return new Renderer_agg<agg::pixfmt_argb32_pre> (32);
  else 
#endif   
#ifdef PIXELFORMAT_BGRA32  
  if (!strcmp(pixelformat, "ABGR32"))
    return new Renderer_agg<agg::pixfmt_abgr32_pre> (32);
        
  else 
#endif
  {
      log_error(_("Unknown pixelformat: %s\n"), pixelformat);
    return NULL;
    //abort();
  }
  
  return NULL; // avoid compiler warning
}


DSOEXPORT const char *agg_detect_pixel_format(unsigned int rofs,
        unsigned int rsize, unsigned int gofs, unsigned int gsize,
        unsigned int bofs, unsigned int bsize, unsigned int bpp)
{
  
  if (!is_little_endian_host() && (bpp>=24)) {
  
    // Swap bits for big endian hosts, because the following tests assume
    // little endians. The pixel format string matches the bytes in memory.
    
    // This applies for 24 bpp and 32 bpp modes only because AGG uses arrays
    // in the premultiply() implementation for these modes. 16 bpp modes 
    // instead use bit shifting, which is transparent to host endianess.
    // See bug #22799.
    
    rofs = bpp - rofs - rsize;
    gofs = bpp - gofs - gsize;
    bofs = bpp - bofs - bsize; 
  
  }
  
  // 15 bits RGB (hicolor)
  if ((rofs==10) && (rsize==5)
   && (gofs==5) && (gsize==5)
   && (bofs==0) && (bsize==5) ) {
   
    return "RGB555";
      
  } else   
  // 16 bits RGB (hicolor)
  if ((rofs==11) && (rsize==5)
   && (gofs==5) && (gsize==6)
   && (bofs==0) && (bsize==5) ) {
   
    return "RGB565";
      
  } else   
  
  // 24 bits RGB (truecolor)
  if ((rofs==16) && (rsize==8)
   && (gofs==8) && (gsize==8)
   && (bofs==0) && (bsize==8) ) {
   
    if (bpp==24)
      return "BGR24";
    else
      return "BGRA32";
      
  } else   
  // 24 bits BGR (truecolor)
  if ((rofs==0) && (rsize==8)
   && (gofs==8) && (gsize==8)
   && (bofs==16) && (bsize==8)) {
   
    if (bpp==24)
      return "RGB24";
    else
      return "RGBA32";
      
  } else
  // special 32 bits (mostly on big endian hosts)
  if ((rofs==8) && (rsize==8)
   && (gofs==16) && (gsize==8)
   && (bofs==24) && (bsize==8)) {
   
   return "ARGB32";
   
  } else
  // special 32 bits (mostly on big endian hosts)
  if ((rofs==24) && (rsize==8)
   && (gofs==16) && (gsize==8)
   && (bofs==8) && (bsize==8)) {
   
   return "ABGR32";
   
  }
  
  return NULL; // unknown format
  
}

} // end of namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
/* vim: set cindent tabstop=8 softtabstop=4 shiftwidth=4: */
