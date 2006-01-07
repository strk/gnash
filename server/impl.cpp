// 
//   Copyright (C) 2005 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
// Some implementation for SWF player.

#include "tu_file.h"
#include "utility.h"
#include "action.h"
#include "button.h"
#include "impl.h"
#include "font.h"
#include "fontlib.h"
#include "log.h"
#include "morph2.h"
#include "render.h"
#include "shape.h"
#include "stream.h"
#include "styles.h"
#include "dlist.h"
#include "timers.h"
#include "image.h"
#include "jpeg.h"
#include "zlib_adapter.h"
#include <string.h>	// for memset
#include <typeinfo>
#include <float.h>


#if TU_CONFIG_LINK_TO_ZLIB
#include <zlib.h>
#endif // TU_CONFIG_LINK_TO_ZLIB


namespace gnash {
	bool	s_verbose_action = false;
	bool	s_verbose_parse = false;

#ifndef NDEBUG
	bool	s_verbose_debug = true;
#else
	bool	s_verbose_debug = false;
#endif


	void	set_verbose_action(bool verbose)
	// Enable/disable log messages re: actions.
	{
		s_verbose_action = verbose;
	}


	void	set_verbose_parse(bool verbose)
	// Enable/disable log messages re: parsing the movie.
	{
		s_verbose_parse = verbose;
	}


	static bool	s_use_cache_files = true;

	void	set_use_cache_files(bool use_cache)
	// Enable/disable attempts to read cache files when loading
	// movies.
	{
		s_use_cache_files = use_cache;
	}


	// Keep a table of loader functions for the different tag types.
	static hash<int, loader_function>	s_tag_loaders;

	void	register_tag_loader(int tag_type, loader_function lf)
	// Associate the specified tag type with the given tag loader
	// function.
	{
		assert(s_tag_loaders.get(tag_type, NULL) == false);
		assert(lf != NULL);

		s_tag_loaders.add(tag_type, lf);
	}


	//
	// file_opener callback stuff
	//

	static file_opener_callback	s_opener_function = NULL;

	void	register_file_opener_callback(file_opener_callback opener)
	// Host calls this to register a function for opening files,
	// for loading movies.
	{
		s_opener_function = opener;
	}

	//
	// progress callback stuff (from Vitaly)
	//

	static progress_callback	s_progress_function = NULL;

	void	register_progress_callback(progress_callback progress_handle)
	// Host calls this to register a function for progress bar handling
	// during loading movies.
	{
		s_progress_function = progress_handle;
	}
	
	//
	// some utility stuff
	//


	static void	execute_actions(as_environment* env, const array<action_buffer*>& action_list)
	// Execute the actions in the action list, in the given
	// environment.
	{
		for (int i = 0; i < action_list.size(); i++)
		{
			action_list[i]->execute(env);
		}
	}


	static void	dump_tag_bytes(stream* in)
	// Log the contents of the current tag, in hex.
	{
		static const int	ROW_BYTES = 16;
		char	row_buf[ROW_BYTES];
		int	row_count = 0;

		while(in->get_position() < in->get_tag_end_position())
		{
			int	c = in->read_u8();
			log_msg("%02X", c);

			if (c < 32) c = '.';
			if (c > 127) c = '.';
			row_buf[row_count] = c;
			
			row_count++;
			if (row_count >= ROW_BYTES)
			{
				log_msg("    ");
				for (int i = 0; i < ROW_BYTES; i++)
				{
					log_msg("%c", row_buf[i]);
				}

				log_msg("\n");
				row_count = 0;
			}
			else
			{
				log_msg(" ");
			}
		}

		if (row_count > 0)
		{
			log_msg("\n");
		}
	}


	character*	character_def::create_character_instance(movie* parent, int id)
	// Default.  Make a generic_character.
	{
		return new generic_character(this, parent, id);
	}


	//
	// ref_counted
	//


	ref_counted::ref_counted()
		:
		m_ref_count(0),
		m_weak_proxy(0)
	{
	}

	ref_counted::~ref_counted()
	{
		assert(m_ref_count == 0);

		if (m_weak_proxy)
		{
			m_weak_proxy->notify_object_died();
			m_weak_proxy->drop_ref();
		}
	}

	void	ref_counted::add_ref() const
	{
		assert(m_ref_count >= 0);
		m_ref_count++;
	}

	void	ref_counted::drop_ref() const
	{
		assert(m_ref_count > 0);
		m_ref_count--;
		if (m_ref_count <= 0)
		{
			// Delete me!
			delete this;
		}
	}

	weak_proxy* ref_counted::get_weak_proxy() const
	{
		assert(m_ref_count > 0);	// By rights, somebody should be holding a ref to us.

		if (m_weak_proxy == NULL)
		{
			m_weak_proxy = new weak_proxy;
			m_weak_proxy->add_ref();
		}

		return m_weak_proxy;
	}


	//
	// character
	//


	void	character::do_mouse_drag()
	// Implement mouse-dragging for this movie.
	{
		drag_state	st;
		get_drag_state(&st);
		if (this == st.m_character)
		{
			// We're being dragged!
			int	x, y, buttons;
			get_root_movie()->get_mouse_state(&x, &y, &buttons);

			point	world_mouse(PIXELS_TO_TWIPS(x), PIXELS_TO_TWIPS(y));
			if (st.m_bound)
			{
				// Clamp mouse coords within a defined rect.
				world_mouse.m_x =
					fclamp(world_mouse.m_x, st.m_bound_x0, st.m_bound_x1);
				world_mouse.m_y =
					fclamp(world_mouse.m_y, st.m_bound_y0, st.m_bound_y1);
			}

			if (st.m_lock_center)
			{
				matrix	world_mat = get_world_matrix();
				point	local_mouse;
				world_mat.transform_by_inverse(&local_mouse, world_mouse);

				matrix	parent_world_mat;
				if (m_parent)
				{
					parent_world_mat = m_parent->get_world_matrix();
				}

				point	parent_mouse;
				parent_world_mat.transform_by_inverse(&parent_mouse, world_mouse);
					
				// Place our origin so that it coincides with the mouse coords
				// in our parent frame.
				matrix	local = get_matrix();
				local.m_[0][2] = parent_mouse.m_x;
				local.m_[1][2] = parent_mouse.m_y;
				set_matrix(local);
			}
			else
			{
				// Implement relative drag...
			}
		}
	}


	//
	// Helper for movie_def_impl
	//

	struct import_info
	{
		tu_string	m_source_url;
		int	        m_character_id;
		tu_string	m_symbol;

		import_info()
			:
			m_character_id(-1)
		{
		}

		import_info(const char* source, int id, const char* symbol)
			:
			m_source_url(source),
			m_character_id(id),
			m_symbol(symbol)
		{
		}
	};


	//
	// movie_def_impl
	//
	// This class holds the immutable definition of a movie's
	// contents.  It cannot be played directly, and does not hold
	// current state; for that you need to call create_instance()
	// to get a movie_instance.
	//


	struct movie_def_impl : public movie_definition_sub
	{
		hash<int, smart_ptr<character_def> >	m_characters;
		hash<int, smart_ptr<font> >	 m_fonts;
		hash<int, smart_ptr<bitmap_character_def> >	m_bitmap_characters;
		hash<int, smart_ptr<sound_sample> >	m_sound_samples;
		array<array<execute_tag*> >	   m_playlist;	// A list of movie control events for each frame.
		array<array<execute_tag*> >	   m_init_action_list;	// Init actions for each frame.
		stringi_hash<int>	           m_named_frames;	// 0-based frame #'s
		stringi_hash<smart_ptr<resource> > m_exports;

		// Items we import.
		array<import_info>	m_imports;

		// Movies we import from; hold a ref on these, to keep them alive
		array<smart_ptr<movie_definition> >	m_import_source_movies;

		// Bitmaps used in this movie; collected in one place to make
		// it possible for the host to manage them as textures.
		array<smart_ptr<bitmap_info> >	m_bitmap_list;

		create_bitmaps_flag	m_create_bitmaps;
		create_font_shapes_flag	m_create_font_shapes;

		rect	m_frame_size;
		float	m_frame_rate;
		int	m_frame_count;
		int	m_version;
		int	m_loading_frame;
		uint32	m_file_length;

		jpeg::input*	m_jpeg_in;


		movie_def_impl(create_bitmaps_flag cbf, create_font_shapes_flag cfs)
			:
			m_create_bitmaps(cbf),
			m_create_font_shapes(cfs),
			m_frame_rate(30.0f),
			m_frame_count(0),
			m_version(0),
			m_loading_frame(0),
			m_jpeg_in(0)
		{
		}

		~movie_def_impl()
		{
			// Release our playlist data.
			{for (int i = 0, n = m_playlist.size(); i < n; i++)
			{
				for (int j = 0, m = m_playlist[i].size(); j < m; j++)
				{
					delete m_playlist[i][j];
				}
			}}

			// Release init action data.
			{for (int i = 0, n = m_init_action_list.size(); i < n; i++)
			{
				for (int j = 0, m = m_init_action_list[i].size(); j < m; j++)
				{
					delete m_init_action_list[i][j];
				}
			}}

			assert(m_jpeg_in == NULL);	// It's supposed to be cleaned up in read()
		}


		movie_interface*	create_instance();

		// ...
		int	get_frame_count() const { return m_frame_count; }
		float	get_frame_rate() const { return m_frame_rate; }
		float	get_width_pixels() const { return ceilf(TWIPS_TO_PIXELS(m_frame_size.width())); }
		float	get_height_pixels() const { return ceilf(TWIPS_TO_PIXELS(m_frame_size.height())); }

		virtual int	get_version() const { return m_version; }

		virtual int	get_loading_frame() const { return m_loading_frame; }

		uint32	get_file_bytes() const { return m_file_length; }

		/* movie_def_impl */
		virtual create_bitmaps_flag	get_create_bitmaps() const
		// Returns DO_CREATE_BITMAPS if we're supposed to
		// initialize our bitmap infos, or DO_NOT_INIT_BITMAPS
		// if we're supposed to create blank placeholder
		// bitmaps (to be init'd later explicitly by the host
		// program).
		{
			return m_create_bitmaps;
		}

		/* movie_def_impl */
		virtual create_font_shapes_flag	get_create_font_shapes() const
		// Returns DO_LOAD_FONT_SHAPES if we're supposed to
		// initialize our font shape info, or
		// DO_NOT_LOAD_FONT_SHAPES if we're supposed to not
		// create any (vector) font glyph shapes, and instead
		// rely on precached textured fonts glyphs.
		{
			return m_create_font_shapes;
		}

		virtual void	add_bitmap_info(bitmap_info* bi)
		// All bitmap_info's used by this movie should be
		// registered with this API.
		{
			m_bitmap_list.push_back(bi);
		}


		virtual int	get_bitmap_info_count() const { return m_bitmap_list.size(); }
		virtual bitmap_info*	get_bitmap_info(int i) const
		{
			return m_bitmap_list[i].get_ptr();
		}

		virtual void	export_resource(const tu_string& symbol, resource* res)
		// Expose one of our resources under the given symbol,
		// for export.  Other movies can import it.
		{
			// SWF sometimes exports the same thing more than once!
			m_exports.set(symbol, res);
		}

		virtual smart_ptr<resource>	get_exported_resource(const tu_string& symbol)
		// Get the named exported resource, if we expose it.
		// Otherwise return NULL.
		{
			smart_ptr<resource>	res;
			m_exports.get(symbol, &res);
			return res;
		}

		virtual void	add_import(const char* source_url, int id, const char* symbol)
		// Adds an entry to a table of resources that need to
		// be imported from other movies.  Client code must
		// call resolve_import() later, when the source movie
		// has been loaded, so that the actual resource can be
		// used.
		{
			assert(in_import_table(id) == false);

			m_imports.push_back(import_info(source_url, id, symbol));
		}

		bool	in_import_table(int character_id)
		// Debug helper; returns true if the given
		// character_id is listed in the import table.
		{
			for (int i = 0, n = m_imports.size(); i < n; i++)
			{
				if (m_imports[i].m_character_id == character_id)
				{
					return true;
				}
			}
			return false;
		}

		virtual void	visit_imported_movies(import_visitor* visitor)
		// Calls back the visitor for each movie that we
		// import symbols from.
		{
			stringi_hash<bool>	visited;	// ugh!

			for (int i = 0, n = m_imports.size(); i < n; i++)
			{
				import_info&	inf = m_imports[i];
				if (visited.find(inf.m_source_url) == visited.end())
				{
					// Call back the visitor.
					visitor->visit(inf.m_source_url.c_str());
					visited.set(inf.m_source_url, true);
				}
			}
		}

		virtual void	resolve_import(const char* source_url, movie_definition* source_movie)
		// Grabs the stuff we want from the source movie.
		{
			// @@ should be safe, but how can we verify
			// it?  Compare a member function pointer, or
			// something?
			movie_def_impl*	def_impl = static_cast<movie_def_impl*>(source_movie);
			movie_definition_sub*	def = static_cast<movie_definition_sub*>(def_impl);

			// Iterate in reverse, since we remove stuff along the way.
			for (int i = m_imports.size() - 1; i >= 0; i--)
			{
				const import_info&	inf = m_imports[i];
				if (inf.m_source_url == source_url)
				{
					// Do the import.
					smart_ptr<resource> res = def->get_exported_resource(inf.m_symbol);
					bool	 imported = true;

					if (res == NULL)
					{
						log_error("import error: resource '%s' is not exported from movie '%s'\n",
							  inf.m_symbol.c_str(), source_url);
					}
					else if (font* f = res->cast_to_font())
					{
						// Add this shared font to our fonts.
						add_font(inf.m_character_id, f);
						imported = true;
					}
					else if (character_def* ch = res->cast_to_character_def())
					{
						// Add this character to our characters.
						add_character(inf.m_character_id, ch);
						imported = true;
					}
					else
					{
						log_error("import error: resource '%s' from movie '%s' has unknown type\n",
							  inf.m_symbol.c_str(), source_url);
					}

					if (imported)
					{
						m_imports.remove(i);

						// Hold a ref, to keep this source movie_definition alive.
						m_import_source_movies.push_back(source_movie);
					}
				}
			}
		}

		void	add_character(int character_id, character_def* c)
		{
			assert(c);
			m_characters.add(character_id, c);
		}

		character_def*	get_character_def(int character_id)
		{
#ifndef NDEBUG
			// make sure character_id is resolved
			if (in_import_table(character_id))
			{
				log_error("get_character_def(): character_id %d is still waiting to be imported\n",
					  character_id);
			}
#endif // not NDEBUG

			smart_ptr<character_def>	ch;
			m_characters.get(character_id, &ch);
			assert(ch == NULL || ch->get_ref_count() > 1);
			return ch.get_ptr();
		}

		bool	get_labeled_frame(const char* label, int* frame_number)
		// Returns 0-based frame #
		{
			return m_named_frames.get(label, frame_number);
		}

		void	add_font(int font_id, font* f)
		{
			assert(f);
			m_fonts.add(font_id, f);
		}

		font*	get_font(int font_id)
		{
#ifndef NDEBUG
			// make sure font_id is resolved
			if (in_import_table(font_id))
			{
				log_error("get_font(): font_id %d is still waiting to be imported\n",
					  font_id);
			}
#endif // not NDEBUG

			smart_ptr<font>	f;
			m_fonts.get(font_id, &f);
			assert(f == NULL || f->get_ref_count() > 1);
			return f.get_ptr();
		}

		bitmap_character_def*	get_bitmap_character(int character_id)
		{
			smart_ptr<bitmap_character_def>	ch;
			m_bitmap_characters.get(character_id, &ch);
			assert(ch == NULL || ch->get_ref_count() > 1);
			return ch.get_ptr();
		}

		void	add_bitmap_character(int character_id, bitmap_character_def* ch)
		{
			assert(ch);
			m_bitmap_characters.add(character_id, ch);

			add_bitmap_info(ch->get_bitmap_info());
		}

		sound_sample*	get_sound_sample(int character_id)
		{
			smart_ptr<sound_sample>	ch;
			m_sound_samples.get(character_id, &ch);
			assert(ch == NULL || ch->get_ref_count() > 1);
			return ch.get_ptr();
		}

		virtual void	add_sound_sample(int character_id, sound_sample* sam)
		{
			assert(sam);
			m_sound_samples.add(character_id, sam);
		}

		void	add_execute_tag(execute_tag* e)
		{
			assert(e);
			m_playlist[m_loading_frame].push_back(e);
		}

		void	add_init_action(int sprite_id, execute_tag* e)
		// Need to execute the given tag before entering the
		// currently-loading frame for the first time.
		//
		// @@ AFAIK, the sprite_id is totally pointless -- correct?
		{
			assert(e);
			m_init_action_list[m_loading_frame].push_back(e);
		}

		void	add_frame_name(const char* name)
		// Labels the frame currently being loaded with the
		// given name.  A copy of the name string is made and
		// kept in this object.
		{
			assert(m_loading_frame >= 0 && m_loading_frame < m_frame_count);

			tu_string	n = name;
			assert(m_named_frames.get(n, NULL) == false);	// frame should not already have a name (?)
			m_named_frames.add(n, m_loading_frame);	// stores 0-based frame #
		}

