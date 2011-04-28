// ASHandlers.cpp:  ActionScript handlers, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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
#include "gnashconfig.h"
#endif

#include "ASHandlers.h"

#include <string>
#include <vector>
#include <boost/scoped_array.hpp>
#include <boost/random.hpp>
#include <boost/lexical_cast.hpp>
#include <algorithm> 

#include "log.h"
#include "SWF.h"
#include "rc.h"
#include "NativeFunction.h"
#include "Function.h"
#include "as_function.h"
#include "Function2.h"
#include "fn_call.h"
#include "ActionExec.h"
#include "MovieClip.h"
#include "as_environment.h"
#include "URL.h"
#include "action_buffer.h"
#include "as_object.h"
#include "DragState.h"
#include "VM.h" // for getting the root
#include "movie_root.h" // for set_DragState (ActionStartDragMovie)
#include "sound_handler.h"
#include "namedStrings.h"
#include "utf8.h"
#include "StringPredicates.h" 
#include "GnashNumeric.h"
#include "Global_as.h"
#include "DisplayObject.h"
#include "as_environment.h"
#include "as_value.h"
#include "RunResources.h"
#include "ObjectURI.h"

// GNASH_PARANOIA_LEVEL:
// 0 : no assertions
// 1 : quick assertions
// 2 : check that handlers are called on correct tag
#ifndef GNASH_PARANOIA_LEVEL
# define GNASH_PARANOIA_LEVEL 1
#endif

namespace gnash {

namespace {

    as_object* construct_object(as_function* ctor_as_func, as_environment& env,
            unsigned int nargs);

    /// Convert to object without an exception being thrown.
    //
    /// @return     null if the value cannot be converted to an object.
    as_object* safeToObject(VM& vm, const as_value& val);

    /// Common code for ActionGetUrl and ActionGetUrl2
    //
    /// @param target         the target window or _level1 to _level10
    /// @param method       0:NONE, 1:GET, 2:POST
    void commonGetURL(as_environment& env, as_value target,
            const std::string& url, boost::uint8_t method);
    
    /// Common code for SetTarget and SetTargetExpression
    ///
    /// @param target_name      The target name. If empty new target will
    ///                         be the main movie.
    /// @param thread           The current execution thread.
    void commonSetTarget(ActionExec& thread, const std::string& target_name);

    
    void ActionEnd(ActionExec& thread);
    void ActionNextFrame(ActionExec& thread);
    void ActionPrevFrame(ActionExec& thread);
    void ActionPlay(ActionExec& thread);
    void ActionStop(ActionExec& thread);
    void ActionToggleQuality(ActionExec& thread);
    void ActionStopSounds(ActionExec& thread);
    void ActionGotoFrame(ActionExec& thread);
    void ActionGetUrl(ActionExec& thread);
    void ActionWaitForFrame(ActionExec& thread);
    void ActionSetTarget(ActionExec& thread);
    void ActionGotoLabel(ActionExec& thread);
    void ActionAdd(ActionExec& thread);
    void ActionSubtract(ActionExec& thread);
    void ActionMultiply(ActionExec& thread);
    void ActionDivide(ActionExec& thread);
    void ActionEqual(ActionExec& thread);
    void ActionLessThan(ActionExec& thread);
    void ActionLogicalAnd(ActionExec& thread);
    void ActionLogicalOr(ActionExec& thread);
    void ActionLogicalNot(ActionExec& thread);
    void ActionStringEq(ActionExec& thread);
    void ActionStringLength(ActionExec& thread);
    void ActionSubString(ActionExec& thread);
    void ActionPop(ActionExec& thread);
    void ActionInt(ActionExec& thread);
    void ActionGetVariable(ActionExec& thread);
    void ActionSetVariable(ActionExec& thread);
    void ActionSetTargetExpression(ActionExec& thread);
    void ActionStringConcat(ActionExec& thread);
    void ActionGetProperty(ActionExec& thread);
    void ActionSetProperty(ActionExec& thread);
    void ActionDuplicateClip(ActionExec& thread);
    void ActionRemoveClip(ActionExec& thread);
    void ActionTrace(ActionExec& thread);
    void ActionStartDragMovie(ActionExec& thread);
    void ActionStopDragMovie(ActionExec& thread);
    void ActionStringCompare(ActionExec& thread);
    void ActionThrow(ActionExec& thread);
    void ActionCastOp(ActionExec& thread);
    void ActionImplementsOp(ActionExec& thread);
    void ActionFscommand2(ActionExec& thread);
    void ActionRandom(ActionExec& thread);
    void ActionMbLength(ActionExec& thread);
    void ActionOrd(ActionExec& thread);
    void ActionChr(ActionExec& thread);
    void ActionGetTimer(ActionExec& thread);
    void ActionMbSubString(ActionExec& thread);
    void ActionMbOrd(ActionExec& thread);
    void ActionMbChr(ActionExec& thread);
    void ActionStrictMode(ActionExec& thread);
    void ActionWaitForFrameExpression(ActionExec& thread);
    void ActionPushData(ActionExec& thread);
    void ActionBranchAlways(ActionExec& thread);
    void ActionGetUrl2(ActionExec& thread);
    void ActionBranchIfTrue(ActionExec& thread);
    void ActionCallFrame(ActionExec& thread);
    void ActionGotoExpression(ActionExec& thread);
    void ActionDelete(ActionExec& thread);
    void ActionDelete2(ActionExec& thread);
    void ActionVarEquals(ActionExec& thread);
    void ActionCallFunction(ActionExec& thread);
    void ActionReturn(ActionExec& thread);
    void ActionModulo(ActionExec& thread);
    void ActionNew(ActionExec& thread);
    void ActionVar(ActionExec& thread);
    void ActionInitArray(ActionExec& thread);
    void ActionInitObject(ActionExec& thread);
    void ActionTypeOf(ActionExec& thread);
    void ActionTargetPath(ActionExec& thread);
    void ActionEnumerate(ActionExec& thread);
    void ActionNewAdd(ActionExec& thread);
    void ActionNewLessThan(ActionExec& thread);
    void ActionNewEquals(ActionExec& thread);
    void ActionToNumber(ActionExec& thread);
    void ActionToString(ActionExec& thread);
    void ActionDup(ActionExec& thread);
    void ActionSwap(ActionExec& thread);
    void ActionGetMember(ActionExec& thread);
    void ActionSetMember(ActionExec& thread);
    void ActionIncrement(ActionExec& thread);
    void ActionDecrement(ActionExec& thread);
    void ActionCallMethod(ActionExec& thread);
    void ActionNewMethod(ActionExec& thread);
    void ActionInstanceOf(ActionExec& thread);
    void ActionEnum2(ActionExec& thread);
    void ActionBitwiseAnd(ActionExec& thread);
    void ActionBitwiseOr(ActionExec& thread);
    void ActionBitwiseXor(ActionExec& thread);
    void ActionShiftLeft(ActionExec& thread);
    void ActionShiftRight(ActionExec& thread);
    void ActionShiftRight2(ActionExec& thread);
    void ActionStrictEq(ActionExec& thread);
    void ActionGreater(ActionExec& thread);
    void ActionStringGreater(ActionExec& thread);
    void ActionExtends(ActionExec& thread);
    void ActionConstantPool(ActionExec& thread);
    void ActionDefineFunction2(ActionExec& thread);
    void ActionTry(ActionExec& thread);
    void ActionWith(ActionExec& thread);
    void ActionDefineFunction(ActionExec& thread);
    void ActionSetRegister(ActionExec& thread);
    void ActionUnsupported(ActionExec& thread);
}

namespace {

class Enumerator : public KeyVisitor
{
public:
    explicit Enumerator(as_environment& env) : _env(env) {}
    virtual void operator()(const ObjectURI& uri) {
        _env.push(uri.toString(getStringTable(_env)));
    }
private:
    as_environment& _env;
};

}

namespace SWF { 

ActionHandler::ActionHandler()
    :
    _callback(ActionUnsupported),
    _arg_format(ARG_NONE)
{
}

ActionHandler::ActionHandler(ActionType type, ActionCallback func,
        ArgumentType format)
    :
    _type(type),
    _callback(func),
    _arg_format(format)
{
}

void
ActionHandler::execute(ActionExec& thread) const
{
    return _callback(thread);
}

SWFHandlers::SWFHandlers()
    :
    _handlers(256)
{

    _handlers[ACTION_END] = ActionHandler(ACTION_END, ActionEnd);
    _handlers[ACTION_NEXTFRAME] = ActionHandler(ACTION_NEXTFRAME,
             ActionNextFrame);
    _handlers[ACTION_PREVFRAME] =  ActionHandler(ACTION_PREVFRAME,
             ActionPrevFrame);
    _handlers[ACTION_PLAY] = ActionHandler(ACTION_PLAY, ActionPlay);
    _handlers[ACTION_STOP] = ActionHandler(ACTION_STOP, ActionStop);
    _handlers[ACTION_TOGGLEQUALITY] = ActionHandler(ACTION_TOGGLEQUALITY,
             ActionToggleQuality);
    _handlers[ACTION_STOPSOUNDS] = ActionHandler(ACTION_STOPSOUNDS,
             ActionStopSounds);
    _handlers[ACTION_GOTOFRAME] = ActionHandler(ACTION_GOTOFRAME,
             ActionGotoFrame, ARG_U16);
    _handlers[ACTION_GETURL] = ActionHandler(ACTION_GETURL,
             ActionGetUrl, ARG_STR);
    _handlers[ACTION_WAITFORFRAME] = ActionHandler(ACTION_WAITFORFRAME,
             ActionWaitForFrame, ARG_HEX);
    _handlers[ACTION_SETTARGET] = ActionHandler(ACTION_SETTARGET,
             ActionSetTarget, ARG_STR);
    _handlers[ACTION_GOTOLABEL] = ActionHandler(ACTION_GOTOLABEL,
             ActionGotoLabel, ARG_STR);
    _handlers[ACTION_ADD] = ActionHandler(ACTION_ADD, ActionAdd);
    _handlers[ACTION_SUBTRACT] = ActionHandler(ACTION_SUBTRACT, ActionSubtract);
    _handlers[ACTION_MULTIPLY] = ActionHandler(ACTION_MULTIPLY, ActionMultiply);
    _handlers[ACTION_DIVIDE] = ActionHandler(ACTION_DIVIDE, ActionDivide);
    _handlers[ACTION_EQUAL] = ActionHandler(ACTION_EQUAL, ActionEqual);
    _handlers[ACTION_LESSTHAN] = ActionHandler(ACTION_LESSTHAN, ActionLessThan);
    _handlers[ACTION_LOGICALAND] = ActionHandler(ACTION_LOGICALAND,
             ActionLogicalAnd);
    _handlers[ACTION_LOGICALOR] = ActionHandler(ACTION_LOGICALOR,
             ActionLogicalOr);
    _handlers[ACTION_LOGICALNOT] = ActionHandler(ACTION_LOGICALNOT,
             ActionLogicalNot);
    _handlers[ACTION_STRINGEQ] = ActionHandler(ACTION_STRINGEQ,
             ActionStringEq);
    _handlers[ACTION_STRINGLENGTH] = ActionHandler(ACTION_STRINGLENGTH,
             ActionStringLength);
    _handlers[ACTION_SUBSTRING] = ActionHandler(ACTION_SUBSTRING,
             ActionSubString);
    _handlers[ACTION_POP] = ActionHandler(ACTION_POP, ActionPop);
    _handlers[ACTION_INT] = ActionHandler(ACTION_INT, ActionInt);
    _handlers[ACTION_GETVARIABLE] = ActionHandler(ACTION_GETVARIABLE,
             ActionGetVariable);
    _handlers[ACTION_SETVARIABLE] = ActionHandler(ACTION_SETVARIABLE,
             ActionSetVariable);
    _handlers[ACTION_SETTARGETEXPRESSION] =
        ActionHandler(ACTION_SETTARGETEXPRESSION, 
             ActionSetTargetExpression);
    _handlers[ACTION_STRINGCONCAT] = ActionHandler(ACTION_STRINGCONCAT,
             ActionStringConcat);
    _handlers[ACTION_GETPROPERTY] = ActionHandler(ACTION_GETPROPERTY,
             ActionGetProperty);
    _handlers[ACTION_SETPROPERTY] = ActionHandler(ACTION_SETPROPERTY,
             ActionSetProperty);
    _handlers[ACTION_DUPLICATECLIP] = ActionHandler(ACTION_DUPLICATECLIP,
             ActionDuplicateClip);
    _handlers[ACTION_REMOVECLIP] = ActionHandler(ACTION_REMOVECLIP,
             ActionRemoveClip);
    _handlers[ACTION_TRACE] = ActionHandler(ACTION_TRACE, ActionTrace);
    _handlers[ACTION_STARTDRAGMOVIE] = ActionHandler(ACTION_STARTDRAGMOVIE,
             ActionStartDragMovie);
    _handlers[ACTION_STOPDRAGMOVIE] = ActionHandler(ACTION_STOPDRAGMOVIE,
             ActionStopDragMovie);
    _handlers[ACTION_STRINGCOMPARE] = ActionHandler(ACTION_STRINGCOMPARE,
             ActionStringCompare);
    _handlers[ACTION_THROW] = ActionHandler(ACTION_THROW, ActionThrow);
    _handlers[ACTION_CASTOP] = ActionHandler(ACTION_CASTOP, ActionCastOp);
    _handlers[ACTION_IMPLEMENTSOP] = ActionHandler(ACTION_IMPLEMENTSOP,
             ActionImplementsOp);
    _handlers[ACTION_FSCOMMAND2] = ActionHandler(ACTION_FSCOMMAND2,
             ActionFscommand2);
    _handlers[ACTION_RANDOM] = ActionHandler(ACTION_RANDOM, ActionRandom);
    _handlers[ACTION_MBLENGTH] = ActionHandler(ACTION_MBLENGTH, ActionMbLength);
    _handlers[ACTION_ORD] = ActionHandler(ACTION_ORD, ActionOrd);
    _handlers[ACTION_CHR] = ActionHandler(ACTION_CHR, ActionChr);
    _handlers[ACTION_GETTIMER] = ActionHandler(ACTION_GETTIMER, ActionGetTimer);
    _handlers[ACTION_MBSUBSTRING] = ActionHandler(ACTION_MBSUBSTRING,
             ActionMbSubString);
    _handlers[ACTION_MBORD] = ActionHandler(ACTION_MBORD, ActionMbOrd);
    _handlers[ACTION_MBCHR] = ActionHandler(ACTION_MBCHR, ActionMbChr);
    _handlers[ACTION_STRICTMODE] = ActionHandler(ACTION_STRICTMODE,
             ActionStrictMode, ARG_U8);
    _handlers[ACTION_WAITFORFRAMEEXPRESSION] =
        ActionHandler(ACTION_WAITFORFRAMEEXPRESSION,
             ActionWaitForFrameExpression, ARG_HEX);
    _handlers[ACTION_PUSHDATA] = ActionHandler(ACTION_PUSHDATA, ActionPushData,
            ARG_PUSH_DATA);
    _handlers[ACTION_BRANCHALWAYS] = ActionHandler(ACTION_BRANCHALWAYS,
             ActionBranchAlways, ARG_S16);
    _handlers[ACTION_GETURL2] = ActionHandler(ACTION_GETURL2, ActionGetUrl2,
            ARG_HEX);
    _handlers[ACTION_BRANCHIFTRUE] = ActionHandler(ACTION_BRANCHIFTRUE,
             ActionBranchIfTrue, ARG_S16);
    _handlers[ACTION_CALLFRAME] = ActionHandler(ACTION_CALLFRAME,
            ActionCallFrame, ARG_HEX);
    _handlers[ACTION_GOTOEXPRESSION] = ActionHandler(ACTION_GOTOEXPRESSION,
            ActionGotoExpression, ARG_HEX);
    _handlers[ACTION_DELETE] = ActionHandler(ACTION_DELETE, ActionDelete);
    _handlers[ACTION_DELETE2] = ActionHandler(ACTION_DELETE2, ActionDelete2);
    _handlers[ACTION_VAREQUALS] = ActionHandler(ACTION_VAREQUALS,
            ActionVarEquals);
    _handlers[ACTION_CALLFUNCTION] = ActionHandler(ACTION_CALLFUNCTION,
            ActionCallFunction);
    _handlers[ACTION_RETURN] = ActionHandler(ACTION_RETURN, ActionReturn);
    _handlers[ACTION_MODULO] = ActionHandler(ACTION_MODULO, ActionModulo);
    _handlers[ACTION_NEW] = ActionHandler(ACTION_NEW, ActionNew);
    _handlers[ACTION_VAR] = ActionHandler(ACTION_VAR, ActionVar);
    _handlers[ACTION_INITARRAY] = ActionHandler(ACTION_INITARRAY,
            ActionInitArray);
    _handlers[ACTION_INITOBJECT] = ActionHandler(ACTION_INITOBJECT,
            ActionInitObject);
    _handlers[ACTION_TYPEOF] = ActionHandler(ACTION_TYPEOF, ActionTypeOf);
    _handlers[ACTION_TARGETPATH] = ActionHandler(ACTION_TARGETPATH,
            ActionTargetPath);
    _handlers[ACTION_ENUMERATE] = ActionHandler(ACTION_ENUMERATE,
            ActionEnumerate);
    _handlers[ACTION_NEWADD] = ActionHandler(ACTION_NEWADD, ActionNewAdd);
    _handlers[ACTION_NEWLESSTHAN] = ActionHandler(ACTION_NEWLESSTHAN,
            ActionNewLessThan);
    _handlers[ACTION_NEWEQUALS] = ActionHandler(ACTION_NEWEQUALS,
            ActionNewEquals);
    _handlers[ACTION_TONUMBER] = ActionHandler(ACTION_TONUMBER, ActionToNumber);
    _handlers[ACTION_TOSTRING] = ActionHandler(ACTION_TOSTRING, ActionToString);
    _handlers[ACTION_DUP] = ActionHandler(ACTION_DUP, ActionDup);
    _handlers[ACTION_SWAP] = ActionHandler(ACTION_SWAP, ActionSwap);
    _handlers[ACTION_GETMEMBER] = ActionHandler(ACTION_GETMEMBER, 
            ActionGetMember);
    _handlers[ACTION_SETMEMBER] = ActionHandler(ACTION_SETMEMBER,
            ActionSetMember);
    _handlers[ACTION_INCREMENT] = ActionHandler(ACTION_INCREMENT,
            ActionIncrement);
    _handlers[ACTION_DECREMENT] = ActionHandler(ACTION_DECREMENT,
            ActionDecrement);
    _handlers[ACTION_CALLMETHOD] = ActionHandler(ACTION_CALLMETHOD,
            ActionCallMethod);
    _handlers[ACTION_NEWMETHOD] = ActionHandler(ACTION_NEWMETHOD,
            ActionNewMethod);
    _handlers[ACTION_INSTANCEOF] = ActionHandler(ACTION_INSTANCEOF,
            ActionInstanceOf);
    _handlers[ACTION_ENUM2] = ActionHandler(ACTION_ENUM2, ActionEnum2);
    _handlers[ACTION_BITWISEAND] = ActionHandler(ACTION_BITWISEAND,
            ActionBitwiseAnd);
    _handlers[ACTION_BITWISEOR] = ActionHandler(ACTION_BITWISEOR,
            ActionBitwiseOr);
    _handlers[ACTION_BITWISEXOR] = ActionHandler(ACTION_BITWISEXOR,
            ActionBitwiseXor);
    _handlers[ACTION_SHIFTLEFT] = ActionHandler(ACTION_SHIFTLEFT,
            ActionShiftLeft);
    _handlers[ACTION_SHIFTRIGHT] = ActionHandler(ACTION_SHIFTRIGHT,
            ActionShiftRight);
    _handlers[ACTION_SHIFTRIGHT2] = ActionHandler(ACTION_SHIFTRIGHT2,
            ActionShiftRight2);
    _handlers[ACTION_STRICTEQ] = ActionHandler(ACTION_STRICTEQ, ActionStrictEq);
    _handlers[ACTION_GREATER] = ActionHandler(ACTION_GREATER, ActionGreater);
    _handlers[ACTION_STRINGGREATER] = ActionHandler(ACTION_STRINGGREATER,
            ActionStringGreater);
    _handlers[ACTION_EXTENDS] = ActionHandler(ACTION_EXTENDS, ActionExtends);
    _handlers[ACTION_CONSTANTPOOL] = ActionHandler(ACTION_CONSTANTPOOL,
            ActionConstantPool, ARG_DECL_DICT);
    _handlers[ACTION_DEFINEFUNCTION2] = ActionHandler(ACTION_DEFINEFUNCTION2,
            ActionDefineFunction2, ARG_FUNCTION2);
    _handlers[ACTION_TRY] = ActionHandler(ACTION_TRY, ActionTry, ARG_FUNCTION2);
    _handlers[ACTION_WITH] = ActionHandler(ACTION_WITH,
            ActionWith, ARG_U16);
    _handlers[ACTION_DEFINEFUNCTION] = ActionHandler(ACTION_DEFINEFUNCTION,
            ActionDefineFunction, ARG_HEX);
    _handlers[ACTION_SETREGISTER] = ActionHandler(ACTION_SETREGISTER,
            ActionSetRegister, ARG_U8);
}

SWFHandlers::~SWFHandlers()
{
}

const SWFHandlers&
SWFHandlers::instance()
{
    static const SWFHandlers instance;
    return instance;
}

void
SWFHandlers::execute(ActionType type, ActionExec& thread) const
{
    try {
        _handlers[type].execute(thread);
    }
    catch (const ActionParserException& e) {
        log_swferror(_("Malformed action code: %s"), e.what());
    }
}

} // namespace SWF


namespace {

void
ActionEnd(ActionExec& thread)
{
#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_END));
#endif
    log_error (_("%s: CHECKME: was broken"), __PRETTY_FUNCTION__);
    thread.skipRemainingBuffer();
}


void
ActionNextFrame(ActionExec& thread)
{
    as_environment& env = thread.env;

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_NEXTFRAME));
#endif

