// ASHandlers.cpp:  ActionScript handlers, for Gnash.
//
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
//


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h" // USE_DEBUGGER
#endif

#include "smart_ptr.h" // GNASH_USE_GC
#include "log.h"
#include "SWF.h"
#include "rc.h"
#include "ASHandlers.h"
#include "movie_definition.h"
#include "Array_as.h"
#include "swf_function.h"
#include "as_function.h"
#include "fn_call.h"
#include "ActionExec.h"
#include "MovieClip.h"
#include "as_environment.h"
#include "URL.h"
#include "action_buffer.h"
#include "as_object.h"
#include "Object.h"
#include "String_as.h" // for automatic as_value::STRING => String as object
#include "Number_as.h" // for automatic as_value::NUMBER => Number as object
#include "drag_state.h"
#include "VM.h" // for getting the root
#include "movie_root.h" // for set_drag_state (ActionStartDragMovie)
#include "debugger.h"
#include "sound_handler.h"
#include "namedStrings.h"
#include "utf8.h"
#include "StringPredicates.h" 
#include "GnashNumeric.h"

#include <string>
#include <vector>
#include <locale>
#include <cstdlib> // std::mbstowcs
#include <boost/scoped_array.hpp>
#include <boost/random.hpp>
#include <boost/lexical_cast.hpp>
#include <algorithm> // std::swap

// GNASH_PARANOIA_LEVEL:
// 0 : no assertions
// 1 : quick assertions
// 2 : check that handlers are called on correct tag
//
#ifndef GNASH_PARANOIA_LEVEL
# define GNASH_PARANOIA_LEVEL 1
#endif

namespace {
#ifdef USE_DEBUGGER
static gnash::Debugger& debugger = gnash::Debugger::getDefaultInstance();
#endif
}

namespace gnash {

namespace SWF { // gnash::SWF

//
// Utility: construct an object using given constructor.
// This is used by both ActionNew and ActionNewMethod and
// hides differences between builtin and actionscript-defined
// constructors.
//
static boost::intrusive_ptr<as_object>
construct_object(as_function* ctor_as_func,
    as_environment& env, unsigned int nargs)
{
    assert(ctor_as_func);
    std::auto_ptr< std::vector<as_value> > args (new std::vector<as_value> );
    args->reserve(nargs);
    for (unsigned int i=0; i<nargs; ++i) args->push_back(env.pop());
    return ctor_as_func->constructInstance(env, args);
}


static void unsupported_action_handler(ActionExec& thread)
{
    log_error(_("Unsupported action handler invoked, code at pc is %#x"),
            static_cast<int>(thread.code[thread.getCurrentPC()]));
}

ActionHandler::ActionHandler()
    :
    _name("unsupported"),
    _callback(unsupported_action_handler),
    _debug(false),
    _arg_format(ARG_NONE)
{
}

ActionHandler::ActionHandler(ActionType type, ActionCallback func)
    :
    _type(type),
    _callback(func),
    _debug(false),
    _arg_format(ARG_NONE)
{
}

ActionHandler::ActionHandler(ActionType type, std::string name,
                             ActionCallback func)
    :
    _type(type),
    _name(name),
    _callback(func),
    _debug(false),
    _arg_format(ARG_NONE)
{
}

ActionHandler::ActionHandler(ActionType type, std::string name,
                             ActionCallback func, ArgumentType format)
    :
    _type(type),
    _name(name),
    _callback(func),
    _debug(false),
    _arg_format(format)
{
}

void
ActionHandler::execute(ActionExec& thread) const
{
    return _callback(thread);
}

SWFHandlers::SWFHandlers()
{

    // Just to be sure we can start using different handler
    // based on version (would make sense)
    if ( ! VM::isInitialized() )
    {
        log_error(_("FIXME: VM not initialized at SWFHandlers construction time, can't set action handlers based on SWF version"));
    }

    container_type & handlers = get_handlers();

    handlers[ACTION_END] = ActionHandler(ACTION_END,
             "<End>", SWFHandlers::ActionEnd);
    handlers[ACTION_NEXTFRAME] = ActionHandler(ACTION_NEXTFRAME,
             "NextFrame", SWFHandlers::ActionNextFrame);
    handlers[ACTION_PREVFRAME] =  ActionHandler(ACTION_PREVFRAME,
             "PreviousFrame", SWFHandlers::ActionPrevFrame);
    handlers[ACTION_PLAY] = ActionHandler(ACTION_PLAY,
             "Play", SWFHandlers::ActionPlay);
    handlers[ACTION_STOP] = ActionHandler(ACTION_STOP,
             "Stop", SWFHandlers::ActionStop);
    handlers[ACTION_TOGGLEQUALITY] = ActionHandler(ACTION_TOGGLEQUALITY,
             "ToggleQuality", SWFHandlers::ActionToggleQuality);
    handlers[ACTION_STOPSOUNDS] = ActionHandler(ACTION_STOPSOUNDS,
             "StopSounds", SWFHandlers::ActionStopSounds);
    handlers[ACTION_GOTOFRAME] = ActionHandler(ACTION_GOTOFRAME,
             "GotoFrame", SWFHandlers::ActionGotoFrame, ARG_U16);
    handlers[ACTION_GETURL] = ActionHandler(ACTION_GETURL,
             "GetUrl", SWFHandlers::ActionGetUrl, ARG_STR);
    handlers[ACTION_WAITFORFRAME] = ActionHandler(ACTION_WAITFORFRAME,
             "WaitForFrame", SWFHandlers::ActionWaitForFrame, ARG_HEX);
    handlers[ACTION_SETTARGET] = ActionHandler(ACTION_SETTARGET,
             "SetTarget", SWFHandlers::ActionSetTarget, ARG_STR);
    handlers[ACTION_GOTOLABEL] = ActionHandler(ACTION_GOTOLABEL,
             "GotoLabel", SWFHandlers::ActionGotoLabel, ARG_STR);
    handlers[ACTION_ADD] = ActionHandler(ACTION_ADD,
             "Add", SWFHandlers::ActionAdd);
    handlers[ACTION_SUBTRACT] = ActionHandler(ACTION_SUBTRACT,
             "Subtract", SWFHandlers::ActionSubtract);
    handlers[ACTION_MULTIPLY] = ActionHandler(ACTION_MULTIPLY,
             "Multiply", SWFHandlers::ActionMultiply);
    handlers[ACTION_DIVIDE] = ActionHandler(ACTION_DIVIDE,
             "Divide", SWFHandlers::ActionDivide);
    handlers[ACTION_EQUAL] = ActionHandler(ACTION_EQUAL,
             "Equal", SWFHandlers::ActionEqual);
    handlers[ACTION_LESSTHAN] = ActionHandler(ACTION_LESSTHAN,
             "LessThan", SWFHandlers::ActionLessThan);
    handlers[ACTION_LOGICALAND] = ActionHandler(ACTION_LOGICALAND,
             "LogicalAnd", SWFHandlers::ActionLogicalAnd);
    handlers[ACTION_LOGICALOR] = ActionHandler(ACTION_LOGICALOR,
             "LogicalOr", SWFHandlers::ActionLogicalOr);
    handlers[ACTION_LOGICALNOT] = ActionHandler(ACTION_LOGICALNOT,
             "LogicalNot", SWFHandlers::ActionLogicalNot);
    handlers[ACTION_STRINGEQ] = ActionHandler(ACTION_STRINGEQ,
             "StringEq", SWFHandlers::ActionStringEq);
    handlers[ACTION_STRINGLENGTH] = ActionHandler(ACTION_STRINGLENGTH,
             "ActionStringLength", SWFHandlers::ActionStringLength);
    handlers[ACTION_SUBSTRING] = ActionHandler(ACTION_SUBSTRING,
             "ActionSubString", SWFHandlers::ActionSubString);
    handlers[ACTION_POP] = ActionHandler(ACTION_POP,
             "ActionPop", SWFHandlers::ActionPop);
    handlers[ACTION_INT] = ActionHandler(ACTION_INT,
             "ActionInt", SWFHandlers::ActionInt);
    handlers[ACTION_GETVARIABLE] = ActionHandler(ACTION_GETVARIABLE,
             "ActionGetVariable", SWFHandlers::ActionGetVariable);
    handlers[ACTION_SETVARIABLE] = ActionHandler(ACTION_SETVARIABLE,
             "ActionSetVariable", SWFHandlers::ActionSetVariable);
    handlers[ACTION_SETTARGETEXPRESSION] = ActionHandler(ACTION_SETTARGETEXPRESSION,
             "ActionSetTargetExpression", SWFHandlers::ActionSetTargetExpression);
    handlers[ACTION_STRINGCONCAT] = ActionHandler(ACTION_STRINGCONCAT,
             "ActionStringConcat", SWFHandlers::ActionStringConcat);
    handlers[ACTION_GETPROPERTY] = ActionHandler(ACTION_GETPROPERTY,
             "ActionGetProperty", SWFHandlers::ActionGetProperty);
    handlers[ACTION_SETPROPERTY] = ActionHandler(ACTION_SETPROPERTY,
             "ActionSetProperty", SWFHandlers::ActionSetProperty);
    handlers[ACTION_DUPLICATECLIP] = ActionHandler(ACTION_DUPLICATECLIP,
             "ActionDuplicateClip", SWFHandlers::ActionDuplicateClip);
    handlers[ACTION_REMOVECLIP] = ActionHandler(ACTION_REMOVECLIP,
             "ActionRemoveClip", SWFHandlers::ActionRemoveClip);
    handlers[ACTION_TRACE] = ActionHandler(ACTION_TRACE,
             "ActionTrace", SWFHandlers::ActionTrace);
    handlers[ACTION_STARTDRAGMOVIE] = ActionHandler(ACTION_STARTDRAGMOVIE,
             "ActionStartDragMovie", SWFHandlers::ActionStartDragMovie);
    handlers[ACTION_STOPDRAGMOVIE] = ActionHandler(ACTION_STOPDRAGMOVIE,
             "ActionStopDragMovie", SWFHandlers::ActionStopDragMovie);
    handlers[ACTION_STRINGCOMPARE] = ActionHandler(ACTION_STRINGCOMPARE,
             "ActionStringCompare", SWFHandlers::ActionStringCompare);
    handlers[ACTION_THROW] = ActionHandler(ACTION_THROW,
             "ActionThrow", SWFHandlers::ActionThrow);
    handlers[ACTION_CASTOP] = ActionHandler(ACTION_CASTOP,
             "ActionCastOp", SWFHandlers::ActionCastOp);
    handlers[ACTION_IMPLEMENTSOP] = ActionHandler(ACTION_IMPLEMENTSOP,
             "ActionImplementsOp", SWFHandlers::ActionImplementsOp);
    handlers[ACTION_FSCOMMAND2] = ActionHandler(ACTION_FSCOMMAND2,
             "ActionFscommand2", SWFHandlers::ActionFscommand2);
    handlers[ACTION_RANDOM] = ActionHandler(ACTION_RANDOM,
             "ActionRandom", SWFHandlers::ActionRandom);
    handlers[ACTION_MBLENGTH] = ActionHandler(ACTION_MBLENGTH,
             "ActionMbLength", SWFHandlers::ActionMbLength);
    handlers[ACTION_ORD] = ActionHandler(ACTION_ORD,
             "ActionOrd", SWFHandlers::ActionOrd);
    handlers[ACTION_CHR] = ActionHandler(ACTION_CHR,
             "ActionChr", SWFHandlers::ActionChr);
    handlers[ACTION_GETTIMER] = ActionHandler(ACTION_GETTIMER,
             "ActionGetTimer", SWFHandlers::ActionGetTimer);
    handlers[ACTION_MBSUBSTRING] = ActionHandler(ACTION_MBSUBSTRING,
             "ActionMbSubString", SWFHandlers::ActionMbSubString);
    handlers[ACTION_MBORD] = ActionHandler(ACTION_MBORD,
             "ActionMbOrd", SWFHandlers::ActionMbOrd);
    handlers[ACTION_MBCHR] = ActionHandler(ACTION_MBCHR,
             "ActionMbChr", SWFHandlers::ActionMbChr);
    handlers[ACTION_WAITFORFRAMEEXPRESSION] = ActionHandler(ACTION_WAITFORFRAMEEXPRESSION,
             "ActionWaitForFrameExpression",
             SWFHandlers::ActionWaitForFrameExpression, ARG_HEX);
    handlers[ACTION_PUSHDATA] = ActionHandler(ACTION_PUSHDATA,
             "ActionPushData", SWFHandlers::ActionPushData, ARG_PUSH_DATA);
    handlers[ACTION_BRANCHALWAYS] = ActionHandler(ACTION_BRANCHALWAYS,
             "ActionBranchAlways", SWFHandlers::ActionBranchAlways, ARG_S16);
    handlers[ACTION_GETURL2] = ActionHandler(ACTION_GETURL2,
             "ActionGetUrl2", SWFHandlers::ActionGetUrl2, ARG_HEX);
    handlers[ACTION_BRANCHIFTRUE] = ActionHandler(ACTION_BRANCHIFTRUE,
             "ActionBranchIfTrue", SWFHandlers::ActionBranchIfTrue, ARG_S16);
    handlers[ACTION_CALLFRAME] = ActionHandler(ACTION_CALLFRAME,
             "ActionCallFrame", SWFHandlers::ActionCallFrame, ARG_HEX);
    handlers[ACTION_GOTOEXPRESSION] = ActionHandler(ACTION_GOTOEXPRESSION,
             "ActionGotoExpression", SWFHandlers::ActionGotoExpression, ARG_HEX);
    handlers[ACTION_DELETE] = ActionHandler(ACTION_DELETE,
             "ActionDelete", SWFHandlers::ActionDelete);
    handlers[ACTION_DELETE2] = ActionHandler(ACTION_DELETE2,
             "ActionDelete2", SWFHandlers::ActionDelete2);
    handlers[ACTION_VAREQUALS] = ActionHandler(ACTION_VAREQUALS,
             "ActionVarEquals", SWFHandlers::ActionVarEquals);
    handlers[ACTION_CALLFUNCTION] = ActionHandler(ACTION_CALLFUNCTION,
             "ActionCallFunction", SWFHandlers::ActionCallFunction);
    handlers[ACTION_RETURN] = ActionHandler(ACTION_RETURN,
             "ActionReturn", SWFHandlers::ActionReturn);
    handlers[ACTION_MODULO] = ActionHandler(ACTION_MODULO,
             "ActionModulo", SWFHandlers::ActionModulo);
    handlers[ACTION_NEW] = ActionHandler(ACTION_NEW,
             "ActionNew", SWFHandlers::ActionNew);
    handlers[ACTION_VAR] = ActionHandler(ACTION_VAR,
             "ActionVar", SWFHandlers::ActionVar);
    handlers[ACTION_INITARRAY] = ActionHandler(ACTION_INITARRAY,
             "ActionInitArray", SWFHandlers::ActionInitArray);
    handlers[ACTION_INITOBJECT] = ActionHandler(ACTION_INITOBJECT,
             "ActionInitObject", SWFHandlers::ActionInitObject);
    handlers[ACTION_TYPEOF] = ActionHandler(ACTION_TYPEOF,
             "ActionTypeOf", SWFHandlers::ActionTypeOf);
    handlers[ACTION_TARGETPATH] = ActionHandler(ACTION_TARGETPATH,
             "ActionTargetPath", SWFHandlers::ActionTargetPath);
    handlers[ACTION_ENUMERATE] = ActionHandler(ACTION_ENUMERATE,
             "ActionEnumerate", SWFHandlers::ActionEnumerate);
    handlers[ACTION_NEWADD] = ActionHandler(ACTION_NEWADD,
             "ActionNewAdd", SWFHandlers::ActionNewAdd);
    handlers[ACTION_NEWLESSTHAN] = ActionHandler(ACTION_NEWLESSTHAN,
             "ActionNewLessThan", SWFHandlers::ActionNewLessThan);
    handlers[ACTION_NEWEQUALS] = ActionHandler(ACTION_NEWEQUALS,
             "ActionNewEquals", SWFHandlers::ActionNewEquals);
    handlers[ACTION_TONUMBER] = ActionHandler(ACTION_TONUMBER,
             "ActionToNumber", SWFHandlers::ActionToNumber);
    handlers[ACTION_TOSTRING] = ActionHandler(ACTION_TOSTRING,
             "ActionToString", SWFHandlers::ActionToString);
    handlers[ACTION_DUP] = ActionHandler(ACTION_DUP,
             "ActionDup", SWFHandlers::ActionDup);
    handlers[ACTION_SWAP] = ActionHandler(ACTION_SWAP,
             "ActionSwap", SWFHandlers::ActionSwap);
    handlers[ACTION_GETMEMBER] = ActionHandler(ACTION_GETMEMBER,
             "ActionGetMember", SWFHandlers::ActionGetMember);
    handlers[ACTION_SETMEMBER] = ActionHandler(ACTION_SETMEMBER,
             "ActionSetMember", SWFHandlers::ActionSetMember);
    handlers[ACTION_INCREMENT] = ActionHandler(ACTION_INCREMENT,
             "ActionIncrement", SWFHandlers::ActionIncrement);
    handlers[ACTION_DECREMENT] = ActionHandler(ACTION_DECREMENT,
             "ActionDecrement", SWFHandlers::ActionDecrement);
    handlers[ACTION_CALLMETHOD] = ActionHandler(ACTION_CALLMETHOD,
             "ActionCallMethod", SWFHandlers::ActionCallMethod);
    handlers[ACTION_NEWMETHOD] = ActionHandler(ACTION_NEWMETHOD,
             "ActionNewMethod", SWFHandlers::ActionNewMethod);
    handlers[ACTION_INSTANCEOF] = ActionHandler(ACTION_INSTANCEOF,
             "ActionInstanceOf", SWFHandlers::ActionInstanceOf);
    handlers[ACTION_ENUM2] = ActionHandler(ACTION_ENUM2,
             "ActionEnum2", SWFHandlers::ActionEnum2);
    handlers[ACTION_BITWISEAND] = ActionHandler(ACTION_BITWISEAND,
             "ActionBitwiseAnd", SWFHandlers::ActionBitwiseAnd);
    handlers[ACTION_BITWISEOR] = ActionHandler(ACTION_BITWISEOR,
             "ActionBitwiseOr", SWFHandlers::ActionBitwiseOr);
    handlers[ACTION_BITWISEXOR] = ActionHandler(ACTION_BITWISEXOR,
             "ActionBitwiseXor", SWFHandlers::ActionBitwiseXor);
    handlers[ACTION_SHIFTLEFT] = ActionHandler(ACTION_SHIFTLEFT,
             "ActionShiftLeft", SWFHandlers::ActionShiftLeft);
    handlers[ACTION_SHIFTRIGHT] = ActionHandler(ACTION_SHIFTRIGHT,
             "ActionShiftRight", SWFHandlers::ActionShiftRight);
    handlers[ACTION_SHIFTRIGHT2] = ActionHandler(ACTION_SHIFTRIGHT2,
             "ActionShiftRight2", SWFHandlers::ActionShiftRight2);
    handlers[ACTION_STRICTEQ] = ActionHandler(ACTION_STRICTEQ,
             "ActionStrictEq", SWFHandlers::ActionStrictEq);
    handlers[ACTION_GREATER] = ActionHandler(ACTION_GREATER,
             "ActionGreater", SWFHandlers::ActionGreater);
    handlers[ACTION_STRINGGREATER] = ActionHandler(ACTION_STRINGGREATER,
             "ActionStringGreater", SWFHandlers::ActionStringGreater);
    handlers[ACTION_EXTENDS] = ActionHandler(ACTION_EXTENDS,
             "ActionExtends", SWFHandlers::ActionExtends);
    handlers[ACTION_CONSTANTPOOL] = ActionHandler(ACTION_CONSTANTPOOL,
             "ActionConstantPool", SWFHandlers::ActionConstantPool, ARG_DECL_DICT);
    handlers[ACTION_DEFINEFUNCTION2] = ActionHandler(ACTION_DEFINEFUNCTION2,
             "ActionDefineFunction2", SWFHandlers::ActionDefineFunction2,
             ARG_FUNCTION2);
    handlers[ACTION_TRY] = ActionHandler(ACTION_TRY,
             "ActionTry", SWFHandlers::ActionTry, ARG_FUNCTION2);
    handlers[ACTION_WITH] = ActionHandler(ACTION_WITH,
             "ActionWith", SWFHandlers::ActionWith, ARG_U16);
    handlers[ACTION_DEFINEFUNCTION] = ActionHandler(ACTION_DEFINEFUNCTION,
             "ActionDefineFunction", SWFHandlers::ActionDefineFunction, ARG_HEX);
    handlers[ACTION_SETREGISTER] = ActionHandler(ACTION_SETREGISTER,
             "ActionSetRegister", SWFHandlers::ActionSetRegister, ARG_U8);
}

SWFHandlers::~SWFHandlers()
{
}


std::vector<ActionHandler> &
SWFHandlers::get_handlers()
{
    static container_type handlers(255);
    return handlers;
}

/// @todo: make properties available outside, for
///        example for Machine.cpp
/// @todo: consider sorting named strings so that
///        the first 22 or more elements have
///        the corresponding property number (drops
///        one level of indirection).
///
static const string_table::key&
propertyKey(unsigned int val)
{
    static const string_table::key invalidKey=0;

    if ( val > 21u ) return invalidKey;

    static const string_table::key props[22] = {
        NSV::PROP_uX, // 0
        NSV::PROP_uY, // 1
        NSV::PROP_uXSCALE, // 2
        NSV::PROP_uYSCALE, // 3
        NSV::PROP_uCURRENTFRAME, // 4
        NSV::PROP_uTOTALFRAMES, // 5
        NSV::PROP_uALPHA, // 6
        NSV::PROP_uVISIBLE, // 7
        NSV::PROP_uWIDTH, // 8
        NSV::PROP_uHEIGHT, // 9
        NSV::PROP_uROTATION, // 10
        NSV::PROP_uTARGET, // 11
        NSV::PROP_uFRAMESLOADED, // 12
        NSV::PROP_uNAME, // 13
        NSV::PROP_uDROPTARGET, // 14
        NSV::PROP_uURL, // 15
        NSV::PROP_uHIGHQUALITY, // 16
        NSV::PROP_uFOCUSRECT, // 17
        NSV::PROP_uSOUNDBUFTIME, // 18
        NSV::PROP_uQUALITY, // 19
        NSV::PROP_uXMOUSE, // 20
        NSV::PROP_uYMOUSE // 21
    };

    return props[val];
}


const SWFHandlers&
SWFHandlers::instance()
{
    static SWFHandlers instance;
    return instance;
}

void
SWFHandlers::execute(ActionType type, ActionExec& thread) const
{
//    It is very heavy operation
//    if ( _handlers[type].getName() == "unsupported" ) return false;
    try {
        get_handlers()[type].execute(thread);
    }
    catch (ActionParserException& e) {
        log_swferror(_("Malformed action code: %s"), e.what());
    }
}

void
SWFHandlers::ActionEnd(ActionExec& thread)
{

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_END));
#endif