		void	set_jpeg_loader(jpeg::input* j_in)
		// Set an input object for later loading DefineBits
		// images (JPEG images without the table info).
		{
			assert(m_jpeg_in == NULL);
			m_jpeg_in = j_in;
		}

		jpeg::input*	get_jpeg_loader()
		// Get the jpeg input loader, to load a DefineBits
		// image (one without table info).
		{
			return m_jpeg_in;
		}


		virtual const array<execute_tag*>&	get_playlist(int frame_number) { return m_playlist[frame_number]; }

		/* movie_def_impl */
		virtual const array<execute_tag*>*	get_init_actions(int frame_number)
		{
			return &m_init_action_list[frame_number];
		}

		/* movie_def_impl */
		void	read(tu_file* in)
		// Read a .SWF movie.
		{
			Uint32	file_start_pos = in->get_position();
			Uint32	header = in->read_le32();
			m_file_length = in->read_le32();
			Uint32	file_end_pos = file_start_pos + m_file_length;

			m_version = (header >> 24) & 255;
			if ((header & 0x0FFFFFF) != 0x00535746
			    && (header & 0x0FFFFFF) != 0x00535743)
			{
				// ERROR
				log_error("gnash::movie_def_impl::read() -- file does not start with a SWF header!\n");
				return;
			}
			bool	compressed = (header & 255) == 'C';

			IF_VERBOSE_PARSE(log_msg("version = %d, file_length = %d\n", m_version, m_file_length));

			tu_file*	original_in = NULL;
			if (compressed)
			{
#if TU_CONFIG_LINK_TO_ZLIB == 0
				log_error("movie_def_impl::read(): unable to read zipped SWF data; TU_CONFIG_LINK_TO_ZLIB is 0\n");
				return;
#endif

				IF_VERBOSE_PARSE(log_msg("file is compressed.\n"));
				original_in = in;

				// Uncompress the input as we read it.
				in = zlib_adapter::make_inflater(original_in);

				// Subtract the size of the 8-byte header, since
				// it's not included in the compressed
				// stream length.
				file_end_pos = m_file_length - 8;
			}

			stream	str(in);

			m_frame_size.read(&str);
			m_frame_rate = str.read_u16() / 256.0f;
			m_frame_count = str.read_u16();

			m_playlist.resize(m_frame_count);
			m_init_action_list.resize(m_frame_count);

			IF_VERBOSE_PARSE(m_frame_size.print());
			IF_VERBOSE_PARSE(log_msg("frame rate = %f, frames = %d\n", m_frame_rate, m_frame_count));

			while ((Uint32) str.get_position() < file_end_pos)
			{
				int	tag_type = str.open_tag();

				if (s_progress_function != NULL)
				{
					s_progress_function((Uint32) str.get_position(), file_end_pos);
				}

				loader_function	lf = NULL;
				//IF_VERBOSE_PARSE(log_msg("tag_type = %d\n", tag_type));
				if (tag_type == 1)
				{
					// show frame tag -- advance to the next frame.
					IF_VERBOSE_PARSE(log_msg("  show_frame\n"));
					m_loading_frame++;
				}
				else if (s_tag_loaders.get(tag_type, &lf))
				{
					// call the tag loader.  The tag loader should add
					// characters or tags to the movie data structure.
					(*lf)(&str, tag_type, this);

				} else {
					// no tag loader for this tag type.
					IF_VERBOSE_PARSE(log_msg("*** no tag loader for type %d\n", tag_type));
					IF_VERBOSE_PARSE(dump_tag_bytes(&str));
				}

				str.close_tag();

				if (tag_type == 0)
				{
					if ((unsigned int) str.get_position() != file_end_pos)
					{
						// Safety break, so we don't read past the end of the
						// movie.
						log_msg("warning: hit stream-end tag, but not at the "
							"end of the file yet; stopping for safety\n");
						break;
					}
				}
			}

			if (m_jpeg_in)
			{
				delete m_jpeg_in;
				m_jpeg_in = NULL;
			}

			if (original_in)
			{
				// Done with the zlib_adapter.
				delete in;
			}
		}


		/* movie_def_impl */
		void	get_owned_fonts(array<font*>* fonts)
		// Fill up *fonts with fonts that we own.
		{
			assert(fonts);
			fonts->resize(0);

			array<int>	font_ids;

			for (hash<int, smart_ptr<font> >::iterator it = m_fonts.begin();
			     it != m_fonts.end();
			     ++it)
			{
				font*	f = it->second.get_ptr();
				if (f->get_owning_movie() == this)
				{
					// Sort by character id, so the ordering is
					// consistent for cache read/write.
					int	id = it->first;

					// Insert in correct place.
					int	insert;
					for (insert = 0; insert < font_ids.size(); insert++)
					{
						if (font_ids[insert] > id)
						{
							// We want to insert here.
							break;
						}
					}
					fonts->insert(insert, f);
					font_ids.insert(insert, id);
				}
			}
		}


		/* movie_def_impl */
		void	generate_font_bitmaps()
		// Generate bitmaps for our fonts, if necessary.
		{
			// Collect list of fonts.
			array<font*>	fonts;
			get_owned_fonts(&fonts);
			fontlib::generate_font_bitmaps(fonts, this);
		}


		// Increment this when the cache data format changes.
		#define CACHE_FILE_VERSION 4


		/* movie_def_impl */
		void	output_cached_data(tu_file* out, const cache_options& options)
		// Dump our cached data into the given stream.
		{
			// Write a little header.
			char	header[5];
			strcpy(header, "gscX");
			header[3] = CACHE_FILE_VERSION;
			compiler_assert(CACHE_FILE_VERSION < 256);

			out->write_bytes(header, 4);

			// Write font data.
			array<font*>	fonts;
			get_owned_fonts(&fonts);
			fontlib::output_cached_data(out, fonts, this, options);

			// Write character data.
			{for (hash<int, smart_ptr<character_def> >::iterator it = m_characters.begin();
			     it != m_characters.end();
			     ++it)
			{
				out->write_le16(it->first);
				it->second->output_cached_data(out, options);
			}}

			out->write_le16((Sint16) -1);	// end of characters marker
		}


		/* movie_def_impl */
		void	input_cached_data(tu_file* in)
		// Read in cached data and use it to prime our loaded characters.
		{
			// Read the header & check version.
			unsigned char	header[4];
			in->read_bytes(header, 4);
			if (header[0] != 'g' || header[1] != 's' || header[2] != 'c')
			{
				log_error("cache file does not have the correct format; skipping\n");
				return;
			}
			else if (header[3] != CACHE_FILE_VERSION)
			{
				log_error(
					"cached data is version %d, but we require version %d; skipping\n",
					int(header[3]), CACHE_FILE_VERSION);
				return;
			}

			// Read the cached font data.
			array<font*>	fonts;
			get_owned_fonts(&fonts);
			fontlib::input_cached_data(in, fonts, this);

			// Read the cached character data.
			for (;;)
			{
				if (in->get_error() != TU_FILE_NO_ERROR)
				{
					log_error("error reading cache file (characters); skipping\n");
					return;
				}
				if (in->get_eof())
				{
					log_error("unexpected eof reading cache file (characters); skipping\n");
					return;
				}

				Sint16	id = in->read_le16();
				if (id == (Sint16) -1) { break; }	// done

				smart_ptr<character_def> ch;
				m_characters.get(id, &ch);
				if (ch != NULL)
				{
					ch->input_cached_data(in);
				}
				else
				{
					log_error("sync error in cache file (reading characters)!  "
						  "Skipping rest of cache data.\n");
					return;
				}
			}
		}

	};


	//
	// movie_root
	//
	// Global, shared root state for a movie and all its characters.
	//

	struct movie_root : public movie_interface
	{
		smart_ptr<movie_def_impl>	m_def;
		smart_ptr<movie>	m_movie;
		int			m_viewport_x0, m_viewport_y0, m_viewport_width, m_viewport_height;
		float			m_pixel_scale;

		rgba			m_background_color;
		float			m_timer;
		int			m_mouse_x, m_mouse_y, m_mouse_buttons;
		void *			m_userdata;
		movie::drag_state	m_drag_state;	// @@ fold this into m_mouse_button_state?
		mouse_button_state	m_mouse_button_state;
		bool			m_on_event_load_called;

		// Flags for event handlers
		bool			m_on_event_xmlsocket_ondata_called;
		bool			m_on_event_xmlsocket_onxml_called;
		bool			m_on_event_load_progress_called;
		array<Timer *>	m_interval_timers;

		movie_root(movie_def_impl* def)
			:
			m_def(def),
			m_movie(NULL),
			m_viewport_x0(0),
			m_viewport_y0(0),
			m_viewport_width(1),
			m_viewport_height(1),
			m_pixel_scale(1.0f),
			m_background_color(0, 0, 0, 255),
			m_timer(0.0f),
			m_mouse_x(0),
			m_mouse_y(0),
			m_mouse_buttons(0),
			m_userdata(NULL),
			m_on_event_load_called(false),
			m_on_event_xmlsocket_ondata_called(false),
			m_on_event_xmlsocket_onxml_called(false),
			m_on_event_load_progress_called(false)
		{
			assert(m_def != NULL);

			set_display_viewport(0, 0, (int) m_def->get_width_pixels(), (int) m_def->get_height_pixels());
		}

		~movie_root()
		{
			assert(m_def != NULL);
			m_movie = NULL;
			m_def = NULL;
		}

		// @@ should these delegate to m_movie?  Probably...
		virtual void	set_member(const tu_stringi& name, const as_value& val) {}
		virtual bool	get_member(const tu_stringi& name, as_value* val) { return false; }
		virtual movie*	to_movie() { assert(0); return 0; } // @@ should this return m_movie.get_ptr()?
		

		void	set_root_movie(movie* root_movie)
		{
			m_movie = root_movie;
			assert(m_movie != NULL);
		}

		void	set_display_viewport(int x0, int y0, int w, int h)
		{
			m_viewport_x0 = x0;
			m_viewport_y0 = y0;
			m_viewport_width = w;
			m_viewport_height = h;

			// Recompute pixel scale.
			float	scale_x = m_viewport_width / TWIPS_TO_PIXELS(m_def->m_frame_size.width());
			float	scale_y = m_viewport_height / TWIPS_TO_PIXELS(m_def->m_frame_size.height());
			m_pixel_scale = fmax(scale_x, scale_y);
		}


		void	notify_mouse_state(int x, int y, int buttons)
		// The host app uses this to tell the movie where the
		// user's mouse pointer is.
		{
			m_mouse_x = x;
			m_mouse_y = y;
			m_mouse_buttons = buttons;
		}

		virtual void	get_mouse_state(int* x, int* y, int* buttons)
		// Use this to retrieve the last state of the mouse, as set via
		// notify_mouse_state().  Coordinates are in PIXELS, NOT TWIPS.
		{
			assert(x);
			assert(y);
			assert(buttons);

			*x = m_mouse_x;
			*y = m_mouse_y;
			*buttons = m_mouse_buttons;
		}

		movie*	get_root_movie() { return m_movie.get_ptr(); }


		void	stop_drag()
		{
			m_drag_state.m_character = NULL;
		}


		movie_definition*	get_movie_definition() { return m_movie->get_movie_definition(); }

		uint32	get_file_bytes() const
		{
			return m_def->get_file_bytes();
		}

		virtual int    add_interval_timer(void *timer)
		{
			Timer *ptr = static_cast<Timer *>(timer);
			
			m_interval_timers.push_back(ptr);
			return m_interval_timers.size();
		}
	
		virtual void    clear_interval_timer(int x)
		{
			m_interval_timers.remove(x-1);
			//m_interval_timers[x]->clearInterval();
		}
	
		virtual void    do_something(void *timer)
		{
			log_msg("FIXME: %s: unimplemented\n", __FUNCTION__);
		}
		
		// 0-based!!
		int	get_current_frame() const { return m_movie->get_current_frame(); }
		float	get_frame_rate() const { return m_def->get_frame_rate(); }

		virtual float	get_pixel_scale() const
		// Return the size of a logical movie pixel as
		// displayed on-screen, with the current device
		// coordinates.
		{
			return m_pixel_scale;
		}

		// @@ Is this one necessary?
		character*	get_character(int character_id)
		{
			return m_movie->get_character(character_id);
		}

		void	set_background_color(const rgba& color)
		{
			m_background_color = color;
		}

		void	set_background_alpha(float alpha)
		{
			m_background_color.m_a = iclamp(frnd(alpha * 255.0f), 0, 255);
		}

		float	get_background_alpha() const
		{
			return m_background_color.m_a / 255.0f;
		}

		float	get_timer() const { return m_timer; }

		void	restart() { m_movie->restart(); }
		
		void	advance(float delta_time)
		{
			int i;
			if (m_on_event_load_called == false)
			{
				// Must do loading events.  For child sprites this is
				// done by the dlist, but root movies don't get added
				// to a dlist, so we do it here.
				m_on_event_load_called = true;
				m_movie->on_event_load();
			}
#if 0
			// Must check the socket connection for data
			if (m_on_event_xmlsocket_ondata_called == true) {
				m_movie->on_event_xmlsocket_ondata();
			}
			
			if (m_on_event_xmlsocket_onxml_called == true) {
				m_movie->on_event_xmlsocket_onxml();
			}


			// Must check the progress of the MovieClip being loaded
			if (m_on_event_load_progress_called == true) {
				m_movie->on_event_load_progress();				
			}
#endif
			if (m_interval_timers.size() > 0) {
				for (i=0; i<m_interval_timers.size(); i++) {
					if (m_interval_timers[i]->expired()) {
						// printf("FIXME: Interval Timer Expired!\n");
						//m_movie->on_event_interval_timer();
						m_movie->do_something(m_interval_timers[i]);
						// clear_interval_timer(m_interval_timers[i]->getIntervalID()); // FIXME: we shouldn't really disable the timer here
					}
				}
			}
			
			
			m_timer += delta_time;
			// @@ TODO handle multi-frame catch-up stuff
			// here, and make it optional.  Make
			// movie::advance() a fixed framerate w/ no
			// dt.

			// Handle the mouse.
			m_mouse_button_state.m_topmost_entity =
				m_movie->get_topmost_mouse_entity(PIXELS_TO_TWIPS(m_mouse_x), PIXELS_TO_TWIPS(m_mouse_y));
			m_mouse_button_state.m_mouse_button_state_current = (m_mouse_buttons & 1);
			generate_mouse_button_events(&m_mouse_button_state);

			m_movie->advance(delta_time);
		}

		// 0-based!!
		void	goto_frame(int target_frame_number) { m_movie->goto_frame(target_frame_number); }

		virtual bool	has_looped() const { return m_movie->has_looped(); }

		void	display()
		{
			if (m_movie->get_visible() == false)
			{
				// Don't display.
				return;
			}

			gnash::render::begin_display(
				m_background_color,
				m_viewport_x0, m_viewport_y0,
				m_viewport_width, m_viewport_height,
				m_def->m_frame_size.m_x_min, m_def->m_frame_size.m_x_max,
				m_def->m_frame_size.m_y_min, m_def->m_frame_size.m_y_max);

			m_movie->display();

			gnash::render::end_display();
		}

		virtual bool	goto_labeled_frame(const char* label)
		{
			int	target_frame = -1;
			if (m_def->get_labeled_frame(label, &target_frame))
			{
				goto_frame(target_frame);
				return true;
			}
			else
			{
				IF_VERBOSE_ACTION(
					log_error("error: movie_impl::goto_labeled_frame('%s') unknown label\n", label));
				return false;
			}
		}

		virtual void	set_play_state(play_state s) { m_movie->set_play_state(s); }
		virtual play_state	get_play_state() const { return m_movie->get_play_state(); }

		/* movie_root */
		virtual void	set_variable(const char* path_to_var, const char* new_value)
		{
			m_movie->set_variable(path_to_var, new_value);
		}

		/* movie_root */
		virtual void	set_variable(const char* path_to_var, const wchar_t* new_value)
		{
			m_movie->set_variable(path_to_var, new_value);
		}

		virtual const char*	get_variable(const char* path_to_var) const
		{
			return m_movie->get_variable(path_to_var);
		}

		/*movie_root*/
		// For ActionScript interfacing convenience.
		virtual const char*	call_method(const char* method_name, const char* method_arg_fmt, ...)
		{
			assert(m_movie != NULL);

			va_list	args;
			va_start(args, method_arg_fmt);
			const char*	result = m_movie->call_method_args(method_name, method_arg_fmt, args);
			va_end(args);

			return result;
		}

		/*movie_root*/
		virtual const char*	call_method_args(const char* method_name, const char* method_arg_fmt, va_list args)
		{
			assert(m_movie != NULL);
			return m_movie->call_method_args(method_name, method_arg_fmt, args);
		}

		virtual void	set_visible(bool visible) { m_movie->set_visible(visible); }
		virtual bool	get_visible() const { return m_movie->get_visible(); }

