// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
//

#ifndef GNASH_SWF_H
#define GNASH_SWF_H

namespace gnash {

/// SWF format parsing classes
namespace SWF { // gnash::SWF

/// SWF tag types. Symbolc names copied from Ming
typedef enum
{
    /// end of sprite or movie definition
    END                   =  0,
    SHOWFRAME             =  1,
    DEFINESHAPE           =  2,
    FREECHARACTER	  =  3,
    PLACEOBJECT           =  4,
    REMOVEOBJECT          =  5,
    DEFINEBITS            =  6,
    DEFINEBUTTON          =  7,
    JPEGTABLES            =  8,
    SETBACKGROUNDCOLOR    =  9,
    DEFINEFONT            = 10,
    DEFINETEXT            = 11,
    DOACTION              = 12,
    DEFINEFONTINFO        = 13,
    DEFINESOUND           = 14,
    STARTSOUND            = 15,
    DEFINEBUTTONSOUND     = 17,
    SOUNDSTREAMHEAD       = 18,
    SOUNDSTREAMBLOCK      = 19,
    DEFINELOSSLESS        = 20,
    DEFINEBITSJPEG2       = 21,
    DEFINESHAPE2          = 22,
    DEFINEBUTTONCXFORM    = 23,
    PROTECT               = 24,
    PATHSAREPOSTSCRIPT	  = 25,
    PLACEOBJECT2          = 26,
    REMOVEOBJECT2         = 28,
    SYNCFRAME		  = 29,
    FREEALL		  = 31,
    DEFINESHAPE3          = 32,
    DEFINETEXT2           = 33,
    DEFINEBUTTON2         = 34,
    DEFINEBITSJPEG3       = 35,
    DEFINELOSSLESS2       = 36,
    DEFINEEDITTEXT	  = 37,
    DEFINEVIDEO		  = 38,

    /// Definition of a Sprite/MovieClip
    DEFINESPRITE          = 39,

    NAMECHARACTER	  = 40,
    SERIALNUMBER          = 41,
    DEFINETEXTFORMAT	  = 42,
    FRAMELABEL            = 43,
    SOUNDSTREAMHEAD2      = 45,
    DEFINEMORPHSHAPE      = 46,
    FRAMETAG		  = 47,
    DEFINEFONT2           = 48,
    GENCOMMAND		  = 49,
    DEFINECOMMANDOBJ	  = 50,
    CHARACTERSET	  = 51,
    FONTREF		  = 52,
    EXPORTASSETS          = 56,
    IMPORTASSETS          = 57,

    /// See http://sswf.sourceforge.net/SWFalexref.html#tag_protectdebug
    ENABLEDEBUGGER	  = 58,

    /// For actions specified with initclip directive
    INITACTION		  = 59,

    DEFINEVIDEOSTREAM	  = 60,
    VIDEOFRAME		  = 61,

    /// DefineFontInfo2 (swf6)
    DEFINEFONTINFO2       = 62,

    /// See http://flasm.sourceforge.net/#protect
    /// See http://sswf.sourceforge.net/SWFalexref.html#tag_protectdebug
    ENABLEDEBUGGER2	  = 64,

    /// SWF_version  >= 7 
    /// See http://sswf.sourceforge.net/SWFalexref.html#tag_scriptlimits
    SCRIPTLIMITS          = 65, 

    /// http://sswf.sourceforge.net/SWFalexref.html#tag_fileattributes
    FILEATTRIBUTES        = 69,

    /// http://sswf.sourceforge.net/SWFalexref.html#tag_placeobject3
    PLACEOBJECT3          = 70,

    /// http://sswf.sourceforge.net/SWFalexref.html#tag_import2
    IMPORTASSETS2         = 71,

    /// http://sswf.sourceforge.net/SWFalexref.html#tag_definefontalignzones
    DEFINEALIGNZONES      = 73,

    /// http://sswf.sourceforge.net/SWFalexref.html#tag_csmtextsettings
    CSMTEXTSETTINGS       = 74,

    /// http://sswf.sourceforge.net/SWFalexref.html#tag_definefont3
    DEFINEFONT3           = 75,

    /// SWF9
    /// http://sswf.sourceforge.net/SWFalexref.html#tag_symbolclass
    SYMBOLCLASS           = 76,

    /// http://sswf.sourceforge.net/SWFalexref.html#tag_metadata
    METADATA              = 77,

    /// http://sswf.sourceforge.net/SWFalexref.html#tag_definescalinggrid
    DEFINESCALINGGRID     = 78,

    /// SWF9
    /// http://sswf.sourceforge.net/SWFalexref.html#tag_doabcdefine
    DOABCDEFINE           = 82,

    /// http://sswf.sourceforge.net/SWFalexref.html#tag_definemorphshape2
    DEFINESHAPE4          = 83,

    /// http://sswf.sourceforge.net/SWFalexref.html#tag_definemorphshape2
    DEFINEMORPHSHAPE2     = 84,

    REFLEX                = 777,

    DEFINEBITSPTR         = 1023
} tag_type;

/// SWF action ids. Symbolic names copied from Ming.
//
/// For semantic of each action see:
/// http://sswf.sourceforge.net/SWFalexref.html
///
typedef enum
{
    ACTION_END                     = 0x00,
    ACTION_NEXTFRAME               = 0x04,
    ACTION_PREVFRAME               = 0x05,
    ACTION_PLAY                    = 0x06,
    ACTION_STOP                    = 0x07,
    ACTION_TOGGLEQUALITY           = 0x08,
    ACTION_STOPSOUNDS              = 0x09,
    ACTION_GOTOFRAME               = 0x81, /* >= 0x80 means record has args */
    ACTION_GETURL                  = 0x83,
    ACTION_WAITFORFRAME            = 0x8A,
    ACTION_SETTARGET               = 0x8B,
    ACTION_GOTOLABEL               = 0x8C,
    ACTION_ADD                     = 0x0A,
    ACTION_SUBTRACT                = 0x0B,
    ACTION_MULTIPLY                = 0x0C,
    ACTION_DIVIDE                  = 0x0D,

    /// Numeric equality (SWF4, replaced by ACTION_NEWEQUALS starting at SWF5)
    ACTION_EQUAL                   = 0x0E,

    ACTION_LESSTHAN                = 0x0F,
    ACTION_LOGICALAND              = 0x10,
    ACTION_LOGICALOR               = 0x11,
    ACTION_LOGICALNOT              = 0x12,
    ACTION_STRINGEQ                = 0x13,
    ACTION_STRINGLENGTH            = 0x14,
    ACTION_SUBSTRING               = 0x15,
    ACTION_POP                     = 0x17,
    ACTION_INT                     = 0x18,
    ACTION_GETVARIABLE             = 0x1C,
    ACTION_SETVARIABLE             = 0x1D,
    ACTION_SETTARGETEXPRESSION     = 0x20,
    ACTION_STRINGCONCAT            = 0x21,
    ACTION_GETPROPERTY             = 0x22,
    ACTION_SETPROPERTY             = 0x23,
    ACTION_DUPLICATECLIP           = 0x24,
    ACTION_REMOVECLIP              = 0x25,
    ACTION_TRACE                   = 0x26,
    ACTION_STARTDRAGMOVIE          = 0x27,
    ACTION_STOPDRAGMOVIE           = 0x28,
    ACTION_STRINGCOMPARE           = 0x29,
    ACTION_THROW                   = 0x2A,

    /// SWF7
    ///
    /// The Cast Object action makes sure that the object
    /// o1 is an instance of the class s2. If it is the case,
    /// then o1 is pushed back onto the stack. Otherwise Null is
    /// pushed back onto the stack. The comparison is identical
    /// to the one applied by the Instance Of  action.
    ///
    /// See:
    /// http://sswf.sourceforge.net/SWFalexref.html#action_cast_object
    ///
    ACTION_CASTOP                  = 0x2B,

    /// SWF7
    ///
    /// This action declares an object as a sub-class of
    /// one or more interfaces. The number of interfaces has to
    /// be indicated by i2. An interface is referenced by its
    /// name (which happens to be the same as the constructor
    /// function name.)
    ///
    /// See:
    /// http://sswf.sourceforge.net/SWFalexref.html#action_implements
    ///
    ACTION_IMPLEMENTSOP            = 0x2C,

    ACTION_RANDOM                  = 0x30,
    ACTION_MBLENGTH                = 0x31,
    ACTION_ORD                     = 0x32,
    ACTION_CHR                     = 0x33,
    ACTION_GETTIMER                = 0x34,
    ACTION_MBSUBSTRING             = 0x35,
    ACTION_MBORD                   = 0x36,
    ACTION_MBCHR                   = 0x37,
    ACTION_WAITFORFRAMEEXPRESSION  = 0x8D,
    ACTION_PUSHDATA                = 0x96,
    ACTION_BRANCHALWAYS            = 0x99,
    ACTION_GETURL2                 = 0x9A,
    ACTION_BRANCHIFTRUE            = 0x9D,
    ACTION_CALLFRAME               = 0x9E,
    ACTION_GOTOEXPRESSION          = 0x9F,

    /// http://sswf.sourceforge.net/SWFalexref.html#action_delete
    ACTION_DELETE                  = 0x3A,

    /// http://sswf.sourceforge.net/SWFalexref.html#action_delete_all
    /// The information in SWFalexref is pretty confusing, not sure it is correct...
    ACTION_DELETE2                 = 0x3B,

    ACTION_VAREQUALS               = 0x3C, // DEFINELOCAL actually
    ACTION_CALLFUNCTION            = 0x3D,
    ACTION_RETURN                  = 0x3E,
    ACTION_MODULO                  = 0x3F,

    /// SWF5
    ///
    /// Pop the number of arguments. Pop each argument.
    /// Create an object of class s1. Call the
    /// constructor function (which has the same name as
    /// the object class: s1). The result of the
    /// constructor is discarded. Push the created object
    /// on the stack. The object should then be saved in
    /// a variable or object method.
    ///
    /// http://sswf.sourceforge.net/SWFalexref.html#action_new
    ///
    ACTION_NEW                     = 0x40,

    ACTION_VAR                     = 0x41,
    ACTION_INITARRAY               = 0x42,


    /// SWF5
    ///
    /// Pops the number of members in the object. Pop
    /// one value and one name per member and set the
    /// corresponding member in the object. The resulting
    /// object is pushed on the stack. It can later be sent
    /// to a function or set in a variable. Note: the member
    /// names are converted to strings; they certainly should
    /// be strings thought anything is supported.
    ///
    /// Also known as 'ACTION_DECLAREOBJECT'.
    ///
    /// http://sswf.sourceforge.net/SWFalexref.html#action_declare_object
    ///
    ACTION_INITOBJECT              = 0x43,

    ACTION_TYPEOF                  = 0x44,

    /// SWF5
    ///
    /// Pop a value from the stack. If it is a valid movieclip push
    /// it's target back on the stack (example: _level0.sprite1.sprite2).
    ///
    /// http://sswf.sourceforge.net/SWFalexref.html#action_get_target
    ///
    ACTION_TARGETPATH              = 0x45,

    /// SWF5
    ///
    /// http://sswf.sourceforge.net/SWFalexref.html#action_enumerate
    ///
    ACTION_ENUMERATE               = 0x46,

    /// SWF5
    ///
    /// http://sswf.sourceforge.net/SWFalexref.html#action_add_typed
    ///
    ACTION_NEWADD                  = 0x47,

    ACTION_NEWLESSTHAN             = 0x48,

    /// ECMA-262 "Abstract Equality Comparison"
    //
    /// See section 11.9.3 of the ECMA 262 spec
    ///
    ACTION_NEWEQUALS               = 0x49,

    /// SWF5
    ///
    /// http://sswf.sourceforge.net/SWFalexref.html#action_number
    ///
    ACTION_TONUMBER                = 0x4A,

    /// SWF5
    ///
    /// http://sswf.sourceforge.net/SWFalexref.html#action_string
    ///
    ACTION_TOSTRING                = 0x4B,

    /// SWF5
    ///
    /// http://sswf.sourceforge.net/SWFalexref.html#action_duplicate
    ///
    ACTION_DUP                     = 0x4C,

    /// SWF5
    ///
    /// http://sswf.sourceforge.net/SWFalexref.html#action_swap
    ///
    ACTION_SWAP                    = 0x4D,

    /// SWF5
    ///
    /// http://sswf.sourceforge.net/SWFalexref.html#action_get_member
    ///
    ACTION_GETMEMBER               = 0x4E,

    /// SWF5
    ///
    /// http://sswf.sourceforge.net/SWFalexref.html#action_set_member
    ///
    ACTION_SETMEMBER               = 0x4F,

    /// SWF5
    ///
    /// http://sswf.sourceforge.net/SWFalexref.html#action_increment
    ///
    ACTION_INCREMENT               = 0x50,

    /// SWF5
    ///
    /// http://sswf.sourceforge.net/SWFalexref.html#action_decrement
    ///
    ACTION_DECREMENT               = 0x51,

    /// SWF5
    ///
    /// Pops the name of a method (can be the empty string),
    /// pop an object, pop the number of arguments, pop each
    /// argument, call the method (function) of the object,
    /// push the returned value on the stack.
    ///
    /// http://sswf.sourceforge.net/SWFalexref.html#action_call_method
    ///
    ACTION_CALLMETHOD              = 0x52,

    /// SWF5
    ///
    /// Pops the name of a method (can be the empty string),
    /// pop an object (created with the Declare Object,)
    /// pop the number of arguments, pop each argument,
    /// create a new object, then call the specified method
    /// (function) as the constructor function of the object,
    /// push the returned value on the stack. This allows
    /// for overloaded constructors as in C++.
    ///
    /// http://sswf.sourceforge.net/SWFalexref.html#action_new_method
    ///
    ACTION_NEWMETHOD               = 0x53, 

    /// SWF6
    ///
    /// Pops the name of a constructor (s1 - ie. "Color")
    /// then the name of an object (s2). Checks whether the
    /// named object is part of the class defined by the
    /// constructor. If so, then true is push on the stack,
    /// otherwise false. Since SWF version 7, it is possible
    /// to cast an object to another using the Cast Object
    /// action. This action returns a copy of the object or
    /// Null, which in many cases can be much more practical.
    ///
    /// http://sswf.sourceforge.net/SWFalexref.html#action_instance_of
    ///
    ACTION_INSTANCEOF              = 0x54,

    /// SWF6
    ///
    /// Pops an object from the stack, push a null, then
    /// push the name of each member on the stack.
    ///
    /// http://sswf.sourceforge.net/SWFalexref.html#action_enumerate_object
    ///
    ACTION_ENUM2                   = 0x55,

    ACTION_BITWISEAND              = 0x60,
    ACTION_BITWISEOR               = 0x61,
    ACTION_BITWISEXOR              = 0x62,
    ACTION_SHIFTLEFT               = 0x63,
    ACTION_SHIFTRIGHT              = 0x64,
    ACTION_SHIFTRIGHT2             = 0x65,
    ACTION_STRICTEQ                = 0x66,

    /// SWF6
    ///
    /// Similar to Swap + Less Than. It checks whether the
    /// second parameter is greater than the first and return
    /// the boolean result on the stack.
    ///
    /// See:
    /// http://sswf.sourceforge.net/SWFalexref.html#action_greater_than_typed
    ///
    ACTION_GREATER                 = 0x67,

    /// SWF6
    ///
    /// Similar to Swap + String Less Than. It checks whether
    /// the second string is greater than the first and
    /// return the boolean result on the stack.
    ///
    /// See:
    /// http://sswf.sourceforge.net/SWFalexref.html#action_string_greater_than
    ///
    ACTION_STRINGGREATER           = 0x68,

    ///
    /// SWF7
    ///
    /// The Extends action will be used to define a new object
    /// which extends another object. The declaration in
    /// ActionScript is:
    ///
    ///	class A extends B;
    ///
    /// In an SWF action script, you don't exactly declare
    /// objects, you actually instantiate them and define their
    /// functions. This action creates a new object named s2
    /// which is an extension of the object s1.
    ///
    /// Use this action whenever you need to inherit an object
    /// without calling its constructor.
    ///
    /// See:
    /// http://sswf.sourceforge.net/SWFalexref.html#action_extends
    ///
    ACTION_EXTENDS                 = 0x69,

    /// See:
    /// http://sswf.sourceforge.net/SWFalexref.html#action_declare_dictionary
    ACTION_CONSTANTPOOL            = 0x88,


    /// SWF7
    ///
    /// See:
    /// http://sswf.sourceforge.net/SWFalexref.html#action_declare_function2
    ///
    ACTION_DEFINEFUNCTION2         = 0x8E,

    ACTION_TRY                     = 0x8F,
    ACTION_WITH                    = 0x94,
    ACTION_DEFINEFUNCTION          = 0x9B,

    ACTION_SETREGISTER             = 0x87
} action_type;

/// SWF fill style types. Symbolic names copied from Ming.
//
/// For more info see:
/// http://sswf.sourceforge.net/SWFalexref.html#swf_fill_style
///
typedef enum {
	FILL_SOLID                   = 0x00,
	FILL_LINEAR_GRADIENT         = 0x10,
	FILL_RADIAL_GRADIENT         = 0x12,
	FILL_TILED_BITMAP            = 0x40,
	FILL_CLIPPED_BITMAP          = 0x41,

	/// swf7, hard edges
	FILL_TILED_BITMAP_HARD       = 0x42, 

	/// swf7, hard edges
	FILL_CLIPPED_BITMAP_HARD     = 0x43

} fill_style_type;


} // namespace gnash::SWF

} // namespace gnash


#endif // GNASH_SWF_H
