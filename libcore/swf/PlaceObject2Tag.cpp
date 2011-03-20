// PlaceObject2Tag.cpp:  for Gnash.
//
//   Copyright (C) 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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
#include "gnashconfig.h" // HAVE_ZLIB_H, USE_SWFTREE
#endif

#include "PlaceObject2Tag.h"

#include <string>

#include "TypesParser.h"
#include "RunResources.h"
#include "DisplayObject.h"
#include "MovieClip.h"
#include "swf_event.h"
#include "log.h"
#include "SWFStream.h"
#include "filter_factory.h"
#include "GnashAlgorithm.h"
#include "action_buffer.h"
#include "smart_ptr.h"

namespace gnash {
namespace SWF {

PlaceObject2Tag::PlaceObject2Tag(const movie_definition& def)
    :
    DisplayListTag(0),
    m_has_flags2(0),
    m_has_flags3(0),
    _id(0),
    _ratio(0),
    m_clip_depth(0),
    _blendMode(0),
    _movie_def(def)
{
}

void
PlaceObject2Tag::readPlaceObject(SWFStream& in)
{
    // Original place_object tag; very simple.
    in.ensureBytes(2 + 2);
    _id = in.read_u16();
    _depth = in.read_u16() + DisplayObject::staticDepthOffset;

    // PlaceObject doesn't know about masks.
    m_clip_depth = DisplayObject::noClipDepthValue;

    // If these flags2 values aren't set here, nothing will
    // ever be displayed.
    m_has_flags2 = HAS_CHARACTER_MASK;

    if (in.tell() < in.get_tag_end_position())
    {
        m_matrix = readSWFMatrix(in);
        m_has_flags2 |= HAS_MATRIX_MASK;
        if (in.tell() < in.get_tag_end_position())
        {
            m_color_transform = readCxFormRGB(in);
            m_has_flags2 |= HAS_CXFORM_MASK;
        }
    }

    IF_VERBOSE_PARSE
    (
            log_parse(_("  PLACEOBJECT: depth=%d(%d) char=%d"),
            	_depth, _depth - DisplayObject::staticDepthOffset,
            	_id);
            if (hasMatrix()) log_parse("  SWFMatrix: %s", m_matrix);
            if (hasCxform()) log_parse(_("  SWFCxForm: %s"), m_color_transform);
    );

    
}

// read placeObject2 actions
void
PlaceObject2Tag::readPlaceActions(SWFStream& in)
{
    const int movie_version = _movie_def.get_version();

    in.ensureBytes(2);
    boost::uint16_t reserved = in.read_u16();
    IF_VERBOSE_MALFORMED_SWF(
        if (reserved != 0) {
            log_swferror(_("Reserved field in PlaceObject actions == "
                    "%u (expected 0)"), reserved);
        }
    );
    
    boost::uint32_t all_event_flags;

    // The logical 'or' of all the following handlers.
    if (movie_version >= 6) {
        in.ensureBytes(4);
        all_event_flags = in.read_u32();
    }
    else {
        in.ensureBytes(2);
        all_event_flags = in.read_u16();        
    }

    IF_VERBOSE_PARSE (
        log_parse(_("  actions: flags = 0x%X"), all_event_flags);
    );

    // Read swf_events.
    for (;;) {
        // Handle SWF malformations locally, by just prematurely interrupting
        // parsing of action events.
        // TODO: a possibly improvement would be using local code for the
        //       equivalent of ensureBytes which has the cost of a function
        //       call for itself plus a repeated useless function call for
        //       get_end_tag_position (which could be cached).
        //       
        try {
            // Read event.
            in.align();
    
            boost::uint32_t flags;
            if (movie_version >= 6) {
                in.ensureBytes(4);
                flags = in.read_u32();
            }
            else {
                in.ensureBytes(2);
                flags = in.read_u16();        
            }
    
            // no other events
            if (flags == 0) {
                break;
            }
    
            in.ensureBytes(4);
            boost::uint32_t event_length = in.read_u32();
            if (in.get_tag_end_position() - in.tell() <  event_length) {
                IF_VERBOSE_MALFORMED_SWF(
                log_swferror(_("swf_event::read(), "
                    "even_length = %u, but only %lu bytes left "
                    "to the end of current tag."
                    " Breaking for safety."),
                    event_length, in.get_tag_end_position() - in.tell());
                );
                break;
            }
    
            key::code ch = key::INVALID;
            
            // has KeyPress event
            if (flags & (1 << 17)) {
                in.ensureBytes(1);
                ch = static_cast<key::code>(in.read_u8());
                --event_length;
            }
    
            // Read the actions for event(s)
            // auto_ptr here prevents leaks on malformed swf
            std::auto_ptr<action_buffer> action(new action_buffer(_movie_def));
            action->read(in, in.tell() + event_length);
            _actionBuffers.push_back(action); 
    
            // If there is no end tag, action_buffer appends a null-terminator,
            // and fails this check. As action_buffer should check bounds, we
            // can just continue here.
            //assert (action->size() == event_length);
    
            // 13 bits reserved, 19 bits used
            static const event_id::EventCode s_code_bits[] = {
                event_id::LOAD,
                event_id::ENTER_FRAME,
                event_id::UNLOAD,
                event_id::MOUSE_MOVE,
                event_id::MOUSE_DOWN,
                event_id::MOUSE_UP,
                event_id::KEY_DOWN,
                event_id::KEY_UP,
    
                event_id::DATA,
                event_id::INITIALIZE,
                event_id::PRESS,
                event_id::RELEASE,
                event_id::RELEASE_OUTSIDE,
                event_id::ROLL_OVER,
                event_id::ROLL_OUT,
                event_id::DRAG_OVER,
    
                event_id::DRAG_OUT,
                event_id::KEY_PRESS,
                event_id::CONSTRUCT
            };
            const size_t total_known_events = arraySize(s_code_bits);
    
            // Let's see if the event flag we received is for an event
            // that we know of.
    
            // Integrity check: all reserved bits should be zero
            if (flags >> total_known_events) {
                IF_VERBOSE_MALFORMED_SWF(
                    log_swferror(_("swf_event::read() -- unknown / unhandled "
                            "event type received, flags = 0x%x"), flags);
                );
            }
    
            // Aah! same action for multiple events !
            for (size_t i = 0, mask = 1; i < total_known_events; ++i, mask <<= 1) {

                if (flags & mask) {
                    // Yes, swf_event stores a reference to an element in
                    // _actionBuffers. A case of remote ownership, but both
                    // swf_event and the actions are owned by this class,
                    // so shouldn't be a problem.
                    action_buffer& thisAction = _actionBuffers.back();

                    const event_id id(s_code_bits[i], (i == 17 ? ch : key::INVALID));

                    std::auto_ptr<swf_event> ev(new swf_event(id, thisAction));

                    IF_VERBOSE_PARSE(
                        log_parse("---- actions for event %s", ev->event());
                    );
    
                    _eventHandlers.push_back(ev);
                }
            }
        }
        catch (const ParserException& what) {
            IF_VERBOSE_MALFORMED_SWF(
                log_swferror(_("Unexpected end of tag while parsing "
                        "PlaceObject tag events"));
            );
            break;
        }
    }
}

// read SWF::PLACEOBJECT2
void
PlaceObject2Tag::readPlaceObject2(SWFStream& in)
{
    in.align();

    in.ensureBytes(1 + 2); // PlaceObject2, depth

    // PlaceObject2 specific flags
    m_has_flags2 = in.read_u8();

    _depth = in.read_u16()+DisplayObject::staticDepthOffset;

    if (hasCharacter()) {
        in.ensureBytes(2);
        _id = in.read_u16();
    }

    if (hasMatrix()) {
        m_matrix = readSWFMatrix(in);
    }

    if (hasCxform()) {
        m_color_transform = readCxFormRGBA(in);
    }

    if (hasRatio()) {
        in.ensureBytes(2);
        _ratio = in.read_u16();
    }

    if (hasName()) {
        in.read_string(m_name);
    }

    if (hasClipDepth()) {
        in.ensureBytes(2);
        m_clip_depth = in.read_u16() + DisplayObject::staticDepthOffset;
    }
    else {
        m_clip_depth = DisplayObject::noClipDepthValue;
    }

    if (hasClipActions()) {
        readPlaceActions(in);
    }

    IF_VERBOSE_PARSE(
        log_parse(_("  PLACEOBJECT2: depth = %d (%d)"),
            _depth, _depth-DisplayObject::staticDepthOffset);
        if (hasCharacter()) log_parse(_("  char id = %d"), _id);
        if (hasMatrix()) {
            log_parse(_("  SWFMatrix: %s"), m_matrix);
        }
        if (hasCxform()) {
            log_parse(_("  SWFCxForm: %s"), m_color_transform);
        }
        if (hasRatio()) log_parse(_("  ratio: %d"), _ratio);
        if (hasName()) log_parse(_("  name = %s"), m_name.c_str());
        if (hasClipDepth()) {
            log_parse(_("  clip_depth = %d (%d)"), m_clip_depth,
                m_clip_depth-DisplayObject::staticDepthOffset);
        }
        log_parse(_(" m_place_type: %d"), getPlaceType());
    );

}

// read SWF::PLACEOBJECT3
void
PlaceObject2Tag::readPlaceObject3(SWFStream& in)
{
    in.align();

    in.ensureBytes(1 + 1 + 2); // PlaceObject2, PlaceObject3, depth

    // PlaceObject2 specific flags
    m_has_flags2 = in.read_u8();

    // PlaceObject3 speckfic flags, first 3 bits are unused
    m_has_flags3 = in.read_u8();
    
    boost::uint8_t bitmask = 0;
    std::string className;

    _depth = in.read_u16() + DisplayObject::staticDepthOffset;

    // This is documented to be here, but real instances of 
    // tags with either className or hasImage defined are rare to
    // non-existent. Alexis' SWF reference has neither of them,
    // instead specifying 5 reserved bits in the PlaceObject3 flags.
    if (hasClassName() || (hasImage() && hasCharacter())) {
        log_unimpl("PLACEOBJECT3 with associated class name");
        in.read_string(className);
    }

    if (hasCharacter()) {
        in.ensureBytes(2);
        _id = in.read_u16();
    }

    if (hasMatrix()) {
        m_matrix = readSWFMatrix(in);
    }

    if (hasCxform()) {
        m_color_transform = readCxFormRGBA(in);
    }

    if (hasRatio()) {
        in.ensureBytes(2);
        _ratio = in.read_u16();
    }
    
    if (hasName()) {
        in.read_string(m_name);
    }

    if (hasClipDepth()) {
        in.ensureBytes(2);
        m_clip_depth = in.read_u16()+DisplayObject::staticDepthOffset;
    }
    else {
        m_clip_depth = DisplayObject::noClipDepthValue;
    }

    if (hasFilters()) {
        Filters v; // TODO: Attach the filters to the display object.
        filter_factory::read(in, true, &v);
	    // at time of writing no renderer supports bitmap filters
	    LOG_ONCE( log_unimpl("Bitmap filters") );
    }

    if (hasBlendMode()) {
        in.ensureBytes(1);
        _blendMode = in.read_u8();
        LOG_ONCE(log_unimpl("Blend mode in PlaceObject tag"));
    }

    if (hasBitmapCaching()) {
        // cacheAsBitmap is a boolean value, so the flag itself ought to be
        // enough. Alexis' SWF reference is unsure about this, but suggests
        // reading a byte here. The official SWF format spec doesn't mention
        // it.
        //
        // However, the movie the-last-stand.swf has one PlaceObject3 tag
        // with both PlaceActions and bitmap caching, and the reserved bytes
        // of the PlaceActions (see readPlaceActions) are not 0 if this byte
        // isn't read.
        // TODO: set object property
        in.ensureBytes(1);
        bitmask = in.read_u8();
	    LOG_ONCE(log_unimpl("Bitmap caching"));
    }

    if (hasClipActions()) {
        readPlaceActions(in);
    }

    IF_VERBOSE_PARSE (

        log_parse(_("  PLACEOBJECT3: depth = %d (%d)"), _depth,
            _depth - DisplayObject::staticDepthOffset);
        if (hasCharacter()) log_parse(_("  char id = %d"), _id);
        if (hasMatrix()) log_parse(_("  SWFMatrix: %s"), m_matrix);
        if (hasCxform()) log_parse(_("  SWFCxForm: %d"), m_color_transform);
        if (hasRatio()) log_parse(_("  ratio: %d"), _ratio);
        if (hasName()) log_parse(_("  name = %s"), m_name);
        if (hasClassName()) log_parse(_("  class name = %s"), className);
        if (hasClipDepth()) log_parse(_("  clip_depth = %d (%d)"),
                    m_clip_depth, m_clip_depth-DisplayObject::staticDepthOffset);
        if (hasBitmapCaching()) log_parse(_("   bitmapCaching enabled"));
        log_parse(_(" m_place_type: %d"), getPlaceType());
    );

}

void
PlaceObject2Tag::read(SWFStream& in, TagType tag)
{
    if (tag == SWF::PLACEOBJECT) {
        readPlaceObject(in);
    }
    else if (tag == SWF::PLACEOBJECT2) {
        readPlaceObject2(in);
    }
    else {
        readPlaceObject3(in);
    }
}

/// Place/move/whatever our object in the given movie.
void
PlaceObject2Tag::executeState(MovieClip* m, DisplayList& dlist) const
{
    switch (getPlaceType()) {
      case PLACE:
          m->add_display_object(this, dlist);
          break;

      case MOVE:
          m->move_display_object(this, dlist);
          break;

      case REPLACE:
          m->replace_display_object(this, dlist);
          break;

      case REMOVE:
		  m->remove_display_object(this, dlist);
          break;
    }
}


PlaceObject2Tag::~PlaceObject2Tag()
{
}

void
PlaceObject2Tag::loader(SWFStream& in, TagType tag, movie_definition& m,
        const RunResources& /*r*/)
{
    assert(tag == SWF::PLACEOBJECT || tag == SWF::PLACEOBJECT2 ||
            tag == SWF::PLACEOBJECT3);

    boost::intrusive_ptr<PlaceObject2Tag> ch(new PlaceObject2Tag(m));
    ch->read(in, tag);

    m.addControlTag(ch);
}

} // namespace gnash::SWF
} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