		virtual void * get_userdata() { return m_userdata; }
		virtual void set_userdata(void * ud ) { m_userdata = ud;  }

		virtual void	attach_display_callback(const char* path_to_object, void (*callback)(void* user_ptr), void* user_ptr)
		{
			m_movie->attach_display_callback(path_to_object, callback, user_ptr);
		}
	};


	static void	ensure_loaders_registered()
	{
		static bool	s_registered = false;
	
		if (s_registered == false)
		{
			// Register the standard loaders.
			s_registered = true;
			register_tag_loader(0, end_loader);
			register_tag_loader(2, define_shape_loader);
			register_tag_loader(4, place_object_2_loader);
			register_tag_loader(5, remove_object_2_loader);
			register_tag_loader(6, define_bits_jpeg_loader);
			register_tag_loader(7, button_character_loader);
			register_tag_loader(8, jpeg_tables_loader);
			register_tag_loader(9, set_background_color_loader);
			register_tag_loader(10, define_font_loader);
			register_tag_loader(11, define_text_loader);
			register_tag_loader(12, do_action_loader);
			register_tag_loader(13, define_font_info_loader);
			register_tag_loader(14, define_sound_loader);
			register_tag_loader(15, start_sound_loader);
			register_tag_loader(17, button_sound_loader);
			register_tag_loader(20, define_bits_lossless_2_loader);
			register_tag_loader(21, define_bits_jpeg2_loader);
			register_tag_loader(22, define_shape_loader);
			register_tag_loader(24, null_loader);	// "protect" tag; we're not an authoring tool so we don't care.
			register_tag_loader(26, place_object_2_loader);
			register_tag_loader(28, remove_object_2_loader);
			register_tag_loader(32, define_shape_loader);
			register_tag_loader(33, define_text_loader);
			register_tag_loader(37, define_edit_text_loader);
			register_tag_loader(34, button_character_loader);
			register_tag_loader(35, define_bits_jpeg3_loader);
			register_tag_loader(36, define_bits_lossless_2_loader);
			register_tag_loader(39, sprite_loader);
			register_tag_loader(43, frame_label_loader);
			register_tag_loader(46, define_shape_morph_loader);
			register_tag_loader(48, define_font_loader);
			register_tag_loader(56, export_loader);
			register_tag_loader(57, import_loader);
			register_tag_loader(59, do_init_action_loader);   
		}
	}



	void	get_movie_info(
		const char* filename,
		int* version,
		int* width,
		int* height,
		float* frames_per_second,
		int* frame_count,
		int* tag_count
		)
	// Attempt to read the header of the given .swf movie file.
	// Put extracted info in the given vars.
	// Sets *version to 0 if info can't be extracted.
	{
		if (s_opener_function == NULL)
		{
			log_error("error: get_movie_info(): no file opener function registered\n");
			if (version) *version = 0;
			return;
		}

		tu_file*	in = s_opener_function(filename);
		if (in == NULL || in->get_error() != TU_FILE_NO_ERROR)
		{
			log_error("error: get_movie_info(): can't open '%s'\n", filename);
			if (version) *version = 0;
			delete in;
			return;
		}

		Uint32	file_start_pos = in->get_position();
		Uint32	header = in->read_le32();
		Uint32	file_length = in->read_le32();
		Uint32	file_end_pos = file_start_pos + file_length;

		int	local_version = (header >> 24) & 255;
		if ((header & 0x0FFFFFF) != 0x00535746
		    && (header & 0x0FFFFFF) != 0x00535743)
		{
			// ERROR
			log_error("error: get_movie_info(): file '%s' does not start with a SWF header!\n", filename);
			if (version) *version = 0;
			delete in;
			return;
		}
		bool	compressed = (header & 255) == 'C';

		tu_file*	original_in = NULL;
		if (compressed)
		{
#if TU_CONFIG_LINK_TO_ZLIB == 0
			log_error("get_movie_info(): can't read zipped SWF data; TU_CONFIG_LINK_TO_ZLIB is 0!\n");
			return;
#endif
			original_in = in;

			// Uncompress the input as we read it.
			in = zlib_adapter::make_inflater(original_in);

			// Subtract the size of the 8-byte header, since
			// it's not included in the compressed
			// stream length.
			file_length -= 8;
		}

		stream	str(in);

		rect	frame_size;
		frame_size.read(&str);

		float	local_frame_rate = str.read_u16() / 256.0f;
		int	local_frame_count = str.read_u16();

		if (version) *version = local_version;
		if (width) *width = int(frame_size.width() / 20.0f + 0.5f);
		if (height) *height = int(frame_size.height() / 20.0f + 0.5f);
		if (frames_per_second) *frames_per_second = local_frame_rate;
		if (frame_count) *frame_count = local_frame_count;

		if (tag_count)
		{
			// Count tags.
			int local_tag_count = 0;
			while ((Uint32) str.get_position() < file_end_pos)
			{
				str.open_tag();
				str.close_tag();
				local_tag_count++;
			}
			*tag_count = local_tag_count;
		}

		delete in;
		delete original_in;
	}



	movie_definition*	create_movie(const char* filename)
	// Create the movie definition from the specified .swf file.
	{
		return create_movie_sub(filename);
	}


	movie_definition_sub*	create_movie_sub(const char* filename)
	{
		if (s_opener_function == NULL)
		{
			// Don't even have a way to open the file.
			log_error("error: no file opener function; can't create movie.  "
				  "See gnash::register_file_opener_callback\n");
			return NULL;
		}

		tu_file* in = s_opener_function(filename);
		if (in == NULL)
		{
			log_error("failed to open '%s'; can't create movie.\n", filename);
			return NULL;
		}
		else if (in->get_error())
		{
			log_error("error: file opener can't open '%s'\n", filename);
			return NULL;
		}

		ensure_loaders_registered();

		movie_def_impl*	m = new movie_def_impl(DO_LOAD_BITMAPS, DO_LOAD_FONT_SHAPES);
		m->read(in);

		delete in;

		if (m && s_use_cache_files)
		{
			// Try to load a .gsc file.
			tu_string	cache_filename(filename);
			cache_filename += ".gsc";
			tu_file*	cache_in = s_opener_function(cache_filename.c_str());
			if (cache_in == NULL
			    || cache_in->get_error() != TU_FILE_NO_ERROR)
			{
				// Can't open cache file; don't sweat it.
				IF_VERBOSE_PARSE(log_msg("note: couldn't open cache file '%s'\n", cache_filename.c_str()));

				m->generate_font_bitmaps();	// can't read cache, so generate font texture data.
			}
			else
			{
				// Load the cached data.
				m->input_cached_data(cache_in);
			}

			delete cache_in;
		}

		m->add_ref();
		return m;
	}


	static bool	s_no_recurse_while_loading = false;	// @@ TODO get rid of this; make it the normal mode.


	movie_definition*	create_movie_no_recurse(
		tu_file* in,
		create_bitmaps_flag cbf,
		create_font_shapes_flag cfs)
	{
		ensure_loaders_registered();

		// @@ TODO make no_recurse the standard way to load.
		// In create_movie(), use the visit_ API to keep
		// visiting dependent movies, until everything is
		// loaded.  That way we only have one code path and
		// the resource_proxy stuff gets tested.
		s_no_recurse_while_loading = true;

		movie_def_impl*	m = new movie_def_impl(cbf, cfs);
		m->read(in);

		s_no_recurse_while_loading = false;

		m->add_ref();
		return m;
	}


	//
	// global gnash management
	//


	// For built-in sprite ActionScript methods.
	as_object*	s_sprite_builtins = 0;	// shared among all sprites.
	static void	sprite_builtins_init();
	static void	sprite_builtins_clear();


	void	clear()
	// Maximum release of resources.
	{
		clear_library();
		sprite_builtins_clear();
		fontlib::clear();
		action_clear();
	}


	//
	// library stuff, for sharing resources among different movies.
	//


	static stringi_hash< smart_ptr<movie_definition_sub> >	s_movie_library;
	static hash< movie_definition_sub*, smart_ptr<movie_interface> >	s_movie_library_inst;
	static array<movie_interface*> s_extern_sprites;
	static movie_interface* s_current_root;

	static tu_string s_workdir;

	void save_extern_movie(movie_interface* m)
	{
		s_extern_sprites.push_back(m);
	}

//#if 0
	void delete_unused_root()
	{
		for (int i = 0; i < s_extern_sprites.size(); i++)
		{
			movie_interface* root_m = s_extern_sprites[i];
			movie* m = root_m->get_root_movie();
      
			if (m->get_ref_count() < 2)
			{
				IF_VERBOSE_ACTION(log_msg("extern movie deleted\n"));
				s_extern_sprites.remove(i);
				i--;
				root_m->drop_ref();
			}
		}
	}
//#endif // 0

	movie_interface* get_current_root()
	{
		assert(s_current_root != NULL);
		return s_current_root;
	}

	void set_current_root(movie_interface* m)
	{
		assert(m != NULL);
		s_current_root = m;
	}

	const char* get_workdir()
	{
		return s_workdir.c_str();
	}

	void set_workdir(const char* dir)
	{
		assert(dir != NULL);
		s_workdir = dir;
	}

	void	clear_library()
	// Drop all library references to movie_definitions, so they
	// can be cleaned up.
	{
		s_movie_library.clear();
		s_movie_library_inst.clear();
	}

	movie_definition*	create_library_movie(const char* filename)
	// Try to load a movie from the given url, if we haven't
	// loaded it already.  Add it to our library on success, and
	// return a pointer to it.
	{
		return create_library_movie_sub(filename);
	}


	movie_definition_sub*	create_library_movie_sub(const char* filename)
	{
		tu_string	fn(filename);

		// Is the movie already in the library?
		{
			smart_ptr<movie_definition_sub>	m;
			s_movie_library.get(fn, &m);
			if (m != NULL)
			{
				// Return cached movie.
				m->add_ref();
				return m.get_ptr();
			}
		}

		// Try to open a file under the filename.
		movie_definition_sub*	mov = create_movie_sub(filename);

		if (mov == NULL)
		{
			log_error("error: couldn't load library movie '%s'\n", filename);
			return NULL;
		}
		else
		{
			s_movie_library.add(fn, mov);
		}

		mov->add_ref();
		return mov;
	}
	
	movie_interface*	create_library_movie_inst(movie_definition* md)
	{
		return create_library_movie_inst_sub((movie_definition_sub*)md);
	}


	movie_interface*	create_library_movie_inst_sub(movie_definition_sub* md)
	{
		// Is the movie instance already in the library?
		{
			smart_ptr<movie_interface>	m;
			s_movie_library_inst.get(md, &m);
			if (m != NULL)
			{
				// Return cached movie instance.
				m->add_ref();
				return m.get_ptr();
			}
		}

		// Try to create movie interface
		movie_interface* mov = md->create_instance();

		if (mov == NULL)
		{
			log_error("error: couldn't create instance\n");

			return NULL;
		}
		else
		{
			s_movie_library_inst.add(md, mov);
		}

		mov->add_ref();
		return mov;
	}


	void	precompute_cached_data(movie_definition* movie_def)
	// Fill in cached data in movie_def.
	// @@@@ NEEDS TESTING -- MIGHT BE BROKEN!!!
	{
		assert(movie_def != NULL);

		// Temporarily install null render and sound handlers,
		// so we don't get output during preprocessing.
		//
		// Use automatic struct var to make sure we restore
		// when exiting the function.
		struct save_stuff
		{
			render_handler*	m_original_rh;
			sound_handler*	m_original_sh;

			save_stuff()
			{
				// Save.
				m_original_rh = get_render_handler();
				m_original_sh = get_sound_handler();
				set_render_handler(NULL);
				set_sound_handler(NULL);
			}

			~save_stuff()
			{
				// Restore.
				set_render_handler(m_original_rh);
				set_sound_handler(m_original_sh);
			}
		} save_stuff_instance;

		// Need an instance.
		gnash::movie_interface*	m = movie_def->create_instance();
		if (m == NULL)
		{
			log_error("error: precompute_cached_data can't create instance of movie\n");
			return;
		}
		
		// Run through the movie's frames.
		//
		// @@ there might be cleaner ways to do this; e.g. do
		// execute_frame_tags(i, true) on movie and all child
		// sprites.
		int	kick_count = 0;
		for (;;)
		{
			// @@ do we also have to run through all sprite frames
			// as well?
			//
			// @@ also, ActionScript can rescale things
			// dynamically -- we can't really do much about that I
			// guess?
			//
			// @@ Maybe we should allow the user to specify some
			// safety margin on scaled shapes.

			int	last_frame = m->get_current_frame();
			m->advance(0.010f);
			m->display();

			if (m->get_current_frame() == movie_def->get_frame_count() - 1)
			{
				// Done.
				break;
			}

			if (m->get_play_state() == gnash::movie_interface::STOP)
			{
				// Kick the movie.
				//printf("kicking movie, kick ct = %d\n", kick_count);
				m->goto_frame(last_frame + 1);
				m->set_play_state(gnash::movie_interface::PLAY);
				kick_count++;

				if (kick_count > 10)
				{
					//printf("movie is stalled; giving up on playing it through.\n");
					break;
				}
			}
			else if (m->get_current_frame() < last_frame)
			{
				// Hm, apparently we looped back.  Skip ahead...
				log_error("loop back; jumping to frame %d\n", last_frame);
				m->goto_frame(last_frame + 1);
			}
			else
			{
				kick_count = 0;
			}
		}

		m->drop_ref();
	}


	//
	// Some tag implementations
	//


	void	null_loader(stream* in, int tag_type, movie_definition_sub* m)
	// Silently ignore the contents of this tag.
	{
	}

	void	frame_label_loader(stream* in, int tag_type, movie_definition_sub* m)
	// Label the current frame of m with the name from the stream.
	{
		char*	n = in->read_string();
		m->add_frame_name(n);
		delete [] n;
	}

	struct set_background_color : public execute_tag
	{
		rgba	m_color;

		void	execute(movie* m)
		{
			float	current_alpha = m->get_background_alpha();
			m_color.m_a = frnd(current_alpha * 255.0f);
			m->set_background_color(m_color);
		}

		void	execute_state(movie* m)
		{
			execute(m);
		}

		void	read(stream* in)
		{
			m_color.read_rgb(in);

			IF_VERBOSE_PARSE(log_msg("  set_background_color: (%d %d %d)\n",
						 m_color.m_r, m_color.m_g, m_color.m_b));
		}
	};


	void	set_background_color_loader(stream* in, int tag_type, movie_definition_sub* m)
	{
		assert(tag_type == 9);
		assert(m);

		set_background_color*	t = new set_background_color;
		t->read(in);

		m->add_execute_tag(t);
	}

#if 0
	// Bitmap character
	struct bitmap_character : public bitmap_character_def
	{
		bitmap_character(bitmap_info* bi)
			:
			m_bitmap_info(bi)
		{
		}

// 		bitmap_character(image::rgb* image)
// 		{
// 			assert(image != 0);

// 			// Create our bitmap info, from our image.
// 			m_bitmap_info = gnash::render::create_bitmap_info_rgb(image);
// 		}

// 		bitmap_character(image::rgba* image)
// 		{
// 			assert(image != 0);

// 			// Create our bitmap info, from our image.
// 			m_bitmap_info = gnash::render::create_bitmap_info_rgba(image);
// 		}

		gnash::bitmap_info*	get_bitmap_info()
		{
			return m_bitmap_info.get_ptr();
		}

	private:
		smart_ptr<gnash::bitmap_info>	m_bitmap_info;
	};
#endif

	void	jpeg_tables_loader(stream* in, int tag_type, movie_definition_sub* m)
	// Load JPEG compression tables that can be used to load
	// images further along in the stream.
	{
		assert(tag_type == 8);

#if TU_CONFIG_LINK_TO_JPEGLIB
		jpeg::input*	j_in = jpeg::input::create_swf_jpeg2_header_only(in->get_underlying_stream());
		assert(j_in);

		m->set_jpeg_loader(j_in);
#endif // TU_CONFIG_LINK_TO_JPEGLIB
	}


	void	define_bits_jpeg_loader(stream* in, int tag_type, movie_definition_sub* m)
	// A JPEG image without included tables; those should be in an
	// existing jpeg::input object stored in the movie.
	{
		assert(tag_type == 6);

		Uint16	character_id = in->read_u16();

		//
		// Read the image data.
		//
		bitmap_info*	bi = NULL;

		if (m->get_create_bitmaps() == DO_LOAD_BITMAPS)
		{
#if TU_CONFIG_LINK_TO_JPEGLIB
			jpeg::input*	j_in = m->get_jpeg_loader();
			assert(j_in);
			j_in->discard_partial_buffer();

			image::rgb*	im = image::read_swf_jpeg2_with_tables(j_in);
			bi = render::create_bitmap_info_rgb(im);
			delete im;
#else
			log_error("gnash is not linked to jpeglib -- can't load jpeg image data!\n");
			bi = render::create_bitmap_info_empty();
#endif
		}
		else
		{
			bi = render::create_bitmap_info_empty();
		}

		assert(bi->get_ref_count() == 0);

		bitmap_character*	ch = new bitmap_character(bi);

		m->add_bitmap_character(character_id, ch);
	}