    log_error (_("%s: CHECKME: was broken"), __PRETTY_FUNCTION__);
    thread.skipRemainingBuffer();
}


void
SWFHandlers::ActionNextFrame(ActionExec& thread)
{

    as_environment& env = thread.env;

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_NEXTFRAME));
#endif

    DisplayObject* tgtch = env.get_target();
    MovieClip* tgt = tgtch ? tgtch->to_movie() : 0;
    if ( tgt ) tgt->goto_frame(tgt->get_current_frame() + 1);
    else log_debug(_("ActionNextFrame: as_environment target is null or not a sprite"));
}

void
SWFHandlers::ActionPrevFrame(ActionExec& thread)
{

    as_environment& env = thread.env;

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_PREVFRAME));
#endif

    DisplayObject* tgtch = env.get_target();
    MovieClip* tgt = tgtch ? tgtch->to_movie() : 0;
    if ( tgt ) tgt->goto_frame(tgt->get_current_frame() - 1);
    else log_debug(_("ActionPrevFrame: as_environment target is null or not a sprite"));
}

void
SWFHandlers::ActionPlay(ActionExec& thread)
{

    as_environment& env = thread.env;

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_PLAY));
#endif

    DisplayObject* tgtch = env.get_target();
    MovieClip* tgt = tgtch ? tgtch->to_movie() : 0;
    if ( tgt ) tgt->setPlayState(MovieClip::PLAYSTATE_PLAY);
    else log_debug(_("ActionPlay: as_environment target is null or not a sprite"));
}

void
SWFHandlers::ActionStop(ActionExec& thread)
{

    as_environment& env = thread.env;

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_STOP));
#endif

    DisplayObject* tgtch = env.get_target();
    MovieClip* tgt = tgtch ? tgtch->to_movie() : 0;
    if ( tgt ) tgt->setPlayState(MovieClip::PLAYSTATE_STOP);
    else log_debug(_("ActionStop: as_environment target is null or not a sprite"));
}

void
SWFHandlers::ActionToggleQuality(ActionExec&
#if GNASH_PARANOIA_LEVEL > 1
    thread
#endif
)
{

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_TOGGLEQUALITY));
#endif

    LOG_ONCE( log_unimpl (__PRETTY_FUNCTION__) );
}

void
SWFHandlers::ActionStopSounds(ActionExec& thread)
{

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_STOPSOUNDS));
#endif

    VM& vm = getVM(thread.env);
    sound::sound_handler* s = vm.getRoot().runResources().soundHandler();
    if (s)
    {
        s->stop_all_sounds();
    }
}

