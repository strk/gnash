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
operator<<(std::ostream& o, TagType t)
{
    switch (t) {
        case END: return o << "END";
        case SHOWFRAME: return o << "SHOWFRAME";
        case DEFINESHAPE: return o << "DEFINESHAPE";
        case FREECHARACTER: return o << "FREECHARACTER";
        case PLACEOBJECT: return o << "PLACEOBJECT";
        case REMOVEOBJECT: return o << "REMOVEOBJECT";
        case DEFINEBITS: return o << "DEFINEBITS";
        case DEFINEBUTTON: return o << "DEFINEBUTTON";
        case JPEGTABLES: return o << "JPEGTABLES";
        case SETBACKGROUNDCOLOR: return o << "SETBACKGROUNDCOLOR";
        case DEFINEFONT: return o << "DEFINEFONT";
        case DEFINETEXT: return o << "DEFINETEXT";
        case DOACTION: return o << "DOACTION";
        case DEFINEFONTINFO: return o << "DEFINEFONTINFO";
        case DEFINESOUND: return o << "DEFINESOUND";
        case STARTSOUND: return o << "STARTSOUND";
        case STOPSOUND: return o << "STOPSOUND";
        case DEFINEBUTTONSOUND: return o << "DEFINEBUTTONSOUND";
        case SOUNDSTREAMHEAD: return o << "SOUNDSTREAMHEAD";
        case SOUNDSTREAMBLOCK: return o << "SOUNDSTREAMBLOCK";
        case DEFINELOSSLESS: return o << "DEFINELOSSLESS";
        case DEFINEBITSJPEG2: return o << "DEFINEBITSJPEG2";
        case DEFINESHAPE2: return o << "DEFINESHAPE2";
        case DEFINEBUTTONCXFORM: return o << "DEFINEBUTTONCXFORM";
        case PROTECT: return o << "PROTECT";
        case PATHSAREPOSTSCRIPT: return o << "PATHSAREPOSTSCRIPT";
        case PLACEOBJECT2: return o << "PLACEOBJECT2";
        case REMOVEOBJECT2: return o << "REMOVEOBJECT2";
        case SYNCFRAME: return o << "SYNCFRAME";
        case FREEALL: return o << "FREEALL";
        case DEFINESHAPE3: return o << "DEFINESHAPE3";
        case DEFINETEXT2: return o << "DEFINETEXT2";
        case DEFINEBUTTON2: return o << "DEFINEBUTTON2";
        case DEFINEBITSJPEG3: return o << "DEFINEBITSJPEG3";
        case DEFINELOSSLESS2: return o << "DEFINELOSSLESS2";
        case DEFINEEDITTEXT: return o << "DEFINEEDITTEXT";
        case DEFINEVIDEO: return o << "DEFINEVIDEO";
        case DEFINESPRITE: return o << "DEFINESPRITE";
        case NAMECHARACTER: return o << "NAMECHARACTER";
        case SERIALNUMBER: return o << "SERIALNUMBER";
        case DEFINETEXTFORMAT: return o << "DEFINETEXTFORMAT";
        case FRAMELABEL: return o << "FRAMELABEL";
        case DEFINEBEHAVIOR: return o << "DEFINEBEHAVIOR";
        case SOUNDSTREAMHEAD2: return o << "SOUNDSTREAMHEAD2";
        case DEFINEMORPHSHAPE: return o << "DEFINEMORPHSHAPE";
        case FRAMETAG: return o << "FRAMETAG";
        case DEFINEFONT2: return o << "DEFINEFONT2";
        case GENCOMMAND: return o << "GENCOMMAND";
        case DEFINECOMMANDOBJ: return o << "DEFINECOMMANDOBJ";
        case CHARACTERSET: return o << "CHARACTERSET";
        case FONTREF: return o << "FONTREF";
        case DEFINEFUNCTION: return o << "DEFINEFUNCTION";
        case PLACEFUNCTION: return o << "PLACEFUNCTION";
        case GENTAGOBJECT: return o << "GENTAGOBJECT";
        case EXPORTASSETS: return o << "EXPORTASSETS";
        case IMPORTASSETS: return o << "IMPORTASSETS";
        case ENABLEDEBUGGER: return o << "ENABLEDEBUGGER";
        case INITACTION: return o << "INITACTION";
        case DEFINEVIDEOSTREAM: return o << "DEFINEVIDEOSTREAM";
        case VIDEOFRAME: return o << "VIDEOFRAME";
        case DEFINEFONTINFO2: return o << "DEFINEFONTINFO2";
        case DEBUGID: return o << "DEBUGID";
        case ENABLEDEBUGGER2: return o << "ENABLEDEBUGGER2";
        case SCRIPTLIMITS: return o << "SCRIPTLIMITS";
        case SETTABINDEX: return o << "SETTABINDEX";
        case DEFINESHAPE4_: return o << "DEFINESHAPE4_";
        case DEFINEMORPHSHAPE2_: return o << "DEFINEMORPHSHAPE2_";
        case FILEATTRIBUTES: return o << "FILEATTRIBUTES";
        case PLACEOBJECT3: return o << "PLACEOBJECT3";
        case IMPORTASSETS2: return o << "IMPORTASSETS2";
        case DOABC: return o << "DOABC";
        case DEFINEALIGNZONES: return o << "DEFINEALIGNZONES";
        case CSMTEXTSETTINGS: return o << "CSMTEXTSETTINGS";
        case DEFINEFONT3: return o << "DEFINEFONT3";
        case SYMBOLCLASS: return o << "SYMBOLCLASS";
        case METADATA: return o << "METADATA";
        case DEFINESCALINGGRID: return o << "DEFINESCALINGGRID";
        case DOABCDEFINE: return o << "DOABCDEFINE";
        case DEFINESHAPE4: return o << "DEFINESHAPE4";
        case DEFINEMORPHSHAPE2: return o << "DEFINEMORPHSHAPE2";
        case DEFINESCENEANDFRAMELABELDATA: return o << "DEFINESCENEANDFRAMELABELDATA";
        case DEFINEBINARYDATA: return o << "DEFINEBINARYDATA";
        case DEFINEFONTNAME: return o << "DEFINEFONTNAME";
        case STARTSOUND2: return o << "STARTSOUND2";
        case DEFINEBITSJPEG4: return o << "DEFINEBITSJPEG4";
        case REFLEX: return o << "REFLEX";
        case DEFINEBITSPTR: return o << "DEFINEBITSPTR";
        default: return o << "Unknown TagType " << (int)t;
    }
    return o;
}

