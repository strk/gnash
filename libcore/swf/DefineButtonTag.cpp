// DefineButtonTag.cpp:  Mouse-sensitive SWF buttons, for Gnash.
//
//   Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012
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
//

#include "DefineButtonTag.h"

#include <string>
#include <boost/functional.hpp>

#include "TypesParser.h"
#include "RunResources.h"
#include "DisplayObject.h"
#include "Button.h" // for createDisplayObject()
#include "DefineButtonCxformTag.h"
#include "SWF.h"
#include "SWFStream.h" // for read()
#include "movie_definition.h"
#include "action_buffer.h"
#include "filter_factory.h"
#include "GnashKey.h" // for gnash::key::codeMap
#include "GnashAlgorithm.h"
#include "Global_as.h"
#include "namedStrings.h"
#include "as_function.h"

namespace gnash {
namespace SWF {

// Forward declarations
namespace {
    std::string computeButtonStatesString(int flags);
}

DefineButtonTag::DefineButtonTag(SWFStream& in, movie_definition& m,
        TagType tag, boost::uint16_t id)
    :
    DefinitionTag(id),
    _soundTag(0),
    _trackAsMenu(false),
    _movieDef(m)
{
    switch (tag) {
        default:
            std::abort();
            break;
        case DEFINEBUTTON:
            readDefineButtonTag(in, m);
            break;
        case DEFINEBUTTON2:
            readDefineButton2Tag(in, m);
            break;
    }
}

DefineButtonTag::~DefineButtonTag()
{
}


void
DefineButtonTag::loader(SWFStream& in, TagType tag, movie_definition& m, 
            const RunResources& /*r*/)
{
    assert(tag == DEFINEBUTTON);
    in.ensureBytes(2);
    const boost::uint16_t id = in.read_u16();

    IF_VERBOSE_PARSE(
        log_parse(_("  DefineButton loader: character id = %d"), id);
    );

    std::auto_ptr<DefineButtonTag> bt(new DefineButtonTag(in, m, tag, id));

    m.addDisplayObject(id, bt.release());
}

void
DefineButton2Tag::loader(SWFStream& in, TagType tag, movie_definition& m, 
            const RunResources& /*r*/)
{
    assert(tag == DEFINEBUTTON2);
    in.ensureBytes(2);
    const boost::uint16_t id = in.read_u16();

    IF_VERBOSE_PARSE(
        log_parse(_("  DefineButton2 loader: chararacter id = %d"), id);
    );

    std::auto_ptr<DefineButtonTag> bt(new DefineButtonTag(in, m, tag, id));

    m.addDisplayObject(id, bt.release());
}


void
DefineButtonTag::readDefineButtonTag(SWFStream& in, movie_definition& m)
{

    // Old button tag.

    unsigned long endTagPos = in.get_tag_end_position();

    // Read button DisplayObject records.
    for (;;) {
        ButtonRecord r;
        if (r.read(in, SWF::DEFINEBUTTON, m, endTagPos) == false) {
            // Null record; marks the end of button records.
            break;
        }

        // SAFETY CHECK:
        // if the ButtonRecord is corrupted, discard it
        if (r.valid()) _buttonRecords.push_back(r);
    }

    if (in.tell() >= endTagPos) {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("Premature end of DEFINEBUTTON tag, "
                "won't read actions"));
        );
        return;
    }

    // Read actions.
    _buttonActions.push_back(new ButtonAction(in, SWF::DEFINEBUTTON,
                endTagPos, m));

}

