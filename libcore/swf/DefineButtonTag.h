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

#ifndef GNASH_SWF_DEFINEBUTTONTAG_H
#define GNASH_SWF_DEFINEBUTTONTAG_H

#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>
#include <memory>
#include <boost/cstdint.hpp> 
#include <memory>

#include "DefinitionTag.h"
#include "SWFMatrix.h" 
#include "SWFCxForm.h" 
#include "action_buffer.h" 
#include "filter_factory.h" 
#include "TypesParser.h"
#include "DefineButtonSoundTag.h"
#include "SWF.h"
#include "Button.h"

// Forward declarations
namespace gnash {
    class movie_definition;
    class event_id;
    class SWFStream;
    class DisplayObject;
}

namespace gnash {
namespace SWF {


/// A class for parsing ButtonRecord, used by DefineButton and DefineButton2
class ButtonRecord
{

public:

    ButtonRecord()
        :
        _definitionTag(0)
    {
    }

    /// Create a DisplayObject from a ButtonRecord.
    //
    /// @param name     Whether the created DisplayObject requires its own
    ///                 instance name.
    /// @param button   The button to which the DisplayObject will belong.
    /// @return         A new DisplayObject. This should never be 0.
    DisplayObject* instantiate(Button* button, bool name = true) const;

    /// Check if this ButtonRecord has a DisplayObject for a particular state
    //
    /// @param state    The Button::MouseState to test for.
    /// @return         Whether the ButtonRecord should be used for that
    ///                 Button::MouseState.
    bool hasState(Button::MouseState st) const;

    /// Read an RGB SWFCxForm for this record.
    //
    /// Cxform is stored in a different tag for SWF2 Buttons
    /// (DEFINEBUTTON tag)
    void readRGBTransform(SWFStream& in) {
        _cxform = readCxFormRGB(in);
    }

    /// Read a ButtonRecord from the SWF stream.
    //
    /// Return true if we read a record; false if this is a null
    ///
    /// @param endPos
    ///    Last stream offset available for a valid read
    ///
    bool read(SWFStream& in, TagType t, movie_definition& m,
            unsigned long endPos);
    
    /// Return true if the ButtonRecord is valid
    //
    /// A ButtonRecord is invalid if it refers to a DisplayObject
    /// which has not been defined.
    bool valid() const {
        return _definitionTag.get();
    }

private:

    /// SWF8 and above can have a number of filters
    /// associated with button records
    //
    /// Currently unused by Gnash.
    Filters _filters;

    /// SWF8 and above can have a blend mode
    /// associated with button records.
    //
    /// Currently unused by Gnash.
    boost::uint8_t _blendMode;

    bool _hitTest;
    bool _down;
    bool _over;
    bool _up;

    // This is a ref-counted resource, so not owned by anyone.
    boost::intrusive_ptr<const DefinitionTag> _definitionTag;

    int _buttonLayer;

    SWFMatrix _matrix;

    SWFCxForm _cxform;

};
    
/// A class for parsing an ActionRecord.
class ButtonAction
{
public:

    // TODO: define ownership of list elements !!
    action_buffer _actions;

    /// @param endPos
    ///    One past last valid-to-read byte position
    ///
    /// @param mdef
    ///    The movie_definition this button action was read from
    ///
    ///
    ButtonAction(SWFStream& in, TagType t, unsigned long endPos,
            movie_definition& mdef);

    /// Return true if this action should be triggered by the given event.
    bool triggeredBy(const event_id& ev) const;

    /// Return true if this action is triggered by a keypress
    bool triggeredByKeyPress() const {
        return (_conditions & KEYPRESS);
    }

    /// Return the keycode triggering this action
    //
    /// Return 0 if no key is supposed to trigger us
    int getKeyCode() const {
        return (_conditions & KEYPRESS) >> 9;
    }

private:

    enum Condition
    {
        IDLE_TO_OVER_UP = 1 << 0,
        OVER_UP_TO_IDLE = 1 << 1,
        OVER_UP_TO_OVER_DOWN = 1 << 2,
        OVER_DOWN_TO_OVER_UP = 1 << 3,
        OVER_DOWN_TO_OUT_DOWN = 1 << 4,
        OUT_DOWN_TO_OVER_DOWN = 1 << 5,
        OUT_DOWN_TO_IDLE = 1 << 6,
        IDLE_TO_OVER_DOWN = 1 << 7,
        OVER_DOWN_TO_IDLE = 1 << 8,
        KEYPRESS = 0xFE00  // highest 7 bits
    };

