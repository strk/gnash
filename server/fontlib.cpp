

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A module to take care of all of gnash's loaded fonts.

/* $Id: fontlib.cpp,v 1.38 2007/12/01 01:08:09 strk Exp $ */

#include "tu_file.h"
#include "gnash.h"
#include "font.h"
#include "impl.h"
#include "log.h"
#include "render.h"
#include "shape_character_def.h"
#include "styles.h"
#include "tesselate.h"
#include "movie_definition.h"

// Define to the name of a default font.
#define DEFAULT_FONT_NAME "_sans"

namespace gnash {
namespace fontlib {

namespace {
	std::vector< boost::intrusive_ptr<font> >	s_fonts;
	boost::intrusive_ptr<font> _defaultFont;
}

	// Size (in TWIPS) of the box that the glyph should
	// stay within.
	static float	s_rendering_box = 1536.0f;	// this *should* be 1024, but some glyphs in some fonts exceed it!

	// The nominal size of the final antialiased glyphs stored in
	// the texture.  This parameter controls how large the very
	// largest glyphs will be in the texture; most glyphs will be
	// considerably smaller.  This is also the parameter that
	// controls the tradeoff between texture RAM usage and
	// sharpness of large text.
	static int	s_glyph_nominal_size =
// You can override the default rendered glyph size in
// compatibility_include.h, to trade off memory vs. niceness of large
// glyphs.  You can also override this at run-time via
// gnash::fontlib::set_nominal_glyph_pixel_size()
#ifndef GNASH_FONT_NOMINAL_GLYPH_SIZE_DEFAULT
	96
#else
	GNASH_FONT_NOMINAL_GLYPH_SIZE_DEFAULT
#endif
	;

	static const int	OVERSAMPLE_BITS = 2;
	static const int	OVERSAMPLE_FACTOR = (1 << OVERSAMPLE_BITS);

	// The dimensions of the textures that the glyphs get packed into.
	static const size_t	GLYPH_CACHE_TEXTURE_SIZE = 256;

	// How much space to leave around the individual glyph image.
	// This should be at least 1.  The bigger it is, the smoother
	// the boundaries of minified text will be, but the more
	// texture space is wasted.
	const int PAD_PIXELS = 3;


	// The raw non-antialiased render size for glyphs.
	static int	s_glyph_render_size = s_glyph_nominal_size << OVERSAMPLE_BITS;

	
	void	set_nominal_glyph_pixel_size(size_t pixel_size)
	{
		static const size_t	MIN_SIZE = 4;
		static const size_t	MAX_SIZE = GLYPH_CACHE_TEXTURE_SIZE / 2;

		if (pixel_size < MIN_SIZE)
		{
			log_error("set_nominal_glyph_pixel_size(%d) too small, clamping to %d\n",
				  pixel_size,
				  MIN_SIZE);
			pixel_size = MIN_SIZE;
		}
		else if (pixel_size > MAX_SIZE)
		{
			log_error("set_nominal_glyph_pixel_size(%d) too large, clamping to %d\n",
				  pixel_size,
				  MAX_SIZE);
			pixel_size = MAX_SIZE;
		}

		s_glyph_nominal_size = pixel_size;
		s_glyph_render_size = s_glyph_nominal_size << OVERSAMPLE_BITS;
	}


	//
	// State for the glyph packer.
	//

	static uint8_t*	s_render_buffer = NULL;
	static matrix	s_render_matrix;

	static uint8_t*	s_current_cache_image = NULL;

	// for setting the bitmap_info after they're packed.
	class pending_glyph_info
	{
	public:
		font*	m_source_font;
		int	m_glyph_index;
		texture_glyph	m_texture_glyph;

		pending_glyph_info()
			:
			m_source_font(NULL),
			m_glyph_index(-1)
		{
		}

		pending_glyph_info(font* f, int gi, const texture_glyph& tg)
			:
			m_source_font(f),
			m_glyph_index(gi),
			m_texture_glyph(tg)
		{
		}
	};
	static std::vector< pending_glyph_info >	s_pending_glyphs;


