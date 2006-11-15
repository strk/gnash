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

// 
//

#ifndef GNASH_SPRITE_DEFINITION_H
#define GNASH_SPRITE_DEFINITION_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <vector>

#include "movie_definition.h"
#include "stream.h"
#include "log.h"

namespace gnash
{


/// \brief
/// Holds the immutable data for a sprite, as read from
/// as SWF stream.
/// @@ should *not* derive from movie_definition, probably!
///
class sprite_definition : public movie_definition
{

public:

	/// \brief
	/// Read the sprite info from input stream.
	//
	/// A sprite definition consists of a series control tags.
	///
	/// @param m
	///	the Top-Level movie_definition this sprite is read
	///	from (not a sprite_definition!)
	///
	/// @param in
	///	The stream associated with the sprite. It is assumed
	///	to be already positioned right before the frame count
	///         
	sprite_definition(movie_definition* m, stream* in);

	/// Destructor, releases playlist data
	~sprite_definition();

private:

	void read(stream* in);

	/// Tags loader table.
	//
	/// TODO: make it a static member, specific to sprite_definition
	SWF::TagLoadersTable& _tag_loaders;

	/// Top-level movie definition
	/// (the definition read from SWF stream)
	movie_definition* m_movie_def;

	/// movie control events for each frame.
	std::vector<PlayList> m_playlist;

	// stores 0-based frame #'s
	stringi_hash<size_t> m_named_frames;

	size_t m_frame_count;

	size_t m_loading_frame;

	// overloads from movie_definition
	virtual float	get_width_pixels() const { return 1; }
	virtual float	get_height_pixels() const { return 1; }

	virtual size_t	get_frame_count() const
	{
		return m_frame_count;
	}

	/// \brief
	/// Return total bytes of the movie from which this sprite
	/// has been read.
	///
	virtual size_t get_bytes_total() const
	{
		return m_movie_def ? m_movie_def->get_bytes_total() : 0;
	}

	/// \brief
	/// Return the number of bytes loaded from the stream of the
	/// the movie from which this sprite is being read.
	///
	virtual size_t get_bytes_loaded() const
	{
		return m_movie_def ? m_movie_def->get_bytes_loaded() : 0;
	}

	virtual float	get_frame_rate() const
	{
		// or should we assert(0) here ?
		return m_movie_def ? m_movie_def->get_frame_rate() : 0.0;
	}

	// Return number of frames loaded (of current sprite)
	virtual size_t	get_loading_frame() const { return m_loading_frame; }

	virtual int	get_version() const
	{
		// how do we tell version if we don't have a refenrece
		// to the topmost movie ??
		assert(m_movie_def);
		return m_movie_def->get_version();
	}

	virtual void add_font(int /*id*/, font* /*ch*/)
	{
		log_error("add_font tag appears in sprite tags! "
			"Malformed SWF?\n");
	}

	virtual font* get_font(int id)
	{
		// how do we resolve fonts if we don't have a reference
		// to the topmost movie ??
		return m_movie_def ? m_movie_def->get_font(id) : NULL;
	}

	virtual void set_jpeg_loader(std::auto_ptr<jpeg::input> /*j_in*/)
	{
		assert(0);
	}

	virtual jpeg::input* get_jpeg_loader()
	{
		return NULL;
	}

	virtual bitmap_character_def* get_bitmap_character_def(int id)
	{
		// how do we query characters by id
		// if we don't have a reference to the topmost movie ??
		return m_movie_def ?
			m_movie_def->get_bitmap_character_def(id)
			:
			NULL;
	}

	virtual void add_bitmap_character_def(int /*id*/,
			bitmap_character_def* /*ch*/)
	{
		log_error("add_bc appears in sprite tags!"
			" Malformed SWF?");
	}

	virtual sound_sample* get_sound_sample(int id)
	{
		// how do we query sound samples by id
		// if we don't have a reference to the topmost movie ??
		return m_movie_def ? m_movie_def->get_sound_sample(id) : NULL;
	}

	virtual void add_sound_sample(int /*id*/, sound_sample* /*sam*/)
	{
		log_error("add sam appears in sprite tags!"
			" Malformed SWF?");
	}

	virtual void set_loading_sound_stream_id(int id) 
	{
		if ( m_movie_def )
		{
			m_movie_def->set_loading_sound_stream_id(id);
		}

	}

	virtual int get_loading_sound_stream_id()
	{ 
		return m_movie_def ? 
			m_movie_def->get_loading_sound_stream_id()
			:
			-1;
	}

	
	// @@ would be nicer to not inherit these...
	virtual create_bitmaps_flag	get_create_bitmaps() const
	{ assert(0); return DO_LOAD_BITMAPS; }
	virtual create_font_shapes_flag	get_create_font_shapes() const
	{ assert(0); return DO_LOAD_FONT_SHAPES; }
	virtual int	get_bitmap_info_count() const
	{ assert(0); return 0; }
	virtual bitmap_info*	get_bitmap_info(int /*i*/) const
	{ assert(0); return NULL; }
	virtual void	add_bitmap_info(bitmap_info* /*bi*/)
	{ assert(0); }

	virtual void export_resource(const tu_string& /*symbol*/,
			resource* /*res*/)
	{
		log_error("can't export from sprite! Malformed SWF?");
	}

	virtual boost::intrusive_ptr<resource> get_exported_resource(const tu_string& sym)
	{
		return m_movie_def ? 
			m_movie_def->get_exported_resource(sym)
			:
			NULL;
	}

	virtual void add_import(const char* /*source_url*/, int /*id*/,
			const char* /*symbol*/)
	{
		assert(0);
	}

	virtual void visit_imported_movies(import_visitor* /*v*/)
	{
		assert(0);
	}

	virtual void resolve_import(const char* /*source_url*/,
			movie_definition* /*d*/)
	{
		assert(0);
	}


	/// \brief
	/// Get a character_def from this Sprite's parent
	/// CharacterDictionary. NOTE that calling this
	/// method on the leaf Sprite of a movie_definition
	/// hierarchy will result in a recursive scan of
	/// all parents until the top-level movie_definition
	/// (movie_def_impl) is found.
	///
	virtual character_def*	get_character_def(int id)
	{
	    return m_movie_def ?
	    	m_movie_def->get_character_def(id)
		:
		NULL;
	}

	/// Calls to this function should only be made when
	/// an invalid SWF is being read, as it would mean
	/// that a Definition tag is been found as part of
	/// a Sprite definition
	///
	virtual void add_character(int /*id*/, character_def* /*ch*/)
	{
		log_error("add_character tag appears in sprite tags!"
				" Maformed SWF?");
	}


	virtual void generate_font_bitmaps()
	{
		assert(0);
	}

	virtual void output_cached_data(tu_file* /*out*/,
		const cache_options& /*options*/)
	{
	    // Nothing to do.
	    return;
	}

	virtual void	input_cached_data(tu_file* /*in*/)
	{
	    // Nothing to do.
	    return;
	}

	virtual movie_interface* create_instance()
	{
	    return NULL;
	}

	// Create a (mutable) instance of our definition.  The
	// instance is created to live (temporarily) on some level on
	// the parent movie's display list.
	//
	// overloads from character_def
	virtual character* create_character_instance(
		character* parent, int id);


	virtual void	add_execute_tag(execute_tag* c)
	{
		m_playlist[m_loading_frame].push_back(c);
	}

	//virtual void	add_init_action(int sprite_id, execute_tag* c)
	virtual void	add_init_action(execute_tag* /*c*/)
	{
	    // Sprite def's should not have do_init_action tags in them!  (@@ correct?)
	    log_error("sprite_definition::add_init_action called!  Ignored. (Malformed SWF?)\n");
	}

	/// \brief
	/// Labels the frame currently being loaded with the
	/// given name.  A copy of the name string is made and
	/// kept in this object.
	///
	virtual void	add_frame_name(const char* name);

	/// Returns 0-based frame #
	bool	get_labeled_frame(const char* label, size_t* frame_number)
	{
	    return m_named_frames.get(label, frame_number);
	}

	/// frame_number is 0-based
	const PlayList& get_playlist(size_t frame_number)
	{
		return m_playlist[frame_number];
	}

	// Sprites do not have init actions in their
	// playlists!  Only the root movie
	// (movie_def_impl) does (@@ correct?)
	virtual const PlayList* get_init_actions(size_t /*frame_number*/)
	{
	    return NULL;
	}

	virtual const std::string& get_url() const
	{
	    static std::string null_url;
	    return m_movie_def ?
	    	m_movie_def->get_url()
		:
		null_url;
	}

	/// \brief
	/// Ensure framenum frames of this sprite 
	/// have been loaded.
	///
	virtual bool ensure_frame_loaded(size_t framenum)
	{
		// TODO: return false on timeout 
		while ( m_loading_frame < framenum )
		{
			log_msg("sprite_definition: "
				"loading of frame " SIZET_FMT " requested "
				"(we are at " SIZET_FMT "/" SIZET_FMT ")",
				framenum, m_loading_frame, m_frame_count);
			// Could this ever happen ?
			assert(0);
		}
		return true;
	}

	virtual void load_next_frame_chunk()
	{
		/// We load full sprite definitions at once, so
		/// this function is a no-op. 
	}

	/// Return the top-level movie definition
	/// (the definition read from SWF stream)
	movie_definition* get_movie_definition() {
		return m_movie_def;
	}

	const rect&	get_bound() const {
    // It is required that get_bound() is implemented in character definition
    // classes. However, it makes no sense to call it for sprite definitions.
    // get_bound() is currently only used by generic_character which normally
    // is used only shape character definitions. See character_def.h to learn
    // why it is virtual anyway.
    assert(0); // should not be called  
		static rect unused;
		return unused;
  }
			
};


} // end of namespace gnash

#endif // GNASH_SPRITE_H
