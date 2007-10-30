// movie_def_impl.cpp:  Load in Movie definitions, for Gnash.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "movie_def_impl.h"
#include "movie_definition.h" // for inheritance
#include "sprite_instance.h" // for create_instance()
#include "tu_file.h"
#include "zlib_adapter.h"
#include "stream.h"
#include "jpeg.h"
#include "fontlib.h"
#include "font.h"
#include "log.h"
#include "sprite_instance.h"
#include "movie_instance.h"
#include "bitmap_character_def.h"
#include "swf/TagLoadersTable.h"
#include "movie_root.h"
#include "VM.h" // for assertions
#include "GnashException.h" // for parser exception
#include "execute_tag.h"
#include "sound_definition.h" // for sound_sample
#include <boost/bind.hpp>

#include <iomanip>
#include <memory>
#include <string>
#include <unistd.h>


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
	_movie_def(md)
	,_thread(NULL)
{
}

MovieLoader::~MovieLoader()
{
	//cout << "MovieLoader dtor called" << endl;

	// we should assert _movie_def._loadingCanceled
	// but we're not friend yet (anyone introduce us ?)
	if ( _thread.get() )
	{
		//cout << "Joining thread.." << endl;
		_thread->join();
	}
}

bool
MovieLoader::started() const
{
	boost::mutex::scoped_lock lock(_mutex);

	return _thread.get() != NULL;
}

bool
MovieLoader::isSelfThread() const
{
	boost::mutex::scoped_lock lock(_mutex);

	if (!_thread.get()) {
		return false;
	}
	boost::thread this_thread;
	return this_thread == *_thread;
}

void
MovieLoader::execute(movie_def_impl* md)
{
	md->read_all_swf();
}

bool
MovieLoader::start()
{
#ifndef LOAD_MOVIES_IN_A_SEPARATE_THREAD
	// don't start MovieLoader thread !
	abort();
#endif
	// We have two sanity checks, started() and isSelfThread() which rely
	// on boost::thread() returning before they are executed. Therefore,
	// we must employ locking.
	// Those tests do seem a bit redundant, though...
	boost::mutex::scoped_lock lock(_mutex);

	_thread.reset( new boost::thread(boost::bind(execute, &_movie_def)) );

	return true;
}

//
// some utility stuff
//

/// Log the contents of the current tag, in hex to the output strream
static void	dump_tag_bytes(stream* in, ostream& os)
{
    static const int	ROW_BYTES = 16;
    char	row_buf[ROW_BYTES];
    int	row_count = 0;

    row_buf[ROW_BYTES-1] = '\0';

    os << endl;
    while(in->get_position() < in->get_tag_end_position())
    {
            int	c = in->read_u8();
            os << std::hex << std::setw(2) << std::setfill('0') << c << " ";

            if (c < 32 || c > 127 ) c = '.';
            row_buf[row_count] = c;

            row_count++;
            if (row_count >= ROW_BYTES)
            {
                    os << row_buf << endl;
                    row_count = 0;
            }
   }
   if ( row_count )
   {
            row_buf[row_count] = '\0';
            while (row_count++ < ROW_BYTES ) os << "   ";
            os << row_buf << endl;
   }
}


//
// progress callback stuff
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
	// FIXME: use a class-static TagLoadersTable for movie_def_impl
	_tag_loaders(SWF::TagLoadersTable::getInstance()),
	m_create_bitmaps(cbf),
	m_create_font_shapes(cfs),
	m_frame_rate(30.0f),
	m_frame_count(0u),
	m_version(0),
	_frames_loaded(0u),
	_frames_loaded_mutex(),
	_frame_reached_condition(),
	_waiting_for_frame(0),
	m_jpeg_in(0),
	_loader(*this),
	_loadingCanceled(false)
{
}