	void	define_bits_jpeg2_loader(stream* in, int tag_type, movie_definition_sub* m)
	{
		assert(tag_type == 21);
		
		Uint16	character_id = in->read_u16();

		IF_VERBOSE_PARSE(log_msg("  define_bits_jpeg2_loader: charid = %d pos = 0x%x\n", character_id, in->get_position()));

		//
		// Read the image data.
		//
		
		bitmap_info*	bi = NULL;

		if (m->get_create_bitmaps() == DO_LOAD_BITMAPS)
		{
#if TU_CONFIG_LINK_TO_JPEGLIB
			image::rgb* im = image::read_swf_jpeg2(in->get_underlying_stream());
			bi = render::create_bitmap_info_rgb(im);
			delete im;
#else
			log_error("gnash is not linked to jpeglib -- can't load jpeg image data!\n");
			bi = render::create_bitmap_info_empty();
#endif
		}
		else
		{
			bi = render::create_bitmap_info_empty();
		}

		assert(bi->get_ref_count() == 0);

		bitmap_character*	ch = new bitmap_character(bi);

		m->add_bitmap_character(character_id, ch);
	}


#if TU_CONFIG_LINK_TO_ZLIB
	void	inflate_wrapper(tu_file* in, void* buffer, int buffer_bytes)
	// Wrapper function -- uses Zlib to uncompress in_bytes worth
	// of data from the input file into buffer_bytes worth of data
	// into *buffer.
	{
		assert(in);
		assert(buffer);
		assert(buffer_bytes > 0);

		int err;
		z_stream d_stream; /* decompression stream */

		d_stream.zalloc = (alloc_func)0;
		d_stream.zfree = (free_func)0;
		d_stream.opaque = (voidpf)0;

		d_stream.next_in  = 0;
		d_stream.avail_in = 0;

		d_stream.next_out = (Byte*) buffer;
		d_stream.avail_out = (uInt) buffer_bytes;

		err = inflateInit(&d_stream);
		if (err != Z_OK) {
			log_error("error: inflate_wrapper() inflateInit() returned %d\n", err);
			return;
		}

		Uint8	buf[1];

		for (;;) {
			// Fill a one-byte (!) buffer.
			buf[0] = in->read_byte();
			d_stream.next_in = &buf[0];
			d_stream.avail_in = 1;

			err = inflate(&d_stream, Z_SYNC_FLUSH);
			if (err == Z_STREAM_END) break;
			if (err != Z_OK)
			{
				log_error("error: inflate_wrapper() inflate() returned %d\n", err);
			}
		}

		err = inflateEnd(&d_stream);
		if (err != Z_OK)
		{
			log_error("error: inflate_wrapper() inflateEnd() return %d\n", err);
		}
	}
#endif // TU_CONFIG_LINK_TO_ZLIB


	void	define_bits_jpeg3_loader(stream* in, int tag_type, movie_definition_sub* m)
	// loads a define_bits_jpeg3 tag. This is a jpeg file with an alpha
	// channel using zlib compression.
	{
		assert(tag_type == 35);

		Uint16	character_id = in->read_u16();

		IF_VERBOSE_PARSE(log_msg("  define_bits_jpeg3_loader: charid = %d pos = 0x%x\n", character_id, in->get_position()));

		Uint32	jpeg_size = in->read_u32();
		Uint32	alpha_position = in->get_position() + jpeg_size;

		bitmap_info*	bi = NULL;

		if (m->get_create_bitmaps() == DO_LOAD_BITMAPS)
		{
#if TU_CONFIG_LINK_TO_JPEGLIB == 0 || TU_CONFIG_LINK_TO_ZLIB == 0
			log_error("gnash is not linked to jpeglib/zlib -- can't load jpeg/zipped image data!\n");
			bi = render::create_bitmap_info_empty();
#else
			//
			// Read the image data.
			//
		
			// Read rgb data.
			image::rgba*	im = image::read_swf_jpeg3(in->get_underlying_stream());

			// Read alpha channel.
			in->set_position(alpha_position);

			int	buffer_bytes = im->m_width * im->m_height;
			Uint8*	buffer = new Uint8[buffer_bytes];

			inflate_wrapper(in->get_underlying_stream(), buffer, buffer_bytes);

			for (int i = 0; i < buffer_bytes; i++)
			{
				im->m_data[4*i+3] = buffer[i];
			}

			delete [] buffer;

			bi = render::create_bitmap_info_rgba(im);

			delete im;
#endif

		}
		else
		{
			bi = render::create_bitmap_info_empty();
		}

		// Create bitmap character.
		bitmap_character*	ch = new bitmap_character(bi);

		m->add_bitmap_character(character_id, ch);
	}


	void	define_bits_lossless_2_loader(stream* in, int tag_type, movie_definition_sub* m)
	{
		assert(tag_type == 20 || tag_type == 36);

		Uint16	character_id = in->read_u16();
		Uint8	bitmap_format = in->read_u8();	// 3 == 8 bit, 4 == 16 bit, 5 == 32 bit
		Uint16	width = in->read_u16();
		Uint16	height = in->read_u16();

		IF_VERBOSE_PARSE(log_msg("  defbitslossless2: tag_type = %d, id = %d, fmt = %d, w = %d, h = %d\n",
				tag_type,
				character_id,
				bitmap_format,
				width,
				height));

		bitmap_info*	bi = NULL;
		if (m->get_create_bitmaps() == DO_LOAD_BITMAPS)
		{
#if TU_CONFIG_LINK_TO_ZLIB == 0
			log_error("gnash is not linked to zlib -- can't load zipped image data!\n");
			return;
#else
			if (tag_type == 20)
			{
				// RGB image data.
				image::rgb*	image = image::create_rgb(width, height);

				if (bitmap_format == 3)
				{
					// 8-bit data, preceded by a palette.

					const int	bytes_per_pixel = 1;
					int	color_table_size = in->read_u8();
					color_table_size += 1;	// !! SWF stores one less than the actual size

					int	pitch = (width * bytes_per_pixel + 3) & ~3;

					int	buffer_bytes = color_table_size * 3 + pitch * height;
					Uint8*	buffer = new Uint8[buffer_bytes];

					inflate_wrapper(in->get_underlying_stream(), buffer, buffer_bytes);
					assert(in->get_position() <= in->get_tag_end_position());

					Uint8*	color_table = buffer;

					for (int j = 0; j < height; j++)
					{
						Uint8*	image_in_row = buffer + color_table_size * 3 + j * pitch;
						Uint8*	image_out_row = image::scanline(image, j);
						for (int i = 0; i < width; i++)
						{
							Uint8	pixel = image_in_row[i * bytes_per_pixel];
							image_out_row[i * 3 + 0] = color_table[pixel * 3 + 0];
							image_out_row[i * 3 + 1] = color_table[pixel * 3 + 1];
							image_out_row[i * 3 + 2] = color_table[pixel * 3 + 2];
						}
					}

					delete [] buffer;
				}
				else if (bitmap_format == 4)
				{
					// 16 bits / pixel
					const int	bytes_per_pixel = 2;
					int	pitch = (width * bytes_per_pixel + 3) & ~3;

					int	buffer_bytes = pitch * height;
					Uint8*	buffer = new Uint8[buffer_bytes];

					inflate_wrapper(in->get_underlying_stream(), buffer, buffer_bytes);
					assert(in->get_position() <= in->get_tag_end_position());
			
					for (int j = 0; j < height; j++)
					{
						Uint8*	image_in_row = buffer + j * pitch;
						Uint8*	image_out_row = image::scanline(image, j);
						for (int i = 0; i < width; i++)
						{
							Uint16	pixel = image_in_row[i * 2] | (image_in_row[i * 2 + 1] << 8);
					
							// @@ How is the data packed???  I'm just guessing here that it's 565!
							image_out_row[i * 3 + 0] = (pixel >> 8) & 0xF8;	// red
							image_out_row[i * 3 + 1] = (pixel >> 3) & 0xFC;	// green
							image_out_row[i * 3 + 2] = (pixel << 3) & 0xF8;	// blue
						}
					}
			
					delete [] buffer;
				}
				else if (bitmap_format == 5)
				{
					// 32 bits / pixel, input is ARGB format (???)
					const int	bytes_per_pixel = 4;
					int	pitch = width * bytes_per_pixel;

					int	buffer_bytes = pitch * height;
					Uint8*	buffer = new Uint8[buffer_bytes];

					inflate_wrapper(in->get_underlying_stream(), buffer, buffer_bytes);
					assert(in->get_position() <= in->get_tag_end_position());
			
					// Need to re-arrange ARGB into RGB.
					for (int j = 0; j < height; j++)
					{
						Uint8*	image_in_row = buffer + j * pitch;
						Uint8*	image_out_row = image::scanline(image, j);
						for (int i = 0; i < width; i++)
						{
							Uint8	a = image_in_row[i * 4 + 0];
							Uint8	r = image_in_row[i * 4 + 1];
							Uint8	g = image_in_row[i * 4 + 2];
							Uint8	b = image_in_row[i * 4 + 3];
							image_out_row[i * 3 + 0] = r;
							image_out_row[i * 3 + 1] = g;
							image_out_row[i * 3 + 2] = b;
							a = a;	// Inhibit warning.
						}
					}

					delete [] buffer;
				}

//				bitmap_character*	ch = new bitmap_character(image);
				bi = render::create_bitmap_info_rgb(image);
				delete image;

// 				// add image to movie, under character id.
// 				m->add_bitmap_character(character_id, ch);
			}
			else
			{
				// RGBA image data.
				assert(tag_type == 36);

				image::rgba*	image = image::create_rgba(width, height);

				if (bitmap_format == 3)
				{
					// 8-bit data, preceded by a palette.

					const int	bytes_per_pixel = 1;
					int	color_table_size = in->read_u8();
					color_table_size += 1;	// !! SWF stores one less than the actual size

					int	pitch = (width * bytes_per_pixel + 3) & ~3;

					int	buffer_bytes = color_table_size * 4 + pitch * height;
					Uint8*	buffer = new Uint8[buffer_bytes];

					inflate_wrapper(in->get_underlying_stream(), buffer, buffer_bytes);
					assert(in->get_position() <= in->get_tag_end_position());

					Uint8*	color_table = buffer;

					for (int j = 0; j < height; j++)
					{
						Uint8*	image_in_row = buffer + color_table_size * 4 + j * pitch;
						Uint8*	image_out_row = image::scanline(image, j);
						for (int i = 0; i < width; i++)
						{
							Uint8	pixel = image_in_row[i * bytes_per_pixel];
							image_out_row[i * 4 + 0] = color_table[pixel * 4 + 0];
							image_out_row[i * 4 + 1] = color_table[pixel * 4 + 1];
							image_out_row[i * 4 + 2] = color_table[pixel * 4 + 2];
							image_out_row[i * 4 + 3] = color_table[pixel * 4 + 3];
						}
					}

					delete [] buffer;
				}
				else if (bitmap_format == 4)
				{
					// 16 bits / pixel
					const int	bytes_per_pixel = 2;
					int	pitch = (width * bytes_per_pixel + 3) & ~3;

					int	buffer_bytes = pitch * height;
					Uint8*	buffer = new Uint8[buffer_bytes];

					inflate_wrapper(in->get_underlying_stream(), buffer, buffer_bytes);
					assert(in->get_position() <= in->get_tag_end_position());
			
					for (int j = 0; j < height; j++)
					{
						Uint8*	image_in_row = buffer + j * pitch;
						Uint8*	image_out_row = image::scanline(image, j);
						for (int i = 0; i < width; i++)
						{
							Uint16	pixel = image_in_row[i * 2] | (image_in_row[i * 2 + 1] << 8);
					
							// @@ How is the data packed???  I'm just guessing here that it's 565!
							image_out_row[i * 4 + 0] = 255;			// alpha
							image_out_row[i * 4 + 1] = (pixel >> 8) & 0xF8;	// red
							image_out_row[i * 4 + 2] = (pixel >> 3) & 0xFC;	// green
							image_out_row[i * 4 + 3] = (pixel << 3) & 0xF8;	// blue
						}
					}
			
					delete [] buffer;
				}
				else if (bitmap_format == 5)
				{
					// 32 bits / pixel, input is ARGB format

					inflate_wrapper(in->get_underlying_stream(), image->m_data, width * height * 4);
					assert(in->get_position() <= in->get_tag_end_position());
			
					// Need to re-arrange ARGB into RGBA.
					for (int j = 0; j < height; j++)
					{
						Uint8*	image_row = image::scanline(image, j);
						for (int i = 0; i < width; i++)
						{
							Uint8	a = image_row[i * 4 + 0];
							Uint8	r = image_row[i * 4 + 1];
							Uint8	g = image_row[i * 4 + 2];
							Uint8	b = image_row[i * 4 + 3];
							image_row[i * 4 + 0] = r;
							image_row[i * 4 + 1] = g;
							image_row[i * 4 + 2] = b;
							image_row[i * 4 + 3] = a;
						}
					}
				}

				bi = render::create_bitmap_info_rgba(image);
//				bitmap_character*	ch = new bitmap_character(image);
				delete image;

//	 			// add image to movie, under character id.
//	 			m->add_bitmap_character(character_id, ch);
			}
#endif // TU_CONFIG_LINK_TO_ZLIB
		}
		else
		{
			bi = render::create_bitmap_info_empty();
		}
		assert(bi->get_ref_count() == 0);

		bitmap_character*	ch = new bitmap_character(bi);

		// add image to movie, under character id.
		m->add_bitmap_character(character_id, ch);
	}


	void	define_shape_loader(stream* in, int tag_type, movie_definition_sub* m)
	{
		assert(tag_type == 2
		       || tag_type == 22
		       || tag_type == 32);

		Uint16	character_id = in->read_u16();
		IF_VERBOSE_PARSE(log_msg("  shape_loader: id = %d\n", character_id));

		shape_character_def*	ch = new shape_character_def;
		ch->read(in, tag_type, true, m);

		IF_VERBOSE_PARSE(log_msg("  bound rect:"); ch->get_bound().print());

		m->add_character(character_id, ch);
	}

	void define_shape_morph_loader(stream* in, int tag_type, movie_definition_sub* m)
	{
		assert(tag_type == 46);
		Uint16 character_id = in->read_u16();
		IF_VERBOSE_PARSE(log_msg("  shape_morph_loader: id = %d\n", character_id));
		morph2_character_def* morph = new morph2_character_def;
		morph->read(in, tag_type, true, m);
		m->add_character(character_id, morph);
	}

	//
	// font loaders
	//


	void	define_font_loader(stream* in, int tag_type, movie_definition_sub* m)
	// Load a DefineFont or DefineFont2 tag.
	{
		assert(tag_type == 10 || tag_type == 48);

		Uint16	font_id = in->read_u16();
		
		font*	f = new font;
		f->read(in, tag_type, m);

		m->add_font(font_id, f);

		// Automatically keeping fonts in fontlib is
		// problematic.  The app should be responsible for
		// optionally adding fonts to fontlib.
		// //fontlib::add_font(f);
	}


	void	define_font_info_loader(stream* in, int tag_type, movie_definition_sub* m)
	// Load a DefineFontInfo tag.  This adds information to an
	// existing font.
	{
		assert(tag_type == 13);

		Uint16	font_id = in->read_u16();
		
		font*	f = m->get_font(font_id);
		if (f)
		{
			f->read_font_info(in);
		}
		else
		{
			log_error("define_font_info_loader: can't find font w/ id %d\n", font_id);
		}
	}


	//
	// swf_event
	//
	// For embedding event handlers in place_object_2


	struct swf_event
	{
		// NOTE: DO NOT USE THESE AS VALUE TYPES IN AN
		// array<>!  They cannot be moved!  The private
		// operator=(const swf_event&) should help guard
		// against that.

		event_id	m_event;
		action_buffer	m_action_buffer;
		as_value	m_method;

		swf_event()
		{
		}

		void	attach_to(character* ch) const
		{
			ch->set_event_handler(m_event, m_method);
		}

