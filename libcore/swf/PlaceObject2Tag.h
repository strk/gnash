// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011. 2012
//   Free Software Foundation, Inc.
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

#include <string>
#include <boost/ptr_container/ptr_vector.hpp>

#include "DisplayListTag.h" // for inheritance
#include "SWF.h" // for TagType definition
#include "SWFMatrix.h" // for composition
#include "SWFCxForm.h" // for composition 

// Forward declarations
namespace gnash {
    class SWFStream;
    class swf_event;
    class action_buffer;
    class movie_definition;
	class DisplayList;
    class RunResources;
}

namespace gnash {
namespace SWF {

/// SWF Tag PlaceObject (4) or PlaceObject2 (9) 
//
/// This tag is owned by the movie_definiton class
///
/// The PlaceObject tags can be used to:
/// - Place a DisplayObject to a depth. See isPlace().
/// - Transform the DisplayObject placed at a depth. See isMove().
/// - Replace a DisplayObject at a depth. See isReplace().
///
/// In any case a single Timeline depth is affected.
/// Postcondition of this tag execution is presence of an instance
/// at the affected depth. See getDepth().
///
///
/// _id:
/// The ID of the DisplayObject to be added.
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
/// The SWFMatrix transform to apply to the newly created instance.
///
/// _ratio
///
/// m_clip_depth:
/// If != DisplayObject::noClipDepthValue, mark the created instance
/// as a clipping layer. The shape of the placed DisplayObject will be
/// used as a mask for all higher depths up to this value.
///
class PlaceObject2Tag : public DisplayListTag
{
public:

    typedef boost::ptr_vector<action_buffer> ActionBuffers;
    typedef boost::ptr_vector<swf_event> EventHandlers;

    PlaceObject2Tag(const movie_definition& def);

    ~PlaceObject2Tag();

    /// Read SWF::PLACEOBJECT or SWF::PLACEOBJECT2
    void read(SWFStream& in, TagType tag);

    /// Place/move/whatever our object in the given movie.
    void executeState(MovieClip* m, DisplayList& dlist) const;

    static void loader(SWFStream& in, TagType tag, movie_definition& m,
            const RunResources& r);

    int getPlaceType() const { 
        return m_has_flags2 & (HAS_CHARACTER_MASK | MOVE_MASK);
    } 

    boost::uint16_t getRatio() const { return _ratio; }
    int getClipDepth() const { return m_clip_depth; }
    boost::uint16_t getID() const { return _id; }
    const std::string& getName() const { return m_name; }
    const SWFMatrix& getMatrix() const { return m_matrix; }
    const SWFCxForm& getCxform() const { return m_color_transform; }
    const EventHandlers& getEventHandlers() const { return _eventHandlers; }
    
    bool hasClipActions() const { return m_has_flags2 & HAS_CLIP_ACTIONS_MASK; }
    bool hasClipDepth()   const { return m_has_flags2 & HAS_CLIP_DEPTH_MASK; };
    bool hasName()        const { return m_has_flags2 & HAS_NAME_MASK; }
    bool hasRatio()       const { return m_has_flags2 & HAS_RATIO_MASK; }
    bool hasCxform()      const { return m_has_flags2 & HAS_CXFORM_MASK; }
    bool hasMatrix()      const { return m_has_flags2 & HAS_MATRIX_MASK; }
    bool hasCharacter()   const { return m_has_flags2 & HAS_CHARACTER_MASK; }

    bool hasImage() const { return m_has_flags3 & HAS_IMAGE_MASK; }

    bool hasClassName() const {
        return m_has_flags3 & HAS_CLASS_NAME_MASK;
    }

    bool hasBitmapCaching() const { 
        return m_has_flags3 & HAS_BITMAP_CACHING_MASK;
    }

    bool hasBlendMode() const {
        return m_has_flags3 & HAS_BLEND_MODE_MASK;
    }

    bool hasFilters() const {
        return m_has_flags3 & HAS_FILTERS_MASK;
    }

    /// Get an associated blend mode.
    //
    /// This is stored as a uint8_t to allow for future expansion of
    /// blend modes.
    boost::uint8_t getBlendMode() const {
        return _blendMode;
    }

private:

    // read SWF::PLACEOBJECT 
    void readPlaceObject(SWFStream& in);

    // read placeObject2 actions
    void readPlaceActions(SWFStream& in);

    // read SWF::PLACEOBJECT2 
    void readPlaceObject2(SWFStream& in);

    // read SWF::PLACEOBJECT3
    void readPlaceObject3(SWFStream& in);

    boost::uint8_t m_has_flags2;
    boost::uint8_t m_has_flags3;
    boost::uint16_t _id;
    SWFCxForm  m_color_transform;
    SWFMatrix  m_matrix;
    boost::uint16_t _ratio;
    std::string m_name;
    int     m_clip_depth;
    
    boost::uint8_t _blendMode;

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
};

} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_PLACEOBJECT2TAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
