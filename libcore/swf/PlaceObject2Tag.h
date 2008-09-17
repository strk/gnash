// 
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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


#ifndef GNASH_SWF_PLACEOBJECT2TAG_H
#define GNASH_SWF_PLACEOBJECT2TAG_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "DisplayListTag.h" // for inheritance
#include "swf.h" // for tag_type definition
#include "matrix.h" // for composition
#include "cxform.h" // for composition 

#include <vector>

// Forward declarations
namespace gnash {
    class SWFStream;
    class sprite_instance;
    class swf_event;
    class action_buffer;
    class movie_definition;
	class DisplayList;
}

namespace gnash {
namespace SWF {

/// SWF Tag PlaceObject (4) or PlaceObject2 (9) 
//
/// This tag is owned by the movie_definiton class
///
/// The PlaceObject tags can be used to:
/// - Place a character to a depth. See isPlace().
/// - Transform the character placed at a depth. See isMove().
/// - Replace a character at a depth. See isReplace().
///
/// In any case a single Timeline depth is affected.
/// Postcondition of this tag execution is presence of an instance
/// at the affected depth. See getDepth().
///
///
/// m_character_id:
/// The ID of the character to be added.
/// It will be seeked in the CharacterDictionary.
///
/// m_name:
/// The name to give to the newly created instance if m_has_name is true.
/// If m_has_name is false, the new instance will be assigned a sequential
/// name in the form 'instanceN', where N is incremented
/// at each call, starting from 1.
///
/// event_handlers
///
/// m_depth:
/// The depth to assign to the newly created instance.
///
/// m_color_transform:
/// The color transform to apply to the newly created instance.
///
/// m_matrix:
/// The matrix transform to apply to the newly created instance.
///
/// m_ratio
///
/// m_clip_depth:
/// If != character::noClipDepthValue, mark the created instance
/// as a clipping layer. The shape of the placed character will be
/// used as a mask for all higher depths up to this value.
///
class PlaceObject2Tag : public DisplayListTag
{
public:

    typedef std::vector<action_buffer*> ActionBuffers;
    typedef std::vector<swf_event*> EventHandlers;

    PlaceObject2Tag(const movie_definition& def)
        :
        DisplayListTag(0), // why is it 0 here and -1 for RemoveObjectTag ??
        m_tag_type(0),
        m_has_flags2(0),
        m_has_flags3(0),
        m_character_id(0),
        m_ratio(0),
        m_name(""),
        m_clip_depth(0),
        _movie_def(def)
    {
    }

    ~PlaceObject2Tag();

    /// Read SWF::PLACEOBJECT or SWF::PLACEOBJECT2
    void read(SWFStream& in, tag_type tag);

    /// Place/move/whatever our object in the given movie.
    void execute(sprite_instance* m, DisplayList& dlist) const;

    static void loader(SWFStream& in, tag_type tag, movie_definition& m);

    int getPlaceType() const { return m_has_flags2 & (HAS_CHARACTER_MASK | MOVE_MASK); } 
    int getRatio()     const { return m_ratio; }
    int getClipDepth() const { return m_clip_depth; }
    int getID()        const { return m_character_id; }
    const std::string& getName() const { return m_name; }
    const matrix& getMatrix()    const { return m_matrix; }
    const cxform& getCxform()    const { return m_color_transform; }
    const EventHandlers& getEventHandlers() const { return _eventHandlers; }
    
    bool hasClipActions() const { return m_has_flags2 & HAS_CLIP_ACTIONS_MASK; }
    bool hasClipDepth()   const { return m_has_flags2 & HAS_CLIP_DEPTH_MASK; };
    bool hasName()        const { return m_has_flags2 & HAS_NAME_MASK; }
    bool hasRatio()       const { return m_has_flags2 & HAS_RATIO_MASK; }
    bool hasCxform()      const { return m_has_flags2 & HAS_CXFORM_MASK; }
    bool hasMatrix()      const { return m_has_flags2 & HAS_MATRIX_MASK; }
    bool hasCharacter()   const { return m_has_flags2 & HAS_CHARACTER_MASK; }

    bool hasImage()         const { return m_has_flags3 & HAS_IMAGE_MASK; }
    bool hasClassName()     const { return m_has_flags3 & HAS_CLASS_NAME_MASK; }
    bool hasBitmapCaching() const { return m_has_flags3 & HAS_BITMAP_CACHING_MASK; }
    bool hasBlendMode()     const { return m_has_flags3 & HAS_BLEND_MODE_MASK; }
    bool hasFilters()       const { return m_has_flags3 & HAS_FILTERS_MASK; }      

private:
    int m_tag_type;
    boost::uint8_t m_has_flags2;
    boost::uint8_t m_has_flags3;
    boost::uint16_t m_character_id;
    cxform  m_color_transform;
    matrix  m_matrix;
    int     m_ratio;
    std::string m_name;
    int     m_clip_depth;
    boost::uint32_t all_event_flags; 
    
    /// NOTE: getPlaceType() is dependent on the enum values.
    enum PlaceType
    {
        REMOVE  = 0, 
        MOVE    = 1,
        PLACE   = 2,
        REPLACE = 3
    };

    enum has_flags2_mask_e
    {
        HAS_CLIP_ACTIONS_MASK = 1 << 7,
        HAS_CLIP_DEPTH_MASK   = 1 << 6,
        HAS_NAME_MASK         = 1 << 5,
        HAS_RATIO_MASK        = 1 << 4,
        HAS_CXFORM_MASK       = 1 << 3,
        HAS_MATRIX_MASK       = 1 << 2,
        HAS_CHARACTER_MASK    = 1 << 1,
        MOVE_MASK             = 1 << 0
    };

    enum has_flags3_mask_e
    {
        HAS_IMAGE_MASK          = 1 << 4,
        HAS_CLASS_NAME_MASK     = 1 << 3,
        HAS_BITMAP_CACHING_MASK = 1 << 2,
        HAS_BLEND_MODE_MASK     = 1 << 1,
        HAS_FILTERS_MASK        = 1 << 0
    };

    const movie_definition& _movie_def;

    ActionBuffers _actionBuffers;

    EventHandlers _eventHandlers;

    // read SWF::PLACEOBJECT 
    void readPlaceObject(SWFStream& in);

    // read placeObject2 actions
    void readPlaceActions(SWFStream& in);

    // read SWF::PLACEOBJECT2 
    void readPlaceObject2(SWFStream& in);

    // read SWF::PLACEOBJECT3
    void readPlaceObject3(SWFStream& in);

};

} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_PLACEOBJECT2TAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
