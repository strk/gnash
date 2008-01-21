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

#ifndef GNASH_MOVIE_DEF_IMPL_H
#define GNASH_MOVIE_DEF_IMPL_H

#include "smart_ptr.h"
#include "fontlib.h"
#include "font.h"
#include "jpeg.h"
#include "tu_file.h"
#include "movie_definition.h" // for inheritance
#include "character_def.h" // for boost::intrusive_ptr visibility of dtor
#include "bitmap_character_def.h" // for boost::intrusive_ptr visibility of dtor
#include "resource.h" // for boost::intrusive_ptr visibility of dtor
#include "stream.h" // for get_bytes_loaded
#include "Timeline.h" // for composition 
#include "StringPredicates.h" // for case-insensitive string comparision (ExportMap)

#include <map> // for CharacterDictionary
#include <string>
#include <memory> // for auto_ptr
#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>

//
// Forward declarations
namespace gnash {
	class import_info;
	class movie_def_impl;
	class movie_root;
	class sprite_instance;
	class movie_instance;
	namespace SWF {
		class TagLoadersTable;
	}
}

namespace gnash
{

//
// Helper for movie_def_impl
//

class import_info
{
private:
    friend class movie_def_impl;

    std::string	m_source_url;
    int	        m_character_id;
    std::string	m_symbol;

    import_info()
	:
	m_character_id(-1)
	{
	}

    import_info(const std::string& source, int id, const std::string& symbol)
	:
	m_source_url(source),
	m_character_id(id),
	m_symbol(symbol)
	{
	}
};

/// \brief
/// movie_def_impl helper class handling start and execution of
/// an SWF loading thread
///
class MovieLoader
{
public:

	MovieLoader(movie_def_impl& md);

	~MovieLoader();

	/// Start loading thread.
	//
	/// The associated movie_def_impl instance
	/// is expected to have already read the SWF
	/// header and applied a zlib adapter if needed.
	///
	bool start();

	/// Return true if the MovieLoader thread was started
	bool started() const;

	/// Return true if called from the MovieLoader thread.
	bool isSelfThread() const;

private:

	movie_def_impl& _movie_def;

	mutable boost::mutex _mutex;
	std::auto_ptr<boost::thread> _thread;

	/// Entry point for the actual thread
	static void execute(movie_def_impl* md);
};

/// The Characters dictionary associated with each SWF file.
//
/// This is a set of Characters defined by define tags and
/// getting assigned a unique ID. 
///
class CharacterDictionary
{
public:

	/// The container used by this dictionary
	//
	/// It contains pairs of 'int' and 'boost::intrusive_ptr<character_def>'
	///
	typedef std::map< int, boost::intrusive_ptr<character_def> > container;

	typedef container::iterator iterator;

	typedef container::const_iterator const_iterator;

	/// Get the Character with the given id
	//
	/// returns a NULL if the id is unknown.
	///
	boost::intrusive_ptr<character_def> get_character(int id);

	/// Add a Character assigning it the given id
	//
	/// replaces any existing character with the same id
	///
	void add_character(int id, boost::intrusive_ptr<character_def> c);

	/// Return an iterator to the first dictionary element
	iterator begin() { return _map.begin(); }

	/// Return a const_iterator to the first dictionary element
	const_iterator begin() const { return _map.begin(); }

	/// Return an iterator to one-past last dictionary element
	iterator end() { return _map.end(); }

	/// Return a const_iterator to one-past last dictionary element
	const_iterator end() const { return _map.end(); }

	/// Dump content of the dictionary (debugging only)
	void dump_chars(void) const;

#ifdef GNASH_USE_GC
	/// Mark all dictionary items to be reachable (for GC)
	void markReachableResources() const
	{
		for(const_iterator i=_map.begin(), e=_map.end(); i!=e; ++i)
		{
			i->second->setReachable();
		}
	}
#endif // GNASH_USE_GC

private:

