//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

/// \page movie SWF Movies
///
/// SWF Movies definitions are created by reading an SWF stream.
/// Gnash doesn't play SWF Movie definitions, but instances.
/// So you can play the same SWF file (Movie definiton) using
/// multiple instances.
///
/// A Movie definition is defined by the gnash::movie_definition class.
/// A Movie instance is defined by the gnash::Movie class.
///
/// A Movie instance exposes the ActionScript
/// Object base interface (gnash::as_object),
/// thus it can manage gnash::as_value members.
///
/// The implementation of SWF parsing for a Movie definition
/// is found in gnash::SWFMovieDefinition::read.
///
/// Note that movie_definition is also used as a base class
/// for gnash::sprite_definition, which is a sub-movie defined in an SWF
/// file. This seems to be the only reason to have a
/// SWFMovieDefinition class, being the top-level definition of
/// a movie (the one with a CharacterDictionary in it).
///
/// Also note that gnash::Movie is a subclass of gnash::MovieClip,
/// being the instance of a gnash::sprite_definition.
///
///


#ifndef GNASH_MOVIE_DEFINITION_H
#define GNASH_MOVIE_DEFINITION_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h" // for USE_SWFTREE
#endif

#include <string>
#include <memory> // for auto_ptr
#include <vector> // for PlayList typedef
#include <set>
#include <boost/intrusive_ptr.hpp>
#include <boost/cstdint.hpp>

#include "DefinitionTag.h"
#include "log.h"

// Forward declarations
namespace gnash {
	class CachedBitmap;
	class Movie;
	class MovieClip;
	namespace SWF {
        class ControlTag;
    }
    class Font;
    class sound_sample;
    namespace image {
        class JpegInput;
    }
}

namespace gnash
{

/// Client program's interface to the definition of a movie or sprite
//
/// This is the shared constant source info, the one that cannot
/// be changed by ActionScript code.
///
/// The class derives from DefinitionTag to allow a movie
/// to be put in the CharacterDictionary. This is probably
/// unneeded for top-level movies, because they don't need
/// to be put in any CharacterDictionary... anyway the
/// current design requires both sprite_definition (a sprite)
/// and SWFMovieDefinition (a top-level movie) to derive from
/// a common class to allow tag_loaders to take a pointer
/// to the base class to act on (consider PLACEOBJECT tags...).
class movie_definition : public SWF::DefinitionTag
{
public:
	typedef std::vector<boost::intrusive_ptr<SWF::ControlTag> > PlayList;

	virtual int	get_version() const = 0;

    /// Frame width in pixels.
    //
    /// The frame size is in twips and may be rounded up.
	virtual size_t get_width_pixels() const = 0;
    
    /// Frame height in pixels.
    //
    /// The frame size is in twips and may be rounded up.
	virtual size_t get_height_pixels() const = 0;

	virtual size_t get_frame_count() const = 0;
	virtual float get_frame_rate() const = 0;

	/// Return size of frame, in TWIPS
	virtual const SWFRect& get_frame_size() const = 0;

	virtual size_t get_bytes_loaded() const = 0;

	/// Get total number of bytes in (uncompressed for SWF) input stream
	//
	/// Note that this is different from actual file size if
	/// this is a *compressed* SWF.
	/// For other kind of movie definitions (Bitmaps, for example),
	/// the returned value should likely match the file size.
	///
	virtual size_t get_bytes_total() const = 0;

	/// Create a movie instance from a def.
	//
	/// Not all movie definitions allow creation of
	/// Movie. In particular, sprite_definition
	/// can only create MovieClip, so will return NULL
	/// from this method.
	///
	/// The default implementation returns NULL.
	///
	/// Override this method for any definition that is
	/// able to be instanciated as a Movie.
	/// SWFMovieDefinition is one such example, future examples
	/// should include jpeg_movie_def and similar..
	///
	virtual Movie* createMovie(Global_as& /*gl*/, DisplayObject* /*parent*/=0)
	{
		return NULL;
	}

    virtual void incrementLoadedFrames() {}

	/// Return the list of execute tags for given frame number
	//
	/// @param frame_number
	///	 Frame number, 0-based (ie: first frame is 0)
	///
	/// @return NULL if no execute tags are defined for the given frame number
	///	    (the default implementation) or a pointer to the vector of them
    ///     (PlayList)
	///
	virtual const PlayList* getPlaylist(size_t /*frame_number*/) const
	{
		return 0;
	}


	typedef std::pair<int, std::string> ImportSpec;
	typedef std::vector< ImportSpec > Imports;