void
DefineButtonTag::readDefineButton2Tag(SWFStream& in, movie_definition& m)
{
    // Character ID has been read already

    in.ensureBytes(1 + 2); // flags + actions offset

    // Read the menu flag
    // (this is a single bit, the first 7 bits are reserved)
    const boost::uint8_t flags = in.read_u8();
    _trackAsMenu = flags & (1 << 0);
    if (_trackAsMenu) {
        LOG_ONCE(log_unimpl("DefineButton2: trackAsMenu"));
    }

    // Read the action offset
    unsigned button_2_action_offset = in.read_u16();

    unsigned long tagEndPosition = in.get_tag_end_position();
    unsigned next_action_pos = in.tell() + button_2_action_offset - 2;

    if ( next_action_pos > tagEndPosition )
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("Next Button2 actionOffset (%u) points past "
                "the end of tag (%lu)"),
            button_2_action_offset, tagEndPosition);
        );
        return;
    }

    unsigned long endOfButtonRecords = tagEndPosition;
    if ( ! button_2_action_offset  ) endOfButtonRecords = tagEndPosition;

    // Read button records.
    // takes at least 1 byte for the end mark button record, so 
    // we don't attempt to parse at all unless we have at least 1 byte left
    while ( in.tell() < endOfButtonRecords )
    {
        ButtonRecord r;
        if (r.read(in, SWF::DEFINEBUTTON2, m, endOfButtonRecords) == false) {
            // Null record marks the end of button records.
            break;
        }

        // SAFETY CHECK:
        // if the ButtonRecord is corrupted, discard it
        if (r.valid()) {
            _buttonRecords.push_back(r);
        }
    }

    if (button_2_action_offset) {

        in.seek(next_action_pos);

        // Read Button2ActionConditions
        // Don't read past tag end
        while (in.tell() < tagEndPosition) {
            in.ensureBytes(2);
            unsigned next_action_offset = in.read_u16();
            if (next_action_offset) {
                next_action_pos = in.tell() + next_action_offset - 2;
                if (next_action_pos > tagEndPosition) {
                    IF_VERBOSE_MALFORMED_SWF(
                        log_swferror(_("Next action offset (%u) in "
                                "Button2ActionConditions points past "
                                "the end of tag"), next_action_offset);
                    );
                    next_action_pos = tagEndPosition;
                }
            }

            const size_t endActionPos = next_action_offset ?
                next_action_pos : tagEndPosition;

            _buttonActions.push_back(new ButtonAction(in, SWF::DEFINEBUTTON2,
                        endActionPos, m));

            if (next_action_offset == 0 ) {
                // done.
                break;
            }

            // seek to next action.
            in.seek(next_action_pos);
        }
    }
}

DisplayObject*
DefineButtonTag::createDisplayObject(Global_as& gl, DisplayObject* parent)
    const
{
    as_object* obj = getObjectWithPrototype(gl, NSV::CLASS_BUTTON);
    DisplayObject* ch = new Button(obj, this, parent);
    return ch;
}

int
DefineButtonTag::getSWFVersion() const
{
    return _movieDef.get_version();
}

bool
DefineButtonTag::hasKeyPressHandler() const
{
    return std::find_if(_buttonActions.begin(), _buttonActions.end(),
            boost::mem_fn(&ButtonAction::triggeredByKeyPress)) !=
            _buttonActions.end();
}

//
// ButtonAction
//

ButtonAction::ButtonAction(SWFStream& in, TagType t, unsigned long endPos,
        movie_definition& mdef)
    :
    _actions(mdef)
{
    // Read condition flags.
    if (t == SWF::DEFINEBUTTON) {
        _conditions = OVER_DOWN_TO_OVER_UP;
    }
    else {
        
        assert(t == SWF::DEFINEBUTTON2);

        if ( in.tell()+2 > endPos ) 
        {
            IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("Premature end of button action input: "
                    "can't read conditions"));
            );
            return;
        }
        in.ensureBytes(2);
        _conditions = in.read_u16();
    }

    IF_VERBOSE_PARSE (
        log_parse(_("   button actions for conditions 0x%x"),
            _conditions); // @@ need more info about which actions
    );

    // Read actions.
    _actions.read(in, endPos);
}

bool
ButtonAction::triggeredBy(const event_id& ev) const
{
    switch (ev.id()) {
        case event_id::ROLL_OVER: return _conditions & IDLE_TO_OVER_UP;
        case event_id::ROLL_OUT: return _conditions & OVER_UP_TO_IDLE;
        case event_id::PRESS: return _conditions & OVER_UP_TO_OVER_DOWN;
        case event_id::RELEASE: return _conditions & OVER_DOWN_TO_OVER_UP;
        case event_id::DRAG_OUT: return _conditions & OVER_DOWN_TO_OUT_DOWN;
        case event_id::DRAG_OVER: return _conditions & OUT_DOWN_TO_OVER_DOWN;
        case event_id::RELEASE_OUTSIDE: return _conditions & OUT_DOWN_TO_IDLE;
        case event_id::KEY_PRESS:
        {
            int keycode = getKeyCode();
            if (!keycode) return false; // not a keypress event
            return key::codeMap[ev.keyCode()][key::SWF] == keycode;
        }
        default: return false;
    }
}

