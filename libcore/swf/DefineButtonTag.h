//
//   Copyright (C) 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include "smart_ptr.h" // GNASH_USE_GC
#include "DefinitionTag.h"
#include "sound_definition.h"
#include "SWFMatrix.h" // for composition
#include "cxform.h" // for composition
#include "action_buffer.h" // for composition of ButtonAction
#include "filter_factory.h" // for Filters (composition of button_record)
#include "sound_handler.h" // for sound_handler::sound_envelope in a vector..
#include "DefineButtonSoundTag.h"
#include "swf.h"

#include <boost/scoped_ptr.hpp>
#include <boost/cstdint.hpp> // for boost::uint64_t typedef
#include <memory>

// Forward declarations
namespace gnash {
	class MovieClip;
	class movie_definition;
	class event_id;
	class SWFStream; // for read signatures
}

namespace gnash {
namespace SWF {


/// A class for parsing ButtonRecord, used by DefineButton and DefineButton2
class ButtonRecord
{

private:

	/// SWF8 and above can have a number of filters
	/// associated with button records
	//
	/// Currently unused by Gnash.
	///
	Filters _filters;

	/// SWF8 and above can have a blend mode
	/// associated with button records.
	//
	/// Currently unused by Gnash.
	///
	boost::uint8_t _blendMode;

// TODO: make private, provide accessors 
public:

	bool	m_hit_test;
	bool	m_down;
	bool	m_over;
	bool	m_up;
	int	_id;

	// Who owns this ?
	DefinitionTag* m_DefinitionTag;

	int	m_button_layer;
	SWFMatrix	m_button_matrix;
	cxform	m_button_cxform;


public:

	ButtonRecord()
		:
		m_DefinitionTag(0)
	{
	}

	/// Read a button record from the SWF stream.
	//
	/// Return true if we read a record; false if this is a null
	///
	/// @param endPos
	///	Last stream offset available for a valid read
	///
	bool read(SWFStream& in, TagType t, movie_definition& m,
            unsigned long endPos);

	/// Return true if the button_record is valid
	//
	/// A button record is invalid if it refers to a DisplayObject
	/// which has not been defined.
	bool is_valid();

#ifdef GNASH_USE_GC
	/// Mark all reachable resources (for GC)
	//
	/// Reachable resources are:
	///  - m_DefinitionTag (??) what's it !?
	///
	void markReachableResources() const
	{
		if ( m_DefinitionTag ) m_DefinitionTag->setReachable();
	}
#endif // GNASH_USE_GC

};
	
/// A class for parsing an ActionRecord.
class ButtonAction
{
public:

	// TODO: define ownership of list elements !!
	action_buffer m_actions;

	/// @param endPos
	///	One past last valid-to-read byte position
	///
	/// @param mdef
	///	The movie_definition this button action was read from
	///
	///
	ButtonAction(SWFStream& in, TagType t, unsigned long endPos,
            movie_definition& mdef);

	/// Return true if this action should be triggered by the given event.
	bool triggeredBy(const event_id& ev) const;

	/// Return true if this action is triggered by a keypress
	bool triggeredByKeyPress() const
	{
		return (m_conditions & KEYPRESS);
	}

private:

	/// Return the keycode triggering this action
	//
	/// Return 0 if no key is supposed to trigger us
	///
	int getKeyCode() const
	{
		return (m_conditions & KEYPRESS) >> 9;
	}

	enum condition
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
	int	m_conditions;

};

/// A class for parsing DefineButton and DefineButton2 tags.
class DefineButtonTag : public DefinitionTag
{
public:

    /// Load a DefineButtonTag.
    static void loader(SWFStream& in, TagType tag, movie_definition& m, 
            const RunInfo& r);

	typedef std::vector<ButtonRecord> ButtonRecords; 
	typedef std::vector<ButtonAction*> ButtonActions;

	virtual ~DefineButtonTag();

	/// Create a mutable instance of our definition.
	DisplayObject* createDisplayObject(DisplayObject* parent, int id);

    /// Access the ButtonRecords directly. Used for modifying the
    /// Cxform by a DefineButtonCxform tag.
    ButtonRecords& buttonRecords() { return _buttonRecords; }

    /// Does this button have an associated DefineButtonSoundTag?
    bool hasSound() const { return (_soundTag.get()); }

    /// Add a DefineButtonSoundTag to the button. This should not be
    /// done twice, so check hasSound() first.
    void addSoundTag(std::auto_ptr<SWF::DefineButtonSoundTag> soundTag) {
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

	/// \brief
	/// Return version of the SWF containing
	/// this button definition.
	int getSWFVersion() const;

	bool hasKeyPressHandler() const;

	/// Invoke a functor for each action triggered by given event
	//
	/// The functor will be passed a const action_buffer&
	/// and is not expected to return anything.
	///
	template <class E>
	void forEachTrigger(const event_id& ev, E& f) const
	{
		for (size_t i = 0, e = _buttonActions.size(); i < e; ++i)
		{
			const ButtonAction& ba = *(_buttonActions[i]);
			if ( ba.triggeredBy(ev) ) f(ba.m_actions);
		}
	}
	
protected:

#ifdef GNASH_USE_GC
	/// Mark all reachable resources (for GC)
	//
	/// Reachable resources are:
	///  - button records (m_button_records)
	///  - button sound definition (m_sound)
	///
	void markReachableResources() const
	{
		assert(isReachable());
		for (ButtonRecords::const_iterator i = _buttonRecords.begin(),
                e = _buttonRecords.end(); i!=e; ++i)
		{
			i->markReachableResources();
		}

		if (_soundTag) _soundTag->markReachableResources();
	}
#endif // GNASH_USE_GC


private:

    /// DefineButton2Tag::loader also needs to create a DefineButtonTag.
    friend class DefineButton2Tag;

	/// Construct a DefineButtonTag (DefinitionTag)
    //
    /// This can only be constructed using a loader() function.
	DefineButtonTag(SWFStream& in, movie_definition& m, TagType tag);

	/// Read a DEFINEBUTTON tag
	void readDefineButtonTag(SWFStream& in, movie_definition& m);

	/// Read a DEFINEBUTTON2 tag
	void readDefineButton2Tag(SWFStream& in, movie_definition& m);

    boost::scoped_ptr<SWF::DefineButtonSoundTag> _soundTag;

	ButtonRecords _buttonRecords;
	ButtonActions _buttonActions;

	/// Currently set but unused (and also unaccessible)
	bool m_menu;

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
            const RunInfo& r);
};

}
}	// end namespace gnash


#endif // GNASH_BUTTON_CHARACTER_DEF_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