	container _map;

};


/// Immutable definition of a movie's contents.
//
/// It cannot be played directly, and does not hold
/// current state; for that you need to call create_movie_instance()
/// to get a movie instance 
///
class movie_def_impl : public movie_definition
{
private:
	/// Characters Dictionary
	CharacterDictionary	_dictionary;

	/// Mutex protecting _dictionary
	mutable boost::mutex _dictionaryMutex;

	/// Tags loader table
	SWF::TagLoadersTable& _tag_loaders;

	typedef std::map<int, boost::intrusive_ptr<font> > FontMap;
	FontMap m_fonts;

	typedef std::map<int, boost::intrusive_ptr<bitmap_character_def> > BitmapMap;
	BitmapMap m_bitmap_characters;

	typedef std::map<int, boost::intrusive_ptr<sound_sample> > SoundSampleMap;
	SoundSampleMap m_sound_samples;

	typedef std::map<size_t, PlayList> PlayListMap;

	/// Movie control events for each frame.
	PlayListMap m_playlist;

	/// 0-based frame #'s
	typedef std::map<std::string, size_t> NamedFrameMap;
	NamedFrameMap _namedFrames;

	// Mutex protecting access to _namedFrames
	mutable boost::mutex _namedFramesMutex;

	typedef std::map<std::string, boost::intrusive_ptr<resource>, StringNoCaseLessThen > ExportMap;
	ExportMap _exportedResources;

	// Mutex protecting access to _exportedResources
	mutable boost::mutex _exportedResourcesMutex;

	/// Items we import.
	std::vector<import_info> m_imports;

	/// Movies we import from; hold a ref on these,
	/// to keep them alive
	typedef std::vector<boost::intrusive_ptr<movie_definition> > ImportVect;
	ImportVect m_import_source_movies;

	/// Bitmaps used in this movie; collected in one place to make
	/// it possible for the host to manage them as textures.
	typedef std::vector<boost::intrusive_ptr<bitmap_info> >	BitmapVect;
	BitmapVect m_bitmap_list;

	rect	m_frame_size;
	float	m_frame_rate;
	size_t	m_frame_count;
	int	m_version;

	/// Number of fully loaded frames
	size_t	_frames_loaded;

	/// A mutex protecting access to _frames_loaded
	//
	/// This is needed because the loader thread will
	/// increment this number, while the virtual machine
	/// thread will read it.
	///
	mutable boost::mutex _frames_loaded_mutex;

	/// A semaphore to signal load of a specific frame
	boost::condition _frame_reached_condition;

	/// Set this to trigger signaling of loaded frame
	//
	/// Make sure you _frames_loaded_mutex is locked
	/// when accessing this member !
	///
	size_t _waiting_for_frame;

	/// Number bytes loaded / parsed
	unsigned long _bytes_loaded;

	/// A mutex protecting access to _bytes_loaded
	//
	/// This is needed because the loader thread will
	/// increment this number, while the virtual machine
	/// thread will read it.
	///
	mutable boost::mutex _bytes_loaded_mutex;



	int	m_loading_sound_stream;
	boost::uint32_t	m_file_length;

	std::auto_ptr<jpeg::input> m_jpeg_in;

	std::string _url;

	std::auto_ptr<stream> _str;

	std::auto_ptr<tu_file> _in;

	/// swf end position (as read from header)
	unsigned int _swf_end_pos;

	/// asyncronous SWF loader and parser
	MovieLoader _loader;

	/// \brief
	/// Increment loaded frames count, signaling frame reached condition if
	/// any thread is waiting for that. See ensure_frame_loaded().
	///
	/// NOTE: this method locks _frames_loaded_mutex
	///
	/// @return the new value of _frames_loaded
	///
	size_t incrementLoadedFrames();

	/// Set number of bytes loaded from input stream
	//
	/// NOTE: this method locks _bytes_loaded_mutex
	///
	void setBytesLoaded(unsigned long bytes)
	{
		boost::mutex::scoped_lock lock(_bytes_loaded_mutex);
		_bytes_loaded=bytes;
	}

	/// Timeline depth info
	//
	/// TODO: move to base class ?
	///
	Timeline _timeline;