	// Integer-bounded 2D rectangle.
	// 
	// TODO: make the gnash::rect a templated class instead
	//
	class recti
	{
	public:
		int	m_x_min, m_x_max, m_y_min, m_y_max;

		recti(int x0 = 0, int x1 = 0, int y0 = 0, int y1 = 0)
			:
			m_x_min(x0),
			m_x_max(x1),
			m_y_min(y0),
			m_y_max(y1)
		{
		}

		bool	is_valid() const
		{
			return m_x_min <= m_x_max
				&& m_y_min <= m_y_max;
		}

		bool	intersects(const recti& r) const
		// Return true if r touches *this.
		{
			if (m_x_min >= r.m_x_max
			    || m_x_max <= r.m_x_min
			    || m_y_min >= r.m_y_max
			    || m_y_max <= r.m_y_min)
			{
				// disjoint.
				return false;
			}
			return true;
		}

		bool	contains(int x, int y) const
		// Return true if (x,y) is inside *this.
		{
			return x >= m_x_min
				&& x < m_x_max
				&& y >= m_y_min
				&& y < m_y_max;
		}
	};
	// Rects already on the texture.
	static std::vector<recti>	s_covered_rects;

	// 2d integer point.
	class pointi
	{
	public:
		int	x, y;

		pointi(int x = 0, int y = 0)
			:
			x(x),
			y(y)
		{
		}

		bool	operator<(const pointi& p) const
		// For sorting anchor points.
		{
			return imin(x, y) < imin(p.x, p.y);
		}
	};
	// Candidates for upper-left corner of a new rectangle.  Use
	// lower-left and upper-right of previously placed rects.
	static std::vector<pointi>	s_anchor_points;


	static bool	s_saving = false;
	static bool	s_save_dummy_bitmaps = false;
	static tu_file*	s_file = NULL;


	static void	ensure_cache_image_available()
	{
		if (s_pending_glyphs.size() == 0)
		{
			// Set up a cache.
			if (s_current_cache_image == NULL)
			{
				s_current_cache_image = new uint8_t[GLYPH_CACHE_TEXTURE_SIZE * GLYPH_CACHE_TEXTURE_SIZE];
			}
			memset(s_current_cache_image, 0, GLYPH_CACHE_TEXTURE_SIZE * GLYPH_CACHE_TEXTURE_SIZE);

			// Initialize the coverage data.
			s_covered_rects.resize(0);
			s_anchor_points.resize(0);
			s_anchor_points.push_back(pointi(0, 0));	// seed w/ upper-left of texture.
		}
	}


	void	finish_current_texture(movie_definition* owner)
	{

		bool embed=true; // use embedded fonts

		if (s_pending_glyphs.size() == 0)
		{
			return;
		}

#if 0
		//xxxxxx debug hack -- dump image data to a file
		static int	s_seq = 0;
		char buffer[100];
		sprintf(buffer, "dump%02d.ppm", s_seq);
		s_seq++;
		FILE*	fp = fopen(buffer, "wb");
		if (fp)
		{
			fprintf(fp, "P6\n%d %d\n255\n", GLYPH_CACHE_TEXTURE_SIZE, GLYPH_CACHE_TEXTURE_SIZE);
			for (size_t i = 0; i < GLYPH_CACHE_TEXTURE_SIZE * GLYPH_CACHE_TEXTURE_SIZE; i++)
			{
				fputc(s_current_cache_image[i], fp);
				fputc(s_current_cache_image[i], fp);
				fputc(s_current_cache_image[i], fp);
			}
			fclose(fp);
		}
		//xxxxxx
#endif // 0

		if (s_saving)		// HACK!!!
		{
			if (s_save_dummy_bitmaps)
			{
				// Save a mini placeholder bitmap.
				s_file->write_le16(1);
				s_file->write_le16(1);
				s_file->write_byte(0);
			}
			else
			{
				size_t w = GLYPH_CACHE_TEXTURE_SIZE;
				size_t h = GLYPH_CACHE_TEXTURE_SIZE;

				// save bitmap size
				s_file->write_le16(w);
				s_file->write_le16(h);

				// save bitmap contents
				s_file->write_bytes(s_current_cache_image, w*h);
			}
		}

		if (owner->get_create_bitmaps() == DO_LOAD_BITMAPS)
		{
			boost::intrusive_ptr<bitmap_info>	bi;
			bi = render::create_bitmap_info_alpha(
				GLYPH_CACHE_TEXTURE_SIZE,
				GLYPH_CACHE_TEXTURE_SIZE,
				s_current_cache_image);
			owner->add_bitmap_info(bi.get());

			// Push finished glyphs into their respective fonts.
			for (int i = 0, n = s_pending_glyphs.size(); i < n; i++)
			{
			pending_glyph_info*	pgi = &s_pending_glyphs[i];
			assert(pgi->m_glyph_index != -1);
			assert(pgi->m_source_font != NULL);

			pgi->m_texture_glyph.set_bitmap_info(bi.get());
			pgi->m_source_font->add_texture_glyph(pgi->m_glyph_index, pgi->m_texture_glyph, embed);
			//s_pending_glyphs[i]->set_bitmap_info(bi.get());
			}
		}
		s_pending_glyphs.clear();
		
	}