	/// Import resources
	//
	/// @param source
	///	Movie containing the resources being imported
	///
	/// @param imports
	///	Resources to import, each with the id to use in our dictionary
	///
	virtual void importResources(
            boost::intrusive_ptr<movie_definition> /*source*/, 
            const Imports& /*imports*/)
	{
		IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("IMPORT tag appears outside SWF definition"));
		);
	}


	/// \brief
	/// Get a DisplayObject from the dictionary.
	//
	/// Note that only top-level movies (those belonging to a single
	/// SWF stream) have a DisplayObjects dictionary, thus our
	/// SWFMovieDefinition. The other derived class, sprite_definition
	/// will seek for DisplayObjects in it's base SWFMovieDefinition.
	///
	/// @return NULL if no DisplayObject with the given ID is found
	///         (this is the default)
	virtual DefinitionTag* getDefinitionTag(boost::uint16_t /*id*/) const
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
	virtual bool get_labeled_frame(const std::string& /*label*/,
            size_t& /*frame_number*/) const
	{
		return false;
	}

	/// Returns 1 based index. Ex: if 1 then 1st frame as been fully loaded
	virtual size_t	get_loading_frame() const = 0;

	/// Add a DefinitionTag with given ID to the CharactersDictionary.
	//
    /// @param id   The id of the tag. All tags store their own id, but
    ///             imported characters receive a new id in the importing
    ///             movie.
    //
	/// This method is here to be called by DEFINE tags loaders.
	/// The default implementation does nothing.
	virtual void addDisplayObject(boost::uint16_t /*id*/, DefinitionTag* /*c*/)
	{
	}

	/// Add a font DisplayObject with given ID to the CharacterDictionary.
	//
	/// This method is here to be called by DEFINEFONT tags loaders.
	/// The default implementation does nothing.
	///
	virtual void add_font(int /*id*/, boost::intrusive_ptr<Font> /*ch*/)
	{
	}

	/// Return the font with given DisplayObject id
	//
	/// @returns NULL if the given id doesn't correspond
	///          to any registered font (default).
	///
	/// @see add_font
	///
	virtual Font* get_font(int /*id*/) const
	{
		return NULL;
	}

	/// Find a font from the movie (not shared) lib
	virtual Font* get_font(const std::string& /*name*/,
            bool /*bold*/, bool /*italic*/) const
	{
		return 0;
	}

	/// Add an ControlTag to this movie_definition's playlist
	//
	/// The default implementation is a no-op.
	///
	/// @param tag
	/// 	The tag to add in the list of executable tags for
	/// 	the frame currently being loaded. Ownership is transferred
	/// 	to the SWFMovieDefinition.
	virtual void addControlTag(boost::intrusive_ptr<SWF::ControlTag> /*c*/)
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
	virtual void set_jpeg_loader(std::auto_ptr<image::JpegInput> /*j_in*/)
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
	virtual image::JpegInput* get_jpeg_loader() const
	{
		return NULL;
	}

	/// \brief
	/// Get a bitmap from the bitmap dictionary.
	//
	/// Note that only top-level movies (those belonging to a single
	/// SWF stream) have a bitmap dictionary, thus our
	/// SWFMovieDefinition. The other derived class, sprite_definition
	/// will seek for DisplayObjects in its base SWFMovieDefinition.
	///
	/// @return 0 if no DisplayObject with the given ID is found, or
	///	    if the corresponding DisplayObject is not a bitmap.
	///
	/// The default implementation returns 0.
	///
	virtual CachedBitmap* getBitmap(int /*DisplayObject_id*/) const
	{
		return 0;
	}

	/// \brief
	/// Add a bitmap DisplayObject in the dictionary, with the specified
	/// DisplayObject id.
	//
	/// The default implementation is a no-op (deletes the image data).
	///
	virtual void addBitmap(int /*id*/, boost::intrusive_ptr<CachedBitmap> /*im*/)
	{
	}

	/// Get the sound sample with given ID.
	//
	/// @return NULL if the given DisplayObject ID isn't found in the
	///         dictionary or it is not a sound sample.
	///
	/// The default implementation always returns NULL
	///
	virtual sound_sample* get_sound_sample(int /*DisplayObject_id*/) const
	{
		return NULL;
	}

	/// \brief
	/// Add a sound sample DisplayObject in the dictionary, with the specified
	/// DisplayObject id.
	//
	/// The default implementation is a no-op
	///
	virtual void add_sound_sample(int /*DisplayObject_id*/, sound_sample* /*sam*/)
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
	virtual int get_loading_sound_stream_id() const
	{
		return -1;
	}

    /// Register a symbol to refer to a character id
    //
    /// The default implementation does nothing.
    virtual void registerExport(const std::string&, boost::uint16_t) {}
    
    /// Get the id that corresponds to a symbol.
    //
    /// @return         The id corresponding to the passed symbol. The default
    ///                 implementation returns 0, as it has no export table.
    virtual boost::uint16_t exportID(const std::string& /*symbol*/) const {
        return 0;
    }

    /// Set whether the SWFMovie should use AVM2 or AVM1.
    //
    /// This is only sensible for SWFMovieDefinitions, so is a no-op here.
    virtual void setAS3() {
    }

    /// True if the SWFMovie should use AVM2.
    //
    /// This is only sensible for SWFMovieDefinitions, and shouldn't be
    /// here once this inheritance mess is cleaned up.
    virtual bool isAS3() const {
        return false;
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
	virtual bool ensure_frame_loaded(size_t /*framenum*/) const {
		return true;
	}

#ifdef USE_SWFTREE

	// These methods attach the contents of the METADATA tag
	// to a movie_definition. They are not used by the player
	// at all, but are stored for display in Movie Properties.
	// To save memory and parsing time, this won't happen
	// when the swf tree view is disabled.
	virtual void storeDescriptiveMetadata(const std::string& /*data*/)
	{
	}

	virtual const std::string& getDescriptiveMetadata() const
	{
	    static const std::string s;
	    return s;
	}

#endif
protected:

    movie_definition(boost::uint16_t id = 0)
        :
        DefinitionTag(id)
    {}

    virtual ~movie_definition() {}
};

} // namespace gnash

#endif // GNASH_MOVIE_DEFINITION_H