    DisplayObject* tgtch = env.target();
    MovieClip* tgt = tgtch ? tgtch->to_movie() : 0;
    if (tgt) tgt->goto_frame(tgt->get_current_frame() + 1);
    else log_debug(_("ActionNextFrame: as_environment target is null or not a sprite"));
}

void
ActionPrevFrame(ActionExec& thread)
{
    as_environment& env = thread.env;

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_PREVFRAME));
#endif

    DisplayObject* tgtch = env.target();
    MovieClip* tgt = tgtch ? tgtch->to_movie() : 0;
    if (tgt) tgt->goto_frame(tgt->get_current_frame() - 1);
    else log_debug(_("ActionPrevFrame: as_environment target is null or not a sprite"));
}

void
ActionPlay(ActionExec& thread)
{
    as_environment& env = thread.env;

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_PLAY));
#endif

    DisplayObject* tgtch = env.target();
    MovieClip* tgt = tgtch ? tgtch->to_movie() : 0;
    if (tgt) tgt->setPlayState(MovieClip::PLAYSTATE_PLAY);
    else log_debug(_("ActionPlay: as_environment target is null or not a sprite"));
}

void
ActionStop(ActionExec& thread)
{
    as_environment& env = thread.env;

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_STOP));
#endif

    DisplayObject* tgtch = env.target();
    MovieClip* tgt = tgtch ? tgtch->to_movie() : 0;
    if (tgt) tgt->setPlayState(MovieClip::PLAYSTATE_STOP);
    else log_debug(_("ActionStop: as_environment target is null or not a sprite"));
}

void
ActionToggleQuality(ActionExec& thread)
{
    movie_root& mr = getRoot(thread.env);
    if (mr.getQuality() != QUALITY_HIGH) {
        mr.setQuality(QUALITY_HIGH);
        return;
    }
    mr.setQuality(QUALITY_LOW);
}

void
ActionStopSounds(ActionExec& thread)
{
#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_STOPSOUNDS));
#endif

    VM& vm = getVM(thread.env);
    sound::sound_handler* s = vm.getRoot().runResources().soundHandler();
    if (s) {
        s->stop_all_sounds();
    }
}

void
ActionGotoFrame(ActionExec& thread)
{
    as_environment& env = thread.env;
    const action_buffer& code = thread.code;

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_GOTOFRAME));
#endif

    size_t frame = code.read_int16(thread.getCurrentPC() + 3);

    DisplayObject* tgtch = env.target();
    MovieClip* tgt = tgtch ? tgtch->to_movie() : 0;

    // frame number within this tag is hard-coded and 0-based
    if (tgt) tgt->goto_frame(frame);
    else {
        log_debug(_("ActionGotoFrame: as_environment target is null "
                "or not a sprite"));
    }
}

void
ActionGetUrl(ActionExec& thread)
{
    as_environment& env = thread.env;
    const action_buffer& code = thread.code;

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_GETURL));
#endif

    size_t pc = thread.getCurrentPC();

    // If this is an FSCommand, then call the callback
    // handler, if any.

    // Two strings as args.
    // TODO: make sure the NULL terminations are there
    // we could implement a safe_read_string(pc, maxlen)
    // and use tag length as maxlen
    //size_t tag_length = code.read_int16(pc+1);
    const char* url = code.read_string(pc+3);
    size_t urlLength = strlen(url)+1;
    
    // Will abort if code.read_string returns 0, but action
    // buffer should always have a null terminator at the
    // end. This replaces an assertion in commonGetURL.
    const std::string target(code.read_string(pc + 3 + urlLength));

    IF_VERBOSE_ACTION (
        log_action(_("GetUrl: target=%s url=%s"), target, url);
    );

    commonGetURL(env, target, url, 0u);
}

void
ActionWaitForFrame(ActionExec& thread)
{
    as_environment& env = thread.env;
    const action_buffer& code = thread.code;

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_WAITFORFRAME));
#endif

    // SWF integrity check
    const size_t tag_len = code.read_int16(thread.getCurrentPC() + 1);
    if (tag_len != 3) {
        IF_VERBOSE_MALFORMED_SWF (
            log_swferror(_("ActionWaitForFrame (0x%X) tag length == %d "
                   "(expected 3)"), SWF::ACTION_WAITFORFRAME, tag_len);
        );
    }

    // If we haven't loaded a specified frame yet, then
    // skip the specified number of actions.
    //
    unsigned int framenum = code.read_int16(thread.getCurrentPC()+3);
    boost::uint8_t skip = code[thread.getCurrentPC()+5];

    DisplayObject* target = env.target();
    MovieClip* target_sprite = target ? target->to_movie() : 0;
    if (!target_sprite) {
        log_error(_("%s: environment target is null or not a MovieClip"),
                __FUNCTION__);
        return;
    }

    const size_t totframes = target_sprite->get_frame_count();
    if (framenum > totframes) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("ActionWaitForFrame(%d): "
                       "target (%s) has only %d frames"),
                       framenum, totframes);
        );
        framenum = totframes;
    }

    // Actually *wait* for target frame, and never skip any action
    const size_t lastloaded = target_sprite->get_loaded_frames();
    if (lastloaded < framenum) {
        // better delegate this to ActionExec
        thread.skip_actions(skip);
    }

}