void
SWFHandlers::ActionGotoFrame(ActionExec& thread)
{

    as_environment& env = thread.env;
    const action_buffer& code = thread.code;

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_GOTOFRAME));
#endif

    size_t frame = code.read_int16(thread.getCurrentPC()+3);

    DisplayObject* tgtch = env.get_target();
    MovieClip* tgt = tgtch ? tgtch->to_movie() : 0;

    // frame number within this tag is hard-coded and 0-based
    if ( tgt ) tgt->goto_frame(frame);
    else log_debug(_("ActionGotoFrame: as_environment target is null or not a sprite"));
}

void
SWFHandlers::ActionGetUrl(ActionExec& thread)
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
    // end. This replaces an assertion in CommonGetUrl.
    const std::string target(code.read_string(pc + 3 + urlLength));

    IF_VERBOSE_ACTION (
        log_action(_("GetUrl: target=%s url=%s"), target, url);
    );

    CommonGetUrl(env, target, url, 0u);
}

void
SWFHandlers::ActionWaitForFrame(ActionExec& thread)
{

    as_environment& env = thread.env;
    const action_buffer& code = thread.code;

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_WAITFORFRAME));
#endif

    // SWF integrity check
    size_t tag_len = code.read_int16(thread.getCurrentPC()+1);
    if ( tag_len != 3 )
    {
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

    DisplayObject* target = env.get_target();
    MovieClip* target_sprite = target ? target->to_movie() : 0;
    if ( ! target_sprite )
    {
        log_error(_("%s: environment target is null or not a MovieClip"),
                __FUNCTION__);
        return;
    }

    unsigned int totframes = target_sprite->get_frame_count();
    if ( framenum > totframes )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("ActionWaitForFrame(%d): "
                       "target (%s) has only %d frames"),
                       framenum, totframes);
        );
        framenum = totframes;
    }

    // Actually *wait* for target frame, and never skip any action

    size_t lastloaded = target_sprite->get_loaded_frames();
    if ( lastloaded < framenum )
    {
        //log_debug(_("%s: frame %u not reached yet (loaded %u for sprite %s), skipping next %u actions"), __FUNCTION__, framenum, lastloaded, target_sprite->getTarget(), skip);
        // better delegate this to ActionExec
        thread.skip_actions(skip);
    }

}

void
SWFHandlers::ActionSetTarget(ActionExec& thread)
{

    const action_buffer& code = thread.code;
    size_t pc = thread.getCurrentPC();

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_SETTARGET)); // 0x8B
#endif

    // Change the movie we're working on.
    std::string target_name ( code.read_string(pc+3) );

    CommonSetTarget(thread, target_name);
}

void
SWFHandlers::ActionGotoLabel(ActionExec& thread)
{

    as_environment& env = thread.env;
    const action_buffer& code = thread.code;

    const char* frame_label = code.read_string(thread.getCurrentPC()+3);
    DisplayObject *target = env.get_target();
    MovieClip *target_sprite = target ? target->to_movie() : 0;
    if ( ! target_sprite )
    {
        log_error(_("%s: environment target is null or not a MovieClip"),
            __FUNCTION__);
    }
    else
    {
        target_sprite->goto_labeled_frame(frame_label);
    }
}

void
SWFHandlers::ActionAdd(ActionExec& thread)
{

    as_environment& env = thread.env;
    
    const double operand2 = env.top(0).to_number();
    const double operand1 = env.top(1).to_number();
    env.top(1) = operand1 + operand2;
    env.drop(1);
}

void
SWFHandlers::ActionSubtract(ActionExec& thread)
{

    as_environment& env = thread.env;
    
    const double operand2 = env.top(0).to_number();
    const double operand1 = env.top(1).to_number();
    env.top(1) = operand1 - operand2;
    env.drop(1);
}

void
SWFHandlers::ActionMultiply(ActionExec& thread)
{

    as_environment& env = thread.env;
    
    const double operand2 = env.top(0).to_number();
    const double operand1 = env.top(1).to_number();
    env.top(1) = operand1 * operand2;
    env.drop(1);
}


// Negative number / 0: -infinity
// Positive number / 0: infinity
// 0 / 0 : NaN
// Either operand is NaN: NaN
void
SWFHandlers::ActionDivide(ActionExec& thread)
{

    as_environment& env = thread.env;
    
    const double operand2 = env.top(0).to_number();
    const double operand1 = env.top(1).to_number();

    if (operand2 == 0)
    {
        if (env.get_version() < 5) {
            env.top(1).set_string("#ERROR#");
        }
        else if (operand1 == 0 || isNaN(operand1) || isNaN(operand2)) {
            env.top(1).set_nan();
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
    else
    {
        env.top(1) = operand1 / operand2;
    }
    env.drop(1);
}

void
SWFHandlers::ActionEqual(ActionExec& thread)
{

    as_environment& env = thread.env;

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_EQUAL)); // 0x0E
#endif

    as_value& op1 = env.top(0);
    as_value& op2 = env.top(1);

    env.top(1).set_bool(op1.to_number() == op2.to_number());

    // Flash4 used 1 and 0 as return from this tag
    if ( env.get_version() < 5 ) env.top(1).convert_to_number();

    env.drop(1);
}

void
SWFHandlers::ActionLessThan(ActionExec& thread)
{

    as_environment& env = thread.env;
    
    env.top(1).set_bool(env.top(1).to_number() < env.top(0).to_number());

    // Flash4 used 1 and 0 as return from this tag
    if ( env.get_version() < 5 ) env.top(1).convert_to_number();

    env.drop(1);
}

void
SWFHandlers::ActionLogicalAnd(ActionExec& thread)
{

    as_environment& env = thread.env;
    
    env.top(1).set_bool(env.top(1).to_bool() && env.top(0).to_bool());
    env.drop(1);
}

void
SWFHandlers::ActionLogicalOr(ActionExec& thread)
{

    as_environment& env = thread.env;
    
    env.top(1).set_bool(env.top(1).to_bool() || env.top(0).to_bool());
    env.drop(1);
}

void
SWFHandlers::ActionLogicalNot(ActionExec& thread)
{

    as_environment& env = thread.env;
    
    env.top(0).set_bool(! env.top(0).to_bool());

    // Flash4 used 1 and 0 as return from this tag
    if ( env.get_version() < 5 ) env.top(0).convert_to_number();
}

void
SWFHandlers::ActionStringEq(ActionExec& thread)
{

    as_environment& env = thread.env;
    
    const int version = env.get_version();
    const std::string& str0 = env.top(0).to_string_versioned(version);
    const std::string& str1 = env.top(1).to_string_versioned(version);

    env.top(1).set_bool(str0 == str1);
    env.drop(1);
}

void
SWFHandlers::ActionStringLength(ActionExec& thread)
{
    as_environment& env = thread.env;

    // NOTE: I've tested that we should change behaviour
    //       based on code definition version, not top-level
    //       SWF version. Just not automated yet.
    //
    const int version = thread.code.getDefinitionVersion();
    if ( version > 5 )
    {
        // when SWF version is > 5 we compute the multi-byte length
        ActionMbLength(thread);
    }
    else
    {
        env.top(0).set_int(env.top(0).to_string_versioned(version).size());
    }
}

void
SWFHandlers::ActionSubString(ActionExec& thread)
{

    // substring("string",  base,  size) 
    // SWF4 function, deprecated in favour of String.substring.
    // 1-based (String object methods are 0-based).

    as_environment& env = thread.env;
    
    const as_value& strval = env.top(2);

    // input checks
    if ( strval.is_undefined() || strval.is_null() )
    {
    IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Undefined or null string passed to ActionSubString, "
        "returning undefined"));
        );
        env.drop(2);
        env.top(0).set_undefined();
        return;
    }
    
    int size = env.top(0).to_int();
    int start = env.top(1).to_int();

    // We don't need to_string_versioned because undefined values have
    // already been dealt with.
    const int version = env.get_version();
    const std::wstring wstr = utf8::decodeCanonicalString(
                                strval.to_string(), version);

    if ( size < 0 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Negative size passed to ActionSubString, "
            "taking as whole length"));
        );
        size = wstr.length();
    }

    if ( size == 0 || wstr.empty() )
    {
        env.drop(2);
        env.top(0).set_string("");
        return;
    }


    // TODO: if 'start' or 'size' do not evaluate to numbers return
    //       the empty string (how do we check if they evaluate ??)
    if ( start < 1 )
    {
        IF_VERBOSE_ASCODING_ERRORS (
            log_aserror(_("Start is less then 1 in ActionSubString, "
            "setting to 1."));
        );
        start = 1;
    }

    // If start is longer than the string length, return empty
    // string
    else if (static_cast<unsigned int>(start) > wstr.length() )
    {
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

    if (static_cast<unsigned int>(start + size) > wstr.length())
    {
        IF_VERBOSE_ASCODING_ERRORS (
            log_aserror(_("start + size goes beyond input string in ActionSubString, "
            "adjusting size"));
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
SWFHandlers::ActionPop(ActionExec& thread)
{

    as_environment& env = thread.env;
    // this is an overhead only if SWF is malformed.
    
    env.drop(1);
}

void
SWFHandlers::ActionInt(ActionExec& thread)
{
    as_environment& env = thread.env;
    
    env.top(0).set_int((env.top(0).to_int()));
}

void
SWFHandlers::ActionGetVariable(ActionExec& thread)
{

    as_environment& env = thread.env;

    as_value& top_value = env.top(0);
    std::string var_string = top_value.to_string();
    if ( var_string.empty() )
    {
        top_value.set_undefined();
        return;
    }

    top_value = thread.getVariable(var_string);
    if ( env.get_version() < 5 && top_value.is_sprite() )
    {
        // See http://www.ferryhalim.com/orisinal/g2/penguin.htm
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Can't assign a sprite/DisplayObject to a variable in SWF%d. "
                    "We'll return undefined instead of %s."),
                    env.get_version(), top_value);
        );
        top_value.set_undefined();
    }

    IF_VERBOSE_ACTION
    (
        log_action(_("-- get var: %s=%s"),
                var_string,
                top_value);
    );
#ifdef USE_DEBUGGER
    debugger.matchWatchPoint(var_string, Debugger::READS);
#endif
}

void
SWFHandlers::ActionSetVariable(ActionExec& thread)
{

    as_environment& env = thread.env;

    const std::string& name = env.top(1).to_string();
    if ( name.empty() )
    {
        IF_VERBOSE_ASCODING_ERRORS (
            // Invalid object, can't set.
            log_aserror(_("ActionSetVariable: %s=%s: variable name "
                    "evaluates to invalid (empty) string"),
                env.top(1),
                env.top(0));
        );
    }
    thread.setVariable(name, env.top(0));

    IF_VERBOSE_ACTION (
        log_action(_("-- set var: %s = %s"), name, env.top(0));
    );

    // TODO: move this to ActionExec::setVariable !
#ifdef USE_DEBUGGER
    debugger.matchWatchPoint(name, Debugger::WRITES);
#endif

    env.drop(2);
}

// See: http://sswf.sourceforge.net/SWFalexref.html#action_get_dynamic
void
SWFHandlers::ActionSetTargetExpression(ActionExec& thread)
{

    as_environment& env = thread.env;

    // we don't ues the target sprite directly, instead we fetch the
    // _target(string type) of that sprite first and then search the
    // final target(might be a different one). See tests in
    // opcode_guard_test2.sc
    //
    // For _versioned, see swfdec's settarget2-tostring.as (swf 7 and 8)
    // 
    std::string target_name = env.top(0).to_string_versioned(env.get_version());

    CommonSetTarget(thread, target_name);

    env.drop(1); // pop the target sprite off the stack
}

void
SWFHandlers::ActionStringConcat(ActionExec& thread)
{

    as_environment& env = thread.env;

    const int version = env.get_version();
    env.top(1).convert_to_string_versioned(version);
    env.top(1).string_concat(env.top(0).to_string_versioned(version));
    env.drop(1);
}

void
SWFHandlers::ActionGetProperty(ActionExec& thread)
{
    
    as_environment& env = thread.env;

    as_value& tgt_val = env.top(1);
    std::string tgt_str = tgt_val.to_string();
    DisplayObject *target = NULL;
    if ( tgt_str.empty() )
    {
        as_object* obj = thread.getTarget();

        target = dynamic_cast<DisplayObject*>(obj);
        if ( ! target )
        {
            log_error(_("ActionGetProperty(<empty>) called, but current "
                        "target is not a DisplayObject"));
        }
    }
    else
    {
        target = env.find_target(tgt_str);
    }
 
    // FIXME: what happens when it's an invalid number? This will cause
    // undefined behaviour on overflow.
    unsigned int prop_number =
        static_cast<unsigned int>(env.top(0).to_number());

    if (target)
    {
        string_table::key propKey = propertyKey(prop_number);
        if ( propKey == 0 )
        {
            log_error(_("invalid property query, property "
                "number %d"), prop_number);
            env.top(1) = as_value();
        }
        else
        {
            as_value val;
            target->get_member(propKey, &val);
            env.top(1) = val;
        }
    }
    else
    {
        // ASCODING error ? (well, last time it was a gnash error ;)
        IF_VERBOSE_ASCODING_ERRORS (
        log_aserror(_("Could not find GetProperty target (%s)"),
                tgt_val);
        );
        env.top(1) = as_value();
    }
    env.drop(1);
}

