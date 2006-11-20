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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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
#include "bitmap_character_def.h"
#include "swf/TagLoadersTable.h"
#include "movie_root.h"

#include <string>
#include <unistd.h> 

#ifdef HAVE_PTHREADS
#include <pthread.h>
#endif

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

#if defined(_WIN32) || defined(WIN32)
#	include <windows.h>
# define usleep(x) Sleep(x/1000)
#endif

namespace gnash
{

MovieLoader::MovieLoader(movie_def_impl& md)
	:
	_waiting_for_frame(0),
	_movie_def(md)
#ifndef WIN32
	,_thread(0)
#endif
{
#ifdef LOAD_MOVIES_IN_A_SEPARATE_THREAD
	pthread_cond_init(&_frame_reached_condition, NULL);
	pthread_mutex_init(&_mutex, NULL);
#endif

#ifdef WIN32
	_thread.p = 0;
#endif
}

MovieLoader::~MovieLoader()
{
#ifdef LOAD_MOVIES_IN_A_SEPARATE_THREAD
	if ( pthread_cond_destroy(&_frame_reached_condition) != 0 )
	{
		log_error("Error destroying MovieLoader condition");
	}

	if ( pthread_mutex_destroy(&_mutex) != 0 )
	{
		log_error("Error destroying MovieLoader mutex");
	}
#endif
}

bool
MovieLoader::started() const
{
#ifdef WIN32
	return _thread.p != NULL;
#else
	return _thread != 0;
#endif
}

bool
MovieLoader::isSelfThread() const
{
#ifdef WIN32
	return _thread.p == pthread_self().p;
#else
	return pthread_self() == _thread;
#endif
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
#ifndef LOAD_MOVIES_IN_A_SEPARATE_THREAD
	// don't start MovieLoader thread !
	assert(0);
#endif
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
#ifndef LOAD_MOVIES_IN_A_SEPARATE_THREAD
	return; 
#endif
	if (_waiting_for_frame &&
		frameno >= _waiting_for_frame )
	{
		pthread_cond_signal(&_frame_reached_condition);
	}
}

inline void
MovieLoader::lock()
{
#ifndef LOAD_MOVIES_IN_A_SEPARATE_THREAD
	return; // nothing to do as there's no mutex
#endif

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
#ifndef LOAD_MOVIES_IN_A_SEPARATE_THREAD
	return;
#endif

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
#ifndef LOAD_MOVIES_IN_A_SEPARATE_THREAD
	assert(0); // don't call wait_for_frame !
#endif

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
                    boost::intrusive_ptr<resource> res = def->get_exported_resource(inf.m_symbol);
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

	boost::intrusive_ptr<character_def> ch = _dictionary.get_character(character_id);
	assert(ch == NULL || ch->get_ref_count() > 1);
	return ch.get(); // mm... why don't we return the boost::intrusive_ptr?
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

    boost::intrusive_ptr<font>	f;
    m_fonts.get(font_id, &f);
    assert(f == NULL || f->get_ref_count() > 1);
    return f.get();
}

bitmap_character_def* movie_def_impl::get_bitmap_character_def(int character_id)
{
    boost::intrusive_ptr<bitmap_character_def>	ch;
    m_bitmap_characters.get(character_id, &ch);
    assert(ch == NULL || ch->get_ref_count() > 1);
    return ch.get();
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
    boost::intrusive_ptr<sound_sample>	ch;
    m_sound_samples.get(character_id, &ch);
    assert(ch == NULL || ch->get_ref_count() > 1);
    return ch.get();
}

void movie_def_impl::add_sound_sample(int character_id, sound_sample* sam)
{
    assert(sam);
	log_msg("Add sound sample %d", character_id);
    m_sound_samples.add(character_id, sam);
}

// Read header and assign url
bool
movie_def_impl::readHeader(tu_file* in, const std::string& url)
{

	// we only read a movie once 
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

	if ( m_version > 7 )
	{
		log_warning("SWF%d is not fully supported, trying anyway "
			"but don't expect it to work", m_version);
	}

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
		log_parse("frame rate = %f, frames = " SIZET_FMT,
			m_frame_rate, m_frame_count);
	);

	return true;
}

