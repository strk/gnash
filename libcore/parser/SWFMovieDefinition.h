// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

// The SWFMovieDefinition is the 'root' definition of a SWF movie, including
// movies loaded into another SWF file. Each self-contained SWF file has exactly
// one SWFMovieDefinition.

#ifndef GNASH_SWF_MOVIE_DEFINITION_H
#define GNASH_SWF_MOVIE_DEFINITION_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h" // for USE_SWFTREE
#endif

#include "smart_ptr.h" // GNASH_USE_GC
#include "fontlib.h"
#include "GnashImageJpeg.h"
#include "IOChannel.h"
#include "movie_definition.h" // for inheritance
#include "character_def.h" // for boost::intrusive_ptr visibility of dtor
#include "StringPredicates.h" 
#include "rect.h"
#include "GnashNumeric.h"

#include <map> // for CharacterDictionary
#include <set> // for _importSources
#include <string>
#include <memory> // for auto_ptr
#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/scoped_ptr.hpp>

//
// Forward declarations
namespace gnash {
	class SWFMovieDefinition;
	class SWFStream;
    class movie_root;
	class MovieClip;
	class movie_instance;
	namespace SWF {
		class TagLoadersTable;
	}
    class RunInfo;
    class Font;
}

namespace gnash
{

/// \brief
/// SWFMovieDefinition helper class handling start and execution of
/// an SWF loading thread
///
class MovieLoader
{
public:

	MovieLoader(SWFMovieDefinition& md);

	~MovieLoader();

	/// Start loading thread.
	//
	/// The associated SWFMovieDefinition instance
	/// is expected to have already read the SWF
	/// header and applied a zlib adapter if needed.
	///
	bool start();

	/// Return true if the MovieLoader thread was started
	bool started() const;

	/// Return true if called from the MovieLoader thread.
	bool isSelfThread() const;

private:

	SWFMovieDefinition& _movie_def;

	mutable boost::mutex _mutex;
	std::auto_ptr<boost::thread> _thread;

	// Barrier to ensure that _thread
	// is initialized before the loader thread
	// continues execution
	boost::barrier _barrier;

	/// Entry point for the actual thread
	static void execute(MovieLoader& ml, SWFMovieDefinition* md);

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
	typedef std::map<int, boost::intrusive_ptr<character_def> >
        CharacterContainer;

	typedef CharacterContainer::iterator CharacterIterator;

	typedef CharacterContainer::const_iterator CharacterConstIterator;

	/// Get the Character with the given id
	//
	/// returns a NULL if the id is unknown.
	///
	boost::intrusive_ptr<character_def> getDisplayObject(int id);

	/// Add a Character assigning it the given id
	//
	/// replaces any existing DisplayObject with the same id
	///
	void addDisplayObject(int id, boost::intrusive_ptr<character_def> c);

	/// Return an iterator to the first dictionary element
	CharacterIterator begin() { return _map.begin(); }

	/// Return a const_iterator to the first dictionary element
	CharacterConstIterator begin() const { return _map.begin(); }

	/// Return an iterator to one-past last dictionary element
	CharacterIterator end() { return _map.end(); }

	/// Return a const_iterator to one-past last dictionary element
	CharacterConstIterator end() const { return _map.end(); }

    friend std::ostream& operator<<(std::ostream& o, const CharacterDictionary& cd);

#ifdef GNASH_USE_GC
	/// Mark all dictionary items to be reachable (for GC)
	void markReachableResources() const
	{
		for(CharacterConstIterator i=_map.begin(), e=_map.end(); i!=e; ++i)
		{
			i->second->setReachable();
		}
	}
#endif // GNASH_USE_GC

private:

	CharacterContainer _map;

};


/// Immutable definition of a SWF movie's contents.
//
/// It cannot be played directly, and does not hold
/// current state; for that you need to call create_movie_instance()
/// to get a movie instance 
///
class SWFMovieDefinition : public movie_definition
{
public:

    /// Construct a SWF movie.
    //
    /// @param runInfo      A RunInfo containing information used for
    ///                     parsing.
	SWFMovieDefinition(const RunInfo& runInfo);

	~SWFMovieDefinition();

	// ...
	size_t get_frame_count() const { return m_frame_count; }
	float	get_frame_rate() const { return m_frame_rate; }
	const rect& get_frame_size() const { return m_frame_size; }

	float get_width_pixels() const
	{
		return std::ceil(twipsToPixels(m_frame_size.width()));
	}

	float	get_height_pixels() const
	{
		return std::ceil(twipsToPixels(m_frame_size.height()));
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

	// See docs in movie_definition.h
	virtual void export_resource(const std::string& symbol,
            ExportableResource* res);

	/// Get the named exported resource, if we expose it.
	//
	/// @return NULL if the label doesn't correspond to an exported
	///         resource, or if a timeout occurs while scanning the movie.
	///
	virtual boost::intrusive_ptr<ExportableResource> get_exported_resource(
            const std::string& symbol) const;

	virtual void importResources(boost::intrusive_ptr<movie_definition> source,
            Imports& imports);

	void addDisplayObject(int DisplayObject_id, character_def* c);

	/// \brief
	/// Return a DisplayObject from the dictionary
	/// NOTE: call add_ref() on the return or put in a boost::intrusive_ptr<>
	/// TODO: return a boost::intrusive_ptr<> directly...
	///
	character_def*	get_character_def(int DisplayObject_id);

	// See dox in movie_definition
	//
	// locks _namedFramesMutex
	//
	bool get_labeled_frame(const std::string& label, size_t& frame_number);

	void	add_font(int font_id, Font* f);

	Font*	get_font(int font_id) const;

	Font* get_font(const std::string& name, bool bold, bool italic) const;

	// See dox in movie_definition.h
	BitmapInfo* getBitmap(int DisplayObject_id);

	// See dox in movie_definition.h
	void addBitmap(int DisplayObject_id, boost::intrusive_ptr<BitmapInfo> im);

	// See dox in movie_definition.h
	sound_sample*	get_sound_sample(int DisplayObject_id);

	// See dox in movie_definition.h
	virtual void	add_sound_sample(int DisplayObject_id, sound_sample* sam);

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
	void set_jpeg_loader(std::auto_ptr<JpegImageInput> j_in)
	{
	    if (m_jpeg_in.get())
	    {
	        /// There should be only one JPEGTABLES tag in an SWF (see: 
	        /// http://www.m2osw.com/en/swf_alexref.html#tag_jpegtables)
	        /// Discard any subsequent attempts to set the jpeg loader
	        /// to avoid crashing on very malformed SWFs. (No conclusive tests
	        /// for pp behaviour, though one version also crashes out on the
	        /// malformed SWF that triggers this assert in Gnash).
	        log_swferror(_("More than one JPEGTABLES tag found: not resetting JPEG loader"));
	        return;
	    }
	    m_jpeg_in = j_in;
	}

	// See dox in movie_definition.h
	JpegImageInput* get_jpeg_loader()
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

	/// Read the header of the SWF file
	//
	/// This function only reads the header of the SWF
	/// stream and assigns the movie an URL.
	/// Call completeLoad() to fire up the loader thread.
	///
	/// @param in the IOChannel from which to read SWF
	/// @param url the url associated with the input
	///
	/// @return false if SWF header could not be parsed
	///
	bool readHeader(std::auto_ptr<IOChannel> in, const std::string& url);

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
	movie_instance* create_movie_instance(DisplayObject* parent=0);

    virtual DisplayObject* createDisplayObject(DisplayObject*, int) {
        return 0;
    }

	virtual const std::string& get_url() const { return _url; }
	
	const rect&	get_bound() const {
    // It is required that get_bound() is implemented in DisplayObject definition
    // classes. However, it makes no sense to call it for movie interfaces.
    // get_bound() is currently only used by DisplayObject which normally
    // is used only shape DisplayObject definitions. See character_def.h to learn
    // why it is virtual anyway.
    abort(); // should not be called  
		static rect unused;
		return unused;
	}

#ifdef USE_SWFTREE

	// These methods attach the contents of the METADATA tag
	// to a movie_definition.
	virtual void storeDescriptiveMetadata(const std::string& data)
	{
	    _metadata = data;
	}

	virtual const std::string& getDescriptiveMetadata() const
	{
	    return _metadata;
	}	

#endif

private:

#ifdef USE_SWFTREE
    // For storing descriptive metadata (information only)
    std::string _metadata;
#endif

	/// Characters Dictionary
	CharacterDictionary	_dictionary;

	/// Mutex protecting _dictionary
	mutable boost::mutex _dictionaryMutex;

	/// Tags loader table
	SWF::TagLoadersTable& _tag_loaders;

	typedef std::map<int, boost::intrusive_ptr<Font> > FontMap;
	FontMap m_fonts;

	typedef std::map<int, boost::intrusive_ptr<BitmapInfo> > Bitmaps;
	Bitmaps _bitmaps;

	typedef std::map<int, boost::intrusive_ptr<sound_sample> > SoundSampleMap;
	SoundSampleMap m_sound_samples;

	typedef std::map<size_t, PlayList> PlayListMap;

	/// Movie control events for each frame.
	PlayListMap m_playlist;

	/// 0-based frame #'s
	typedef std::map<std::string, size_t, StringNoCaseLessThan> NamedFrameMap;
	NamedFrameMap _namedFrames;

	// Mutex protecting access to _namedFrames
	mutable boost::mutex _namedFramesMutex;

	typedef std::map<std::string, boost::intrusive_ptr<ExportableResource>,
            StringNoCaseLessThan > ExportMap;
	ExportMap _exportedResources;

	// Mutex protecting access to _exportedResources
	mutable boost::mutex _exportedResourcesMutex;

	/// Movies we import from; hold a ref on these,
	/// to keep them alive
	typedef std::vector<boost::intrusive_ptr<movie_definition> > ImportVect;
	ImportVect m_import_source_movies;

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

	std::auto_ptr<JpegImageInput> m_jpeg_in;

	std::string _url;

    /// Non transferable stream.
    boost::scoped_ptr<SWFStream> _str;

	std::auto_ptr<IOChannel> _in;

	/// swf end position (as read from header)
    size_t _swf_end_pos;

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

	/// A flag set to true when load cancelation is requested
	bool _loadingCanceled;

	/// Movies we import resources from
	std::set< boost::intrusive_ptr<movie_definition> > _importSources;


protected:

#ifdef GNASH_USE_GC
	/// Mark reachable resources of a SWFMovieDefinition
	//
	/// Reachable resources are:
	///	- fonts (m_fonts)
	///	- bitmaps (_bitmaps)
	///	- bitmaps (m_bitmap_list) [ what's the difference with bitmap
    ///   DisplayObjects ?? ]
	///	- sound samples (m_sound_samples)
	///	- exports (m_exports)
	///	- imported movies (m_import_source_movies)
	///	- DisplayObject dictionary (_dictionary)
	///
	/// TODO: do we really need all this stuff to be a GcResource ??
	///
	void markReachableResources() const;
#endif // GNASH_USE_GC

private:

    /// Used to retrieve the sound::sound_handler and other resources
    /// for parsing.
    //
    /// @todo   Add to base class? This would make it available for other
    ///         kinds of movies (e.g. FLV) and make it easier to initialize
    ///         movie_root with the same RunInfo as its first definition.
    const RunInfo& _runInfo;

};

} // namespace gnash

#endif // GNASH_MOVIE_DEF_IMPL_H