	bool	is_rect_available(const recti& r)
	// Return true if the given rect can be packed into the
	// currently active texture.
	{
		assert(r.is_valid());
		assert(r.m_x_min >= 0);
		assert(r.m_y_min >= 0);

		if ((size_t)r.m_x_max > GLYPH_CACHE_TEXTURE_SIZE
		    || (size_t)r.m_y_max > GLYPH_CACHE_TEXTURE_SIZE)
		{
			// Rect overflows the texture bounds.
			return false;
		}

		// Check against existing rects.
		for (int i = 0, n = s_covered_rects.size(); i < n; i++)
		{
			if (r.intersects(s_covered_rects[i]))
			{
				return false;
			}
		}

		// Spot appears to be open.
		return true;
	}


	void	add_cover_rect(const recti& r)
	// Add the given rect to our list.  Eliminate any anchor
	// points that are disqualified by this new rect.
	{
		s_covered_rects.push_back(r);

		for (unsigned int i = 0; i < s_anchor_points.size(); i++)
		{
			const pointi&	p = s_anchor_points[i];
			if (r.contains(p.x, p.y))
			{
				// Eliminate this point from consideration.
				s_anchor_points.erase(s_anchor_points.begin() + i);
				i--;
			}
		}
	}


	void	add_anchor_point(const pointi& p)
	// Add point to our list of anchors.  Keep the list sorted.
	{
		// Add it to end, since we expect new points to be
		// relatively greater than existing points.
		s_anchor_points.push_back(p);

		// Insertion sort -- bubble down into correct spot.
		for (int i = s_anchor_points.size() - 2; i >= 0; i--)
		{
			if (s_anchor_points[i + 1] < s_anchor_points[i])
			{
				swap(&(s_anchor_points[i]), &(s_anchor_points[i + 1]));
			}
			else
			{
				// Done bubbling down.
				break;
			}
		}
	}


	bool	pack_rectangle(int* px, int* py, size_t width, size_t height)
	// Find a spot for the rectangle in the current cache image.
	// Return true if there's a spot; false if there's no room.
	{
		// Nice algo, due to JARE:
		//
		// * keep a list of "candidate points"; initialize it with {0,0}
		//
		// * each time we add a rect, add its lower-left and
		// upper-right as candidate points.
		//
		// * search the candidate points only, when looking
		// for a good spot.  If we find a good one, also try
		// scanning left or up as well; sometimes this can
		// close some open space.
		//
		// * when we use a candidate point, remove it from the list.

		// Consider candidate spots.
		for (int i = 0, n = s_anchor_points.size(); i < n; i++)
		{
			const pointi&	p = s_anchor_points[i];
			recti	r(p.x, p.x + width, p.y, p.y + height);

			// Is this spot any good?
			if (is_rect_available(r))
			{
				// Good spot.  Scan left to see if we can tighten it up.
				while (r.m_x_min > 0)
				{
					recti	r2(r.m_x_min - 1, r.m_x_min - 1 + width, r.m_y_min, r.m_y_min + height);
					if (is_rect_available(r2))
					{
						// Shift left.
						r = r2;
					}
					else
					{
						// Not clear; stop scanning.
						break;
					}
				}

				// Mark our covered rect; remove newly covered anchors.
				add_cover_rect(r);

				// Found our desired spot.  Add new
				// candidate points to the anchor list.
				add_anchor_point(pointi(r.m_x_min, r.m_y_max));	// lower-left
				add_anchor_point(pointi(r.m_x_max, r.m_y_min));	// upper-right

				*px = r.m_x_min;
				*py = r.m_y_min;

				return true;
			}
		}

		// Couldn't find a good spot.
		return false;
	}


