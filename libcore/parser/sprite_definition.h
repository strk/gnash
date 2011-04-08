// sprite_definition.h:  Holds immutable data for a sprite, for Gnash.
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
//

#ifndef GNASH_SPRITE_DEFINITION_H
#define GNASH_SPRITE_DEFINITION_H

#include <boost/intrusive_ptr.hpp>
#include <boost/cstdint.hpp>
#include <string>
#include <map>
#include "movie_definition.h" // for inheritance
#include "log.h"
#include "SWFRect.h"
#include "StringPredicates.h" // StringNoCaseLessThan

// Forward declarations
namespace gnash {
	class SWFStream;
	class as_function;
    class RunResources;
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
            const RunResources& runResources, boost::uint16_t id);

	/// Destructor, releases playlist data
	~sprite_definition();

	/// Register a class to this definition.
	//
	/// New instances of this symbol will get the given class
	/// interface.
	///
	/// @param the_class
	///	The class constructor to associate with
	///	new instances of this DisplayObject. If NULL
	///	new instances will get the MovieClip interface.
	///
	void registerClass(as_function* the_class);

	/// Get the Class registered to this definition.
	as_function* getRegisteredClass() const {
		return registeredClass;
	}

    virtual void incrementLoadedFrames() {
        ++m_loading_frame;
    }

	// overload from movie_definition
	virtual size_t get_width_pixels() const { return 1; }

	// overload from movie_definition
	virtual size_t get_height_pixels() const { return 1; }

	// overload from movie_definition
	virtual size_t get_frame_count() const { return m_frame_count; }

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

	const SWFRect& get_frame_size() const
	{
		abort();
		static const SWFRect unused;
		return unused;
	}

	// Return number of frames loaded (of current sprite)
	virtual size_t	get_loading_frame() const { return m_loading_frame; }

	virtual int	get_version() const
	{
		return m_movie_def.get_version();
	}

	/// Overridden just for complaining  about malformed SWF
	virtual void add_font(int /*id*/, boost::intrusive_ptr<Font> /*ch*/)
	{
		IF_VERBOSE_MALFORMED_SWF (
		log_swferror(_("add_font tag appears in sprite tags"));
		);
	}

	/// Delegate call to associated root movie
	virtual Font* get_font(int id) const
	{
		return m_movie_def.get_font(id);
	}

	/// Delegate call to associated root movie
	virtual CachedBitmap* getBitmap(int id) const
	{
		return m_movie_def.getBitmap(id);
	}

	/// Overridden just for complaining  about malformed SWF
	virtual void addBitmap(int /*id*/, boost::intrusive_ptr<CachedBitmap> /*im*/)
	{
		IF_VERBOSE_MALFORMED_SWF (
		log_swferror(_("add_bitmap_SWF::DefinitionTag appears in sprite tags"));
		);
	}

	/// Delegate call to associated root movie
	virtual sound_sample* get_sound_sample(int id) const
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
	virtual int get_loading_sound_stream_id() const
	{
		return _loadingSoundStream;
	}

    virtual boost::uint16_t exportID(const std::string& symbol) const {
        return m_movie_def.exportID(symbol);
    }
    
    virtual void registerExport(const std::string& s, boost::uint16_t id) {
        m_movie_def.registerExport(s, id);
    }

	/// \brief
	/// Get a SWF::DefinitionTag from this Sprite's root movie
	/// CharacterDictionary.
	///
	virtual SWF::DefinitionTag*	getDefinitionTag(boost::uint16_t id) const
	{
	    return m_movie_def.getDefinitionTag(id);
	}

	/// Delegate call to associated root movie
	virtual void addDisplayObject(boost::uint16_t id, SWF::DefinitionTag* c)
	{
		m_movie_def.addDisplayObject(id, c);
	}

	// Create a (mutable) instance of our definition.  The
	// instance is created to live (temporarily) on some level on
	// the parent movie's display list.
	//
	// overloads from SWF::DefinitionTag
	virtual DisplayObject* createDisplayObject(Global_as& gl,
            DisplayObject* parent) const;

	// See dox in movie_definition.h
	virtual void addControlTag(boost::intrusive_ptr<SWF::ControlTag> c)
	{
		m_playlist[m_loading_frame].push_back(c);
	}

private:

	void read(SWFStream& in, const RunResources& runResources);

	/// Top-level movie definition
	/// (the definition read from SWF stream)
	movie_definition& m_movie_def;

	typedef std::map<size_t, PlayList> PlayListMap;

	/// movie control events for each frame.
	PlayListMap m_playlist;

	// stores 0-based frame #'s
	typedef std::map<std::string, size_t, StringNoCaseLessThan> NamedFrameMap;
	NamedFrameMap _namedFrames;

	size_t m_frame_count;

	// Number of frames completely parsed 
	size_t m_loading_frame;

	// See dox in movie_definition.h
	virtual void add_frame_name(const std::string& name);

	// See dox in movie_definition
	bool get_labeled_frame(const std::string& label, size_t& frame_number)
        const;

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
	virtual bool ensure_frame_loaded(size_t framenum) const
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

	/// The constructor to use for setting up the interface
	/// for new instances of this sprite
	//
    /// TODO: this really shouldn't be stored in an immutable definition.
	as_function* registeredClass;

	int	_loadingSoundStream;

protected:

	/// Mark reachable resources of a sprite_definition
	//
	/// Reachable resources are:
	///	- registered class (registeredClass)
	///
	void markReachableResources() const;
};


} // end of namespace gnash

#endif 
