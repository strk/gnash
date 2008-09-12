// button_character_def.h:  Mouse-sensitive SWF buttons, for Gnash.
//
//   Copyright (C) 2006, 2007, 2008 Free Software Foundation, Inc.
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



#ifndef GNASH_BUTTON_CHARACTER_DEF_H
#define GNASH_BUTTON_CHARACTER_DEF_H

#include "smart_ptr.h" // GNASH_USE_GC
#include "character_def.h"
#include "sound_definition.h"
#include "rect.h" // for get_bound
#include "matrix.h" // for composition
#include "cxform.h" // for composition
#include "action_buffer.h" // for composition of button_action
#include "filter_factory.h" // for Filters (composition of button_record)

#include <boost/scoped_ptr.hpp>
#include <boost/cstdint.hpp> // for boost::uint64_t typedef

// Forward declarations
namespace gnash {
	class sprite_instance;
	class movie_definition;
	class event_id;
	class SWFStream; // for read signatures
}

namespace gnash {

class button_record
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
	int	m_character_id;

	// Who owns this ?
	character_def* m_character_def;

	int	m_button_layer;
	matrix	m_button_matrix;
	cxform	m_button_cxform;


public:

	button_record()
		:
		m_character_def(0)
	{
	}

	/// Read a button record from the SWF stream.
	//
	/// Return true if we read a record; false if this is a null
	///
	/// @param endPos
	///	Last stream offset available for a valid read
	///
	/// TODO: take the stream by ref, not pointer
	///
	bool	read(SWFStream& in, int tag_type, movie_definition& m, unsigned long endPos);

	/// Return true if the button_record is valid
	//
	/// A button record is invalid if it refers to a character
	/// which has not been defined.
	bool is_valid();

#ifdef GNASH_USE_GC
	/// Mark all reachable resources (for GC)
	//
	/// Reachable resources are:
	///  - m_character_def (??) what's it !?
	///
	void markReachableResources() const
	{
		if ( m_character_def ) m_character_def->setReachable();
	}
#endif // GNASH_USE_GC

};
	
class button_action
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
	button_action(SWFStream& in, int tag_type, unsigned long endPos, movie_definition& mdef);

	/// Return true if this action should be triggered by the given event.
	bool triggeredBy(const event_id& ev) const;

	/// Return true if this action is triggered by a keypress
	bool triggeredByKeyPress() const
	{
		return m_conditions&KEYPRESS;
	}

private:

	/// Return the keycode triggering this action
	//
	/// Return 0 if no key is supposed to trigger us
	///
	int getKeyCode() const
	{
		return (m_conditions&KEYPRESS) >> 9;
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


class button_character_definition : public character_def
{
public:

	struct sound_info
	{
		void read(SWFStream& in);

		bool m_no_multiple;
		bool m_stop_playback;
		bool m_has_envelope;
		bool m_has_loops;
		bool m_has_out_point;
		bool m_has_in_point;
		boost::uint32_t m_in_point;
		boost::uint32_t m_out_point;
		boost::uint16_t m_loop_count;
		std::vector<media::sound_handler::sound_envelope> m_envelopes;
	};

	struct button_sound_info
	{
		boost::uint16_t m_sound_id;
		sound_sample*	m_sam;
		sound_info m_sound_style;

		button_sound_info()
			:
			m_sam(0)
		{
		}

#ifdef GNASH_USE_GC
		/// Mark all reachable resources (for GC)
		//
		/// Reachable resources are:
		///  - sound sample (m_sam)
		///
		void markReachableResources() const;
#endif // GNASH_USE_GC
	};

	struct button_sound_def
	{
		// TODO: implement ?
		//void	read(SWFStream& in, movie_definition& m);

		button_sound_info m_button_sounds[4];

#ifdef GNASH_USE_GC
		/// Mark all reachable resources (for GC)
		//
		/// Reachable resources are:
		///  - button sound infos (m_button_sounds)
		///
		void markReachableResources() const
		{
			for (int i=0; i<4; ++i)
			{
				m_button_sounds[i].markReachableResources();
			}
		}
#endif // GNASH_USE_GC
	};

	/// \brief
	/// Construct a character definition as read from
	/// the given movie_definition (SWF)
	button_character_definition(movie_definition& m);

	virtual ~button_character_definition();

	/// Create a mutable instance of our definition.
	character* create_character_instance(character* parent, int id);

	/// Read a SWF::DEFINEBUTTON, SWF::DEFINEBUTTONSOUND or SWF::DEFINEBUTTON2
	void	read(SWFStream& in, int tag_type, movie_definition& m);

	/// Read a SWF::DEFINEBUTTON tag
	void	readDefineButton(SWFStream& in, movie_definition& m);

	/// Read a SWF::DEFINEBUTTON2 tag
	void	readDefineButton2(SWFStream& in, movie_definition& m);

	/// Read a SWF::DEFINEBUTTONSOUND tag
	void	readDefineButtonSound(SWFStream& in, movie_definition& m);
	
	/// Read a SWF::DEFINEBUTTONCXFORM tag
	void readDefineButtonCxform(SWFStream& in, movie_definition& m);
	
	const rect&	get_bound() const {
		// It is required that get_bound() is implemented in character definition
		// classes. However, button character definitions do not have shape 
		// definitions themselves. Instead, they hold a list of shape_character_def.
		// get_bound() is currently only used by generic_character which normally
		// is used only shape character definitions. See character_def.h to learn
		// why it is virtual anyway.
		// get_button_bound() is used for buttons.
		abort(); // should not be called  
		static rect unused;
		return unused;
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
		for (size_t i = 0, e = m_button_actions.size(); i < e; ++i)
		{
			const button_action& ba = *(m_button_actions[i]);
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
		for (ButtonRecVect::const_iterator i=m_button_records.begin(), e=m_button_records.end(); i!=e; ++i)
		{
			i->markReachableResources();
		}

		if ( m_sound ) m_sound->markReachableResources();
	}
#endif // GNASH_USE_GC

public: // TODO: make private

	typedef std::vector<button_record> ButtonRecVect; 
	ButtonRecVect m_button_records;

	boost::scoped_ptr<button_sound_def> m_sound;

private:

	typedef std::vector<button_action*> ButtonActVect;
	ButtonActVect m_button_actions;

	/// Currently set but unused (and also unaccessible)
	bool m_menu;

	/// The movie definition containing definition of this button
	movie_definition& _movieDef;
};

}	// end namespace gnash


#endif // GNASH_BUTTON_CHARACTER_DEF_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