	// This is for keeping track of our rendered glyphs, before
	// packing them into textures and registering with the font.
	class rendered_glyph_info
	{
	public:
		font*	m_source_font;
		int	m_glyph_index;
		image::alpha*	m_image;
		unsigned int	m_image_hash;
		float	m_offset_x;
		float	m_offset_y;

		rendered_glyph_info()
			:
			m_source_font(0),
			m_glyph_index(0),
			m_image(0),
			m_image_hash(0),
			m_offset_x(0),
			m_offset_y(0)
		{
		}
	};


	static void	software_trapezoid(
		float y0, float y1,
		float xl0, float xl1,
		float xr0, float xr1)
	// Fill the specified trapezoid in the software output buffer.
	{
		assert(s_render_buffer);

		int	iy0 = static_cast<int>(ceilf(y0));
		int	iy1 = static_cast<int>(ceilf(y1));
		float	dy = y1 - y0;

		for (int y = iy0; y < iy1; y++)
		{
			if (y < 0) continue;
			if (y >= s_glyph_render_size) return;

			float	f = (y - y0) / dy;
			int	xl = static_cast<int>(ceilf(flerp(xl0, xl1, f)));
			int	xr = static_cast<int>(ceilf(flerp(xr0, xr1, f)));
			
			xl = iclamp(xl, 0, s_glyph_render_size - 1);
			xr = iclamp(xr, 0, s_glyph_render_size - 1);

			if (xr > xl)
			{
				memset(s_render_buffer + y * s_glyph_render_size + xl,
				       255,
				       xr - xl);
			}
		}
	}


	// A trapezoid accepter that does B&W rendering into our
	// software buffer.
        class draw_into_software_buffer : public tesselate::trapezoid_accepter
	{
	public:
		// Overrides from trapezoid_accepter
		virtual void	accept_trapezoid(int /* style */, const tesselate::trapezoid& tr)
		{
			// Transform the coords.
			float	x_scale = s_render_matrix.m_[0][0];
			float	y_scale = s_render_matrix.m_[1][1];
			float	x_offset = s_render_matrix.m_[0][2];
			float	y_offset = s_render_matrix.m_[1][2];

			float	y0 = tr.m_y0 * y_scale + y_offset;
			float	y1 = tr.m_y1 * y_scale + y_offset;
			float	lx0 = tr.m_lx0 * x_scale + x_offset;
			float	lx1 = tr.m_lx1 * x_scale + x_offset;
			float	rx0 = tr.m_rx0 * x_scale + x_offset;
			float	rx1 = tr.m_rx1 * x_scale + x_offset;

			// Draw into the software buffer.
			software_trapezoid(y0, y1, lx0, lx1, rx0, rx1);
		}

		virtual void	accept_line_strip(int /*style*/,
				const point* /*coords*/, int /*coord_count*/)
		{
			// Shape glyphs should not contain lines.
			abort();
		}
	};