		void	read(stream* in, Uint32 flags)
		{
			assert(flags != 0);

			// Scream if more than one bit is set, since we're not set up to handle
			// that, and it doesn't seem possible to express in ActionScript source,
			// so it's important to know if this ever occurs in the wild.
			if (flags & (flags - 1))
			{
				log_error("error: swf_event::read() -- more than one event type encoded!  "
					  "unexpected! flags = 0x%x\n", flags);
			}

			// 14 bits reserved, 18 bits used

			static const event_id	s_code_bits[18] =
			{
				event_id::LOAD,
				event_id::ENTER_FRAME,
				event_id::UNLOAD,
				event_id::MOUSE_MOVE,
				event_id::MOUSE_DOWN,
				event_id::MOUSE_UP,
				event_id::KEY_DOWN,
				event_id::KEY_UP,
				event_id::DATA,
				event_id::INITIALIZE,
				event_id::PRESS,
				event_id::RELEASE,
				event_id::RELEASE_OUTSIDE,
				event_id::ROLL_OVER,
				event_id::ROLL_OUT,
				event_id::DRAG_OVER,
				event_id::DRAG_OUT,
			};

			for (int i = 0, mask = 1; i < int(sizeof(s_code_bits)/sizeof(s_code_bits[0])); i++, mask <<= 1)
			{
				if (flags & mask)
				{
					m_event = s_code_bits[i];
					break;
				}
			}

			// what to do w/ key_press???  Is the data in the reserved parts of the flags???
			if (flags & (1 << 17))
			{
				log_error("swf_event::read -- KEY_PRESS found, not handled yet, flags = 0x%x\n", flags);
			}

			Uint32	event_length = in->read_u32();
			UNUSED(event_length);

			// Read the actions.
			IF_VERBOSE_ACTION(log_msg("---- actions for event %s\n", m_event.get_function_name().c_str()));
			m_action_buffer.read(in);

			if (m_action_buffer.get_length() != (int) event_length)
			{
				log_error("error -- swf_event::read(), event_length = %d, but read %d\n",
					  event_length,
					  m_action_buffer.get_length());
				// @@ discard this event handler??
			}

			// Create a function to execute the actions.
			array<with_stack_entry>	empty_with_stack;
			as_as_function*	func = new as_as_function(&m_action_buffer, NULL, 0, empty_with_stack);
			func->set_length(m_action_buffer.get_length());

			m_method.set_as_as_function(func);
		}

	private:
		// DON'T USE THESE
		swf_event(const swf_event& s) { assert(0); }
		void	operator=(const swf_event& s) { assert(0); }
	};


	//
	// place_object_2
	//
	
	struct place_object_2 : public execute_tag
	{
		int	m_tag_type;
		char*	m_name;
		float	m_ratio;
		cxform	m_color_transform;
		matrix	m_matrix;
		bool	m_has_matrix;
		bool	m_has_cxform;
		Uint16	m_depth;
		Uint16	m_character_id;
                Uint16 	m_clip_depth;
		enum place_type {
			PLACE,
			MOVE,
			REPLACE,
		} m_place_type;
		array<swf_event*>	m_event_handlers;


		place_object_2()
			:
			m_tag_type(0),
			m_name(NULL),
			m_ratio(0),
			m_has_matrix(false),
			m_has_cxform(false),
			m_depth(0),
			m_character_id(0),
                        m_clip_depth(0),
			m_place_type(PLACE)
		{
		}

		~place_object_2()
		{
			delete [] m_name;
			m_name = NULL;

			for (int i = 0, n = m_event_handlers.size(); i < n; i++)
			{
				delete m_event_handlers[i];
			}
			m_event_handlers.resize(0);
		}

		void	read(stream* in, int tag_type, int movie_version)
		{
			assert(tag_type == 4 || tag_type == 26);

			m_tag_type = tag_type;

			if (tag_type == 4)
			{
				// Original place_object tag; very simple.
				m_character_id = in->read_u16();
				m_depth = in->read_u16();
				m_matrix.read(in);

				IF_VERBOSE_PARSE(
					log_msg("  char_id = %d\n"
						"  depth = %d\n"
						"  mat = \n",
						m_character_id,
						m_depth);
					m_matrix.print());

				if (in->get_position() < in->get_tag_end_position())
				{
					m_color_transform.read_rgb(in);
					IF_VERBOSE_PARSE(log_msg("  cxform:\n"); m_color_transform.print());
				}
			}
			else if (tag_type == 26)
			{
				in->align();

				bool	has_actions = in->read_uint(1) ? true : false;
				bool	has_clip_bracket = in->read_uint(1) ? true : false;
				bool	has_name = in->read_uint(1) ? true : false;
				bool	has_ratio = in->read_uint(1) ? true : false;
				bool	has_cxform = in->read_uint(1) ? true : false;
				bool	has_matrix = in->read_uint(1) ? true : false;
				bool	has_char = in->read_uint(1) ? true : false;
				bool	flag_move = in->read_uint(1) ? true : false;

				m_depth = in->read_u16();
				IF_VERBOSE_PARSE(log_msg("  depth = %d\n", m_depth));

				if (has_char) {
					m_character_id = in->read_u16();
					IF_VERBOSE_PARSE(log_msg("  char id = %d\n", m_character_id));
				}

				if (has_matrix) {
					m_has_matrix = true;
					m_matrix.read(in);
					IF_VERBOSE_PARSE(log_msg("  mat:\n"); m_matrix.print());
				}
				if (has_cxform) {
					m_has_cxform = true;
					m_color_transform.read_rgba(in);
					IF_VERBOSE_PARSE(log_msg("  cxform:\n"); m_color_transform.print());
				}
				
				if (has_ratio) {
					m_ratio = (float)in->read_u16() / (float)65535;
					IF_VERBOSE_PARSE(log_msg("  ratio: %f\n", m_ratio));
				}
				
				if (has_name) {
					m_name = in->read_string();
					IF_VERBOSE_PARSE(log_msg("  name = %s\n", m_name ? m_name : "<null>"));
				}
				if (has_clip_bracket) {
					m_clip_depth = in->read_u16(); 
					IF_VERBOSE_PARSE(log_msg("  clip_depth = %d\n", m_clip_depth));
				}
				if (has_actions)
				{
					Uint16	reserved = in->read_u16();
					UNUSED(reserved);

					// The logical 'or' of all the following handlers.
					// I don't think we care about this...
					Uint32	all_flags = 0;
					if (movie_version >= 6)
					{
						all_flags = in->read_u32();
					}
					else
					{
						all_flags = in->read_u16();
					}
					UNUSED(all_flags);

					IF_VERBOSE_PARSE(log_msg("  actions: flags = 0x%X\n", all_flags));

					// Read swf_events.
					for (;;)
					{
						// Read event.
						in->align();

						Uint32	this_flags = 0;
						if (movie_version >= 6)
						{
							this_flags = in->read_u32();
						}
						else
						{
							this_flags = in->read_u16();
						}

						if (this_flags == 0)
						{
							// Done with events.
							break;
						}

						swf_event*	ev = new swf_event;
						ev->read(in, this_flags);

						m_event_handlers.push_back(ev);
					}
				}


				if (has_char == true && flag_move == true)
				{
					// Remove whatever's at m_depth, and put m_character there.
					m_place_type = REPLACE;
				}
				else if (has_char == false && flag_move == true)
				{
					// Moves the object at m_depth to the new location.
					m_place_type = MOVE;
				}
				else if (has_char == true && flag_move == false)
				{
					// Put m_character at m_depth.
					m_place_type = PLACE;
				}
                                
				//log_msg("place object at depth %i\n", m_depth);
			}
		}

		
		void	execute(movie* m)
		// Place/move/whatever our object in the given movie.
		{
			switch (m_place_type)
			{
			case PLACE:
				m->add_display_object(
					m_character_id,
					m_name,
					m_event_handlers,
					m_depth,
					m_tag_type != 4,	// original place_object doesn't do replacement
					m_color_transform,
					m_matrix,
					m_ratio,
					m_clip_depth);
				break;

			case MOVE:
				m->move_display_object(
					m_depth,
					m_has_cxform,
					m_color_transform,
					m_has_matrix,
					m_matrix,
					m_ratio,
					m_clip_depth);
				break;

			case REPLACE:
				m->replace_display_object(
					m_character_id,
					m_name,
					m_depth,
					m_has_cxform,
					m_color_transform,
					m_has_matrix,
					m_matrix,
					m_ratio,
					m_clip_depth);
				break;
			}
		}

		void	execute_state(movie* m)
		{
			execute(m);
		}

		void	execute_state_reverse(movie* m, int frame)
		{
			switch (m_place_type)
			{
			case PLACE:
				// reverse of add is remove
				m->remove_display_object(m_depth, m_tag_type == 4 ? m_character_id : -1);
				break;

			case MOVE:
				// reverse of move is move
				m->move_display_object(
					m_depth,
					m_has_cxform,
					m_color_transform,
					m_has_matrix,
					m_matrix,
					m_ratio,
					m_clip_depth);
				break;

			case REPLACE:
			{
				// reverse of replace is to re-add the previous object.
				execute_tag*	last_add = m->find_previous_replace_or_add_tag(frame, m_depth, -1);
				if (last_add)
				{
					last_add->execute_state(m);
				}
				else
				{
					log_error("reverse REPLACE can't find previous replace or add tag(%d, %d)\n",
						  frame, m_depth);
					
				}
				break;
			}
			}
		}

		virtual uint32	get_depth_id_of_replace_or_add_tag() const
		// "depth_id" is the 16-bit depth & id packed into one 32-bit int.
		{
			if (m_place_type == PLACE || m_place_type == REPLACE)
			{
				int	id = -1;
				if (m_tag_type == 4)
				{
					// Old-style PlaceObject; the corresponding Remove
					// is specific to the character_id.
					id = m_character_id;
				}
				return ((m_depth & 0x0FFFF) << 16) | (id & 0x0FFFF);
			}
			else
			{
				return (uint32) -1;
			}
		}
	};


	
	void	place_object_2_loader(stream* in, int tag_type, movie_definition_sub* m)
	{
		assert(tag_type == 4 || tag_type == 26);

		IF_VERBOSE_PARSE(log_msg("  place_object_2\n"));

		place_object_2*	ch = new place_object_2;
		ch->read(in, tag_type, m->get_version());

		m->add_execute_tag(ch);
	}


	//
	// sprite_definition
	//


	// A sprite is a mini movie-within-a-movie.  It doesn't define
	// its own characters; it uses the characters from the parent
	// movie, but it has its own frame counter, display list, etc.
	//
	// The sprite implementation is divided into a
	// sprite_definition and a sprite_instance.  The _definition
	// holds the immutable data for a sprite, while the _instance
	// contains the state for a specific instance being updated
	// and displayed in the parent movie's display list.


	struct sprite_definition : public movie_definition_sub
	{
		movie_definition_sub*	     m_movie_def;		// parent movie.
		array<array<execute_tag*> >  m_playlist;	// movie control events for each frame.
		stringi_hash<int>	     m_named_frames;	// stores 0-based frame #'s
		int	                     m_frame_count;
		int	                     m_loading_frame;
		sprite_definition(movie_definition_sub* m)
			:
			m_movie_def(m),
			m_frame_count(0),
			m_loading_frame(0)
		{
			assert(m_movie_def);
		}

		~sprite_definition()
		{
			// Release our playlist data.
			{for (int i = 0, n = m_playlist.size(); i < n; i++)
			{
				for (int j = 0, m = m_playlist[i].size(); j < m; j++)
				{
					delete m_playlist[i][j];
				}
			}}
		}

		// overloads from movie_definition
		virtual float	get_width_pixels() const { return 1; }
		virtual float	get_height_pixels() const { return 1; }
		virtual int	get_frame_count() const { return m_frame_count; }
		virtual float	get_frame_rate() const { return m_movie_def->get_frame_rate(); }
		virtual int	get_loading_frame() const { return m_loading_frame; }
		virtual int	get_version() const { return m_movie_def->get_version(); }
		virtual void	add_character(int id, character_def* ch) { log_error("add_character tag appears in sprite tags!\n"); }
		virtual void	add_font(int id, font* ch) { log_error("add_font tag appears in sprite tags!\n"); }
		virtual font*	get_font(int id) { return m_movie_def->get_font(id); }
		virtual void	set_jpeg_loader(jpeg::input* j_in) { assert(0); }
		virtual jpeg::input*	get_jpeg_loader() { return NULL; }
		virtual bitmap_character_def*	get_bitmap_character(int id) { return m_movie_def->get_bitmap_character(id); }
		virtual void	add_bitmap_character(int id, bitmap_character_def* ch) { log_error("add_bc appears in sprite tags!\n"); }
		virtual sound_sample*	get_sound_sample(int id) { return m_movie_def->get_sound_sample(id); }
		virtual void	add_sound_sample(int id, sound_sample* sam) { log_error("add sam appears in sprite tags!\n"); }

		// @@ would be nicer to not inherit these...
		virtual create_bitmaps_flag	get_create_bitmaps() const { assert(0); return DO_LOAD_BITMAPS; }
		virtual create_font_shapes_flag	get_create_font_shapes() const { assert(0); return DO_LOAD_FONT_SHAPES; }
		virtual int	get_bitmap_info_count() const { assert(0); return 0; }
		virtual bitmap_info*	get_bitmap_info(int i) const { assert(0); return NULL; }
		virtual void	add_bitmap_info(bitmap_info* bi) { assert(0); }

		virtual void	export_resource(const tu_string& symbol, resource* res) { log_error("can't export from sprite\n"); }
		virtual smart_ptr<resource>	get_exported_resource(const tu_string& sym) { return m_movie_def->get_exported_resource(sym); }
		virtual void	add_import(const char* source_url, int id, const char* symbol) { assert(0); }
		virtual void	visit_imported_movies(import_visitor* v) { assert(0); }
		virtual void	resolve_import(const char* source_url, movie_definition* d) { assert(0); }
		virtual character_def*	get_character_def(int id)
		{
			return m_movie_def->get_character_def(id);
		}
		virtual void	generate_font_bitmaps() { assert(0); }


		virtual void	output_cached_data(tu_file* out, const cache_options& options)
		{
			// Nothing to do.
			return;
		}
		virtual void	input_cached_data(tu_file* in)
		{
			// Nothing to do.
			return;
		}

		virtual movie_interface*	create_instance()
		{
			return NULL;
		}

		// overloads from character_def
		virtual character*	create_character_instance(movie* parent, int id);


		/* sprite_definition */
		virtual void	add_execute_tag(execute_tag* c)
		{
			m_playlist[m_loading_frame].push_back(c);
		}

		/* sprite_definition */
		virtual void	add_init_action(int sprite_id, execute_tag* c)
		{
			// Sprite def's should not have do_init_action tags in them!  (@@ correct?)
			log_error("sprite_definition::add_init_action called!  Ignored.\n");
		}

		/* sprite_definition */
		virtual void	add_frame_name(const char* name)
		// Labels the frame currently being loaded with the
		// given name.  A copy of the name string is made and
		// kept in this object.
		{
			assert(m_loading_frame >= 0 && m_loading_frame < m_frame_count);

			tu_string	n = name;
			int	currently_assigned = 0;
			if (m_named_frames.get(n, &currently_assigned) == true)
			{
				log_error("add_frame_name(%d, '%s') -- frame name already assigned to frame %d; overriding\n",
					  m_loading_frame,
					  name,
					  currently_assigned);
			}
			m_named_frames.set(n, m_loading_frame);	// stores 0-based frame #
		}

		/* sprite_definition */
		bool	get_labeled_frame(const char* label, int* frame_number)
		// Returns 0-based frame #
		{
			return m_named_frames.get(label, frame_number);
		}

		const array<execute_tag*>&	get_playlist(int frame_number)
		// frame_number is 0-based
		{
			return m_playlist[frame_number];
		}

		/* sprite_definition */
		virtual const array<execute_tag*>*	get_init_actions(int frame_number)
		{
			// Sprites do not have init actions in their
			// playlists!  Only the root movie
			// (movie_def_impl) does (@@ correct?)
			return NULL;
		}


		/* sprite_definition */
		void	read(stream* in)
		// Read the sprite info.  Consists of a series of tags.
		{
			int	tag_end = in->get_tag_end_position();

			m_frame_count = in->read_u16();
			// ALEX: some SWF files have been seen that have 0-frame sprites.
			// The Macromedia player behaves as if they have 1 frame.
			if (m_frame_count < 1)
			{
				m_frame_count = 1;
			}
			m_playlist.resize(m_frame_count);	// need a playlist for each frame

			IF_VERBOSE_PARSE(log_msg("  frames = %d\n", m_frame_count));

			m_loading_frame = 0;

			while ((Uint32) in->get_position() < (Uint32) tag_end)
			{
				int	tag_type = in->open_tag();
				loader_function lf = NULL;
				if (tag_type == 1)
				{
					// show frame tag -- advance to the next frame.
					IF_VERBOSE_PARSE(log_msg("  show_frame (sprite)\n"));
					m_loading_frame++;
				}
				else if (s_tag_loaders.get(tag_type, &lf))
				{
					// call the tag loader.  The tag loader should add
					// characters or tags to the movie data structure.
					(*lf)(in, tag_type, this);
				}
				else
				{
					// no tag loader for this tag type.
					IF_VERBOSE_PARSE(log_msg("*** no tag loader for type %d\n", tag_type));
				}

				in->close_tag();
			}

			IF_VERBOSE_PARSE(log_msg("  -- sprite END --\n"));
		}
	};


	//
	// sprite_instance
	//


	struct sprite_instance : public character
	{
		smart_ptr<movie_definition_sub>	m_def;
		movie_root*	m_root;

		display_list	m_display_list;
		array<action_buffer*>	m_action_list;

		play_state	m_play_state;
		int		m_current_frame;
		float		m_time_remainder;
		bool		m_update_frame;
		bool		m_has_looped;
		bool	        m_accept_anim_moves;	// once we've been moved by ActionScript, don't accept moves from anim tags.
		array<bool>	m_init_actions_executed;	// a bit-array class would be ideal for this

