//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifndef GNASH_ASHANDLERS_H
#define GNASH_ASHANDLERS_H

#include <string>
#include <map>
#include <vector>
#include "action.h" // we should get rid of this probably
#include "swf.h"


// Forward declarations
namespace gnash {
	class ActionExec;
}

namespace gnash {

namespace SWF { // gnash::SWF

enum ArgumentType {
    ARG_NONE = 0,
    ARG_STR,
    // default hex dump, in case the format is unknown or unsupported
    ARG_HEX,
    ARG_U8,
    ARG_U16,
    ARG_S16,
    ARG_PUSH_DATA,
    ARG_DECL_DICT,
    ARG_FUNCTION2
};

enum as_encoding_guess_t {
    ENCGUESS_UNICODE = 0,
    ENCGUESS_JIS = 1,
    ENCGUESS_OTHER = 2
};


class ActionHandler
{
    typedef void (*ActionCallback)(ActionExec& thread);

public:

    ActionHandler();
    ActionHandler(ActionType type, ActionCallback func);
    ActionHandler(ActionType type, std::string name, 
                  ActionCallback func);
    ActionHandler(ActionType type, std::string name, 
                  ActionCallback func, ArgumentType format);

    /// Execute the action
    void execute(ActionExec& thread) const;

    void toggleDebug(bool state) const { _debug = state; }
    ActionType getType()   const { return _type; }
    std::string getName()   const { return _name; }
    ArgumentType getArgFormat() const { return _arg_format; }

private:
    ActionType _type;
    std::string _name;
    ActionCallback _callback;
    mutable bool _debug;
    ArgumentType _arg_format;
};

/// A singleton containing the supported SWF Action handlers.
class SWFHandlers
{
public:

	/// TODO: use a vector as we can have at most 254 actions
	/// (127 w/out length, 127 with length, the high bit is
	///  used to distinguish the two types)
	//typedef std::map<ActionType, ActionHandler> container_type;

	// Indexed by action id
	typedef std::vector<ActionHandler> container_type;

	/// Return the singleton instance of SWFHandlers class
	static const SWFHandlers& instance();

	/// Execute the action identified by 'type' action type
	void execute(ActionType type, ActionExec& thread) const;

	void toggleDebug(bool state) { _debug = state; }

	size_t size() const { return get_handlers().size(); }

	ActionType lastType() const
	{
		return ACTION_GOTOEXPRESSION;
	}

	const ActionHandler &operator[] (ActionType x) const
	{
		return get_handlers()[x];
	}

	const char* action_name(ActionType x) const;

private:

	static container_type & get_handlers();

	/// Common code for ActionGetUrl and ActionGetUrl2
	//
	/// @see http://sswf.sourceforge.net/SWFalexref.html#action_get_url2
	/// @see http://sswf.sourceforge.net/SWFalexref.html#action_get_url
	///
	/// @param target
	///	the target window or _level1 to _level10
	///
	/// @param method
	///	0:NONE, 1:GET, 2:POST
	///
	static void CommonGetUrl(as_environment& env, 
			as_value target, const std::string& url,
			boost::uint8_t method);

	/// Common code for SetTarget and SetTargetExpression
	//
	/// @see http://sswf.sourceforge.net/SWFalexref.html#action_set_target
	/// @see http://sswf.sourceforge.net/SWFalexref.html#action_get_dynamic
	///
	/// @param target_name
	///	The target name. If empty new target will be the main movie.
	///
	/// @param thread
	///	The current execution thread.
	///
	static void CommonSetTarget(ActionExec& thread, 
			const std::string& target_name);

    /// Common code for guessing at the encoding of random text, between
    // Shift-Jis, UTF8, and other. Puts the DisplayObject count in length,
    // and the offsets to the DisplayObjects in offsets, if offsets is not NULL.
    // If not NULL, offsets should be at least s.length().
    // offsets are not accurate if the return value is GUESSENC_OTHER
    static as_encoding_guess_t guessEncoding(const std::string& s, int& length,
            std::vector<int>& offsets);