	/// A flag set to true when load cancelation is requested
	bool _loadingCanceled;

	/// Debug helper; returns true if the given
	/// character_id is listed in the import table.
	bool in_import_table(int character_id) const;

public:

	movie_def_impl();

	~movie_def_impl();

	// ...
	size_t get_frame_count() const { return m_frame_count; }
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

	/// Get the number of fully loaded frames
	//
	/// The number returned is also the index
	/// of the frame currently being loaded/parsed,
	/// except when parsing finishes, in which case
	/// it an index to on-past-last frame.
	///
	/// NOTE: this method locks _frames_loaded_mutex
	///
	virtual size_t	get_loading_frame() const;

	/// Get number of bytes loaded from input stream
	//
	/// NOTE: this method locks _bytes_loaded_mutex
	///
	size_t	get_bytes_loaded() const
	{
		boost::mutex::scoped_lock lock(_bytes_loaded_mutex);
		return _bytes_loaded;
	}

	/// Get total number of bytes as parsed from the SWF header
	size_t	get_bytes_total() const {
		return m_file_length;
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
		return m_bitmap_list[i].get();
	}

	// See docs in movie_definition.h
	virtual void export_resource(const std::string& symbol,
			resource* res);

	/// Get the named exported resource, if we expose it.
	//
	/// @return NULL if the label doesn't correspond to an exported
	///         resource, or if a timeout occurs while scanning the movie.
	///
	virtual boost::intrusive_ptr<resource> get_exported_resource(const std::string& symbol);

	// see docs in movie_definition.h
	virtual void add_import(const std::string& source_url, int id, const std::string& symbol)
	{
	    assert(in_import_table(id) == false);
	    m_imports.push_back(import_info(source_url, id, symbol));
	}

	/// \brief
	/// Calls back the visitor for each movie that we
	/// import symbols from.
	virtual void visit_imported_movies(import_visitor& visitor);

	// See docs in movie_definition.h
	virtual void resolve_import(const std::string& source_url,
		movie_definition* source_movie);

	void add_character(int character_id, character_def* c);

	/// \brief
	/// Return a character from the dictionary
	/// NOTE: call add_ref() on the return or put in a boost::intrusive_ptr<>
	/// TODO: return a boost::intrusive_ptr<> directly...
	///
	character_def*	get_character_def(int character_id);

	// See dox in movie_definition
	//
	// locks _namedFramesMutex
	//
	bool get_labeled_frame(const std::string& label, size_t& frame_number);

	void	add_font(int font_id, font* f);

	font*	get_font(int font_id) const;

	// See dox in movie_definition.h
	bitmap_character_def*	get_bitmap_character_def(int character_id);

	// See dox in movie_definition.h
	void	add_bitmap_character_def(int character_id, bitmap_character_def* ch);

	// See dox in movie_definition.h
	sound_sample*	get_sound_sample(int character_id);

	// See dox in movie_definition.h
	virtual void	add_sound_sample(int character_id, sound_sample* sam);

	// See dox in movie_definition.h
	virtual void	set_loading_sound_stream_id(int id) { m_loading_sound_stream = id; }

	// See dox in movie_definition.h
	int get_loading_sound_stream_id() { return m_loading_sound_stream; }

	// See dox in movie_definition.h
	void	addControlTag(ControlTag* tag)
	{
	    assert(tag);
	    boost::mutex::scoped_lock lock(_frames_loaded_mutex);
	    m_playlist[_frames_loaded].push_back(tag);
	}

	// See dox in movie_definition.h
	//
	// locks _namedFramesMutex and _frames_loaded_mutex
	//
	void add_frame_name(const std::string& name);

	/// Set an input object for later loading DefineBits
	/// images (JPEG images without the table info).
	void	set_jpeg_loader(std::auto_ptr<jpeg::input> j_in)
	{
	    assert(m_jpeg_in.get() == NULL);
	    m_jpeg_in = j_in;
	}

	// See dox in movie_definition.h
	jpeg::input*	get_jpeg_loader()
	{
	    return m_jpeg_in.get();
	}