void
ActionSetTarget(ActionExec& thread)
{
    const action_buffer& code = thread.code;
    size_t pc = thread.getCurrentPC();

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_SETTARGET)); // 0x8B
#endif

    // Change the movie we're working on.
    const std::string target_name(code.read_string(pc+3));

    commonSetTarget(thread, target_name);
}

void
ActionGotoLabel(ActionExec& thread)
{
    as_environment& env = thread.env;
    const action_buffer& code = thread.code;

    const char* frame_label = code.read_string(thread.getCurrentPC()+3);
    DisplayObject *target = env.target();
    MovieClip *target_sprite = target ? target->to_movie() : 0;
    if (!target_sprite) {
        log_error(_("GotoLabel: environment target is null or not a "
                    "MovieClip"));
    }
    else {
        target_sprite->goto_labeled_frame(frame_label);
    }
}

void
ActionAdd(ActionExec& thread)
{
    as_environment& env = thread.env;
    
    const double operand2 = toNumber(env.top(0), getVM(env));
    const double operand1 = toNumber(env.top(1), getVM(env));
    env.top(1) = operand1 + operand2;
    env.drop(1);
}

void
ActionSubtract(ActionExec& thread)
{
    as_environment& env = thread.env;
    subtract(env.top(1), env.top(0), getVM(env));
    env.drop(1);
}

void
ActionMultiply(ActionExec& thread)
{
    as_environment& env = thread.env;
    
    const double operand2 = toNumber(env.top(0), getVM(env));
    const double operand1 = toNumber(env.top(1), getVM(env));
    env.top(1) = operand1 * operand2;
    env.drop(1);
}


// Negative number / 0: -infinity
// Positive number / 0: infinity
// 0 / 0 : NaN
// Either operand is NaN: NaN
void
ActionDivide(ActionExec& thread)
{
    as_environment& env = thread.env;
    
    const double operand2 = toNumber(env.top(0), getVM(env));
    const double operand1 = toNumber(env.top(1), getVM(env));

    if (operand2 == 0) {
        if (env.get_version() < 5) {
            env.top(1).set_string("#ERROR#");
        }
        else if (operand1 == 0 || isNaN(operand1) || isNaN(operand2)) {
            setNaN(env.top(1));
        }
        else {
            // Division by -0.0 is not possible in AS, so 
            // the sign of the resulting infinity should match the 
            // sign of operand1. Division by 0 in C++ is undefined
            // behaviour.
            env.top(1) = operand1 < 0 ?
                - std::numeric_limits<double>::infinity() :
                std::numeric_limits<double>::infinity();
        }

    }
    else {
        env.top(1) = operand1 / operand2;
    }
    env.drop(1);
}

void
ActionEqual(ActionExec& thread)
{
    as_environment& env = thread.env;

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_EQUAL)); // 0x0E
#endif

    const double op1 = toNumber(env.top(0), getVM(env));
    const double op2 = toNumber(env.top(1), getVM(env));

    env.top(1).set_bool(op2 == op1);

    // Flash4 used 1 and 0 as return from this tag
    if (env.get_version() < 5) convertToNumber(env.top(1), getVM(env));

    env.drop(1);
}

void
ActionLessThan(ActionExec& thread)
{
    as_environment& env = thread.env;
    
    // NB: this unusual order is correct!
    const double d2 = toNumber(env.top(1), getVM(env));
    const double d1 = toNumber(env.top(0), getVM(env));

    env.top(1).set_bool(d2 < d1);

    // Flash4 used 1 and 0 as return from this tag
    if (env.get_version() < 5) convertToNumber(env.top(1), getVM(env));

    env.drop(1);
}

void
ActionLogicalAnd(ActionExec& thread)
{
    as_environment& env = thread.env;
    
    // Note: the order of evaluation of the && operands is specified.
    env.top(1).set_bool(toBool(env.top(1), getVM(env)) &&
            toBool(env.top(0), getVM(env)));
    env.drop(1);
}

void
ActionLogicalOr(ActionExec& thread)
{
    as_environment& env = thread.env;
    
    // Note: the order of evaluation of the || operands is specified.
    env.top(1).set_bool(toBool(env.top(1), getVM(env)) ||
            toBool(env.top(0), getVM(env)));
    env.drop(1);
}

void
ActionLogicalNot(ActionExec& thread)
{
    as_environment& env = thread.env;
    
    env.top(0).set_bool(!toBool(env.top(0), getVM(env)));

    // Flash4 used 1 and 0 as return from this tag
    if (env.get_version() < 5) convertToNumber(env.top(0), getVM(env));
}

void
ActionStringEq(ActionExec& thread)
{
    as_environment& env = thread.env;
    
    const int version = env.get_version();
    const std::string& str0 = env.top(0).to_string(version);
    const std::string& str1 = env.top(1).to_string(version);

    env.top(1).set_bool(str0 == str1);
    env.drop(1);
}

void
ActionStringLength(ActionExec& thread)
{
    as_environment& env = thread.env;

    // NOTE: I've tested that we should change behaviour
    //       based on code definition version, not top-level
    //       SWF version. Just not automated yet.
    //
    const int version = thread.code.getDefinitionVersion();
    if (version > 5) {
        // when SWF version is > 5 we compute the multi-byte length
        ActionMbLength(thread);
    }
    else {
        env.top(0).set_double(env.top(0).to_string(version).size());
    }
}

void
ActionSubString(ActionExec& thread)
{
    // substring("string",  base,  size) 
    // SWF4 function, deprecated in favour of String.substring.
    // 1-based (String object methods are 0-based).
    as_environment& env = thread.env;
    
    const as_value& strval = env.top(2);

    // Undefined values should resolve to 0.
    int size = toInt(env.top(0), getVM(env));
    int start = toInt(env.top(1), getVM(env));

    const int version = env.get_version();
    const std::wstring wstr = utf8::decodeCanonicalString(
                                strval.to_string(version), version);
    
    if (size < 0) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Negative size passed to ActionSubString, "
            "taking as whole length"));
        );
        size = wstr.length();
    }

    if (size == 0 || wstr.empty()) {
        env.drop(2);
        env.top(0).set_string("");
        return;
    }


    // TODO: if 'start' or 'size' do not evaluate to numbers return
    //       the empty string (how do we check if they evaluate ??)
    if (start < 1) {
        IF_VERBOSE_ASCODING_ERRORS (
            log_aserror(_("Start is less then 1 in ActionSubString, "
            "setting to 1."));
        );
        start = 1;
    }

    // If start is longer than the string length, return empty
    // string
    else if (static_cast<unsigned int>(start) > wstr.length()) {
        IF_VERBOSE_ASCODING_ERRORS (
            log_aserror(_("Start goes beyond input string in ActionSubString, "
            "returning the empty string."));
        );
        env.drop(2);
        env.top(0).set_string("");
        return;
    }

    // Adjust the start for our own use.
    --start;

    if (static_cast<unsigned int>(start + size) > wstr.length()) {
        IF_VERBOSE_ASCODING_ERRORS (
            log_aserror(_("start + size goes beyond input string in "
                    "ActionSubString, adjusting size"));
        );
        size = wstr.length() - start;
    }

#if GNASH_PARANOIA_LEVEL > 1
    assert(start >= 0);
    assert(static_cast<unsigned int>(start) < wstr.length() );
    assert(size >= 0);
#endif

    env.drop(2);
    env.top(0).set_string(utf8::encodeCanonicalString(
                                    wstr.substr(start, size), version));
}

void
ActionPop(ActionExec& thread)
{
    as_environment& env = thread.env;
    env.drop(1);
}

void
ActionInt(ActionExec& thread)
{
    as_environment& env = thread.env;
    env.top(0).set_double(toInt(env.top(0), getVM(env)));
}

void
ActionGetVariable(ActionExec& thread)
{
    as_environment& env = thread.env;

    as_value& top_value = env.top(0);
    const std::string& var_string = top_value.to_string();
    if (var_string.empty()) {
        top_value.set_undefined();
        return;
    }

    top_value = thread.getVariable(var_string);
    if (env.get_version() < 5 && top_value.is_sprite()) {
        // See http://www.ferryhalim.com/orisinal/g2/penguin.htm
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Can't assign a sprite/DisplayObject to a "
                "variable in SWF%d. We'll return undefined instead of %s."),
                env.get_version(), top_value);
        );
        top_value.set_undefined();
    }

    IF_VERBOSE_ACTION(
        log_action(_("-- get var: %s=%s"), var_string, top_value);
    );
}

void
ActionSetVariable(ActionExec& thread)
{
    as_environment& env = thread.env;

    const std::string& name = env.top(1).to_string();
    if (name.empty()) {
        IF_VERBOSE_ASCODING_ERRORS (
            // Invalid object, can't set.
            log_aserror(_("ActionSetVariable: %s=%s: variable name "
                    "evaluates to invalid (empty) string"),
                    env.top(1), env.top(0));
        );
    }
    thread.setVariable(name, env.top(0));

    IF_VERBOSE_ACTION(
        log_action(_("-- set var: %s = %s"), name, env.top(0));
    );

    env.drop(2);
}

void
ActionSetTargetExpression(ActionExec& thread)
{
    as_environment& env = thread.env;

    // we don't ues the target sprite directly, instead we fetch the
    // _target(string type) of that sprite first and then search the
    // final target(might be a different one). See tests in
    // opcode_guard_test2.sc
    //
    // For _versioned, see swfdec's settarget2-tostring.as (swf 7 and 8)
    // 
    std::string target_name = env.top(0).to_string(env.get_version());

    commonSetTarget(thread, target_name);

    env.drop(1); // pop the target sprite off the stack
}

void
ActionStringConcat(ActionExec& thread)
{
    as_environment& env = thread.env;
    const int version = getSWFVersion(env);

    const std::string& op1 = env.top(0).to_string(version);
    const std::string& op2 = env.top(1).to_string(version);

    env.top(1).set_string(op2 + op1);
    env.drop(1);
}

void
ActionGetProperty(ActionExec& thread)
{
    as_environment& env = thread.env;

    as_value& tgt_val = env.top(1);
    std::string tgt_str = tgt_val.to_string();

    DisplayObject* target;
    if (tgt_str.empty()) {
        target = get<DisplayObject>(thread.getTarget());
        if (!target) {
            IF_VERBOSE_MALFORMED_SWF(
                log_swferror(_("ActionGetProperty(<empty>) called, but "
                    "current target is not a DisplayObject"));
            );
        }
    }
    else {
        target = findTarget(env, tgt_str);
    }
 
    // FIXME: what happens when it's an invalid number? This will cause
    // undefined behaviour on overflow.
    unsigned int prop_number = toNumber(env.top(0), getVM(env));

    if (target) {
        getIndexedProperty(prop_number, *target, env.top(1));
    }
    else {
        // ASCODING error ? (well, last time it was a gnash error ;)
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Could not find GetProperty target (%s)"),
                    tgt_val);
        );
        env.top(1) = as_value();
    }
    env.drop(1);
}

void
ActionSetProperty(ActionExec& thread)
{
    as_environment& env = thread.env;

    DisplayObject *target = findTarget(env, env.top(2).to_string());
    // FIXME: what happens when it's an invalid number? This will cause
    // undefined behaviour on overflow.
    unsigned int prop_number = toNumber(env.top(1), getVM(env));

    as_value prop_val = env.top(0);

    if (target) {
        setIndexedProperty(prop_number, *target, prop_val);
    }
    else {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("ActionSetProperty: can't find target %s for "
                    "setting property %s"), env.top(2), prop_number);
        )
    }
    env.drop(3);
}

void
ActionDuplicateClip(ActionExec& thread)
{
    as_environment& env = thread.env;

    // Movies should be attachable from -16384 to 2130690044. See
    // Tests in misc-ming.all/DepthLimitsTest.c.
    const double depth = toNumber(env.top(0), getVM(env)) +
        DisplayObject::staticDepthOffset;
  
    // This also checks for overflow, as both numbers are expressible as
    // boost::int32_t.
    if (depth < DisplayObject::lowerAccessibleBound ||
      depth > DisplayObject::upperAccessibleBound) {

        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("duplicateMovieClip: invalid depth %d passed; "
                    "not duplicating"), depth);
        );  
        env.drop(3);
        return;
    }
  
    boost::int32_t depthValue = static_cast<boost::int32_t>(depth);
    
    const std::string& newname = env.top(1).to_string();
    const std::string& path = env.top(2).to_string();

    DisplayObject* ch = findTarget(env, path);
    if (!ch) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Path given to duplicateMovieClip(%s) doesn't "
                    "point to a DisplayObject"),
                path);
        );
        env.drop(3);
        return;
    }

    MovieClip* sprite = ch->to_movie();
    if (!sprite) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Path given to duplicateMovieClip(%s) is not a sprite"),
            path);
        );
        env.drop(3);
        return;
    }

    sprite->duplicateMovieClip(newname, depthValue);
    env.drop(3);
}

