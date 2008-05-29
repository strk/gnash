// movie_def_impl.cpp:  Load in Movie definitions, for Gnash.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "smart_ptr.h" // GNASH_USE_GC
#include "movie_def_impl.h"
#include "movie_definition.h" // for inheritance
#include "sprite_instance.h" // for ??
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
#include "ControlTag.h"
#include "sound_definition.h" // for sound_sample
#include <boost/bind.hpp>
#include <boost/version.hpp>

#include <iomanip>
#include <memory>
#include <string>
#include <algorithm> // std::make_pair
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
	_movie_def(md),
	_thread(NULL),
	_barrier(2) // us and the main thread..
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
#if BOOST_VERSION < 103500
	boost::thread this_thread;
	return this_thread == *_thread;
#else
	return boost::this_thread::get_id() == _thread->get_id();
#endif

}

// static..
void
MovieLoader::execute(MovieLoader& ml, movie_def_impl* md)
{
	ml._barrier.wait(); // let _thread assignment happen before going on
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

	_thread.reset( new boost::thread(boost::bind(execute, boost::ref(*this), &_movie_def)) );

	_barrier.wait(); // let execution start befor returning

	return true;
}

//
// some utility stuff
//

/// Log the contents of the current tag, in hex to the output strream
static void	dump_tag_bytes(stream* in, std::ostream& os)
{
    static const int	ROW_BYTES = 16;
    char	row_buf[ROW_BYTES];
    int	row_count = 0;

    row_buf[ROW_BYTES-1] = '\0';

    os << std::endl;
    while(in->get_position() < in->get_tag_end_position())
    {
            int	c = in->read_u8();
            os << std::hex << std::setw(2) << std::setfill('0') << c << " ";

            if (c < 32 || c > 127 ) c = '.';
            row_buf[row_count] = c;

            row_count++;
            if (row_count >= ROW_BYTES)
            {
                    os << row_buf << std::endl;
                    row_count = 0;
            }
   }
   if ( row_count )
   {
            row_buf[row_count] = '\0';
            while (row_count++ < ROW_BYTES ) os << "   ";
            os << row_buf << std::endl;
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

movie_def_impl::movie_def_impl()
	:
	// FIXME: use a class-static TagLoadersTable for movie_def_impl
	_tag_loaders(SWF::TagLoadersTable::getInstance()),
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
        }

	// It's supposed to be cleaned up in read()
	// TODO: join with loader thread instead ?
	//assert(m_jpeg_in.get() == NULL);
}

void movie_def_impl::add_character(int character_id, character_def* c)
{
	assert(c);
	boost::mutex::scoped_lock lock(_dictionaryMutex);
	_dictionary.add_character(character_id, c);
}

character_def*
movie_def_impl::get_character_def(int character_id)
{

	boost::mutex::scoped_lock lock(_dictionaryMutex);

	boost::intrusive_ptr<character_def> ch = _dictionary.get_character(character_id);
#ifndef GNASH_USE_GC
	assert(ch == NULL || ch->get_ref_count() > 1);
#endif // ndef GNASH_USE_GC
	return ch.get(); // mm... why don't we return the boost::intrusive_ptr?
}

void movie_def_impl::add_font(int font_id, font* f)
{
    assert(f);
    m_fonts.insert(std::make_pair(font_id, boost::intrusive_ptr<font>(f)));
}