	virtual const PlayList* getPlaylist(size_t frame_number) const
	{
#ifndef NDEBUG
		boost::mutex::scoped_lock lock(_frames_loaded_mutex);
		assert(frame_number <= _frames_loaded);
#endif

		PlayListMap::const_iterator it = m_playlist.find(frame_number);
		if ( it == m_playlist.end() ) return NULL;
		else return &(it->second);
	}

	/// Calls readHeader() and completeLoad() in sequence.
	//
	/// @return false on failure
	/// 	see description of readHeader() and completeLoad()
	///	for possible reasons of failures
	///
	bool read(std::auto_ptr<tu_file> in, const std::string& url);

	/// Read the header of the SWF file
	//
	/// This function only reads the header of the SWF
	/// stream and assigns the movie an URL.
	/// Call completeLoad() to fire up the loader thread.
	///
	/// @param in the tu_file from which to read SWF
	/// @param url the url associated with the input
	///
	/// @return false if SWF header could not be parsed
	///
	bool readHeader(std::auto_ptr<tu_file> in, const std::string& url);

	/// Complete load of the SWF file
	//
	/// This function completes parsing of the SWF stream
	/// engaging a separate thread.
	/// Make sure you called readHeader before this!
	///
	/// @return false if the loading thread could not be started.
	///
	bool completeLoad();

	/// \brief
	/// Ensure that frame number 'framenum' (1-based offset)
	/// has been loaded (load on demand).
	///
	bool ensure_frame_loaded(size_t framenum);

	/// Read and parse all the SWF stream (blocking until load is finished)
	//
	/// This function uses a private TagLoadersTable
	/// to interpret specific tag types.
	/// Currently the TagLoadersTable in use is the
	/// TagLoadersTable singleton.
	///
	void read_all_swf();

	virtual void load_next_frame_chunk();

	/// Fill up *fonts with fonts that we own.
	void get_owned_fonts(std::vector<font*>* fonts);

	/// Create an instance of this movie.
	//
	/// TOCHECK:
	/// Make sure you called completeLoad() before this
	/// function is invoked (calling read() will do that for you).
	///
	/// TOCHECK:
	/// The _root reference of the newly created movie_root
	/// will be set to a newly created movie_instance.
	///
	movie_instance* create_movie_instance(character* parent=0);

	virtual const std::string& get_url() const { return _url; }
	
	const rect&	get_bound() const {
    // It is required that get_bound() is implemented in character definition
    // classes. However, it makes no sense to call it for movie interfaces.
    // get_bound() is currently only used by generic_character which normally
    // is used only shape character definitions. See character_def.h to learn
    // why it is virtual anyway.
    abort(); // should not be called  
		static rect unused;
		return unused;
	}

	// See dox in base class
	//
	// TODO: implement in base class ?
	//
	void addTimelineDepth(int depth)
	{
		_timeline.addDepth(depth);
	}

	// See dox in base class
	//
	// TODO: implement in base class ?
	//
	void removeTimelineDepth(int depth)
	{
		_timeline.removeDepth(depth);
	}

	// See dox in base class
	//
	// TODO: implement in base class ?
	//
	void getTimelineDepths(size_t frameno, std::vector<int>& depths)
	{
		_timeline.getFrameDepths(frameno, depths);
	}

protected:

#ifdef GNASH_USE_GC
	/// Mark reachable resources of a movie_def_impl
	//
	/// Reachable resources are:
	///	- fonts (m_fonts)
	///	- bitmap characters (m_bitmap_characters)
	///	- bitmaps (m_bitmap_list) [ what's the difference with bitmap chracters ?? ]
	///	- sound samples (m_sound_samples)
	///	- exports (m_exports)
	///	- imported movies (m_import_source_movies)
	///	- character dictionary (_dictionary)
	///
	/// TODO: do we really need all this stuff to be a GcResource ??
	///
	void markReachableResources() const;
#endif // GNASH_USE_GC

};

} // namespace gnash

#endif // GNASH_MOVIE_DEF_IMPL_H