void
ActionRemoveClip(ActionExec& thread)
{
    as_environment& env = thread.env;

    const std::string path = env.pop().to_string();

    DisplayObject* ch = findTarget(env, path);
    if (!ch) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Path given to removeMovieClip(%s) doesn't "
                    "point to a DisplayObject"),
                path);
        );
        return;
    }

    MovieClip* sprite = ch->to_movie();
    if (!sprite) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Path given to removeMovieClip(%s) is not a sprite"),
            path);
        );
        return;
    }
    sprite->removeMovieClip();
}

/// \brief Trace messages from the Flash movie using trace();
void
ActionTrace(ActionExec& thread)
{
    as_environment& env = thread.env;
    const std::string val = env.pop().to_string();
    
    // Logging with a std::string here fails the swfdec testsuite,
    // probably because the first 0 character terminates the output
    // with a c_str, whereas a std::string outputs the entire length
    // of the string.
    log_trace("%s", val.c_str());
}

void
ActionStartDragMovie(ActionExec& thread)
{
    as_environment& env = thread.env;

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_STARTDRAGMOVIE));
#endif

    DisplayObject* tgt = findTarget(env, env.top(0).to_string());
    if (tgt) {
        // mark this DisplayObject as script transformed.
        tgt->transformedByScript();
    }
    else {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("startDrag: unknown target '%s'"), env.top(0));
        );
    }
    DragState st(tgt);

    st.setLockCentered(toBool(env.top(1), getVM(env)));

    // Handle bounds.
    if (toBool(env.top(2), getVM(env))) {
        // strk: this works if we didn't drop any before, in
        // a contrary case (if we used pop(), which I suggest)
        // we must remember to updated this as required
        boost::int32_t y1 = pixelsToTwips(toNumber(env.top(3), getVM(env)));
        boost::int32_t x1 = pixelsToTwips(toNumber(env.top(4), getVM(env)));
        boost::int32_t y0 = pixelsToTwips(toNumber(env.top(5), getVM(env)));
        boost::int32_t x0 = pixelsToTwips(toNumber(env.top(6), getVM(env)));

        // check for swapped values
        if (y1 < y0) {
            IF_VERBOSE_MALFORMED_SWF(
                log_swferror(_("Y values in ActionStartDrag swapped, fixing"));
            );
            std::swap(y1, y0);
        }

        if (x1 < x0) {
            IF_VERBOSE_MALFORMED_SWF(
                log_swferror(_("X values in ActionStartDrag swapped, fixing"));
            );
            std::swap(x1, x0);
        }
        const SWFRect bounds(x0, y0, x1, y1);
        st.setBounds(bounds);
        env.drop(4);
    }

    env.drop(3);

    if (tgt) {
        VM& vm = getVM(env);
        vm.getRoot().setDragState(st);
    }
}

void
ActionStopDragMovie(ActionExec& thread)
{
    as_environment& env = thread.env;
    DisplayObject* tgtch = env.target();
    MovieClip* root_movie = tgtch ? tgtch->get_root() : 0;
    if (root_movie) root_movie->stop_drag();
    else log_debug(_("ActionStopDragMovie: as_environment target is "
                "null or not a sprite"));
}

void
ActionStringCompare(ActionExec& thread)
{
    as_environment& env = thread.env;
    
    const int ver = env.get_version();
    const std::string& op1 = env.top(0).to_string(ver);
    const std::string& op2 = env.top(1).to_string(ver);
    env.top(1).set_bool(op2 < op1);
    env.drop(1);
}

void
ActionThrow(ActionExec& thread)
{
    as_environment& env = thread.env;

    // Throw the value on the top of the stack.
    env.top(0).flag_exception();

    // Proceed to the end of the code block to throw.
    thread.skipRemainingBuffer();
}

void
ActionCastOp(ActionExec& thread)
{
    as_environment& env = thread.env;

    // Get the "instance"
    as_object* instance = safeToObject(getVM(thread.env), env.top(0));

    // Get the "super" function
    as_object* super = safeToObject(getVM(thread.env), env.top(1));

    // Invalid args!
    if (!super || !instance) {
        IF_VERBOSE_ASCODING_ERRORS (
        log_aserror(_("-- %s cast_to %s (invalid args?)"),
            env.top(1),
            env.top(0));
        );

        env.drop(1);
        env.top(0).set_null(); 
        return;
    }

    env.drop(1);
    if (instance->instanceOf(super)) {
        env.top(0) = as_value(instance);
    }
    else {
        env.top(0).set_null(); 
    }

}


/// Implements the "implements" opcode
//
/// Example AS code: "C implements B;"
//
/// The above code makes an instance of C ("var c = new C();") return true
/// for "c instanceOf B);". That seems to be the end of its usefulness, as
/// c inherits no properties from B.
void
ActionImplementsOp(ActionExec& thread)
{
    as_environment& env = thread.env;

    as_value objval = env.pop();
    as_object* obj = safeToObject(getVM(thread.env), objval);
    int count = toNumber(env.pop(), getVM(env));

    if (!obj) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Stack value on IMPLEMENTSOP is not an object: %s."),
                objval);
        );
        return;
    }

    as_value protoval;
    if (!obj->get_member(NSV::PROP_PROTOTYPE, &protoval)) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Target object for IMPLEMENTSOP has no prototype."));
        );
        return;
    }
    obj = safeToObject(getVM(thread.env), protoval);
    if (!obj) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("IMPLEMENTSOP target object's prototype is not "
                    "an object (%s)"), protoval);
        );
        return;
    }

    if (count <= 0) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Invalid interfaces count (%d) on IMPLEMENTSOP"),
                count);
        );
        return;
    }

    while (count--) {
        as_value ctorval = env.pop();
        as_object* ctor = safeToObject(getVM(thread.env), ctorval);
        if (!ctor) {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("class found on stack on IMPLEMENTSOP is "
                        "not an object: %s"), ctorval);
            );
            continue;
        }
        if (!ctor->get_member(NSV::PROP_PROTOTYPE, &protoval)) {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("Interface object for IMPLEMENTSOP has no "
                        "prototype."));
            );
            continue;
        }
        as_object *inter = safeToObject(getVM(thread.env), protoval);
        if (!inter) {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("Prototype of interface object for "
                        "IMPLEMENTSOP is not an object (%s)."), protoval);
            );
            continue;
        }

        IF_VERBOSE_ACTION(
            log_action("%s (with .prototype %p) implements %s (with "
                ".prototype %p)", objval, (void*)obj, ctorval, (void*)inter);
        );
        obj->addInterface(inter);
    }
}

/// FsCommand2 is not part of the Flash spec. It belongs to
/// Flash Lite (from 1.1 onwards) and is used to control
/// devices (backlight, vibrate etc).
void
ActionFscommand2(ActionExec& thread)
{
#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_FSCOMMAND2)); // 0x0E
#endif

    as_environment& env = thread.env;

    unsigned int off=0;
    
    const unsigned int nargs = toInt(env.top(off++), getVM(env));

    std::string cmd = env.top(off++).to_string();

    std::ostringstream ss;
    ss << cmd << "(";
    for (unsigned int i = 1; i < nargs; ++i)
    {
        as_value arg = env.top(off++);
        if ( i > 1 ) ss << ", ";
        ss << arg;
    }
    ss << ")";

    LOG_ONCE( log_unimpl("fscommand2:%s", ss.str()) );

    // TODO: check wheter or not we should drop anything from
    //       the stack, some reports and the Canonical tests
    //       suggest we shoudn't
    
}

void
ActionRandom(ActionExec& thread)
{
    as_environment& env = thread.env;

    // Action random(n) should return an integer from 0 up to (not
    // including) n.
    // It was introduced in SWF4 and deprecated in favour of
    // Math.random() in SWF5.

    int max = toInt(env.top(0), getVM(env));

    if (max < 1) max = 1;

    // Get pointer to static random generator in VM
    VM::RNG& rnd = getVM(env).randomNumberGenerator();

    // Produces int (0 <= n <= max - 1)
    boost::uniform_int<> uni_dist(0, max - 1);
    boost::variate_generator<VM::RNG&,
        boost::uniform_int<> > uni(rnd, uni_dist);

    env.top(0).set_double(uni());
}

void
ActionMbLength(ActionExec& thread)
{
    as_environment& env = thread.env;
    
    std::string str = env.top(0).to_string();

    if (str.empty()) {
        env.top(0).set_double(0);
    }
    else {
        int length;
        std::vector<int> unused;
        unused.resize(str.length()+1);
        utf8::guessEncoding(str, length, unused);
        env.top(0).set_double(length);
    }
}

void
ActionOrd(ActionExec& thread)
{
    as_environment& env = thread.env;
    
    // Should return 0 
    const int swfVersion = thread.code.getDefinitionVersion();
    
    std::string str = env.top(0).to_string();
    
    if (str.empty()) {
        env.top(0).set_double(0);
        return;
    }

    std::wstring wstr = utf8::decodeCanonicalString(str, swfVersion);

    // decodeCanonicalString should correctly work out what the first
    // character is according to version.
    env.top(0).set_double(wstr.at(0));
}

void
ActionChr(ActionExec& thread)
{
    as_environment& env = thread.env;
    
    // This is UB:
    const boost::uint16_t c = toInt(env.top(0), getVM(env));

    // If the argument to chr() is '0', we return
    // nothing, not NULL
    if (c == 0) {
        env.top(0).set_string("");
        return;
    }
    
    const int swfVersion = thread.code.getDefinitionVersion();
    if (swfVersion > 5) {
        env.top(0).set_string(utf8::encodeUnicodeCharacter(c));
        return;
    }

    // SWF 5 only:
    // This casts to unsigned char to a string, giving
    // IS0-8859-1 8-bit characters.
    // Values above 256 evaluate to value % 256, 
    // through the cast, which is expected behaviour.
    const unsigned char uc = static_cast<unsigned char>(c);
    if (uc == 0) {
        env.top(0).set_string("");
        return;
    }
    env.top(0).set_string(std::string(1, uc));
}

void
ActionGetTimer(ActionExec& thread)
{
    as_environment& env = thread.env;
    const VM& vm = getVM(env);
    env.push(vm.getTime());
}

void
ActionMbSubString(ActionExec& thread)
{
    as_environment& env = thread.env;

    const as_value& arg0 = env.top(0);
    const as_value& arg1 = env.top(1);

    // Undefined values should resolve to 0.
    int size = toInt(env.top(0), getVM(env));
    int start = toInt(env.top(1), getVM(env));

    as_value& string_val = env.top(2);

    IF_VERBOSE_ACTION(
        log_action(" ActionMbSubString(%s, %d, %d)", string_val, arg0, arg1);
    );

    env.drop(2);

    const int version = env.get_version();
    std::string str = string_val.to_string(version);
    int length = 0;
    std::vector<int> offsets;

    utf8::EncodingGuess encoding = utf8::guessEncoding(str, length, offsets);

    if (size < 0) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Negative size passed to ActionSubString, "
            "taking as whole length"));
        );
        size = length;
    }

    if (start < 1) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Base is less then 1 in ActionMbSubString, "
            "setting to 1."));
        );
        start = 1;
    }
    else if ( start > length) {
        IF_VERBOSE_ASCODING_ERRORS (
            log_aserror(_("base goes beyond input string in ActionMbSubString, "
            "returning the empty string."));
        );
        env.top(0).set_string("");
        return;
    }

    // Adjust the start for our own use.
    --start;

    if (size + start > length) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("base+size goes beyond input string in ActionMbSubString, "
                "adjusting size based on length:%d and start:%d"), length, start);
        );
        size = length - start;
    }

    if (encoding == utf8::ENCGUESS_OTHER) {
        env.top(0).set_string(str.substr(start, size));
    }
    else {
        env.top(0).set_string(str.substr(offsets.at(start),
                        offsets.at(start + size) - offsets.at(start)));
    }
}

void
ActionMbOrd(ActionExec& thread)
{
    /// This only deals with UTF-8 characters.
    /// TODO: what else is possible?
    /// TODO: fix for SWF5
    as_environment& env = thread.env;

    if (env.get_version() == 5) {
        log_unimpl("Not properly implemented for SWF5");
        // No need to return - it works a bit.
    }

    const std::string s = env.top(0).to_string();
    
    std::string::const_iterator it = s.begin(), e = s.end();
    
    boost::uint32_t out = utf8::decodeNextUnicodeCharacter(it, e);
    
    /// Always valid, or can it be undefined?
    env.top(0).set_double(out);
}

void
ActionMbChr(ActionExec& thread)
{
    /// This only generates UTF-8 characters. No idea
    /// what difference user locale might make, but UTF-8
    /// is generally GOOD.
    
    /// TODO: fix for SWF5
    as_environment& env = thread.env;
    
    if (env.get_version() == 5) {
        log_unimpl(_("Not properly implemented for SWF5"));
        // No need to return.
    }

    // This is UB
    const boost::uint16_t i = toInt(env.top(0), getVM(env));
    
    std::string out = utf8::encodeUnicodeCharacter(i);
    
    /// Always valid, or can it be undefined?
    env.top(0).set_string(out);

}

