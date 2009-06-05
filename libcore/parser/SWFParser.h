
#ifndef GNASH_SWFPARSER_H
#define GNASH_SWFPARSER_H

#include "swf.h"
#include "TagLoadersTable.h"

namespace gnash {
    class SWFStream;
    class movie_definition;
    class RunInfo;
}

namespace gnash {

class SWFParser
{

public:
    SWFParser(SWFStream& in, movie_definition* md, const RunInfo& runInfo)
        :
        _tagLoaders(SWF::TagLoadersTable::getInstance()),
        _stream(in),
        _md(md),
        _runInfo(runInfo),
        _bytesRead(0),
        _tagOpen(false),
        _endRead(0),
        _nextTagEnd(0)

    {
    }

    size_t bytesRead() const {
        return _bytesRead;
    }

    /// Parse a specified number of bytes from the stream.
    //
    /// This always stops at the end of the stream.
    bool read(std::streamsize bytes);
    
private:

    size_t openTag();

    void closeTag();

    SWF::TagLoadersTable& _tagLoaders;

    SWFStream& _stream;
    
    movie_definition* _md;
    
    const RunInfo& _runInfo;
    
    size_t _bytesRead;
    
    bool _tagOpen;
    
    size_t _endRead;
    
    size_t _nextTagEnd;
    
    SWF::TagType _tag;

};

} // namespace gnash

#endif
