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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "swf.h"
#include "ASHandlers.h"
#include "movie_definition.h"
#include "array.h"
#include "Function.h"
#include "tu_random.h"
#include "fn_call.h"
#include "ActionExec.h"
#include "sprite_instance.h"
#include "as_environment.h"

#include <string>
#include <map>
#include <vector>

using namespace std;

// Forward declarations
namespace gnash {
	extern fscommand_callback s_fscommand_handler;
}

namespace gnash {
namespace SWF { // gnash::SWF

SWFHandlers::container_type SWFHandlers::_handlers(255);
std::vector<std::string> SWFHandlers::_property_names;


// Utility.  Try to convert str to a number.  If successful,
// put the result in *result, and return true.  If not
// successful, put 0 in *result, and return false.
static bool string_to_number(double* result, const char* str)
{
    char* tail = 0;
    *result = strtod(str, &tail);
    if (tail == str || *tail != 0)
	{
	    // Failed conversion to Number.
	    return false;
	}
    return true;
}




static void unsupported_action_handler(ActionExec& /*thread*/)
{
	log_error("Unsupported action handler invoked");
}

ActionHandler::ActionHandler()
	:
	_name("unsupported"),
	_callback(unsupported_action_handler),
	_debug(false),
	_stack_args(0),
	_arg_format(ARG_NONE)
{
//    GNASH_REPORT_FUNCTION;    
}

ActionHandler::ActionHandler(action_type type, action_callback_t func)
	:
	_type(type),
	_callback(func),
	_debug(false),
	_stack_args(0),
	_arg_format(ARG_NONE)
{
//	GNASH_REPORT_FUNCTION;
}

ActionHandler::ActionHandler(action_type type, string name,
                             action_callback_t func)
	:
	_type(type),
	_name(name),
	_callback(func),
	_debug(false),
	_stack_args(0),
	_arg_format(ARG_NONE)
{
//	GNASH_REPORT_FUNCTION;
}

ActionHandler::ActionHandler(action_type type, string name,
                             action_callback_t func, as_arg_t format)
    : _debug(false), _stack_args(0)
{
//    GNASH_REPORT_FUNCTION;
    _name = name;
    _type = type;
    _callback = func;
    _arg_format = format;
}

ActionHandler::ActionHandler(action_type type, string name,
                             action_callback_t func, as_arg_t format, int nargs)
    : _debug(false)
{
//    GNASH_REPORT_FUNCTION;
    _name = name;
    _type = type;
    _callback = func;
    _stack_args = nargs;
    _arg_format = format;
}

ActionHandler::~ActionHandler()
{
//    GNASH_REPORT_FUNCTION;
}

void
ActionHandler::execute(ActionExec& thread) const
{
//    GNASH_REPORT_FUNCTION;
    return _callback(thread);
}

SWFHandlers::SWFHandlers()
{
//    GNASH_REPORT_FUNCTION;
	_property_names.reserve(32);
    _property_names.push_back("_x");
    _property_names.push_back("_y");
    _property_names.push_back("_xscale");
    _property_names.push_back("_yscale");
    _property_names.push_back("_currentframe");
    _property_names.push_back("_totalframes");
    _property_names.push_back("_alpha");
    _property_names.push_back("_visible");
    _property_names.push_back("_width");
    _property_names.push_back("_height");
    _property_names.push_back("_rotation");
    _property_names.push_back("_target");
    _property_names.push_back("_framesloaded");
    _property_names.push_back("_name");
    _property_names.push_back("_droptarget");
    _property_names.push_back("_url");
    _property_names.push_back("_highquality");
    _property_names.push_back("_focusrect");
    _property_names.push_back("_soundbuftime");
    _property_names.push_back("@@ mystery quality member");
    _property_names.push_back("_xmouse");
    _property_names.push_back("_ymouse");

    _handlers[ACTION_END] = ActionHandler(ACTION_END,
             string("<End>"), SWFHandlers::ActionEnd);
    _handlers[ACTION_NEXTFRAME] = ActionHandler(ACTION_NEXTFRAME,
             string("NextFrame"), SWFHandlers::ActionNextFrame);
    _handlers[ACTION_PREVFRAME] =  ActionHandler(ACTION_PREVFRAME,
             string("PreviousFrame"), SWFHandlers::ActionPrevFrame);
    _handlers[ACTION_PLAY] = ActionHandler(ACTION_PLAY,
             string("Play"), SWFHandlers::ActionPlay);
    _handlers[ACTION_STOP] = ActionHandler(ACTION_STOP,
             string("Stop"), SWFHandlers::ActionStop);
    _handlers[ACTION_TOGGLEQUALITY] = ActionHandler(ACTION_TOGGLEQUALITY,
             string("ToggleQuality"), SWFHandlers::ActionToggleQuality);
    _handlers[ACTION_STOPSOUNDS] = ActionHandler(ACTION_STOPSOUNDS,
             string("StopSounds"), SWFHandlers::ActionStopSounds);
    _handlers[ACTION_GOTOFRAME] = ActionHandler(ACTION_GOTOFRAME,
             string("GotoFrame"), SWFHandlers::ActionGotoFrame, ARG_U16);
    _handlers[ACTION_GETURL] = ActionHandler(ACTION_GETURL,
             string("GetUrl"), SWFHandlers::ActionGetUrl, ARG_STR);
    _handlers[ACTION_WAITFORFRAME] = ActionHandler(ACTION_WAITFORFRAME,
             string("WaitForFrame"), SWFHandlers::ActionWaitForFrame, ARG_HEX);
    _handlers[ACTION_SETTARGET] = ActionHandler(ACTION_SETTARGET,
             string("SetTarget"), SWFHandlers::ActionSetTarget, ARG_STR);
    _handlers[ACTION_GOTOLABEL] = ActionHandler(ACTION_GOTOLABEL,
             string("GotoLabel"), SWFHandlers::ActionGotoLabel, ARG_STR);
    _handlers[ACTION_ADD] = ActionHandler(ACTION_ADD,
             string("Add"), SWFHandlers::ActionAdd);
    _handlers[ACTION_SUBTRACT] = ActionHandler(ACTION_SUBTRACT,
             string("Subtract"), SWFHandlers::ActionSubtract);
    _handlers[ACTION_MULTIPLY] = ActionHandler(ACTION_MULTIPLY,
             string("Multiply"), SWFHandlers::ActionMultiply);
    _handlers[ACTION_DIVIDE] = ActionHandler(ACTION_DIVIDE,
             string("Divide"), SWFHandlers::ActionDivide);
    _handlers[ACTION_EQUAL] = ActionHandler(ACTION_EQUAL,
             string("Equal"), SWFHandlers::ActionEqual);
    _handlers[ACTION_LESSTHAN] = ActionHandler(ACTION_LESSTHAN,
             string("LessThan"), SWFHandlers::ActionLessThan);
    _handlers[ACTION_LOGICALAND] = ActionHandler(ACTION_LOGICALAND,
             string("LogicalAnd"), SWFHandlers::ActionLogicalAnd);
    _handlers[ACTION_LOGICALOR] = ActionHandler(ACTION_LOGICALOR,
             string("LogicalOr"), SWFHandlers::ActionLogicalOr);
    _handlers[ACTION_LOGICALNOT] = ActionHandler(ACTION_LOGICALNOT,
             string("LogicalNot"), SWFHandlers::ActionLogicalNot);
    _handlers[ACTION_STRINGEQ] = ActionHandler(ACTION_STRINGEQ,
             string("StringEq"), SWFHandlers::ActionStringEq);    
    _handlers[ACTION_STRINGLENGTH] = ActionHandler(ACTION_STRINGLENGTH,
             string("ActionStringLength"), SWFHandlers::ActionStringLength);
    _handlers[ACTION_SUBSTRING] = ActionHandler(ACTION_SUBSTRING,
             string("ActionSubString"), SWFHandlers::ActionSubString);
    _handlers[ACTION_POP] = ActionHandler(ACTION_POP,
             string("ActionPop"), SWFHandlers::ActionPop);
    _handlers[ACTION_INT] = ActionHandler(ACTION_INT,
             string("ActionInt"), SWFHandlers::ActionInt);
    _handlers[ACTION_GETVARIABLE] = ActionHandler(ACTION_GETVARIABLE,
             string("ActionGetVariable"), SWFHandlers::ActionGetVariable);
    _handlers[ACTION_SETVARIABLE] = ActionHandler(ACTION_SETVARIABLE,
             string("ActionSetVariable"), SWFHandlers::ActionSetVariable);
    _handlers[ACTION_SETTARGETEXPRESSION] = ActionHandler(ACTION_SETTARGETEXPRESSION,
             string("ActionSetTargetExpression"), SWFHandlers::ActionSetTargetExpression);
    _handlers[ACTION_STRINGCONCAT] = ActionHandler(ACTION_STRINGCONCAT,
             string("ActionStringConcat"), SWFHandlers::ActionStringConcat);
    _handlers[ACTION_GETPROPERTY] = ActionHandler(ACTION_GETPROPERTY,
             string("ActionGetProperty"), SWFHandlers::ActionGetProperty);
    _handlers[ACTION_SETPROPERTY] = ActionHandler(ACTION_SETPROPERTY,
             string("ActionSetpProperty"), SWFHandlers::ActionSetProperty);
    _handlers[ACTION_DUPLICATECLIP] = ActionHandler(ACTION_DUPLICATECLIP,
             string("ActionDuplicateClip"), SWFHandlers::ActionDuplicateClip);
    _handlers[ACTION_REMOVECLIP] = ActionHandler(ACTION_REMOVECLIP,
             string("ActionRemoveClip"), SWFHandlers::ActionRemoveClip);
    _handlers[ACTION_TRACE] = ActionHandler(ACTION_TRACE,
             string("ActionTrace"), SWFHandlers::ActionTrace);
    _handlers[ACTION_STARTDRAGMOVIE] = ActionHandler(ACTION_STARTDRAGMOVIE,
             string("ActionStartDragMovie"), SWFHandlers::ActionStartDragMovie);
    _handlers[ACTION_STOPDRAGMOVIE] = ActionHandler(ACTION_STOPDRAGMOVIE,
             string("ActionStopDragMovie"), SWFHandlers::ActionStopDragMovie);
    _handlers[ACTION_STRINGCOMPARE] = ActionHandler(ACTION_STRINGCOMPARE,
             string("ActionStringCompare"), SWFHandlers::ActionStringCompare);
    _handlers[ACTION_THROW] = ActionHandler(ACTION_THROW,
             string("ActionThrow"), SWFHandlers::ActionThrow);
    _handlers[ACTION_CASTOP] = ActionHandler(ACTION_CASTOP,
             string("ActionCastOp"), SWFHandlers::ActionCastOp);
    _handlers[ACTION_IMPLEMENTSOP] = ActionHandler(ACTION_IMPLEMENTSOP,
             string("ActionImplementsOp"), SWFHandlers::ActionImplementsOp);
    _handlers[ACTION_RANDOM] = ActionHandler(ACTION_RANDOM,
             string("ActionRandom"), SWFHandlers::ActionRandom);
    _handlers[ACTION_MBLENGTH] = ActionHandler(ACTION_MBLENGTH,
             string("ActionMbLength"), SWFHandlers::ActionMbLength);
    _handlers[ACTION_ORD] = ActionHandler(ACTION_ORD,
             string("ActionOrd"), SWFHandlers::ActionOrd);
    _handlers[ACTION_CHR] = ActionHandler(ACTION_CHR,
             string("ActionChr"), SWFHandlers::ActionChr);
    _handlers[ACTION_GETTIMER] = ActionHandler(ACTION_GETTIMER,
             string("ActionGetTimer"), SWFHandlers::ActionGetTimer);
    _handlers[ACTION_MBSUBSTRING] = ActionHandler(ACTION_MBSUBSTRING,
             string("ActionMbSubString"), SWFHandlers::ActionMbSubString);
    _handlers[ACTION_MBORD] = ActionHandler(ACTION_MBORD,
             string("ActionMbOrd"), SWFHandlers::ActionMbOrd);
    _handlers[ACTION_MBCHR] = ActionHandler(ACTION_MBCHR,
             string("ActionMbChr"), SWFHandlers::ActionMbChr);
    _handlers[ACTION_WAITFORFRAMEEXPRESSION] = ActionHandler(ACTION_WAITFORFRAMEEXPRESSION,
             string("ActionWaitForFrameExpression"),
             SWFHandlers::ActionWaitForFrameExpression, ARG_HEX);
    _handlers[ACTION_PUSHDATA] = ActionHandler(ACTION_PUSHDATA,
             string("ActionPushData"), SWFHandlers::ActionPushData, ARG_PUSH_DATA);
    _handlers[ACTION_BRANCHALWAYS] = ActionHandler(ACTION_BRANCHALWAYS,
             string("ActionBranchAlways"), SWFHandlers::ActionBranchAlways, ARG_S16);
    _handlers[ACTION_GETURL2] = ActionHandler(ACTION_GETURL2,
             string("ActionGetUrl2"), SWFHandlers::ActionGetUrl2, ARG_HEX);
    _handlers[ACTION_BRANCHIFTRUE] = ActionHandler(ACTION_BRANCHIFTRUE,
             string("ActionBranchIfTrue"), SWFHandlers::ActionBranchIfTrue, ARG_S16);
    _handlers[ACTION_CALLFRAME] = ActionHandler(ACTION_CALLFRAME,
             string("ActionCallFrame"), SWFHandlers::ActionCallFrame, ARG_HEX);
    _handlers[ACTION_GOTOEXPRESSION] = ActionHandler(ACTION_GOTOEXPRESSION,
             string("ActionGotoExpression"), SWFHandlers::ActionGotoExpression, ARG_HEX);
    _handlers[ACTION_DELETEVAR] = ActionHandler(ACTION_DELETEVAR,
             string("ActionDeleteVar"), SWFHandlers::ActionDeleteVar);
    _handlers[ACTION_DELETE] = ActionHandler(ACTION_DELETE,
             string("ActionDelete"), SWFHandlers::ActionDelete);
    _handlers[ACTION_VAREQUALS] = ActionHandler(ACTION_VAREQUALS,
             string("ActionVarEquals"), SWFHandlers::ActionVarEquals);
    _handlers[ACTION_CALLFUNCTION] = ActionHandler(ACTION_CALLFUNCTION,
             string("ActionCallFunction"), SWFHandlers::ActionCallFunction);
    _handlers[ACTION_RETURN] = ActionHandler(ACTION_RETURN,
             string("ActionReturn"), SWFHandlers::ActionReturn);
    _handlers[ACTION_MODULO] = ActionHandler(ACTION_MODULO,
             string("ActionModulo"), SWFHandlers::ActionModulo);
    _handlers[ACTION_NEW] = ActionHandler(ACTION_NEW,
             string("ActionNew"), SWFHandlers::ActionNew);
    _handlers[ACTION_VAR] = ActionHandler(ACTION_VAR,
             string("ActionVar"), SWFHandlers::ActionVar);    
    _handlers[ACTION_INITARRAY] = ActionHandler(ACTION_INITARRAY,
             string("ActionInitArray"), SWFHandlers::ActionInitArray);
    _handlers[ACTION_INITOBJECT] = ActionHandler(ACTION_INITOBJECT,
             string("ActionInitObject"), SWFHandlers::ActionInitObject);
    _handlers[ACTION_TYPEOF] = ActionHandler(ACTION_TYPEOF,
             string("ActionTypeOf"), SWFHandlers::ActionTypeOf);
    _handlers[ACTION_TARGETPATH] = ActionHandler(ACTION_TARGETPATH,
             string("ActionTargetPath"), SWFHandlers::ActionTargetPath);
    _handlers[ACTION_ENUMERATE] = ActionHandler(ACTION_ENUMERATE,
             string("ActionEnumerate"), SWFHandlers::ActionEnumerate);
    _handlers[ACTION_NEWADD] = ActionHandler(ACTION_NEWADD,
             string("ActionNewAdd"), SWFHandlers::ActionNewAdd);
    _handlers[ACTION_NEWLESSTHAN] = ActionHandler(ACTION_NEWLESSTHAN,
             string("ActionNewLessThan"), SWFHandlers::ActionNewLessThan);
    _handlers[ACTION_NEWEQUALS] = ActionHandler(ACTION_NEWEQUALS,
             string("ActionNewEquals"), SWFHandlers::ActionNewEquals);
    _handlers[ACTION_TONUMBER] = ActionHandler(ACTION_TONUMBER,
             string("ActionToNumber"), SWFHandlers::ActionToNumber);
    _handlers[ACTION_TOSTRING] = ActionHandler(ACTION_TOSTRING,
             string("ActionToString"), SWFHandlers::ActionToString);
    _handlers[ACTION_DUP] = ActionHandler(ACTION_DUP,
             string("ActionDup"), SWFHandlers::ActionDup);    
    _handlers[ACTION_SWAP] = ActionHandler(ACTION_SWAP,
             string("ActionSwap"), SWFHandlers::ActionSwap);    
    _handlers[ACTION_GETMEMBER] = ActionHandler(ACTION_GETMEMBER,
             string("ActionGetMember"), SWFHandlers::ActionGetMember);
    _handlers[ACTION_SETMEMBER] = ActionHandler(ACTION_SETMEMBER,
             string("ActionSetMember"), SWFHandlers::ActionSetMember);
    _handlers[ACTION_INCREMENT] = ActionHandler(ACTION_INCREMENT,
             string("ActionIncrement"), SWFHandlers::ActionIncrement);
    _handlers[ACTION_DECREMENT] = ActionHandler(ACTION_DECREMENT,
             string("ActionDecrement"), SWFHandlers::ActionDecrement);    
    _handlers[ACTION_CALLMETHOD] = ActionHandler(ACTION_CALLMETHOD,
             string("ActionCallMethod"), SWFHandlers::ActionCallMethod);
    _handlers[ACTION_NEWMETHOD] = ActionHandler(ACTION_NEWMETHOD,
             string("ActionNewMethod"), SWFHandlers::ActionNewMethod);
    _handlers[ACTION_INSTANCEOF] = ActionHandler(ACTION_INSTANCEOF,
             string("ActionInstanceOf"), SWFHandlers::ActionInstanceOf);
    _handlers[ACTION_ENUM2] = ActionHandler(ACTION_ENUM2,
             string("ActionEnum2"), SWFHandlers::ActionEnum2);    
    _handlers[ACTION_BITWISEAND] = ActionHandler(ACTION_BITWISEAND,
             string("ActionBitwiseAnd"), SWFHandlers::ActionBitwiseAnd);
    _handlers[ACTION_BITWISEOR] = ActionHandler(ACTION_BITWISEOR,
             string("ActionBitwiseOr"), SWFHandlers::ActionBitwiseOr);
    _handlers[ACTION_BITWISEXOR] = ActionHandler(ACTION_BITWISEXOR,
             string("ActionBitwiseXor"), SWFHandlers::ActionBitwiseXor);
    _handlers[ACTION_SHIFTLEFT] = ActionHandler(ACTION_SHIFTLEFT,
             string("ActionShiftLeft"), SWFHandlers::ActionShiftLeft);
    _handlers[ACTION_SHIFTRIGHT] = ActionHandler(ACTION_SHIFTRIGHT,
             string("ActionShiftRight"), SWFHandlers::ActionShiftRight);
    _handlers[ACTION_SHIFTRIGHT2] = ActionHandler(ACTION_SHIFTRIGHT2,
             string("ActionShiftRight2"), SWFHandlers::ActionShiftRight2);
    _handlers[ACTION_STRICTEQ] = ActionHandler(ACTION_STRICTEQ,
             string("ActionStrictEq"), SWFHandlers::ActionStrictEq);
    _handlers[ACTION_GREATER] = ActionHandler(ACTION_GREATER,
             string("ActionGreater"), SWFHandlers::ActionGreater);
    _handlers[ACTION_STRINGGREATER] = ActionHandler(ACTION_STRINGGREATER,
             string("ActionStringGreater"), SWFHandlers::ActionStringGreater);
    _handlers[ACTION_EXTENDS] = ActionHandler(ACTION_EXTENDS,
             string("ActionExtends"), SWFHandlers::ActionExtends);
    _handlers[ACTION_CONSTANTPOOL] = ActionHandler(ACTION_CONSTANTPOOL,
             string("ActionConstantPool"), SWFHandlers::ActionConstantPool, ARG_DECL_DICT);
    _handlers[ACTION_DEFINEFUNCTION2] = ActionHandler(ACTION_DEFINEFUNCTION2,
             string("ActionDefineFunction2"), SWFHandlers::ActionDefineFunction2,
             ARG_FUNCTION2);
    _handlers[ACTION_TRY] = ActionHandler(ACTION_TRY,
             string("ActionTry"), SWFHandlers::ActionTry, ARG_FUNCTION2);
    _handlers[ACTION_WITH] = ActionHandler(ACTION_WITH,
             string("ActionWith"), SWFHandlers::ActionWith, ARG_U16);
    _handlers[ACTION_DEFINEFUNCTION] = ActionHandler(ACTION_DEFINEFUNCTION,
             string("ActionDefineFunction"), SWFHandlers::ActionDefineFunction, ARG_HEX);
    _handlers[ACTION_SETREGISTER] = ActionHandler(ACTION_SETREGISTER,
             string("ActionSetRegister"), SWFHandlers::ActionSetRegister, ARG_U8);
    

}

SWFHandlers::~SWFHandlers()
{
//    GNASH_REPORT_FUNCTION;
}

const SWFHandlers&
SWFHandlers::instance()
{
	static SWFHandlers* _instance = new SWFHandlers();
	return *_instance;
}

bool
SWFHandlers::execute(action_type type, ActionExec& thread) const
{
//    GNASH_REPORT_FUNCTION;

	if ( _handlers[type].getName() == "unsupported" ) return false;
	_handlers[type].execute(thread);
	return true;
}

void
SWFHandlers::ActionEnd(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;

	//as_environment& env = thread.env;
#ifndef NDEBUG
	const action_buffer& code = thread.code;
	assert( code[thread.pc] == SWF::ACTION_END );
#endif

	dbglogfile << __PRETTY_FUNCTION__ << ": CHECKME: was broken" << endl;
	thread.next_pc=thread.stop_pc;
}
void
SWFHandlers::ActionNextFrame(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;

    as_environment& env = thread.env;
    const action_buffer& code = thread.code;

    assert( code[thread.pc] == SWF::ACTION_NEXTFRAME );
    env.get_target()->goto_frame(env.get_target()->get_current_frame() + 1);
}

void
SWFHandlers::ActionPrevFrame(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;

    as_environment& env = thread.env;
    const action_buffer& code = thread.code;

    assert( code[thread.pc] == SWF::ACTION_PREVFRAME );
    env.get_target()->goto_frame(env.get_target()->get_current_frame() - 1);
}

void
SWFHandlers::ActionPlay(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;

    as_environment& env = thread.env;
    const action_buffer& code = thread.code;

    assert( code[thread.pc] == SWF::ACTION_PLAY );
    env.get_target()->set_play_state(movie::PLAY);
}

void
SWFHandlers::ActionStop(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;

    as_environment& env = thread.env;
    const action_buffer& code = thread.code;

    assert( code[thread.pc] == SWF::ACTION_STOP );
    env.get_target()->set_play_state(movie::STOP);
}

void
SWFHandlers::ActionToggleQuality(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;

    //as_environment& env = thread.env;
    const action_buffer& code = thread.code;

    assert( code[thread.pc] == SWF::ACTION_TOGGLEQUALITY );
    dbglogfile << __PRETTY_FUNCTION__ << ": unimplemented!" << endl;
}

void
SWFHandlers::ActionStopSounds(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;

	//as_environment& env = thread.env;
	const action_buffer& code = thread.code;

	assert( code[thread.pc] == SWF::ACTION_STOPSOUNDS );

	sound_handler* s = get_sound_handler();
	if (s != NULL)
	{
		s->stop_all_sounds();
	}
}

void
SWFHandlers::ActionGotoFrame(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;

	as_environment& env = thread.env;
	const action_buffer& code = thread.code;

	assert( code[thread.pc] == SWF::ACTION_GOTOFRAME );

	int frame = code.read_int16(thread.pc+3);

	// 0-based already?
	//// Convert from 1-based to 0-based
	//frame--;
	env.get_target()->goto_frame(frame);
}

void
SWFHandlers::ActionGetUrl(ActionExec& thread)
{
//	GNASH_REPORT_FUNCTION;
	as_environment& env = thread.env;
	const action_buffer& code = thread.code;

	assert( code[thread.pc] == SWF::ACTION_GETURL );

	size_t& pc = thread.pc;

	// If this is an FSCommand, then call the callback
	// handler, if any.
		  
	// Two strings as args.
	const char* url = code.read_string(pc+3);
	//size_t url_len = strlen(url);
	const char* target = code.read_string(pc+3);
		  
	// If the url starts with an "http" or "https",
	// then we want to load it into a web browser.
	if (strncmp(url, "http", 4) == 0)
	{
//		if (windowid) {
// 				      Atom mAtom = 486;
// 				      Display *mDisplay = XOpenDisplay(NULL);
// 				      XLockDisplay(mDisplay);
// 				      XChangeProperty (mDisplay, windowid, mAtom,
// 						       XA_STRING, 8, PropModeReplace,
// 						       (unsigned char *)url,
// 						       url_len);
		      
// 				      XUnlockDisplay(mDisplay);
// 				      XCloseDisplay(mDisplay);
//		} else {
		      string command = "firefox -remote \"openurl(";
		      command += url;
		      command += ")\"";
		      dbglogfile << "Launching URL... " << command << endl;
//				  sprite_instance *target = env.get_target();
//				  target->get_url(url);
		      system(command.c_str());
//		}
		return;
	}
		  
	// If the url starts with "FSCommand:", then this is
	// a message for the host app.
	else if (strncmp(url, "FSCommand:", 10) == 0)
	{
		      if (s_fscommand_handler) {
			  // Call into the app.
			  (*s_fscommand_handler)(env.get_target()->get_root_interface(), url + 10, target);
		      }
	}
	else
	{
#ifdef EXTERN_MOVIE
//		log_error("get url: target=%s, url=%s\n", target, url);
		      
		tu_string tu_target = target;
		sprite_instance* target_movie = env.find_target(tu_target);
		if (target_movie != NULL)
		{
			sprite_instance* root_movie = env.get_target()->get_root_movie();
			attach_extern_movie(url, target_movie, root_movie);
		}
		else
		{
			log_error("get url: target %s not found\n", target);
		}
#endif // EXTERN_MOVIE
	}
		  
}

void
SWFHandlers::ActionWaitForFrame(ActionExec& thread)
{
//	GNASH_REPORT_FUNCTION;

	as_environment& env = thread.env;

	// If we haven't loaded a specified frame yet, then we're supposed
	// to skip some specified number of actions.
	//
	// Since we don't load incrementally, just ignore this opcode.

	env.drop(1);

	dbglogfile << __PRETTY_FUNCTION__
		<< ": unimplemented (no need until we play while reading)"
		<< endl;
}

void
SWFHandlers::ActionSetTarget(ActionExec& thread)
{
//	GNASH_REPORT_FUNCTION;

	as_environment& env = thread.env;
	const action_buffer& code = thread.code;
	size_t pc = thread.pc;

	// Change the movie we're working on.
	const char* target_name = code.read_string(pc+3);
	sprite_instance *new_target;
		  
	// if the string is blank, we set target to the root movie
	// TODO - double check this is correct?
	if (target_name[0] == '\0')
	{
		new_target = env.find_target((tu_string)"/");
	}
	else
	{
		new_target = env.find_target((tu_string)target_name);
	}
		  
	if (new_target == NULL)
	{
		log_action("ERROR: Couldn't find movie \"%s\" "
			"to set target to! Not setting target at all...",
			(const char *)target_name);
	}
	else
	{
		env.set_target(new_target);
	}
}

void
SWFHandlers::ActionGotoLabel(ActionExec& thread)
{
//	GNASH_REPORT_FUNCTION;
	as_environment& env = thread.env;
	const action_buffer& code = thread.code;

	const char* frame_label = code.read_string(thread.pc+3);
	sprite_instance *target = env.get_target();
	target->goto_labeled_frame(frame_label);
}

void
SWFHandlers::ActionAdd(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    env.top(1) += env.top(0);
    env.drop(1);
}

void
SWFHandlers::ActionSubtract(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    env.top(1) -= env.top(0);
    env.drop(1);
}

void
SWFHandlers::ActionMultiply(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    env.top(1) *= env.top(0);
    env.drop(1);
}

void
SWFHandlers::ActionDivide(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    env.top(1) /= env.top(0);
    env.drop(1);
}

void
SWFHandlers::ActionEqual(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    env.top(1).set_bool(env.top(1) == env.top(0));
    env.drop(1);
}

void
SWFHandlers::ActionLessThan(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    env.top(1).set_bool(env.top(1) < env.top(0));
    env.drop(1);
}

void
SWFHandlers::ActionLogicalAnd(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    env.top(1).set_bool(env.top(1).to_bool() && env.top(0).to_bool());
    env.drop(1);
}

void
SWFHandlers::ActionLogicalOr(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    env.top(1).set_bool(env.top(1).to_bool() && env.top(0).to_bool());
    env.drop(1);
}

void
SWFHandlers::ActionLogicalNot(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    env.top(0).set_bool(! env.top(0).to_bool());
}

void
SWFHandlers::ActionStringEq(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    env.top(1).set_bool(env.top(1).to_tu_string() == env.top(0).to_tu_string());
    env.drop(1);    
}

void
SWFHandlers::ActionStringLength(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    int version = env.get_target()->get_movie_definition()->get_version();    
    env.top(0).set_int(env.top(0).to_tu_string_versioned(version).utf8_length());
}

void
SWFHandlers::ActionSubString(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    int	size = int(env.top(0).to_number());
    int	base = int(env.top(1).to_number()) - 1;  // 1-based indices
    int version = env.get_target()->get_movie_definition()->get_version();    
    const tu_string&	str = env.top(2).to_tu_string_versioned(version);
    
    // Keep base within range.
    base = iclamp(base, 0, str.length());
    
    // Truncate if necessary.
    size = imin(str.length() - base, size);
    
    // @@ This can be done without new allocations if we get dirtier w/ internals
    // of as_value and tu_string...
    tu_string	new_string = str.c_str() + base;
    new_string.resize(size);
    
    env.drop(2);
    env.top(0).set_tu_string(new_string);
}

void
SWFHandlers::ActionPop(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
	as_environment& env = thread.env;
	if ( ! env.stack_size() )
	{
		// Malformed SWF
		log_warning("Empty stack on ActionPop, bogus SWF ?");
	}
	else
	{
		env.drop(1);
	}
}

void
SWFHandlers::ActionInt(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    env.top(0).set_int(int(floor(env.top(0).to_number())));
}

void
SWFHandlers::ActionGetVariable(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    as_value var_name = env.pop();
    tu_string var_string = var_name.to_tu_string();
    
    as_value variable = env.get_variable(var_string);
    env.push(variable);
    if (variable.to_object() == NULL) {
        log_action("-- get var: %s=%s\n",
                   var_string.c_str(),
                   variable.to_tu_string().c_str());
    } else {
        log_action("-- get var: %s=%s at %p\n",
                   var_string.c_str(),
                   variable.to_tu_string().c_str(),
                   (void*)variable.to_object());
    }
}

void
SWFHandlers::ActionSetVariable(ActionExec& thread)
{
//	GNASH_REPORT_FUNCTION;

	as_environment& env = thread.env;

	// stack must be contain at least two items
	assert ( env.stack_size() > 1 );

	env.set_variable(env.top(1).to_tu_string(), env.top(0));
	log_action("\n-- set var: %s", env.top(1).to_string());

	env.drop(2);
}

void
SWFHandlers::ActionSetTargetExpression(ActionExec& thread)
{
//	GNASH_REPORT_FUNCTION;

	as_environment& env = thread.env;

	const char * target_name = env.top(0).to_string();
	env.drop(1); // pop the target name off the stack
	sprite_instance *new_target;
    
	// if the string is blank, we set target to the root movie
	// TODO - double check this is correct?
	if (target_name[0] == '\0')
	{
		new_target = env.find_target((tu_string)"/");
	}
	else
	{
		new_target = env.find_target((tu_string)target_name);
	}
    
	if (new_target == NULL)
	{
		log_action("ERROR: "
			" Couldn't find movie \"%s\" to set target to!"
			" Not setting target at all...",
			target_name);
	}
	else
	{
		env.set_target(new_target);
	}
}

void
SWFHandlers::ActionStringConcat(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    int version = env.get_target()->get_movie_definition()->get_version();    
    env.top(1).convert_to_string_versioned(version);
    env.top(1).string_concat(env.top(0).to_tu_string_versioned(version));
    env.drop(1);
}

void
SWFHandlers::ActionGetProperty(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
	as_environment& env = thread.env;
	sprite_instance *target = env.find_target(env.top(1));
	unsigned int prop_number = (unsigned int)env.top(0).to_number();
	if (target)
	{
		if ( prop_number < _property_names.size() )
		{
			as_value val;
			target->get_member(_property_names[prop_number].c_str(),
				&val);
			env.top(1) = val;
        	}
		else
		{
			log_error("invalid property query, property "
				"number %d\n", prop_number);
		}
    	}
	else
	{
		env.top(1) = as_value();
	}
	env.drop(1);
}

void
SWFHandlers::ActionSetProperty(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    
    sprite_instance *target = env.find_target(env.top(2));
    unsigned int prop_number = (unsigned int)env.top(1).to_number();
    as_value prop_val = env.top(0);
    
    if (target) {
//        set_property(target, prop_number, env.top(0));
        if ( prop_number < _property_names.size() )
	{
	    target->set_member(_property_names[prop_number].c_str(), prop_val);
	}
	else
	{
	    log_error("invalid set_property, property number %d\n", prop_number);
	}
        
    }
    env.drop(3);
}

void
SWFHandlers::ActionDuplicateClip(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    env.get_target()->clone_display_object(
        env.top(2).to_tu_string(),
        env.top(1).to_tu_string(),
        (int) env.top(0).to_number());
    env.drop(3);
}

void
SWFHandlers::ActionRemoveClip(ActionExec& thread)
{
//	GNASH_REPORT_FUNCTION;
	as_environment& env = thread.env;

	env.get_target()->remove_display_object(env.top(0).to_tu_string());
	env.drop(1);
}

/// \brief Trace messages from the Flash movie using trace();
void
SWFHandlers::ActionTrace(ActionExec& thread)
{
////    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    dbglogfile << env.top(0).to_string() << endl;
    env.drop(1);
}

void
SWFHandlers::ActionStartDragMovie(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    movie::drag_state	st;
    
    st.m_character = env.find_target(env.top(0));
    if (st.m_character == NULL) {
        log_error("start_drag of invalid target '%s'.\n",
                  env.top(0).to_string());
    }
    
    st.m_lock_center = env.top(1).to_bool();
    st.m_bound = env.top(2).to_bool();
    if (st.m_bound) {
        st.m_bound_x0 = (float) env.top(6).to_number();
        st.m_bound_y0 = (float) env.top(5).to_number();
        st.m_bound_x1 = (float) env.top(4).to_number();
        st.m_bound_y1 = (float) env.top(3).to_number();
        env.drop(4);
    }
    env.drop(3);
    
    sprite_instance *root_movie = env.get_target()->get_root_movie();
    assert(root_movie);
    
    if (root_movie && st.m_character) {
        root_movie->set_drag_state(st);
    }
    
}

void
SWFHandlers::ActionStopDragMovie(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    sprite_instance *root_movie = env.get_target()->get_root_movie();
    assert(root_movie);
    root_movie->stop_drag();
}

void
SWFHandlers::ActionStringCompare(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    env.top(1).set_bool(env.top(1).to_tu_string() < env.top(0).to_tu_string());
}

void
SWFHandlers::ActionThrow(ActionExec& /*thread*/)
{
//    GNASH_REPORT_FUNCTION;
    //as_environment& env = thread.env;
    dbglogfile << __PRETTY_FUNCTION__ << ": unimplemented!" << endl;
}

void
SWFHandlers::ActionCastOp(ActionExec& thread)
{
//	GNASH_REPORT_FUNCTION;

	as_environment& env = thread.env;

	// Get the "super" function
	function_as_object* super = env.top(0).to_as_function();

	// Get the "instance" 
	as_object* instance = env.top(1).to_object();

	// Invalid args!
	if (!super || ! instance)
	{
		log_action("-- %s instance_of %s (invalid args?)\n",
			env.top(1).to_string(),
			env.top(0).to_string());

		env.drop(1);
		env.top(0) = as_value(); 
		return;
	}

	env.drop(1);
	if ( instance->instanceOf(super) )
	{
		env.top(0) = as_value(instance);
	}
	else
	{
		env.top(0) = as_value();
	}
}

void
SWFHandlers::ActionImplementsOp(ActionExec& /*thread*/)
{
//	GNASH_REPORT_FUNCTION;

	//as_environment& env = thread.env;
	dbglogfile << __PRETTY_FUNCTION__ << ": unimplemented!" << endl;
}

void
SWFHandlers::ActionRandom(ActionExec& thread)
{
//	GNASH_REPORT_FUNCTION;
	as_environment& env = thread.env;
	int	max = int(env.top(0).to_number());
	if (max < 1) max = 1;
	env.top(0).set_int(tu_random::next_random() % max);
}

void
SWFHandlers::ActionMbLength(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    dbglogfile << __PRETTY_FUNCTION__ << ": unimplemented!" << endl;
}

void
SWFHandlers::ActionOrd(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    env.top(0).set_int(env.top(0).to_string()[0]);
}

void
SWFHandlers::ActionChr(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    char	buf[2];
    buf[0] = int(env.top(0).to_number());
    buf[1] = 0;
    env.top(0).set_string(buf);
}

void
SWFHandlers::ActionGetTimer(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    env.push(floorf(env.get_target()->get_timer() * 1000.0f));
}

void
SWFHandlers::ActionMbSubString(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    dbglogfile << __PRETTY_FUNCTION__ << ": unimplemented!" << endl;
}

void
SWFHandlers::ActionMbOrd(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    dbglogfile << __PRETTY_FUNCTION__ << ": unimplemented!" << endl;
}

void
SWFHandlers::ActionMbChr(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    dbglogfile << __PRETTY_FUNCTION__ << ": unimplemented!" << endl;
}

void
SWFHandlers::ActionWaitForFrameExpression(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    dbglogfile << __PRETTY_FUNCTION__ << ": unimplemented!" << endl;
}

void
SWFHandlers::ActionPushData(ActionExec& thread)
{
//	GNASH_REPORT_FUNCTION;
	as_environment& env = thread.env;

	const action_buffer& code = thread.code;

	size_t pc = thread.pc;
	int16_t length = code.read_int16(pc+1);
	assert( length >= 0 );

	log_action("-------------- push len=%d", length);

	//---------------
	size_t i = pc;
	while (i - pc < length) {
	      uint8_t type = code[3 + i];
		log_action("-------------- push type=%d", type);
	      i++;
	      if (type == 0) {
		  // string
		  const char* str = code.read_string(i+3);
		  i += strlen(str) + 1;
		  env.push(str);
		  
		  log_action("-------------- pushed '%s'", str);
	      } else if (type == 1) {
		
		  float f = code.read_float_little(i+3);
		  i += 4;
		  env.push(f);
		  log_action("-------------- pushed '%g'", f);
	      } else if (type == 2) {
		  as_value nullvalue;
		  nullvalue.set_null();
		  env.push(nullvalue);	
		  
		  log_action("-------------- pushed NULL");
	      } else if (type == 3) {
		  env.push(as_value());
		  
		  log_action("-------------- pushed UNDEFINED");
	      } else if (type == 4) {
		  // contents of register
		  int	reg = code[3 + i];
		  i++;
		  if ( thread.isFunction2() ) {
		      env.push(*(env.local_register_ptr(reg)));
		      log_action("-------------- pushed local register[%d] = '%s'\n",
				  reg,
				  env.top(0).to_string());
		  } else if (reg < 0 || reg >= 4) {
		      env.push(as_value());
		      log_error("push register[%d] -- register out of bounds!\n", reg);
		  } else {
		      env.push(env.m_global_register[reg]);
		      log_action("-------------- pushed global register[%d] = '%s'\n",
				  reg,
				  env.top(0).to_string());
		  }
		  
	      } else if (type == 5) {
		  bool	bool_val = code[i+3] ? true : false;
		  i++;
//			  log_msg("bool(%d)\n", bool_val);
		  env.push(bool_val);
		  
		  log_action("-------------- pushed %s",
			     (bool_val ? "true" : "false"));
	      } else if (type == 6) {
		  double d = code.read_double_wacky(i+3);
		  i += 8;
		  env.push(d);
		  
		  log_action("-------------- pushed double %g", d);
	      } else if (type == 7) {
		  // int32
		  int32_t val = code.read_int32(i+3);
		  i += 4;
		  
		  env.push(val);
		  
		  log_action("-------------- pushed int32 %d",val);
	      } else if (type == 8) {
		  int id = code[3 + i];
		  i++;
		  if ( id < (int) code.dictionary_size() ) {
		      env.push( code.dictionary_get(id) );
		      
		      log_action("-------------- pushed '%s'",
				 code.dictionary_get(id));
		  } else {
		      log_error("dict_lookup(%d) is out of bounds!\n", id);
		      env.push(0);
		      log_action("-------------- pushed 0");
		  }
	      } else if (type == 9) {
		  int	id = code.read_int16(i+3);
		  i += 2;
		  if ( id < (int) code.dictionary_size() ) {
		      env.push( code.dictionary_get(id) );
		      log_action("-------------- pushed '%s'\n",
				code.dictionary_get(id) );
		  } else {
		      log_error("dict_lookup(%d) is out of bounds!\n", id);
		      env.push(0);
		      
		      log_action("-------------- pushed 0");
		  }
	      }
	}
}

void
SWFHandlers::ActionBranchAlways(ActionExec& thread)
{
//	GNASH_REPORT_FUNCTION;

	int16_t offset = thread.code.read_int16(thread.pc+3);
	thread.next_pc += offset;
	// @@ TODO range checks
}

void
SWFHandlers::ActionGetUrl2(ActionExec& thread)
{
//	GNASH_REPORT_FUNCTION;
	as_environment& env = thread.env;
	const action_buffer& code = thread.code;

	assert( code[thread.pc] == SWF::ACTION_GETURL2 );

	// int	method = code[pc + 3];

	const char*	target = env.top(0).to_string();
	const char*	url = env.top(1).to_string();
		  
	// If the url starts with "FSCommand:", then this is
	// a message for the host app.
	if (strncmp(url, "FSCommand:", 10) == 0)
	{
		if (s_fscommand_handler)
		{
			// Call into the app.
			(*s_fscommand_handler)(env.get_target()->get_root_interface(), url + 10, target);
		}
	}
	else
	{
#ifdef EXTERN_MOVIE
//		log_error("get url2: target=%s, url=%s\n", target, url);
		      
		sprite_instance* target_movie = env.find_target(env.top(0));
		if (target_movie != NULL)
		{
			sprite_instance* root_movie = env.get_target()->get_root_movie();
			attach_extern_movie(url, target_movie, root_movie);
		}
		else
		{
			log_error("get url2: target %s not found\n", target);
		}
#endif // EXTERN_MOVIE
	}
	env.drop(2);
}

void
SWFHandlers::ActionBranchIfTrue(ActionExec& thread)
{
//	GNASH_REPORT_FUNCTION;

	as_environment& env = thread.env;
	const action_buffer& code = thread.code;

	// Alias these
	size_t& pc = thread.pc;
	size_t& next_pc = thread.next_pc;
	size_t& stop_pc = thread.stop_pc;

	int16_t offset = code.read_int16(pc+3);

	bool test = env.top(0).to_bool();
	env.drop(1);
	if (test)
	{
		next_pc += offset;
		      
		if (next_pc > stop_pc)
		{
			log_error("branch to offset %d -- "
				" this section only runs to %d. "
				" Malformed SWF !.\n",
				next_pc,
				stop_pc);
		}
	}
}

void
SWFHandlers::ActionCallFrame(ActionExec& thread)
{
//	GNASH_REPORT_FUNCTION;
	as_environment& env = thread.env;

	// Note: no extra data in this instruction!
	assert(env.get_target());
	env.get_target()->call_frame_actions(env.top(0));
	env.drop(1);
}

void
SWFHandlers::ActionGotoExpression(ActionExec& thread)
{
//	GNASH_REPORT_FUNCTION;

	as_environment& env = thread.env;
	const action_buffer& code = thread.code;
	size_t pc = thread.pc;

    //dbglogfile << __PRETTY_FUNCTION__ << ": unimplemented!" << endl;

	// From Alexi's SWF ref:
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
	movie::play_state state = play_flag ? movie::PLAY : movie::STOP;
		  
	sprite_instance* target = env.get_target();
	bool success = false;
		  
	if (env.top(0).get_type() == as_value::UNDEFINED)
	{
		// No-op.
	}
	else if (env.top(0).get_type() == as_value::STRING)
	{
		// @@ TODO: parse possible sprite path...
		//
		// Also, if the frame spec is actually a number (not a label),
		// then we need to do the conversion...
		      
		const char* frame_label = env.top(0).to_string();
		if (target->goto_labeled_frame(frame_label))
		{
			success = true;
		}
		else
		{
			// Couldn't find the label. Try converting to a number.
			double num;
			if (string_to_number(&num, env.top(0).to_string()))
			{
				int frame_number = int(num);
				target->goto_frame(frame_number);
				success = true;
			}
			// else no-op.
		      }
	}
	else if (env.top(0).get_type() == as_value::OBJECT)
	{
		// This is a no-op; see test_goto_frame.swf
	}
	else if (env.top(0).get_type() == as_value::NUMBER)
	{
		// Frame numbers appear to be 0-based!  @@ Verify.
		int frame_number = int(env.top(0).to_number());
		target->goto_frame(frame_number);
		success = true;
	}
		  
	if (success)
	{
		target->set_play_state(state);
	}
		  
	env.drop(1);  
}

void
SWFHandlers::ActionDeleteVar(ActionExec& thread)
{
//	GNASH_REPORT_FUNCTION;
	as_environment& env = thread.env;

	as_value var = env.pop();
	as_value object = env.top(0);
	if (object.get_type() == as_value::OBJECT)
	{
		as_object* obj = (as_object*) object.to_object();
		if (obj)
		{
			// set to NaN and eventually release memory
			obj->set_member(var.to_tu_string(), as_value());

			// TODO: remove a member  from object if it there is

			env.top(0).set_bool(true);
			return;
		}
	}

	env.top(0).set_bool(false);
}

void
SWFHandlers::ActionDelete(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    as_value var = env.top(0);
    
    as_value oldval = env.get_variable_raw(var.to_tu_string()); 
    
    if (!oldval.get_type() == as_value::UNDEFINED) {
        // set variable to 'undefined'
        // that hopefully --ref_count and eventually
        // release memory. 
        env.set_variable_raw(var.to_tu_string(), as_value());
        env.top(0).set_bool(true);
    } else {
        env.top(0).set_bool(false);
    }
}

void
SWFHandlers::ActionVarEquals(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    as_value value = env.pop();
    as_value varname = env.pop();
    env.set_local(varname.to_tu_string(), value);
}

void
SWFHandlers::ActionCallFunction(ActionExec& thread)
{
	//GNASH_REPORT_FUNCTION;
	as_environment& env = thread.env;

	//cerr << "At ActionCallFunction enter:"<<endl;
	//env.dump_stack();

	as_value function;
	if (env.top(0).get_type() == as_value::STRING)
	{
		// Function is a string; lookup the function.
		const tu_string &function_name = env.top(0).to_tu_string();
		function = env.get_variable(function_name);
		
		if (function.get_type() != as_value::AS_FUNCTION &&
		    function.get_type() != as_value::C_FUNCTION)
		{
		    log_error("error in call_function: '%s' is not a function\n",
			      function_name.c_str());
		}
	}
	else
	{
		// Hopefully the actual
		// function object is here.
		// QUESTION: would this be
		// an ActionScript-defined
		// function ?
		function = env.top(0);
	}
	int	nargs = (int)env.top(1).to_number();

	//log_msg("Function's nargs: %d", nargs);
    
	as_value result = call_method(function, &env, env.get_target(),
				  nargs, env.get_top_index() - 2);

	//log_msg("Function's result: %s", result.to_string());
    
	env.drop(nargs + 1);
	env.top(0) = result;

	//cerr << "After ActionCallFunction:"<<endl;
	//env.dump_stack();
}

void
SWFHandlers::ActionReturn(ActionExec& thread)
{
	//GNASH_REPORT_FUNCTION;
	as_environment& env = thread.env;
	as_value* retval = thread.retval;

	//log_msg("Before top/drop (retval=%p)", (void*)retval);
	//env.dump_stack();

	// Put top of stack in the provided return slot, if
	// it's not NULL.
	if (retval) {
		*retval = env.top(0);
	}
	env.drop(1);

	//log_msg("After top/drop");
	//env.dump_stack();
    
	// Skip the rest of this buffer (return from this action_buffer).
	thread.next_pc = thread.stop_pc;

}

void
SWFHandlers::ActionModulo(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    as_value	result;
    double	y = env.pop().to_number();
    double	x = env.pop().to_number();
//  Don't need to check for y being 0 here - if it's zero, fmod returns NaN
//  which is what flash would do too
    result = fmod(x, y);
//  env.top(1).set_double(fmod(env.top(1).to_bool() && env.top(0).to_bool());
//  env.drop(1);
//  log_error("modulo x=%f, y=%f, z=%f\n",x,y,result.to_number());
    env.push(result);
}

void
SWFHandlers::ActionNew(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
//    doActionNew(env, with_stack);

    as_value	classname = env.pop();
    log_action("---new object: %s\n",
               classname.to_tu_string().c_str());
    int	nargs = (int) env.pop().to_number();

    as_value constructor = env.get_variable(classname.to_tu_string());
    as_value new_obj;
    if (constructor.get_type() == as_value::C_FUNCTION)	{
        log_action("Constructor is a C_FUNCTION\n");
        // C function is responsible for creating the new object and setting members.
        (constructor.to_c_function())(fn_call(&new_obj, NULL, &env, nargs, env.get_top_index()));
    } else if (function_as_object* ctor_as_func = constructor.to_as_function())	{
        // This function is being used as a constructor; make sure
        // it has a prototype object.
        log_action("Constructor is an AS_FUNCTION\n");
        
        // a built-in class takes care of assigning a prototype
        if ( ctor_as_func->isBuiltin() ) {
            log_action("it's a built-in class");
            (*ctor_as_func)(fn_call(&new_obj, NULL, &env, nargs, env.get_top_index()));
        } else {
            // Set up the prototype.
            as_value	proto;
            bool func_has_prototype = ctor_as_func->get_member("prototype", &proto);
            assert(func_has_prototype);
            
            log_action("constructor prototype is %s\n", proto.to_string());
            
            // Create an empty object, with a ref to the constructor's prototype.
            smart_ptr<as_object>	new_obj_ptr(new as_object(proto.to_object()));
            
            new_obj.set_as_object(new_obj_ptr.get_ptr());
            
            // Call the actual constructor function; new_obj is its 'this'.
            // We don't need the function result.
            call_method(constructor, &env, new_obj_ptr.get_ptr(), nargs, env.get_top_index());
        }
    } else {
        if (classname != "String") {
            log_error("can't create object with unknown class '%s'\n",
                      classname.to_tu_string().c_str());
        } else {
            log_msg("Created special String class\n");
        }
    }
    
    env.drop(nargs);
    env.push(new_obj);
#if 0
    log_msg("new object at %p\n", new_obj.to_object());
#endif
    
}

void
SWFHandlers::ActionVar(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    const tu_string &varname = env.top(0).to_tu_string();
    env.declare_local(varname);
    env.drop(1);
}

void
SWFHandlers::ActionInitArray(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    int	array_size = (int) env.pop().to_number();
    
    //log_msg("xxx init array: size = %d, top of stack = %d\n", array_size, env.get_top_index());//xxxxx
    
    // Call the array constructor, to create an empty array.
    as_value	result;
    array_new(fn_call(&result, NULL, &env, 0, env.get_top_index()));
    
    as_object*	ao = result.to_object();
    assert(ao);
    
    // Fill the elements with the initial values from the stack.
    as_value	index_number;
    for (int i = 0; i < array_size; i++) {
        // @@ TODO a set_member that takes an int or as_value?
        index_number.set_int(i);
        ao->set_member(index_number.to_string(), env.pop());
    }
    
    env.push(result);
    
    //log_msg("xxx init array end: top of stack = %d, trace(top(0)) =", env.get_top_index());//xxxxxxx
    
    //as_global_trace(fn_call(NULL, NULL, env, 1, env.get_top_index()));	//xxxx
    
}

void
SWFHandlers::ActionInitObject(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    // 
    //    SWFACTION_PUSH
    //     [000]   Constant: 1 "obj"
    //     [001]   Constant: 0 "member" <-- we handle up to here
    //     [002]   Integer: 1
    //     [003]   Integer: 1
    //    SWFACTION_INITOBJECT
    
    int nmembers = (int) env.pop().to_number();
    
    smart_ptr<as_object> new_obj_ptr(new as_object); 
    
    // Set provided members
    for (int i=0; i<nmembers; ++i) {
        as_value member_value = env.pop();
        tu_stringi member_name = env.pop().to_tu_stringi();
        new_obj_ptr->set_member(member_name, member_value);
    }
    
    // @@ TODO
    //log_error("checkme opcode: %02X\n", action_id);
    
    as_value new_obj;
    new_obj.set_as_object(new_obj_ptr.get_ptr());
    
    //env.drop(nmembers*2);
    env.push(new_obj); 
    
}

void
SWFHandlers::ActionTypeOf(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    switch(env.top(0).get_type()) {
      case as_value::UNDEFINED:
          env.top(0).set_string("undefined");
          break;
      case as_value::STRING:
          env.top(0).set_string("string");
          break;
      case as_value::NUMBER:
          env.top(0).set_string("number");
          break;
      case as_value::BOOLEAN:
          env.top(0).set_string("boolean");
          break;
      case as_value::OBJECT:
          env.top(0).set_string("object");
          break;
      case as_value::NULLTYPE:
          env.top(0).set_string("null");
          break;
      case as_value::AS_FUNCTION:
          env.top(0).set_string("function");
          break;
      default:
          log_error("typeof unknown type: %02X\n", env.top(0).get_type());
          break;
    }
}

void
SWFHandlers::ActionTargetPath(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    dbglogfile << __PRETTY_FUNCTION__ << ": unimplemented!" << endl;
}

void
SWFHandlers::ActionEnumerate(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    as_value var_name = env.pop();
    const tu_string& var_string = var_name.to_tu_string();
    
    as_value variable = env.get_variable(var_string);
    
	// @@ shouldn't we return *only* after pushing the nullvalue
	// below ?
    if (variable.to_object() == NULL) {
    	dbglogfile << __PRETTY_FUNCTION__ << ": CHECKME: are we required to always push at least a NULL value ?" << endl;
        return;
    }
    const as_object* object = (as_object*) (variable.to_object());
    
    // The end of the enumeration
    as_value nullvalue;
    nullvalue.set_null();
    env.push(nullvalue);
    log_action("---enumerate - push: NULL\n");
    
    stringi_hash<as_member>::const_iterator it = object->m_members.begin();
    while (it != object->m_members.end()) {
        const as_member member = (it->second);
        
        if (! member.get_member_flags().get_dont_enum()) {
            env.push(as_value(it->first.c_str()));
            
            log_action("---enumerate - push: %s\n",
                                      it->first.c_str());
        }
        
        ++it;
    }
    
    const as_object * prototype = (as_object *) object->m_prototype;
    if (prototype != NULL) {
        stringi_hash<as_member>::const_iterator it = prototype->m_members.begin();
        while (it != prototype->m_members.end()) {
            const as_member member = (it->second);
            
            if (! member.get_member_flags().get_dont_enum()) {
                env.push(as_value(it->first.c_str()));
                
                log_action("---enumerate - push: %s\n",
                           it->first.c_str());
            }
            
            ++it;
        };
    }
}

void
SWFHandlers::ActionNewAdd(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    int version = env.get_target()->get_movie_definition()->get_version();    
    if (env.top(0).get_type() == as_value::STRING
        || env.top(1).get_type() == as_value::STRING) {
        env.top(1).convert_to_string_versioned(version);
        env.top(1).string_concat(env.top(0).to_tu_string_versioned(version));
    } else {
        env.top(1) += env.top(0);
    }
    env.drop(1);
}

void
SWFHandlers::ActionNewLessThan(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    if (env.top(1).get_type() == as_value::STRING) {
        env.top(1).set_bool(env.top(1).to_tu_string() < env.top(0).to_tu_string());
    } else {
        env.top(1).set_bool(env.top(1) < env.top(0));
    }
    env.drop(1);
}

void
SWFHandlers::ActionNewEquals(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    env.top(1).set_bool(env.top(1) == env.top(0));
    env.drop(1);
}

void
SWFHandlers::ActionToNumber(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    env.top(0).convert_to_number();
}

void
SWFHandlers::ActionToString(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    int version = env.get_target()->get_movie_definition()->get_version();    
    env.top(0).convert_to_string_versioned(version);
}

void
SWFHandlers::ActionDup(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    env.push(env.top(0));
}

void
SWFHandlers::ActionSwap(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    as_value	temp = env.top(1);
    env.top(1) = env.top(0);
    env.top(0) = temp;
}

void
SWFHandlers::ActionGetMember(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;

    // Some corner case behaviors depend on the SWF file version.
    int version = env.get_target()->get_movie_definition()->get_version();
    
    as_value member_name = env.top(0);
    as_value target = env.top(1);
    
    as_object* obj = target.to_object();
    if (!obj) {
//         IF_VERBOSE_DEBUG(log_msg("getMember called against "
//                                  "a value that does not cast "
//                                  "to an as_object: %s\n", target.to_string()));
        env.top(1).set_undefined();
        env.drop(1);
        return;
    }
    
    log_action(" ActionGetMember: target: %s (object %p)\n",
               target.to_string(), (void*)obj);
    
    // Special case: String has a member "length"
    // @@ FIXME: we shouldn't have all this "special" cases --strk;
    if (target.get_type() == as_value::STRING && member_name.to_tu_stringi() == "length") {
        int len = target.to_tu_string_versioned(version).utf8_length();
        env.top(1).set_int(len); 
    } else {
        if (!obj->get_member(member_name.to_tu_string(), &(env.top(1)))) {
            env.top(1).set_undefined();
        }
        
        log_action("-- get_member %s=%s\n",
                   member_name.to_tu_string().c_str(),
                   env.top(1).to_tu_string().c_str());
    }
    env.drop(1);
    
}

void
SWFHandlers::ActionSetMember(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    as_object*	obj = env.top(2).to_object();
    if (obj) {
        obj->set_member(env.top(1).to_tu_string(), env.top(0));
        log_action("-- set_member %s.%s=%s\n",
                   env.top(2).to_tu_string().c_str(),
                   env.top(1).to_tu_string().c_str(),
                   env.top(0).to_tu_string().c_str());
    } else {
        // Invalid object, can't set.
        log_action("-- set_member %s.%s=%s on invalid object!\n",
                   env.top(2).to_tu_string().c_str(),
                   env.top(1).to_tu_string().c_str(),
                   env.top(0).to_tu_string().c_str());
    }
    env.drop(3);
}

void
SWFHandlers::ActionIncrement(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    env.top(0) += 1;
}

void
SWFHandlers::ActionDecrement(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    env.top(0) -= 1;
}

void
SWFHandlers::ActionCallMethod(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;

    as_value result;

    // Some corner case behaviors depend on the SWF file version.
    //int version = env.get_target()->get_movie_definition()->get_version();

    // Get name of the method
    const tu_string &method_name = env.top(0).to_tu_string();
    log_action(" method name: %s\n", method_name.c_str());

    // Get an object
    as_value& obj_value = env.top(1);
    as_object *obj = obj_value.to_object();
    log_action(" method object: %p\n", (void*)obj);

    // Get number of arguments
    int	nargs = (int) env.top(2).to_number();
    log_action(" method nargs: %d\n", nargs);

    if (!obj) {
        log_error("call_method invoked in something that "
                  "doesn't cast to an as_object: %s\n",
                  obj_value.to_string());
    } else {
        as_value method;
        if (obj->get_member(method_name, &method)) {
            if (method.get_type() != as_value::AS_FUNCTION &&
                method.get_type() != as_value::C_FUNCTION) {
                log_error("call_method: '%s' is not a method\n",
                          method_name.c_str());
            } else {
                result = call_method( method, &env, obj, nargs,
                                      env.get_top_index() - 3);
            }
        } else {
            log_error("call_method can't find method %s "
                      "for object %s (%p)\n", method_name.c_str(), 
                      typeid(*obj).name(), (void*)obj);
        }
    }
    
    env.drop(nargs + 2);
    env.top(0) = result;

    // This is to check stack status after call method
    //log_msg("at doActionCallMethod() end, stack: \n"); env.dump_stack();
    
}

void
SWFHandlers::ActionNewMethod(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    dbglogfile << __PRETTY_FUNCTION__ << ": unimplemented!" << endl;
}

void
SWFHandlers::ActionInstanceOf(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;

    // Get the "super" function
    function_as_object* super = env.top(0).to_as_function();

    // Get the "instance" 
    as_object* instance = env.top(1).to_object();

    // Invalid args!
    if (!super || ! instance) {
        //IF_VERBOSE_ACTION(
        log_action("-- %s instance_of %s (invalid args?)\n",
                env.top(1).to_string(),
                env.top(0).to_string());
        //);

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
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    dbglogfile << __PRETTY_FUNCTION__ << ": unimplemented!" << endl;
}

void
SWFHandlers::ActionBitwiseAnd(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    env.top(1) &= env.top(0);
    env.drop(1);
}

void
SWFHandlers::ActionBitwiseOr(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    env.top(1) |= env.top(0);
    env.drop(1);
}

void
SWFHandlers::ActionBitwiseXor(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    env.top(1) ^= env.top(0);
    env.drop(1);
}

void
SWFHandlers::ActionShiftLeft(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    env.top(1).asr(env.top(0));
    env.drop(1);
}

void
SWFHandlers::ActionShiftRight(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    env.top(1).lsr(env.top(0));
    env.drop(1);
}

void
SWFHandlers::ActionShiftRight2(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    env.top(1).lsr(env.top(0));
    env.drop(1);
}

void
SWFHandlers::ActionStrictEq(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    if (env.top(1).get_type() != env.top(0).get_type()) {
        // Types don't match.
        env.top(1).set_bool(false);
        env.drop(1);
    } else {
        env.top(1).set_bool(env.top(1) == env.top(0));
        env.drop(1);
    }
}

void
SWFHandlers::ActionGreater(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    if (env.top(1).get_type() == as_value::STRING) {
        env.top(1).set_bool(env.top(1).to_tu_string() > env.top(0).to_tu_string());
    } else {
        env.top(1).set_bool(env.top(1).to_number() > env.top(0).to_number());
    }
    env.drop(1);
}

void
SWFHandlers::ActionStringGreater(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    as_environment& env = thread.env;
    env.top(1).set_bool(env.top(1).to_tu_string() > env.top(0).to_tu_string());
    env.drop(1);
}

void
SWFHandlers::ActionExtends(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    dbglogfile << __PRETTY_FUNCTION__ << ": unimplemented!" << endl;
}

void
SWFHandlers::ActionConstantPool(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
	thread.code.process_decl_dict(thread.pc, thread.next_pc);
}

void
SWFHandlers::ActionDefineFunction2(ActionExec& thread)
{
//	GNASH_REPORT_FUNCTION;

	as_environment& env = thread.env;
	const action_buffer& code = thread.code;

	// Code starts at thread.next_pc as the DefineFunction tag
	// contains name and args, while next tag is first tag
	// of the function body.
	function_as_object* func = new function_as_object(
		&code, &env, thread.next_pc, thread.with_stack);

	func->set_is_function2();

	size_t i = thread.pc + 3; // skip tag id and length

	// Extract name.
	// @@ security: watch out for possible missing terminator here!
	tu_string name = code.read_string(i);
	i += name.length() + 1; // add NULL-termination

	//cerr << " name:" << name << endl;

	// Get number of arguments.
	int nargs = code.read_int16(i);
	i += 2;

	//cerr << " nargs:" << nargs << endl;

	// Get the count of local registers used by this function.
	uint8 register_count = code[i];
	i += 1;

	//cerr << " nregisters:" << nargs << endl;

	func->set_local_register_count(register_count);

	// Flags, for controlling register assignment of implicit args.
	uint16	flags = code.read_int16(i);
	i += 2;

	func->set_function2_flags(flags);

	// Get the register assignments and names of the arguments.
	for (int n = 0; n < nargs; n++)
	{
		uint8 arg_register = code[i];
		++i;
	
		// @@ security: watch out for possible missing terminator here!
		const char* arg = code.read_string(i);

		func->add_arg(arg_register, arg);
		i += strlen(arg)+1;
	}

	// Get the length of the actual function code.
	int16_t code_size = code.read_int16(i);
	assert( code_size >= 0 );
	i += 2;
	func->set_length(code_size);

	// Skip the function body (don't interpret it now).
	thread.next_pc += code_size; 

	// If we have a name, then save the function in this
	// environment under that name.
	as_value function_value(func);
	if (name.length() > 0)
	{
		// @@ NOTE: should this be m_target->set_variable()???
		env.set_member(name, function_value);
	}
    
	// Also leave it on the stack.
	env.push_val(function_value);
}

void
SWFHandlers::ActionTry(ActionExec& thread)
{
//    GNASH_REPORT_FUNCTION;
    dbglogfile << __PRETTY_FUNCTION__ << ": unimplemented!" << endl;
}

void
SWFHandlers::ActionWith(ActionExec& thread)
{
//	GNASH_REPORT_FUNCTION;
    dbglogfile << __PRETTY_FUNCTION__ << ": unimplemented!" << endl;

	as_environment& env = thread.env;
	const action_buffer& code = thread.code;
	std::vector<with_stack_entry>& with_stack = thread.with_stack;

	size_t pc = thread.pc;
	size_t next_pc = thread.next_pc;

	log_action("-------------- with block start: stack size is %zd\n",
		with_stack.size());

	if (with_stack.size() < 8)
	{
		int block_length = code.read_int16(pc+3);
		int block_end = next_pc + block_length;
		as_object* with_obj = env.top(0).to_object();
		with_stack.push_back(
			with_stack_entry(with_obj, block_end)
		);
	}
	env.drop(1);
}

void
SWFHandlers::ActionDefineFunction(ActionExec& thread)
{
//	GNASH_REPORT_FUNCTION;

	as_environment& env = thread.env;
	const action_buffer& code = thread.code;

	int16_t length = code.read_int16(thread.pc+1);
	assert( length >= 0 );

	//cerr << " length:" << length << endl;

	// Create a new function_as_object
	// Code starts at thread.next_pc as the DefineFunction tag
	// contains name and args, while next tag is first tag
	// of the function body.
	function_as_object* func = new function_as_object(
		&code, &env, thread.next_pc, thread.with_stack);

	size_t i = thread.pc + 3;

	// Extract name.
	// @@ security: watch out for possible missing terminator here!
	tu_string name = code.read_string(i);
	i += name.length() + 1;

	//cerr << " name:" << name << endl;

	// Get number of arguments.
	int nargs = code.read_int16(i);
	i += 2;

	//cerr << " nargs:" << nargs << endl;

	// Get the names of the arguments.
	for (int n = 0; n < nargs; n++)
	{
		const char* arg = code.read_string(i);
		//cerr << " arg" << n << " : " << arg << endl;

		// @@ security: watch out for possible missing terminator here!
		func->add_arg(0, arg);
		// wouldn't it be simpler to use strlen(arg)+1 ?
		i += func->m_args.back().m_name.length() + 1;
	}

	// Get the length of the actual function code.
	int16_t code_size = code.read_int16(i);

	//cerr << " code size:" << code_size << endl;

	func->set_length(code_size);

    
	// Skip the function body (don't interpret it now).
	// next_pc is assumed to point to first action of
	// the function body (one-past the current tag, whic
	// is DefineFunction). We add code_size to it.
	thread.next_pc += code_size;

	// If we have a name, then save the function in this
	// environment under that name.
	as_value	function_value(func);
	if (name.length() > 0)
	{
		// @@ NOTE: should this be m_target->set_variable()???
		env.set_member(name, function_value);
	}
    
	// Also leave it on the stack.
	env.push_val(function_value);

	//cerr << "After ActionDefineFunction:"<<endl;
	//env.dump_stack();
}

void
SWFHandlers::ActionSetRegister(ActionExec& thread)
{
//	GNASH_REPORT_FUNCTION;

	as_environment& env = thread.env;
	const action_buffer& code = thread.code;

	int reg = code[thread.pc + 3];

	// Save top of stack in specified register.
	if ( thread.isFunction2() )
	{
		*(env.local_register_ptr(reg)) = env.top(0);
		      
		log_action("-------------- local register[%d] = '%s'\n",
			reg, env.top(0).to_string());

	}
	else if (reg >= 0 && reg < 4)
	{
		env.m_global_register[reg] = env.top(0);
		      
		log_action("-------------- global register[%d] = '%s'\n",
			reg, env.top(0).to_string() );

	}
	else
	{
		log_error("store_register[%d] -- register out of bounds!",
			reg);
	}
		  
}

} // namespace gnash::SWF

} // namespace gnash