/// Sets strict mode in the compiler.
//
/// This is irrelevant for execution, but included for completeness.
void
ActionStrictMode(ActionExec& thread)
{
    const action_buffer& code = thread.code;
    
    // off if 0, on for anything else.
    const bool on = code[thread.getCurrentPC() + 3];
    
    IF_VERBOSE_ACTION(
        log_action(_("ActionStrictMode set to %1%"), on);
    );
}

// also known as WaitForFrame2
void
ActionWaitForFrameExpression(ActionExec& thread)
{
    as_environment& env = thread.env;
    const action_buffer& code = thread.code;

    // how many actions to skip if frame has not been loaded
    const boost::uint8_t skip = code[thread.getCurrentPC() + 3];

    // env.top(0) contains frame specification,
    // evaluated as for ActionGotoExpression
    as_value framespec = env.pop();

    DisplayObject* tgtch = env.target();
    MovieClip* target_sprite = tgtch ? tgtch->to_movie() : 0;
    if (!target_sprite) {
        log_error(_("%s: environment target is null or not a MovieClip"),
            __FUNCTION__);
        return;
    }

    size_t framenum;
    if (!target_sprite->get_frame_number(framespec, framenum)) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Frame spec found on stack "
            "at ActionWaitForFrame doesn't evaluate "
                "to a valid frame: %s"),
            framespec);
        );
        return;
    }

    const size_t lastloaded = target_sprite->get_loaded_frames();
    if (lastloaded < framenum) {
        // better delegate this to ActionExec
        thread.skip_actions(skip);
    }

}

void
ActionPushData(ActionExec& thread)
{
    as_environment& env = thread.env;

    enum {
        pushString,  
        pushFloat,
        pushNull,
        pushUndefined,
        pushRegister,
        pushBool,
        pushDouble,
        pushInt32,
        pushDict8,
        pushDict16
    };
    const char* pushType[] = {
        "string",
        "float",
        "null",
        "undefined",
        "register",
        "bool",
        "double",
        "int",
        "dict8",
        "dict16"
    };

    const action_buffer& code = thread.code;

    const size_t pc = thread.getCurrentPC();
    const boost::uint16_t length = code.read_uint16(pc + 1);

    //---------------
    size_t i = pc;
    size_t count = 0;
    while (i - pc < length) {

        const boost::uint8_t type = code[3 + i];
        ++i;

        switch (type)
        {
            default:
            {
                IF_VERBOSE_MALFORMED_SWF(
                    log_swferror(_("Unknown push type %d. Execution will "
                        "continue but it is likely to fail due to lost "
                        "sync."), +type);
                );
                continue;
            }

            case pushString: // 0
            {
                const std::string str(code.read_string(i + 3));
                i += str.size() + 1; 
                env.push(str);
                break;
            }

            case pushFloat: // 1
            {
                const float f = code.read_float_little(i + 3);
                i += 4;
                env.push(f);
                break;
            }

            case pushNull: // 2
            {
                as_value nullvalue;
                nullvalue.set_null();
                env.push(nullvalue);
                break;
            }

            case pushUndefined: // 3
                env.push(as_value());
                break;

            case pushRegister: // 4
            {
                const size_t reg = code[3 + i];
                ++i;
                const as_value* v = getVM(env).getRegister(reg);
                if (!v) {
                    IF_VERBOSE_MALFORMED_SWF(
                        log_swferror(_("Invalid register %d in ActionPush"),
                            reg);
                    );
                    env.push(as_value());
                }
                else env.push(*v);
                break;
            }

            case pushBool: // 5
            {
                const bool bool_val = code[i + 3];
                ++i;
                env.push(bool_val);
                break;
            }

            case pushDouble: // 6
            {
                const double d = code.read_double_wacky(i + 3);
                i += 8;
                env.push(d);
                break;
            }

            case pushInt32: // 7
            {
                const boost::int32_t val = code.read_int32(i + 3);
                i += 4;
                env.push(val);
                break;
            }

            case pushDict8: // 8
            {
                const boost::uint8_t id = code[3 + i];
                ++i;
                if (id < code.dictionary_size()) {
                    env.push(code.dictionary_get(id));
                }
                else {
                    IF_VERBOSE_MALFORMED_SWF(
                        log_swferror(_("dict entry %d is out of bounds"), +id);
                    );
                    env.push(as_value());
                }
                break;
            }

            case pushDict16: // 9
            {
                const boost::uint16_t id = code.read_int16(i + 3);
                i += 2;
                if (id < code.dictionary_size()) {
                    env.push(code.dictionary_get(id));
                }
                else {
                    IF_VERBOSE_MALFORMED_SWF(
                        log_swferror(_("dict entry %d is out of bounds"), id);
                    );
                    env.push(as_value());
                }
                break;
            }
        }

        IF_VERBOSE_ACTION(
            log_action(_("\t%d) type=%s, value=%s"),
                count, pushType[type], env.top(0));
            ++count;
        );

    }
}

void
ActionBranchAlways(ActionExec& thread)
{
    boost::int16_t offset = thread.code.read_int16(thread.getCurrentPC()+3);
    thread.adjustNextPC(offset);
    // @@ TODO range checks
}

void
ActionGetUrl2(ActionExec& thread)
{
    as_environment& env = thread.env;

    const action_buffer& code = thread.code;

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_GETURL2));
#endif

    const boost::uint8_t method = code[thread.getCurrentPC() + 3];

    as_value url_val = env.top(1);
    if (url_val.is_undefined()) {
        log_error(_("Undefined GetUrl2 url on stack, skipping"));
    }
    else {
        const std::string& url = url_val.to_string();
        commonGetURL(env, env.top(0), url, method);
    }

    env.drop(2);
}

void
ActionBranchIfTrue(ActionExec& thread)
{
    as_environment& env = thread.env;
    const action_buffer& code = thread.code;
    size_t pc = thread.getCurrentPC();
    size_t nextPC = thread.getNextPC();
    size_t stopPC = thread.getStopPC();

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_BRANCHIFTRUE));
#endif

    boost::int16_t offset = code.read_int16(pc+3);

    const bool test = toBool(env.pop(), getVM(env));
    if (test) {
        thread.adjustNextPC(offset);

        if (nextPC > stopPC)
        {
            IF_VERBOSE_MALFORMED_SWF (
            log_swferror(_("branch to offset %d  -- "
                " this section only runs to %d"),
                    nextPC, stopPC);
            )
        }
    }
}

void
ActionCallFrame(ActionExec& thread)
{
    as_environment& env = thread.env;

    const std::string& target_frame = env.top(0).to_string();
    std::string target_path;
    std::string frame_var;

    DisplayObject* target = 0;
    if (parsePath(target_frame, target_path, frame_var)) {
        target = findTarget(env, target_path);
    }
    else {
        frame_var = target_frame;
        target = env.target();
    }

    MovieClip* target_sprite = target ? target->to_movie() : 0;
    if (target_sprite) {
        target_sprite->call_frame_actions(frame_var);
    }
    else {
        IF_VERBOSE_ASCODING_ERRORS (
        log_aserror(_("Couldn't find target_sprite \"%s\" in ActionCallFrame!"
            " target frame actions will not be called..."), target_path);
        )
    }

    env.drop(1);
}

void
ActionGotoExpression(ActionExec& thread)
{
    as_environment& env = thread.env;

    const action_buffer& code = thread.code;
    size_t pc = thread.getCurrentPC();

    // From Alexis SWF ref:
    //
    // Pop a value or a string and jump to the specified
    // frame. When a string is specified, it can include a
    // path to a sprite as in:
    //
    //   /Test:55
    //
    // When f_play is ON, the action is to play as soon as
    // that frame is reached. Otherwise, the
    // frame is shown in stop mode.
    unsigned char play_flag = code[pc + 3];
    const MovieClip::PlayState state = 
        play_flag ? MovieClip::PLAYSTATE_PLAY : MovieClip::PLAYSTATE_STOP;

    std::string target_frame = env.pop().to_string();
    std::string target_path;
    std::string frame_var;

    DisplayObject* target = NULL;
    if (parsePath(target_frame, target_path, frame_var)) {
        target = findTarget(env, target_path);
    }

    // 4.11 would make parsePath above return true,
    // we should check if a sprite named '4' is supposed to work
    // in that case
    if (!target) {
        target = env.target();
        frame_var = target_frame;
    }

    MovieClip *target_sprite = target ? target->to_movie() : NULL;
    if (target_sprite) {
        size_t frame_number;
        if (!target_sprite->get_frame_number(frame_var, frame_number)) {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Frame spec found on stack "
                "at ActionGotoExpression doesn't evaluate "
                "to a valid frame: %s"),
                target_frame);
            );
            return;
        }
        target_sprite->goto_frame(frame_number);
        target_sprite->setPlayState(state);
    }
    else {
        IF_VERBOSE_ASCODING_ERRORS (
        log_aserror(_("Couldn't find target sprite \"%s\" in "
                "ActionGotoExpression. Will not go to target frame..."),
                target_frame);
        )
    }
}


void
ActionDelete(ActionExec& thread)
{
    as_environment& env = thread.env;

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_DELETE)); // 0x3A
#endif

    // Stack                    Result
    // 'a'      |               nothing
    // 'a.b'    |               a.b deleted.
    // 'b'      | a [obj]       a.b deleted (normal use).
    // 'a.b'    | a [obj]       nothing.
    // 'a.b'    | 'string'      nothing.
    // 'c'      | a.b [obj]     a.b.c deleted (normal use).
    // 'a.b.c'  |               a.b.c deleted
    // 'b.c'    | a [obj]       nothing
    // 'a.b.c'  | string        nothing

    const size_t stackSize = env.stack_size();
    const int version = getSWFVersion(env);

    std::string propertyname = env.top(0).to_string();

    as_object* obj(0);

    // Behaviour is different according to version. For SWF7 and above,
    // the delete fails if there aren't two items on the stack. For SWF6
    // and below, a single item should be parsed to see if it's a path,
    // then we try to delete it. If it's not a path, we try to delete it as
    // a variable.
    //
    // In both cases, if there are two or more items on the stack, they
    // have to be property and object.
    if (stackSize < 2) {
        if (version > 6) {
            env.top(1).set_bool(false);
            env.drop(1);
            return;
        }

        std::string path, var;
        if (!parsePath(propertyname, path, var)) {
            // It's not a path. For SWF 7 and above, don't delete. Otherwise
            // assume it's a variable and try to delete.
            env.top(1).set_bool(thread.delVariable(propertyname));
            env.drop(1);
            return;
        }
        else {
            as_value target = thread.getVariable(path);
  
            // Don't create an object! Only get the value if it is an object
            // already.
            if (target.is_object()) {
                obj = safeToObject(getVM(thread.env), target);
                propertyname = var;
            }
        }
    }
    else {
        // Don't create an object! Only get the value if it is an object
        // already.
        if (env.top(1).is_object()) {
            obj = safeToObject(getVM(thread.env), env.top(1));
        }
    }

    if (!obj) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("delete %s.%s: no object found to delete"),
                        env.top(1), env.top(0));
        );
        env.top(1).set_bool(false);
        env.drop(1); 
        return;
    }

    VM& vm = getVM(env);
    const std::pair<bool, bool> ret = obj->delProperty(getURI(vm, propertyname));

    env.top(1).set_bool(ret.second);

    env.drop(1);
}

void
ActionDelete2(ActionExec& thread)
{
    as_environment& env = thread.env;

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_DELETE2)); // 0x3B
#endif

    const std::string& propertyname = env.top(0).to_string();

    // If it's not a path, delete it as a variable.
    std::string path, var;
    if (!parsePath(propertyname, path, var)) {
        // See bug #18482, this works fine now (assuming the bug
        // report is correct)
        env.top(0) = thread.delVariable(propertyname);
        return;
    }
    
    // Otherwise see if it's an object and delete it.
    as_value target = thread.getVariable(path);
    if (!target.is_object()) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("delete2 called with a path that does not resolve "
                    "to an object"), env.top(1), env.top(0));
        );
        env.top(1).set_bool(false);
        env.drop(1); 
        return;
    }

    as_object* obj = safeToObject(getVM(thread.env), target);

    VM& vm = getVM(env);
    const std::pair<bool, bool> ret = obj->delProperty(getURI(vm, var));

    env.top(1).set_bool(ret.second);
}

void
ActionVarEquals(ActionExec& thread)
{
    as_environment& env = thread.env;

    as_value& value = env.top(0);
    as_value& varname = env.top(1);
    thread.setLocalVariable(varname.to_string(), value);

    IF_VERBOSE_ACTION(
        log_action(_("-- set local var: %s = %s"), varname.to_string(), value);
    );

    env.drop(2);
}

