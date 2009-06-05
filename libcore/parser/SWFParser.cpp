

#include "SWFStream.h"
#include "movie_definition.h"
#include "RunInfo.h"
#include "SWFParser.h"
#include "TagLoadersTable.h"

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

    while (_stream.tell() != _endRead) {

        try {

            // If a tag hasn't been opened, open one and check
            // how many bytes are needed. The size reported by the
            // tag seems to be the value used, even when it's wrong.
            if (!_tagOpen) {
                _nextTagEnd = openTag();
            }
         
            // Check if we are now supposed to read enough bytes to get to the
            // end of the tag.   
            if (_nextTagEnd > _endRead) {
                return true;
            }

            if (_tag == SWF::END) {
                closeTag();
                return false;
            }

            SWF::TagLoadersTable::loader_function lf = 0;

            if (_tag == SWF::SHOWFRAME) {
                // show frame tag -- advance to the next frame.
                IF_VERBOSE_PARSE(log_parse("SHOWFRAME tag"));
                _md->incrementLoadedFrames();
            }
            else if (_tagLoaders.get(_tag, &lf)) {
                // call the tag loader.  The tag loader should add
                // DisplayObjects or tags to the movie data structure.
                lf(_stream, _tag, *_md, _runInfo);
            }
            else {
                // no tag loader for this tag type.
                log_error(_("*** no tag loader for type %d (movie)"), _tag);
                IF_VERBOSE_PARSE(
                    std::ostringstream ss;
                    dumpTagBytes(_stream, ss);
                    log_error("tag dump follows: %s", ss.str());
                );
            }

        }
        catch (const ParserException& e) {
            // Log and continue parsing...
            log_error(_("Parsing exception: %s"), e.what());
        }

        if (_tagOpen) closeTag();
        _bytesLoaded = _stream.tell();
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
        os << std::left << std::setw(3 * rowlength) << hexify(buf, got, false);
        
        // and once as ASCII
        os << "| " << hexify(buf, got, true) << std::endl;

        toRead -= got;
    }
}

} // anonymous namespace

} // namespace gnash