void
SWFHandlers::ActionSetProperty(ActionExec& thread)
{
    
    as_environment& env = thread.env;

    DisplayObject *target = env.find_target(env.top(2).to_string());
    // FIXME: what happens when it's an invalid number? This will cause
    // undefined behaviour on overflow.
    unsigned int prop_number = (unsigned int)env.top(1).to_number();

    as_value prop_val = env.top(0);

    if (target)
    {
        string_table::key propKey = propertyKey(prop_number);
        if ( propKey == 0 )
        {
            // Malformed SWF ? (don't think this is possible to do with syntactically valid ActionScript)
            IF_VERBOSE_MALFORMED_SWF (
            log_swferror(_("invalid set_property, property number %d"), prop_number);
            )
        }
        else
        {
            target->set_member(propKey, prop_val);
        }
    }
    else
    {
        IF_VERBOSE_ASCODING_ERRORS (
        log_aserror(_("ActionSetProperty: can't find target %s for setting property %s"),
            env.top(2), prop_number);
        )
    }
    env.drop(3);
}

void
SWFHandlers::ActionDuplicateClip(ActionExec& thread)
{
    //GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;

    // Movies should be attachable from -16384 to 2130690044. See
    // Tests in misc-ming.all/DepthLimitsTest.c.
    const double depth = env.top(0).to_number() + DisplayObject::staticDepthOffset;
  
    // This also checks for overflow, as both numbers are expressible as
    // boost::int32_t.
    if (depth < DisplayObject::lowerAccessibleBound ||
      depth > DisplayObject::upperAccessibleBound)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("duplicateMovieClip: invalid depth %d passed; not duplicating"), depth);
        );  
        env.drop(3);
        return;
    }
  
    boost::int32_t depthValue = static_cast<boost::int32_t>(depth);
    
    const std::string& newname = env.top(1).to_string();
    const std::string& path = env.top(2).to_string();

    DisplayObject* ch = env.find_target(path);
    if ( ! ch )
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Path given to duplicateMovieClip(%s) doesn't point to a DisplayObject"),
                path);
        );
        env.drop(3);
        return;
    }

    boost::intrusive_ptr<MovieClip> sprite = ch->to_movie();
    if ( ! sprite )
    {
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
SWFHandlers::ActionRemoveClip(ActionExec& thread)
{

    as_environment& env = thread.env;

    const std::string path = env.pop().to_string();

    DisplayObject* ch = env.find_target(path);
    if ( ! ch )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Path given to removeMovieClip(%s) doesn't point to a DisplayObject"),
            path);
        );
        return;
    }

    MovieClip* sprite = ch->to_movie();
    if ( ! sprite )
    {
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
SWFHandlers::ActionTrace(ActionExec& thread)
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
SWFHandlers::ActionStartDragMovie(ActionExec& thread)
{

    as_environment& env = thread.env;

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_STARTDRAGMOVIE));
#endif

    

    drag_state st;

    DisplayObject* tgt = env.find_target(env.top(0).to_string());
    if ( tgt )
    {
        // mark this DisplayObject as script transformed.
        tgt->transformedByScript();
        st.setCharacter( tgt );
    }
    else
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("startDrag: unknown target '%s'"),
            env.top(0));
        );
    }

    st.setLockCentered( env.top(1).to_bool() );
    if ( env.top(2).to_bool() ) // has bounds !
    {
        // strk: this works if we didn't drop any before, in
        // a contrary case (if we used pop(), which I suggest)
        // we must remember to updated this as required
        

        boost::int32_t y1 = pixelsToTwips(env.top(3).to_number());
        boost::int32_t x1 = pixelsToTwips(env.top(4).to_number());
        boost::int32_t y0 = pixelsToTwips(env.top(5).to_number());
        boost::int32_t x0 = pixelsToTwips(env.top(6).to_number());

        // check for swapped values
        if ( y1 < y0 )
        {
            IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("Y values in ActionStartDrag swapped, fixing"));
            );
            std::swap(y1, y0);
        }

        if ( x1 < x0 )
        {
            IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("X values in ActionStartDrag swapped, fixing"));
            );
            std::swap(x1, x0);
        }

        rect bounds(x0, y0, x1, y1);
        st.setBounds(bounds);

        env.drop(4);
    }

    env.drop(3);

    if (tgt)
    {
        VM& vm = getVM(env);
        vm.getRoot().set_drag_state(st);
    }

}

void
SWFHandlers::ActionStopDragMovie(ActionExec& thread)
{
    
    as_environment& env = thread.env;
    DisplayObject* tgtch = env.get_target();
    MovieClip *root_movie = tgtch ? tgtch->get_root() : 0;
    if ( root_movie ) root_movie->stop_drag();
    else log_debug(_("ActionStopDragMovie: as_environment target is null or not a sprite"));
}

void
SWFHandlers::ActionStringCompare(ActionExec& thread)
{
    
    as_environment& env = thread.env;
    
    const int ver = env.get_version();
    env.top(1).set_bool(env.top(1).to_string_versioned(ver) < env.top(0).to_string_versioned(ver));
    env.drop(1);
}

void
SWFHandlers::ActionThrow(ActionExec& thread)
{
    
    as_environment& env = thread.env;

    // Throw the value on the top of the stack.
    env.top(0).flag_exception();

    // Proceed to the end of the code block to throw.
    thread.skipRemainingBuffer();
}

void
SWFHandlers::ActionCastOp(ActionExec& thread)
{
    

    as_environment& env = thread.env;

    

    // Get the "instance"
    boost::intrusive_ptr<as_object> instance = env.top(0).to_object();

    // Get the "super" function
    as_function* super = env.top(1).to_as_function();

    // Invalid args!
    if (!super || ! instance)
    {
        IF_VERBOSE_ASCODING_ERRORS (
        log_aserror(_("-- %s cast_to %s (invalid args?)"),
            env.top(1),
            env.top(0));
        );

        env.drop(1);
        env.top(0).set_null(); // null, not undefined 
        return;
    }

    env.drop(1);
    if (instance->instanceOf(super))
    {
        env.top(0) = as_value(instance);
    }
    else
    {
        env.top(0).set_null(); // null, not undefined.
    }

    static bool warned=false;
    if ( ! warned ) {
        log_debug(_("ActionCastOp TESTING"));
        warned=true;
    }
}

void
SWFHandlers::ActionImplementsOp(ActionExec& thread)
{
    
//    TODO: This doesn't work quite right, yet.
    as_environment& env = thread.env;

    

    as_value objval = env.pop();
    as_object *obj = objval.to_object().get();
    int count = static_cast<int>(env.pop().to_number());
    as_value a(1);

    if (!obj)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Stack value on IMPLEMENTSOP is not an object: %s."),
            objval);
        );
        return;
    }

    as_value protoval;
    if ( ! obj->get_member(NSV::PROP_PROTOTYPE, &protoval) )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Target object for IMPLEMENTSOP has no prototype."));
        );
        return;
    }
    obj = protoval.to_object().get();
    if (!obj)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("IMPLEMENTSOP target object's prototype is not an object (%s)"),
            protoval);
        );
        return;
    }

    if ( count <= 0 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Invalid interfaces count (%d) on IMPLEMENTSOP"), count);
        );
        return;
    }

    while (count--)
    {
        as_value ctorval = env.pop();

        as_object* ctor = ctorval.to_object().get();
        if ( ! ctor )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("class found on stack on IMPLEMENTSOP is not an object: %s"), ctorval);
            );
            continue;
        }
        if ( ! ctor->get_member(NSV::PROP_PROTOTYPE, &protoval) )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Interface object for IMPLEMENTSOP has no prototype."));
            );
            continue;
        }
        as_object *inter = protoval.to_object().get();
        if ( ! inter )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Prototype of interface object for IMPLEMENTSOP is not an object (%s)."),
                protoval);
            );
            continue;
        }

        IF_VERBOSE_ACTION(
        log_action("%s (with .prototype %p) implements %s (with .prototype %p)",
            objval, (void*)obj, ctorval,
            (void*)inter);
        );
        obj->add_interface(inter);
    }
}

/// FsCommand2 is not part of the Flash spec. It belongs to
/// Flash Lite (from 1.1 onwards) and is used to control
/// devices (backlight, vibrate etc).
void
SWFHandlers::ActionFscommand2(ActionExec& thread)
{
    

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_FSCOMMAND2)); // 0x0E
#endif

    as_environment& env = thread.env;

    unsigned int off=0;
    
    const unsigned int nargs = env.top(off++).to_int();

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
SWFHandlers::ActionRandom(ActionExec& thread)
{

    // Action random(n) should return an integer from 0 up to (not
    // including) n.
    // It was introduced in SWF4 and deprecated in favour of
    // Math.random() in SWF5.
    
    as_environment& env = thread.env;

    int max = env.top(0).to_int();

    if (max < 1) max = 1;

    // Get pointer to static random generator in VM
    VM::RNG& rnd = getVM(env).randomNumberGenerator();

    // Produces int (0 <= n <= max - 1)
    boost::uniform_int<> uni_dist(0, max - 1);
    boost::variate_generator<VM::RNG&,
        boost::uniform_int<> > uni(rnd, uni_dist);

    env.top(0).set_int(uni());
}

as_encoding_guess_t
SWFHandlers::guessEncoding(const std::string &str, int &length, std::vector<int>& offsets)
{
    int width = 0; // The remaining width, not the total.
    bool is_sought = true;

    std::string::const_iterator it = str.begin();
    const std::string::const_iterator e = str.end();

    length = 0;
    
    // First, assume it's UTF8 and try to be wrong.
    while (it != e && is_sought)
    {
        ++length;

        offsets.push_back(it - str.begin()); // current position

        // Advances the iterator to point to the next 
        boost::uint32_t c = utf8::decodeNextUnicodeCharacter(it, e);

        if (c == utf8::invalid)
        {
            is_sought = false;
            break;
        }
    }

    offsets.push_back(it - str.begin()); // current position

    if (it == e && is_sought)
    {
        // No characters left, so it's almost certainly UTF8.
        return ENCGUESS_UNICODE;
    }

    it = str.begin();
    int index = 0;
    is_sought = true;
    width = 0;
    length = 0;
    bool was_odd = true;
    bool was_even = true;
    // Now, assume it's SHIFT_JIS and try to be wrong.
    while (it != e && is_sought)
    {
        int c = static_cast<int> (*it);

        if (width)
        {
            --width;
            if ((c < 0x40) || ((c < 0x9F) && was_even) ||
                ((c > 0x9E) && was_odd) || (c == 0x7F))
            {
                is_sought = false;
            }
            continue;
        }

        ++length;
        offsets.push_back(index); // [length - 1] = index;

        if ((c == 0x80) || (c == 0xA0) || (c >= 0xF0))
        {
            is_sought = false;
            break;
        }

        if (((c >= 0x81) && (c <= 0x9F)) || ((c >= 0xE0) && (c <= 0xEF)))
        {
            width = 1;
            was_odd = c & 0x01;
            was_even = !was_odd;
        }
    
        it++;
        index++;    
    }
    offsets.push_back(index); // [length - 1] = index;
    
    if (!width && is_sought)
    {
        // No width left, so it's probably SHIFT_JIS.
        return ENCGUESS_JIS;
    }

    // It's something else.
    length = std::mbstowcs(NULL, str.c_str(), 0);
    if (length == -1)
    {
        length = str.length();
    }
    return ENCGUESS_OTHER;
}

void
SWFHandlers::ActionMbLength(ActionExec& thread)
{
    
    as_environment& env = thread.env;

    
    std::string str = env.top(0).to_string();

    if (str.empty())
    {
        env.top(0).set_int(0);
    }
    else
    {
        int length;
        std::vector<int> unused;
        unused.resize(str.length()+1);
        (void) guessEncoding(str, length, unused);
        env.top(0).set_int(length);
    }
}

void
SWFHandlers::ActionOrd(ActionExec& thread)
{
    
    as_environment& env = thread.env;
    
    
    // Should return 0 

    const int swfVersion = thread.code.getDefinitionVersion();
    
    std::string str = env.top(0).to_string();
    
    if (str.empty())
    {
        env.top(0).set_int(0);
        return;
    }

    std::wstring wstr = utf8::decodeCanonicalString(str, swfVersion);

    // decodeCanonicalString should correctly work out what the first
    // character is according to version.
    env.top(0).set_int(wstr.at(0));
}

