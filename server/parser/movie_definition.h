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


/// \page movie SWF Movies
///
/// SWF Movies definitions are created by reading an SWF stream.
/// Gnash doesn't play SWF Movie definitions, but instances.
/// So you can play the same SWF file (Movie definiton) using
/// multiple instances.
///
/// A Movie definition is defined by the gnash::movie_definition class.
/// A Movie instance is defined by the gnash::movie_interface class.
/// 
/// A Movie instance exposes the ActionScript
/// Object base interface (gnash::as_object),
/// thus it can manage gnash::as_value members.
///
/// The implementation of SWF parsing for a Movie definition
/// is found in gnash::movie_def_impl::read.
/// Note that movie_definition is also used as a base class
/// to sprite_definition, which is a sub-movie defined in an SWF
/// file. This seems to be the only reason to have a
/// movie_def_impl class, being the top-level definition of
/// a movie (the one with a CharacterDictionary in it).
///


#ifndef GNASH_MOVIE_DEFINITION_H
#define GNASH_MOVIE_DEFINITION_H

#include "character_def.h" // for inheritance
#include "container.h"
//#include "button.h" // for mouse_button_state
#include "timers.h" // for Timer
#include "fontlib.h"
#include "font.h"
#include "jpeg.h"
#include "tu_file.h"

#include <string>
#include <memory> // for auto_ptr

namespace gnash
{

/// Client program's interface to the definition of a movie
//
/// (i.e. the shared constant source info).
///
class movie_definition : public character_def
{
public:
	typedef std::vector<execute_tag*> PlayList;

	virtual int	get_version() const = 0;
	virtual float	get_width_pixels() const = 0;
	virtual float	get_height_pixels() const = 0;
	virtual size_t	get_frame_count() const = 0;
	virtual float	get_frame_rate() const = 0;

	virtual size_t get_bytes_loaded() const = 0;
	virtual size_t get_bytes_total() const = 0;
	
	/// Create a playable movie instance from a def.
	//
	/// This calls add_ref() on the movie_interface internally.
	/// Call drop_ref() on the movie_interface when you're done with it.
	/// Or use boost::intrusive_ptr<T> from base/smart_ptr.h if you want.
	///
	virtual movie_interface*	create_instance() = 0;
	
	virtual void	output_cached_data(tu_file* out, const cache_options& options) = 0;
	virtual void	input_cached_data(tu_file* in) = 0;
	
	/// \brief
	/// Causes this movie def to generate texture-mapped
	/// versions of all the fonts it owns. 
	//
	/// This improves
	/// speed and quality of text rendering.  The
	/// texture-map data is serialized in the
	/// output/input_cached_data() calls, so you can
	/// preprocess this if you load cached data.
	///
	virtual void	generate_font_bitmaps() = 0;
	
	//
	// (optional) API to support gnash::create_movie_no_recurse().
	//
	
	/// \brief
	/// Call visit_imported_movies() to retrieve a list of
	/// names of movies imported into this movie.
	//
	/// visitor->visit() will be called back with the name
	/// of each imported movie.
	class import_visitor
	{
	public:
	    virtual ~import_visitor() {}
	    virtual void	visit(const char* imported_movie_filename) = 0;
	};
	virtual void	visit_imported_movies(import_visitor* visitor) = 0;
	
	/// Call this to resolve an import of the given movie.
	/// Replaces the dummy placeholder with the real
	/// movie_definition* given.
	virtual void	resolve_import(const char* name, movie_definition* def) = 0;
	