movie_def_impl::~movie_def_impl()
{
	//cout << "movie_def_impl dtor called" << endl;

	// Request cancelation of the loading thread
	_loadingCanceled = true;

	// Release frame tags
	for (PlayListMap::iterator i=m_playlist.begin(), e=m_playlist.end(); i!=e; ++i)
	{
		PlayList& pl = i->second;

		for (PlayList::iterator j=pl.begin(), je=pl.end(); j!=je; ++j)
		{
                    delete *j;
                }

		//pl.clear(); // useless, as we're going to clean the whole map
        }
	//m_playlist.clear(); // useless, as we're going to call it's destructor next 

	// Release init action data.
	for (PlayListMap::iterator i=m_init_action_list.begin(), e=m_init_action_list.end(); i!=e; ++i)
	{
		PlayList& pl = i->second;

		for (PlayList::iterator j=pl.begin(), je=pl.end(); j!=je; ++j)
		{
                    delete *j;
                }

		//pl.clear(); // useless, as we're going to clean the whole map
        }
	//m_init_action_list.clear(); // useless, as we're going to call it's destructor next 

	// It's supposed to be cleaned up in read()
	// TODO: join with loader thread instead ?
	//assert(m_jpeg_in.get() == NULL);
}

bool movie_def_impl::in_import_table(int character_id) const
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

void movie_def_impl::visit_imported_movies(import_visitor& visitor)
{
    // don't call the visitor twice for a single URL
    std::set<std::string> visited;

    for (size_t i = 0, n = m_imports.size(); i < n; i++)
    {
        const import_info& inf = m_imports[i];
        if (visited.insert(inf.m_source_url).second)
        {
            // Call back the visitor.
            visitor.visit(inf.m_source_url);
        }
    }
}

