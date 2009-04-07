// morph2.h -- Mike Shaver <shaver@off.net> 2003, , Vitalij Alexeev <tishka92@mail.ru> 2004.

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

#ifndef GNASH_MORPH2_H
#define GNASH_MORPH2_H

#include "smart_ptr.h" // GNASH_USE_GC
#include "swf.h"
#include "swf/ShapeRecord.h"
#include "character_def.h"

// Forward declarations.
namespace gnash {
    class movie_definition;
    class SWFStream;
    class MorphShape;
}

namespace gnash {

/// DefineMorphShape tag
//
class morph_character_def : public character_def
{
public:

    morph_character_def(SWFStream& in, SWF::TagType tag, movie_definition& md);

    virtual ~morph_character_def() {}

	virtual DisplayObject* createDisplayObject(DisplayObject* parent, int id);

    virtual void display(const MorphShape& inst) const;

    const SWF::ShapeRecord& shape1() const { 
        return _shape1;
    }
    
    const SWF::ShapeRecord& shape2() const { 
        return _shape2;
    }

    virtual const rect& get_bound() const {
        return _bounds;
    }

private:
    
    /// Read a DefineMorphShape tag from stream
    //
    /// Throw ParserException if the tag is malformed
    ///
    /// @param in
    ///	The stream to read the definition from.
    ///	Tag type is assumed to have been read already
    ///
    /// @param TagType
    ///	Type of the tag.
    ///	Need be SWF::DEFINEMORPHSHAPE or an assertion would fail.
    ///	TODO: drop ?
    ///
    /// @param md
    ///	Movie definition. Used to resolv DisplayObject ids for fill styles.
    ///	Must be not-null or would segfault. 
    ///
    void read(SWFStream& in, SWF::TagType tag, movie_definition& m);

    SWF::ShapeRecord _shape1;
    SWF::ShapeRecord _shape2;
    
    rect _bounds;

};

} // namespace gnash


#endif 

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