		as_environment	m_as_environment;

		enum mouse_state
		{
			UP = 0,
			DOWN,
			OVER
		};
		mouse_state m_mouse_state;

		sprite_instance(movie_definition_sub* def, movie_root* r, movie* parent, int id)
			:
			character(parent, id),
			m_def(def),
			m_root(r),
			m_play_state(PLAY),
			m_current_frame(0),
			m_time_remainder(0),
			m_update_frame(true),
			m_has_looped(false),
			m_accept_anim_moves(true),
			m_mouse_state(UP)
		{
			assert(m_def != NULL);
			assert(m_root != NULL);
			
			//m_root->add_ref();	// @@ circular!
			m_as_environment.set_target(this);

			sprite_builtins_init();

			// Initialize the flags for init action executed.
			m_init_actions_executed.resize(m_def->get_frame_count());
			memset(&m_init_actions_executed[0], 0,
			       sizeof(m_init_actions_executed[0]) * m_init_actions_executed.size());
		}

		// sprite instance of add_interval_handler()
		virtual int    add_interval_timer(void *timer)
		{
			// log_msg("FIXME: %s:\n", __FUNCTION__);
			return m_root->add_interval_timer(timer);
		}

		virtual void    clear_interval_timer(int x)
		{
			// log_msg("FIXME: %s:\n", __FUNCTION__);
			m_root->clear_interval_timer(x);
		}
	

		/* sprite_instance */
		virtual void    do_something(void *timer)
		{
			as_value	val;
			as_object      *obj, *this_ptr;
			as_environment *as_env;

			//printf("FIXME: %s:\n", __FUNCTION__);
			Timer *ptr = (Timer *)timer;
			//log_msg("INTERVAL ID is %d\n", ptr->getIntervalID());

			const as_value&	timer_method = ptr->getASFunction();
			as_env = ptr->getASEnvironment();
			this_ptr = ptr->getASObject();
			obj = ptr->getObject();
			//m_as_environment.push(obj);
			
			as_c_function_ptr	cfunc = timer_method.to_c_function();
			if (cfunc) {
				// It's a C function. Call it.
				//log_msg("Calling C function for interval timer\n");
				//(*cfunc)(&val, obj, as_env, 0, 0);
				(*cfunc)(fn_call(&val, obj, &m_as_environment, 0, 0));
				
			} else if (as_as_function* as_func = timer_method.to_as_function()) {
				// It's an ActionScript function. Call it.
				as_value method;
				//log_msg("Calling ActionScript function for interval timer\n");
				(*as_func)(fn_call(&val, (as_object_interface *)this_ptr, as_env, 0, 0));
				//(*as_func)(&val, (as_object_interface *)this_ptr, &m_as_environment, 1, 1);
			} else {
				log_error("error in call_method(): method is not a function\n");
			}    
		}	

		virtual ~sprite_instance()
		{
			m_display_list.clear();
			//m_root->drop_ref();
		}

		movie_interface*	get_root_interface() { return m_root; }
		movie_root*	get_root() { return m_root; }
		movie*	get_root_movie() { return m_root->get_root_movie(); }

		movie_definition*	get_movie_definition() { return m_def.get_ptr(); }

		float	get_width()
		{
			float	w = 0;
			int i, n = m_display_list.get_character_count();
			character* ch;
			for (i = 0; i < n; i++)
			{
				ch = m_display_list.get_character(i);
				if (ch != NULL)
				{
					float ch_w = ch->get_width();
					if (ch_w > w)
					{
						w = ch_w;
					}
				}
			}

			return w;
		}


		float	get_height()
		{
			float	h = 0; 
			int i, n = m_display_list.get_character_count();
			character* ch;
			for (i=0; i < n; i++)
			{
				ch = m_display_list.get_character(i);
				if (ch != NULL)
				{
					float	ch_h = ch->get_height();
					if (ch_h > h)
					{
						h = ch_h;
					}
				}
			}
			return h;
		}

		int	get_current_frame() const { return m_current_frame; }
		int	get_frame_count() const { return m_def->get_frame_count(); }

		void	set_play_state(play_state s)
		// Stop or play the sprite.
		{
			if (m_play_state != s)
			{
				m_time_remainder = 0;
			}

			m_play_state = s;
		}
		play_state	get_play_state() const { return m_play_state; }


		character*	get_character(int character_id)
		{
//			return m_def->get_character_def(character_id);
			// @@ TODO -- look through our dlist for a match
			return NULL;
		}

		float	get_background_alpha() const
		{
			// @@ this doesn't seem right...
			return m_root->get_background_alpha();
		}

		float	get_pixel_scale() const { return m_root->get_pixel_scale(); }

		virtual void	get_mouse_state(int* x, int* y, int* buttons)
		{
			m_root->get_mouse_state(x, y, buttons);
		}

		void	set_background_color(const rgba& color)
		{
			m_root->set_background_color(color);
		}

		float	get_timer() const { return m_root->get_timer(); }

		void	restart()
		{
			m_current_frame = 0;
			m_time_remainder = 0;
			m_update_frame = true;
			m_has_looped = false;
			m_play_state = PLAY;

			execute_frame_tags(m_current_frame);
			m_display_list.update();
		}


		virtual bool	has_looped() const { return m_has_looped; }

		virtual bool	get_accept_anim_moves() const { return m_accept_anim_moves; }

		inline int	transition(int a, int b) const
		// Combine the flags to avoid a conditional. It would be faster with a macro.
		{
			return (a << 2) | b;
		}


		bool can_handle_mouse_event()
		// Return true if we have any mouse event handlers.
		{
			// We should cache this!
			as_value dummy;

			// Functions that qualify as mouse event handlers.
			const char* FN_NAMES[] = {
				"onKeyPress",
				"onRelease",
				"onDragOver",
				"onDragOut",
				"onPress",
				"onReleaseOutside",
				"onRollout",
				"onRollover",
			};
			for (int i = 0; i < ARRAYSIZE(FN_NAMES); i++) {
				if (get_member(FN_NAMES[i], &dummy)) {
					return true;
				}
			}

			// Event handlers that qualify as mouse event handlers.
			const event_id::id_code EH_IDS[] = {
				event_id::PRESS,
				event_id::RELEASE,
				event_id::RELEASE_OUTSIDE,
				event_id::ROLL_OVER,
				event_id::ROLL_OUT,
				event_id::DRAG_OVER,
				event_id::DRAG_OUT,
			};
			{for (int i = 0; i < ARRAYSIZE(EH_IDS); i++) {
				if (get_event_handler(EH_IDS[i], &dummy)) {
					return true;
				}
			}}

			return false;
		}
		

		/* sprite_instance */
		virtual movie*	get_topmost_mouse_entity(float x, float y)
		// Return the topmost entity that the given point
		// covers that can receive mouse events.  NULL if
		// none.  Coords are in parent's frame.
		{
			if (get_visible() == false) {
				return NULL;
			}

			matrix	m = get_matrix();
			point	p;
			m.transform_by_inverse(&p, point(x, y));

			int i, n = m_display_list.get_character_count();
			// Go backwards, to check higher objects first.
			for (i = n - 1; i >= 0; i--)
			{
				character* ch = m_display_list.get_character(i);
				
				if (ch != NULL && ch->get_visible())
				{
					movie*	te = ch->get_topmost_mouse_entity(p.m_x, p.m_y);
					if (te)
					{
						// The containing entity that 1) is closest to root and 2) can
						// handle mouse events takes precedence.
						if (can_handle_mouse_event()) {
							return this;
						} else {
							return te;
						}
					}
				}
			}

			return NULL;
		}


		/* sprite_instance */
		void	increment_frame_and_check_for_loop()
		// Increment m_current_frame, and take care of looping.
		{
			m_current_frame++;

			int	frame_count = m_def->get_frame_count();
			if (m_current_frame >= frame_count)
			{
				// Loop.
				m_current_frame = 0;
				m_has_looped = true;
				if (frame_count > 1)
				{
					m_display_list.reset();
				}
			}
		}

		/* sprite_instance */
		virtual void	advance(float delta_time)
		{
			// Keep this (particularly m_as_environment) alive during execution!
			smart_ptr<as_object_interface>	this_ptr(this);

			assert(m_def != NULL && m_root != NULL);

			// Advance everything in the display list.
			m_display_list.advance(delta_time);

			// mouse drag.
			character::do_mouse_drag();

			m_time_remainder += delta_time;

			const float	frame_time = 1.0f / m_root->get_frame_rate();	// @@ cache this

			// Check for the end of frame
			if (m_time_remainder >= frame_time)
			{
				m_time_remainder -= frame_time;

				// Update current and next frames.
				if (m_play_state == PLAY)
				{
					int	current_frame0 = m_current_frame;
					increment_frame_and_check_for_loop();

					// Execute the current frame's tags.
					if (m_current_frame != current_frame0)
					{
						execute_frame_tags(m_current_frame);
					}
				}

				// Dispatch onEnterFrame event.
				on_event(event_id::ENTER_FRAME);

				do_actions();

				// Clean up display list (remove dead objects).
				m_display_list.update();
			}

			// Skip excess time.  TODO root caller should
			// loop to prevent this happening; probably
			// only root should keep m_time_remainder, and
			// advance(dt) should be a discrete tick()
			// with no dt.
			m_time_remainder = fmod(m_time_remainder, frame_time);
		}

		/*sprite_instance*/
		void	execute_frame_tags(int frame, bool state_only = false)
		// Execute the tags associated with the specified frame.
		// frame is 0-based
		{
			// Keep this (particularly m_as_environment) alive during execution!
			smart_ptr<as_object_interface>	this_ptr(this);

			assert(frame >= 0);
			assert(frame < m_def->get_frame_count());

			// Execute this frame's init actions, if necessary.
			if (m_init_actions_executed[frame] == false)
			{
				const array<execute_tag*>*	init_actions = m_def->get_init_actions(frame);
				if (init_actions && init_actions->size() > 0)
				{
					// Need to execute these actions.
					for (int i= 0; i < init_actions->size(); i++)
					{
						execute_tag*	e = (*init_actions)[i];
						e->execute(this);
					}

					// Mark this frame done, so we never execute these init actions
					// again.
					m_init_actions_executed[frame] = true;
				}
			}

			const array<execute_tag*>&	playlist = m_def->get_playlist(frame);
			for (int i = 0; i < playlist.size(); i++)
			{
				execute_tag*	e = playlist[i];
				if (state_only)
				{
					e->execute_state(this);
				}
				else
				{
					e->execute(this);
				}
			}
		}


		/*sprite_instance*/
		void	execute_frame_tags_reverse(int frame)
		// Execute the tags associated with the specified frame, IN REVERSE.
		// I.e. if it's an "add" tag, then we do a "remove" instead.
		// Only relevant to the display-list manipulation tags: add, move, remove, replace.
		//
		// frame is 0-based
		{
			// Keep this (particularly m_as_environment) alive during execution!
			smart_ptr<as_object_interface>	this_ptr(this);

			assert(frame >= 0);
			assert(frame < m_def->get_frame_count());

			const array<execute_tag*>&	playlist = m_def->get_playlist(frame);
			for (int i = 0; i < playlist.size(); i++)
			{
				execute_tag*	e = playlist[i];
				e->execute_state_reverse(this, frame);
			}
		}

		
		/*sprite_instance*/
		execute_tag*	find_previous_replace_or_add_tag(int frame, int depth, int id)
		{
			uint32	depth_id = ((depth & 0x0FFFF) << 16) | (id & 0x0FFFF);

			for (int f = frame - 1; f >= 0; f--)
			{
				const array<execute_tag*>&	playlist = m_def->get_playlist(f);
				for (int i = playlist.size() - 1; i >= 0; i--)
				{
					execute_tag*	e = playlist[i];
					if (e->get_depth_id_of_replace_or_add_tag() == depth_id)
					{
						return e;
					}
				}
			}

			return NULL;
		}


		/*sprite_instance*/
		void	execute_remove_tags(int frame)
		// Execute any remove-object tags associated with the specified frame.
		// frame is 0-based
		{
			assert(frame >= 0);
			assert(frame < m_def->get_frame_count());

			const array<execute_tag*>&	playlist = m_def->get_playlist(frame);
			for (int i = 0; i < playlist.size(); i++)
			{
				execute_tag*	e = playlist[i];
				if (e->is_remove_tag())
				{
					e->execute_state(this);
				}
			}
		}


		/*sprite_instance*/
		void	do_actions()
		// Take care of this frame's actions.
		{
			// Keep m_as_environment alive during any method calls!
			smart_ptr<as_object_interface>	this_ptr(this);

			execute_actions(&m_as_environment, m_action_list);
			m_action_list.resize(0);
		}


		/*sprite_instance*/
		void	goto_frame(int target_frame_number)
		// Set the sprite state at the specified frame number.
		// 0-based frame numbers!!  (in contrast to ActionScript and Flash MX)
		{
//			IF_VERBOSE_DEBUG(log_msg("sprite::goto_frame(%d)\n", target_frame_number));//xxxxx

			target_frame_number = iclamp(target_frame_number, 0, m_def->get_frame_count() - 1);

			if (target_frame_number < m_current_frame)
			{
				for (int f = m_current_frame; f > target_frame_number; f--)
				{
					execute_frame_tags_reverse(f);
				}

				execute_frame_tags(target_frame_number, false);
				m_display_list.update();
			}
			else if (target_frame_number > m_current_frame)
			{
				for (int f = m_current_frame + 1; f < target_frame_number; f++)
				{
					execute_frame_tags(f, true);
				}

				execute_frame_tags(target_frame_number, false);
				m_display_list.update();
			}

			m_current_frame = target_frame_number;      

			// goto_frame stops by default.
			m_play_state = STOP;
		}


		bool	goto_labeled_frame(const char* label)
		// Look up the labeled frame, and jump to it.
		{
			int	target_frame = -1;
			if (m_def->get_labeled_frame(label, &target_frame))
			{
				goto_frame(target_frame);
				return true;
			}
			else
			{
				IF_VERBOSE_ACTION(
					log_error("error: movie_impl::goto_labeled_frame('%s') unknown label\n", label));
				return false;
			}
		}

		
		/*sprite_instance*/
		void	display()
		{
			if (get_visible() == false)
			{
				// We're invisible, so don't display!
				return;
			}

			m_display_list.display();

			do_display_callback();
		}

		/*sprite_instance*/
		character*	add_display_object(
			Uint16 character_id,
			const char* name,
			const array<swf_event*>& event_handlers,
			Uint16 depth,
			bool replace_if_depth_is_occupied,
			const cxform& color_transform,
			const matrix& matrix,
			float ratio,
			Uint16 clip_depth)
		// Add an object to the display list.
		{
			assert(m_def != NULL);

			character_def*	cdef = m_def->get_character_def(character_id);
			if (cdef == NULL)
			{
				log_error("sprite::add_display_object(): unknown cid = %d\n", character_id);
				return NULL;
			}

			// If we already have this object on this
			// plane, then move it instead of replacing
			// it.
			character*	existing_char = m_display_list.get_character_at_depth(depth);
			if (existing_char
			    && existing_char->get_id() == character_id
			    && ((name == NULL && existing_char->get_name().length() == 0)
				|| (name && existing_char->get_name() == name)))
			{
//				IF_VERBOSE_DEBUG(log_msg("add changed to move on depth %d\n", depth));//xxxxxx
				move_display_object(depth, true, color_transform, true, matrix, ratio, clip_depth);
				return NULL;
			}
			//printf("%s: character %s, id is %d, count is %d\n", __FUNCTION__, existing_char->get_name(), character_id,m_display_list.get_character_count()); // FIXME:

			assert(cdef);
			smart_ptr<character>	ch = cdef->create_character_instance(this, character_id);
			assert(ch != NULL);
			if (name != NULL && name[0] != 0)
			{
				ch->set_name(name);
			}

			// Attach event handlers (if any).
			{for (int i = 0, n = event_handlers.size(); i < n; i++)
			{
				event_handlers[i]->attach_to(ch.get_ptr());
			}}

			m_display_list.add_display_object(
				ch.get_ptr(),
				depth,
				replace_if_depth_is_occupied,
				color_transform,
				matrix,
				ratio,
				clip_depth);

			assert(ch == NULL || ch->get_ref_count() > 1);
			return ch.get_ptr();
		}


		/*sprite_instance*/
		void	move_display_object(
			Uint16 depth,
			bool use_cxform,
			const cxform& color_xform,
			bool use_matrix,
			const matrix& mat,
			float ratio,
			Uint16 clip_depth)
		// Updates the transform properties of the object at
		// the specified depth.
		{
			m_display_list.move_display_object(depth, use_cxform, color_xform, use_matrix, mat, ratio, clip_depth);
		}