	//
	// (optional) API to support host-driven creation of textures.
	//
	// Create the movie using gnash::create_movie_no_recurse(..., DO_NOT_LOAD_BITMAPS),
	// and then initialize each bitmap info via get_bitmap_info_count(), get_bitmap_info(),
	// and bitmap_info::init_*_image() or your own subclassed API.
	//
	// E.g.:
	//
	// // During preprocessing:
	// // This will create bitmap_info's using the rgba, rgb, alpha contructors.
	// my_def = gnash::create_movie_no_recurse("myfile.swf", DO_LOAD_BITMAPS);
	// int ct = my_def->get_bitmap_info_count();
	// for (int i = 0; i < ct; i++)
	// {
	//	my_bitmap_info_subclass*	bi = NULL;
	//	my_def->get_bitmap_info(i, (bitmap_info**) &bi);
	//	my_precomputed_textures.push_back(bi->m_my_internal_texture_reference);
	// }
	// // Save out my internal data.
	// my_precomputed_textures->write_into_some_cache_stream(...);
	//
	// // Later, during run-time loading:
	// my_precomputed_textures->read_from_some_cache_stream(...);
	// // This will create blank bitmap_info's.
	// my_def = gnash::create_movie_no_recurse("myfile.swf", DO_NOT_LOAD_BITMAPS);
	// 
	// // Push cached texture info into the movie's bitmap_info structs.
	// int	ct = my_def->get_bitmap_info_count();
	// for (int i = 0; i < ct; i++)
	// {
	//	my_bitmap_info_subclass*	bi = (my_bitmap_info_subclass*) my_def->get_bitmap_info(i);
	//	bi->set_internal_texture_reference(my_precomputed_textures[i]);
	// }
	virtual int	get_bitmap_info_count() const = 0;
	virtual bitmap_info*	get_bitmap_info(int i) const = 0;

	// From movie_definition_sub

	virtual const PlayList& get_playlist(size_t frame_number) = 0;
	virtual const PlayList* get_init_actions(size_t frame_number) = 0;
	virtual boost::intrusive_ptr<resource>	get_exported_resource(const tu_string& symbol) = 0;


	/// \brief
	/// Get a character from the dictionary.
	///
	/// Note that only top-level movies (those belonging to a single
	/// SWF stream) have a characters dictionary, thus our
	/// movie_def_impl. The other derived class, sprite_definition
	/// will seek for characters in it's base movie_def_impl.
	///
	virtual character_def*	get_character_def(int id) = 0;

	virtual bool get_labeled_frame(const char* label, size_t* frame_number) = 0;

	//
	// For use during creation.
	//

	/// Returns 1 based index. Ex: if 1 then 1st frame as been fully loaded
	virtual size_t	get_loading_frame() const = 0;

	virtual void	add_character(int id, character_def* ch) = 0;

	virtual void	add_font(int id, font* ch) = 0;

	virtual font*	get_font(int id) = 0;

	virtual void	add_execute_tag(execute_tag* c) = 0;

	// sprite_id was useless
	//virtual void	add_init_action(int sprite_id, execute_tag* c) = 0;
	virtual void	add_init_action(execute_tag* c) = 0;

	virtual void	add_frame_name(const char* name) = 0;

	virtual void	set_jpeg_loader(std::auto_ptr<jpeg::input> j_in) = 0;

	virtual jpeg::input*	get_jpeg_loader() = 0;

	virtual bitmap_character_def* get_bitmap_character_def(int character_id)=0;

	virtual void add_bitmap_character_def(int character_id,
			bitmap_character_def* ch) = 0;

	virtual sound_sample* get_sound_sample(int character_id) = 0;

	virtual void add_sound_sample(int character_id, sound_sample* sam) = 0;

	virtual void set_loading_sound_stream_id(int id) = 0;
	
	virtual int get_loading_sound_stream_id() = 0;


	virtual void export_resource(const tu_string& symbol,
			resource* res) = 0;

	virtual void add_import(const char* source_url, int id,
			const char* symbol_name) = 0;

	virtual void add_bitmap_info(bitmap_info* ch) = 0;

	// ...

	virtual create_bitmaps_flag	get_create_bitmaps() const = 0;
	virtual create_font_shapes_flag	get_create_font_shapes() const = 0;

	/// \brief
	/// Return the URL of the SWF stream this definition has been read
	/// from.
	virtual const std::string& get_url() const = 0;

	/// \brief
	/// Ensure that frame number 'framenum' (1-based offset)
	/// has been loaded (load on demand).
	//
	/// @return false on error (like not enough frames available).
	///
	virtual bool ensure_frame_loaded(size_t framenum) = 0;

	/// \brief
	/// Load next chunk of this movie/sprite frames if available.
	///
	virtual void load_next_frame_chunk() = 0;
};

} // namespace gnash

#endif // GNASH_MOVIE_DEFINITION_H
