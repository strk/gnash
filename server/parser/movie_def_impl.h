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
#include "smart_ptr.h"
//#include "button.h" // for mouse_button_state
#include "timers.h" // for Timer
#include "fontlib.h"
#include "font.h"
#include "jpeg.h"
#include "tu_file.h"
#include "movie_definition.h" // for inheritance
#include "character_def.h" // for smart_ptr visibility of dtor
#include "bitmap_character_def.h" // for smart_ptr visibility of dtor
#include "resource.h" // for smart_ptr visibility of dtor
#include "stream.h" // for get_bytes_loaded

#include <map> // for CharacterDictionary
#include <string>
#include <memory> // for auto_ptr

// We'd avoid SDL threads if possible. Please define the macro below
// if you experience problems and report the difference on gnash-dev
#undef REALLY_USE_SDL_THREADS

#ifdef REALLY_USE_SDL_THREADS
#ifdef HAVE_SDL_H
# define USE_SDL_THREADS 1
#endif
#endif

#ifdef USE_SDL_THREADS
#	include "SDL.h"
#	include "SDL_thread.h"
#endif


namespace gnash
{

// Forward declarations
class import_info;
class movie_def_impl;
class movie_root;

//
// Helper for movie_def_impl
//

class import_info
{
private:
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

	/// Wait for specified frame number (1-based) to be loaded
	//
	/// Block caller thread until frame is loaded.
	///
	void wait_for_frame(size_t framenum);

	/// Signal load of given frame number (if anyone waiting for it)
	void signal_frame_loaded(size_t frameno);

	void lock();

	void unlock();

private:

	size_t _waiting_for_frame;
	movie_def_impl& _movie_def;

#ifdef USE_SDL_THREADS

	static int execute(void* arg);

	SDL_Thread* _thread;
	SDL_cond* _frame_reached_condition;
	SDL_mutex* _mutex;

#else

	pthread_cond_t _frame_reached_condition;
	pthread_mutex_t _mutex;
	pthread_t _thread;

	/// Entry point for the actual thread
	static void *execute(void* arg);

#endif

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
	/// It contains pairs of 'int' and 'smart_ptr<character_def>'
	///
	typedef std::map< int, smart_ptr<character_def> > container;
	//typedef hash< int, smart_ptr<character_def> >container;

	typedef container::iterator iterator;

	typedef container::const_iterator const_iterator;

	/// Get the Character with the given id
	//
	/// returns a NULL if the id is unknown.
	///
	smart_ptr<character_def> get_character(int id);

	/// Add a Character assigning it the given id
	//
	/// replaces any existing character with the same id
	///
	void add_character(int id, smart_ptr<character_def> c);

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
private:

	container _map;

};


/// Immutable definition of a movie's contents.
//
/// It cannot be played directly, and does not hold
/// current state; for that you need to call create_instance()
/// to get a movie instance (gnash::movie_interface).
///
class movie_def_impl : public movie_definition
{
private:
	/// Characters Dictionary
	CharacterDictionary	_dictionary;
	//hash<int, smart_ptr<character_def> >		m_characters;

	/// Tags loader table
	SWF::TagLoadersTable& _tag_loaders;

	hash<int, smart_ptr<font> >	 		m_fonts;
	hash<int, smart_ptr<bitmap_character_def> >	m_bitmap_characters;
	hash<int, smart_ptr<sound_sample> >		m_sound_samples;
	hash<int, smart_ptr<sound_sample> >		m_sound_streams;

	/// A list of movie control events for each frame.
	std::vector<std::vector<execute_tag*> >	   	m_playlist;

	/// Init actions for each frame.
	std::vector<std::vector<execute_tag*> >	   m_init_action_list;

	/// 0-based frame #'s
	stringi_hash<size_t> m_named_frames;

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
	size_t	m_frame_count;
	int	m_version;
	size_t	m_loading_frame;
	int	m_loading_sound_stream;
	uint32	m_file_length;

	std::auto_ptr<jpeg::input> m_jpeg_in;

	std::string _url;

	std::auto_ptr<stream> _str;

	tu_file* in;

	std::auto_ptr<tu_file> _zlib_file;