void
SWFHandlers::ActionChr(ActionExec& thread)
{

    as_environment& env = thread.env;
    
    // Only handles values up to 65535
    boost::uint16_t c = static_cast<boost::uint16_t>(env.top(0).to_int());

    // If the argument to chr() is '0', we return
    // nothing, not NULL
    if (c == 0)
    {
        env.top(0).set_string("");
        return;
    }
    
    int swfVersion = thread.code.getDefinitionVersion();
    if (swfVersion > 5)
    {
        env.top(0).set_string(utf8::encodeUnicodeCharacter(c));
        return;
    }

    // SWF 5 only:
    // This casts to unsigned char to a string, giving
    // IS0-8859-1 8-bit characters.
    // Values above 256 evaluate to value % 256, 
    // through the cast, which is expected behaviour.
    const unsigned char uc = static_cast<unsigned char>(c);
    if (uc == 0)
    {
        env.top(0).set_string("");
        return;
    }
    std::string s;
    s.push_back(uc);
    env.top(0).set_string(s);
}

void
SWFHandlers::ActionGetTimer(ActionExec& thread)
{
    
    
    as_environment& env = thread.env;

    const VM& vm = getVM(env);
    env.push(vm.getTime());
}

void
SWFHandlers::ActionMbSubString(ActionExec& thread)
{
    
    as_environment& env = thread.env;

    

    int size = env.top(0).to_int();
    int start = env.top(1).to_int();
    as_value& string_val = env.top(2);

    IF_VERBOSE_ACTION(
    log_action(" ActionMbSubString(%s, %d, %d)", string_val, start, size);
    );

    env.drop(2);

    if (string_val.is_undefined() || string_val.is_null())
    {
        log_error(_("Undefined or null string passed to ActionMBSubString, "
            "returning undefined"));
        env.top(0).set_undefined();
        return;
    }

    std::string str = string_val.to_string();
    int length = 0;
    std::vector<int> offsets;

    as_encoding_guess_t encoding = guessEncoding(str, length, offsets);

    //log_debug("Guessed encoding for %s: %d - len:%d, offsets.size:%d", str, encoding, length, offsets.size());
    //for (int i=0; i<offsets.size(); ++i) log_debug("  offsets[%d]: %d", i, offsets[i]);

    if (size < 0)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Negative size passed to ActionSubString, "
            "taking as whole length"));
        );
        size = length;
    }

    if (start < 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Base is less then 1 in ActionMbSubString, "
            "setting to 1."));
        );
        start = 1;
    }

    else if ( start > length)
    {
    IF_VERBOSE_ASCODING_ERRORS (
        log_aserror(_("base goes beyond input string in ActionMbSubString, "
        "returning the empty string."));
    );
        env.top(0).set_string("");
    return;
    }

    // Adjust the start for our own use.
    --start;

    if (size + start > length)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("base+size goes beyond input string in ActionMbSubString, "
            "adjusting size based on length:%d and start:%d"), length, start);
        );
        size = length - start;
    }

    //log_debug("Adjusted start:%d size:%d", start, size);

    if (encoding == ENCGUESS_OTHER)
    {
        env.top(0).set_string(str.substr(start, size));
    }
    else
    {
        env.top(0).set_string(str.substr(offsets.at(start),
                            offsets.at(start + size) - offsets.at(start)));
    }
    return;
}

void
SWFHandlers::ActionMbOrd(ActionExec& thread)
{
    /// This only deals with UTF-8 characters.
    /// TODO: what else is possible?
    /// TODO: fix for SWF5

    as_environment& env = thread.env;

    if (env.get_version() == 5)
    {
        log_unimpl("Not properly implemented for SWF5");
        // No need to return - it works a bit.
    }

    const std::string s = env.top(0).to_string();
    
    std::string::const_iterator it = s.begin(), e = s.end();
    
    boost::uint32_t out = utf8::decodeNextUnicodeCharacter(it, e);
    
    /// Always valid, or can it be undefined?
    env.top(0).set_int(out);
}

void
SWFHandlers::ActionMbChr(ActionExec& thread)
{
    /// This only generates UTF-8 characters. No idea
    /// what difference user locale might make, but UTF-8
    /// is generally GOOD.
    
    /// TODO: fix for SWF5
    as_environment& env = thread.env;
    
    if (env.get_version() == 5)
    {
        log_unimpl(_("Not properly implemented for SWF5"));
        // No need to return.
    }

    // Cut to uint16, as characters above 65535 'wrap around'
    const boost::uint16_t i = static_cast<boost::uint16_t> (env.top(0).to_int());
    
    std::string out = utf8::encodeUnicodeCharacter(i);
    
    /// Always valid, or can it be undefined?
    env.top(0).set_string(out);

}

// also known as WaitForFrame2
void
SWFHandlers::ActionWaitForFrameExpression(ActionExec& thread)
{
    
    as_environment& env = thread.env;
    const action_buffer& code = thread.code;

    

    // how many actions to skip if frame has not been loaded
    boost::uint8_t skip = code[thread.getCurrentPC()+3];

    // env.top(0) contains frame specification,
    // evaluated as for ActionGotoExpression
    as_value framespec = env.pop();

    DisplayObject* tgtch = env.get_target();
    MovieClip* target_sprite = tgtch ? tgtch->to_movie() : 0;
    if ( ! target_sprite )
    {
        log_error(_("%s: environment target is null or not a MovieClip"),
            __FUNCTION__);
        return;
    }

    size_t framenum;
        if ( ! target_sprite->get_frame_number(framespec, framenum) )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Frame spec found on stack "
            "at ActionWaitForFrame doesn't evaluate "
                "to a valid frame: %s"),
            framespec);
        );
        return;
    }

    size_t lastloaded = target_sprite->get_loaded_frames();
    if ( lastloaded < framenum )
    {
        //log_debug(_("ActionWaitForFrameExpression: frame %u not reached yet (loaded %u), skipping next %u actions"), framenum, lastloaded, skip);
        // better delegate this to ActionExec
        thread.skip_actions(skip);
    }

}

void
SWFHandlers::ActionPushData(ActionExec& thread)
{
    
    as_environment& env = thread.env;

    enum {
        pushString,    // 0
        pushFloat,    // 1
        pushNull,    // 2
        pushUndefined,    // 3
        pushRegister,    // 4
        pushBool,    // 5
        pushDouble,    // 6
        pushInt32,    // 7
        pushDict8,    // 8
        pushDict16,    // 9
        pushLast    // 10 - sentinel
    };
    const char* pushType[] = {
        "string",    // 0
        "float",    // 1
        "null",        // 2
        "undefined",    // 3
        "register",    // 4
        "bool",        // 5
        "double",    // 6
        "boost::int32_t",    // 7
        "dict8",    // 8
        "dict16"    // 9
    };


    const action_buffer& code = thread.code;

    size_t pc = thread.getCurrentPC();
    boost::uint16_t length = code.read_uint16(pc+1);

    //---------------
    size_t i = pc;
    size_t count = 0;
    while (i - pc < length) {
        int id=0; // for dict (constant pool) lookup
                  // declared here because also used
              // by verbose action output
        boost::uint8_t type = code[3 + i];
        i++;

        switch (type)
        {
            default:
            {
                IF_VERBOSE_MALFORMED_SWF(
                    log_swferror(_("Unknown push type %d."
                        " Execution will continue "
                        "but it is likely to fail "
                        "due to lost sync."), type);
                );
                continue;
            }

            case  pushString: // 0
            {
                const char* cstr = code.read_string(i+3);
                const std::string str(cstr);
                i += str.size() + 1; 
                env.push(str);
                break;
            }

            case pushFloat: // 1
            {
                float f = code.read_float_little(i+3);
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
            {
                env.push(as_value());
                break;
            }

            case pushRegister: // 4
            {
                unsigned int reg = code[3 + i];
                ++i;
		as_value v;
		if ( ! env.getRegister(reg, v) )
		{
                        IF_VERBOSE_MALFORMED_SWF(
                        log_swferror(_("Invalid register %d in ActionPush"), reg);
                        );
		}
		env.push(v);
                break;
            }

            case pushBool: // 5
            {
                bool    bool_val = code[i+3] ? true : false;
                i++;
                env.push(bool_val);
                break;
            }

            case pushDouble: // 6
            {
                double d = code.read_double_wacky(i+3);
                i += 8;
                env.push(d);
                break;
            }

            case pushInt32: // 7
            {
                boost::int32_t val = code.read_int32(i+3);
                i += 4;
                env.push(val);
                break;
            }

            case pushDict8: // 8
            {
                id = code[3 + i];
                i++;
                if (static_cast<size_t>(id) < code.dictionary_size())
                {
                    env.push( code.dictionary_get(id) );
                }
                else
                {
                    IF_VERBOSE_MALFORMED_SWF(
                    log_swferror(_("dict_lookup %d "
                    "is out of bounds"), id);
                    );
                    env.push(as_value());
                }
                break;
            }

            case pushDict16: // 9
            {
                id = code.read_int16(i+3);
                i += 2;
                if ( static_cast<size_t>(id) < code.dictionary_size())
                {
                    env.push( code.dictionary_get(id) );
                }
                else
                {
                    IF_VERBOSE_MALFORMED_SWF(
                    log_swferror(_("dict_lookup %d "
                    "is out of bounds"), id);
                    );
                    env.push(as_value());
                }
                break;
            }
        }

        IF_VERBOSE_ACTION (
        if ( type == pushDict8 || type == pushDict16 )
        {
            log_action(_("\t%d) type=%s (%d), value=%s"),
                count, pushType[type], id, env.top(0));
        }
        else
        {
            log_action(_("\t%d) type=%s, value=%s"),
                count, pushType[type], env.top(0));
        }
        ++count;
        );
    }
}

void
SWFHandlers::ActionBranchAlways(ActionExec& thread)
{
    

    boost::int16_t offset = thread.code.read_int16(thread.getCurrentPC()+3);
    thread.adjustNextPC(offset);
    // @@ TODO range checks
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
void
SWFHandlers::CommonGetUrl(as_environment& env,
        as_value target, // the target window, or _level1..10
        const std::string& url, boost::uint8_t method)
{

    if (url.empty())
    {
        log_error(_("Bogus empty GetUrl url in SWF file, skipping"));
        return;
    }

    // Parse the method bitfield
    bool loadTargetFlag    = method & 64;
    bool loadVariableFlag  = method & 128;

    MovieClip::VariablesMethod sendVarsMethod;

    // handle malformed sendVarsMethod
    if ((method & 3) == 3)
    {
        log_error(_("Bogus GetUrl2 send vars method "
            " in SWF file (both GET and POST requested). Using GET"));
        sendVarsMethod = MovieClip::METHOD_GET;
    }
    else sendVarsMethod =
        static_cast<MovieClip::VariablesMethod>(method & 3);

    std::string target_string;
    if ( ! target.is_undefined() && ! target.is_null() )
    {
        target_string = target.to_string();
    }

    VM& vm = getVM(env);
    movie_root& m = vm.getRoot();
 
    // If the url starts with "FSCommand:", then this is
    // a message for the host app.
    StringNoCaseEqual noCaseCompare;
    if (noCaseCompare(url.substr(0, 10), "FSCommand:"))
    {
        m.handleFsCommand(url.substr(10), target_string);
        return;
    }

    // If the url starts with "print:", then this is
    // a print request.
    if (noCaseCompare(url.substr(0, 6), "print:"))
    {
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

    DisplayObject* target_ch = env.find_target(target.to_string());
    MovieClip* target_movie = target_ch ? target_ch->to_movie() : 0;

    if (loadVariableFlag)
    {
        log_debug(_("getURL2 loadVariable"));

        if (!target_ch)
        {
            log_error(_("getURL: target %s not found"), target_string);
            // might want to invoke the external url opener here...
            return;
        }

        if (!target_movie)
        {
            log_error(_("getURL: target %s is not a sprite"), target_string);
            // might want to invoke the external url opener here...
            return;
        }

        target_movie->loadVariables(url, sendVarsMethod);

        return;
    }

    std::string varsToSend;
    if (sendVarsMethod != MovieClip::METHOD_NONE)
    {

        // TESTED: variables sent are those in current target,
        //         no matter the target found on stack (which
        //         is the target to load the resource into).
        //
        DisplayObject* curtgt = env.get_target();
        if ( ! curtgt )
        {
            log_error(_("CommonGetUrl: current target is undefined"));
            return;
        }
        curtgt->getURLEncodedVars(varsToSend);
    }


    if ( loadTargetFlag )
    {
        log_debug(_("getURL2 target load"));

        if (!target_ch)
        {
            unsigned int levelno;
            if (m.isLevelTarget(target_string, levelno))
            {
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

        if (!target_movie)
        {
            log_error(_("get url: target %s is not a sprite"), target_string);
            return;
        }

        std::string s = target_movie->getTarget(); // or getOrigTarget ?
        if (s != target_movie->getOrigTarget())
        {
            log_debug(_("TESTME: target of a loadMovie changed its target "
                        "path"));
        }
        
        // TODO: try to trigger this !
        assert(m.findCharacterByTarget(s) == target_movie );

        m.loadMovie(url, s, varsToSend, sendVarsMethod); 
        return;
    }

    unsigned int levelno;
    if (m.isLevelTarget(target_string, levelno))
    {
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
SWFHandlers::CommonSetTarget(ActionExec& thread, const std::string& target_name)
{
    as_environment& env = thread.env;

    // see swfdec's settarget-relative-*.swf
    env.reset_target();

    DisplayObject *new_target;

    // if the string is blank, we reset the target to its original value
    if ( target_name.empty() ) return;

    new_target = env.find_target(target_name); // TODO: pass thread.getScopeStack()
    if (new_target == NULL)
    {
        IF_VERBOSE_ASCODING_ERRORS (
        log_aserror(_("Couldn't find movie \"%s\" to set target to!"
            " Setting target to NULL..."), target_name);
        );
        //return;
    }
    //else
    //{
        env.set_target(new_target);
    //}
}

void
SWFHandlers::ActionGetUrl2(ActionExec& thread)
{
    
    as_environment& env = thread.env;

    const action_buffer& code = thread.code;

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_GETURL2));
#endif

    boost::uint8_t method = code[thread.getCurrentPC() + 3];

    as_value url_val = env.top(1);
    if ( url_val.is_undefined() )
    {
        log_error(_("Undefined GetUrl2 url on stack, skipping"));
    }
    else
    {
        const std::string& url = url_val.to_string();
        CommonGetUrl(env, env.top(0), url, method);
    }

    env.drop(2);
}

void
SWFHandlers::ActionBranchIfTrue(ActionExec& thread)
{

    // Alias these
    as_environment& env = thread.env;
    const action_buffer& code = thread.code;
    size_t pc = thread.getCurrentPC();
    size_t nextPC = thread.getNextPC();
    size_t stopPC = thread.getStopPC();

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_BRANCHIFTRUE));
#endif

    boost::int16_t offset = code.read_int16(pc+3);

    bool test = env.pop().to_bool();
    if (test)
    {
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
SWFHandlers::ActionCallFrame(ActionExec& thread)
{
    //GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;

    const std::string& target_frame = env.top(0).to_string();
    std::string target_path;
    std::string frame_var;

    DisplayObject * target = NULL;
    if( env.parse_path(target_frame, target_path, frame_var) )
    {
        target = env.find_target(target_path);
    }
    else
    {
        frame_var = target_frame;
        target = env.get_target();
    }

    MovieClip *target_sprite = target ? target->to_movie() : NULL;
    if(target_sprite)
    {
        target_sprite->call_frame_actions(frame_var);
    }
    else
    {
        IF_VERBOSE_ASCODING_ERRORS (
        log_aserror(_("Couldn't find target_sprite \"%s\" in ActionCallFrame!"
            " target frame actions will not be called..."), target_path);
        )
    }

    env.drop(1);
}

void
SWFHandlers::ActionGotoExpression(ActionExec& thread)
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

    DisplayObject * target = NULL;
    if( env.parse_path(target_frame, target_path, frame_var) )
    {
        target = env.find_target(target_path);
    }

    if ( ! target ) // 4.11 would make parse_path above return true,
                    // we should check if a sprite named '4' is supposed to work
                    // in that case
    {
        target = env.get_target();
        frame_var = target_frame;
    }

    MovieClip *target_sprite = target ? target->to_movie() : NULL;
    if(target_sprite)
    {
        size_t frame_number;
        if ( ! target_sprite->get_frame_number(frame_var, frame_number) )
        {
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
    else
    {
        IF_VERBOSE_ASCODING_ERRORS (
        log_aserror(_("Couldn't find target sprite \"%s\" in "
                "ActionGotoExpression. Will not go to target frame..."),
                target_frame);
        )
    }
}


void
SWFHandlers::ActionDelete(ActionExec& thread)
{
    //GNASH_REPORT_FUNCTION;
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

    boost::intrusive_ptr<as_object> obj;

    // Behaviour is different according to version. For SWF7 and above,
    // the delete fails if there aren't two items on the stack. For SWF6
    // and below, a single item should be parsed to see if it's a path,
    // then we try to delete it. If it's not a path, we try to delete it as
    // a variable.
    //
    // In both cases, if there are two or more items on the stack, they
    // have to be property and object.
    if (stackSize < 2)
    {
        if (version > 6) {
            env.top(1).set_bool(false);
            env.drop(1);
            return;
        }

        std::string path, var;
        if (!as_environment::parse_path(propertyname, path, var))
        {
            // It's not a path. For SWF 7 and above, don't delete. Otherwise
            // assume it's a variable and try to delete.
            env.top(1).set_bool(thread.delVariable(propertyname));
        }
        else {
            as_value target = thread.getVariable(path);
            obj = target.to_object();
            propertyname = var;
        }
    }
    else obj = env.top(1).to_object();

    if (!obj)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("delete %s.%s: no object found to delete"),
                        env.top(1), env.top(0));
        );
        env.top(1).set_bool(false);
        env.drop(1);
        return;
    }

    env.top(1).set_bool(thread.delObjectMember(*obj, propertyname));

    env.drop(1);

}

void
SWFHandlers::ActionDelete2(ActionExec& thread)
{

    as_environment& env = thread.env;

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_DELETE2)); // 0x3B
#endif

    const std::string& propertyname = env.top(0).to_string();

    // If it's not a path, delete it as a variable.
    std::string path, var;
    if (!as_environment::parse_path(propertyname, path, var)) {
        // See bug #18482, this works fine now (assuming the bug
        // report is correct)
        env.top(0) = thread.delVariable(propertyname);
        return;
    }
    
    // Otherwise see if it's an object and delete it.
    as_value target = thread.getVariable(path);
    boost::intrusive_ptr<as_object> obj = target.to_object();

    if (!obj)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("delete2 called with a path that does not resolve "
                    "to an object"), env.top(1), env.top(0));
        );
        env.top(1).set_bool(false);
        env.drop(1);
        return;
    }

    env.top(1).set_bool(thread.delObjectMember(*obj, var));
}

