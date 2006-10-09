#ifndef GNASH_AGG_RENDERER_SCANLINE_INCLUDED
#define GNASH_AGG_RENDERER_SCANLINE_INCLUDED

//
// This file has been included to support the AGG2 package
// found in debian testing, which laks the
// render_scanlines_coumpund_layered function
//
// It should only define it if not defined, but dunno how to check ...
//

namespace agg {

typedef unsigned char cover_type;

    //=======================================render_scanlines_compound_layered
    template<class Rasterizer, 
             class ScanlineAA, 
             class BaseRenderer, 
             class SpanAllocator,
             class StyleHandler>
    void render_scanlines_compound_layered(Rasterizer& ras, 
                                           ScanlineAA& sl_aa,
                                           BaseRenderer& ren,
                                           SpanAllocator& alloc,
                                           StyleHandler& sh)
    {
        if(ras.rewind_scanlines())
        {
            int min_x = ras.min_x();
            int len = ras.max_x() - min_x + 2;
            sl_aa.reset(min_x, ras.max_x());

            typedef typename BaseRenderer::color_type color_type;
            color_type* color_span   = alloc.allocate(len * 2);
            color_type* mix_buffer   = color_span + len;
            cover_type* cover_buffer = ras.allocate_cover_buffer(len);
            unsigned num_spans;

            unsigned num_styles;
            unsigned style;
            bool     solid;
            while((num_styles = ras.sweep_styles()) > 0)
            {
                typename ScanlineAA::const_iterator span_aa;
                if(num_styles == 1)
                {
                    // Optimization for a single style. Happens often
                    //-------------------------
                    if(ras.sweep_scanline(sl_aa, 0))
                    {
                        style = ras.style(0);
                        if(sh.is_solid(style))
                        {
                            // Just solid fill
                            //-----------------------
                            render_scanline_aa_solid(sl_aa, ren, sh.color(style));
                        }
                        else
                        {
                            // Arbitrary span generator
                            //-----------------------
                            span_aa   = sl_aa.begin();
                            num_spans = sl_aa.num_spans();
                            for(;;)
                            {
                                len = span_aa->len;
                                sh.generate_span(color_span, 
                                                 span_aa->x, 
                                                 sl_aa.y(), 
                                                 len, 
                                                 style);

                                ren.blend_color_hspan(span_aa->x, 
                                                      sl_aa.y(), 
                                                      span_aa->len,
                                                      color_span,
                                                      span_aa->covers);
                                if(--num_spans == 0) break;
                                ++span_aa;
                            }
                        }
                    }
                }
                else
                {
                    int      sl_start = ras.scanline_start();
                    unsigned sl_len   = ras.scanline_length();

                    if(sl_len)
                    {
                        memset(mix_buffer + sl_start - min_x, 
                               0, 
                               sl_len * sizeof(color_type));

                        memset(cover_buffer + sl_start - min_x, 
                               0, 
                               sl_len * sizeof(cover_type));

                        int sl_y = 0x7FFFFFFF;
                        unsigned i;
                        for(i = 0; i < num_styles; i++)
                        {
                            style = ras.style(i);
                            solid = sh.is_solid(style);

                            if(ras.sweep_scanline(sl_aa, i))
                            {
                                unsigned    cover;
                                color_type* colors;
                                color_type* cspan;
                                cover_type* src_covers;
                                cover_type* dst_covers;
                                span_aa   = sl_aa.begin();
                                num_spans = sl_aa.num_spans();
                                sl_y      = sl_aa.y();
                                if(solid)
                                {
                                    // Just solid fill
                                    //-----------------------
                                    for(;;)
                                    {
                                        color_type c = sh.color(style);
                                        len    = span_aa->len;
                                        colors = mix_buffer + span_aa->x - min_x;
                                        src_covers = span_aa->covers;
                                        dst_covers = cover_buffer + span_aa->x - min_x;
                                        do
                                        {
                                            cover = *src_covers;
                                            if(*dst_covers + cover > cover_full)
                                            {
                                                cover = cover_full - *dst_covers;
                                            }
                                            if(cover)
                                            {
                                                colors->add(c, cover);
                                                *dst_covers += cover;
                                            }
                                            ++colors;
                                            ++src_covers;
                                            ++dst_covers;
                                        }
                                        while(--len);
                                        if(--num_spans == 0) break;
                                        ++span_aa;
                                    }
                                }
                                else
                                {
                                    // Arbitrary span generator
                                    //-----------------------
                                    for(;;)
                                    {
                                        len = span_aa->len;
                                        colors = mix_buffer + span_aa->x - min_x;
                                        cspan  = color_span;
                                        sh.generate_span(cspan, 
                                                         span_aa->x, 
                                                         sl_aa.y(), 
                                                         len, 
                                                         style);
                                        src_covers = span_aa->covers;
                                        dst_covers = cover_buffer + span_aa->x - min_x;
                                        do
                                        {
                                            cover = *src_covers;
                                            if(*dst_covers + cover > cover_full)
                                            {
                                                cover = cover_full - *dst_covers;
                                            }
                                            if(cover)
                                            {
                                                colors->add(*cspan, cover);
                                                *dst_covers += cover;
                                            }
                                            ++cspan;
                                            ++colors;
                                            ++src_covers;
                                            ++dst_covers;
                                        }
                                        while(--len);
                                        if(--num_spans == 0) break;
                                        ++span_aa;
                                    }
                                }
                            }
                        }
                        ren.blend_color_hspan(sl_start, 
                                              sl_y, 
                                              sl_len,
                                              mix_buffer + sl_start - min_x,
                                              0,
                                              cover_full);
                    } //if(sl_len)
                } //if(num_styles == 1) ... else
            } //while((num_styles = ras.sweep_styles()) > 0)
        } //if(ras.rewind_scanlines())
    }
} // end of namespace agg

#endif