		/*sprite_instance*/
		void	replace_display_object(
			Uint16 character_id,
			const char* name,
			Uint16 depth,
			bool use_cxform,
			const cxform& color_transform,
			bool use_matrix,
			const matrix& mat,
			float ratio,
			Uint16 clip_depth)
		{
			assert(m_def != NULL);
			// printf("%s: character %s, id is %d\n", __FUNCTION__, name, character_id); // FIXME: debugging crap

			character_def*	cdef = m_def->get_character_def(character_id);
			if (cdef == NULL)
			{
				log_error("sprite::replace_display_object(): unknown cid = %d\n", character_id);
				return;
			}
			assert(cdef);

			smart_ptr<character>	ch = cdef->create_character_instance(this, character_id);
			assert(ch != NULL);

			if (name != NULL && name[0] != 0)
			{
				ch->set_name(name);
			}

			m_display_list.replace_display_object(
				ch.get_ptr(),
				depth,
				use_cxform,
				color_transform,
				use_matrix,
				mat,
				ratio,
				clip_depth);
		}


		/*sprite_instance*/
		void	replace_display_object(
			character* ch,
			const char* name,
			Uint16 depth,
			bool use_cxform,
			const cxform& color_transform,
			bool use_matrix,
			const matrix& mat,
			float ratio,
			Uint16 clip_depth)
		{
			printf("%s: character %s, id is %d\n", __FUNCTION__, name, ch->get_id()); // FIXME:

			assert(ch != NULL);

			if (name != NULL && name[0] != 0)
			{
				ch->set_name(name);
			}

			m_display_list.replace_display_object(
				ch,
				depth,
				use_cxform,
				color_transform,
				use_matrix,
				mat,
				ratio,
				clip_depth);
		}


		/*sprite_instance*/
		void	remove_display_object(Uint16 depth, int id)
		// Remove the object at the specified depth.
		// If id != -1, then only remove the object at depth with matching id.
		{
			m_display_list.remove_display_object(depth, id);
		}


		/*sprite_instance*/
		void	add_action_buffer(action_buffer* a)
		// Add the given action buffer to the list of action
		// buffers to be processed at the end of the next
		// frame advance.
		{
			m_action_list.push_back(a);
		}


		/*sprite_instance*/
		int	get_id_at_depth(int depth)
		// For debugging -- return the id of the character at the specified depth.
		// Return -1 if nobody's home.
		{
			int	index = m_display_list.get_display_index(depth);
			if (index == -1)
			{
				return -1;
			}

			character*	ch = m_display_list.get_display_object(index).m_character.get_ptr();

			return ch->get_id();
		}


		//
		// ActionScript support
		//


		/* sprite_instance */
		virtual void	set_variable(const char* path_to_var, const char* new_value)
		{
			assert(m_parent == NULL);	// should only be called on the root movie.

			if (path_to_var == NULL)
			{
				log_error("error: NULL path_to_var passed to set_variable()\n");
				return;
			}
			if (new_value == NULL)
			{
				log_error("error: NULL passed to set_variable('%s', NULL)\n", path_to_var);
				return;
			}

			array<with_stack_entry>	empty_with_stack;
			tu_string	path(path_to_var);
			as_value	val(new_value);

			m_as_environment.set_variable(path, val, empty_with_stack);
		}

		/* sprite_instance */
		virtual void	set_variable(const char* path_to_var, const wchar_t* new_value)
		{
			if (path_to_var == NULL)
			{
				log_error("error: NULL path_to_var passed to set_variable()\n");
				return;
			}
			if (new_value == NULL)
			{
				log_error("error: NULL passed to set_variable('%s', NULL)\n", path_to_var);
				return;
			}

			assert(m_parent == NULL);	// should only be called on the root movie.

			array<with_stack_entry>	empty_with_stack;
			tu_string	path(path_to_var);
			as_value	val(new_value);

			m_as_environment.set_variable(path, val, empty_with_stack);
		}

		/* sprite_instance */
		virtual const char*	get_variable(const char* path_to_var) const
		{
			assert(m_parent == NULL);	// should only be called on the root movie.

			array<with_stack_entry>	empty_with_stack;
			tu_string	path(path_to_var);

			// NOTE: this is static so that the string
			// value won't go away after we return!!!
			// It'll go away during the next call to this
			// function though!!!  NOT THREAD SAFE!
			static as_value	val;

			val = m_as_environment.get_variable(path, empty_with_stack);

			return val.to_string();	// ack!
		}

		
		/* sprite_instance */
		virtual void	set_member(const tu_stringi& name, const as_value& val)
		// Set the named member to the value.  Return true if we have
		// that member; false otherwise.
		{
			as_standard_member	std_member = get_standard_member(name);
			switch (std_member)
			{
			default:
			case M_INVALID_MEMBER:
				break;
			case M_X:
				//if (name == "_x")
			{
				matrix	m = get_matrix();
				m.m_[0][2] = (float) PIXELS_TO_TWIPS(val.to_number());
				set_matrix(m);

				m_accept_anim_moves = false;

				return;
			}
			case M_Y:
				//else if (name == "_y")
			{
				matrix	m = get_matrix();
				m.m_[1][2] = (float) PIXELS_TO_TWIPS(val.to_number());
				set_matrix(m);

				m_accept_anim_moves = false;

				return;
			}
			case M_XSCALE:
				//else if (name == "_xscale")
			{
				matrix	m = get_matrix();

				// Decompose matrix and insert the desired value.
				float	x_scale = (float) val.to_number() / 100.f;	// input is in percent
				float	y_scale = m.get_y_scale();
				float	rotation = m.get_rotation();
				m.set_scale_rotation(x_scale, y_scale, rotation);

				set_matrix(m);
				m_accept_anim_moves = false;
				return;
			}
			case M_YSCALE:
				//else if (name == "_yscale")
			{
				matrix	m = get_matrix();

				// Decompose matrix and insert the desired value.
				float	x_scale = m.get_x_scale();
				float	y_scale = (float) val.to_number() / 100.f;	// input is in percent
				float	rotation = m.get_rotation();
				m.set_scale_rotation(x_scale, y_scale, rotation);

				set_matrix(m);
				m_accept_anim_moves = false;
				return;
			}
			case M_ALPHA:
				//else if (name == "_alpha")
			{
				// Set alpha modulate, in percent.
				cxform	cx = get_cxform();
				cx.m_[3][0] = float(val.to_number()) / 100.f;
				set_cxform(cx);
				m_accept_anim_moves = false;
				return;
			}
			case M_VISIBLE:
				//else if (name == "_visible")
			{
				set_visible(val.to_bool());
				m_accept_anim_moves = false;
				return;
			}
			case M_WIDTH:
				//else if (name == "_width")
			{
				// @@ tulrich: is parameter in world-coords or local-coords?
				matrix	m = get_matrix();
				m.m_[0][0] = float(PIXELS_TO_TWIPS(val.to_number()));
				float w = get_width();
				if (fabsf(w) > 1e-6f)
				{
					 m.m_[0][0] /= w;
				}
				set_matrix(m);
				m_accept_anim_moves = false;
				return;
			}
			case M_HEIGHT:
				//else if (name == "_height")
			{
				// @@ tulrich: is parameter in world-coords or local-coords?
				matrix	m = get_matrix();
				m.m_[1][1] = float(PIXELS_TO_TWIPS(val.to_number()));
				float h = get_width();
				if (fabsf(h) > 1e-6f)
				{
					m.m_[1][1] /= h;
				}
				set_matrix(m);
				m_accept_anim_moves = false;
				return;
			}
			case M_ROTATION:
				//else if (name == "_rotation")
			{
				matrix	m = get_matrix();

				// Decompose matrix and insert the desired value.
				float	x_scale = m.get_x_scale();
				float	y_scale = m.get_y_scale();
				float	rotation = (float) val.to_number() * float(M_PI) / 180.f;	// input is in degrees
				m.set_scale_rotation(x_scale, y_scale, rotation);

				set_matrix(m);
				m_accept_anim_moves = false;
				return;
			}
			case M_HIGHQUALITY:
				//else if (name == "_highquality")
			{
				// @@ global { 0, 1, 2 }
//				// Whether we're in high quality mode or not.
//				val->set(true);
				return;
			}
			case M_FOCUSRECT:
				//else if (name == "_focusrect")
			{
//				// Is a yellow rectangle visible around a focused movie clip (?)
//				val->set(false);
				return;
			}
			case M_SOUNDBUFTIME:
				//else if (name == "_soundbuftime")
			{
				// @@ global
//				// Number of seconds before sound starts to stream.
//				val->set(0.0);
				return;
			}
			}	// end switch

			// Not a built-in property.  See if we have a
			// matching edit_text character in our display
			// list.
			bool	text_val = val.get_type() == as_value::STRING
				|| val.get_type() == as_value::NUMBER;
			if (text_val)
			{
				bool	success = false;
				for (int i = 0, n = m_display_list.get_character_count(); i < n; i++)
				{
					character*	ch = m_display_list.get_character(i);
					// CASE INSENSITIVE compare.  In ActionScript 2.0, this
					// changes to CASE SENSITIVE!!!
					if (name == ch->get_text_name())
					{
						const char* text = val.to_string();
						ch->set_text_value(text);
						success = true;
					}
				}
				if (success) return;
			}

			// If that didn't work, set a variable within this environment.
			m_as_environment.set_member(name, val);
		}


		/* sprite_instance */
		virtual bool	get_member(const tu_stringi& name, as_value* val)
		// Set *val to the value of the named member and
		// return true, if we have the named member.
		// Otherwise leave *val alone and return false.
		{
			as_standard_member	std_member = get_standard_member(name);
			switch (std_member)
			{
			default:
			case M_INVALID_MEMBER:
				break;
			case M_X:
				//if (name == "_x")
			{
				matrix	m = get_matrix();
				val->set_double(TWIPS_TO_PIXELS(m.m_[0][2]));
				return true;
			}
			case M_Y:
				//else if (name == "_y")
			{
				matrix	m = get_matrix();
				val->set_double(TWIPS_TO_PIXELS(m.m_[1][2]));
				return true;
			}
			case M_XSCALE:
				//else if (name == "_xscale")
			{
				matrix m = get_matrix();	// @@ or get_world_matrix()?  Test this.
				float xscale = m.get_x_scale();
				val->set_double(xscale * 100);		// result in percent
				return true;
			}
			case M_YSCALE:
				//else if (name == "_yscale")
			{
				matrix m = get_matrix();	// @@ or get_world_matrix()?  Test this.
				float yscale = m.get_y_scale();
				val->set_double(yscale * 100);		// result in percent
				return true;
			}
			case M_CURRENTFRAME:
				//else if (name == "_currentframe")
			{
				val->set_int(m_current_frame + 1);
				return true;
			}
			case M_TOTALFRAMES:
				//else if (name == "_totalframes")
			{
				// number of frames.  Read only.
				val->set_int(m_def->get_frame_count());
				return true;
			}
			case M_ALPHA:
				//else if (name == "_alpha")
			{
				// Alpha units are in percent.
				val->set_double(get_cxform().m_[3][0] * 100.f);
				return true;
			}
			case M_VISIBLE:
				//else if (name == "_visible")
			{
				val->set_bool(get_visible());
				return true;
			}
			case M_WIDTH:
				//else if (name == "_width")
			{
				matrix	m = get_world_matrix();
				rect	transformed_rect;

				// @@ not sure about this...
				rect	source_rect;
				source_rect.m_x_min = 0;
				source_rect.m_y_min = 0;
				source_rect.m_x_max = (float) get_width();
				source_rect.m_y_max = (float) get_height();

				transformed_rect.enclose_transformed_rect(get_world_matrix(), source_rect);
				val->set_double(TWIPS_TO_PIXELS(transformed_rect.width()));
				return true;
			}
			case M_HEIGHT:
				//else if (name == "_height")
			{
				rect	transformed_rect;

				// @@ not sure about this...
				rect	source_rect;
				source_rect.m_x_min = 0;
				source_rect.m_y_min = 0;
				source_rect.m_x_max = (float) get_width();
				source_rect.m_y_max = (float) get_height();

				transformed_rect.enclose_transformed_rect(get_world_matrix(), source_rect);
				val->set_double(TWIPS_TO_PIXELS(transformed_rect.height()));
				return true;
			}
			case M_ROTATION:
				//else if (name == "_rotation")
			{
				// Verified against Macromedia player using samples/test_rotation.swf
				float	angle = get_matrix().get_rotation();

				// Result is CLOCKWISE DEGREES, [-180,180]
				angle *= 180.0f / float(M_PI);

				val->set_double(angle);
				return true;
			}
			case M_TARGET:
				//else if (name == "_target")
			{
				// Full path to this object; e.g. "/_level0/sprite1/sprite2/ourSprite"
				val->set_string("/_root");
				return true;
			}
			case M_FRAMESLOADED:
				//else if (name == "_framesloaded")
			{
				val->set_int(m_def->get_frame_count());
				return true;
			}
			case M_NAME:
				//else if (name == "_name")
			{
				val->set_tu_string(get_name());
				return true;
			}
			case M_DROPTARGET:
				//else if (name == "_droptarget")
			{
				// Absolute path in slash syntax where we were last dropped (?)
				// @@ TODO
				val->set_string("/_root");
				return true;
			}
			case M_URL:
				//else if (name == "_url")
			{
				// our URL.
				val->set_string("gnash");
				return true;
			}
			case M_HIGHQUALITY:
				//else if (name == "_highquality")
			{
				// Whether we're in high quality mode or not.
				val->set_bool(true);
				return true;
			}
			case M_FOCUSRECT:
				//else if (name == "_focusrect")
			{
				// Is a yellow rectangle visible around a focused movie clip (?)
				val->set_bool(false);
				return true;
			}
			case M_SOUNDBUFTIME:
				//else if (name == "_soundbuftime")
			{
				// Number of seconds before sound starts to stream.
				val->set_double(0.0);
				return true;
			}
			case M_XMOUSE:
				//else if (name == "_xmouse")
			{
				// Local coord of mouse IN PIXELS.
				int	x, y, buttons;
				assert(m_root);
				m_root->get_mouse_state(&x, &y, &buttons);

				matrix	m = get_world_matrix();

				point	a(PIXELS_TO_TWIPS(x), PIXELS_TO_TWIPS(y));
				point	b;
				
				m.transform_by_inverse(&b, a);

				val->set_double(TWIPS_TO_PIXELS(b.m_x));
				return true;
			}
			case M_YMOUSE:
				//else if (name == "_ymouse")
			{
				// Local coord of mouse IN PIXELS.
				int	x, y, buttons;
				assert(m_root);
				m_root->get_mouse_state(&x, &y, &buttons);

				matrix	m = get_world_matrix();

				point	a(PIXELS_TO_TWIPS(x), PIXELS_TO_TWIPS(y));
				point	b;
				
				m.transform_by_inverse(&b, a);

				val->set_double(TWIPS_TO_PIXELS(b.m_y));
				return true;
			}
			case M_PARENT:
			{
				val->set_as_object_interface(static_cast<as_object_interface*>(m_parent));
				return true;
			}
			case M_ONLOAD:
			{
				if (m_as_environment.get_member(name, val))
				{
					return true;
				}
				// Optimization: if no hit, don't bother looking in the display list, etc.
				return false;
			}
			}	// end switch

			// Try variables.
			if (m_as_environment.get_member(name, val))
			{
				return true;
			}

			// Not a built-in property.  Check items on our
			// display list.
			character*	ch = m_display_list.get_character_by_name_i(name);
			if (ch)
			{
				// Found object.
				val->set_as_object_interface(static_cast<as_object_interface*>(ch));
				return true;
			}

			// Try static builtin functions.
			assert(s_sprite_builtins);
			if (s_sprite_builtins->get_member(name, val))
			{
				return true;
			}

			return false;
		}


		/* sprite_instance */
		virtual movie*	get_relative_target(const tu_string& name)
		// Find the movie which is one degree removed from us,
		// given the relative pathname.
		//
		// If the pathname is "..", then return our parent.
		// If the pathname is ".", then return ourself.  If
		// the pathname is "_level0" or "_root", then return
		// the root movie.
		//
		// Otherwise, the name should refer to one our our
		// named characters, so we return it.
		//
		// NOTE: In ActionScript 2.0, top level names (like
		// "_root" and "_level0") are CASE SENSITIVE.
		// Character names in a display list are CASE
		// SENSITIVE. Member names are CASE INSENSITIVE.  Gah.
		//
		// In ActionScript 1.0, everything seems to be CASE
		// INSENSITIVE.
		{
			if (name == "." || name == "this")
			{
				return this;
			}
			else if (name == "..")
			{
				return get_parent();
			}
			else if (name == "_level0"
				 || name == "_root")
			{
				return m_root->m_movie.get_ptr();
			}

			// See if we have a match on the display list.
			return m_display_list.get_character_by_name(name);
		}


		/* sprite_instance */
		virtual void	call_frame_actions(const as_value& frame_spec)
		// Execute the actions for the specified frame.  The
		// frame_spec could be an integer or a string.
		{
			int	frame_number = -1;

			// Figure out what frame to call.
			if (frame_spec.get_type() == as_value::STRING)
			{
				if (m_def->get_labeled_frame(frame_spec.to_string(), &frame_number) == false)
				{
					// Try converting to integer.
					frame_number = (int) frame_spec.to_number();
				}
			}
			else
			{
				// convert from 1-based to 0-based
				frame_number = (int) frame_spec.to_number() - 1;
			}

			if (frame_number < 0 || frame_number >= m_def->get_frame_count())
			{
				// No dice.
				log_error("error: call_frame('%s') -- unknown frame\n", frame_spec.to_string());
				return;
			}

			int	top_action = m_action_list.size();

			// Execute the actions.
			const array<execute_tag*>&	playlist = m_def->get_playlist(frame_number);
			for (int i = 0; i < playlist.size(); i++)
			{
				execute_tag*	e = playlist[i];
				if (e->is_action_tag())
				{
					e->execute(this);
				}
			}

			// Execute any new actions triggered by the tag,
			// leaving existing actions to be executed.
			while (m_action_list.size() > top_action)
			{
				m_action_list[top_action]->execute(&m_as_environment);
				m_action_list.remove(top_action);
			}
			assert(m_action_list.size() == top_action);
		}


		/* sprite_instance */
		virtual void	set_drag_state(const drag_state& st)
		{
			m_root->m_drag_state = st;
		}

		/* sprite_instance */
		virtual void	stop_drag()
		{
			assert(m_parent == NULL);	// we must be the root movie!!!
			
			m_root->stop_drag();
		}


		/* sprite_instance */
		virtual void	get_drag_state(drag_state* st)
		{
			*st = m_root->m_drag_state;
		}


		void	clone_display_object(const tu_string& name, const tu_string& newname, Uint16 depth)
		// Duplicate the object with the specified name and add it with a new name 
		// at a new depth.
		{
			character* ch = m_display_list.get_character_by_name(name);
			if (ch)
			{
				array<swf_event*>	dummy_event_handlers;

				add_display_object(
					ch->get_id(),
					newname.c_str(),
					dummy_event_handlers,
					depth,
					true,	// replace if depth is occupied
					ch->get_cxform(),
					ch->get_matrix(),
					ch->get_ratio(),
					ch->get_clip_depth());
				// @@ TODO need to duplicate ch's event handlers, and presumably other members?
				// Probably should make a character::clone() function to handle this.
			}
		}


		void	remove_display_object(const tu_string& name)
		// Remove the object with the specified name.
		{
			character* ch = m_display_list.get_character_by_name(name);
			if (ch)
			{
				// @@ TODO: should only remove movies that were created via clone_display_object --
				// apparently original movies, placed by anim events, are immune to this.
				remove_display_object(ch->get_depth(), ch->get_id());
			}
		}

		
		/* sprite_instance */
		virtual bool	on_event(event_id id)
		// Dispatch event handler(s), if any.
		{
			// Keep m_as_environment alive during any method calls!
			smart_ptr<as_object_interface>	this_ptr(this);

			bool called = false;
			
			// First, check for built-in event handler.
			{
				as_value	method;
				if (get_event_handler(id, &method))
				{
					// Dispatch.
					call_method0(method, &m_as_environment, this);

					called = true;
					// Fall through and call the function also, if it's defined!
					// (@@ Seems to be the behavior for mouse events; not tested & verified for
					// every event type.)
				}
			}

			// Check for member function.
			{
				// In ActionScript 2.0, event method names are CASE SENSITIVE.
				// In ActionScript 1.0, event method names are CASE INSENSITIVE.
				const tu_stringi&	method_name = id.get_function_name().to_tu_stringi();
				if (method_name.length() > 0)
				{
					as_value	method;
					if (get_member(method_name, &method))
					{
						call_method0(method, &m_as_environment, this);
						called = true;
					}
				}
			}

			return called;
		}


		/*sprite_instance*/
		virtual void	on_event_load()
		// Do the events that (appear to) happen as the movie
		// loads.  frame1 tags and actions are executed (even
		// before advance() is called).  Then the onLoad event
		// is triggered.
		{
			execute_frame_tags(0);
			do_actions();
			on_event(event_id::LOAD);
		}

		// Do the events that happen when there is XML data waiting
		// on the XML socket connection.
		virtual void	on_event_xmlsocket_onxml()
		{
			log_msg("FIXME: %s: unimplemented\n", __FUNCTION__);
			on_event(event_id::SOCK_XML);
		}
		
		// Do the events that (appear to) happen on a specified interval.
		virtual void	on_event_interval_timer()
		{
			log_msg("FIXME: %s: unimplemented\n", __FUNCTION__);
			on_event(event_id::TIMER);
		}

		// Do the events that happen as a MovieClip (swf 7 only) loads.
		virtual void	on_event_load_progress()
		{
			log_msg("FIXME: %s: unimplemented\n", __FUNCTION__);
			on_event(event_id::LOAD_PROGRESS);
		}

	/*sprite_instance*/
		virtual const char*	call_method_args(const char* method_name, const char* method_arg_fmt, va_list args)
		{
			// Keep m_as_environment alive during any method calls!
			smart_ptr<as_object_interface>	this_ptr(this);

			return call_method_parsed(&m_as_environment, this, method_name, method_arg_fmt, args);
		}

		/* sprite_instance */
		virtual void	attach_display_callback(const char* path_to_object, void (*callback)(void*), void* user_ptr)
		{
			assert(m_parent == NULL);	// should only be called on the root movie.

			array<with_stack_entry>	dummy;
			as_value	obj = m_as_environment.get_variable(tu_string(path_to_object), dummy);
			as_object_interface*	as_obj = obj.to_object();
			if (as_obj)
			{
				movie*	m = as_obj->to_movie();
				if (m)
				{
					m->set_display_callback(callback, user_ptr);
				}
			}
		}
	};


	character*	sprite_definition::create_character_instance(movie* parent, int id)
	// Create a (mutable) instance of our definition.  The
	// instance is created to live (temporarily) on some level on
	// the parent movie's display list.
	{
		sprite_instance*	si = new sprite_instance(this, parent->get_root(), parent, id);

		return si;
	}


	movie_interface*	movie_def_impl::create_instance()
	// Create a playable movie instance from a def.
	{
		movie_root*	m = new movie_root(this);
		assert(m);

		sprite_instance*	root_movie = new sprite_instance(this, m, NULL, -1);
		assert(root_movie);

		root_movie->set_name("_root");
		m->set_root_movie(root_movie);

		m->add_ref();
		return m;
	}


	void	sprite_loader(stream* in, int tag_type, movie_definition_sub* m)
	// Create and initialize a sprite, and add it to the movie.
	{
		assert(tag_type == 39);
                
		int	character_id = in->read_u16();

		IF_VERBOSE_PARSE(log_msg("  sprite\n  char id = %d\n", character_id));

		sprite_definition*	ch = new sprite_definition(m);	// @@ combine sprite_definition with movie_def_impl
		ch->read(in);

		m->add_character(character_id, ch);
	}


	//
	// sprite built-in ActionScript methods
	//

	void	sprite_play(const fn_call& fn)
	{
		sprite_instance* sprite = (sprite_instance*) fn.this_ptr;
		if (sprite == NULL)
		{
			sprite = (sprite_instance*) fn.env->get_target();
		}
		assert(sprite);
		sprite->set_play_state(movie_interface::PLAY);
	}

	void	sprite_stop(const fn_call& fn)
	{
		sprite_instance* sprite = (sprite_instance*) fn.this_ptr;
		if (sprite == NULL)
		{
			sprite = (sprite_instance*) fn.env->get_target();
		}
		assert(sprite);
		sprite->set_play_state(movie_interface::STOP);
	}

	void	sprite_goto_and_play(const fn_call& fn)
	{
		sprite_instance* sprite = (sprite_instance*) fn.this_ptr;
		if (sprite == NULL)
		{
			sprite = (sprite_instance*) fn.env->get_target();
		}
		assert(sprite);

		if (fn.nargs < 1)
		{
			log_error("error: sprite_goto_and_play needs one arg\n");
			return;
		}

		int	target_frame = int(fn.arg(0).to_number() - 1);	// Convert to 0-based

		sprite->goto_frame(target_frame);
		sprite->set_play_state(movie_interface::PLAY);
	}

	void	sprite_goto_and_stop(const fn_call& fn)
	{
		sprite_instance* sprite = (sprite_instance*) fn.this_ptr;
		if (sprite == NULL)
		{
			sprite = (sprite_instance*) fn.env->get_target();
		}
		assert(sprite);

		if (fn.nargs < 1)
		{
			log_error("error: sprite_goto_and_stop needs one arg\n");
			return;
		}

		int	target_frame = int(fn.arg(0).to_number() - 1);	// Convert to 0-based

		sprite->goto_frame(target_frame);
		sprite->set_play_state(movie_interface::STOP);
	}

	void	sprite_next_frame(const fn_call& fn)
	{
		sprite_instance* sprite = (sprite_instance*) fn.this_ptr;
		if (sprite == NULL)
		{
			sprite = (sprite_instance*) fn.env->get_target();
		}
		assert(sprite);

		int frame_count = sprite->get_frame_count();
		int current_frame = sprite->get_current_frame();
		if (current_frame < frame_count)
		{
			sprite->goto_frame(current_frame + 1);
		}
		sprite->set_play_state(movie_interface::STOP);
	}

	void	sprite_prev_frame(const fn_call& fn)
	{
		sprite_instance* sprite = (sprite_instance*) fn.this_ptr;
		if (sprite == NULL)
		{
			sprite = (sprite_instance*) fn.env->get_target();
		}
		assert(sprite);

		int current_frame = sprite->get_current_frame();
		if (current_frame > 0)
		{
			sprite->goto_frame(current_frame - 1);
		}
		sprite->set_play_state(movie_interface::STOP);
	}

	void	sprite_get_bytes_loaded(const fn_call& fn)
	{
		sprite_instance* sprite = (sprite_instance*) fn.this_ptr;
		if (sprite == NULL)
		{
			sprite = (sprite_instance*) fn.env->get_target();
		}
		assert(sprite);

		fn.result->set_int(sprite->get_root()->get_file_bytes());
	}

	void	sprite_get_bytes_total(const fn_call& fn)
	{
		sprite_instance* sprite = (sprite_instance*) fn.this_ptr;
		if (sprite == NULL)
		{
			sprite = (sprite_instance*) fn.env->get_target();
		}
		assert(sprite);

		fn.result->set_int(sprite->get_root()->get_file_bytes());
	}


	static void	sprite_builtins_init()
	{
		if (s_sprite_builtins)
		{
			return;
		}

		s_sprite_builtins = new as_object;
		s_sprite_builtins->set_member("play", &sprite_play);
		s_sprite_builtins->set_member("stop", &sprite_stop);
		s_sprite_builtins->set_member("gotoAndStop", &sprite_goto_and_stop);
		s_sprite_builtins->set_member("gotoAndPlay", &sprite_goto_and_play);
		s_sprite_builtins->set_member("nextFrame", &sprite_next_frame);
		s_sprite_builtins->set_member("prevFrame", &sprite_prev_frame);
		s_sprite_builtins->set_member("getBytesLoaded", &sprite_get_bytes_loaded);
		s_sprite_builtins->set_member("getBytesTotal", &sprite_get_bytes_loaded);

		// @TODO
//		s_sprite_builtins->set_member("startDrag", &sprite_start_drag);
//		s_sprite_builtins->set_member("stopDrag", &sprite_stop_drag);
//		s_sprite_builtins->set_member("getURL", &sprite_get_url);
//		s_sprite_builtins->set_member("swapDepths", &sprite_swap_depths);
	}

	static void	sprite_builtins_clear()
	{
		if (s_sprite_builtins)
		{
			delete s_sprite_builtins;
			s_sprite_builtins = 0;
		}
	}





	//
	// end_tag
	//

	// end_tag doesn't actually need to exist.

	void	end_loader(stream* in, int tag_type, movie_definition_sub* m)
	{
		assert(tag_type == 0);
		assert(in->get_position() == in->get_tag_end_position());
	}


	//
	// remove_object_2
	//

	
	struct remove_object_2 : public execute_tag
	{
		int	m_depth, m_id;

		remove_object_2() : m_depth(-1), m_id(-1) {}

		void	read(stream* in, int tag_type)
		{
			assert(tag_type == 5 || tag_type == 28);

			if (tag_type == 5)
			{
				// Older SWF's allow multiple objects at the same depth;
				// this m_id disambiguates.  Later SWF's just use one
				// object per depth.
				m_id = in->read_u16();
			}
			m_depth = in->read_u16();
		}

		virtual void	execute(movie* m)
		{
			m->remove_display_object(m_depth, m_id);
		}

		virtual void	execute_state(movie* m)
		{
			execute(m);
		}

		virtual void	execute_state_reverse(movie* m, int frame)
		{
			// reverse of remove is to re-add the previous object.
			execute_tag*	last_add = m->find_previous_replace_or_add_tag(frame, m_depth, m_id);
			if (last_add)
			{
				last_add->execute_state(m);
			}
			else
			{
				log_error("reverse REMOVE can't find previous replace or add tag(%d, %d)\n",
					  frame, m_depth);
					
			}
		}

		virtual bool	is_remove_tag() const { return true; }
	};


	void	remove_object_2_loader(stream* in, int tag_type, movie_definition_sub* m)
	{
		assert(tag_type == 5 || tag_type == 28);

		remove_object_2*	t = new remove_object_2;
		t->read(in, tag_type);

		IF_VERBOSE_PARSE(log_msg("  remove_object_2(%d)\n", t->m_depth));

		m->add_execute_tag(t);
	}


	void	button_sound_loader(stream* in, int tag_type, movie_definition_sub* m)
	{
		assert(tag_type == 17);

		int	button_character_id = in->read_u16();
		button_character_definition* ch = (button_character_definition*) m->get_character_def(button_character_id);
		assert(ch != NULL);

		ch->read(in, tag_type, m);
	}


	void	button_character_loader(stream* in, int tag_type, movie_definition_sub* m)
	{
		assert(tag_type == 7 || tag_type == 34);

		int	character_id = in->read_u16();

		IF_VERBOSE_PARSE(log_msg("  button character loader: char_id = %d\n", character_id));

		button_character_definition*	ch = new button_character_definition;
		ch->read(in, tag_type, m);

		m->add_character(character_id, ch);
	}


	//
	// export
	//


	void	export_loader(stream* in, int tag_type, movie_definition_sub* m)
	// Load an export tag (for exposing internal resources of m)
	{
		assert(tag_type == 56);

		int	count = in->read_u16();

		IF_VERBOSE_PARSE(log_msg("  export: count = %d\n", count));

		// Read the exports.
		for (int i = 0; i < count; i++)
		{
			Uint16	id = in->read_u16();
			char*	symbol_name = in->read_string();
			IF_VERBOSE_PARSE(log_msg("  export: id = %d, name = %s\n", id, symbol_name));

			if (font* f = m->get_font(id))
			{
				// Expose this font for export.
				m->export_resource(tu_string(symbol_name), f);
			}
			else if (character_def* ch = m->get_character_def(id))
			{
				// Expose this movie/button/whatever for export.
				m->export_resource(tu_string(symbol_name), ch);
			}
			else if (sound_sample* ch = m->get_sound_sample(id))
			{
				m->export_resource(tu_string(symbol_name), ch);
			}
			else
			{
				log_error("export error: don't know how to export resource '%s'\n",
					  symbol_name);
			}

			delete [] symbol_name;
		}
	}


	//
	// import
	//


	void	import_loader(stream* in, int tag_type, movie_definition_sub* m)
	// Load an import tag (for pulling in external resources)
	{
		assert(tag_type == 57);

		char*	source_url = in->read_string();
		int	count = in->read_u16();

		IF_VERBOSE_PARSE(log_msg("  import: source_url = %s, count = %d\n", source_url, count));

		// Try to load the source movie into the movie library.
		movie_definition_sub*	source_movie = NULL;

		if (s_no_recurse_while_loading == false)
		{
			source_movie = create_library_movie_sub(source_url);
			if (source_movie == NULL)
			{
				// Give up on imports.
				log_error("can't import movie from url %s\n", source_url);
				return;
			}
		}

		// Get the imports.
		for (int i = 0; i < count; i++)
		{
			Uint16	id = in->read_u16();
			char*	symbol_name = in->read_string();
			IF_VERBOSE_PARSE(log_msg("  import: id = %d, name = %s\n", id, symbol_name));

			if (s_no_recurse_while_loading)
			{
				m->add_import(source_url, id, symbol_name);
			}
			else
			{
				// @@ TODO get rid of this, always use
				// s_no_recurse_while_loading, change
				// create_movie_sub().

				smart_ptr<resource> res = source_movie->get_exported_resource(symbol_name);
				if (res == NULL)
				{
					log_error("import error: resource '%s' is not exported from movie '%s'\n",
						  symbol_name, source_url);
				}
				else if (font* f = res->cast_to_font())
				{
					// Add this shared font to the currently-loading movie.
					m->add_font(id, f);
				}
				else if (character_def* ch = res->cast_to_character_def())
				{
					// Add this character to the loading movie.
					m->add_character(id, ch);
				}
				else
				{
					log_error("import error: resource '%s' from movie '%s' has unknown type\n",
						  symbol_name, source_url);
				}
			}

			delete [] symbol_name;
		}

		delete [] source_url;
	}
}