void
SWFHandlers::ActionVarEquals(ActionExec& thread)
{
    
    as_environment& env = thread.env;

    as_value& value = env.top(0);
    as_value& varname = env.top(1);
    thread.setLocalVariable(varname.to_string(), value);

    IF_VERBOSE_ACTION (
    log_action(_("-- set local var: %s = %s"), varname.to_string(), value);
    );

    env.drop(2);
}

void
SWFHandlers::ActionCallFunction(ActionExec& thread)
{

    as_environment& env = thread.env;
    std::string function_name;

    // Let's consider it a as a string and lookup the function.
    const std::string& funcname = env.pop().to_string();
    as_object* this_ptr = thread.getThisPointer();
    as_object* super = NULL;

    //log_debug("ActionCallFunction: thread.getThisPointer returned %s @ %p", typeName(*this_ptr), (void*)this_ptr);

    as_value function = thread.getVariable(funcname, &this_ptr);

    if ( ! function.is_object() )
    {
        IF_VERBOSE_ASCODING_ERRORS (
        log_aserror(_("ActionCallFunction: %s is not an object"), funcname);
        )
    }
    else if ( ! function.is_function() )
    {
        log_error(_("ActionCallFunction: function name %s evaluated to "
                "non-function value %s"), funcname, function);
        // Calling super ? 
        boost::intrusive_ptr<as_object> obj = function.to_object();
        this_ptr = thread.getThisPointer();
        if (!obj->get_member(NSV::PROP_CONSTRUCTOR, &function) )
        {
            IF_VERBOSE_ASCODING_ERRORS (
            log_aserror(_("Object doesn't have a constructor"));
            )
        }
    }
    else if ( function.to_as_function()->isSuper() )
    {
        this_ptr = thread.getThisPointer();

        // the new 'super' will be computed from the old one
        as_function* oldSuper = function.to_as_function();
        super = oldSuper->get_super();
    }

    // Get number of args, modifying it if not enough values are on the stack.
    unsigned nargs = unsigned(env.pop().to_number());
    unsigned available_args = env.stack_size(); 
    if ( available_args < nargs )
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("Attempt to call a function with %u arguments "
            "while only %u are available on the stack."),
            nargs, available_args);
        );
        nargs = available_args;
    }

#ifdef USE_DEBUGGER
    debugger.callStackPush(function_name);
    debugger.matchBreakPoint(function_name, true);
#endif

    std::auto_ptr< std::vector<as_value> > args ( new std::vector<as_value> );
    args->reserve(nargs);
    for (size_t i=0; i<nargs; ++i) args->push_back(env.pop()); 

    as_value result = call_method(function, env, this_ptr,
                  args, super, &(thread.code.getMovieDefinition()));

    env.push(result);

    // If the function threw an exception, do so here.
    if (result.is_exception())
    {
        thread.skipRemainingBuffer();
    }

}

void
SWFHandlers::ActionReturn(ActionExec& thread)
{

    as_environment& env = thread.env;

    // Put top of stack in the provided return slot, if
    // it's not NULL.
    thread.pushReturn(env.top(0));
    env.drop(1);

#ifdef USE_DEBUGGER
        debugger.callStackPop();
#endif

    // Skip the rest of this buffer (return from this action_buffer).
    thread.skipRemainingBuffer();

}

void
SWFHandlers::ActionModulo(ActionExec& thread)
{
    
    as_environment& env = thread.env;

    as_value    result;
    const double y = env.pop().to_number();
    const double x = env.pop().to_number();
    // Don't need to check for y being 0 here - if it's zero,
    // fmod returns NaN, which is what flash would do too
    result = std::fmod(x, y);

    env.push(result);
}

void
SWFHandlers::ActionNew(ActionExec& thread)
{
    
    as_environment& env = thread.env;

    as_value val = env.pop();
    const std::string& classname = val.to_string();

    IF_VERBOSE_ACTION (
        log_action(_("---new object: %s"),
            classname);
    );

    unsigned nargs = unsigned(env.pop().to_number());

    as_value constructorval = thread.getVariable(classname);
    boost::intrusive_ptr<as_function> constructor = constructorval.to_as_function();
    if ( ! constructor )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("ActionNew: "
            "'%s' is not a constructor"), classname);
        );
        env.drop(nargs);
        env.push(as_value()); // should we push an object anyway ?
        return;
    }

    boost::intrusive_ptr<as_object> newobj = construct_object(constructor.get(),
            env, nargs);

#ifdef USE_DEBUGGER
#ifndef GNASH_USE_GC
    // WARNING: new_obj.to_object() can return a newly allocated
    //          thing into the intrusive_ptr, so the debugger
    //          will be left with a deleted object !!
    //          Rob: we don't want to use void pointers here..
    newobj->add_ref(); // this will leak, but at least debugger won't end up
                         // with a dangling reference...
#endif //ndef GNASH_USE_GC
        debugger.addSymbol(newobj.get(), classname);
#endif

    env.push(as_value(newobj));

}

void
SWFHandlers::ActionVar(ActionExec& thread)
{
    
    as_environment& env = thread.env;
    
    const std::string& varname = env.top(0).to_string();
    if ( thread.isFunction() )
    {
       env.declare_local(varname);
    }
    else
    {
       IF_VERBOSE_ASCODING_ERRORS(
       log_aserror(_("The 'var whatever' syntax in timeline context is a no-op."));
       );
    }
    env.drop(1);
}

void
SWFHandlers::ActionInitArray(ActionExec& thread)
{
    
    as_environment& env = thread.env;

    const int array_size = env.pop().to_int();
    assert(array_size >= 0); // TODO: trigger this !!

    // Call the array constructor, to create an empty array.
    as_value result = array_new(fn_call(NULL, env));

    boost::intrusive_ptr<as_object> ao = result.to_object();
    assert(ao);

    // Fill the elements with the initial values from the stack.
    for (int i = 0; i < array_size; i++) {
        // @@ TODO a set_member that takes an int or as_value?
        thread.setObjectMember(*ao, boost::lexical_cast<std::string>(i),
                env.pop());
    }

    env.push(result);

}

void
SWFHandlers::ActionInitObject(ActionExec& thread)
{

    as_environment& env = thread.env;

    //
    //    SWFACTION_PUSH
    //     [000]   Constant: 1 "obj"
    //     [001]   Constant: 0 "member" <-- we handle up to here
    //     [002]   Integer: 1
    //     [003]   Integer: 1
    //    SWFACTION_INITOBJECT

    const int nmembers = env.pop().to_int();

    boost::intrusive_ptr<as_object> new_obj_ptr(init_object_instance());

    // Set provided members
    for (int i = 0; i < nmembers; ++i) {
        as_value member_value = env.top(0);
        std::string member_name = env.top(1).to_string();

        thread.setObjectMember(*new_obj_ptr, member_name, member_value);
        env.drop(2);
    }

    as_value new_obj;
    new_obj.set_as_object(new_obj_ptr.get());

    env.push(new_obj);

}

void
SWFHandlers::ActionTypeOf(ActionExec& thread)
{
    as_environment& env = thread.env;
    env.top(0).set_string(env.top(0).typeOf());
}

void
SWFHandlers::ActionTargetPath(ActionExec& thread)
{
    

    as_environment& env = thread.env;

    boost::intrusive_ptr<MovieClip> sp = env.top(0).to_sprite();
    if ( sp )
    {
        env.top(0).set_string(sp->getTarget());
    }
    else
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Argument to TargetPath(%s) doesn't cast to a MovieClip"),
            env.top(0));
        );
        env.top(0).set_undefined();
    }
}