	static bool	render_glyph(rendered_glyph_info& rgi, const shape_character_def* sh)
	// Render the given outline shape into a cached font texture.
	// Return true if the glyph is not empty; false if it's
	// totally empty.
	// 
	// Return fill in the image and offset members of the given
	// rgi.
	{
		assert(sh);
		assert(s_render_buffer);

		//
		// Tesselate and render the shape into a software buffer.
		//

		// Clear the render output to 0.
		memset(s_render_buffer, 0, s_glyph_render_size * s_glyph_render_size);

		// Look at glyph bounds; adjust origin to make sure
		// the shape will fit in our output.
		float	offset_x = 0.f;
		float	offset_y = s_rendering_box;
		rect	glyph_bounds;
		sh->compute_bound(&glyph_bounds);
		if (glyph_bounds.get_x_min() < 0)
		{
			offset_x = - glyph_bounds.get_x_min();
		}
		if (glyph_bounds.get_y_max() > 0)
		{
			offset_y = s_rendering_box - glyph_bounds.get_y_max();
		}

		s_render_matrix.set_identity();
		s_render_matrix.concatenate_scale(s_glyph_render_size / s_rendering_box);
		s_render_matrix.concatenate_translation(offset_x, offset_y);

		// Tesselate & draw the shape.
		draw_into_software_buffer	accepter;
		sh->tesselate(s_rendering_box / s_glyph_render_size * 0.5f, &accepter);

		//
		// Process the results of rendering.
		//

		// Shrink the results down by a factor of 4x, to get
		// antialiasing.  Also, analyze the data boundaries.
		bool	any_nonzero_pixels = false;
		int	min_x = s_glyph_nominal_size;
		int	max_x = 0;
		int	min_y = s_glyph_nominal_size;
		int	max_y = 0;
		uint8_t*	output = new uint8_t[s_glyph_nominal_size * s_glyph_nominal_size];
		for (int j = 0; j < s_glyph_nominal_size; j++)
		{
			for (int i = 0; i < s_glyph_nominal_size; i++)
			{
				// Sum up the contribution to this output texel.
				int	sum = 0;
				for (int jj = 0; jj < OVERSAMPLE_FACTOR; jj++)
				{
					for (int ii = 0; ii < OVERSAMPLE_FACTOR; ii++)
					{
						uint8_t	texel = s_render_buffer[
							((j << OVERSAMPLE_BITS) + jj) * s_glyph_render_size
							+ ((i << OVERSAMPLE_BITS) + ii)];
						sum += texel;
					}
				}
				sum >>= OVERSAMPLE_BITS;
				sum >>= OVERSAMPLE_BITS;
				if (sum > 0)
				{
					any_nonzero_pixels = true;
					min_x = imin(min_x, i);
					max_x = imax(max_x, i);
					min_y = imin(min_y, j);
					max_y = imax(max_y, j);
				}
				output[j * s_glyph_nominal_size + i] = (uint8_t) sum;
			}
		}

		if (any_nonzero_pixels)
		{
			// Fill in rendered_glyph_info.
			rgi.m_image = new image::alpha(max_x - min_x + 1, max_y - min_y + 1);
			rgi.m_offset_x = offset_x / s_rendering_box * s_glyph_nominal_size - min_x;
			rgi.m_offset_y = offset_y / s_rendering_box * s_glyph_nominal_size - min_y;

			// Copy the rendered glyph into the new image.
			for (size_t j = 0, n = rgi.m_image->height(); j < n; j++)
			{
				memcpy(
					rgi.m_image->scanline(j),
					output + (min_y + j) * s_glyph_nominal_size + min_x,
					rgi.m_image->width());
			}
		}
		else
		{
			// Glyph is empty; don't create an image for it.
			return false;
		}

		delete [] output;	// @@ TODO should keep this around longer, instead of new/delete for each glyph

		rgi.m_image_hash = rgi.m_image->compute_hash();

		return true;
	}