//
// ButtonRecord
//

DisplayObject*
ButtonRecord::instantiate(Button* button, bool name) const
{
    assert(button);
    assert(_definitionTag);

    Global_as& gl = getGlobal(*getObject(button));

    DisplayObject* o = _definitionTag->createDisplayObject(gl, button);

    o->setMatrix(_matrix, true);
    o->setCxForm(_cxform);
    o->set_depth(_buttonLayer + DisplayObject::staticDepthOffset + 1);
    if (name && isReferenceable(*o)) {
        o->set_name(button->getNextUnnamedInstanceName());
    }
    return o;
}

bool
ButtonRecord::hasState(Button::MouseState st) const
{
    switch (st)
    {
        case Button::MOUSESTATE_UP: return _up;
        case Button::MOUSESTATE_DOWN: return _down;
        case Button::MOUSESTATE_OVER: return _over;
        case Button::MOUSESTATE_HIT: return _hitTest;
        default: return false;
    }
}

bool
ButtonRecord::read(SWFStream& in, TagType t,
        movie_definition& m, unsigned long endPos)
{
    // caller should check this
    if (in.tell() + 1 > endPos)
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("   premature end of button record input stream, "
                "can't read flags"));
        );
        return false;
    }

    in.ensureBytes(1);
    boost::uint8_t flags = in.read_u8();
    if (!flags) return false;

    // Upper 4 bits are:
    //
    bool buttonHasBlendMode = flags & (1 << 5); 
    bool buttonHasFilterList = flags & (1 << 4);
    _hitTest = flags & (1 << 3);
    _down = flags & (1 << 2);
    _over = flags & (1 << 1); 
    _up = flags & (1 << 0); 

    if (in.tell() + 2 > endPos) {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("   premature end of button record input stream, "
                "can't read DisplayObject id"));
        );
        return false;
    }
    in.ensureBytes(2);
    const boost::uint16_t id = in.read_u16();

    // Get DisplayObject definition now (safer)
    _definitionTag = m.getDefinitionTag(id);

    // If no DisplayObject with given ID is found in the movie
    // definition, we print an error, but keep parsing.
    if (!_definitionTag) {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("   button record for states [%s] refer to "
            "DisplayObject with id %d, which is not found "
            "in the chars dictionary"), computeButtonStatesString(flags), id);
        );
    }
    else {
        IF_VERBOSE_PARSE(
        log_parse(_("   button record for states [%s] contain "
            "DisplayObject %d (%s)"), computeButtonStatesString(flags),
            id, typeName(*_definitionTag));
        );
    }

    if (in.tell() + 2 > endPos) {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("   premature end of button record input stream, "
                "can't read button layer (depth?)"));
        );
        return false;
    }
    in.ensureBytes(2);
    _buttonLayer = in.read_u16();

    _matrix = readSWFMatrix(in);

    if (t == SWF::DEFINEBUTTON2) {
        _cxform = readCxFormRGBA(in);
    }

    if (buttonHasFilterList) {
        filter_factory::read(in, true, &_filters);
        LOG_ONCE(
            log_unimpl("Button filters"); 
        );
    }

    if (buttonHasBlendMode) {
        in.ensureBytes(1);
        _blendMode = in.read_u8();
        LOG_ONCE(
            log_unimpl("Button blend mode");
        );
    }

    return true;
}

namespace {

std::string
computeButtonStatesString(int flags)
{
    std::string ret;
    if (flags & (1 << 3)) ret += "hit";
    if (flags & (1 << 2)) { if (!ret.empty()) ret += ","; ret += "down"; }
    if (flags & (1 << 1)) { if (!ret.empty()) ret += ","; ret += "over"; }
    if (flags & (1 << 0)) { if (!ret.empty()) ret += ","; ret += "up"; }
    return ret;
}

} // anonymous namespace

} // namespace SWF
} // namespace gnash

// Local Variables:
// mode: C++
// c-basic-offset: 8
// tab-width: 8
// indent-tabs-mode: t
// End:
