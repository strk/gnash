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

#ifndef GNASH_SPRITE_DEFINITION_H
#define GNASH_SPRITE_DEFINITION_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <vector>

#include "Movie.h"
//#include "dlist.h" // display_list 
#include "stream.h"
#include "log.h"

namespace gnash
{


/// \brief
/// Holds the immutable data for a sprite, as read from
/// as SWF stream.
///
class sprite_definition : public movie_definition
{

public:

	sprite_definition(movie_definition* m);

	/// Destructor, releases playlist data
	~sprite_definition();

	/// \brief
	/// Read the sprite info from input stream.
	//
	/// Consists of a series of tags.
	///
	// @@ Could be another constructor
	void read(stream* in);

private:

	/// parent movie.
	movie_definition* m_movie_def;

	/// movie control events for each frame.
	std::vector<std::vector<execute_tag*> > m_playlist;

	// stores 0-based frame #'s
	stringi_hash<int> m_named_frames;

	int m_frame_count;

	int m_loading_frame;

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

	virtual void output_cached_data(tu_file* out,
		const cache_options& options)
	{
	    // Nothing to do.
	    return;
	}

	virtual void	input_cached_data(tu_file* in)
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
		movie* parent, int id);


	virtual void	add_execute_tag(execute_tag* c)
	{
		m_playlist[m_loading_frame].push_back(c);
	}

	virtual void	add_init_action(int sprite_id, execute_tag* c)
	{
	    // Sprite def's should not have do_init_action tags in them!  (@@ correct?)
	    log_error("sprite_definition::add_init_action called!  Ignored.\n");
	}

	/// \brief
	/// Labels the frame currently being loaded with the
	/// given name.  A copy of the name string is made and
	/// kept in this object.
	///
	virtual void	add_frame_name(const char* name);

	/// Returns 0-based frame #
	bool	get_labeled_frame(const char* label, int* frame_number)
	{
	    return m_named_frames.get(label, frame_number);
	}

	/// frame_number is 0-based
	const std::vector<execute_tag*>&	get_playlist(int frame_number)
	{
	    return m_playlist[frame_number];
	}

	// Sprites do not have init actions in their
	// playlists!  Only the root movie
	// (movie_def_impl) does (@@ correct?)
	virtual const std::vector<execute_tag*>*get_init_actions(int frame_number)
	{
	    return NULL;
	}


};


} // end of namespace gnash

#endif // GNASH_SPRITE_H