// Fire up the loading thread
bool
movie_def_impl::completeLoad()
{

	// should call this only once
	assert( ! _loader.started() );

	// should call readHeader before this
	assert( _str.get() != NULL );

#ifdef LOAD_MOVIES_IN_A_SEPARATE_THREAD

	// Start the loading frame
	if ( ! _loader.start() )
	{
		log_error("Could not start loading thread");
		return false;
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

// Read a .SWF movie.
bool
movie_def_impl::read(tu_file* in, const std::string& url)
{

	if ( ! readHeader(in, url) ) return false;

	return completeLoad();
}


// 1-based frame number
bool
movie_def_impl::ensure_frame_loaded(size_t framenum)
{
#ifdef LOAD_MOVIES_IN_A_SEPARATE_THREAD 
        //log_msg("Waiting for frame %u to be loaded", framenum);
	_loader.wait_for_frame(framenum);
        //log_msg("Condition reached (m_loading_frame=%u)", m_loading_frame);
#else
	assert ( framenum <= m_loading_frame );
#endif


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

    for (hash<int, boost::intrusive_ptr<font> >::iterator it = m_fonts.begin();
         it != m_fonts.end();
         ++it)
        {
            font*	f = it->second.get();
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
	for (hash<int, boost::intrusive_ptr<character_def> >::iterator it = m_characters.begin();
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

            boost::intrusive_ptr<character_def> ch = _dictionary.get_character(id);
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

#ifdef LOAD_MOVIES_IN_A_SEPARATE_THREAD
	// Guess we want to make sure the loader is started
	// before we create an instance, right ?
	assert (_loader.started());
#endif

	// @@ Shouldn't we return a movie_instance instead ?
	// @@ and leave movie_root creation to the caller ..

    movie_root*	m = new movie_root(this);

    movie_instance* root_movie = new movie_instance(this, m, NULL);

    root_movie->set_name("_root");
    m->set_root_movie(root_movie);

    m->add_ref();

	// I don't think we should be executing actions
	// in first frame from *this* function, rather
	// it should be the advance() function taking
	// care of it... anyway, since we do, better
	// to ensure the frame number 1 is loaded before
	// messing with it.
	ensure_frame_loaded(1);
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
		log_msg("Character %d @ %p", it->first, static_cast<void*>(it->second.get()));
		//character_def* cdef = it->second;
	}
}

boost::intrusive_ptr<character_def>
CharacterDictionary::get_character(int id)
{
	container::iterator it = _map.find(id);
	if ( it == _map.end() )
	{
		IF_VERBOSE_PARSE(
		log_parse("Could not find char %d, dump is:", id);
		dump_chars();
		);
		return boost::intrusive_ptr<character_def>();
	}
	else return it->second;
}

void
CharacterDictionary::add_character(int id, boost::intrusive_ptr<character_def> c)
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
			log_error("Could not advance to frame " SIZET_FMT "!",
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

#ifdef LOAD_MOVIES_IN_A_SEPARATE_THREAD 
	assert( _loader.isSelfThread() );
	assert( _loader.started() );
#else
	assert( ! _loader.started() );
	assert( ! _loader.isSelfThread() );
#endif

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
			IF_VERBOSE_PARSE(
				dump_tag_bytes(&str);
			);
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

void
movie_def_impl::export_resource(const tu_string& symbol, resource* res)
{
	// FIXME: m_exports access should be protected by a mutex

	// SWF sometimes exports the same thing more than once!
	m_exports[symbol] = res;
}


boost::intrusive_ptr<resource>
movie_def_impl::get_exported_resource(const tu_string& symbol)
{
	boost::intrusive_ptr<resource> res;

#ifdef DEBUG_EXPORTS
	log_msg("get_exported_resource called, frame count=%u", m_frame_count);
#endif

	// Don't call get_exported_resource() from this movie loader
	assert( ! _loader.isSelfThread() );

	// this is a simple utility so we don't forget
	// to release our locks...
	struct scoped_loader_locker {
		MovieLoader& _loader;
		scoped_loader_locker(MovieLoader& loader)
			:
			_loader(loader)
		{
			_loader.lock();
		}
		~scoped_loader_locker()
		{
			_loader.unlock();
		}
	};


	// Keep trying until either we found the export or
	// the stream is over, or there is NO frames progress
	// after def_timeout microseconds.
	//
	// Note that the NO frame progress might be due
	// to a circular import chain:
	//
	// 	A imports B imports A
	// 

	// Sleep 1/2 of a second between checks
	// NOTE: make sure the nap is enough time for
	//       thread execution switch !!
	const unsigned long naptime=5e5;

	// Timeout after two seconds of NO frames progress
	const unsigned long def_timeout=2e6/naptime;

	unsigned long timeout=def_timeout;
	size_t loading_frame = (size_t)-1; // used to keep track of advancements
	for (;;)
	{
		// FIXME: make m_exports access thread-safe
		if ( m_exports.get(symbol, &res) )
		{
			return res;
		}

		// FIXME: make get_loading_frame() thread-safe
		size_t new_loading_frame = get_loading_frame();

		if ( new_loading_frame != loading_frame )
		{
			log_msg("frame load advancement (from %lu to %lu)",
				loading_frame, new_loading_frame);
			loading_frame = new_loading_frame;
			timeout = def_timeout;
		}
		else
		{
			if ( ! timeout-- )
			{
				log_warning("No frame progress in movie %s "
					"after %lu milliseconds "
					"(%lu microseconds = %lu iterations), "
					"giving up on "
					"get_exported_resource(%s): "
					"circular IMPORTS?",
					get_url().c_str(),
					(def_timeout*naptime)/1000,
					def_timeout*naptime,
					def_timeout,
					symbol.c_str());
				return res;
			}

			log_warning("no frame progress at iteration %lu", timeout);

			continue; // not worth checking
		}

		if ( loading_frame >= m_frame_count )
		{
			log_msg("At end of stream, still no '%s' symbol found "
				"in m_exports (%u entries in it, follow)",
				symbol.c_str(), m_exports.size());
			return res;
		}

#ifdef DEBUG_EXPORTS
		log_msg("We haven't finished loading (loading frame %u), "
			"and m_exports.get returned no entries, "
			"sleeping a bit and trying again",
			get_loading_frame());
#endif

		usleep(naptime); // take a breath
	}

	return res;
}


} // namespace gnash