std::ostream&
operator<<(std::ostream& o, ActionType a)
{
    o << "Action";
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
        case ACTION_STRINGLENGTH: return o << "StringLength";
        case ACTION_SUBSTRING: return o << "SubString";
        case ACTION_POP: return o << "Pop";
        case ACTION_INT: return o << "Int";
        case ACTION_GETVARIABLE: return o << "GetVariable";
        case ACTION_SETVARIABLE: return o << "SetVariable";
        case ACTION_SETTARGETEXPRESSION: return o << "SetTargetExpression";
        case ACTION_STRINGCONCAT: return o << "StringConcat";
        case ACTION_GETPROPERTY: return o << "GetProperty";
        case ACTION_SETPROPERTY: return o << "SetProperty";
        case ACTION_DUPLICATECLIP: return o << "DuplicateClip";
        case ACTION_REMOVECLIP: return o << "RemoveClip";
        case ACTION_TRACE: return o << "Trace";
        case ACTION_STARTDRAGMOVIE: return o << "StartDragMovie";
        case ACTION_STOPDRAGMOVIE: return o << "StopDragMovie";
        case ACTION_STRINGCOMPARE: return o << "StringCompare";
        case ACTION_THROW: return o << "Throw";
        case ACTION_CASTOP: return o << "CastOp";
        case ACTION_IMPLEMENTSOP: return o << "ImplementsOp";
        case ACTION_FSCOMMAND2: return o << "Fscommand2";
        case ACTION_RANDOM: return o << "Random";
        case ACTION_MBLENGTH: return o << "MbLength";
        case ACTION_ORD: return o << "Ord";
        case ACTION_CHR: return o << "Chr";
        case ACTION_GETTIMER: return o << "GetTimer";
        case ACTION_MBSUBSTRING: return o << "MbSubString";
        case ACTION_MBORD: return o << "MbOrd";
        case ACTION_MBCHR: return o << "MbChr";
        case ACTION_STRICTMODE: return o << "StrictMode";
        case ACTION_WAITFORFRAMEEXPRESSION:
            return o << "WaitForFrameExpression";
        case ACTION_PUSHDATA: return o << "PushData";
        case ACTION_BRANCHALWAYS: return o << "BranchAlways";
        case ACTION_GETURL2: return o << "GetUrl2";
        case ACTION_BRANCHIFTRUE: return o << "BranchIfTrue";
        case ACTION_CALLFRAME: return o << "CallFrame";
        case ACTION_GOTOEXPRESSION: return o << "GotoExpression";
        case ACTION_DELETE: return o << "Delete";
        case ACTION_DELETE2: return o << "Delete2";
        case ACTION_VAREQUALS: return o << "VarEquals";
        case ACTION_CALLFUNCTION: return o << "CallFunction";
        case ACTION_RETURN: return o << "Return";
        case ACTION_MODULO: return o << "Modulo";
        case ACTION_NEW: return o << "New";
        case ACTION_VAR: return o << "Var";
        case ACTION_INITARRAY: return o << "InitArray";
        case ACTION_INITOBJECT: return o << "InitObject";
        case ACTION_TYPEOF: return o << "TypeOf";
        case ACTION_TARGETPATH: return o << "TargetPath";
        case ACTION_ENUMERATE: return o << "Enumerate";
        case ACTION_NEWADD: return o << "NewAdd";
        case ACTION_NEWLESSTHAN: return o << "NewLessThan";
        case ACTION_NEWEQUALS: return o << "NewEquals";
        case ACTION_TONUMBER: return o << "ToNumber";
        case ACTION_TOSTRING: return o << "ToString";
        case ACTION_DUP: return o << "Dup";
        case ACTION_SWAP: return o << "Swap";
        case ACTION_GETMEMBER: return o << "GetMember";
        case ACTION_SETMEMBER: return o << "SetMember";
        case ACTION_INCREMENT: return o << "Increment";
        case ACTION_DECREMENT: return o << "Decrement";
        case ACTION_CALLMETHOD: return o << "CallMethod";
        case ACTION_NEWMETHOD: return o << "NewMethod";
        case ACTION_INSTANCEOF: return o << "InstanceOf";
        case ACTION_ENUM2: return o << "Enum2";
        case ACTION_BITWISEAND: return o << "BitwiseAnd";
        case ACTION_BITWISEOR: return o << "BitwiseOr";
        case ACTION_BITWISEXOR: return o << "BitwiseXor";
        case ACTION_SHIFTLEFT: return o << "ShiftLeft";
        case ACTION_SHIFTRIGHT: return o << "ShiftRight";
        case ACTION_SHIFTRIGHT2: return o << "ShiftRight2";
        case ACTION_STRICTEQ: return o << "StrictEq";
        case ACTION_GREATER: return o << "Greater";
        case ACTION_STRINGGREATER: return o << "StringGreater";
        case ACTION_EXTENDS: return o << "Extends";
        case ACTION_CONSTANTPOOL: return o << "ConstantPool";
        case ACTION_DEFINEFUNCTION2: return o << "DefineFunction2";
        case ACTION_TRY: return o << "Try";
        case ACTION_WITH: return o << "With";
        case ACTION_DEFINEFUNCTION: return o << "DefineFunction";
        case ACTION_SETREGISTER: return o << "SetRegister";
        default: return o << " Unknown Type " << +a;
    }
    return o;
}

