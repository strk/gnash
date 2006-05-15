// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//

#ifndef __ASHANDLERS_H__
#define __ASHANDLERS_H__

#include <string>
#include <map>
#include <vector>
#include "action.h"
#include "swf.h"
#include "log.h"

namespace gnash {

/// SWF format parsing classes
namespace SWF { // gnash::SWF

typedef enum {
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
} as_arg_t;

typedef bool (*action_callback_t)(as_environment &env);
class ActionHandler
{
public:
    ActionHandler();
    ActionHandler(action_type type, action_callback_t func);
    ActionHandler(action_type type, std::string name, 
                  action_callback_t func);
    ActionHandler(action_type type, std::string name, 
                  action_callback_t func, as_arg_t format);
    ActionHandler(action_type type, std::string name, 
                  action_callback_t func, as_arg_t format, int nargs);
    ~ActionHandler();
    bool execute(as_environment &env);
    void toggleDebug(bool state) { _debug = state; }
    action_type getType()   { return _type; }
    std::string getName()   { return _name; }
    int getNumArgs()        { return _stack_args; }
    as_arg_t getArgFormat() { return _arg_format; }
private:
    action_type       _type;
    std::string       _name;
    action_callback_t _callback;
    bool              _debug;
    int               _stack_args; // the number of args to pop from the stack
    as_arg_t          _arg_format;
};

class SWFHandlers
{
public:
    SWFHandlers();
    ~SWFHandlers();
    bool execute(action_type type, as_environment &env);
    void toggleDebug(bool state) { _debug = state; }

    static bool ActionEnd(as_environment &end);    
    static bool ActionNextFrame(as_environment &env);
    static bool ActionPrevFrame(as_environment &env);
    static bool ActionPlay(as_environment &env);
    static bool ActionStop(as_environment &env);
    static bool ActionToggleQuality(as_environment &env);
    static bool ActionStopSounds(as_environment &env);
    static bool ActionGotoFrame(as_environment &env);
    static bool ActionGetUrl(as_environment &env);
    static bool ActionWaitForFrame(as_environment &env);
    static bool ActionSetTarget(as_environment &env);
    static bool ActionGotoLabel(as_environment &env);
    static bool ActionAdd(as_environment &env);
    static bool ActionSubtract(as_environment &env);
    static bool ActionMultiply(as_environment &env);
    static bool ActionDivide(as_environment &env);
    static bool ActionEqual(as_environment &env);
    static bool ActionLessThan(as_environment &env);
    static bool ActionLogicalAnd(as_environment &env);
    static bool ActionLogicalOr(as_environment &env);
    static bool ActionLogicalNot(as_environment &env);
    static bool ActionStringEq(as_environment &env);
    static bool ActionStringLength(as_environment &env);
    static bool ActionSubString(as_environment &env);
    static bool ActionPop(as_environment &env);
    static bool ActionInt(as_environment &env);
    static bool ActionGetVariable(as_environment &env);
    static bool ActionSetVariable(as_environment &env);
    static bool ActionSetTargetExpression(as_environment &env);
    static bool ActionStringConcat(as_environment &env);
    static bool ActionGetProperty(as_environment &env);
    static bool ActionSetProperty(as_environment &env);
    static bool ActionDuplicateClip(as_environment &env);
    static bool ActionRemoveClip(as_environment &env);
    static bool ActionTrace(as_environment &env);
    static bool ActionStartDragMovie(as_environment &env);
    static bool ActionStopDragMovie(as_environment &env);
    static bool ActionStringCompare(as_environment &env);
    static bool ActionThrow(as_environment &env);
    static bool ActionCastOp(as_environment &env);
    static bool ActionImplementsOp(as_environment &env);
    static bool ActionRandom(as_environment &env);
    static bool ActionMbLength(as_environment &env);
    static bool ActionOrd(as_environment &env);
    static bool ActionChr(as_environment &env);
    static bool ActionGetTimer(as_environment &env);
    static bool ActionMbSubString(as_environment &env);
    static bool ActionMbOrd(as_environment &env);
    static bool ActionMbChr(as_environment &env);
    static bool ActionWaitForFrameExpression(as_environment &env);
    static bool ActionPushData(as_environment &env);
    static bool ActionBranchAlways(as_environment &env);
    static bool ActionGetUrl2(as_environment &env);
    static bool ActionBranchIfTrue(as_environment &env);
    static bool ActionCallFrame(as_environment &env);
    static bool ActionGotoExpression(as_environment &env);
    static bool ActionDeleteVar(as_environment &env);
    static bool ActionDelete(as_environment &env);
    static bool ActionVarEquals(as_environment &env);
    static bool ActionCallFunction(as_environment &env);
    static bool ActionReturn(as_environment &env);
    static bool ActionModulo(as_environment &env);
    static bool ActionNew(as_environment &env);
    static bool ActionVar(as_environment &env);
    static bool ActionInitArray(as_environment &env);
    static bool ActionInitObject(as_environment &env);
    static bool ActionTypeOf(as_environment &env);
    static bool ActionTargetPath(as_environment &env);
    static bool ActionEnumerate(as_environment &env);
    static bool ActionNewAdd(as_environment &env);
    static bool ActionNewLessThan(as_environment &env);
    static bool ActionNewEquals(as_environment &env);
    static bool ActionToNumber(as_environment &env);
    static bool ActionToString(as_environment &env);
    static bool ActionDup(as_environment &env);
    static bool ActionSwap(as_environment &env);
    static bool ActionGetMember(as_environment &env);
    static bool ActionSetMember(as_environment &env);
    static bool ActionIncrement(as_environment &env);
    static bool ActionDecrement(as_environment &env);
    static bool ActionCallMethod(as_environment &env);
    static bool ActionNewMethod(as_environment &env);
    static bool ActionInstanceOf(as_environment &env);
    static bool ActionEnum2(as_environment &env);
    static bool ActionBitwiseAnd(as_environment &env);
    static bool ActionBitwiseOr(as_environment &env);
    static bool ActionBitwiseXor(as_environment &env);
    static bool ActionShiftLeft(as_environment &env);
    static bool ActionShiftRight(as_environment &env);
    static bool ActionShiftRight2(as_environment &env);
    static bool ActionStrictEq(as_environment &env);
    static bool ActionGreater(as_environment &env);
    static bool ActionStringGreater(as_environment &env);
    static bool ActionExtends(as_environment &env);
    static bool ActionConstantPool(as_environment &env);
    static bool ActionDefineFunction2(as_environment &env);
    static bool ActionTry(as_environment &env);
    static bool ActionWith(as_environment &env);
    static bool ActionDefineFunction(as_environment &env);
    static bool ActionSetRegister(as_environment &env);
    
    int size() { return (int)_handlers.size(); }
    action_type lastType() { return _handlers[ACTION_GOTOEXPRESSION].getType(); }
    ActionHandler &operator [](action_type x) { return _handlers[x]; }    
private:
    bool _debug;
    static std::map<action_type, ActionHandler> _handlers;
    static std::vector<std::string> _property_names;
};


} // namespace gnash::SWF

} // namespace gnash

#endif // end of __ASHANDLERS_H__
