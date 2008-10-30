// sprite_definition.h:  Holds immutable data for a sprite, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#ifndef GNASH_SPRITE_DEFINITION_H
#define GNASH_SPRITE_DEFINITION_H


#include <vector>
#include <map>
#include "smart_ptr.h" // GNASH_USE_GC
#include "movie_definition.h" // for inheritance
#include "log.h"
#include "rect.h"
#include "StringPredicates.h" // StringNoCaseLessThen
#include "TagLoadersTable.h"

// Forward declarations
namespace gnash {
	class SWFStream; // for read signature
	class as_function; // for registerClass/getRegisteredClass
}

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
	sprite_definition(movie_definition& m, SWFStream& in,
            const RunInfo& runInfo);

	/// \brief
	/// Create an empty sprite
	//
	/// A sprite definition consists of a series control tags.
	///
	/// @param m
	///	the Top-Level movie_definition this sprite is read
	///	from (not a sprite_definition!)
	sprite_definition(movie_definition& m);


	/// Destructor, releases playlist data
	~sprite_definition();

	/// Register a class to this definition.
	//
	/// New instances of this symbol will get the given class
	/// interface.
	///
	/// @param the_class
	///	The class constructor to associate with
	///	new instances of this character. If NULL
	///	new instances will get the MovieClip interface.
	///
	void registerClass(as_function* the_class);

	/// Get the Class registered to this definition.
	as_function* getRegisteredClass()
	{
		return registeredClass.get();
	}

	// overload from movie_definition
	virtual float	get_width_pixels() const { return 1; }

	// overload from movie_definition
	virtual float	get_height_pixels() const { return 1; }

	// overload from movie_definition
	virtual size_t	get_frame_count() const { return m_frame_count; }

	/// \brief
	/// Return total bytes of the movie from which this sprite
	/// has been read.
	///
	virtual size_t get_bytes_total() const
	{
		return m_movie_def.get_bytes_total();
	}

	/// \brief
	/// Return the number of bytes loaded from the stream of the
	/// the movie from which this sprite is being read.
	///
	virtual size_t get_bytes_loaded() const
	{
		return m_movie_def.get_bytes_loaded();
	}

	virtual float	get_frame_rate() const
	{
		return m_movie_def.get_frame_rate();
	}

	const rect& get_frame_size() const
	{
		abort();
		static const rect unused;
		return unused;
	}

	// Return number of frames loaded (of current sprite)
	virtual size_t	get_loading_frame() const { return m_loading_frame; }

	virtual int	get_version() const
	{
		return m_movie_def.get_version();
	}

	/// Overridden just for complaining  about malformed SWF
	virtual void add_font(int /*id*/, font* /*ch*/)
	{
		IF_VERBOSE_MALFORMED_SWF (
		log_swferror(_("add_font tag appears in sprite tags"));
		);
	}

	/// Delegate call to associated root movie
	virtual font* get_font(int id) const
	{
		return m_movie_def.get_font(id);
	}

	/// Delegate call to associated root movie
	virtual bitmap_character_def* get_bitmap_character_def(int id)
	{
		return m_movie_def.get_bitmap_character_def(id);
	}

	/// Overridden just for complaining  about malformed SWF
	virtual void add_bitmap_character_def(int /*id*/,
			bitmap_character_def* /*ch*/)
	{
		IF_VERBOSE_MALFORMED_SWF (
		log_swferror(_("add_bitmap_character_def appears in sprite tags"));
		);
	}

	/// Delegate call to associated root movie
	virtual sound_sample* get_sound_sample(int id)
	{
		return m_movie_def.get_sound_sample(id);
	}

	/// Overridden just for complaining  about malformed SWF
	virtual void add_sound_sample(int id, sound_sample* sam)
	{
		// DEFINESOUND tags *are* allowed in a sprite context,
		// and it is *expected* for them to be registered into
		// the main dictionary.
		m_movie_def.add_sound_sample(id,sam);
	}

	// See dox in movie_definition.h
	virtual void set_loading_sound_stream_id(int id)
	{
		_loadingSoundStream = id;
	}

	// See dox in movie_definition.h
	virtual int get_loading_sound_stream_id()
	{
		return _loadingSoundStream;
	}


	/// Delegate call to associated root movie
	virtual void export_resource(const std::string& sym,
			ExportableResource* res)
	{
		m_movie_def.export_resource(sym, res);
	}

	/// Delegate call to associated root movie
	virtual boost::intrusive_ptr<ExportableResource> get_exported_resource(
            const std::string& sym)
	{
		return m_movie_def.get_exported_resource(sym);
	}

	/// Overridden just for complaining  about malformed SWF
	virtual void importResources(boost::intrusive_ptr<movie_definition> /*source*/, Imports& /*imports*/)
	{
		IF_VERBOSE_MALFORMED_SWF (
		log_swferror(_("IMPORT tag appears in DEFINESPRITE tag"));
		);
	}

	/// \brief
	/// Get a character_def from this Sprite's root movie
	/// CharacterDictionary.
	///
	virtual character_def*	get_character_def(int id)
	{
	    return m_movie_def.get_character_def(id);
	}

	/// Delegate call to associated root movie
	virtual void add_character(int id, character_def* ch)
	{
		m_movie_def.add_character(id, ch);
	}

	// Create a (mutable) instance of our definition.  The
	// instance is created to live (temporarily) on some level on
	// the parent movie's display list.
	//
	// overloads from character_def
	virtual character* create_character_instance(
		character* parent, int id);


