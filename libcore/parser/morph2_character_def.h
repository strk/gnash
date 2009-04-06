// morph2.h -- Mike Shaver <shaver@off.net> 2003, , Vitalij Alexeev <tishka92@mail.ru> 2004.

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

#ifndef GNASH_MORPH2_H
#define GNASH_MORPH2_H

#include "smart_ptr.h" // GNASH_USE_GC
#include "shape_character_def.h" 
#include "swf.h"

// Forward declarations.
namespace gnash {
    class SWFStream;
    class MorphShape;
}

namespace gnash {

/// DefineMorphShape tag
//
class morph_character_def : public character_def
{
public:

    morph_character_def();

	virtual DisplayObject* createDisplayObject(DisplayObject* parent, int id);
    
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

    virtual void display(const MorphShape& inst);

    const shape_character_def& shape1() const { 
        return *_shape1;
    }
    
    const shape_character_def& shape2() const { 
        return *_shape2;
    }

    virtual const rect& get_bound() const {
        return _bounds;
    }

protected:

#ifdef GNASH_USE_GC
    /// Reachable resources are:
    ///	- The start and end shapes (m_shape1, m_shape2)
    virtual void markReachableResources() const
    {
        if (_shape1) _shape1->setReachable();
        if (_shape2) _shape2->setReachable();
    }
#endif 

private:

    boost::intrusive_ptr<shape_character_def> _shape1;
    boost::intrusive_ptr<shape_character_def> _shape2;
    
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
