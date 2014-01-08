// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#include "SWFStream.h"
#include "movie_definition.h"
#include "RunResources.h"
#include "SWFParser.h"
#include "TagLoadersTable.h"
#include "log.h"

#include <iomanip>

namespace gnash {

// Forward declarations
namespace {
    void dumpTagBytes(SWFStream& in, std::ostream& os);
}

size_t
SWFParser::openTag()
{
    _tag = _stream.open_tag();
    _tagOpen = true;
    return _stream.get_tag_end_position();
}

void
SWFParser::closeTag()
{
    _stream.close_tag();
    _tagOpen = false;
}

bool
SWFParser::read(std::streamsize bytes)
{

    // If we didn't use all the bytes given to us last time,
    // we may read more than the size passed.
    _endRead += bytes;

    const SWF::TagLoadersTable& tagLoaders = _runResources.tagLoaders();

    while (_bytesRead < _endRead) {
        
        const size_t startPos = _stream.tell();
        
        // If a tag hasn't been opened, open one and check
        // how many bytes are needed. The size reported by the
        // tag seems to be the value used, even when it's wrong.
        if (!_tagOpen) {
            _nextTagEnd = openTag() - startPos;
        }

        try {

            // Check if we are now supposed to read enough bytes to get to the
            // end of the tag.   
            if (_nextTagEnd > _endRead) {
                return true;
            }

            // Signal that we have reached the end of a SWF or sprite when
            // a SWF::END tag is encountered.
            if (_tag == SWF::END) {
                closeTag();
                return false;
            }

            SWF::TagLoadersTable::TagLoader lf = 0;

            if (_tag == SWF::SHOWFRAME) {
                // show frame tag -- advance to the next frame.
                IF_VERBOSE_PARSE(log_parse(_("SHOWFRAME tag")));
                _md->incrementLoadedFrames();
            }
            else if (tagLoaders.get(_tag, lf)) {
                // call the tag loader.  The tag loader should add
                // DisplayObjects or tags to the movie data structure.
                lf(_stream, _tag, *_md, _runResources);
            }
            else {
                // no tag loader for this tag type.
                log_error(_("Encountered unknown tag %d. These usually store "
                        "creation tool data and do not affect playback"), _tag);
                IF_VERBOSE_PARSE(
                    std::ostringstream ss;
                    dumpTagBytes(_stream, ss);
                    log_error(_("tag dump follows: %s"), ss.str());
                );
            }

        }
        catch (const ParserException& e) {
            // If the error occurred in a tag, we continue parsing so that
            // single malformed tags don't prevent reading subsequent tags.
            log_error(_("Parsing exception: %s"), e.what());
        }

        if (_tagOpen) closeTag();
        _bytesRead += (_stream.tell() - startPos);
    }

    return true;

}

namespace {

/// Log the contents of the current tag, in hex to the output strream
void 
dumpTagBytes(SWFStream& in, std::ostream& os)
{
    const std::streamsize rowlength = 16;
    os << std::endl;
    
    const std::streamsize end = in.get_tag_end_position();

    // This is decremented until we reach the end of the stream.
    std::streamsize toRead = end - in.tell();
    in.ensureBytes(toRead);

    unsigned char buf[rowlength];    
    while (toRead)
    {
        // Read in max row length or remainder of stream.
        const std::streamsize thisRow = 
            std::min<std::streamsize>(toRead, rowlength);
        
        const std::streamsize got = 
            in.read(reinterpret_cast<char*>(&buf), thisRow);
        
        // Check that we read all the bytes we expected.
        if (got < thisRow) {
            throw ParserException(_("Unexpected end of stream while reading"));
        }
        
        // Stream once as hex
        std::ios::fmtflags f(os.flags());
        os << std::left << std::setw(3 * rowlength) << hexify(buf, got, false);
        
        // and once as ASCII
        os << "| " << hexify(buf, got, true) << std::endl;
        os.flags(f);

        toRead -= got;
    }
}

} // anonymous namespace

} // namespace gnash