void movie_def_impl::resolve_import(const std::string& source_url, movie_definition* source_movie)
{

    // Iterate in reverse, since we remove stuff along the way.
    for (size_t i = m_imports.size(); i > 0; i--)
        {
            const import_info&	inf = m_imports[i-1];
            if (inf.m_source_url == source_url)
                {
                    // Do the import.
                    boost::intrusive_ptr<resource> res = source_movie->get_exported_resource(inf.m_symbol);
                    bool	 imported = false;

                    if (res == NULL)
                        {
                            log_error(_("import error: resource '%s' is not exported from movie '%s'"),
                                      inf.m_symbol.c_str(), source_url.c_str());
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
                            log_error(_("import error: resource '%s' from movie '%s' has unknown type"),
                                      inf.m_symbol.c_str(), source_url.c_str());
                        }

                    if (imported)
                        {
				// TODO: a std::list would be faster here
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
            log_error(_("get_character_def(): character_id %d is still waiting to be imported"),
                      character_id);
        }
#endif // not NDEBUG

	boost::intrusive_ptr<character_def> ch = _dictionary.get_character(character_id);
#ifndef GNASH_USE_GC
	assert(ch == NULL || ch->get_ref_count() > 1);
#endif // ndef GNASH_USE_GC
	return ch.get(); // mm... why don't we return the boost::intrusive_ptr?
}

void movie_def_impl::add_font(int font_id, font* f)
{
    assert(f);
    m_fonts.insert(make_pair(font_id, boost::intrusive_ptr<font>(f)));
}

font* movie_def_impl::get_font(int font_id) const
{
#ifndef NDEBUG
    // make sure font_id is resolved
    if (in_import_table(font_id))
        {
            log_error(_("get_font(): font_id %d is still waiting to be imported"),
                      font_id);
        }
#endif // not NDEBUG

    FontMap::const_iterator it = m_fonts.find(font_id);
    if ( it == m_fonts.end() ) return NULL;
    boost::intrusive_ptr<font> f = it->second;
    assert(f->get_ref_count() > 1);
    return f.get();
}

bitmap_character_def* movie_def_impl::get_bitmap_character_def(int character_id)
{
    BitmapMap::iterator it = m_bitmap_characters.find(character_id);
    if ( it == m_bitmap_characters.end() ) return NULL;
    else return it->second.get();
}

void
movie_def_impl::add_bitmap_character_def(int character_id,
		bitmap_character_def* ch)
{
    assert(ch);
    //log_msg(_("Add bitmap character %d"), character_id);
    //m_bitmap_characters.add(character_id, ch);
    m_bitmap_characters.insert(make_pair(character_id, boost::intrusive_ptr<bitmap_character_def>(ch)));

	// we can *NOT* generate bitmap_info until
	// a renderer is present
    add_bitmap_info(ch->get_bitmap_info());
}

sound_sample* movie_def_impl::get_sound_sample(int character_id)
{
    SoundSampleMap::iterator it = m_sound_samples.find(character_id);
    if ( it == m_sound_samples.end() ) return NULL;

    boost::intrusive_ptr<sound_sample> ch = it->second;
#ifndef GNASH_USE_GC
    assert(ch->get_ref_count() > 1);
#endif // ndef GNASH_USE_GC

    return ch.get();
}

void movie_def_impl::add_sound_sample(int character_id, sound_sample* sam)
{
    assert(sam);
    IF_VERBOSE_PARSE(
    log_parse(_("Add sound sample %d assigning id %d"),
		character_id, sam->m_sound_handler_id);
    );
    m_sound_samples.insert(make_pair(character_id,
			    boost::intrusive_ptr<sound_sample>(sam)));
}

// Read header and assign url
bool
movie_def_impl::readHeader(std::auto_ptr<tu_file> in, const std::string& url)
{

	_in = in;

	// we only read a movie once
	assert(_str.get() == NULL);

	if ( url == "" ) _url = "<anonymous>";
	else _url = url;

	uint32_t file_start_pos = _in->get_position();
	uint32_t header = _in->read_le32();
	m_file_length = _in->read_le32();
	_swf_end_pos = file_start_pos + m_file_length;

	m_version = (header >> 24) & 255;
	if ((header & 0x0FFFFFF) != 0x00535746
		&& (header & 0x0FFFFFF) != 0x00535743)
        {
		// ERROR
		log_error(_("gnash::movie_def_impl::read() -- "
			"file does not start with a SWF header"));
		return false;
        }
	bool	compressed = (header & 255) == 'C';

	IF_VERBOSE_PARSE(
		log_parse(_("version = %d, file_length = %d"),
			m_version, m_file_length);
	);

	if ( m_version > 7 )
	{
		log_unimpl(_("SWF%d is not fully supported, trying anyway "
			"but don't expect it to work"), m_version);
	}

	if (compressed)
        {
#ifndef HAVE_ZLIB_H
		log_error(_("movie_def_impl::read(): unable to read "
			"zipped SWF data; gnash was compiled without zlib support"));
		return false;
#else
		IF_VERBOSE_PARSE(
			log_parse(_("file is compressed"));
		);

		// Uncompress the input as we read it.
		_in = zlib_adapter::make_inflater(_in);
#endif
        }

	assert(_in.get());

	_str.reset(new stream(_in.get()));

	m_frame_size.read(_str.get());
	// If the rect is malformed, rect::read would already 
	// print an error. We check again here just to give 
	// the error are better context.
	if ( m_frame_size.is_null() )
	{
		IF_VERBOSE_MALFORMED_SWF(
		log_swferror("non-finite movie bounds");
		);
	}
	m_frame_rate = _str->read_u16() / 256.0f;
	m_frame_count = _str->read_u16();

	/* Markus: Probably this is better anyways */

	// TODO: This seems dangerous, check closely
	if(m_frame_count == 0) m_frame_count++;

	// Allocate 1 more then the expected slots
	// for actions, to make handling malformed SWF easier.
	//m_playlist.resize(m_frame_count+1);
	//m_init_action_list.resize(m_frame_count+1);

	IF_VERBOSE_PARSE(
		m_frame_size.print();
		log_parse(_("frame rate = %f, frames = " SIZET_FMT),
			m_frame_rate, m_frame_count);
	);

	setBytesLoaded(_str->get_position());
	return true;
}

// Fire up the loading thread
bool
movie_def_impl::completeLoad()
{

	// should call this only once
	assert( ! _loader.started() );

	// The VM is needed by the parser
	// to allocate swf_function objects !
	assert ( VM::isInitialized() );

	// should call readHeader before this
	assert( _str.get() != NULL );

#ifdef LOAD_MOVIES_IN_A_SEPARATE_THREAD

	// Start the loading frame
	if ( ! _loader.start() )
	{
		log_error(_("Could not start loading thread"));
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

	return true;
}

// Read a .SWF movie.
bool
movie_def_impl::read(std::auto_ptr<tu_file> in, const std::string& url)
{

	if ( ! readHeader(in, url) ) return false;

	return completeLoad();
}


// 1-based frame number
bool
movie_def_impl::ensure_frame_loaded(size_t framenum)
{
#ifndef LOAD_MOVIES_IN_A_SEPARATE_THREAD
	return ( framenum <= _frames_loaded );
#endif

	boost::mutex::scoped_lock lock(_frames_loaded_mutex);

	if ( framenum <= _frames_loaded ) return true;

	_waiting_for_frame = framenum;
        //log_msg(_("Waiting for frame %u to be loaded"), framenum);

	// TODO: return false on timeout
	_frame_reached_condition.wait(lock);

        //log_msg(_("Condition reached (_frames_loaded=%u)"), _frames_loaded);

	return ( framenum <= _frames_loaded );
}


/* movie_def_impl */
void movie_def_impl::get_owned_fonts(std::vector<font*>* fonts)
    // Fill up *fonts with fonts that we own.
{
    assert(fonts);
    fonts->resize(0);

    std::vector<int>	font_ids;

    for (FontMap::iterator it = m_fonts.begin(), itEnd=m_fonts.end();
         it != itEnd; ++it)
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
            log_error(_("cache file does not have the correct format; skipping"));
            return;
        }
    else if (header[3] != CACHE_FILE_VERSION)
        {
            log_error(
                _("cached data is version %d, but we require version %d; skipping"),
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
                    log_error(_("error reading cache file (characters); skipping"));
                    return;
                }
            if (in->get_eof())
                {
                    log_error(_("unexpected eof reading cache file (characters); skipping"));
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
                    log_error(_("sync error in cache file (reading characters).  "
                              "Skipping rest of cache data."));
                    return;
                }
        }
}

sprite_instance*
movie_def_impl::create_instance()
{
	return create_movie_instance();
}

movie_instance*
movie_def_impl::create_movie_instance()
{

	// @@ Shouldn't we return a movie_instance instead ?
	// @@ and leave movie_root creation to the caller ..

	return new movie_instance(this, NULL);
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
		log_msg(_("Character %d @ %p"), it->first, static_cast<void*>(it->second.get()));
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
		log_parse(_("Could not find char %d, dump is:"), id);
		dump_chars();
		);
		return boost::intrusive_ptr<character_def>();
	}
	else return it->second;
}

