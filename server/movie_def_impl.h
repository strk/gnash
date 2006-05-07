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

#ifndef GNASH_MOVIE_DEF_IMPL_H
#define GNASH_MOVIE_DEF_IMPL_H

#include "container.h"
#include "button.h" // for mouse_button_state
#include "timers.h" // for Timer
#include "fontlib.h"
#include "font.h"
#include "jpeg.h"
#include "tu_file.h"
#include "movie_definition.h" // for inheritance

namespace gnash
{
	// Forward declarations
	class import_info;
	struct movie_def_impl;
	struct movie_root;
	struct import_visitor; // in gnash.h

	//
	// Helper for movie_def_impl
	//
	class import_info
	{
	    friend class movie_def_impl;

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


/// Immutable definition of a movie's contents.
//
/// It cannot be played directly, and does not hold
/// current state; for that you need to call create_instance()
/// to get a movie_instance (movie_interface).
///
class movie_def_impl : public movie_definition
{
	hash<int, smart_ptr<character_def> >		m_characters;
	hash<int, smart_ptr<font> >	 		m_fonts;
	hash<int, smart_ptr<bitmap_character_def> >	m_bitmap_characters;
	hash<int, smart_ptr<sound_sample> >		m_sound_samples;

	/// A list of movie control events for each frame.
	std::vector<std::vector<execute_tag*> >	   	m_playlist;

	/// Init actions for each frame.
	std::vector<std::vector<execute_tag*> >	   m_init_action_list;

	/// 0-based frame #'s
	stringi_hash<int> m_named_frames;

	stringi_hash<smart_ptr<resource> > m_exports;

	/// Items we import.
	std::vector<import_info> m_imports;

	/// Movies we import from; hold a ref on these,
	/// to keep them alive
	std::vector<smart_ptr<movie_definition> > m_import_source_movies;

	/// Bitmaps used in this movie; collected in one place to make
	/// it possible for the host to manage them as textures.
	std::vector<smart_ptr<bitmap_info> >	m_bitmap_list;

	create_bitmaps_flag	m_create_bitmaps;
	create_font_shapes_flag	m_create_font_shapes;

	rect	m_frame_size;
	float	m_frame_rate;
	int	m_frame_count;
	int	m_version;
	int	m_loading_frame;
	uint32	m_file_length;

	jpeg::input*	m_jpeg_in;

public:
	movie_def_impl(create_bitmaps_flag cbf,
			create_font_shapes_flag cfs)
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

	~movie_def_impl();

	// ...
	int	get_frame_count() const { return m_frame_count; }
	float	get_frame_rate() const { return m_frame_rate; }
	const rect& get_frame_size() const { return m_frame_size; }

	float	get_width_pixels() const
	{
		return ceilf(TWIPS_TO_PIXELS(m_frame_size.width()));
	}

	float	get_height_pixels() const
	{
		return ceilf(TWIPS_TO_PIXELS(m_frame_size.height()));
	}

	virtual int	get_version() const { return m_version; }

	virtual int	get_loading_frame() const
	{
		return m_loading_frame;
	}

	uint32	get_file_bytes() const { return m_file_length; }

	/// Returns DO_CREATE_BITMAPS if we're supposed to
	/// initialize our bitmap infos, or DO_NOT_INIT_BITMAPS
	/// if we're supposed to create blank placeholder
	/// bitmaps (to be init'd later explicitly by the host
	/// program).
	virtual create_bitmaps_flag get_create_bitmaps() const
	{
		return m_create_bitmaps;
	}

	/// Returns DO_LOAD_FONT_SHAPES if we're supposed to
	/// initialize our font shape info, or
	/// DO_NOT_LOAD_FONT_SHAPES if we're supposed to not
	/// create any (vector) font glyph shapes, and instead
	/// rely on precached textured fonts glyphs.
	virtual create_font_shapes_flag	get_create_font_shapes() const
	{
	    return m_create_font_shapes;
	}

	/// All bitmap_info's used by this movie should be
	/// registered with this API.
	virtual void	add_bitmap_info(bitmap_info* bi)
	{
	    m_bitmap_list.push_back(bi);
	}

	virtual int get_bitmap_info_count() const
	{
		return m_bitmap_list.size();
	}

	virtual bitmap_info*	get_bitmap_info(int i) const
	{
		return m_bitmap_list[i].get_ptr();
	}

