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
#ifdef HAVE_PTHREADS
#include <pthread.h>
#endif

#include <string>

#include "movie_def_impl.h"
#include "movie_definition.h" // for inheritance
#include "movie_instance.h" // for create_instance()
#include "tu_file.h"
#include "zlib_adapter.h"
#include "stream.h"
#include "jpeg.h"
#include "fontlib.h"
#include "font.h"
#include "log.h"
#include "sprite_instance.h"
//#include "render.h"
#include "bitmap_character_def.h"
#include "smart_ptr.h"
#include "swf/TagLoadersTable.h"
//#include "execute_tag.h"
#include "movie_root.h"

// Increment this when the cache data format changes.
#define CACHE_FILE_VERSION 4

// If != 0 this is the number of frames to load at each iteration
// of the main loop. Loading in chunks greatly speeds the process up
#define FRAMELOAD_CHUNK 0

// Debug frames load
#undef DEBUG_FRAMES_LOAD 

// Define this this to load movies using a separate thread
// (undef and it will fully load a movie before starting to play it)
#define LOAD_MOVIES_IN_A_SEPARATE_THREAD 1

// Debug threads locking
#undef DEBUG_THREADS_LOCKING

namespace gnash
{

#ifdef USE_SDL_THREADS

MovieLoader::MovieLoader(movie_def_impl& md)
	:
	_waiting_for_frame(0),
	_movie_def(md)
{
	_frame_reached_condition = SDL_CreateCond();
	_mutex = SDL_CreateMutex();
}

MovieLoader::~MovieLoader()
{
	SDL_DestroyMutex(_mutex);
	SDL_DestroyCond(_frame_reached_condition);
}

int MovieLoader::execute(void* arg)
{
	movie_def_impl* md = static_cast<movie_def_impl*>(arg);
	md->read_all_swf();
	return 0;
}

bool MovieLoader::start()
{
	 _thread = SDL_CreateThread(execute, &_movie_def);
	 if (_thread == NULL)
	 {
		 return false;
	 }
	return true;
}

inline void MovieLoader::signal_frame_loaded(size_t frameno)
{
	if (_waiting_for_frame &&
		frameno >= _waiting_for_frame )
	{
		SDL_CondSignal(_frame_reached_condition);
	}
}

void MovieLoader::wait_for_frame(size_t framenum)
{

	lock();

	if (_movie_def.get_loading_frame() < framenum)
	{
		//log_msg("Waiting for frame %u to load", framenum);

		assert(_waiting_for_frame == 0);
		_waiting_for_frame = framenum;

		do
		{
			SDL_CondWait(_frame_reached_condition, _mutex);
		}
		while (_movie_def.get_loading_frame() < framenum);

		_waiting_for_frame = 0;

		//log_msg("Done waiting (frame %u/%u loaded)",
		//	_movie_def.get_loading_frame(),
		//	_movie_def.get_frame_count());
	}

	unlock();
}

inline void MovieLoader::lock()
{
	if ( -1 == SDL_mutexP(_mutex) )
	{
		log_error("Error unlocking MovieLoader");
	}
}

inline void MovieLoader::unlock()
{
	if ( -1 == SDL_mutexV(_mutex) )
	{
		log_error("Error unlocking MovieLoader");
	}
}

#else

MovieLoader::MovieLoader(movie_def_impl& md)
	:
	_waiting_for_frame(0),
	_movie_def(md)
{
	pthread_cond_init(&_frame_reached_condition, NULL);
	pthread_mutex_init(&_mutex, NULL);
}

MovieLoader::~MovieLoader()
{
	if ( pthread_cond_destroy(&_frame_reached_condition) != 0 )
	{
		log_error("Error destroying MovieLoader condition");
	}

	if ( pthread_mutex_destroy(&_mutex) != 0 )
	{
		log_error("Error destroying MovieLoader mutex");
	}
}

void*
MovieLoader::execute(void* arg)
{
	movie_def_impl* md = static_cast<movie_def_impl*>(arg);
	md->read_all_swf();
	// maybe this frees all resources and that's bad !
	//pthread_exit(NULL);
	
	/* Better to cancel yourself methinks: 'man 3p pthread_cancel' */
	pthread_cancel(pthread_self());
        pthread_testcancel();
	return NULL;
}

bool
MovieLoader::start()
{
	if ( pthread_create(&_thread, NULL, execute, &_movie_def) )
	{
		return false;
	}

	// should set some mutexes ?

	return true;
}

inline void
MovieLoader::signal_frame_loaded(size_t frameno)
{
	if (_waiting_for_frame &&
		frameno >= _waiting_for_frame )
	{
		pthread_cond_signal(&_frame_reached_condition);
	}
}

inline void
MovieLoader::lock()
{

#ifdef DEBUG_THREADS_LOCKING
	// debugging
	if ( pthread_equal(pthread_self(), _thread) ) {
		log_msg("MovieLoader locking itself");
	} else {
		log_msg("MovieLoader being locked by another thread");
	}
#endif

	if ( pthread_mutex_lock(&_mutex) != 0 )
	{
		log_error("Error locking MovieLoader");
	}

#ifdef DEBUG_THREADS_LOCKING
	// debugging
	if ( pthread_equal(pthread_self(), _thread) ) {
		log_msg("MovieLoader locked by itself");
	} else {
		log_msg("MovieLoader locked by another thread");
	}
#endif
}

inline void
MovieLoader::unlock()
{

#ifdef DEBUG_THREADS_LOCKING
	// debugging
	if ( pthread_equal(pthread_self(), _thread) ) {
		log_msg("MovieLoader unlocking itself");
	} else {
		log_msg("MovieLoader being unlocked by another thread");
	}
#endif

	if ( pthread_mutex_unlock(&_mutex) != 0 )
	{
		log_error("Error unlocking MovieLoader");
	}

#ifdef DEBUG_THREADS_LOCKING
	// debugging
	if ( pthread_equal(pthread_self(), _thread) ) {
		log_msg("MovieLoader unlocked itself");
	} else {
		log_msg("MovieLoader unlocked by another thread");
	}
#endif
}

void
MovieLoader::wait_for_frame(size_t framenum)
{

	// lock the loader so we can rely on m_loading_frame
	lock();

	if ( _movie_def.get_loading_frame() < framenum )
	{
		assert(_waiting_for_frame == 0);
		_waiting_for_frame = framenum;
		pthread_cond_wait(&_frame_reached_condition, &_mutex);
		_waiting_for_frame = 0;
	}

	unlock();
}

#endif	// PTHREAD MovieLoader


//
// some utility stuff
//

void	dump_tag_bytes(stream* in)
    // Log the contents of the current tag, in hex.
{
    static const int	ROW_BYTES = 16;
    char	row_buf[ROW_BYTES];
    int	row_count = 0;

    while(in->get_position() < in->get_tag_end_position())
        {
            int	c = in->read_u8();
            log_msg("%02X", c);

            if (c < 32) c = '.';
            if (c > 127) c = '.';
            row_buf[row_count] = c;
				
            row_count++;
            if (row_count >= ROW_BYTES)
                {
                    log_msg("    ");
                    for (int i = 0; i < ROW_BYTES; i++)
                        {
                            log_msg("%c", row_buf[i]);
                        }

                    log_msg("\n");
                    row_count = 0;
                }
            else
                {
                    log_msg(" ");
                }
        }

    if (row_count > 0)
        {
            log_msg("\n");
        }
}


//
// progress callback stuff (from Vitaly)
//
progress_callback	s_progress_function = NULL;

// Host calls this to register a function for progress bar handling
// during loading movies.
void
register_progress_callback(progress_callback progress_handle)
{
    s_progress_function = progress_handle;
}

//
// movie_def_impl
//

movie_def_impl::movie_def_impl(create_bitmaps_flag cbf,
		create_font_shapes_flag cfs)
	:
	_tag_loaders(s_tag_loaders), // FIXME: use a class-static TagLoadersTable for movie_def_impl
	m_create_bitmaps(cbf),
	m_create_font_shapes(cfs),
	m_frame_rate(30.0f),
	m_frame_count(0u),
	m_version(0),
	m_loading_frame(0u),
	m_jpeg_in(0),
	_loader(*this)
{
}

movie_def_impl::~movie_def_impl()
{
    // Release our playlist data.
    {for (size_t i = m_playlist.size() - 1; i != static_cast<size_t>(-1); i--) // Optimized
        {
            for (size_t j = m_playlist[i].size()-1; j != static_cast<size_t>(-1); j--)
                {
                    delete m_playlist[i][j];
                }
        }}
	
    // Release init action data.
    {for (size_t i = m_init_action_list.size() - 1; i != static_cast<size_t>(-1); i--) //Optimized
        {
            for (size_t j = m_init_action_list[i].size()-1; j != static_cast<size_t>(-1); j--)
                {
                    delete m_init_action_list[i][j];
                }
        }}
		
	// It's supposed to be cleaned up in read()
	// TODO: join with loader thread instead ?
	//assert(m_jpeg_in.get() == NULL);
}

bool movie_def_impl::in_import_table(int character_id)
{
    for (size_t i = 0, n = m_imports.size(); i < n; i++)
        {
            if (m_imports[i].m_character_id == character_id)
                {
                    return true;
                }
        }
    return false;
}

void movie_def_impl::visit_imported_movies(import_visitor* visitor)
{
    stringi_hash<bool>	visited;	// ugh!

    for (size_t i = 0, n = m_imports.size(); i < n; i++)
        {
            const import_info&	inf = m_imports[i];
            if (visited.find(inf.m_source_url) == visited.end())
                {
                    // Call back the visitor.
                    visitor->visit(inf.m_source_url.c_str());
                    visited[inf.m_source_url] = true;
                }
        }
}

void movie_def_impl::resolve_import(const char* source_url, movie_definition* source_movie)
{
    // @@ should be safe, but how can we verify
    // it?  Compare a member function pointer, or
    // something?
    movie_def_impl*	def_impl = static_cast<movie_def_impl*>(source_movie);
    movie_definition*	def = static_cast<movie_definition*>(def_impl);

    // Iterate in reverse, since we remove stuff along the way.
    for (size_t i = m_imports.size(); i > 0; i--)
        {
            const import_info&	inf = m_imports[i-1];
            if (inf.m_source_url == source_url)
                {
                    // Do the import.
                    smart_ptr<resource> res = def->get_exported_resource(inf.m_symbol);
                    bool	 imported = true;

                    if (res == NULL)
                        {
                            log_error("import error: resource '%s' is not exported from movie '%s'\n",
                                      inf.m_symbol.c_str(), source_url);
                        }
                    else if (font* f = res->cast_to_font())
                        {
                            // Add this shared font to our fonts.
                            add_font(inf.m_character_id, f);
                            imported = true;
                        }
                    else if (character_def* ch = res->cast_to_character_def())
                        {
                            // Add this character to our characters.
                            add_character(inf.m_character_id, ch);
                            imported = true;
                        }
                    else
                        {
                            log_error("import error: resource '%s' from movie '%s' has unknown type\n",
                                      inf.m_symbol.c_str(), source_url);
                        }

                    if (imported)
                        {
                            m_imports.erase(m_imports.begin() + i);

                            // Hold a ref, to keep this source movie_definition alive.
                            m_import_source_movies.push_back(source_movie);
                        }
                }
        }
}

void movie_def_impl::add_character(int character_id, character_def* c)
{
	assert(c);
	_dictionary.add_character(character_id, c);
}

character_def*
movie_def_impl::get_character_def(int character_id)
{
#ifndef NDEBUG
    // make sure character_id is resolved
    if (in_import_table(character_id))
        {
            log_error("get_character_def(): character_id %d is still waiting to be imported\n",
                      character_id);
        }
#endif // not NDEBUG

	smart_ptr<character_def> ch = _dictionary.get_character(character_id);
	assert(ch == NULL || ch->get_ref_count() > 1);
	return ch.get_ptr(); // mm... why don't we return the smart_ptr?
}

void movie_def_impl::add_font(int font_id, font* f)
{
    assert(f);
    m_fonts.add(font_id, f);
}

font* movie_def_impl::get_font(int font_id)
{
#ifndef NDEBUG
    // make sure font_id is resolved
    if (in_import_table(font_id))
        {
            log_error("get_font(): font_id %d is still waiting to be imported\n",
                      font_id);
        }
#endif // not NDEBUG

    smart_ptr<font>	f;
    m_fonts.get(font_id, &f);
    assert(f == NULL || f->get_ref_count() > 1);
    return f.get_ptr();
}

bitmap_character_def* movie_def_impl::get_bitmap_character_def(int character_id)
{
    smart_ptr<bitmap_character_def>	ch;
    m_bitmap_characters.get(character_id, &ch);
    assert(ch == NULL || ch->get_ref_count() > 1);
    return ch.get_ptr();
}

void
movie_def_impl::add_bitmap_character_def(int character_id,
		bitmap_character_def* ch)
{
    assert(ch);
    //log_msg("Add bitmap character %d", character_id);
    m_bitmap_characters.add(character_id, ch);

	// we can *NOT* generate bitmap_info until
	// a renderer is present
    add_bitmap_info(ch->get_bitmap_info());
}

sound_sample* movie_def_impl::get_sound_sample(int character_id)
{
    smart_ptr<sound_sample>	ch;
    m_sound_samples.get(character_id, &ch);
    assert(ch == NULL || ch->get_ref_count() > 1);
    return ch.get_ptr();
}

void movie_def_impl::add_sound_sample(int character_id, sound_sample* sam)
{
    assert(sam);
	log_msg("Add sound sample %d", character_id);
    m_sound_samples.add(character_id, sam);
}


// Read a .SWF movie.
bool
movie_def_impl::read(tu_file* in, const std::string& url)
{

	// we only read a movie once (well, headers at least)
	assert(_str.get() == NULL);

	if ( url == "" ) _url = "<anonymous>";
	else _url = url;

	uint32_t file_start_pos = in->get_position();
	uint32_t header = in->read_le32();
	m_file_length = in->read_le32();
	_swf_end_pos = file_start_pos + m_file_length;

	m_version = (header >> 24) & 255;
	if ((header & 0x0FFFFFF) != 0x00535746
		&& (header & 0x0FFFFFF) != 0x00535743)
        {
		// ERROR
		log_error("gnash::movie_def_impl::read() -- "
			"file does not start with a SWF header!\n");
		return false;
        }
	bool	compressed = (header & 255) == 'C';
    
	IF_VERBOSE_PARSE(
		log_parse("version = %d, file_length = %d",
			m_version, m_file_length);
	);

	tu_file* original_in = NULL;
	if (compressed)
        {
#if TU_CONFIG_LINK_TO_ZLIB == 0
		log_error("movie_def_impl::read(): unable to read "
			"zipped SWF data; TU_CONFIG_LINK_TO_ZLIB is 0\n");
		return false;
#endif

		IF_VERBOSE_PARSE(
			log_parse("file is compressed.");
		);

		original_in = in;

		// Uncompress the input as we read it.
		_zlib_file.reset(zlib_adapter::make_inflater(original_in));
		in = _zlib_file.get();

        }

	//stream str(in);
	_str.reset(new stream(in));

	m_frame_size.read(_str.get());
	m_frame_rate = _str->read_u16() / 256.0f;
	m_frame_count = _str->read_u16();

	// hack
	// Vitaly: I am not assured that it is correctly
	//m_frame_count = (m_frame_count == 0) ? 1 : m_frame_count;

	/* Markus: Probably this is better anyways */
	
	if(m_frame_count == 0)m_frame_count++;
	
	m_playlist.resize(m_frame_count);
	m_init_action_list.resize(m_frame_count);

	IF_VERBOSE_PARSE(
		m_frame_size.print();
		log_parse("frame rate = %f, frames = %d",
			m_frame_rate, m_frame_count);
	);

#ifdef LOAD_MOVIES_IN_A_SEPARATE_THREAD

	// Start the loading frame
	if ( ! _loader.start() )
	{
		log_error("Could not start loading thread");
	}

	// Wait until 'startup_frames' have been loaded
#if 1
	size_t startup_frames = 0;
#else
	size_t startup_frames = m_frame_count;
#endif
	ensure_frame_loaded(startup_frames);

#else // undef LOAD_MOVIES_IN_A_SEPARATE_THREAD

	read_all_swf();
#endif

// Can't delete here as we will keep reading from it while playing
// FIXME: remove this at end of reading (or in destructor)
#if 0
	if (original_in)
        {
            // Done with the zlib_adapter.
            delete in;
        }
#endif

	return true;
}


// 1-based frame number
inline bool
movie_def_impl::ensure_frame_loaded(size_t framenum)
{
        //log_msg("Waiting for frame %u to be loaded", framenum);
	_loader.wait_for_frame(framenum);
        //log_msg("Condition reached (m_loading_frame=%u)", m_loading_frame);


	// TODO: return false on timeout 
	return true;
}


/* movie_def_impl */
void movie_def_impl::get_owned_fonts(std::vector<font*>* fonts)
    // Fill up *fonts with fonts that we own.
{
    assert(fonts);
    fonts->resize(0);

    std::vector<int>	font_ids;

    for (hash<int, smart_ptr<font> >::iterator it = m_fonts.begin();
         it != m_fonts.end();
         ++it)
        {
            font*	f = it->second.get_ptr();
            if (f->get_owning_movie() == this)
                {
                    // Sort by character id, so the ordering is
                    // consistent for cache read/write.
                    int	id = it->first;

                    // Insert in correct place.
                    unsigned int insert;
                    for (insert = 0; insert < font_ids.size(); insert++)
                        {
                            if (font_ids[insert] > id)
                                {
                                    // We want to insert here.
                                    break;
                                }
                        }
                    fonts->insert(fonts->begin() + insert, f);
                    font_ids.insert(font_ids.begin() + insert, id);
                }
        }
}


/* movie_def_impl */
void movie_def_impl::generate_font_bitmaps()
    // Generate bitmaps for our fonts, if necessary.
{
    // Collect list of fonts.
    std::vector<font*>	fonts;
    get_owned_fonts(&fonts);
    fontlib::generate_font_bitmaps(fonts, this);
}

void
movie_def_impl::output_cached_data(tu_file* out, const cache_options& options)
{
    // Write a little header.
    char	header[5];
    strcpy(header, "gscX");
    header[3] = CACHE_FILE_VERSION;
    compiler_assert(CACHE_FILE_VERSION < 256);

    out->write_bytes(header, 4);

    // Write font data.
    std::vector<font*>	fonts;
    get_owned_fonts(&fonts);
    fontlib::output_cached_data(out, fonts, this, options);

	// Write character data.
	{

	for ( CharacterDictionary::iterator
		it = _dictionary.begin(), itEnd = _dictionary.end();
		it != itEnd;
		++it )
	{
		out->write_le16(it->first);
		it->second->output_cached_data(out, options);
	}
			
#if 0
	for (hash<int, smart_ptr<character_def> >::iterator it = m_characters.begin();
          it != m_characters.end();
          ++it)
        {
            out->write_le16(it->first);
            it->second->output_cached_data(out, options);
        }
#endif
	}

    out->write_le16((int16_t) -1);	// end of characters marker
}


void
movie_def_impl::input_cached_data(tu_file* in)
{
    // Read the header & check version.
    unsigned char	header[4];
    in->read_bytes(header, 4);
    if (header[0] != 'g' || header[1] != 's' || header[2] != 'c')
        {
            log_error("cache file does not have the correct format; skipping\n");
            return;
        }
    else if (header[3] != CACHE_FILE_VERSION)
        {
            log_error(
                "cached data is version %d, but we require version %d; skipping\n",
                int(header[3]), CACHE_FILE_VERSION);
            return;
        }

    // Read the cached font data.
    std::vector<font*>	fonts;
    get_owned_fonts(&fonts);
    fontlib::input_cached_data(in, fonts, this);

    // Read the cached character data.
    for (;;)
        {
            if (in->get_error() != TU_FILE_NO_ERROR)
                {
                    log_error("error reading cache file (characters); skipping\n");
                    return;
                }
            if (in->get_eof())
                {
                    log_error("unexpected eof reading cache file (characters); skipping\n");
                    return;
                }

            int16_t	id = in->read_le16();
            if (id == (int16_t) -1) { break; }	// done

            smart_ptr<character_def> ch = _dictionary.get_character(id);
            //m_characters.get(id, &ch);
            if (ch != NULL)
                {
                    ch->input_cached_data(in);
                }
            else
                {
                    log_error("sync error in cache file (reading characters)!  "
                              "Skipping rest of cache data.\n");
                    return;
                }
        }
}

movie_interface*
movie_def_impl::create_instance()
{
    movie_root*	m = new movie_root(this);
    assert(m);

    sprite_instance* root_movie = new movie_instance(this, m, NULL);
    assert(root_movie);

    root_movie->set_name("_root");
    m->set_root_movie(root_movie);

    // @@ somewhere in here I *might* add _url variable
    // (or is it a member?)

    m->add_ref();

		root_movie->execute_frame_tags(0); // create _root dlist

    return m;
}


//
// CharacterDictionary
//

void
CharacterDictionary::dump_chars() const
{
	for ( const_iterator it=begin(), endIt=end();
		it != endIt; ++it )
	{
		log_msg("Character %d @ %p", it->first, static_cast<void*>(it->second.get_ptr()));
		//character_def* cdef = it->second;
	}
}

smart_ptr<character_def>
CharacterDictionary::get_character(int id)
{
	container::iterator it = _map.find(id);
	if ( it == _map.end() )
	{
		log_msg("Could not find char %d, dump is:", id);
		dump_chars();
		return smart_ptr<character_def>();
	}
	else return it->second;
}

void
CharacterDictionary::add_character(int id, smart_ptr<character_def> c)
{
	//log_msg("CharacterDictionary: add char %d", id);
	_map[id] = c;
	//dump_chars();
}

// Load next chunk of this sprite frames.
// This is possibly better defined in movie_definition
void
movie_def_impl::load_next_frame_chunk()
{

	size_t framecount = get_frame_count();
	size_t lastloaded = get_loading_frame();

	// nothing to do
	if ( lastloaded == framecount ) return;

	size_t nextframe = lastloaded+1;

#if FRAMELOAD_CHUNK
	nextframe += FRAMELOAD_CHUNK; // load in chunks of 10 frames 
	if ( nextframe > framecount ) nextframe = framecount;
#endif
	//log_msg("Framecount: %u, Lastloaded: %u", framecount, lastloaded);
	if ( nextframe <= framecount )
	{
#ifdef DEBUG_FRAMES_LOAD // debugging
		log_msg("Ensure load of frame %u/%u (last loaded is: %u)",
			nextframe, framecount, lastloaded);
#endif
		if ( ! ensure_frame_loaded(nextframe) )
		{
			log_error("Could not advance to frame %d!",
				nextframe);
			// these kind of errors should be handled by callers
			assert(0);
		}
	}
#ifdef DEBUG_FRAMES_LOAD
	else
	{
		log_msg("No more frames to load. Framecount: %u, Lastloaded: %u, next to load: %u", framecount, lastloaded, nextframe);
	}
#endif
}

void
movie_def_impl::read_all_swf()
{
	assert(_str.get() != NULL);

	stream &str = *_str;

	//size_t it=0;
	while ( (uint32_t) str.get_position() < _swf_end_pos )
	{
		// Get exclusive lock on loader, to avoid
		// race conditions with wait_for_frame
		_loader.lock();
	
		//log_msg("Loading thread iteration %u", it++);

		SWF::tag_type tag_type = str.open_tag();

		if (s_progress_function != NULL)
                {
			s_progress_function((uint32_t)str.get_position(),
				_swf_end_pos);
                }

		SWF::TagLoadersTable::loader_function lf = NULL;
		//log_parse("tag_type = %d\n", tag_type);
		if (tag_type == SWF::SHOWFRAME)
		{
			// show frame tag -- advance to the next frame.

			IF_VERBOSE_PARSE(
				log_parse("  show_frame\n");
			);

			++m_loading_frame;

			// signal load of frame
			_loader.signal_frame_loaded(m_loading_frame);

#ifdef DEBUG_FRAMES_LOAD
			log_msg("Loaded frame %u/%u",
				m_loading_frame, m_frame_count);
#endif
		}
		else if (_tag_loaders.get(tag_type, &lf))
                {
			// call the tag loader.  The tag loader should add
			// characters or tags to the movie data structure.
			(*lf)(&str, tag_type, this);
		}
		else
		{
			// no tag loader for this tag type.
				log_error("*** no tag loader for type %d (movie)",
					tag_type);
				dump_tag_bytes(&str);
		} 

		str.close_tag();

		if (tag_type == SWF::END)
                {
			if ((unsigned int) str.get_position() != _swf_end_pos)
                        {
				// Safety break, so we don't read past
				// the end of the  movie.
				log_warning("hit stream-end tag, "
					"but not at the advertised SWF end; "
					"stopping for safety.");
				_loader.unlock();
				break;
			}
		}
		_loader.unlock();
	}

}

} // namespace gnash

