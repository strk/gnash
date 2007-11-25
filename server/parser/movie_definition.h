// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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


/// \page movie SWF Movies
///
/// SWF Movies definitions are created by reading an SWF stream.
/// Gnash doesn't play SWF Movie definitions, but instances.
/// So you can play the same SWF file (Movie definiton) using
/// multiple instances.
///
/// A Movie definition is defined by the gnash::movie_definition class.
/// A Movie instance is defined by the gnash::movie_instance class.
/// 
/// A Movie instance exposes the ActionScript
/// Object base interface (gnash::as_object),
/// thus it can manage gnash::as_value members.
///
/// The implementation of SWF parsing for a Movie definition
/// is found in gnash::movie_def_impl::read.
///
/// Note that movie_definition is also used as a base class
/// for gnash::sprite_definition, which is a sub-movie defined in an SWF
/// file. This seems to be the only reason to have a
/// movie_def_impl class, being the top-level definition of
/// a movie (the one with a CharacterDictionary in it).
///
/// Also note that gnash::movie_instance is a subclass of gnash::sprite_instance,
/// being the instance of a gnash::sprite_definition.
///
///


#ifndef GNASH_MOVIE_DEFINITION_H
#define GNASH_MOVIE_DEFINITION_H

#include "character_def.h" // for inheritance
#include "container.h"
#include "fontlib.h"
#include "jpeg.h"
#include "tu_file.h"
#include "gnash.h" // For create_bitmaps_flag and create_font_shapes_flag.
                   // Would be much better to have those two enums defined
		   // inside movie_definition instead, btw...

#include <string>
#include <memory> // for auto_ptr
#include <set> 

// Forward declarations
namespace gnash {
	class bitmap_character_def;
	class bitmap_info;
	class movie_instance;
	class sprite_instance;
	class ControlTag;
}

namespace gnash
{

/// Client program's interface to the definition of a movie or sprite
//
/// This is the shared constant source info, the one that cannot
/// be changed by ActionScript code.
///
/// The class derives from character_def to allow a movie
/// to be put in the CharacterDictionary. This is probably
/// unneeded for top-level movies, because they don't need
/// to be put in any CharacterDictionary... anyway the
/// current design requires both sprite_definition (a sprite)
/// and movie_def_impl (a top-level movie) to derive from
/// a common class to allow tag_loaders to take a pointer
/// to the base class to act on (consider PLACEOBJECT tags...).
///
/// This design is uncomfortable when it comes to programmatically
/// created characters, in that they do NOT have any *fixed* definition.
/// A possible workaround to this would be not *requiring* character
/// instances to have an associated character_def. I'll work on this
/// --strk 2006-12-05.
///
class movie_definition : public character_def
{
public:
	typedef std::vector<ControlTag*> PlayList;

	virtual int	get_version() const = 0;
	virtual float	get_width_pixels() const = 0;
	virtual float	get_height_pixels() const = 0;
	virtual size_t	get_frame_count() const = 0;
	virtual float	get_frame_rate() const = 0;

	/// Return size of frame, in TWIPS 
	virtual const rect& get_frame_size() const = 0;

	virtual size_t get_bytes_loaded() const = 0;

	/// Get total number of bytes in (uncompressed for SWF) input stream
	//
	/// Note that this is different from actual file size if
	/// this is a *compressed* SWF.
	/// For other kind of movie definitions (Bitmaps, for example),
	/// the returned value should likely match the file size.
	///
	virtual size_t get_bytes_total() const = 0;
	
	/// Create a playable sprite_instance from a def.
	//
	/// This calls add_ref() on the movie_interface internally.
	/// Call drop_ref() on the movie_interface when you're done with it.
	/// Or use boost::intrusive_ptr<T> from base/smart_ptr.h if you want.
	///
	virtual sprite_instance* create_instance() = 0;

	/// Create a movie instance from a def.
	//
	/// Not all movie definitions allow creation of
	/// movie_instance. In particular, sprite_definition
	/// can only create sprite_instance (see create_instance)
	///
	/// The default implementation returns NULL.
	///
	/// Override this method for any definition that is
	/// able to be instanciated as a movie_instance.
	/// movie_def_impl is one such example, future examples
	/// should include jpeg_movie_def and similar..
	///
	virtual movie_instance* create_movie_instance() {
		return NULL;
	}
	