font* movie_def_impl::get_font(int font_id) const
{

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
    //log_debug(_("Add bitmap character %d"), character_id);
    //m_bitmap_characters.add(character_id, ch);
    m_bitmap_characters.insert(std::make_pair(character_id, boost::intrusive_ptr<bitmap_character_def>(ch)));

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
    m_sound_samples.insert(std::make_pair(character_id,
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

	boost::uint32_t file_start_pos = _in->get_position();
	boost::uint32_t header = _in->read_le32();
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

	// It seems frame rate is limited to a max 
	// 84 was found by testing the pp, might be turned into a compile-time define
	static const int maxfps = 84;
	m_frame_rate = _str->read_u16();
	if ( ! m_frame_rate )
	{
		log_debug("Frame rate of 0 taken as %d (upper bound)", maxfps);
		m_frame_rate = maxfps;
	}
	else
	{
		m_frame_rate /= 256.0f;
		if ( m_frame_rate > maxfps )
		{
			log_debug("Frame rate of %d too high, we'll use %d (upper bound)",
				m_frame_rate, maxfps);
			m_frame_rate = maxfps;
		}
	}

	m_frame_count = _str->read_u16();

	/* Markus: Probably this is better anyways */

	// TODO: This seems dangerous, check closely
	if(m_frame_count == 0) m_frame_count++;

	IF_VERBOSE_PARSE(
		m_frame_size.print();
		log_parse(_("frame rate = %f, frames = %d"),
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
	boost::mutex::scoped_lock lock(_frames_loaded_mutex);

#ifndef LOAD_MOVIES_IN_A_SEPARATE_THREAD
	return ( framenum <= _frames_loaded );
#endif

	if ( framenum <= _frames_loaded ) return true;

	_waiting_for_frame = framenum;
        //log_debug(_("Waiting for frame %u to be loaded"), framenum);

	// TODO: return false on timeout
	_frame_reached_condition.wait(lock);

        //log_debug(_("Condition reached (_frames_loaded=%u)"), _frames_loaded);

	return ( framenum <= _frames_loaded );
}

movie_instance*
movie_def_impl::create_movie_instance(character* parent)
{
	return new movie_instance(this, parent);
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
		log_debug(_("Character %d @ %p"), it->first, static_cast<void*>(it->second.get()));
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
	//log_debug(_("CharacterDictionary: add char %d"), id);
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
	//log_debug(_("Framecount: %u, Lastloaded: %u"), framecount, lastloaded);
	if ( nextframe <= framecount )
	{
#ifdef DEBUG_FRAMES_LOAD // debugging
		log_debug(_("Ensure load of frame %u/%u (last loaded is: %u)"),
			nextframe, framecount, lastloaded);
#endif
		if ( ! ensure_frame_loaded(nextframe) )
		{
			log_error(_("Could not advance to frame %d"),
				nextframe);
			// these kind of errors should be handled by callers
			abort();
		}
	}
#ifdef DEBUG_FRAMES_LOAD
	else
	{
		log_debug(_("No more frames to load. Framecount: %u, Lastloaded: %u, next to load: %u"), framecount, lastloaded, nextframe);
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
	while ( (boost::uint32_t) str.get_position() < _swf_end_pos )
	{
		if ( _loadingCanceled )
		{
			log_debug("Loading thread cancelation requested, returning from read_all_swf");
			return;
		}

		//log_debug(_("Loading thread iteration %u"), it++);

		SWF::tag_type tag_type = str.open_tag();

parse_tag:

		if (s_progress_function != NULL)
                {
			s_progress_function((boost::uint32_t)str.get_position(),
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

			size_t floaded = incrementLoadedFrames();
			if ( floaded == m_frame_count )
			{
				str.close_tag();
				tag_type = str.open_tag();
				if ( tag_type != SWF::END )
				{
					IF_VERBOSE_MALFORMED_SWF(
					log_swferror(_("last expected SHOWFRAME "
						"in SWF stream '%s' isn't "
						"followed by an END (%d)."),
						get_url(), tag_type);
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
				std::stringstream ss;
				dump_tag_bytes(&str, ss);
				log_error("tag dump follows: %s", ss.str());
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

	size_t floaded = get_loading_frame();
	if ( ! m_playlist[floaded].empty() )
	{
		IF_VERBOSE_MALFORMED_SWF(
		log_swferror(_("%d control tags are NOT followed by"
			" a SHOWFRAME tag"),
			m_playlist[floaded].size());
		);
	}

	if ( m_frame_count > floaded )
	{
		IF_VERBOSE_MALFORMED_SWF(
		log_swferror(_("%d frames advertised in header, but only %d SHOWFRAME tags "
			"found in stream. Pretending we loaded all advertised frames"), m_frame_count, floaded);
		);
		boost::mutex::scoped_lock lock(_frames_loaded_mutex);
		_frames_loaded = m_frame_count;
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

size_t
movie_def_impl::incrementLoadedFrames()
{
	boost::mutex::scoped_lock lock(_frames_loaded_mutex);

	++_frames_loaded;

	if ( _frames_loaded > m_frame_count )
	{
		IF_VERBOSE_MALFORMED_SWF(
			log_swferror(_("number of SHOWFRAME tags "
				"in SWF stream '%s' (%d) exceeds "
				"the advertised number in header (%d)."),
				get_url(), _frames_loaded,
				m_frame_count);
		);
		//m_playlist.resize(_frames_loaded+1);
	}

#ifdef DEBUG_FRAMES_LOAD
	log_debug(_("Loaded frame %u/%u"),
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

	return _frames_loaded;
}

void
movie_def_impl::export_resource(const std::string& symbol, resource* res)
{
	// _exportedResources access should be protected by a mutex
	boost::mutex::scoped_lock lock(_exportedResourcesMutex);

	// SWF sometimes exports the same thing more than once!
	_exportedResources[symbol] = res;
}


boost::intrusive_ptr<resource>
movie_def_impl::get_exported_resource(const std::string& symbol)
{
	boost::intrusive_ptr<resource> res;

#ifdef DEBUG_EXPORTS
	log_debug(_("get_exported_resource called, frame count=%u"), m_frame_count);
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
		// _exportedResources access is thread-safe
		{
			boost::mutex::scoped_lock lock(_exportedResourcesMutex);
			ExportMap::iterator it = _exportedResources.find(symbol);
			if ( it != _exportedResources.end() ) return it->second;
		}

		size_t new_loading_frame = get_loading_frame();

		if ( new_loading_frame != loading_frame )
		{
#ifdef DEBUG_EXPORTS
			log_debug(_("looking for exported resource: frame load "
						"advancement (from %d to %d)"),
				loading_frame, new_loading_frame);
#endif
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
					get_url(),
					(def_timeout*naptime)/1000,
					def_timeout*naptime,
					def_timeout,
					symbol);
				return res;
			}

#ifdef DEBUG_EXPORTS
			log_debug(_("No frame progress at iteration %lu of get_exported_resource(%s)"),
				timeout, symbol);
#endif

			continue; // not worth checking
		}

		if ( loading_frame >= m_frame_count )
		{
#ifdef DEBUG_EXPORTS
			boost::mutex::scoped_lock lock(_exportedResourcesMutex);
			log_debug(_("At end of stream, still no '%s' symbol found "
				"in _exportedResources (%d entries in it, "
				"follow)"), symbol, _exportedResources.size());
			for (ExportMap::const_iterator it=_exportedResources.begin(); it!=_exportedResources.end(); ++it)
			{
				log_debug(" symbol %s (%s)", it->first, typeName(*(it->second)));
			}
#endif
			return res;
		}

#ifdef DEBUG_EXPORTS
		log_debug(_("We haven't finished loading (loading frame %u), "
			"and _exportedResources.get returned no entries, "
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
	boost::mutex::scoped_lock lock1(_namedFramesMutex);
	boost::mutex::scoped_lock lock2(_frames_loaded_mutex);

	_namedFrames.insert(std::make_pair(n, _frames_loaded));
}

bool
movie_def_impl::get_labeled_frame(const std::string& label, size_t& frame_number)
{
    boost::mutex::scoped_lock lock(_namedFramesMutex);
    NamedFrameMap::const_iterator it = _namedFrames.find(label);
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

	// TODO: turn this into a markExportedResources()
	{
		boost::mutex::scoped_lock lock(_exportedResourcesMutex);
		for (ExportMap::const_iterator i=_exportedResources.begin(), e=_exportedResources.end(); i!=e; ++i)
		{
			i->second->setReachable();
		}
	}

	for (ImportVect::const_iterator i=m_import_source_movies.begin(), e=m_import_source_movies.end(); i!=e; ++i)
	{
		(*i)->setReachable();
	}

	boost::mutex::scoped_lock lock(_dictionaryMutex);
	_dictionary.markReachableResources();

}
#endif // GNASH_USE_GC

} // namespace gnash