	static void ActionEnd(ActionExec& thread);
	static void ActionNextFrame(ActionExec& thread);
	static void ActionPrevFrame(ActionExec& thread);
	static void ActionPlay(ActionExec& thread);
	static void ActionStop(ActionExec& thread);
	static void ActionToggleQuality(ActionExec& thread);
	static void ActionStopSounds(ActionExec& thread);
	static void ActionGotoFrame(ActionExec& thread);
	static void ActionGetUrl(ActionExec& thread);
	static void ActionWaitForFrame(ActionExec& thread);
	static void ActionSetTarget(ActionExec& thread);
	static void ActionGotoLabel(ActionExec& thread);
	static void ActionAdd(ActionExec& thread);
	static void ActionSubtract(ActionExec& thread);
	static void ActionMultiply(ActionExec& thread);
	static void ActionDivide(ActionExec& thread);
	static void ActionEqual(ActionExec& thread);
	static void ActionLessThan(ActionExec& thread);
	static void ActionLogicalAnd(ActionExec& thread);
	static void ActionLogicalOr(ActionExec& thread);
	static void ActionLogicalNot(ActionExec& thread);
	static void ActionStringEq(ActionExec& thread);
	static void ActionStringLength(ActionExec& thread);
	static void ActionSubString(ActionExec& thread);
	static void ActionPop(ActionExec& thread);
	static void ActionInt(ActionExec& thread);
	static void ActionGetVariable(ActionExec& thread);
	static void ActionSetVariable(ActionExec& thread);
	static void ActionSetTargetExpression(ActionExec& thread);
	static void ActionStringConcat(ActionExec& thread);
	static void ActionGetProperty(ActionExec& thread);
	static void ActionSetProperty(ActionExec& thread);
	static void ActionDuplicateClip(ActionExec& thread);
	static void ActionRemoveClip(ActionExec& thread);
	static void ActionTrace(ActionExec& thread);
	static void ActionStartDragMovie(ActionExec& thread);
	static void ActionStopDragMovie(ActionExec& thread);
	static void ActionStringCompare(ActionExec& thread);
	static void ActionThrow(ActionExec& thread);
	static void ActionCastOp(ActionExec& thread);
	static void ActionImplementsOp(ActionExec& thread);
	static void ActionFscommand2(ActionExec& thread);
	static void ActionRandom(ActionExec& thread);
	static void ActionMbLength(ActionExec& thread);
	static void ActionOrd(ActionExec& thread);
	static void ActionChr(ActionExec& thread);
	static void ActionGetTimer(ActionExec& thread);
	static void ActionMbSubString(ActionExec& thread);
	static void ActionMbOrd(ActionExec& thread);
	static void ActionMbChr(ActionExec& thread);
	static void ActionWaitForFrameExpression(ActionExec& thread);
	static void ActionPushData(ActionExec& thread);
	static void ActionBranchAlways(ActionExec& thread);
	static void ActionGetUrl2(ActionExec& thread);
	static void ActionBranchIfTrue(ActionExec& thread);
	static void ActionCallFrame(ActionExec& thread);
	static void ActionGotoExpression(ActionExec& thread);
	static void ActionDelete(ActionExec& thread);
	static void ActionDelete2(ActionExec& thread);
	static void ActionVarEquals(ActionExec& thread);
	static void ActionCallFunction(ActionExec& thread);
	static void ActionReturn(ActionExec& thread);
	static void ActionModulo(ActionExec& thread);
	static void ActionNew(ActionExec& thread);
	static void ActionVar(ActionExec& thread);
	static void ActionInitArray(ActionExec& thread);
	static void ActionInitObject(ActionExec& thread);
	static void ActionTypeOf(ActionExec& thread);
	static void ActionTargetPath(ActionExec& thread);
	static void ActionEnumerate(ActionExec& thread);
	static void ActionNewAdd(ActionExec& thread);
	static void ActionNewLessThan(ActionExec& thread);
	static void ActionNewEquals(ActionExec& thread);
	static void ActionToNumber(ActionExec& thread);
	static void ActionToString(ActionExec& thread);
	static void ActionDup(ActionExec& thread);
	static void ActionSwap(ActionExec& thread);
	static void ActionGetMember(ActionExec& thread);
	static void ActionSetMember(ActionExec& thread);
	static void ActionIncrement(ActionExec& thread);
	static void ActionDecrement(ActionExec& thread);
	static void ActionCallMethod(ActionExec& thread);
	static void ActionNewMethod(ActionExec& thread);
	static void ActionInstanceOf(ActionExec& thread);
	static void ActionEnum2(ActionExec& thread);
	static void ActionBitwiseAnd(ActionExec& thread);
	static void ActionBitwiseOr(ActionExec& thread);
	static void ActionBitwiseXor(ActionExec& thread);
	static void ActionShiftLeft(ActionExec& thread);
	static void ActionShiftRight(ActionExec& thread);
	static void ActionShiftRight2(ActionExec& thread);
	static void ActionStrictEq(ActionExec& thread);
	static void ActionGreater(ActionExec& thread);
	static void ActionStringGreater(ActionExec& thread);
	static void ActionExtends(ActionExec& thread);
	static void ActionConstantPool(ActionExec& thread);
	static void ActionDefineFunction2(ActionExec& thread);
	static void ActionTry(ActionExec& thread);
	static void ActionWith(ActionExec& thread);
	static void ActionDefineFunction(ActionExec& thread);
	static void ActionSetRegister(ActionExec& thread);

	bool _debug;

	// Use the ::instance() method to get a reference
	SWFHandlers();

	// You won't destroy a singleton
	~SWFHandlers();

};


} // namespace gnash::SWF

} // namespace gnash

#endif // end of __ASHANDLERS_H__