std::ostream&
operator<<(std::ostream& os, const abc_action_type& opcode)
{
    os << "ABC action: ";

    switch (opcode) {
        case ABC_ACTION_END: return os << "END";
        case ABC_ACTION_BKPT: return os << "BKPT";
        case ABC_ACTION_NOP: return os << "NOP";
        case ABC_ACTION_THROW: return os << "THROW";
        case ABC_ACTION_GETSUPER: return os << "GETSUPER";
        case ABC_ACTION_SETSUPER: return os << "SETSUPER";
        case ABC_ACTION_DXNS: return os << "DXNS";
        case ABC_ACTION_DXNSLATE: return os << "DXNSLATE";
        case ABC_ACTION_KILL: return os << "KILL";
        case ABC_ACTION_LABEL: return os << "LABEL";
        case ABC_ACTION_IFNLT: return os << "IFNLT";
        case ABC_ACTION_IFNLE: return os << "IFNLE";
        case ABC_ACTION_IFNGT: return os << "IFNGT";
        case ABC_ACTION_IFNGE: return os << "IFNGE";
        case ABC_ACTION_JUMP: return os << "JUMP";
        case ABC_ACTION_IFTRUE: return os << "IFTRUE";
        case ABC_ACTION_IFFALSE: return os << "IFFALSE";
        case ABC_ACTION_IFEQ: return os << "IFEQ";
        case ABC_ACTION_IFNE: return os << "IFNE";
        case ABC_ACTION_IFLT: return os << "IFLT";
        case ABC_ACTION_IFLE: return os << "IFLE";
        case ABC_ACTION_IFGT: return os << "IFGT";
        case ABC_ACTION_IFGE: return os << "IFGE";
        case ABC_ACTION_IFSTRICTEQ: return os << "IFSTRICTEQ";
        case ABC_ACTION_IFSTRICTNE: return os << "IFSTRICTNE";
        case ABC_ACTION_LOOKUPSWITCH: return os << "LOOKUPSWITCH";
        case ABC_ACTION_PUSHWITH: return os << "PUSHWITH";
        case ABC_ACTION_POPSCOPE: return os << "POPSCOPE";
        case ABC_ACTION_NEXTNAME: return os << "NEXTNAME";
        case ABC_ACTION_HASNEXT: return os << "HASNEXT";
        case ABC_ACTION_PUSHNULL: return os << "PUSHNULL";
        case ABC_ACTION_PUSHUNDEFINED: return os << "PUSHUNDEFINED";
        case ABC_ACTION_NEXTVALUE: return os << "NEXTVALUE";
        case ABC_ACTION_PUSHBYTE: return os << "PUSHBYTE";
        case ABC_ACTION_PUSHSHORT: return os << "PUSHSHORT";
        case ABC_ACTION_PUSHTRUE: return os << "PUSHTRUE";
        case ABC_ACTION_PUSHFALSE: return os << "PUSHFALSE";
        case ABC_ACTION_PUSHNAN: return os << "PUSHNAN";
        case ABC_ACTION_POP: return os << "POP";
        case ABC_ACTION_DUP: return os << "DUP";
        case ABC_ACTION_SWAP: return os << "SWAP";
        case ABC_ACTION_PUSHSTRING: return os << "PUSHSTRING";
        case ABC_ACTION_PUSHINT: return os << "PUSHINT";
        case ABC_ACTION_PUSHUINT: return os << "PUSHUINT";
        case ABC_ACTION_PUSHDOUBLE: return os << "PUSHDOUBLE";
        case ABC_ACTION_PUSHSCOPE: return os << "PUSHSCOPE";
        case ABC_ACTION_PUSHNAMESPACE: return os << "PUSHNAMESPACE";
        case ABC_ACTION_HASNEXT2: return os << "HASNEXT2";
        case ABC_ACTION_NEWFUNCTION: return os << "NEWFUNCTION";
        case ABC_ACTION_CALL: return os << "CALL";
        case ABC_ACTION_CONSTRUCT: return os << "CONSTRUCT";
        case ABC_ACTION_CALLMETHOD: return os << "CALLMETHOD";
        case ABC_ACTION_CALLSTATIC: return os << "CALLSTATIC";
        case ABC_ACTION_CALLSUPER: return os << "CALLSUPER";
        case ABC_ACTION_CALLPROPERTY: return os << "CALLPROPERTY";
        case ABC_ACTION_RETURNVOID: return os << "RETURNVOID";
        case ABC_ACTION_RETURNVALUE: return os << "RETURNVALUE";
        case ABC_ACTION_CONSTRUCTSUPER: return os << "CONSTRUCTSUPER";
        case ABC_ACTION_CONSTRUCTPROP: return os << "CONSTRUCTPROP";
        case ABC_ACTION_CALLSUPERID: return os << "CALLSUPERID";
        case ABC_ACTION_CALLPROPLEX: return os << "CALLPROPLEX";
        case ABC_ACTION_CALLINTERFACE: return os << "CALLINTERFACE";
        case ABC_ACTION_CALLSUPERVOID: return os << "CALLSUPERVOID";
        case ABC_ACTION_CALLPROPVOID: return os << "CALLPROPVOID";
        case ABC_ACTION_NEWOBJECT: return os << "NEWOBJECT";
        case ABC_ACTION_NEWARRAY: return os << "NEWARRAY";
        case ABC_ACTION_NEWACTIVATION: return os << "NEWACTIVATION";
        case ABC_ACTION_NEWCLASS: return os << "NEWCLASS";
        case ABC_ACTION_GETDESCENDANTS: return os << "GETDESCENDANTS";
        case ABC_ACTION_NEWCATCH: return os << "NEWCATCH";
        case ABC_ACTION_FINDPROPSTRICT: return os << "FINDPROPSTRICT";
        case ABC_ACTION_FINDPROPERTY: return os << "FINDPROPERTY";
        case ABC_ACTION_FINDDEF: return os << "FINDDEF";
        case ABC_ACTION_GETLEX: return os << "GETLEX";
        case ABC_ACTION_SETPROPERTY: return os << "SETPROPERTY";
        case ABC_ACTION_GETLOCAL: return os << "GETLOCAL";
        case ABC_ACTION_SETLOCAL: return os << "SETLOCAL";
        case ABC_ACTION_GETGLOBALSCOPE: return os << "GETGLOBALSCOPE";
        case ABC_ACTION_GETSCOPEOBJECT: return os << "GETSCOPEOBJECT";
        case ABC_ACTION_GETPROPERTY: return os << "GETPROPERTY";
        case ABC_ACTION_INITPROPERTY: return os << "INITPROPERTY";
        case ABC_ACTION_DELETEPROPERTY: return os << "DELETEPROPERTY";
        case ABC_ACTION_GETSLOT: return os << "GETSLOT";
        case ABC_ACTION_SETSLOT: return os << "SETSLOT";
        case ABC_ACTION_GETGLOBALSLOT: return os << "GETGLOBALSLOT";
        case ABC_ACTION_SETGLOBALSLOT: return os << "SETGLOBALSLOT";
        case ABC_ACTION_CONVERT_S: return os << "CONVERT_S";
        case ABC_ACTION_ESC_XELEM: return os << "ESC_XELEM";
        case ABC_ACTION_ESC_XATTR: return os << "ESC_XATTR";
        case ABC_ACTION_CONVERT_I: return os << "CONVERT_I";
        case ABC_ACTION_CONVERT_U: return os << "CONVERT_U";
        case ABC_ACTION_CONVERT_D: return os << "CONVERT_D";
        case ABC_ACTION_CONVERT_B: return os << "CONVERT_B";
        case ABC_ACTION_CONVERT_O: return os << "CONVERT_O";
        case ABC_ACTION_CHECKFILTER: return os << "CHECKFILTER";
        case ABC_ACTION_COERCE: return os << "COERCE";
        case ABC_ACTION_COERCE_B: return os << "COERCE_B";
        case ABC_ACTION_COERCE_A: return os << "COERCE_A";
        case ABC_ACTION_COERCE_I: return os << "COERCE_I";
        case ABC_ACTION_COERCE_D: return os << "COERCE_D";
        case ABC_ACTION_COERCE_S: return os << "COERCE_S";
        case ABC_ACTION_ASTYPE: return os << "ASTYPE";
        case ABC_ACTION_ASTYPELATE: return os << "ASTYPELATE";
        case ABC_ACTION_COERCE_U: return os << "COERCE_U";
        case ABC_ACTION_COERCE_O: return os << "COERCE_O";
        case ABC_ACTION_NEGATE: return os << "NEGATE";
        case ABC_ACTION_INCREMENT: return os << "INCREMENT";
        case ABC_ACTION_INCLOCAL: return os << "INCLOCAL";
        case ABC_ACTION_DECREMENT: return os << "DECREMENT";
        case ABC_ACTION_DECLOCAL: return os << "DECLOCAL";
        case ABC_ACTION_ABC_TYPEOF: return os << "ABC_TYPEOF";
        case ABC_ACTION_NOT: return os << "NOT";
        case ABC_ACTION_BITNOT: return os << "BITNOT";
        case ABC_ACTION_CONCAT: return os << "CONCAT";
        case ABC_ACTION_ADD_D: return os << "ADD_D";
        case ABC_ACTION_ADD     : return os << "ADD       ";
        case ABC_ACTION_SUBTRACT: return os << "SUBTRACT";
        case ABC_ACTION_MULTIPLY: return os << "MULTIPLY";
        case ABC_ACTION_DIVIDE: return os << "DIVIDE";
        case ABC_ACTION_MODULO: return os << "MODULO";
        case ABC_ACTION_LSHIFT: return os << "LSHIFT";
        case ABC_ACTION_RSHIFT: return os << "RSHIFT";
        case ABC_ACTION_URSHIFT: return os << "URSHIFT";
        case ABC_ACTION_BITAND: return os << "BITAND";
        case ABC_ACTION_BITOR: return os << "BITOR";
        case ABC_ACTION_BITXOR: return os << "BITXOR";
        case ABC_ACTION_EQUALS: return os << "EQUALS";
        case ABC_ACTION_STRICTEQUALS: return os << "STRICTEQUALS";
        case ABC_ACTION_LESSTHAN: return os << "LESSTHAN";
        case ABC_ACTION_LESSEQUALS: return os << "LESSEQUALS";
        case ABC_ACTION_GREATERTHAN: return os << "GREATERTHAN";
        case ABC_ACTION_GREATEREQUALS: return os << "GREATEREQUALS";
        case ABC_ACTION_INSTANCEOF: return os << "INSTANCEOF";
        case ABC_ACTION_ISTYPE: return os << "ISTYPE";
        case ABC_ACTION_ISTYPELATE: return os << "ISTYPELATE";
        case ABC_ACTION_IN: return os << "IN";
        case ABC_ACTION_INCREMENT_I: return os << "INCREMENT_I";
        case ABC_ACTION_DECREMENT_I: return os << "DECREMENT_I";
        case ABC_ACTION_INCLOCAL_I: return os << "INCLOCAL_I";
        case ABC_ACTION_DECLOCAL_I: return os << "DECLOCAL_I";
        case ABC_ACTION_NEGATE_I: return os << "NEGATE_I";
        case ABC_ACTION_ADD_I: return os << "ADD_I";
        case ABC_ACTION_SUBTRACT_I: return os << "SUBTRACT_I";
        case ABC_ACTION_MULTIPLY_I: return os << "MULTIPLY_I";
        case ABC_ACTION_GETLOCAL0: return os << "GETLOCAL0";
        case ABC_ACTION_GETLOCAL1: return os << "GETLOCAL1";
        case ABC_ACTION_GETLOCAL2: return os << "GETLOCAL2";
        case ABC_ACTION_GETLOCAL3: return os << "GETLOCAL3";
        case ABC_ACTION_SETLOCAL0: return os << "SETLOCAL0";
        case ABC_ACTION_SETLOCAL1: return os << "SETLOCAL1";
        case ABC_ACTION_SETLOCAL2: return os << "SETLOCAL2";
        case ABC_ACTION_SETLOCAL3: return os << "SETLOCAL3";
        case ABC_ACTION_ABS_JUMP: return os << "ABS_JUMP";
        case ABC_ACTION_DEBUG: return os << "DEBUG";
        case ABC_ACTION_DEBUGLINE: return os << "DEBUGLINE";
        case ABC_ACTION_DEBUGFILE: return os << "DEBUGFILE";
        case ABC_ACTION_BKPTLINE: return os << "BKPTLINE";
        case ABC_ACTION_TIMESTAMP: return os << "TIMESTAMP";
        case ABC_ACTION_VERIFYPASS: return os << "VERIFYPASS";
        case ABC_ACTION_ALLOC: return os << "ALLOC";
        case ABC_ACTION_MARK: return os << "MARK";
        case ABC_ACTION_WB: return os << "WB";
        case ABC_ACTION_PROLOGUE: return os << "PROLOGUE";
        case ABC_ACTION_SENDENTER: return os << "SENDENTER";
        case ABC_ACTION_DOUBLETOATOM: return os << "DOUBLETOATOM";
        case ABC_ACTION_SWEEP: return os << "SWEEP";
        case ABC_ACTION_CODEGENOP: return os << "CODEGENOP";
        case ABC_ACTION_VERIFYOP: return os << "VERIFYOP";
        default: return os << "UNKNOWN " << std::hex << +opcode;
    }
}

} // namespace gnash::SWF
} // namespace gnash