// Push a each object's member value on the stack
// This is an utility function for use by ActionEnumerate
// and ActionEnum2. The caller is expected to have
// already set the top-of-stack to undefined (as an optimization)
static void
enumerateObject(as_environment& env, const as_object& obj)
{
    assert(env.top(0).is_undefined());
    obj.enumerateProperties(env);
}

void
SWFHandlers::ActionEnumerate(ActionExec& thread)
{
    
    as_environment& env = thread.env;

    // Get the object
    as_value var_name = env.top(0);
    std::string var_string = var_name.to_string();

    as_value variable = thread.getVariable(var_string);

    env.top(0).set_undefined();

    const boost::intrusive_ptr<as_object> obj = variable.to_object();
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
SWFHandlers::ActionNewAdd(ActionExec& thread)
{
    //GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;

    as_value v1 = env.top(0);
    as_value v2 = env.top(1);

    try { v1 = v1.to_primitive(); }
    catch (ActionTypeError& e)
    {
        log_debug(_("%s.to_primitive() threw an error during ActionNewAdd"),
            env.top(0));
    }

    try { v2 = v2.to_primitive(); }
    catch (ActionTypeError& e)
    {
        log_debug(_("%s.to_primitive() threw an error during ActionNewAdd"),
            env.top(1));
    }

#if GNASH_DEBUG
    log_debug(_("ActionNewAdd(%s, %s) [primitive conversion done]"),
                v1, v2);
#endif

    if (v1.is_string() || v2.is_string())
    {
        // NOTE: I've tested that we should change behaviour
        //       based on code definition version, not top-level
        //       SWF version. Just not automated yet.
        //
        const int version = thread.code.getDefinitionVersion();
        v2.convert_to_string_versioned(version);
        v2.string_concat(v1.to_string_versioned(version));
        env.top(1) = v2;
    }
    else
    {
        // use numeric semantic
        const double v2num = v2.to_number();
        const double v1num = v1.to_number();

        v2.set_double(v2num + v1num); 

        env.top(1) = v2;
    }
    env.drop(1);
}

void
SWFHandlers::ActionNewLessThan(ActionExec& thread)
{

    as_environment& env = thread.env;

    as_value operand1 = env.top(1);
    as_value operand2 = env.top(0);

    try { operand1 = operand1.to_primitive(as_value::NUMBER); }
    catch (ActionTypeError& e)
    {
        log_debug(_("%s.to_primitive() threw an error during "
                "ActionNewLessThan"), operand1);
    }
    if ( operand1.is_object() && !operand1.is_sprite() )
    {
        // comparison involving an object (NOT sprite!) is always false
        env.top(1).set_bool(false);
        env.drop(1);
        return;
    }

    try { operand2 = operand2.to_primitive(as_value::NUMBER); }
    catch (ActionTypeError& e)
    {
        log_debug(_("%s.to_primitive() threw an error during "
                "ActionNewLessThan"), operand2);
    }
    if ( operand2.is_object() && !operand2.is_sprite() )
    {
        // comparison involving an object (NOT sprite!) is always false
        env.top(1).set_bool(false);
        env.drop(1);
        return;
    }

    if ( operand1.is_string() && operand2.is_string() )
    {
        const std::string& s1 = operand1.to_string();
        const std::string& s2 = operand2.to_string();
        // Don't ask me why, but an empty string is not less than a non-empty one
        if ( s1.empty() ) {
            env.top(1).set_bool(false);
        } else if ( s2.empty() ) {
            env.top(1).set_bool(true);
        }
        else env.top(1).set_bool(s1 < s2);
    }
    else
    {
        const double op1 = operand1.to_number();
        const double op2 = operand2.to_number();

        if ( isNaN(op1) || isNaN(op2) )
        {
            env.top(1).set_undefined();
        }
        else
        {
            env.top(1).set_bool(op1<op2);
        }
    }
    env.drop(1);
}

void
SWFHandlers::ActionNewEquals(ActionExec& thread)
{
    as_environment& env = thread.env;

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_NEWEQUALS));
#endif

    const VM& vm = getVM(env);

    int swfVersion = vm.getSWFVersion();
    if ( swfVersion <= 5 )
    {
        as_value op1 = env.top(0);
        try { op1 = op1.to_primitive(); }
        catch (ActionTypeError& e)
        {
            log_debug(_("to_primitive(%s) threw an ActionTypeError %s"),
                    op1, e.what());
        }

        as_value op2 = env.top(1);
        try { op2 = op2.to_primitive(); }
        catch (ActionTypeError& e)
        {
            log_debug(_("to_primitive(%s) threw an ActionTypeError %s"),
                    op2, e.what());
        }

        env.top(1).set_bool(op1.equals(op2));
    }
    else
    {
        /// ECMA-262 abstract equality comparison (sect 11.9.3)
        env.top(1).set_bool(env.top(1).equals(env.top(0)));
    }
    env.drop(1);
}

void
SWFHandlers::ActionToNumber(ActionExec& thread)
{
    as_environment& env = thread.env;
    env.top(0).convert_to_number();
}

void
SWFHandlers::ActionToString(ActionExec& thread)
{
    as_environment& env = thread.env;
    
    const int version = env.get_version();
    env.top(0).convert_to_string_versioned(version);
}

void
SWFHandlers::ActionDup(ActionExec& thread)
{
    as_environment& env = thread.env;
    
    env.push(env.top(0));
}

void
SWFHandlers::ActionSwap(ActionExec& thread)
{
    as_environment& env = thread.env;
    
    as_value    temp = env.top(1);
    env.top(1) = env.top(0);
    env.top(0) = temp;
}

void
SWFHandlers::ActionGetMember(ActionExec& thread)
{
        
    as_environment& env = thread.env;

    as_value member_name = env.top(0);
    as_value target = env.top(1);

    boost::intrusive_ptr<as_object> obj = target.to_object();
    if (!obj)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("getMember called against "
            "a value that does not cast "
            "to an as_object: %s"),
            target));
        env.top(1).set_undefined();
        env.drop(1);
        return;
    }

    IF_VERBOSE_ACTION (
    log_action(_(" ActionGetMember: target: %s (object %p)"),
               target, (void*)obj.get());
    );

    if (!thread.getObjectMember(*obj, member_name.to_string(), env.top(1)))
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror("Reference to undefined member %s of object %s",
            member_name,
            target);
        );
        env.top(1).set_undefined();
        }

    IF_VERBOSE_ACTION (
        log_action(_("-- get_member %s.%s=%s"),
           target,
                   member_name,
                   env.top(1));
    );

    env.drop(1);

}

void
SWFHandlers::ActionSetMember(ActionExec& thread)
{
    
    as_environment& env = thread.env;

    boost::intrusive_ptr<as_object> obj = env.top(2).to_object();
    const std::string& member_name = env.top(1).to_string();
    const as_value& member_value = env.top(0);

    if ( member_name.empty() )
    {
        IF_VERBOSE_ASCODING_ERRORS (
            // Invalid object, can't set.
            log_aserror(_("ActionSetMember: %s.%s=%s: member name evaluates to invalid (empty) string"),
                env.top(2),
                env.top(1),
                env.top(0));
        );
    }
    else if (obj)
    {
        thread.setObjectMember(*(obj.get()), member_name, member_value);

        IF_VERBOSE_ACTION (
            log_action(_("-- set_member %s.%s=%s"),
                env.top(2),
                member_name,
                member_value);
        );
    }
    else
    {
        // Malformed SWF ? (don't think this is possible to do with ActionScript syntax)
        // FIXME, should this be log_swferror?
        IF_VERBOSE_ASCODING_ERRORS (
            // Invalid object, can't set.
            log_aserror(_("-- set_member %s.%s=%s on invalid object!"),
                env.top(2),
                member_name,
                member_value);
        );
    }

    env.drop(3);
}

void
SWFHandlers::ActionIncrement(ActionExec& thread)
{
    as_environment& env = thread.env;
    env.top(0).set_double(env.top(0).to_number()+1);
}

void
SWFHandlers::ActionDecrement(ActionExec& thread)
{
    as_environment& env = thread.env;
    env.top(0).set_double(env.top(0).to_number()-1);
}

void
SWFHandlers::ActionCallMethod(ActionExec& thread)
{
    as_environment& env = thread.env;

    // Get name function of the method
    as_value method_name = env.pop();

    // Get an object
    as_value obj_value = env.pop();

    // Get number of args, modifying it if not enough values are on the stack.
    unsigned nargs = unsigned(env.pop().to_number());
    unsigned available_args = env.stack_size(); 
    if ( available_args < nargs )
    {
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

    std::string method_string = method_name.to_string();

    bool hasMethodName = ((!method_name.is_undefined()) &&
            (!method_string.empty()) );

    boost::intrusive_ptr<as_object> obj = obj_value.to_object();
    if (!obj) {
        // SWF integrity check
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("ActionCallMethod invoked with "
            "non-object object/func (%s)"), obj_value);
        );
        env.drop(nargs);
        env.push(as_value());
        return;
    }

    as_object* this_ptr = obj.get();

    if ( obj->isSuper() )
    {
        if ( thread.isFunction() ) this_ptr = thread.getThisPointer();
    }
    as_object* super = obj->get_super(hasMethodName ? method_string.c_str() : 0);

    as_value method_val;

    if (!hasMethodName) {
        // We'll be calling the super constructor here
        method_val = obj_value;

        if (!method_val.is_function())
        {

            log_debug(_("Function object given to ActionCallMethod"
                       " is not a function (%s), will try to use"
                       " its 'constructor' member (but should instead "
                       "invoke its [[Call]] method"), obj_value);

            // TODO: all this crap should go into an
            // as_object::getConstructor instead
            as_value ctor;
            if (!obj->get_member(NSV::PROP_CONSTRUCTOR, &ctor) )
            {
                IF_VERBOSE_ASCODING_ERRORS(
                    log_aserror(_("ActionCallMethod: object has no "
                            "constructor"));
                );
                env.drop(nargs);
                env.push(as_value());
                return;
            }
            if (!ctor.is_function())
            {
                IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("ActionCallMethod: object constructor "
                        "is not a function"));
                );
                env.drop(nargs);
                env.push(as_value());
                return;
            }
            method_val = ctor;
        }
    }
    else
    {

        if ( ! thread.getObjectMember(*obj, method_string, method_val) )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("ActionCallMethod: "
                "Can't find method %s of object %s"),
                method_name,
                obj_value);
            );
            env.drop(nargs);
            env.push(as_value()); // should we push an object anyway ?
            return;
        }
    }

#ifdef USE_DEBUGGER
//    log_debug (_("FIXME: method name is: %s"), method_namexxx);
//    // IT IS NOT GUARANTEE WE DO HAVE A METHOD NAME HERE !
    if ( ! method_name.is_undefined() )
    {
        debugger.callStackPush(method_name.to_string());
        debugger.matchBreakPoint(method_name.to_string(), true);
    }
    else
    {
        LOG_ONCE( log_unimpl(_("FIXME: debugger doesn't deal with anonymous function calls")) );
    }
#endif

    std::auto_ptr< std::vector<as_value> > args ( new std::vector<as_value> );
    args->reserve(nargs);
    for (size_t i=0; i<nargs; ++i) args->push_back(env.pop()); 

    as_value result = call_method(method_val, env, this_ptr, 
            args, super, &(thread.code.getMovieDefinition()));

    env.push(result);

    // Now, if there was an exception, proceed to the end of the block.
    if (result.is_exception()) thread.skipRemainingBuffer();

}

void
SWFHandlers::ActionNewMethod(ActionExec& thread)
{

    as_environment& env = thread.env;

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_NEWMETHOD));
#endif

    as_value method_name = env.pop();
    as_value obj_val = env.pop();

    // Get number of args, modifying it if not enough values are on the stack.
    unsigned nargs = unsigned(env.pop().to_number());
    unsigned available_args = env.stack_size(); // previous 3 entries popped
    if (available_args < nargs) {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("Attempt to call a constructor with %u arguments "
            "while only %u are available on the stack."),
            nargs, available_args);
        );
        nargs = available_args;
    }

    boost::intrusive_ptr<as_object> obj = obj_val.to_object();
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

    std::string method_string = method_name.to_string();
    as_value method_val;
    if (method_name.is_undefined() || method_string.empty()) {
        method_val = obj_val;
    }
    else {
        if (!thread.getObjectMember(*obj, method_string, method_val)) {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("ActionNewMethod: can't find method %s of "
                        "object %s"), method_string, obj_val);
            );
            env.drop(nargs);
            env.push(as_value()); 
            return;
        }
    }

    boost::intrusive_ptr<as_function> method = method_val.to_as_function();
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
    boost::intrusive_ptr<as_object> new_obj = construct_object(method.get(),
            env, nargs);

    env.push(as_value(new_obj));

}