void
CharacterDictionary::add_character(int id, boost::intrusive_ptr<character_def> c)
{
	//log_msg(_("CharacterDictionary: add char %d"), id);
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
	//log_msg(_("Framecount: %u, Lastloaded: %u"), framecount, lastloaded);
	if ( nextframe <= framecount )
	{
#ifdef DEBUG_FRAMES_LOAD // debugging
		log_msg(_("Ensure load of frame %u/%u (last loaded is: %u)"),
			nextframe, framecount, lastloaded);
#endif
		if ( ! ensure_frame_loaded(nextframe) )
		{
			log_error(_("Could not advance to frame " SIZET_FMT),
				nextframe);
			// these kind of errors should be handled by callers
			abort();
		}
	}
#ifdef DEBUG_FRAMES_LOAD
	else
	{
		log_msg(_("No more frames to load. Framecount: %u, Lastloaded: %u, next to load: %u"), framecount, lastloaded, nextframe);
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

	try {

	//size_t it=0;
	while ( (uint32_t) str.get_position() < _swf_end_pos )
	{
		if ( _loadingCanceled )
		{
			log_debug("Loading thread cancelation requested, returning from read_all_swf");
			return;
		}

		//log_msg(_("Loading thread iteration %u"), it++);

		SWF::tag_type tag_type = str.open_tag();

parse_tag:

		if (s_progress_function != NULL)
                {
			s_progress_function((uint32_t)str.get_position(),
				_swf_end_pos);
                }

		if (tag_type == SWF::END)
                {
			if ((unsigned int) str.get_position() != _swf_end_pos)
                        {
		    		IF_VERBOSE_MALFORMED_SWF(
				// Safety break, so we don't read past
				// the end of the  movie.
				log_swferror(_("Hit stream-end tag, "
					"but not at the advertised SWF end; "
					"stopping for safety."));
		    		)
		    		break;
			}
		}

		SWF::TagLoadersTable::loader_function lf = NULL;
		//log_parse("tag_type = %d\n", tag_type);
		if (tag_type == SWF::SHOWFRAME)
		{
			// show frame tag -- advance to the next frame.

			IF_VERBOSE_PARSE(
				log_parse("  show_frame");
			);

			incrementLoadedFrames();
			if ( _frames_loaded == m_frame_count )
			{
				str.close_tag();
				tag_type = str.open_tag();
				if ( tag_type != SWF::END )
				{
					IF_VERBOSE_MALFORMED_SWF(
					log_swferror(_("last expected SHOWFRAME "
						"in SWF stream '%s' isn't "
						"followed by an END (%d)."),
						get_url().c_str(), tag_type);
					);
				}
				goto parse_tag;
			}

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
			log_error(_("*** no tag loader for type %d (movie)"),
				tag_type);
			IF_VERBOSE_PARSE(
				stringstream ss;
				dump_tag_bytes(&str, ss);
				log_error("tag dump follows: %s", ss.str().c_str());
			);
		}

		str.close_tag();

		setBytesLoaded(str.get_position());
	}

	} catch (const std::exception& e) {
		// FIXME: we should be setting some variable
		//        so that it is possible for clients
		//        to check the parser status
		//        Also, we should probably call _loader.unlock()
		//        and make sure any wait_for_frame call is
		//        released (condition set and false result)
		log_error(_("Parsing exception: %s"), e.what());
	}

	if ( ! m_playlist[_frames_loaded].empty() || ! m_init_action_list[_frames_loaded].empty() )
	{
		IF_VERBOSE_MALFORMED_SWF(
		log_swferror(_(SIZET_FMT " action blocks and " SIZET_FMT " init action blocks are NOT followed by"
			" a SHOWFRAME tag"),
			m_playlist[_frames_loaded].size(),
			m_init_action_list[_frames_loaded].size());
		);
	}

	if ( m_frame_count > _frames_loaded )
	{
		IF_VERBOSE_MALFORMED_SWF(
		log_swferror(_(SIZET_FMT " frames advertised in header, but only " SIZET_FMT " SHOWFRAME tags "
			"found in stream. Updating total frames count"), m_frame_count, _frames_loaded);
		);
		boost::mutex::scoped_lock lock(_frames_loaded_mutex);
		m_frame_count = _frames_loaded;
		// Notify any thread waiting on frame reached condition
		_frame_reached_condition.notify_all();
	}
}

size_t
movie_def_impl::get_loading_frame() const
{
	boost::mutex::scoped_lock lock(_frames_loaded_mutex);
	return _frames_loaded;
}

void
movie_def_impl::incrementLoadedFrames()
{
	boost::mutex::scoped_lock lock(_frames_loaded_mutex);

	++_frames_loaded;

	// Close current frame definition in Timeline object
	_timeline.closeFrame();

	if ( _frames_loaded > m_frame_count )
	{
		IF_VERBOSE_MALFORMED_SWF(
			log_swferror(_("number of SHOWFRAME tags "
				"in SWF stream '%s' (" SIZET_FMT
				") exceeds "
				"the advertised number in header ("
			        SIZET_FMT ")."),
				get_url().c_str(), _frames_loaded,
				m_frame_count);
		);
		//m_playlist.resize(_frames_loaded+1);
		//m_init_action_list.resize(_frames_loaded+1);
	}

#ifdef DEBUG_FRAMES_LOAD
	log_msg(_("Loaded frame %u/%u"),
		_frames_loaded, m_frame_count);
#endif

	// signal load of frame if anyone requested it
	// FIXME: _waiting_form_frame needs mutex ?
	if (_waiting_for_frame && _frames_loaded >= _waiting_for_frame )
	{
		// or should we notify_one ?
		// See: http://boost.org/doc/html/condition.html
		_frame_reached_condition.notify_all();
	}
}

void
movie_def_impl::export_resource(const std::string& symbol, resource* res)
{
	// FIXME: m_exports access should be protected by a mutex

	// SWF sometimes exports the same thing more than once!
	m_exports[symbol] = res;
}


boost::intrusive_ptr<resource>
movie_def_impl::get_exported_resource(const std::string& symbol)
{
	boost::intrusive_ptr<resource> res;

#ifdef DEBUG_EXPORTS
	log_msg(_("get_exported_resource called, frame count=%u"), m_frame_count);
#endif

	// Don't call get_exported_resource() from this movie loader
	assert( ! _loader.isSelfThread() );

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
	const unsigned long naptime=500000;

	// Timeout after two seconds of NO frames progress
	const unsigned long def_timeout=2000000/naptime;

	unsigned long timeout=def_timeout;
	size_t loading_frame = (size_t)-1; // used to keep track of advancements
	for (;;)
	{
		// FIXME: make m_exports access thread-safe
        ExportMap::iterator it = m_exports.find(symbol);
        if ( it != m_exports.end() ) return it->second;

		size_t new_loading_frame = get_loading_frame();

		if ( new_loading_frame != loading_frame )
		{
			log_msg(_("frame load advancement (from "
				SIZET_FMT " to " SIZET_FMT ")"),
				loading_frame, new_loading_frame);
			loading_frame = new_loading_frame;
			timeout = def_timeout;
		}
		else
		{
			if ( ! timeout-- )
			{
				log_error(_("No frame progress in movie %s "
					"after %lu milliseconds "
					"(%lu microseconds = %lu iterations), "
					"giving up on "
					"get_exported_resource(%s): "
					"circular IMPORTS?"),
					get_url().c_str(),
					(def_timeout*naptime)/1000,
					def_timeout*naptime,
					def_timeout,
					symbol.c_str());
				return res;
			}

			log_error(_("no frame progress at iteration %lu"), timeout);

			continue; // not worth checking
		}

		if ( loading_frame >= m_frame_count )
		{
			log_error(_("At end of stream, still no '%s' symbol found "
				"in m_exports (" SIZET_FMT " entries in it, "
				"follow)"), symbol.c_str(), m_exports.size());
			return res;
		}

#ifdef DEBUG_EXPORTS
		log_msg(_("We haven't finished loading (loading frame %u), "
			"and m_exports.get returned no entries, "
			"sleeping a bit and trying again"),
			get_loading_frame());
#endif

		usleep(naptime); // take a breath
	}

	return res;
}

void
movie_def_impl::add_frame_name(const std::string& n)
{
	boost::mutex::scoped_lock lock(_namedFramesMutex);
	//log_debug(_("labelframe: frame %d, name %s"), _frames_loaded, n.c_str());
	_namedFrames[n] = _frames_loaded;
}

bool
movie_def_impl::get_labeled_frame(const std::string& label, size_t& frame_number)
{
    boost::mutex::scoped_lock lock(_namedFramesMutex);
    NamedFrameMap::iterator it = _namedFrames.find(label);
    if ( it == _namedFrames.end() ) return false;
    frame_number = it->second;
    return true;
}

#ifdef GNASH_USE_GC
void
movie_def_impl::markReachableResources() const
{
	for (FontMap::const_iterator i=m_fonts.begin(), e=m_fonts.end(); i!=e; ++i)
	{
		i->second->setReachable();
	}

	for (BitmapMap::const_iterator i=m_bitmap_characters.begin(), e=m_bitmap_characters.end(); i!=e; ++i)
	{
		i->second->setReachable();
	}

	for (BitmapVect::const_iterator i=m_bitmap_list.begin(), e=m_bitmap_list.end(); i!=e; ++i)
	{
		(*i)->setReachable();
	}

	for (SoundSampleMap::const_iterator i=m_sound_samples.begin(), e=m_sound_samples.end(); i!=e; ++i)
	{
		i->second->setReachable();
	}

	for (ExportMap::const_iterator i=m_exports.begin(), e=m_exports.end(); i!=e; ++i)
	{
		i->second->setReachable();
	}

	for (ImportVect::const_iterator i=m_import_source_movies.begin(), e=m_import_source_movies.end(); i!=e; ++i)
	{
		(*i)->setReachable();
	}

	_dictionary.markReachableResources();

}
#endif // GNASH_USE_GC

void
movie_def_impl::add_init_action(execute_tag* e, int cid)
{
	assert(e);
	if ( m_init_action_defined.insert(cid).second )
	{
		m_init_action_list[_frames_loaded].push_back(e);
	}
}

} // namespace gnash