	virtual void	output_cached_data(tu_file* /*out*/, const cache_options& /*options*/) {}
	virtual void	input_cached_data(tu_file* /*in*/) {}
	
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
	virtual void	generate_font_bitmaps() {}
	
	//
	// (optional) API to support gnash::create_movie_no_recurse().
	//
	
	/// \brief
	/// Call visit_imported_movies() to retrieve a list of
	/// names of movies imported into this movie.
	//
	/// visitor.visit() will be called back with the name
	/// of each imported movie.
	///
	class import_visitor
	{
	public:
	    virtual ~import_visitor() {}
	    virtual void visit(const std::string& imported_movie_filename) = 0;
	};
	virtual void visit_imported_movies(import_visitor& /*visitor*/) {}
	
	/// Call this to resolve an import of the given movie.
	//
	/// Replaces the dummy placeholder with the real
	/// movie_definition* given.
	///
	/// The default implementation is a no-op.
	///
	/// @see add_import
	///
	virtual void resolve_import(const std::string& /*name*/,
			movie_definition* /*def*/) {}
	
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
	//
	virtual int	get_bitmap_info_count() const { return 0; }

	virtual bitmap_info*	get_bitmap_info(int /*i*/) const { return NULL; }

	// From movie_definition_sub

	/// Return the list of execute tags for given frame number
	//
	/// @param frame_number
	///	 Frame number, 0-based (ie: first frame is 0)
	///
	/// @return NULL if no execute tags are defined for the given frame number
	///	    (the default implementation) or a pointer to the vector of them (PlayList)
	///
	virtual const PlayList* getPlaylist(size_t /*frame_number*/) const
	{
		return 0;
	}

	/// Get the named exported resource, if we expose it.
	//
	/// @param symbol
	///	The symbol name. Matching should be case-insensitive for all SWF versions.
	///
	/// @return NULL if the label doesn't correspond to an exported
	///         resource. This is the default behaviour.
	///
	virtual boost::intrusive_ptr<resource>	get_exported_resource(const std::string& /*symbol*/)
	{
		return NULL;
	}


	/// \brief
	/// Get a character from the dictionary.
	//
	/// Note that only top-level movies (those belonging to a single
	/// SWF stream) have a characters dictionary, thus our
	/// movie_def_impl. The other derived class, sprite_definition
	/// will seek for characters in it's base movie_def_impl.
	///
	/// @return NULL if no character with the given ID is found
	///         (this is the default)
	///
	virtual character_def*	get_character_def(int /*id*/)
	{
		return NULL;
	}

	/// Get 0-based index of the frame with given label.
	//
	///
	/// The default implementation is to always return false, as
	/// if NO frame with given label was found.
	///
	/// @param label
	/// 	Label of the frame we're looking for.
	///
	/// @param frame_number
	/// 	Where to write frame number to (if a match is found).
	///	A 0-based index will be written there.
	///
	/// @return true if a frame with that label was found, false otherwise
	///
	virtual bool get_labeled_frame(const std::string& /*label*/, size_t& /*frame_number*/)
	{
		return false;
	}

	//
	// For use during creation.
	//

	/// Returns 1 based index. Ex: if 1 then 1st frame as been fully loaded
	virtual size_t	get_loading_frame() const = 0;

	/// Add a character with given ID to the CharactersDictionary.
	//
	/// This method is here to be called by DEFINE tags loaders.
	/// The default implementation does nothing.
	///
	virtual void add_character(int /*id*/, character_def* /*ch*/)
	{
	}

	/// Add a font character with given ID to the CharacterDictionary.
	//
	/// This method is here to be called by DEFINEFONT tags loaders.
	/// The default implementation does nothing.
	///
	virtual void add_font(int /*id*/, font* /*ch*/)
	{
	}