	bool	try_to_reuse_previous_image(
		const rendered_glyph_info& rgi,
		const map<unsigned int, const rendered_glyph_info*>& image_hash)
	// See if we've already packed an identical glyph image for
	// another glyph.  If so, then reuse it, and return true.
	// If no reusable image, return false.
	//
	// Reusing identical images can be a huge win, especially for
	// fonts that use the same dummy glyph for many undefined
	// characters.
	{
		bool embed=true; // use embedded fonts

		const map<unsigned int, const rendered_glyph_info*>::const_iterator image =
			image_hash.find(rgi.m_image_hash);


		const rendered_glyph_info*	identical_image = NULL;
		if (image != image_hash.end()) {
		  identical_image = (*image).second;
		}

		if (identical_image)
		{
			// Found a match.  But is it *really* a match?  Do a
			// bitwise compare.
			if (*(rgi.m_image) == *(identical_image->m_image))
			{
				// Yes, a real bitwise match.  Use the previous
				// image's texture data.
				texture_glyph	identical_tg =
					identical_image->
					m_source_font->
					get_texture_glyph(identical_image->m_glyph_index, embed); 

				if (identical_tg.is_renderable() == false)
				{
					// The matching glyph hasn't been pushed into the font yet.
					// Search for it in s_pending_glyphs.
					bool	found_it = false;
					for (int i = 0, n = s_pending_glyphs.size(); i < n; i++)
					{
						const pending_glyph_info&	pgi = s_pending_glyphs[i];
						if (pgi.m_source_font == identical_image->m_source_font
						    && pgi.m_glyph_index == identical_image->m_glyph_index)
						{
							// This is the one we want to alias with.
							identical_tg = pgi.m_texture_glyph;
							found_it = true;
						}
					}

					if (found_it == false)
					{
						// Should not happen -- glyph should either be in the font, or in s_pending_glyphs.
						abort();
						return false;
					}
				}

				texture_glyph	tg;

				// copy the bitmap & uv data from identical_tg
				tg = identical_tg;

				// Use our own offset, in case it's different.
				tg.m_uv_origin.x = tg.m_uv_bounds.get_x_min()
					+ rgi.m_offset_x / GLYPH_CACHE_TEXTURE_SIZE;
				tg.m_uv_origin.y = tg.m_uv_bounds.get_y_min()
					+ rgi.m_offset_y / GLYPH_CACHE_TEXTURE_SIZE;

				if (identical_tg.is_renderable())
				{
					// This image is already packed and has a valid bitmap_info.
					// Push straight into our font.
					rgi.m_source_font->add_texture_glyph(rgi.m_glyph_index, tg, embed); // embed font only
				}
				else
				{
					// Set bitmap_info and push into font once texture is done being packed.
					s_pending_glyphs.push_back(
						pending_glyph_info(
							rgi.m_source_font,
							rgi.m_glyph_index,
							tg));
				}

				return true;
			}
			// else hash matched, but images didn't.
		}
		else
		{
#if 0
#ifndef NDEBUG
			// Sanity check the hash -- there should be no
			// image in it that exactly matches this
			// image.
			for (hash<unsigned int, const rendered_glyph_info*>::const_iterator it = image_hash.begin();
			     it != image_hash.end();
			     ++it)
			{
				if (*(rgi.m_image) == *(it->second->m_image))
				{
					// bah!  what up???
					unsigned int	hash_a = rgi.m_image->compute_hash();
					unsigned int	hash_b = it->second->m_image->compute_hash();

					log_msg("a = %x, b = %x\n", hash_a, hash_b);//xxxxx
				}
			}
#endif // not NDEBUG
#endif // 0
		}

		return false;
	}