void
ActionCallFunction(ActionExec& thread)
{
    as_environment& env = thread.env;

    // Let's consider it a as a string and lookup the function.
    //
    // Note: this produces the correct behaviour: Any as_value, including
    // null or numbers, are converted to a string and the corresponding
    // function is called. If it is undefined, nothing happens, even if
    // there is a function called 'undefined'.
    //
    // In all cases, even undefined, the specified number of arguments
    // is dropped from the stack.
    const std::string& funcname = env.pop().to_string();

    as_object* super(0);

    as_object* this_ptr;
    as_value function = thread.getVariable(funcname, &this_ptr);

    if (!function.is_object()) {
        // In this case the call to invoke() will fail. We won't return 
        // because we still need to handle the stack, and the attempt to
        // convert the value to an object may have effects in AS (haven't
        // checked).
        IF_VERBOSE_ASCODING_ERRORS (
            log_aserror(_("ActionCallFunction: %s is not an object"),
                funcname);
        )
    }
    else if (!function.is_function()) {
        as_object* obj = toObject(function, getVM(thread.env));
        super = obj->get_super();
        this_ptr = thread.getThisPointer();
    }

    // Get number of args, modifying it if not enough values are on the stack.
    // TODO: this may cause undefined behaviour if the number on the stack
    // is too large. Fix it.
    size_t nargs = toNumber(env.pop(), getVM(env));
    const size_t available_args = env.stack_size(); 
    if (available_args < nargs) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("Attempt to call a function with %u arguments "
                "while only %u are available on the stack."),
                nargs, available_args);
        );
        nargs = available_args;
    }

    fn_call::Args args;
    for (size_t i = 0; i < nargs; ++i) {
        args += env.pop();
    } 

    as_value result = invoke(function, env, this_ptr,
                  args, super, &(thread.code.getMovieDefinition()));

    env.push(result);

    // If the function threw an exception, do so here.
    if (result.is_exception()) {
        thread.skipRemainingBuffer();
    }

}

void
ActionReturn(ActionExec& thread)
{
    as_environment& env = thread.env;

    // Put top of stack in the provided return slot, if
    // it's not NULL.
    thread.pushReturn(env.top(0));
    env.drop(1);

    // Skip the rest of this buffer (return from this action_buffer).
    thread.skipRemainingBuffer();

}

void
ActionModulo(ActionExec& thread)
{
    as_environment& env = thread.env;

    const double y = toNumber(env.pop(), getVM(env));
    const double x = toNumber(env.pop(), getVM(env));
    // Don't need to check for y being 0 here - if it's zero,
    // fmod returns NaN, which is what flash would do too
    as_value result = std::fmod(x, y);

    env.push(result);
}

void
ActionNew(ActionExec& thread)
{
    as_environment& env = thread.env;

    as_value val = env.pop();
    const std::string& classname = val.to_string();

    IF_VERBOSE_ACTION (
        log_action(_("---new object: %s"),
            classname);
    );

    unsigned nargs = toNumber(env.pop(), getVM(env));

    as_value constructorval = thread.getVariable(classname);
    as_function* constructor = constructorval.to_function();
    if (!constructor) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("ActionNew: "
                "'%s' is not a constructor"), classname);
        );
        env.drop(nargs);
        env.push(as_value()); // should we push an object anyway ?
        return;
    }

    // It is possible for constructors to fail, for instance if a
    // conversion to object calls a built-in constructor that has been
    // deleted. BitmapData also fails to construct anything under
    // some circumstances.
    try {
        as_object* newobj = construct_object(constructor, env, nargs);
        env.push(newobj);
        return;
    }
    catch (const GnashException& ) {
        env.push(as_value());
        return;
    }

}

void
ActionVar(ActionExec& thread)
{
    as_environment& env = thread.env;
    
    const std::string& varname = env.top(0).to_string();
    const ObjectURI& name = getURI(getVM(env), varname);
    VM& vm = getVM(env);

    if (vm.calling()) {
        declareLocal(vm.currentCall(), name);
    }
    else {
       IF_VERBOSE_ASCODING_ERRORS(
           log_aserror(_("The 'var whatever' syntax in timeline context is a "
                   "no-op."));
       );
    }
    env.drop(1);
}

void
ActionInitArray(ActionExec& thread)
{
    as_environment& env = thread.env;

    const int array_size = toInt(env.pop(), getVM(env));
    assert(array_size >= 0); // TODO: trigger this !!
    
    Global_as& gl = getGlobal(env);

    as_object* ao = gl.createArray();

    VM& vm = getVM(env);
    // Fill the elements with the initial values from the stack.
    for (int i = 0; i < array_size; i++) {
        const ObjectURI& k = 
            getURI(vm, boost::lexical_cast<std::string>(i));
        ao->set_member(k, env.pop());
    }

    env.push(ao);

}

void
ActionInitObject(ActionExec& thread)
{
    as_environment& env = thread.env;

    //
    //    SWFACTION_PUSH
    //     [000]   Constant: 1 "obj"
    //     [001]   Constant: 0 "member" <-- we handle up to here
    //     [002]   Integer: 1
    //     [003]   Integer: 1
    //    SWFACTION_INITOBJECT

    const int nmembers = toInt(env.pop(), getVM(env));

    // TODO: see if this could call the ASnative function(101, 9).
    Global_as& gl = getGlobal(env);
    as_object* obj = createObject(gl);

    obj->init_member(NSV::PROP_CONSTRUCTOR, getMember(gl, NSV::CLASS_OBJECT));

    VM& vm = getVM(env);

    // Set provided members
    for (int i = 0; i < nmembers; ++i) {
        const as_value& member_value = env.top(0);
        std::string member_name = env.top(1).to_string();
        obj->set_member(getURI(vm, member_name), member_value);
        env.drop(2);
    }

    as_value new_obj;
    new_obj.set_as_object(obj);

    env.push(new_obj);

}

void
ActionTypeOf(ActionExec& thread)
{
    as_environment& env = thread.env;
    env.top(0).set_string(env.top(0).typeOf());
}

void
ActionTargetPath(ActionExec& thread)
{
    as_environment& env = thread.env;

    DisplayObject* sp = env.top(0).toDisplayObject();
    if (sp) {
        env.top(0).set_string(sp->getTarget());
        return;
    }

    IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Argument to TargetPath(%s) doesn't cast "
                "to a DisplayObject"), env.top(0));
    );
    env.top(0).set_undefined();
}

// Push a each object's member value on the stack
// This is an utility function for use by ActionEnumerate
// and ActionEnum2. The caller is expected to have
// already set the top-of-stack to undefined (as an optimization)
static void
enumerateObject(as_environment& env, const as_object& obj)
{
    assert(env.top(0).is_undefined());
    Enumerator en(env);
    obj.visitKeys(en);
}

void
ActionEnumerate(ActionExec& thread)
{
    as_environment& env = thread.env;

    // Get the object
    as_value var_name = env.top(0);
    std::string var_string = var_name.to_string();

    as_value variable = thread.getVariable(var_string);

    env.top(0).set_undefined();

    const as_object* obj = safeToObject(getVM(thread.env), variable);
    if ( !obj || !variable.is_object() )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Top of stack doesn't evaluate to an object (%s) at "
            "ActionEnumerate execution"),
            var_name);
        );
        return;
    }

    enumerateObject(env, *obj);
}

void
ActionNewAdd(ActionExec& thread)
{
    as_environment& env = thread.env;

    newAdd(env.top(1), env.top(0), getVM(env));

    env.drop(1);
}

void
ActionNewLessThan(ActionExec& thread)
{
    as_environment& env = thread.env;
    env.top(1) = newLessThan(env.top(1), env.top(0), getVM(env));
    env.drop(1);
}

void
ActionNewEquals(ActionExec& thread)
{
    as_environment& env = thread.env;

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_NEWEQUALS));
#endif

    VM& vm = getVM(env);

    int swfVersion = vm.getSWFVersion();
    if (swfVersion <= 5)
    {
        as_value op1 = env.top(0);
        try { convertToPrimitive(op1, vm); }
        catch (const ActionTypeError& e) {
            log_debug(_("to_primitive(%s) threw an ActionTypeError %s"),
                    op1, e.what());
        }

        as_value op2 = env.top(1);
        try { convertToPrimitive(op2, vm); }
        catch (const ActionTypeError& e) {
            log_debug(_("to_primitive(%s) threw an ActionTypeError %s"),
                    op2, e.what());
        }

        env.top(1).set_bool(equals(op1, op2, getVM(env)));
    }
    else
    {
        /// ECMA-262 abstract equality comparison (sect 11.9.3)
        env.top(1).set_bool(equals(env.top(1), env.top(0), getVM(env)));
    }
    env.drop(1);
}

void
ActionToNumber(ActionExec& thread)
{
    as_environment& env = thread.env;
    convertToNumber(env.top(0), getVM(env));
}

void
ActionToString(ActionExec& thread)
{
    as_environment& env = thread.env;
    convertToString(env.top(0), getVM(env));
}

void
ActionDup(ActionExec& thread)
{
    as_environment& env = thread.env;
    env.push(env.top(0));
}

void
ActionSwap(ActionExec& thread)
{
    as_environment& env = thread.env;
    std::swap(env.top(1), env.top(0));
}

void
ActionGetMember(ActionExec& thread)
{
    as_environment& env = thread.env;

    as_value member_name = env.top(0);
    as_value target = env.top(1);

    as_object* obj = safeToObject(getVM(thread.env), target);
    if (!obj) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("getMember called against a value that does not "
                    "cast to an as_object: %s"), target)
        );
        env.top(1).set_undefined();
        env.drop(1);
        return;
    }

    IF_VERBOSE_ACTION(
        log_action(_(" ActionGetMember: target: %s (object %p)"),
                   target, static_cast<void*>(obj));
    );

    const ObjectURI& k = getURI(getVM(env), member_name.to_string());

    if (!obj->get_member(k, &env.top(1))) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror("Reference to undefined member %s of object %s",
                member_name, target);
        );
        env.top(1).set_undefined();
    }

    IF_VERBOSE_ACTION (
        log_action(_("-- get_member %s.%s=%s"),
           target, member_name, env.top(1));
    );

    env.drop(1);
}

void
ActionSetMember(ActionExec& thread)
{
    as_environment& env = thread.env;

    as_object* obj = safeToObject(getVM(thread.env), env.top(2));
    const std::string& member_name = env.top(1).to_string();
    const as_value& member_value = env.top(0);

    if (member_name.empty()) {
        IF_VERBOSE_ASCODING_ERRORS (
            // Invalid object, can't set.
            log_aserror(_("ActionSetMember: %s.%s=%s: member name "
                    "evaluates to invalid (empty) string"),
                    env.top(2), env.top(1), env.top(0));
        );
    }
    else if (obj) {
        obj->set_member(getURI(getVM(env), member_name), member_value);

        IF_VERBOSE_ACTION (
            log_action(_("-- set_member %s.%s=%s"),
                env.top(2),
                member_name,
                member_value);
        );
    }
    else {
        // Malformed SWF ? (don't think this is possible to do with
        // ActionScript syntax)
        // FIXME, should this be log_swferror?
        IF_VERBOSE_ASCODING_ERRORS(
            // Invalid object, can't set.
            log_aserror(_("-- set_member %s.%s=%s on invalid object!"),
                env.top(2), member_name, member_value);
        );
    }

    env.drop(3);
}

void
ActionIncrement(ActionExec& thread)
{
    as_environment& env = thread.env;
    env.top(0).set_double(toNumber(env.top(0), getVM(env)) + 1);
}

void
ActionDecrement(ActionExec& thread)
{
    as_environment& env = thread.env;
    env.top(0).set_double(toNumber(env.top(0), getVM(env)) - 1);
}


/// Call a method of an object
//
/// Stack: method, object, argc, arg0 ... argn
//
/// The standard use of this opcode is:
/// 1. First stack value converted to a string (method name).
/// 2. Second stack value converted to an object (this pointer).
/// 3. Arg count and arguments parsed.
/// 4. The method name must be a property of the object and is called with
///    the object as its 'this'.
//
/// But it can also be used in a different way under some circumstances.
//
/// 1. If the method name is defined and not empty, the object value must
///    be an object and the method name must be a property of the object
///    (may be inherited). Otherwise the call fails and returns undefined.
/// 2. If the method name is undefined or empty, the second stack value is
///    called, and call's 'this' pointer is undefined.
//
/// In both usages the arguments are passed.
void
ActionCallMethod(ActionExec& thread)
{
    as_environment& env = thread.env;

    // Get name function of the method
    as_value method_name = env.pop();

    std::string method_string = method_name.to_string();
    
    // Get an object
    as_value obj_value = env.pop();

    // Get number of args, modifying it if not enough values are on the stack.
    size_t nargs = toNumber(env.pop(), getVM(env));
    const size_t available_args = env.stack_size(); 
    if (available_args < nargs) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("Attempt to call a method with %u arguments "
                "while only %u are available on the stack."),
                nargs, available_args);
        );
        nargs = available_args;
    }

    IF_VERBOSE_ACTION (
        log_action(_(" method name: %s"), method_name);
        log_action(_(" method object/func: %s"), obj_value);
        log_action(_(" method nargs: %d"), nargs);
    );

    as_object* obj = safeToObject(getVM(thread.env), obj_value);
    if (!obj) {
        // If this value is not an object, it can neither have any members
        // nor be called as a function, so neither opcode usage is possible.
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("ActionCallMethod invoked with "
                "non-object object/func (%s)"), obj_value);
        );
        env.drop(nargs);
        env.push(as_value());
        return;
    }

    const bool noMeth = (method_name.is_undefined() || method_string.empty());

    as_object* method_obj; // The method to call, as an object

    // The object to be the 'this' pointer during the call.
    as_object* this_ptr(0);

    // Will be used to find super later
    ObjectURI methURI;

    // If the method name is undefined or evaluates to an empty string,
    // the first argument is used as the method name and the 'this' pointer
    // is undefined. We can signify this by leaving the 'this' pointer as
    // null.a
    if (noMeth) {
        method_obj = obj;
    }
    else {

        methURI = getURI(getVM(env), method_string);

        // The method value
        as_value method_value; 

        if (!obj->get_member(methURI, &method_value) ) {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("ActionCallMethod: "
                "Can't find method %s of object %s"),
                method_name, obj_value);
            );
            env.drop(nargs);
            env.push(as_value()); 
            return;
        }

        method_obj = safeToObject(getVM(thread.env), method_value);
        if ( ! method_obj ) {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("ActionCallMethod: "
                    "property %d of object %d is not callable (%s)"),
                    method_name, obj_value, method_value);
            );
            env.drop(nargs);
            env.push(as_value()); 
            return;
        }
        this_ptr = obj;
    }

    assert(method_obj); // or we would should have returned already by now

    // If we are calling a method of a super object, the 'this' pointer
    // for the call is always the this pointer of the function that called
    // super().
    if (obj->isSuper()) {
        if (thread.isFunction()) this_ptr = thread.getThisPointer();
    }

    fn_call::Args args;
    for (size_t i = 0; i < nargs; ++i) {
        args += env.pop();
    } 

    as_object* super;
    as_function* func = method_obj->to_function();
    if (func && func->isBuiltin()) {
        // Do not construct super if method is a builtin
        // TODO: check if this is correct!!
        super = 0;
    }
    else {
        super = obj->get_super(methURI);
    }

    fn_call call(this_ptr, env, args);
    call.super = super;
    call.callerDef = &(thread.code.getMovieDefinition());
    as_value result;
    try {
        result = method_obj->call(call);
    } 
    catch (const ActionTypeError& e) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror("ActionCallMethod: %s", e.what());
        );
    }

    env.push(result);

    // Now, if there was an exception, proceed to the end of the block.
    if (result.is_exception()) thread.skipRemainingBuffer();

}

