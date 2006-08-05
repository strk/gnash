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

#include <iostream>
#include <string>

#include "movie_def_impl.h"
#include "movie_definition.h" // for inheritance
#include "tu_file.h"
#include "zlib_adapter.h"
#include "stream.h"
#include "jpeg.h"
#include "fontlib.h"
#include "font.h"
#include "log.h"
//#include "Sprite.h"
#include "sprite_instance.h"
#include "render.h"
#include "bitmap_character_def.h"
#include "smart_ptr.h"
#include "swf/TagLoadersTable.h"
#include "execute_tag.h"
#include "movie_root.h"

using namespace std;

// Increment this when the cache data format changes.
#define CACHE_FILE_VERSION 4

namespace gnash
{

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

movie_def_impl::~movie_def_impl()
{
    // Release our playlist data.
    {for (int i = 0, n = m_playlist.size(); i < n; i++)
        {
            for (int j = 0, m = m_playlist[i].size(); j < m; j++)
                {
                    delete m_playlist[i][j];
                }
        }}

    // Release init action data.
    {for (size_t i = 0, n = m_init_action_list.size(); i < n; i++)
        {
            for (size_t j = 0, m = m_init_action_list[i].size(); j < m; j++)
                {
                    delete m_init_action_list[i][j];
                }
        }}

    assert(m_jpeg_in == NULL);	// It's supposed to be cleaned up in read()
}

bool movie_def_impl::in_import_table(int character_id)
{
    for (int i = 0, n = m_imports.size(); i < n; i++)
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

    for (int i = 0, n = m_imports.size(); i < n; i++)
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
    for (int i = m_imports.size() - 1; i >= 0; i--)
        {
            const import_info&	inf = m_imports[i];
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

bitmap_character_def* movie_def_impl::get_bitmap_character(int character_id)
{
    smart_ptr<bitmap_character_def>	ch;
    m_bitmap_characters.get(character_id, &ch);
    assert(ch == NULL || ch->get_ref_count() > 1);
    return ch.get_ptr();
}

void movie_def_impl::add_bitmap_character(int character_id, bitmap_character_def* ch)
{
    assert(ch);
    //log_msg("Add bitmap character %d", character_id);
    m_bitmap_characters.add(character_id, ch);

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
    
	log_parse("version = %d, file_length = %d\n", m_version, m_file_length);

	tu_file* original_in = NULL;
	if (compressed)
        {
#if TU_CONFIG_LINK_TO_ZLIB == 0
		log_error("movie_def_impl::read(): unable to read "
			"zipped SWF data; TU_CONFIG_LINK_TO_ZLIB is 0\n");
		return false;
#endif

		log_parse("file is compressed.\n");
		original_in = in;

		// Uncompress the input as we read it.
		_zlib_file.reset(zlib_adapter::make_inflater(original_in));
		in = _zlib_file.get();

		// Subtract the size of the 8-byte header, since
		// it's not included in the compressed
		// stream length.
		_swf_end_pos = m_file_length - 8;
        }

	//stream str(in);
	_str.reset(new stream(in));

	m_frame_size.read(_str.get());
	m_frame_rate = _str->read_u16() / 256.0f;
	m_frame_count = _str->read_u16();

	// hack
	// Vitaly: I am not assured that it correctly
	m_frame_count = (m_frame_count == 0) ? 1 : m_frame_count;

	m_playlist.resize(m_frame_count);
	m_init_action_list.resize(m_frame_count);

	if (dbglogfile.getParserDump()) {
		m_frame_size.print();
	}
	log_parse("frame rate = %f, frames = %d\n",
		m_frame_rate, m_frame_count);

#if 0
	size_t startup_frames = m_frame_count;
#else
	// Don't load any frame 
	// Other parts of the code will need to call ensure_frame_loaded(#)
	// whenever in need to access a new frame
	size_t startup_frames = 1; // always load first frame (must try w/out)
#endif
	if ( startup_frames && ! ensure_frame_loaded(startup_frames) )
	{
		log_error("Could not load to frame %u !", startup_frames);
		return false;
	}

	if (m_jpeg_in)
        {
            delete m_jpeg_in;
            m_jpeg_in = NULL;
        }

// automatically deleted at movie_def_impl removal, can't delete here
// as we will keep reading from it while playing
#if 0
	if (original_in)
        {
            // Done with the zlib_adapter.
            delete in;
        }
#endif

	return true;
}


// 0-based frame number
bool
movie_def_impl::ensure_frame_loaded(size_t framenum)
{
	assert(_str.get() != NULL);

	// Don't ask me to load more then available frames !
	// (this might be due to malformed SWF (header reporting
	//  less frames then available - still, check for it should
	//  be implemented in tag loaders (possibly action executors).
	assert(framenum <= m_frame_count);

	// We already loaded that frame...
	// (could turn into an assertion directly)
	if ( framenum <= m_loading_frame )
	{
		log_msg("Frame %u already loaded (we loaded %u/%u)",
			framenum, m_loading_frame, m_frame_count);

		// we make this an assertion to catch callers
		// that might check for this condition themself
		// rather then rely on this function.
		assert(0);

		return true;
	}
#if 0 // debugging
	else
	{
		log_msg("Loading of frame %u requested (we are at %u/%u)",
			framenum, m_loading_frame, m_frame_count);
	}
#endif

	stream& str=*_str;
	// when m_loading_frame is 1 we've read frame 1
	while ( m_loading_frame < framenum )
// (uint32_t) str.get_position() < _swf_end_pos)
	{
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
			log_parse("  show_frame\n");
			++m_loading_frame;
#if 0 // debugging
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
			log_parse("*** no tag loader for type %d\n", tag_type);
			if (dbglogfile.getParserDump()) {
				dump_tag_bytes(&str);
			}
		} 

		str.close_tag();

		if (tag_type == SWF::END)
                {
			if ( m_loading_frame < framenum ) {
				log_warning("hit SWF::END before reaching "
					"requested frame number %u",
					framenum);
				_loaded_bytes = str.get_position();
				return false;
			}

			if ((unsigned int) str.get_position() != _swf_end_pos)
                        {
				// Safety break, so we don't read past
				// the end of the  movie.
				log_warning("hit stream-end tag, "
					"but not at the advertised SWF end; "
					"stopping for safety.");
				break;
			}
		}
	}

	size_t cur_pos = (size_t)str.get_position();
	//assert( _loaded_bytes < cur_pos )
	if ( cur_pos < _loaded_bytes )
	{
		log_warning("After load of frame %u:\n"
			" current stream position: %u\n"
			" _loaded_bytes=%u\n"
			"(some tag triggered a seek-back?!)",
			m_loading_frame,
			cur_pos,
			_loaded_bytes);
	}
	else
	{
		_loaded_bytes = cur_pos;
	}

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

    sprite_instance* root_movie = new sprite_instance(this, m,
                                                      NULL, -1);
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
		log_msg("Character %d @ %p", it->first, it->second.get_ptr());
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

} // namespace gnash