void
SWFHandlers::ActionInstanceOf(ActionExec& thread)
{
    
    as_environment& env = thread.env;

    // Get the "super" function
    as_object* super = env.top(0).to_object().get();

    // Get the "instance" (but avoid implicit conversion of primitive values!)
    as_object* instance = env.top(1).is_object() ? env.top(1).to_object().get() : NULL;

    // Invalid args!
    if (!super || ! instance) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("-- %s instanceof %s (invalid args?)"),
                env.top(1),
                env.top(0));
        );

        env.drop(1);
        env.top(0) = as_value(false);
        return;
    }

    env.drop(1);
    env.top(0) = as_value(instance->instanceOf(super));
}

void
SWFHandlers::ActionEnum2(ActionExec& thread)
{

    as_environment& env = thread.env;

    // Get the object.
    // Copy it so we can override env.top(0)
    as_value obj_val = env.top(0);

    // End of the enumeration. Won't override the object
    // as we copied that as_value.
    env.top(0).set_undefined();

    const boost::intrusive_ptr<as_object> obj = obj_val.to_object();
    if ( !obj || !obj_val.is_object() )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Top of stack not an object %s at ActionEnum2 "
            " execution"), obj_val);
        );
        return;
    }

    enumerateObject(env, *obj);

}

void
SWFHandlers::ActionBitwiseAnd(ActionExec& thread)
{
    as_environment& env = thread.env;

    int operand1 = env.top(1).to_int();
    int operand2 = env.top(0).to_int();

    env.top(1) = operand1 & operand2;
    env.drop(1);
}

void
SWFHandlers::ActionBitwiseOr(ActionExec& thread)
{
    
    as_environment& env = thread.env;

    int operand1 = env.top(1).to_int();
    int operand2 = env.top(0).to_int();

    env.top(1) = operand1|operand2;
    env.drop(1);
}

void
SWFHandlers::ActionBitwiseXor(ActionExec& thread)
{

    as_environment& env = thread.env;

    int operand1 = env.top(1).to_int();
    int operand2 = env.top(0).to_int();

    env.top(1) = operand1^operand2;
    env.drop(1);
}

void
SWFHandlers::ActionShiftLeft(ActionExec& thread)
{

    as_environment& env = thread.env;

    /// A left shift of more than or equal to the size in
    /// bits of the left operand, or a negative shift, results
    /// in undefined behaviour in C++.
    boost::int32_t amount = env.top(0).to_int() % 32;
    if (amount < 0) amount += 32;
    
    boost::int32_t value = env.top(1).to_int();

    value = value << amount;

    env.top(1) = value;
    env.drop(1);
}

void
SWFHandlers::ActionShiftRight(ActionExec& thread)
{

    as_environment& env = thread.env;

    boost::uint32_t amount = env.top(0).to_int();
    boost::int32_t value = env.top(1).to_int();

    value = value >> amount;

    env.top(1) = value;
    env.drop(1);
}

void
SWFHandlers::ActionShiftRight2(ActionExec& thread)
{

    as_environment& env = thread.env;

    boost::uint32_t amount = env.top(0).to_int(); 
    boost::int32_t value = env.top(1).to_int();

    value = boost::uint32_t(value) >> amount;

    env.top(1) = value;
    env.drop(1);
}

void
SWFHandlers::ActionStrictEq(ActionExec& thread)
{
    
    as_environment& env = thread.env;
    
    env.top(1).set_bool(env.top(1).strictly_equals(env.top(0)));
        env.drop(1);
}

void
SWFHandlers::ActionGreater(ActionExec& thread)
{
    // Just swap the operator and invoke ActionNewLessThan
    as_environment& env = thread.env;
    as_value tmp = env.top(1);
    env.top(1) = env.top(0);
    env.top(0) = tmp;
    ActionNewLessThan(thread);
}

void
SWFHandlers::ActionStringGreater(ActionExec& thread)
{
    
    as_environment& env = thread.env;
    
    // No need to use to_string_versioned() here, this is a swf7 opcode
    env.top(1).set_bool(env.top(1).to_string() > env.top(0).to_string());
    env.drop(1);
}

void
SWFHandlers::ActionExtends(ActionExec& thread)
{

    as_environment& env = thread.env;

    as_function* super = env.top(0).to_as_function();
    as_function* sub = env.top(1).to_as_function();

    if ( ! super || ! sub )
    {
        IF_VERBOSE_ASCODING_ERRORS
        (
            if ( ! super )
            {
                log_aserror(_("ActionExtends: Super is not an as_function (%s)"),
                    env.top(0));
            }
            if ( ! sub )
            {
                log_aserror(_("ActionExtends: Sub is not an as_function (%s)"),
                    env.top(1));
            }
        );
        env.drop(2);
        return;
    }
    env.drop(2);

    sub->extends(*super);

}

void
SWFHandlers::ActionConstantPool(ActionExec& thread)
{
    thread.code.process_decl_dict(thread.getCurrentPC(), thread.getNextPC());
}

void
SWFHandlers::ActionDefineFunction2(ActionExec& thread)
{

    as_environment& env = thread.env;
    const action_buffer& code = thread.code;

    // Code starts at thread.getNextPC() as the DefineFunction tag
    // contains name and args, while next tag is first tag
    // of the function body.
    swf_function* func = new swf_function(
        &code, &env, thread.getNextPC(), thread.getScopeStack());

    func->set_is_function2();

    size_t i = thread.getCurrentPC() + 3; // skip tag id and length

    // Extract name.
    // @@ security: watch out for possible missing terminator here!
    std::string name = code.read_string(i);
    i += name.length() + 1; // add NULL-termination

    // Get number of arguments.
    const unsigned nargs = code.read_int16(i);
    i += 2;

    // Get the count of local registers used by this function.
    boost::uint8_t register_count = code[i];
    i++;

    func->set_local_register_count(register_count);

    // Flags, for controlling register assignment of implicit args.
    boost::uint16_t    flags = code.read_int16(i);
    i += 2;

    func->set_function2_flags(flags);

    // Get the register assignments and names of the arguments.
    for (unsigned n = 0; n < nargs; n++)
    {
        boost::uint8_t arg_register = code[i];
        ++i;

        // @@ security: watch out for possible missing terminator here!
        const char* arg = code.read_string(i);

        func->add_arg(arg_register, arg);
        i += strlen(arg)+1;
    }

    // Get the length of the actual function code.
    boost::uint16_t code_size = code.read_int16(i);

    // Check code_size value consistency
    size_t actionbuf_size = thread.code.size();
    if ( thread.getNextPC() + code_size > actionbuf_size )
    {
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
    func->set_length(code_size);

    // Skip the function body (don't interpret it now).
    thread.adjustNextPC(code_size);

    // If we have a name, then save the function in this
    // environment under that name.
    as_value function_value(func);
    if (!name.empty())
    {
        IF_VERBOSE_ACTION(
            log_action(_("DefineFunction2: named function '%s' "
                        "starts at PC %d"), name, func->getStartPC());
        );

        //env.set_member(name, function_value);
        thread.setVariable(name, function_value);
    }

    // Otherwise push the function literal on the stack
    else
    {
        IF_VERBOSE_ACTION(
            log_action(_("DefineFunction2: anonymous function starts at "
                        "PC %d"), func->getStartPC());
        );
        env.push(function_value);
    }
#ifdef USE_DEBUGGER
    // WARNING: function_value.to_object() can return a newly allocated
    //          thing into the intrusive_ptr, so the debugger
    //          will be left with a deleted object !!
    //          Rob: we don't want to use void pointers here..
    boost::intrusive_ptr<as_object> o = function_value.to_object();
#ifndef GNASH_USE_GC
    o->add_ref(); // this will leak, but at least debugger won't end up
                  // with a dangling reference...
#endif //ndef GNASH_USE_GC
        debugger.addSymbol(o.get(), name);
#endif
}

void
SWFHandlers::ActionTry(ActionExec& thread)
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

    if (!catchInRegister)
    {
        catchName = code.read_string(i);
        i += strlen(catchName) + 1;
        TryBlock t(i, trySize, catchSize, finallySize, catchName);
        thread.pushTryBlock(t);
    }
    else
    {
        catchRegister = code[i];
        ++i;
        TryBlock t(i, trySize, catchSize, finallySize, catchRegister);
        thread.pushTryBlock(t);
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
SWFHandlers::ActionWith(ActionExec& thread)
{
    

    as_environment& env = thread.env;
    const action_buffer& code = thread.code;
    size_t pc = thread.getCurrentPC();

#if GNASH_PARANOIA_LEVEL > 1
    assert(thread.atActionTag(SWF::ACTION_WITH));
#endif

    
    as_value with_obj_val = env.pop().to_object();
    boost::intrusive_ptr<as_object> with_obj = with_obj_val.to_object();

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

    if ( ! with_obj )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("with(%s) : first argument doesn't cast to an object!"),
            with_obj_val);
        );
        // skip the full block
        thread.adjustNextPC(block_length);
        return;
    }

    // where does the 'with' block ends ?
    unsigned block_end = thread.getNextPC() + block_length;

    if ( ! thread.pushWithEntry(with_stack_entry(with_obj, block_end)) )
    {
        // skip the full block
        thread.adjustNextPC(block_length);
    }

}

void
SWFHandlers::ActionDefineFunction(ActionExec& thread)
{

    as_environment& env = thread.env;
    const action_buffer& code = thread.code;

#ifndef NDEBUG
    // TODO: check effects of the following 'length' 
    boost::int16_t length = code.read_int16(thread.getCurrentPC()+1);
    assert( length >= 0 );
#endif

    // Create a new swf_function
    // Code starts at thread.getNextPC() as the DefineFunction tag
    // contains name and args, while next tag is first tag
    // of the function body.
    swf_function* func = new swf_function(
        &code, &env, thread.getNextPC(), thread.getScopeStack());

    size_t i = thread.getCurrentPC() + 3;

    // Extract name.
    // @@ security: watch out for possible missing terminator here!
    std::string name = code.read_string(i);
    i += name.length() + 1;

    // Get number of arguments.
    unsigned nargs = code.read_int16(i);
    i += 2;

    // Get the names of the arguments.
    for (unsigned n = 0; n < nargs; n++)
    {
        const char* arg = code.read_string(i);

        // @@ security: watch out for possible missing terminator here!
        func->add_arg(0, arg);
        // wouldn't it be simpler to use strlen(arg)+1 ?
        i += strlen(arg)+1; // func->m_args.back().m_name.length() + 1;
    }

    // Get the length of the actual function code.
    boost::int16_t code_size = code.read_int16(i);

    func->set_length(code_size);


    // Skip the function body (don't interpret it now).
    // getNextPC() is assumed to point to first action of
    // the function body (one-past the current tag, whic
    // is DefineFunction). We add code_size to it.
    thread.adjustNextPC(code_size);

    // If we have a name, then save the function in this
    // environment under that name.
    as_value    function_value(func);
    if (!name.empty())
    {
        IF_VERBOSE_ACTION(
            log_action("DefineFunction: named function '%s' starts at "
                        "PC %d", name, func->getStartPC());
        );

        //env.set_member(name, function_value);
        thread.setVariable(name, function_value);
#ifdef USE_DEBUGGER
        // WARNING: new_obj.to_object() can return a newly allocated
        //          thing into the intrusive_ptr, so the debugger
        //          will be left with a deleted object !!
        //          Rob: we don't want to use void pointers here..
        boost::intrusive_ptr<as_object> o = function_value.to_object();
#ifndef GNASH_USE_GC
        o->add_ref(); // this will leak, but at least debugger won't end up
                  // with a dandling reference...
#endif //ndef GNASH_USE_GC
                debugger.addSymbol(o.get(), name);
#endif
    }

    // Otherwise push the function literal on the stack
    else
    {
        IF_VERBOSE_ACTION(
        log_action("DefineFunction: anonymous function starts at "
                    "PC %d", func->getStartPC());
        );

        env.push(function_value);
    }

    //env.dump_stack();
}

void
SWFHandlers::ActionSetRegister(ActionExec& thread)
{

    as_environment& env = thread.env;

    const action_buffer& code = thread.code;

    unsigned int reg = code[thread.getCurrentPC() + 3];

    // Save top of stack in specified register.
    int ret = env.setRegister(reg, env.top(0));
    if ( ! ret )
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("Invalid register %d in ActionSetRegister"), reg);
        );
    }
    else if ( ret == 1 )
    {
        IF_VERBOSE_ACTION (
        log_action(_("-------------- global register[%d] = '%s'"),
            reg, env.top(0));
        );
    }
    else
    {
        IF_VERBOSE_ACTION (
        log_action(_("-------------- local register[%d] = '%s'"),
            reg, env.top(0));
        );
    }
}

const char*
SWFHandlers::action_name(ActionType x) const
{
    if ( static_cast<size_t>(x) > get_handlers().size() )
    {
        log_error(_("at SWFHandlers::action_name(%d) call time, "
                    "_handlers size is %d"),
                    x, get_handlers().size());
        return NULL;
    }
    else
    {
        return get_handlers()[x].getName().c_str();
    }
}

} // namespace gnash::SWF

} // namespace gnash