void
ActionNewMethod(ActionExec& thread)
{
    as_environment& env = thread.env;

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_NEWMETHOD));
#endif

    const as_value method_name = env.pop();
    const as_value obj_val = env.pop();

    // Get number of args, modifying it if not enough values are on the stack.
    unsigned nargs = toNumber(env.pop(), getVM(env));
    const size_t available_args = env.stack_size(); // previous 3 entries popped
    if (available_args < nargs) {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("Attempt to call a constructor with %u arguments "
            "while only %u are available on the stack."),
            nargs, available_args);
        );
        nargs = available_args;
    }

    as_object* obj = safeToObject(getVM(thread.env), obj_val);
    if (!obj) {
        // SWF integrity check
        // FIXME, should this be log_swferror?  Or log_aserror?
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("On ActionNewMethod: "
                "no object found on stack on ActionMethod"));
        );
        env.drop(nargs);
        env.push(as_value());
        return;
    }

    const std::string& method_string = method_name.to_string();
    as_value method_val;
    if (method_name.is_undefined() || method_string.empty()) {
        method_val = obj_val;
    }
    else {
        const ObjectURI& k = getURI(getVM(env), method_string);
        if (!obj->get_member(k, &method_val)) {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("ActionNewMethod: can't find method %s of "
                        "object %s"), method_string, obj_val);
            );
            env.drop(nargs);
            env.push(as_value()); 
            return;
        }
    }

    as_function* method = method_val.to_function();
    if (!method) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("ActionNewMethod: method name is undefined "
                "and object is not a function"));
        );
        env.drop(nargs);
        env.push(as_value()); 
        return;
    }

    // Construct the object
    // It is possible for constructors to fail, for instance if a
    // conversion to object calls a built-in constructor that has been
    // deleted. BitmapData also fails to construct anything under
    // some circumstances.
    try {
        as_object* newobj = construct_object(method, env, nargs);
        env.push(newobj);
        return;
    }
    catch (const GnashException&) {
        env.push(as_value());
        return;
    }
}

void
ActionInstanceOf(ActionExec& thread)
{
    as_environment& env = thread.env;

    // Get the "super" function
    as_object* super = safeToObject(getVM(thread.env), env.top(0));

    // Get the "instance" (but avoid implicit conversion of primitive values!)
    as_object* instance = env.top(1).is_object() ?
        safeToObject(getVM(thread.env), env.top(1)) : 0;

    // Invalid args!
    if (!super || ! instance) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("-- %s instanceof %s (invalid args?)"),
                env.top(1), env.top(0));
        );

        env.drop(1);
        env.top(0) = as_value(false);
        return;
    }

    env.drop(1);
    env.top(0) = as_value(instance->instanceOf(super));
}

void
ActionEnum2(ActionExec& thread)
{
    as_environment& env = thread.env;

    // Get the object.
    // Copy it so we can override env.top(0)
    as_value obj_val = env.top(0);

    // End of the enumeration. Won't override the object
    // as we copied that as_value.
    env.top(0).set_undefined();

    as_object* obj = safeToObject(getVM(thread.env), obj_val);
    if (!obj || !obj_val.is_object()) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Top of stack not an object %s at ActionEnum2 "
            " execution"), obj_val);
        );
        return;
    }

    enumerateObject(env, *obj);
}

void
ActionBitwiseAnd(ActionExec& thread)
{
    as_environment& env = thread.env;

    const int operand1 = toInt(env.top(1), getVM(env));
    const int operand2 = toInt(env.top(0), getVM(env));

    env.top(1) = operand1 & operand2;
    env.drop(1);
}

void
ActionBitwiseOr(ActionExec& thread)
{
    as_environment& env = thread.env;

    const int operand1 = toInt(env.top(1), getVM(env));
    const int operand2 = toInt(env.top(0), getVM(env));

    env.top(1) = operand1|operand2;
    env.drop(1);
}

void
ActionBitwiseXor(ActionExec& thread)
{
    as_environment& env = thread.env;

    const int operand1 = toInt(env.top(1), getVM(env));
    const int operand2 = toInt(env.top(0), getVM(env));

    env.top(1) = operand1^operand2;
    env.drop(1);
}

void
ActionShiftLeft(ActionExec& thread)
{
    as_environment& env = thread.env;

    /// A left shift of more than or equal to the size in
    /// bits of the left operand, or a negative shift, results
    /// in undefined behaviour in C++.
    boost::int32_t amount = toInt(env.top(0), getVM(env)) % 32;
    if (amount < 0) amount += 32;
    
    boost::int32_t value = toInt(env.top(1), getVM(env));

    value = value << amount;

    env.top(1) = value;
    env.drop(1);
}

void
ActionShiftRight(ActionExec& thread)
{
    as_environment& env = thread.env;

    // This is UB.
    boost::uint32_t amount = toInt(env.top(0), getVM(env));
    boost::int32_t value = toInt(env.top(1), getVM(env));

    value = value >> amount;

    env.top(1) = value;
    env.drop(1);
}

void
ActionShiftRight2(ActionExec& thread)
{
    as_environment& env = thread.env;

    // This is UB
    boost::uint32_t amount = toInt(env.top(0), getVM(env)); 
    boost::int32_t value = toInt(env.top(1), getVM(env));

    value = boost::uint32_t(value) >> amount;

    env.top(1) = value;
    env.drop(1);
}

void
ActionStrictEq(ActionExec& thread)
{
    as_environment& env = thread.env;
    
    env.top(1).set_bool(env.top(1).strictly_equals(env.top(0)));
    env.drop(1);
}

void
ActionGreater(ActionExec& thread)
{
    // Just swap the operator and invoke ActionNewLessThan
    as_environment& env = thread.env;
    std::swap(env.top(1), env.top(0));
    ActionNewLessThan(thread);
}

void
ActionStringGreater(ActionExec& thread)
{
    as_environment& env = thread.env;
    
    const std::string& op1 = env.top(0).to_string();
    const std::string& op2 = env.top(1).to_string();
    env.top(1).set_bool(op2 > op1);
    env.drop(1);
}

void
ActionExtends(ActionExec& thread)
{
    as_environment& env = thread.env;

    as_object* super = toObject(env.top(0), getVM(thread.env));
    as_function* sub = env.top(1).to_function();

    if (!super ||!sub) {
        IF_VERBOSE_ASCODING_ERRORS(
            if (!super) {
                log_aserror(_("ActionExtends: Super is not an object (%s)"),
                    env.top(0));
            }
            if (!sub) {
                log_aserror(_("ActionExtends: Sub is not a function (%s)"),
                    env.top(1));
            }
        );
        env.drop(2);
        return;
    }
    env.drop(2);

    as_object* newproto = new as_object(getGlobal(thread.env));
    as_object* p =
        toObject(getMember(*super, NSV::PROP_PROTOTYPE), getVM(thread.env));
    newproto->set_prototype(p);

    if (getSWFVersion(*super) > 5) {
        const int flags = PropFlags::dontEnum;
        newproto->init_member(NSV::PROP_uuCONSTRUCTORuu, super, flags);
    }

    sub->init_member(NSV::PROP_PROTOTYPE, as_value(newproto));

}

void
ActionConstantPool(ActionExec& thread)
{
    thread.code.process_decl_dict(thread.getCurrentPC(), thread.getNextPC());
}

void
ActionDefineFunction2(ActionExec& thread)
{
    as_environment& env = thread.env;
    const action_buffer& code = thread.code;

    // Code starts at thread.getNextPC() as the DefineFunction tag
    // contains name and args, while next tag is first tag
    // of the function body.
    Function2* func = new Function2(code, env, thread.getNextPC(),
            thread.getScopeStack());
    
    // We're stuck initializing our own prototype at the moment.
    as_object* proto = createObject(getGlobal(env));
    proto->init_member(NSV::PROP_CONSTRUCTOR, func); 
    func->init_member(NSV::PROP_PROTOTYPE, proto);
    
    Global_as& gl = getGlobal(env);
    
    as_function* f = getOwnProperty(gl, NSV::CLASS_FUNCTION).to_function();
    if (f) {
        const int flags = as_object::DefaultFlags | PropFlags::onlySWF6Up;
        func->init_member(NSV::PROP_uuPROTOuu, getMember(*f,
                    NSV::PROP_PROTOTYPE), flags);
        func->init_member(NSV::PROP_CONSTRUCTOR, f);
    }

    size_t i = thread.getCurrentPC() + 3; // skip tag id and length

    // Extract name.
    // @@ security: watch out for possible missing terminator here!
    const std::string name = code.read_string(i);
    i += name.length() + 1; // add NULL-termination

    // Get number of arguments.
    const boost::uint16_t nargs = code.read_uint16(i);
    i += 2;

    // Get the count of local registers used by this function.
    const boost::uint8_t register_count = code[i];
    ++i;

    func->setRegisterCount(register_count);

    // Flags, for controlling register assignment of implicit args.
    const boost::uint16_t flags = code.read_uint16(i);
    i += 2;

    func->setFlags(flags);

    // Get the register assignments and names of the arguments.
    for (size_t n = 0; n < nargs; ++n) {
        boost::uint8_t arg_register = code[i];
        ++i;

        // @@ security: watch out for possible missing terminator here!
        const std::string arg(code.read_string(i));

        func->add_arg(arg_register, getURI(getVM(env), arg));
        i += arg.size() + 1;
    }

    // Get the length of the actual function code.
    boost::uint16_t code_size = code.read_int16(i);

    // Check code_size value consistency
    const size_t actionbuf_size = thread.code.size();
    if (thread.getNextPC() + code_size > actionbuf_size) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("function2 code len (%u) "
                "overflows DOACTION tag boundaries "
                "(DOACTION tag len=%d"
                ", function2 code offset=%d). "
                "Forcing code len to eat the whole buffer "
                "(would this work?)."),
                code_size, actionbuf_size, thread.getNextPC());
        );
        code_size = actionbuf_size-thread.getNextPC();
    }

    i += 2;
    func->setLength(code_size);

    // Skip the function body (don't interpret it now).
    thread.adjustNextPC(code_size);

    // If we have a name, then save the function in this
    // environment under that name.
    as_value function_value(func);
    if (!name.empty()) {
        IF_VERBOSE_ACTION(
            log_action(_("DefineFunction2: named function '%s' "
                        "starts at PC %d"), name, func->getStartPC());
        );

        thread.setVariable(name, function_value);
    }

    // Otherwise push the function literal on the stack
    else {
        IF_VERBOSE_ACTION(
            log_action(_("DefineFunction2: anonymous function starts at "
                        "PC %d"), func->getStartPC());
        );
        env.push(function_value);
    }
}

void
ActionTry(ActionExec& thread)
{
    const action_buffer& code = thread.code;

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_TRY));
#endif

    size_t i = thread.getCurrentPC() + 3; // skip tag id and length

    boost::uint8_t flags = code[i];
    ++i;

    bool doCatch = flags & 1;
    bool doFinally = flags & (1<<1);
    bool catchInRegister = flags&(1<<2);
    boost::uint8_t reserved = flags&0xE0;

    boost::uint16_t trySize = code.read_uint16(i); i += 2;
    boost::uint16_t catchSize = code.read_uint16(i); i += 2;
    boost::uint16_t finallySize = code.read_uint16(i); i += 2;

    const char* catchName = NULL;
    boost::uint8_t catchRegister = 0;

    if (!doFinally) finallySize = 0;
    if (!doCatch) catchSize = 0;

    if (!catchInRegister) {
        catchName = code.read_string(i);
        i += strlen(catchName) + 1;
        thread.pushTryBlock(
                TryBlock(i, trySize, catchSize, finallySize, catchName));
    }
    else {
        catchRegister = code[i];
        ++i;
        thread.pushTryBlock(
                TryBlock(i, trySize, catchSize, finallySize, catchRegister));
    }

    thread.setNextPC(i); // Proceed into the try block.

    IF_VERBOSE_ACTION(
    log_action(_("ActionTry: reserved:%x doFinally:%d doCatch:%d trySize:%u "
                "catchSize:%u finallySize:%u catchName:%s catchRegister:%u"),
                static_cast<int>(reserved), doFinally, doCatch, trySize,
                catchSize, finallySize,
                catchName ? catchName : "(null)", catchRegister);
    );
}