private:

	void read(SWFStream& in, const RunInfo& runInfo);

	/// Tags loader table.
	//
	/// TODO: make it a static member, specific to sprite_definition
	SWF::TagLoadersTable& _tag_loaders;

	/// Top-level movie definition
	/// (the definition read from SWF stream)
	movie_definition& m_movie_def;

	typedef std::map<size_t, PlayList> PlayListMap;

	/// movie control events for each frame.
	PlayListMap m_playlist;

	// stores 0-based frame #'s
	typedef std::map<std::string, size_t, StringNoCaseLessThen> NamedFrameMap;
	NamedFrameMap _namedFrames;

	size_t m_frame_count;

	// Number of frames completely parsed 
	size_t m_loading_frame;


	// See dox in movie_definition.h
	virtual void	addControlTag(ControlTag* c)
	{
		m_playlist[m_loading_frame].push_back(c);
	}

	// See dox in movie_definition.h
	virtual void add_frame_name(const std::string& name);

	// See dox in movie_definition
	bool get_labeled_frame(const std::string& label, size_t& frame_number);

	/// frame_number is 0-based
	const PlayList* getPlaylist(size_t frame_number) const
	{
		// Don't access playlist of a frame which has not been
		// completely parsed yet.
		//assert(frame_number < m_loading_frame);

		PlayListMap::const_iterator it = m_playlist.find(frame_number);
		if ( it == m_playlist.end() ) return NULL;
		else return &(it->second);
	}

	virtual const std::string& get_url() const
	{
		return m_movie_def.get_url();
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
			log_debug(_("sprite_definition: "
				"loading of frame %d requested "
				"(we are at %d/%d)"),
				framenum, m_loading_frame, m_frame_count);
			// Could this ever happen ? YES ! See tuner_7_6_0_0_pandora.swf
			return false;
		}
		return true;
	}

	const rect&	get_bound() const {
    // It is required that get_bound() is implemented in character definition
    // classes. However, it makes no sense to call it for sprite definitions.
    // get_bound() is currently only used by generic_character which normally
    // is used only shape character definitions. See character_def.h to learn
    // why it is virtual anyway.
    abort(); // should not be called
		static rect unused;
		return unused;
  }

	/// \brief
	/// The constructor to use for setting up the interface
	/// for new instances of this sprite
	//
	/// If NULL, new instances will have the default MovieClip
	/// interface.
	///
	boost::intrusive_ptr<as_function> registeredClass;

	int	_loadingSoundStream;

protected:


#ifdef GNASH_USE_GC
	/// Mark reachable resources of a sprite_definition
	//
	/// Reachable resources are:
	///	- registered class (registeredClass)
	///
	void markReachableResources() const;
#endif // GNASH_USE_GC
};


} // end of namespace gnash

#endif // GNASH_SPRITE_H
