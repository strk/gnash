// morph2.h -- Mike Shaver <shaver@off.net> 2003, , Vitalij Alexeev <tishka92@mail.ru> 2004.

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

#ifndef GNASH_MORPH2_H
#define GNASH_MORPH2_H

#include "smart_ptr.h" // GNASH_USE_GC
#include "shape_character_def.h" // for inheritance of morph2_character_def


namespace gnash {

class SWFStream;

/// DefineMorphShape tag
//
class morph2_character_def : public shape_character_def
{
public:
    morph2_character_def();
    virtual ~morph2_character_def();

    /// Read a DefineMorphShape tag from stream
    //
    /// Throw ParserException if the tag is malformed
    ///
    /// @param in
    ///	The stream to read the definition from.
    ///	Tag type is assumed to have been read already
    ///
    /// @param tag_type
    ///	Type of the tag.
    ///	Need be SWF::DEFINEMORPHSHAPE or an assertion would fail.
    ///	TODO: drop ?
    ///
    /// @param md
    ///	Movie definition. Used to resolv character ids for fill styles.
    ///	Must be not-null or would segfault. 
    ///
    void read(SWFStream& in, int tag_type, movie_definition& m);

    virtual void display(character* inst);
    void lerp_matrix(SWFMatrix& t, const SWFMatrix& m1, const SWFMatrix& m2, const float ratio);

    // Question: What is the bound of a morph? Is this conceptually correct?
    /// TODO: optimize this by take ratio into consideration, to decrease some
    /// invalidated area when rendering morphs
    virtual const rect&	get_bound() const 
    { 
        m_bound.expand_to_rect(m_shape1->m_bound);
        m_bound.expand_to_rect(m_shape2->m_bound);
        return m_bound;
    }

protected:

#ifdef GNASH_USE_GC
/// Mark all reachable resources of a morph2_character_def, for the GC
//
/// Reachable resources are:
///	- The start and end shapes (m_shape1, m_shape2)
///
virtual void markReachableResources() const
{
    if ( m_shape1 ) m_shape1->setReachable();
    if ( m_shape2 ) m_shape2->setReachable();
}
#endif // GNASH_USE_GC

private:
    boost::intrusive_ptr<shape_character_def> m_shape1;
    boost::intrusive_ptr<shape_character_def> m_shape2;
    unsigned int offset;
    int fill_style_count;
    int line_style_count;
    float m_last_ratio;
    mutable rect m_bound;
};
}


#endif // GNASH_MORPH2_H

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