	/// Return the font with given character id
	//
	/// @returns NULL if the given id doesn't correspond
	///          to any registered font (default).
	///
	/// @see add_font
	///
	virtual font* get_font(int /*id*/) const
	{
		return NULL;
	}

	/// Add an ControlTag to this movie_definition's playlist
	//
	/// The default implementation is a no-op.
	///
	/// @param tag
	/// 	The tag to add in the list of executable tags for
	/// 	the frame currently being loaded. Ownership is transferred
	/// 	to the movie_def_impl.
	///
	/// TODO: take an auto_ptr.
	/// NOTE: the default implementation just let the ControlTag leak.
	///
	virtual void	addControlTag(ControlTag* /*c*/)
	{
	}

	/// \brief
	/// Add an init action to the frame currently being loaded
	/// and the specified character id
	//
	/// The default implementation is a no-op.
	///
	virtual void	add_init_action(ControlTag* /*c*/, int /*cid*/)
	{
	}

	/// Labels the frame currently being loaded with the given name. 
	//
	/// A copy of the name string is made and kept in this object.
    /// In case of multiple frames with the same name, the last added
    /// will be the one referenced by that name.
	///
	/// The default implementation is a no-op.
	///
	virtual void add_frame_name(const std::string& /*name*/)
	{
	}

	/// This method should probably not be there but in some higher-level
	/// class, like a Parser class..
	///
	/// The default implementation is a no-op. Actually, an implicit op
	/// *is* performed, and it is deleting the jpeg::input instance since
	/// it is passed in an auto_ptr...
	///
	virtual void set_jpeg_loader(std::auto_ptr<jpeg::input> /*j_in*/)
	{
	}

	/// \brief
	/// Get the jpeg input loader, to load a DefineBits image
	/// (one without table info).
	///
	/// This method should probably not be there but in some higher-level
	/// class, like a Parser class..
	///
	/// The default implementation returns NULL
	///
	/// NOTE: ownership of the returned object is NOT transferred
	///
	virtual jpeg::input*	get_jpeg_loader()
	{
		return NULL;
	}

	/// \brief
	/// Get a bitmap character from the dictionary.
	//
	/// Note that only top-level movies (those belonging to a single
	/// SWF stream) have a characters dictionary, thus our
	/// movie_def_impl. The other derived class, sprite_definition
	/// will seek for characters in it's base movie_def_impl.
	///
	/// @return NULL if no character with the given ID is found, or
	///	    if the corresponding character is not a bitmap.
	///
	/// The default implementation returns NULL.
	///
	virtual bitmap_character_def* get_bitmap_character_def(int /*character_id*/)
	{
		return NULL;
	}

	/// \brief
	/// Add a bitmap character in the dictionary, with the specified
	/// character id.
	//
	/// The default implementation is a no-op
	///
	virtual void add_bitmap_character_def(int /*character_id*/,
			bitmap_character_def* /*ch*/)
	{
	}

	/// Get the sound sample with given ID.
	//
	/// @return NULL if the given character ID isn't found in the
	///         dictionary or it is not a sound sample.
	///
	/// The default implementation always returns NULL
	///
	virtual sound_sample* get_sound_sample(int /*character_id*/)
	{
		return NULL;
	}

	/// \brief
	/// Add a sound sample character in the dictionary, with the specified
	/// character id.
	//
	/// The default implementation is a no-op
	///
	virtual void add_sound_sample(int /*character_id*/, sound_sample* /*sam*/)
	{
	}

	/// Set the currently being loaded sound stream
	//
	/// The default implementation is a no-op
	///
	virtual void set_loading_sound_stream_id(int /*id*/)
	{
	}
	
	/// Get the currently being loaded sound stream, if any
	//
	/// @see set_loading_sound_stream_id
	///
	/// The default implementation returns -1
	///
	/// @returns -1 if no sound stream is being currently loading
	///
	virtual int get_loading_sound_stream_id()
	{
		return -1;
	}

	/// Mark the given resource as "exported" with the given linkage name.
	//
    /// Note that any previously exported resource with the same linkage 
    /// name will become unreachable (export override).
    ///
	/// @see get_exported_resource
	///
	/// The default implementation is a no-op
	///
	virtual void export_resource(const std::string& /*symbol*/,
			resource* /*res*/)
	{
	}