	/// swf end position (as read from header)
	unsigned int _swf_end_pos;

	/// asyncronous SWF loader and parser
	MovieLoader _loader;

public:

	movie_def_impl(create_bitmaps_flag cbf, create_font_shapes_flag cfs);

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

	virtual size_t	get_loading_frame() const
	{
		return m_loading_frame;
	}

	/// Get number of bytes loaded from input stream
	size_t	get_bytes_loaded() const {
		// we assume seek-backs are disabled
		return _str->get_position();
	}

	/// Get total number of bytes in input stream
	size_t	get_bytes_total() const {
		return m_file_length;
	}

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

	/// \brief
	/// Return a character from the dictionary
	/// NOTE: call add_ref() on the return or put in a smart_ptr<>
	/// TODO: return a smart_ptr<> directly...
	///
	character_def*	get_character_def(int character_id);

	/// Returns 0-based frame #
	bool get_labeled_frame(const char* label, size_t* frame_number)
	{
		return m_named_frames.get(label, frame_number);
	}

	void	add_font(int font_id, font* f);
	font*	get_font(int font_id);
	bitmap_character_def*	get_bitmap_character_def(int character_id);
	void	add_bitmap_character_def(int character_id, bitmap_character_def* ch);
	sound_sample*	get_sound_sample(int character_id);
	virtual void	add_sound_sample(int character_id, sound_sample* sam);
	virtual void	set_loading_sound_stream_id(int id) { m_loading_sound_stream = id; }
	int		get_loading_sound_stream_id() { return m_loading_sound_stream; }

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
	//void	add_init_action(int sprite_id, execute_tag* e)
	void	add_init_action(execute_tag* e)
	{
	    assert(e);
	    m_init_action_list[m_loading_frame].push_back(e);
	}

	/// Labels the frame currently being loaded with the
	/// given name.  A copy of the name string is made and
	/// kept in this object.
	void	add_frame_name(const char* name)
	{
	    assert(m_loading_frame < m_frame_count);

	    tu_string	n = name;

			if (m_named_frames.get(n, NULL) == false)	// frame should not already have a name (?)
			{
		    m_named_frames.add(n, m_loading_frame);	// stores 0-based frame #
			}
	}

	/// Set an input object for later loading DefineBits
	/// images (JPEG images without the table info).
	void	set_jpeg_loader(std::auto_ptr<jpeg::input> j_in)
	{
	    assert(m_jpeg_in.get() == NULL);
	    m_jpeg_in = j_in;
	}

	/// Get the jpeg input loader, to load a DefineBits
	/// image (one without table info).
	//
	/// NOTE: ownership is NOT transferred
	///
	jpeg::input*	get_jpeg_loader()
	{
	    return m_jpeg_in.get();
	}

	virtual const std::vector<execute_tag*>& get_playlist(size_t frame_number)
	{
		return m_playlist[frame_number];
	}

	virtual const std::vector<execute_tag*>* get_init_actions(size_t frame_number)
	{
		assert(frame_number <= m_loading_frame);
		//ensure_frame_loaded(frame_number);
		return &m_init_action_list[frame_number];
	}

	/// Read (w/out playing) a Movie definition from an SWF file.
	//
	/// Note that the *full* SWF is read before
	/// this function returns. We should change this
	/// interface to both read and play a file instead.
	///
	/// This function uses a private TagLoadersTable
	/// to interpret specific tag types.
	/// Currently the TagLoadersTable in use is the
	/// gnash::s_tag_loaders global variable
	///
	/// @param in the tu_file from which to read SWF
	/// @param url the url associated with the input
	/// @return false if SWF file could not be parsed
	///
	bool read(tu_file *in, const std::string& url);

	/// \brief
	/// Ensure that frame number 'framenum' (1-based offset)
	/// has been loaded (load on demand).
	///
	bool ensure_frame_loaded(size_t framenum);

	/// Read and parse all the SWF stream (blocking until load is finished)
	void read_all_swf();

	virtual void load_next_frame_chunk();

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

	virtual const std::string& get_url() const { return _url; }


};

} // namespace gnash

#endif // GNASH_MOVIE_DEF_IMPL_H
