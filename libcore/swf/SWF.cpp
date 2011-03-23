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

#include "SWF.h"
#include <ostream>

namespace gnash {
namespace SWF { 

std::ostream&
operator<<(std::ostream& o, ActionType a)
{
    switch (a) {
        case ACTION_END: return o << "End";
        case ACTION_NEXTFRAME: return o << "NextFrame";
        case ACTION_PREVFRAME: return o << "PreviousFrame";
        case ACTION_PLAY: return o << "Play";
        case ACTION_STOP: return o << "Stop";
        case ACTION_TOGGLEQUALITY: return o << "ToggleQuality";
        case ACTION_STOPSOUNDS: return o << "StopSounds";
        case ACTION_GOTOFRAME: return o << "GotoFrame";
        case ACTION_GETURL: return o << "GetUrl";
        case ACTION_WAITFORFRAME: return o << "WaitForFrame";
        case ACTION_SETTARGET: return o << "SetTarget";
        case ACTION_GOTOLABEL: return o << "GotoLabel";
        case ACTION_ADD: return o << "Add";
        case ACTION_SUBTRACT: return o << "Subtract";
        case ACTION_MULTIPLY: return o << "Multiply";
        case ACTION_DIVIDE: return o << "Divide";
        case ACTION_EQUAL: return o << "Equal";
        case ACTION_LESSTHAN: return o << "LessThan";
        case ACTION_LOGICALAND: return o << "LogicalAnd";
        case ACTION_LOGICALOR: return o << "LogicalOr";
        case ACTION_LOGICALNOT: return o << "LogicalNot";
        case ACTION_STRINGEQ: return o << "StringEq";
        case ACTION_STRINGLENGTH: return o << "ActionStringLength";
        case ACTION_SUBSTRING: return o << "ActionSubString";
        case ACTION_POP: return o << "ActionPop";
        case ACTION_INT: return o << "ActionInt";
        case ACTION_GETVARIABLE: return o << "ActionGetVariable";
        case ACTION_SETVARIABLE: return o << "ActionSetVariable";
        case ACTION_SETTARGETEXPRESSION:
             return o << "ActionSetTargetExpression";
        case ACTION_STRINGCONCAT: return o << "ActionStringConcat";
        case ACTION_GETPROPERTY: return o << "ActionGetProperty";
        case ACTION_SETPROPERTY: return o << "ActionSetProperty";
        case ACTION_DUPLICATECLIP: return o << "ActionDuplicateClip";
        case ACTION_REMOVECLIP: return o << "ActionRemoveClip";
        case ACTION_TRACE: return o << "ActionTrace";
        case ACTION_STARTDRAGMOVIE: return o << "ActionStartDragMovie";
        case ACTION_STOPDRAGMOVIE: return o << "ActionStopDragMovie";
        case ACTION_STRINGCOMPARE: return o << "ActionStringCompare";
        case ACTION_THROW: return o << "ActionThrow";
        case ACTION_CASTOP: return o << "ActionCastOp";
        case ACTION_IMPLEMENTSOP: return o << "ActionImplementsOp";
        case ACTION_FSCOMMAND2: return o << "ActionFscommand2";
        case ACTION_RANDOM: return o << "ActionRandom";
        case ACTION_MBLENGTH: return o << "ActionMbLength";
        case ACTION_ORD: return o << "ActionOrd";
        case ACTION_CHR: return o << "ActionChr";
        case ACTION_GETTIMER: return o << "ActionGetTimer";
        case ACTION_MBSUBSTRING: return o << "ActionMbSubString";
        case ACTION_MBORD: return o << "ActionMbOrd";
        case ACTION_MBCHR: return o << "ActionMbChr";
        case ACTION_STRICTMODE: return o << "ActionStrictMode";
        case ACTION_WAITFORFRAMEEXPRESSION:
            return o << "ActionWaitForFrameExpression";
        case ACTION_PUSHDATA: return o << "ActionPushData";
        case ACTION_BRANCHALWAYS: return o << "ActionBranchAlways";
        case ACTION_GETURL2: return o << "ActionGetUrl2";
        case ACTION_BRANCHIFTRUE: return o << "ActionBranchIfTrue";
        case ACTION_CALLFRAME: return o << "ActionCallFrame";
        case ACTION_GOTOEXPRESSION: return o << "ActionGotoExpression";
        case ACTION_DELETE: return o << "ActionDelete";
        case ACTION_DELETE2: return o << "ActionDelete2";
        case ACTION_VAREQUALS: return o << "ActionVarEquals";
        case ACTION_CALLFUNCTION: return o << "ActionCallFunction";
        case ACTION_RETURN: return o << "ActionReturn";
        case ACTION_MODULO: return o << "ActionModulo";
        case ACTION_NEW: return o << "ActionNew";
        case ACTION_VAR: return o << "ActionVar";
        case ACTION_INITARRAY: return o << "ActionInitArray";
        case ACTION_INITOBJECT: return o << "ActionInitObject";
        case ACTION_TYPEOF: return o << "ActionTypeOf";
        case ACTION_TARGETPATH: return o << "ActionTargetPath";
        case ACTION_ENUMERATE: return o << "ActionEnumerate";
        case ACTION_NEWADD: return o << "ActionNewAdd";
        case ACTION_NEWLESSTHAN: return o << "ActionNewLessThan";
        case ACTION_NEWEQUALS: return o << "ActionNewEquals";
        case ACTION_TONUMBER: return o << "ActionToNumber";
        case ACTION_TOSTRING: return o << "ActionToString";
        case ACTION_DUP: return o << "ActionDup";
        case ACTION_SWAP: return o << "ActionSwap";
        case ACTION_GETMEMBER: return o << "ActionGetMember";
        case ACTION_SETMEMBER: return o << "ActionSetMember";
        case ACTION_INCREMENT: return o << "ActionIncrement";
        case ACTION_DECREMENT: return o << "ActionDecrement";
        case ACTION_CALLMETHOD: return o << "ActionCallMethod";
        case ACTION_NEWMETHOD: return o << "ActionNewMethod";
        case ACTION_INSTANCEOF: return o << "ActionInstanceOf";
        case ACTION_ENUM2: return o << "ActionEnum2";
        case ACTION_BITWISEAND: return o << "ActionBitwiseAnd";
        case ACTION_BITWISEOR: return o << "ActionBitwiseOr";
        case ACTION_BITWISEXOR: return o << "ActionBitwiseXor";
        case ACTION_SHIFTLEFT: return o << "ActionShiftLeft";
        case ACTION_SHIFTRIGHT: return o << "ActionShiftRight";
        case ACTION_SHIFTRIGHT2: return o << "ActionShiftRight2";
        case ACTION_STRICTEQ: return o << "ActionStrictEq";
        case ACTION_GREATER: return o << "ActionGreater";
        case ACTION_STRINGGREATER: return o << "ActionStringGreater";
        case ACTION_EXTENDS: return o << "ActionExtends";
        case ACTION_CONSTANTPOOL: return o << "ActionConstantPool";
        case ACTION_DEFINEFUNCTION2: return o << "ActionDefineFunction2";
        case ACTION_TRY: return o << "ActionTry";
        case ACTION_WITH: return o << "ActionWith";
        case ACTION_DEFINEFUNCTION: return o << "ActionDefineFunction";
        case ACTION_SETREGISTER: return o << "ActionSetRegister";
        default: return o << "Unknown ActionType";
    }
    return o;
}

std::ostream&
operator<<(std::ostream& os, const abc_action_type& opcode)
{
    switch (opcode)
    {
        case ABC_ACTION_END:
            os << "ABC_ACTION_END";
            break;
        case ABC_ACTION_BKPT:
            os << "ABC_ACTION_BKPT";
            break;
        case ABC_ACTION_NOP:
            os << "ABC_ACTION_NOP";
            break;
        case ABC_ACTION_THROW:
            os << "ABC_ACTION_THROW";
            break;
        case ABC_ACTION_GETSUPER:
            os << "ABC_ACTION_GETSUPER";
            break;
        case ABC_ACTION_SETSUPER:
            os << "ABC_ACTION_SETSUPER";
            break;
        case ABC_ACTION_DXNS:
            os << "ABC_ACTION_DXNS";
            break;
        case ABC_ACTION_DXNSLATE:
            os << "ABC_ACTION_DXNSLATE";
            break;
        case ABC_ACTION_KILL:
            os << "ABC_ACTION_KILL";
            break;
        case ABC_ACTION_LABEL:
            os << "ABC_ACTION_LABEL";
            break;
        case ABC_ACTION_0x0A:
            os << "ABC_ACTION_0x0A";
            break;
        case ABC_ACTION_0X0B:
            os << "ABC_ACTION_0X0B";
            break;
        case ABC_ACTION_IFNLT:
            os << "ABC_ACTION_IFNLT";
            break;
        case ABC_ACTION_IFNLE:
            os << "ABC_ACTION_IFNLE";
            break;
        case ABC_ACTION_IFNGT:
            os << "ABC_ACTION_IFNGT";
            break;
        case ABC_ACTION_IFNGE:
            os << "ABC_ACTION_IFNGE";
            break;
        case ABC_ACTION_JUMP:
            os << "ABC_ACTION_JUMP";
            break;
        case ABC_ACTION_IFTRUE:
            os << "ABC_ACTION_IFTRUE";
            break;
        case ABC_ACTION_IFFALSE:
            os << "ABC_ACTION_IFFALSE";
            break;
        case ABC_ACTION_IFEQ:
            os << "ABC_ACTION_IFEQ";
            break;
        case ABC_ACTION_IFNE:
            os << "ABC_ACTION_IFNE";
            break;
        case ABC_ACTION_IFLT:
            os << "ABC_ACTION_IFLT";
            break;
        case ABC_ACTION_IFLE:
            os << "ABC_ACTION_IFLE";
            break;
        case ABC_ACTION_IFGT:
            os << "ABC_ACTION_IFGT";
            break;
        case ABC_ACTION_IFGE:
            os << "ABC_ACTION_IFGE";
            break;
        case ABC_ACTION_IFSTRICTEQ:
            os << "ABC_ACTION_IFSTRICTEQ";
            break;
        case ABC_ACTION_IFSTRICTNE:
            os << "ABC_ACTION_IFSTRICTNE";
            break;
        case ABC_ACTION_LOOKUPSWITCH:
            os << "ABC_ACTION_LOOKUPSWITCH";
            break;
        case ABC_ACTION_PUSHWITH:
            os << "ABC_ACTION_PUSHWITH";
            break;
        case ABC_ACTION_POPSCOPE:
            os << "ABC_ACTION_POPSCOPE";
            break;
        case ABC_ACTION_NEXTNAME:
            os << "ABC_ACTION_NEXTNAME";
            break;
        case ABC_ACTION_HASNEXT:
            os << "ABC_ACTION_HASNEXT";
            break;
        case ABC_ACTION_PUSHNULL:
            os << "ABC_ACTION_PUSHNULL";
            break;
        case ABC_ACTION_PUSHUNDEFINED:
            os << "ABC_ACTION_PUSHUNDEFINED";
            break;
        case ABC_ACTION_0x22:
            os << "ABC_ACTION_0x22";
            break;
        case ABC_ACTION_NEXTVALUE:
            os << "ABC_ACTION_NEXTVALUE";
            break;
        case ABC_ACTION_PUSHBYTE:
            os << "ABC_ACTION_PUSHBYTE";
            break;
        case ABC_ACTION_PUSHSHORT:
            os << "ABC_ACTION_PUSHSHORT";
            break;
        case ABC_ACTION_PUSHTRUE:
            os << "ABC_ACTION_PUSHTRUE";
            break;
        case ABC_ACTION_PUSHFALSE:
            os << "ABC_ACTION_PUSHFALSE";
            break;
        case ABC_ACTION_PUSHNAN:
            os << "ABC_ACTION_PUSHNAN";
            break;
        case ABC_ACTION_POP:
            os << "ABC_ACTION_POP";
            break;
        case ABC_ACTION_DUP:
            os << "ABC_ACTION_DUP";
            break;
        case ABC_ACTION_SWAP:
            os << "ABC_ACTION_SWAP";
            break;
        case ABC_ACTION_PUSHSTRING:
            os << "ABC_ACTION_PUSHSTRING";
            break;
        case ABC_ACTION_PUSHINT:
            os << "ABC_ACTION_PUSHINT";
            break;
        case ABC_ACTION_PUSHUINT:
            os << "ABC_ACTION_PUSHUINT";
            break;
        case ABC_ACTION_PUSHDOUBLE:
            os << "ABC_ACTION_PUSHDOUBLE";
            break;
        case ABC_ACTION_PUSHSCOPE:
            os << "ABC_ACTION_PUSHSCOPE";
            break;
        case ABC_ACTION_PUSHNAMESPACE:
            os << "ABC_ACTION_PUSHNAMESPACE";
            break;
        case ABC_ACTION_HASNEXT2:
            os << "ABC_ACTION_HASNEXT2";
            break;
        case ABC_ACTION_0x33:
            os << "ABC_ACTION_0x33";
            break;
        case ABC_ACTION_0x34:
            os << "ABC_ACTION_0x34";
            break;
        case ABC_ACTION_0x35:
            os << "ABC_ACTION_0x35";
            break;
        case ABC_ACTION_0x36:
            os << "ABC_ACTION_0x36";
            break;
        case ABC_ACTION_0x37:
            os << "ABC_ACTION_0x37";
            break;
        case ABC_ACTION_0x38:
            os << "ABC_ACTION_0x38";
            break;
        case ABC_ACTION_0x39:
            os << "ABC_ACTION_0x39";
            break;
        case ABC_ACTION_0x3A:
            os << "ABC_ACTION_0x3A";
            break;
        case ABC_ACTION_0x3B:
            os << "ABC_ACTION_0x3B";
            break;
        case ABC_ACTION_0x3C:
            os << "ABC_ACTION_0x3C";
            break;
        case ABC_ACTION_0x3D:
            os << "ABC_ACTION_0x3D";
            break;
        case ABC_ACTION_0x3E:
            os << "ABC_ACTION_0x3E";
            break;
        case ABC_ACTION_0x3F:
            os << "ABC_ACTION_0x3F";
            break;
        case ABC_ACTION_NEWFUNCTION:
            os << "ABC_ACTION_NEWFUNCTION";
            break;
        case ABC_ACTION_CALL:
            os << "ABC_ACTION_CALL";
            break;
        case ABC_ACTION_CONSTRUCT:
            os << "ABC_ACTION_CONSTRUCT";
            break;
        case ABC_ACTION_CALLMETHOD:
            os << "ABC_ACTION_CALLMETHOD";
            break;
        case ABC_ACTION_CALLSTATIC:
            os << "ABC_ACTION_CALLSTATIC";
            break;
        case ABC_ACTION_CALLSUPER:
            os << "ABC_ACTION_CALLSUPER";
            break;
        case ABC_ACTION_CALLPROPERTY:
            os << "ABC_ACTION_CALLPROPERTY";
            break;
        case ABC_ACTION_RETURNVOID:
            os << "ABC_ACTION_RETURNVOID";
            break;
        case ABC_ACTION_RETURNVALUE:
            os << "ABC_ACTION_RETURNVALUE";
            break;
        case ABC_ACTION_CONSTRUCTSUPER:
            os << "ABC_ACTION_CONSTRUCTSUPER";
            break;
        case ABC_ACTION_CONSTRUCTPROP:
            os << "ABC_ACTION_CONSTRUCTPROP";
            break;
        case ABC_ACTION_CALLSUPERID:
            os << "ABC_ACTION_CALLSUPERID";
            break;
        case ABC_ACTION_CALLPROPLEX:
            os << "ABC_ACTION_CALLPROPLEX";
            break;
        case ABC_ACTION_CALLINTERFACE:
            os << "ABC_ACTION_CALLINTERFACE";
            break;
        case ABC_ACTION_CALLSUPERVOID:
            os << "ABC_ACTION_CALLSUPERVOID";
            break;
        case ABC_ACTION_CALLPROPVOID:
            os << "ABC_ACTION_CALLPROPVOID";
            break;
        case ABC_ACTION_0x50:
            os << "ABC_ACTION_0x50";
            break;
        case ABC_ACTION_0x51:
            os << "ABC_ACTION_0x51";
            break;
        case ABC_ACTION_0x52:
            os << "ABC_ACTION_0x52";
            break;
        case ABC_ACTION_0x53:
            os << "ABC_ACTION_0x53";
            break;
        case ABC_ACTION_0x54:
            os << "ABC_ACTION_0x54";
            break;
        case ABC_ACTION_NEWOBJECT:
            os << "ABC_ACTION_NEWOBJECT";
            break;
        case ABC_ACTION_NEWARRAY:
            os << "ABC_ACTION_NEWARRAY";
            break;
        case ABC_ACTION_NEWACTIVATION:
            os << "ABC_ACTION_NEWACTIVATION";
            break;
        case ABC_ACTION_NEWCLASS:
            os << "ABC_ACTION_NEWCLASS";
            break;
        case ABC_ACTION_GETDESCENDANTS:
            os << "ABC_ACTION_GETDESCENDANTS";
            break;
        case ABC_ACTION_NEWCATCH:
            os << "ABC_ACTION_NEWCATCH";
            break;
        case ABC_ACTION_0x5B:
            os << "ABC_ACTION_0x5B";
            break;
        case ABC_ACTION_0x5C:
            os << "ABC_ACTION_0x5C";
            break;
        case ABC_ACTION_FINDPROPSTRICT:
            os << "ABC_ACTION_FINDPROPSTRICT";
            break;
        case ABC_ACTION_FINDPROPERTY:
            os << "ABC_ACTION_FINDPROPERTY";
            break;
        case ABC_ACTION_FINDDEF:
            os << "ABC_ACTION_FINDDEF";
            break;
        case ABC_ACTION_GETLEX:
            os << "ABC_ACTION_GETLEX";
            break;
        case ABC_ACTION_SETPROPERTY:
            os << "ABC_ACTION_SETPROPERTY";
            break;
        case ABC_ACTION_GETLOCAL:
            os << "ABC_ACTION_GETLOCAL";
            break;
        case ABC_ACTION_SETLOCAL:
            os << "ABC_ACTION_SETLOCAL";
            break;
        case ABC_ACTION_GETGLOBALSCOPE:
            os << "ABC_ACTION_GETGLOBALSCOPE";
            break;
        case ABC_ACTION_GETSCOPEOBJECT:
            os << "ABC_ACTION_GETSCOPEOBJECT";
            break;
        case ABC_ACTION_GETPROPERTY:
            os << "ABC_ACTION_GETPROPERTY";
            break;
        case ABC_ACTION_0x67:
            os << "ABC_ACTION_0x67";
            break;
        case ABC_ACTION_INITPROPERTY:
            os << "ABC_ACTION_INITPROPERTY";
            break;
        case ABC_ACTION_0x69:
            os << "ABC_ACTION_0x69";
            break;
        case ABC_ACTION_DELETEPROPERTY:
            os << "ABC_ACTION_DELETEPROPERTY";
            break;
        case ABC_ACTION_0x6B:
            os << "ABC_ACTION_0x6B";
            break;
        case ABC_ACTION_GETSLOT:
            os << "ABC_ACTION_GETSLOT";
            break;
        case ABC_ACTION_SETSLOT:
            os << "ABC_ACTION_SETSLOT";
            break;
        case ABC_ACTION_GETGLOBALSLOT:
            os << "ABC_ACTION_GETGLOBALSLOT";
            break;
        case ABC_ACTION_SETGLOBALSLOT:
            os << "ABC_ACTION_SETGLOBALSLOT";
            break;
        case ABC_ACTION_CONVERT_S:
            os << "ABC_ACTION_CONVERT_S";
            break;
        case ABC_ACTION_ESC_XELEM:
            os << "ABC_ACTION_ESC_XELEM";
            break;
        case ABC_ACTION_ESC_XATTR:
            os << "ABC_ACTION_ESC_XATTR";
            break;
        case ABC_ACTION_CONVERT_I:
            os << "ABC_ACTION_CONVERT_I";
            break;
        case ABC_ACTION_CONVERT_U:
            os << "ABC_ACTION_CONVERT_U";
            break;
        case ABC_ACTION_CONVERT_D:
            os << "ABC_ACTION_CONVERT_D";
            break;
        case ABC_ACTION_CONVERT_B:
            os << "ABC_ACTION_CONVERT_B";
            break;
        case ABC_ACTION_CONVERT_O:
            os << "ABC_ACTION_CONVERT_O";
            break;
        case ABC_ACTION_CHECKFILTER:
            os << "ABC_ACTION_CHECKFILTER";
            break;
        case ABC_ACTION_0x79:
            os << "ABC_ACTION_0x79";
            break;
        case ABC_ACTION_0x7A:
            os << "ABC_ACTION_0x7A";
            break;
        case ABC_ACTION_0x7B:
            os << "ABC_ACTION_0x7B";
            break;
        case ABC_ACTION_0x7C:
            os << "ABC_ACTION_0x7C";
            break;
        case ABC_ACTION_0x7D:
            os << "ABC_ACTION_0x7D";
            break;
        case ABC_ACTION_0x7E:
            os << "ABC_ACTION_0x7E";
            break;
        case ABC_ACTION_0x7F:
            os << "ABC_ACTION_0x7F";
            break;
        case ABC_ACTION_COERCE:
            os << "ABC_ACTION_COERCE";
            break;
        case ABC_ACTION_COERCE_B:
            os << "ABC_ACTION_COERCE_B";
            break;
        case ABC_ACTION_COERCE_A:
            os << "ABC_ACTION_COERCE_A";
            break;
        case ABC_ACTION_COERCE_I:
            os << "ABC_ACTION_COERCE_I";
            break;
        case ABC_ACTION_COERCE_D:
            os << "ABC_ACTION_COERCE_D";
            break;
        case ABC_ACTION_COERCE_S:
            os << "ABC_ACTION_COERCE_S";
            break;
        case ABC_ACTION_ASTYPE:
            os << "ABC_ACTION_ASTYPE";
            break;
        case ABC_ACTION_ASTYPELATE:
            os << "ABC_ACTION_ASTYPELATE";
            break;
        case ABC_ACTION_COERCE_U:
            os << "ABC_ACTION_COERCE_U";
            break;
        case ABC_ACTION_COERCE_O:
            os << "ABC_ACTION_COERCE_O";
            break;
        case ABC_ACTION_0x8A:
            os << "ABC_ACTION_0x8A";
            break;
        case ABC_ACTION_0x8B:
            os << "ABC_ACTION_0x8B";
            break;
        case ABC_ACTION_0x8C:
            os << "ABC_ACTION_0x8C";
            break;
        case ABC_ACTION_0x8D:
            os << "ABC_ACTION_0x8D";
            break;
        case ABC_ACTION_0x8E:
            os << "ABC_ACTION_0x8E";
            break;
        case ABC_ACTION_0x8F:
            os << "ABC_ACTION_0x8F";
            break;
        case ABC_ACTION_NEGATE:
            os << "ABC_ACTION_NEGATE";
            break;
        case ABC_ACTION_INCREMENT:
            os << "ABC_ACTION_INCREMENT";
            break;
        case ABC_ACTION_INCLOCAL:
            os << "ABC_ACTION_INCLOCAL";
            break;
        case ABC_ACTION_DECREMENT:
            os << "ABC_ACTION_DECREMENT";
            break;
        case ABC_ACTION_DECLOCAL:
            os << "ABC_ACTION_DECLOCAL";
            break;
        case ABC_ACTION_ABC_TYPEOF:
            os << "ABC_ACTION_ABC_TYPEOF";
            break;
        case ABC_ACTION_NOT:
            os << "ABC_ACTION_NOT";
            break;
        case ABC_ACTION_BITNOT:
            os << "ABC_ACTION_BITNOT";
            break;
        case ABC_ACTION_0x98:
            os << "ABC_ACTION_0x98";
            break;
        case ABC_ACTION_0x99:
            os << "ABC_ACTION_0x99";
            break;
        case ABC_ACTION_CONCAT:
            os << "ABC_ACTION_CONCAT";
            break;
        case ABC_ACTION_ADD_D:
            os << "ABC_ACTION_ADD_D";
            break;
        case ABC_ACTION_0x9C:
            os << "ABC_ACTION_0x9C";
            break;
        case ABC_ACTION_0x9D:
            os << "ABC_ACTION_0x9D";
            break;
        case ABC_ACTION_0x9E:
            os << "ABC_ACTION_0x9E";
            break;
        case ABC_ACTION_0x9F:
            os << "ABC_ACTION_0x9F";
            break;
        case ABC_ACTION_ADD     :
            os << "ABC_ACTION_ADD       ";
            break;
        case ABC_ACTION_SUBTRACT:
            os << "ABC_ACTION_SUBTRACT";
            break;
        case ABC_ACTION_MULTIPLY:
            os << "ABC_ACTION_MULTIPLY";
            break;
        case ABC_ACTION_DIVIDE:
            os << "ABC_ACTION_DIVIDE";
            break;
        case ABC_ACTION_MODULO:
            os << "ABC_ACTION_MODULO";
            break;
        case ABC_ACTION_LSHIFT:
            os << "ABC_ACTION_LSHIFT";
            break;
        case ABC_ACTION_RSHIFT:
            os << "ABC_ACTION_RSHIFT";
            break;
        case ABC_ACTION_URSHIFT:
            os << "ABC_ACTION_URSHIFT";
            break;
        case ABC_ACTION_BITAND:
            os << "ABC_ACTION_BITAND";
            break;
        case ABC_ACTION_BITOR:
            os << "ABC_ACTION_BITOR";
            break;
        case ABC_ACTION_BITXOR:
            os << "ABC_ACTION_BITXOR";
            break;
        case ABC_ACTION_EQUALS:
            os << "ABC_ACTION_EQUALS";
            break;
        case ABC_ACTION_STRICTEQUALS:
            os << "ABC_ACTION_STRICTEQUALS";
            break;
        case ABC_ACTION_LESSTHAN:
            os << "ABC_ACTION_LESSTHAN";
            break;
        case ABC_ACTION_LESSEQUALS:
            os << "ABC_ACTION_LESSEQUALS";
            break;
        case ABC_ACTION_GREATERTHAN:
            os << "ABC_ACTION_GREATERTHAN";
            break;
        case ABC_ACTION_GREATEREQUALS:
            os << "ABC_ACTION_GREATEREQUALS";
            break;
        case ABC_ACTION_INSTANCEOF:
            os << "ABC_ACTION_INSTANCEOF";
            break;
        case ABC_ACTION_ISTYPE:
            os << "ABC_ACTION_ISTYPE";
            break;
        case ABC_ACTION_ISTYPELATE:
            os << "ABC_ACTION_ISTYPELATE";
            break;
        case ABC_ACTION_IN:
            os << "ABC_ACTION_IN";
            break;
        case ABC_ACTION_0xB5:
            os << "ABC_ACTION_0xB5";
            break;
        case ABC_ACTION_0xB6:
            os << "ABC_ACTION_0xB6";
            break;
        case ABC_ACTION_0xB7:
            os << "ABC_ACTION_0xB7";
            break;
        case ABC_ACTION_0xB8:
            os << "ABC_ACTION_0xB8";
            break;
        case ABC_ACTION_0xB9:
            os << "ABC_ACTION_0xB9";
            break;
        case ABC_ACTION_0xBA:
            os << "ABC_ACTION_0xBA";
            break;
        case ABC_ACTION_0xBB:
            os << "ABC_ACTION_0xBB";
            break;
        case ABC_ACTION_0xBC:
            os << "ABC_ACTION_0xBC";
            break;
        case ABC_ACTION_0xBD:
            os << "ABC_ACTION_0xBD";
            break;
        case ABC_ACTION_0xBE:
            os << "ABC_ACTION_0xBE";
            break;
        case ABC_ACTION_0xBF:
            os << "ABC_ACTION_0xBF";
            break;
        case ABC_ACTION_INCREMENT_I:
            os << "ABC_ACTION_INCREMENT_I";
            break;
        case ABC_ACTION_DECREMENT_I:
            os << "ABC_ACTION_DECREMENT_I";
            break;
        case ABC_ACTION_INCLOCAL_I:
            os << "ABC_ACTION_INCLOCAL_I";
            break;
        case ABC_ACTION_DECLOCAL_I:
            os << "ABC_ACTION_DECLOCAL_I";
            break;
        case ABC_ACTION_NEGATE_I:
            os << "ABC_ACTION_NEGATE_I";
            break;
        case ABC_ACTION_ADD_I:
            os << "ABC_ACTION_ADD_I";
            break;
        case ABC_ACTION_SUBTRACT_I:
            os << "ABC_ACTION_SUBTRACT_I";
            break;
        case ABC_ACTION_MULTIPLY_I:
            os << "ABC_ACTION_MULTIPLY_I";
            break;
        case ABC_ACTION_0xC8:
            os << "ABC_ACTION_0xC8";
            break;
        case ABC_ACTION_0xC9:
            os << "ABC_ACTION_0xC9";
            break;
        case ABC_ACTION_0xCA:
            os << "ABC_ACTION_0xCA";
            break;
        case ABC_ACTION_0xCB:
            os << "ABC_ACTION_0xCB";
            break;
        case ABC_ACTION_0xCC:
            os << "ABC_ACTION_0xCC";
            break;
        case ABC_ACTION_0xCD:
            os << "ABC_ACTION_0xCD";
            break;
        case ABC_ACTION_0xCE:
            os << "ABC_ACTION_0xCE";
            break;
        case ABC_ACTION_0xCF:
            os << "ABC_ACTION_0xCF";
            break;
        case ABC_ACTION_GETLOCAL0:
            os << "ABC_ACTION_GETLOCAL0";
            break;
        case ABC_ACTION_GETLOCAL1:
            os << "ABC_ACTION_GETLOCAL1";
            break;
        case ABC_ACTION_GETLOCAL2:
            os << "ABC_ACTION_GETLOCAL2";
            break;
        case ABC_ACTION_GETLOCAL3:
            os << "ABC_ACTION_GETLOCAL3";
            break;
        case ABC_ACTION_SETLOCAL0:
            os << "ABC_ACTION_SETLOCAL0";
            break;
        case ABC_ACTION_SETLOCAL1:
            os << "ABC_ACTION_SETLOCAL1";
            break;
        case ABC_ACTION_SETLOCAL2:
            os << "ABC_ACTION_SETLOCAL2";
            break;
        case ABC_ACTION_SETLOCAL3:
            os << "ABC_ACTION_SETLOCAL3";
            break;
        case ABC_ACTION_0xD8:
            os << "ABC_ACTION_0xD8";
            break;
        case ABC_ACTION_0xD9:
            os << "ABC_ACTION_0xD9";
            break;
        case ABC_ACTION_0xDA:
            os << "ABC_ACTION_0xDA";
            break;
        case ABC_ACTION_0xDB:
            os << "ABC_ACTION_0xDB";
            break;
        case ABC_ACTION_0xDC:
            os << "ABC_ACTION_0xDC";
            break;
        case ABC_ACTION_0xDD:
            os << "ABC_ACTION_0xDD";
            break;
        case ABC_ACTION_0xDE:
            os << "ABC_ACTION_0xDE";
            break;
        case ABC_ACTION_0xDF:
            os << "ABC_ACTION_0xDF";
            break;
        case ABC_ACTION_0xE0:
            os << "ABC_ACTION_0xE0";
            break;
        case ABC_ACTION_0xE1:
            os << "ABC_ACTION_0xE1";
            break;
        case ABC_ACTION_0xE2:
            os << "ABC_ACTION_0xE2";
            break;
        case ABC_ACTION_0xE3:
            os << "ABC_ACTION_0xE3";
            break;
        case ABC_ACTION_0xE4:
            os << "ABC_ACTION_0xE4";
            break;
        case ABC_ACTION_0xE5:
            os << "ABC_ACTION_0xE5";
            break;
        case ABC_ACTION_0xE6:
            os << "ABC_ACTION_0xE6";
            break;
        case ABC_ACTION_0xE7:
            os << "ABC_ACTION_0xE7";
            break;
        case ABC_ACTION_0xE8:
            os << "ABC_ACTION_0xE8";
            break;
        case ABC_ACTION_0xE9:
            os << "ABC_ACTION_0xE9";
            break;
        case ABC_ACTION_0xEA:
            os << "ABC_ACTION_0xEA";
            break;
        case ABC_ACTION_0xEB:
            os << "ABC_ACTION_0xEB";
            break;
        case ABC_ACTION_0xEC:
            os << "ABC_ACTION_0xEC";
            break;
        case ABC_ACTION_0xED:
            os << "ABC_ACTION_0xED";
            break;
        case ABC_ACTION_ABS_JUMP:
            os << "ABC_ACTION_ABS_JUMP";
            break;
        case ABC_ACTION_DEBUG:
            os << "ABC_ACTION_DEBUG";
            break;
        case ABC_ACTION_DEBUGLINE:
            os << "ABC_ACTION_DEBUGLINE";
            break;
        case ABC_ACTION_DEBUGFILE:
            os << "ABC_ACTION_DEBUGFILE";
            break;
        case ABC_ACTION_BKPTLINE:
            os << "ABC_ACTION_BKPTLINE";
            break;
        case ABC_ACTION_TIMESTAMP:
            os << "ABC_ACTION_TIMESTAMP";
            break;
        case ABC_ACTION_0xF4:
            os << "ABC_ACTION_0xF4";
            break;
        case ABC_ACTION_VERIFYPASS:
            os << "ABC_ACTION_VERIFYPASS";
            break;
        case ABC_ACTION_ALLOC:
            os << "ABC_ACTION_ALLOC";
            break;
        case ABC_ACTION_MARK:
            os << "ABC_ACTION_MARK";
            break;
        case ABC_ACTION_WB:
            os << "ABC_ACTION_WB";
            break;
        case ABC_ACTION_PROLOGUE:
            os << "ABC_ACTION_PROLOGUE";
            break;
        case ABC_ACTION_SENDENTER:
            os << "ABC_ACTION_SENDENTER";
            break;
        case ABC_ACTION_DOUBLETOATOM:
            os << "ABC_ACTION_DOUBLETOATOM";
            break;
        case ABC_ACTION_SWEEP:
            os << "ABC_ACTION_SWEEP";
            break;
        case ABC_ACTION_CODEGENOP:
            os << "ABC_ACTION_CODEGENOP";
            break;
        case ABC_ACTION_VERIFYOP:
            os << "ABC_ACTION_VERIFYOP";
            break;
        default:
            os << "UNKNOWN";
            break;
    }
    return os;
}

} // namespace gnash::SWF
} // namespace gnash