	/// \brief
	/// Adds an entry to a table of resources that need to
	/// be imported from other movies. 
	//
	/// Client code must call resolve_import() later, when the
	/// source movie has been loaded, so that the actual resource
	/// can be used.
	///
	/// This mechanism (add_import/resolve_import) is only used
	/// by the IMPORT tag loader if s_no_recurse_while_loading is
	/// true, which currently NEVER happens using the standard
	/// executables.
	///
	/// The default implementation is a no-op.
	///
	virtual void add_import(const std::string& /*source_url*/,
			int /*id*/, const std::string& /*symbol_name*/)
	{
	}

	/// \brief
	/// All bitmap_info's used by this movie should be
	/// registered with this API.
	//
	/// This was likely used for 'caching' renderer-specific
	/// versions of bitmaps. I'm not sure it is really needed
	/// currently (caching on disk is broken).
	///
	/// The default implementation is a no-op
	///
	virtual void add_bitmap_info(bitmap_info* /*ch*/)
	{
	}

	// ...

	// This interface should be nice not to have in movie_definition
	// The default returns DO_LOAD_BITMAPS.
	virtual create_bitmaps_flag	get_create_bitmaps() const
	{
		return DO_LOAD_BITMAPS;
	}

	// This interface should be nice not to have in movie_definition
	// The default returns DO_LOAD_FONT_SHAPES.
	virtual create_font_shapes_flag	get_create_font_shapes() const
	{
		return DO_LOAD_FONT_SHAPES;
	}

	/// \brief
	/// Return the URL of the SWF stream this definition has been read
	/// from.
	virtual const std::string& get_url() const = 0;

	// Start the loader thread. By default no loader thread is engaged
	// so this function is a no-op.
	virtual bool completeLoad() {
		return true;
	}

	/// \brief
	/// Ensure that frame number 'framenum' (1-based offset)
	/// has been loaded (load on demand).
	//
	/// @param framenum
	///	1-based frame index that we want to be fully loaded
	///	before this function returns
	///
	/// @return false on error (like not enough frames available).
	///
	/// The default implementation is to always return true.
	///
	virtual bool ensure_frame_loaded(size_t /*framenum*/) {
		return true;
	}

	/// \brief
	/// Load next chunk of this movie/sprite frames if available.
	//
	/// The default implementation is a no-op
	///
	virtual void load_next_frame_chunk() 
	{
	}

	/// Add a depth to the timeline depth set in current frame 
	//
	/// The default implementations is a no-op
	///
	/// It is overridden by timeline-equipped definitions:
	///	- sprite_definition
	///	- movie_def_impl
	///
	/// @param depth
	///	Depth of an instance placed by PlaceObject* tag.
	///	Assumed to be in the static zone (an assertion would fail otherwise).
	///
	virtual void addTimelineDepth(int /*depth*/) {}

	/// Remove a depth from the timeline depth set in current frame 
	//
	/// The default implementations is a no-op
	///
	/// It is overridden by timeline-equipped definitions:
	///	- sprite_definition
	///	- movie_def_impl
	///
	/// @param depth
	///	Depth of an instance placed by PlaceObject* tag.
	///	Assumed to be in the static zone (an assertion would fail otherwise).
	///
	virtual void removeTimelineDepth(int /*depth*/) {}

	/// \brief
	/// Get the set of timeline depths supposed to contain
	/// an instance in the given frame (0-based)
	//
	/// The default implementations is a no-op
	///
	/// PRECONDITIONS:
	///	- Target frame was completely parsed.
	///
	/// @param frameno
	///	0-based frame number we want to inspect.
	///	Assumed to be in the static zone.
	///
	/// @param depths
	///	A vector to copy all frame depths to.
	///
	virtual void getTimelineDepths(size_t /*frameno*/, std::vector<int>& /*depths*/) 
	{}
    
};

} // namespace gnash

#endif // GNASH_MOVIE_DEFINITION_H