	void	pack_and_assign_glyphs(std::vector<rendered_glyph_info>& glyph_info, movie_definition* owner)
	// Pack the given glyphs into textures, and push the
	// texture_glyph info into the source fonts.
	//
	// Re-arranges the glyphs (i.e. sorts them by size) but
	// otherwise doesn't munge the array.
	{
		// Sort the glyphs by size (biggest first).
		class sorter
		{
		public:
			static int	sort_by_size(const void* a, const void* b)
			// For qsort.
			{
				const rendered_glyph_info*	ga = (const rendered_glyph_info*) a;
				const rendered_glyph_info*	gb = (const rendered_glyph_info*) b;

				int	a_size = ga->m_image->width() + ga->m_image->height();
				int	b_size = gb->m_image->width() + gb->m_image->height();

				return b_size - a_size;
			}
		};
		if (glyph_info.size())
		{
			qsort(&glyph_info[0], glyph_info.size(), sizeof(glyph_info[0]), sorter::sort_by_size);
		}

		// Flag for whether we've processed this glyph yet.
		std::vector<bool> packed(glyph_info.size(), false);

		// Share identical texture data where possible, by
		// doing glyph image comparisons.
		map<unsigned int, const rendered_glyph_info*>	image_hash;

		// Pack the glyphs.
		{for (int i = 0, n = glyph_info.size(); i < n; )
		{
			int	index = i;

			// Try to pack a glyph into the existing texture.
			for (;;)
			{
				const rendered_glyph_info&	rgi = glyph_info[index];

				// First things first: are we identical to a glyph that has
				// already been packed?
				if (try_to_reuse_previous_image(rgi, image_hash))
				{
					packed[index] = true;
					break;
				}

				size_t	raw_width = rgi.m_image->width();
				size_t	raw_height = rgi.m_image->height();

				// Need to pad around the outside.
				size_t	width = raw_width + (PAD_PIXELS * 2);
				size_t	height = raw_height + (PAD_PIXELS * 2);

				assert(width < GLYPH_CACHE_TEXTURE_SIZE);
				assert(height < GLYPH_CACHE_TEXTURE_SIZE);

				// Does this glyph fit?
				int	pack_x = 0, pack_y = 0;
				ensure_cache_image_available();
				if (pack_rectangle(&pack_x, &pack_y, width, height))
				{
					// Fits!
					// Blit the output image into its new spot.
					for (size_t j = 0; j < raw_height; j++)
					{
						memcpy(s_current_cache_image
						       + (pack_y + PAD_PIXELS + j) * GLYPH_CACHE_TEXTURE_SIZE
						       + pack_x + PAD_PIXELS,
						       rgi.m_image->scanline(j),
						       raw_width);
					}

					// Fill out the glyph info.
					texture_glyph	tg;
					tg.m_uv_origin.x = (pack_x + rgi.m_offset_x) / (GLYPH_CACHE_TEXTURE_SIZE);
					tg.m_uv_origin.y = (pack_y + rgi.m_offset_y) / (GLYPH_CACHE_TEXTURE_SIZE);
					tg.m_uv_bounds.enclose_point(
						float(pack_x) / (GLYPH_CACHE_TEXTURE_SIZE),
						float(pack_y) / (GLYPH_CACHE_TEXTURE_SIZE)
					);
					tg.m_uv_bounds.expand_to_point(
						float(pack_x + width) / (GLYPH_CACHE_TEXTURE_SIZE),
						float(pack_y + height) / (GLYPH_CACHE_TEXTURE_SIZE)
					);

					// Fill in bitmap info and push into the source font later.
					s_pending_glyphs.push_back(
						pending_glyph_info(
							rgi.m_source_font,
							rgi.m_glyph_index,
							tg));

					// Add this into the hash so it can possibly be reused.
					map<unsigned int, const rendered_glyph_info*>::const_iterator image =
						image_hash.find(rgi.m_image_hash);
					if (image == image_hash.end())
					{
						image_hash[rgi.m_image_hash] = &rgi;
					}

					packed[index] = true;

					break;
				}
				else
				{
					// Try the next unpacked glyph.
					index++;
					while (index < n && packed[index]) index++;

					if (index >= n)
					{
						// None of the glyphs will fit.  Finish off this texture.
						finish_current_texture(owner);

						// And go around again.
						index = i;
					}
				}
			}

			// Skip to the next unpacked glyph.
			while (i < n && packed[i]) i++;
		}}
	}


	float	get_texture_glyph_max_height(const font* f)
	{
		return 1024.0f / s_rendering_box * f->get_texture_glyph_nominal_size(); //  s_glyph_nominal_size;
	}


	static void	wipe_font_textures(const std::vector<font*>& fonts)
	{
		for (int i = 0, n = fonts.size(); i < n; i++)
		{
			font*	f = fonts[i];
			f->wipe_texture_glyphs();
		}
	}