    boost::uint16_t _conditions;

};

/// A class for parsing DefineButton and DefineButton2 tags.
class DefineButtonTag : public DefinitionTag
{
public:

    /// Load a DefineButtonTag.
    static void loader(SWFStream& in, TagType tag, movie_definition& m, 
            const RunResources& r);

    typedef std::vector<ButtonRecord> ButtonRecords; 
    typedef boost::ptr_vector<ButtonAction> ButtonActions;

    virtual ~DefineButtonTag();

    /// Create a mutable instance of our definition.
    DisplayObject* createDisplayObject(Global_as& gl, DisplayObject* parent)
        const;

    /// Access the ButtonRecords directly. Used for modifying the
    /// Cxform by a DefineButtonCxform tag.
    ButtonRecords& buttonRecords() { return _buttonRecords; }
    
    /// Read-only access to the ButtonRecords directly. 
    const ButtonRecords& buttonRecords() const { return _buttonRecords; }

    /// Does this button have an associated DefineButtonSoundTag?
    bool hasSound() const { return (_soundTag.get()); }

    /// Add a DefineButtonSoundTag to the button. This should not be
    /// done twice, so check hasSound() first.
    void addSoundTag(std::unique_ptr<SWF::DefineButtonSoundTag> soundTag) {
        // Do not replace a sound tag.
        assert(!_soundTag.get());
        _soundTag.reset(soundTag.release());
    }

    /// Return one of the four sounds associated with this Button
    //
    /// @param index    The sound index (0-3) to get.
    /// Do not call this function without checking hasSound() first.
    const DefineButtonSoundTag::ButtonSound& buttonSound(size_t index) const {
        assert(_soundTag.get());
        return _soundTag->getSound(index);
    }

    /// Return version of the SWF containing this button definition.
    int getSWFVersion() const;

    /// Whether to track this button as a menu.
    bool trackAsMenu() const {
        return _trackAsMenu;
    }

    bool hasKeyPressHandler() const;

    /// Invoke a functor for each action triggered by given event
    //
    /// The functor will be passed a const action_buffer&
    /// and is not expected to return anything.
    template <class E>
    void forEachTrigger(const event_id& ev, E& f) const {
        for (size_t i = 0, e = _buttonActions.size(); i < e; ++i) {
            const ButtonAction& ba = _buttonActions[i];
            if (ba.triggeredBy(ev)) f(ba._actions);
        }
    }

    /// Invoke a functor for each key code that should trigger an action.
    //
    /// Note: the key code is neither ascii nor a key index, but rather a
    /// special button key code (the SWF column of GnashKey.h).
    template<class E>
    void visitKeyCodes(E& f) const {
        std::for_each(_buttonActions.begin(), _buttonActions.end(),
            std::bind(f, std::bind(
                    boost::mem_fn(&ButtonAction::getKeyCode), std::placeholders::_1)));
    }
    
private:

    /// DefineButton2Tag::loader also needs to create a DefineButtonTag.
    friend class DefineButton2Tag;

    /// Construct a DefineButtonTag (DefinitionTag)
    //
    /// This can only be constructed using a loader() function.
    DefineButtonTag(SWFStream& in, movie_definition& m, TagType tag, 
            boost::uint16_t id);

    /// Read a DEFINEBUTTON tag
    void readDefineButtonTag(SWFStream& in, movie_definition& m);

    /// Read a DEFINEBUTTON2 tag
    void readDefineButton2Tag(SWFStream& in, movie_definition& m);

    std::unique_ptr<SWF::DefineButtonSoundTag> _soundTag;

    ButtonRecords _buttonRecords;

    ButtonActions _buttonActions;

    /// Whether to enable the trackAsMenu property.
    bool _trackAsMenu;

    /// The movie definition containing definition of this button
    movie_definition& _movieDef;
};

/// A class for parsing a DefineButton2 tag.
//
/// This only contains a loader because a DefineButton2Tag uses the same
/// code as DefineButtonTag with minor modifications. 
class DefineButton2Tag
{
public:
    /// Load a DefineButton2 tag.
    static void loader(SWFStream& in, TagType tag, movie_definition& m, 
            const RunResources& r);
};

}
}    // end namespace gnash


#endif // GNASH_BUTTON_CHARACTER_DEF_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