	/// Expose one of our resources under the given symbol,
	/// for export.  Other movies can import it.
	virtual void export_resource(const tu_string& symbol,
			resource* res)
	{
	    // SWF sometimes exports the same thing more than once!
	    m_exports[symbol] = res;
	}

	/// Get the named exported resource, if we expose it.
	/// Otherwise return NULL.
	virtual smart_ptr<resource> get_exported_resource(const tu_string& symbol)
	{
	    smart_ptr<resource>	res;
	    m_exports.get(symbol, &res);
	    return res;
	}

	/// Adds an entry to a table of resources that need to
	/// be imported from other movies.  Client code must
	/// call resolve_import() later, when the source movie
	/// has been loaded, so that the actual resource can be
	/// used.
	virtual void add_import(const char* source_url, int id, const char* symbol)
	{
	    assert(in_import_table(id) == false);

	    m_imports.push_back(import_info(source_url, id, symbol));
	}

	/// Debug helper; returns true if the given
	/// character_id is listed in the import table.
	bool in_import_table(int character_id);

	/// Calls back the visitor for each movie that we
	/// import symbols from.
	virtual void visit_imported_movies(import_visitor* visitor);

	/// Grabs the stuff we want from the source movie.
	virtual void resolve_import(const char* source_url,
		movie_definition* source_movie);

	void add_character(int character_id, character_def* c);

	character_def*	get_character_def(int character_id);

	/// Returns 0-based frame #
	bool get_labeled_frame(const char* label, int* frame_number)
	{
		return m_named_frames.get(label, frame_number);
	}

	void	add_font(int font_id, font* f);
	font*	get_font(int font_id);
	bitmap_character_def*	get_bitmap_character(int character_id);
	void	add_bitmap_character(int character_id, bitmap_character_def* ch);
	sound_sample*	get_sound_sample(int character_id);
	virtual void	add_sound_sample(int character_id, sound_sample* sam);

	/// Add an execute_tag to this movie_definition's playlist
	void	add_execute_tag(execute_tag* e)
	{
	    assert(e);
	    m_playlist[m_loading_frame].push_back(e);
	}

	/// Need to execute the given tag before entering the
	/// currently-loading frame for the first time.
	///
	/// @@ AFAIK, the sprite_id is totally pointless -- correct?
	void	add_init_action(int sprite_id, execute_tag* e)
	{
	    assert(e);
	    m_init_action_list[m_loading_frame].push_back(e);
	}

	/// Labels the frame currently being loaded with the
	/// given name.  A copy of the name string is made and
	/// kept in this object.
	void	add_frame_name(const char* name)
	{
	    assert(m_loading_frame >= 0 && m_loading_frame < m_frame_count);

	    tu_string	n = name;
	    assert(m_named_frames.get(n, NULL) == false);	// frame should not already have a name (?)
	    m_named_frames.add(n, m_loading_frame);	// stores 0-based frame #
	}

	/// Set an input object for later loading DefineBits
	/// images (JPEG images without the table info).
	void	set_jpeg_loader(jpeg::input* j_in)
	{
	    assert(m_jpeg_in == NULL);
	    m_jpeg_in = j_in;
	}

	/// Get the jpeg input loader, to load a DefineBits
	/// image (one without table info).
	jpeg::input*	get_jpeg_loader()
	{
	    return m_jpeg_in;
	}

	virtual const std::vector<execute_tag*>& get_playlist(int frame_number) { return m_playlist[frame_number]; }

	virtual const std::vector<execute_tag*>*get_init_actions(int frame_number) { return &m_init_action_list[frame_number]; }

	/// Read Movie definition from an SWF file.
	//
	/// This function uses the gnash::s_tag_loaders
	/// global variable to interpret specific tag types.
	///
	void read(tu_file *in);

	/// Fill up *fonts with fonts that we own.
	void get_owned_fonts(std::vector<font*>* fonts);

	/// Generate bitmaps for our fonts, if necessary.
	void generate_font_bitmaps();

	/// Dump our cached data into the given stream.
	void output_cached_data(tu_file* out,
		const cache_options& options);

	/// \brief
	/// Read in cached data and use it to prime our
	/// loaded characters.
	void	input_cached_data(tu_file* in);

	/// \brief
	/// Create a playable movie_root instance from a def.
	//
	/// The _root reference of the newly created instance
	/// will be set to a newly created sprite_instace (Help!)
	///
	movie_interface* create_instance();
};

} // namespace gnash

#endif // GNASH_MOVIE_DEF_IMPL_H