	//
	// Public interface
	//


	void	clear()
	// Release all the fonts we know about.
	{
		s_fonts.clear();
	}

boost::intrusive_ptr<font>
get_default_font()
{
	if ( _defaultFont ) return _defaultFont;
	_defaultFont = new font(DEFAULT_FONT_NAME);
	return _defaultFont;
}

	int	get_font_count()
	// Return the number of fonts in our library.
	{
		return s_fonts.size();
	}


	font*	get_font(int index)
	// Retrieve one of our fonts, by index.
	{
		if (index < 0 || index >= (int) s_fonts.size())
		{
			return NULL;
		}

		return s_fonts[index].get();
	}


	font*	get_font(const std::string& name)
	// Return the named font.
	{
		// Dumb linear search.
		for (unsigned int i = 0; i < s_fonts.size(); i++)
		{
			font*	f = s_fonts[i].get();
			if (f != NULL)
			{
				if (f->get_name() == name)
				{
					return f;
				}
			}
		}
		return NULL;
	}

	void	add_font(font* f)
	// Add the given font to our library.
	{
		assert(f);

#ifndef NDEBUG
		// Make sure font isn't already in the list.
		for (unsigned int i = 0; i < s_fonts.size(); i++)
		{
			assert(s_fonts[i] != f);
		}
#endif // not NDEBUG

		s_fonts.push_back(f);
	}


	void	draw_glyph(const matrix& mat, const texture_glyph& tg, const rgba& color, int nominal_glyph_height)
	// Draw the given texture glyph using the given transform, in
	// the given color.
	{
		assert(tg.is_renderable());

		// @@ worth it to precompute these bounds?

		rect	bounds = tg.m_uv_bounds;
		if ( bounds.is_null() )
		{
			//log_debug("Textured glyph rendering skipped, since it's bounds are null");
			return;
		}

		bounds.shift_x (-tg.m_uv_origin.x);
		bounds.shift_y (-tg.m_uv_origin.y);

		// Scale from uv coords to the 1024x1024 glyph square.
		// @@ need to factor this out!
		static float	s_scale = GLYPH_CACHE_TEXTURE_SIZE * s_rendering_box / nominal_glyph_height;

		//log_msg("Scaling bounds %s by factor %g (nominal_glyph_height: %d)", bounds.toString().c_str(), s_scale, nominal_glyph_height);

		bounds.scale_x(s_scale);
		bounds.scale_y(s_scale);
		
		render::draw_bitmap(mat, tg.m_bitmap_info.get(), bounds, tg.m_uv_bounds, color);
	}


#if 0
	void	draw_string(const font* f, float x, float y, float size, const char* text)
	// Host-driven text rendering function. This not-tested and unfinished.
	{
		// Dummy arrays with a white fill style.  For passing to shape_character::display().
		static std::vector<fill_style>	s_dummy_style;
		static std::vector<line_style>	s_dummy_line_style;
		static display_info	s_dummy_display_info;
		if (s_dummy_style.size() < 1)
		{
			s_dummy_style.resize(1);
			s_dummy_style.back().set_color(rgba(255, 255, 255, 255));
		}

		// Render each glyph in the string.
		for (int i = 0; text[i]; i++)
		{
			int g = f->get_glyph_index(text[i]);
			if (g == -1) 
			{
				continue;	// FIXME: advance?
			}

			const texture_glyph&	tg = f->get_texture_glyph(g);
			
			matrix m;
			m.concatenate_translation(x, y);
			m.concatenate_scale(size / 1024.0f);

			if (tg.is_renderable())
			{
				// Draw the glyph using the cached texture-map info.
				fontlib::draw_glyph(m, tg, rgba());
			}
			else
			{
				shape_character_def*	glyph = f->get_glyph(g);

				// Draw the character using the filled outline.
				if (glyph)
				{
					glyph->display(dummy_inst, s_dummy_style, s_dummy_line_style);
				}
			}

			x += f->get_advance(g);
		}

	}
#endif // 0

}	// end namespace fontlib
}	// end namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