/// See: http://sswf.sourceforge.net/SWFalexref.html#action_with
void
ActionWith(ActionExec& thread)
{
    as_environment& env = thread.env;
    const action_buffer& code = thread.code;
    size_t pc = thread.getCurrentPC();

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_WITH));
#endif

    const as_value& val = env.pop();
    as_object* with_obj = toObject(val, getVM(thread.env));

    ++pc; // skip tag code

    int tag_length = code.read_int16(pc); // read tag len (should be 2)
    if ( tag_length != 2 )
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("ActionWith tag length != 2; skipping"));
        );
        return;
    }
    pc += 2; // skip tag len

    unsigned block_length = code.read_int16(pc); // read 'with' body size
    if ( block_length == 0 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Empty with() block..."));
        );
        return;
    }
    pc += 2; // skip with body size

    // now we should be on the first action of the 'with' body
    assert(thread.getNextPC() == pc);

    if (!with_obj) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("with(%s) : first argument doesn't cast to an object!"),
            val);
        );
        // skip the full block
        thread.adjustNextPC(block_length);
        return;
    }

    // where does the 'with' block end?
    const size_t block_end = thread.getNextPC() + block_length;

    if (!thread.pushWith(With(with_obj, block_end))) {
        // skip the full block
        thread.adjustNextPC(block_length);
    }

}

void
ActionDefineFunction(ActionExec& thread)
{
    as_environment& env = thread.env;
    const action_buffer& code = thread.code;

#ifndef NDEBUG
    // TODO: check effects of the following 'length' 
    boost::int16_t length = code.read_int16(thread.getCurrentPC()+1);
    assert( length >= 0 );
#endif

    // Create a new Function
    // Code starts at thread.getNextPC() as the DefineFunction tag
    // contains name and args, while next tag is first tag
    // of the function body.
    Function* func = new Function(code, env, thread.getNextPC(),
            thread.getScopeStack());
    
    // We're stuck initializing our own prototype at the moment.
    as_object* proto = createObject(getGlobal(env));
    proto->init_member(NSV::PROP_CONSTRUCTOR, func); 
    func->init_member(NSV::PROP_PROTOTYPE, proto);
    
    Global_as& gl = getGlobal(env);
    
    as_function* f = getOwnProperty(gl, NSV::CLASS_FUNCTION).to_function();
    if (f) {
        const int flags = as_object::DefaultFlags | PropFlags::onlySWF6Up;
        func->init_member(NSV::PROP_uuPROTOuu, getMember(*f,
                    NSV::PROP_PROTOTYPE), flags);
        func->init_member(NSV::PROP_CONSTRUCTOR, f);
    }

    size_t i = thread.getCurrentPC() + 3;

    // Extract name.
    // @@ security: watch out for possible missing terminator here!
    const std::string name = code.read_string(i);
    i += name.length() + 1;

    // Get number of arguments.
    const size_t nargs = code.read_uint16(i);
    i += 2;
    
    // Get the names of the arguments.
    for (size_t n = 0; n < nargs; ++n) {
        const std::string arg(code.read_string(i));
        func->add_arg(0, getURI(getVM(env), arg));
        i += arg.size() + 1; 
    }

    // Get the length of the actual function code.
    const boost::uint16_t code_size = code.read_uint16(i);

    func->setLength(code_size);

    // Skip the function body (don't interpret it now).
    // getNextPC() is assumed to point to first action of
    // the function body (one-past the current tag, whic
    // is DefineFunction). We add code_size to it.
    thread.adjustNextPC(code_size);

    // If we have a name, then save the function in this
    // environment under that name.
    as_value function_value(func);
    if (!name.empty()) {
        IF_VERBOSE_ACTION(
            log_action("DefineFunction: named function '%s' starts at "
                        "PC %d", name, func->getStartPC());
        );
        thread.setVariable(name, function_value);
    }
    else {
        // Otherwise push the function literal on the stack
        IF_VERBOSE_ACTION(
            log_action("DefineFunction: anonymous function starts at "
                        "PC %d", func->getStartPC());
        );
        env.push(function_value);
    }
}

void
ActionSetRegister(ActionExec& thread)
{
    as_environment& env = thread.env;
    const action_buffer& code = thread.code;
    const size_t reg = code[thread.getCurrentPC() + 3];
    // Save top of stack in specified register.
    getVM(env).setRegister(reg, env.top(0));
}


void
ActionUnsupported(ActionExec& thread)
{
    log_error(_("Unsupported action handler invoked, code at pc is %#x"),
            static_cast<int>(thread.code[thread.getCurrentPC()]));
}

as_object*
safeToObject(VM& vm, const as_value& val)
{
    try {
        return toObject(val, vm);
    }
    catch (const GnashException&) {
        return 0;
    }
}

// Utility: construct an object using given constructor.
// This is used by both ActionNew and ActionNewMethod and
// hides differences between builtin and actionscript-defined
// constructors.
as_object*
construct_object(as_function* ctor_as_func, as_environment& env,
        unsigned int nargs)
{
    assert(ctor_as_func);
    fn_call::Args args;
    for (size_t i = 0; i < nargs; ++i) {
        args += env.pop();
    } 
    return constructInstance(*ctor_as_func, env, args);
}

// Common code for GetUrl and GetUrl2. See:
// http://sswf.sourceforge.net/SWFalexref.html#action_get_url
// http://sswf.sourceforge.net/SWFalexref.html#action_get_url2
//
// Testcases:
//
// - http://www.garfield.com/comics/comics_todays.html
//   lower_todayscomic.swf should render four flash files in its canvas
//
// - http://www.voiptalk.org
//   pressing 'My Account' button should open
//   https://www.voiptalk.org/products/login.php
//   NOTE: this is affected by the GetUrl bug reported with an excerpt
//         from Colin Moock book, see below. (won't work, and won't fix)
//
// - http://www.uptoten.com
//   Should load in _level0, with loadTargetFlag set.
//
// Method isbBit-packed as follows:
// SendVarsMethod:2 (0:NONE 1:GET 2:POST)
// Reserved:4
// LoadTargetFlag:1
// LoadVariableFlag:1
/// @param target        the target window, or _level1..10
void
commonGetURL(as_environment& env, as_value target,
        const std::string& url, boost::uint8_t method)
{
    if (url.empty()) {
        log_error(_("Bogus empty GetUrl url in SWF file, skipping"));
        return;
    }

    // Parse the method bitfield
    bool loadTargetFlag    = method & 64;
    bool loadVariableFlag  = method & 128;

    MovieClip::VariablesMethod sendVarsMethod;

    // handle malformed sendVarsMethod
    if ((method & 3) == 3) {
        log_error(_("Bogus GetUrl2 send vars method "
            " in SWF file (both GET and POST requested). Using GET"));
        sendVarsMethod = MovieClip::METHOD_GET;
    }
    else {
        sendVarsMethod = static_cast<MovieClip::VariablesMethod>(method & 3);
    }

    std::string target_string;
    if (!target.is_undefined() && !target.is_null()) {
        target_string = target.to_string();
    }

    VM& vm = getVM(env);
    movie_root& m = vm.getRoot();
 
    // If the url starts with "FSCommand:", then this is
    // a message for the host app.
    StringNoCaseEqual noCaseCompare;
    if (noCaseCompare(url.substr(0, 10), "FSCommand:")) {
        m.handleFsCommand(url.substr(10), target_string);
        return;
    }

    // If the url starts with "print:", then this is
    // a print request.
    if (noCaseCompare(url.substr(0, 6), "print:")) {
        log_unimpl("print: URL");
        return;
    }

    //
    // From "ActionScript: The Definitive Guide" by Colin Moock p. 470
    // --------8<------------------------------------------------------
    // In most browsers, getURL() relative links are resolved relative
    // to the HTML file that contains the .swf file. In IE 4.5 and older
    // versions on Macintosh, relative links are resolved relative to
    // the location of the .swf file, not the HTML file, which causes
    // problems when the two are in different directories. To avoid
    // the problem, either place the .swf and the .html file in the
    // same directory or use absolute URLs when invoking getURL().
    // --------8<------------------------------------------------------
    //
    // We'll resolve relative to our "base url".
    // The base url must be set with the set_base_url() command.
    //

    log_debug(_("get url: target=%s, url=%s, method=%x "
                "(sendVars:%X, loadTarget:%d, loadVariable:%d)"),
            target_string, url, static_cast<int>(method),
            sendVarsMethod, loadTargetFlag, loadVariableFlag);

    DisplayObject* target_ch = findTarget(env, target_string);
    MovieClip* target_movie = target_ch ? target_ch->to_movie() : 0;

    if (loadVariableFlag) {
        log_debug(_("getURL2 loadVariable"));

        if (!target_ch) {
            log_error(_("getURL: target %s not found"), target_string);
            // might want to invoke the external url opener here...
            return;
        }

        if (!target_movie) {
            log_error(_("getURL: target %s is not a sprite"), target_string);
            // might want to invoke the external url opener here...
            return;
        }

        target_movie->loadVariables(url, sendVarsMethod);

        return;
    }

    std::string varsToSend;
    if (sendVarsMethod != MovieClip::METHOD_NONE) {

        // TESTED: variables sent are those in current target,
        //         no matter the target found on stack (which
        //         is the target to load the resource into).
        //
        as_object* curtgt = getObject(env.target());
        if (!curtgt) {
            log_error(_("commonGetURL: current target is undefined"));
            return;
        }
        varsToSend = getURLEncodedVars(*curtgt);
    }


    if (loadTargetFlag) {
        log_debug(_("getURL2 target load"));

        if (!target_ch) {

            unsigned int levelno;
            if (isLevelTarget(getSWFVersion(env), target_string, levelno)) {
                log_debug(_("Testing _level loading (level %u)"), levelno);
                m.loadMovie(url, target_string, varsToSend, sendVarsMethod);
                return;
            }

            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("Unknown loadMovie target: %s"),
                    target_string);
            );

            // TESTED: Even if the target is created right-after 
            //         the load request, the player won't load
            //         into it. In other words, the target MUST
            //         exist at time of interpreting the GETURL2
            //         tag with loadTarget flag.

            return;
        }

        if (!target_movie) {
            log_error(_("get url: target %s is not a sprite"), target_string);
            return;
        }

        const std::string s = target_movie->getTarget(); // or getOrigTarget ?
        if (s != target_movie->getOrigTarget()) {
            log_debug("TESTME: target of a loadMovie changed its target path");
        }
        
        // To trigger: https://savannah.gnu.org/bugs/?32506
        if ( m.findCharacterByTarget(s) != target_movie ) {
            log_error("FIXME: "
                "getURL target %1% is resolved by findTarget(env) "
                "to sprite %2%. Sprite %2% has "
                "target %3%. Target %3% will be resolved "
                "by movie_root::findCharacterByTarget() to %4%",
                target_string, target_movie, s,
                m.findCharacterByTarget(s));
        }

        // We might probably use target_string here rather than
        // ``s'' (which is the _official_ target) but at time   
        // of writing the MovieLoader will use
        // movie_root::findCharacterByTarget to resolve paths,
        // and that one, in turn, won't support /slash/notation
        // which may be found in target_string (see also loadMovieTest.swf
        // under misc-ming.all)
        //
        m.loadMovie(url, s, varsToSend, sendVarsMethod); 
        return;
    }

    unsigned int levelno;
    if (isLevelTarget(getSWFVersion(env), target_string, levelno)) {
        log_debug(_("Testing _level loading (level %u)"), levelno);
        m.loadMovie(url, target_string, varsToSend, sendVarsMethod);
        return;
    }

    // Just plain getURL
    // This should be the original URL string, as the hosting application
    // will decide how to resolve the URL. If there is no hosting
    // application, movie_root::getURL will resolve the URL.
    m.getURL(url, target_string, varsToSend, sendVarsMethod);

}

// Common code for SetTarget and SetTargetExpression. See:
// http://sswf.sourceforge.net/SWFalexref.html#action_set_target
// http://sswf.sourceforge.net/SWFalexref.html#action_get_dynamic
void
commonSetTarget(ActionExec& thread, const std::string& target_name)
{
    as_environment& env = thread.env;

    // see swfdec's settarget-relative-*.swf
    env.reset_target();

    // if the string is blank, we reset the target to its original value
    if (target_name.empty()) return;

    // TODO: pass thread.getScopeStack()
    DisplayObject* new_target = findTarget(env, target_name); 
    if (!new_target) {
        IF_VERBOSE_ASCODING_ERRORS (
            log_aserror(_("Couldn't find movie \"%s\" to set target to!"
                " Setting target to NULL..."), target_name);
        );
    }
    
    env.set_target(new_target);
}

}


} // namespace gnash
