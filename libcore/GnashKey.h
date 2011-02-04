// GnashKey.h: map sets of keycodes to each other.
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

#ifndef GNASHKEY_H
#define GNASHKEY_H

namespace gnash {

namespace key {

/// Conflicts with mingw header with gcc3.x (or whatever
/// antiquated version mingw has).
#ifdef _WIN32
# undef REGISTERED
#endif

enum modifier
{
    GNASH_MOD_NONE = 0,
    GNASH_MOD_SHIFT = 1,
    GNASH_MOD_CONTROL = 2,
    GNASH_MOD_ALT = 4
};

// Gnash DisplayObject codes. Each code represents a single DisplayObject on the keyboard.
// The first 128 code are ordered by their correspond ASCII value.
enum code
{
  INVALID = 0,
  UNKNOWN1,
  UNKNOWN2,
  UNKNOWN3,
  UNKNOWN4,
  UNKNOWN5,
  UNKNOWN6,
  UNKNOWN7,
  BACKSPACE = 8,
  TAB = 9,
  UNKNOWN10,
  UNKNOWN11,
  CLEAR = 12,
  ENTER = 13,
  UNKNOWN14,
  UNKNOWN15,
  SHIFT = 16,
  CONTROL = 17,
  ALT = 18,
  PAUSE = 19,
  CAPSLOCK = 20,
  UNKNOWN21,
  UNKNOWN22,
  UNKNOWN23,
  UNKNOWN24,
  UNKNOWN25,  
  UNKNOWN26,
  ESCAPE = 27,
  UNKNOWN28,
  UNKNOWN29,
  UNKNOWN30,
  UNKNOWN31,
  SPACE = 32,
  EXCLAM = 33,
  DOUBLE_QUOTE = 34,
  HASH = 35,
  DOLLAR = 36,
  PERCENT = 37,
  AMPERSAND = 38 ,
  SINGLE_QUOTE  = 39,
  PAREN_LEFT = 40,
  PAREN_RIGHT = 41,
  ASTERISK = 42,
  PLUS = 43,
  COMMA = 44,
  MINUS = 45,
  PERIOD = 46,
  SLASH =47,

  _0 = 48,
  _1,
  _2,
  _3,
  _4,
  _5,
  _6,
  _7,
  _8,
  _9 = 57,
  
  COLON = 58,
  SEMICOLON = 59,
  LESS = 60,
  EQUALS = 61,
  MORE = 62,
  QUESTION = 63,
  AT = 64,
  
  A = 65,
  B,
  C,
  D,
  E,
  F,
  G,
  H,
  I,
  J,
  K,
  L,
  M,
  N,
  O,
  P,
  Q,
  R,
  S,
  T,
  U,
  V,
  W,
  X,
  Y,
  Z = 90,
  
  LEFT_BRACKET =  91,
  BACKSLASH =92,
  RIGHT_BRACKET = 93,
  CARET = 94,
  UNDERSCORE = 95,
  BACKQUOTE = 96,
  
  a = 97,
  b,
  c,
  d,
  e,
  f,
  g,
  h,
  i,
  j,
  k,
  l,
  m,
  n,
  o,
  p,
  q,
  r,
  s,
  t,
  u,
  v,
  w,
  x,
  y,
  z = 122,
  LEFT_BRACE = 123,
  PIPE = 124,
  RIGHT_BRACE = 125,
  ASCIITILDE = 126,
  DELETEKEY = 127,
  
  KP_0 = 128,
  KP_1,
  KP_2,
  KP_3,
  KP_4,
  KP_5,
  KP_6,
  KP_7,
  KP_8,
  KP_9 = 137,
      
  F1 = 138,
  F2,
  F3,
  F4,
  F5,
  F6,
  F7,
  F8,
  F9,
  F10,
  F11,
  F12,
  F13,
  F14,
  F15 = 152,
  
  UP = 153,
  DOWN = 154,
  RIGHT = 155,
  LEFT = 156,
  INSERT = 157,
  HOME = 158,
  END  = 159,
  PGUP = 160,
  PGDN = 161,

  KP_ADD = 162,
  KP_SUBTRACT = 163,
  KP_MULTIPLY = 164,
  KP_DIVIDE = 165,
  KP_DECIMAL = 166,
  KP_ENTER = 167,
  
  NUM_LOCK = 168,

// Extended ASCII

  NOBREAKSPACE = 169,
  EXCLAM_DOWN = 170,
  CENT = 171,
  STERLING = 172,
  CURRENCY = 173,
  YEN = 174,
  BROKENBAR = 175,
  SECTION = 176,
  DIAERESIS = 177,
  COPYRIGHT = 178,
  ORDFEMININE = 179,
  GUILLEMOTLEFT = 180,
  NOTSIGN = 181,
  HYPHEN = 182,
  REGISTERED = 183,
  MACRON = 184,
  DEGREE = 185,
  PLUSMINUS = 186,
  TWOSUPERIOR = 187,
  THREESUPERIOR = 188,
  ACUTE = 189,
  MU = 190,
  PARAGRAPH = 191,
  PERIODCENTRED = 192,
  CEDILLA = 193,
  ONESUPERIOR = 194,
  MASCULINE = 195,
  GUILLEMOTRIGHT = 196,
  ONEQUARTER = 197,
  ONEHALF = 198,
  THREEQUARTERS = 199,
  QUESTIONDOWN = 200,
  AGRAVE = 201,
  AACUTE = 202, 
  ACIRCUMFLEX = 203,
  ATILDE = 204,
  ADIAERESIS = 205,
  ARING = 206,
  AE = 207,
  CCEDILLA = 208,
  EGRAVE = 209,
  EACUTE = 210,
  ECIRCUMFLEX = 211,
  EDIAERESIS = 212,
  IGRAVE = 213,
  IACUTE = 214,
  ICIRCUMFLEX = 215,
  IDIAERESIS = 216,
  ETH = 217,
  NTILDE = 218,
  OGRAVE = 219,
  OACUTE = 220,
  OCIRCUMFLEX = 221,
  OTILDE = 222,
  ODIAERESIS = 223,
  MULTIPLY = 224,
  OSLASH = 225,
  UGRAVE = 226,
  UACUTE = 227,
  UCIRCUMFLEX = 228,
  UDIAERESIS = 229,
  YACUTE = 230,
  THORN = 231,
  sSHARP = 232,
  aGRAVE = 233,
  aACUTE = 234,
  aCIRCUMFLEX = 235,
  aTILDE = 236,
  aDIAERESIS = 237,
  aRING = 238,
  ae = 239,
  cCEDILLA = 240,
  eGRAVE = 241,
  eACUTE = 242,
  eCIRCUMFLEX = 243,
  eDIAERESIS = 244,
  iGRAVE = 245,
  iACUTE = 246,
  iCIRCUMFLEX = 247,
  iDIAERESIS = 248,
  eth = 249,
  nTILDE = 250,
  oGRAVE = 251,
  oACUTE = 252,
  oCIRCUMFLEX = 253,
  oTILDE = 254,
  oDIAERESIS = 255,
  DIVISION = 256,
  oSLASH = 257,
  uGRAVE = 258,
  uACUTE = 259,
  uCIRCUMFLEX = 260,
  uDIAERESIS = 261,
  yACUTE = 262,
  thorn = 263,
  yDIAERESIS = 264,
  
  // TODO: add other function keys and DisplayObjects not yet known.
  HELP = 265,
  KEYCOUNT
};

enum type
{
	SWF,
	KEY,
	ASCII,
	TYPES
};

const unsigned char codeMap[KEYCOUNT][TYPES] = {
//{swfKeyCode, keycode, asciiKeyCode}
  {0,   0,   0}, // INVALID = 0
  {0,   0,   0}, // UNKNOWN1
  {0,   0,   0}, // UNKNOWN2
  {0,   0,   0}, // UNKNOWN3
  {0,   0,   0}, // UNKNOWN4
  {0,   0,   0}, // UNKNOWN5
  {0,   0,   0}, // UNKNOWN6
  {0,   0,   0}, // UNKNOWN7
  {8,   8,   8}, // BACKSPACE = 8
  {18,  9,   9}, // TAB = 9
  {0,   0,   0}, // UNKNOWN10
  {0,   0,   0}, // UNKNOWN11
  {0,  12,   0}, // CLEAR = 12
  {13, 13,  13}, // ENTER = 13
  {0,   0,   0}, // UNKNOWN14
  {0,   0,   0}, // UNKNOWN15
  {0,  16,   0}, // SHIFT = 16
  {0,  17,   0}, // CONTROL = 17
  {0,  18,   0}, // ALT = 18
  {0,  19,   0}, // PAUSE = 19
  {0,  20,   0}, // CAPSLOCK = 20
  {0,   0,   0}, // UNKNOWN21
  {0,   0,   0}, // UNKNOWN22
  {0,   0,   0}, // UNKNOWN23
  {0,   0,   0}, // UNKNOWN24
  {0,   0,   0}, // UNKNOWN25 
  {0,   0,   0}, // UNKNOWN26
  {19, 27,  27}, // ESCAPE = 27
  {0,   0,   0}, // UNKNOWN28
  {0,   0,   0}, // UNKNOWN29
  {0,   0,   0}, // UNKNOWN30
  {0,   0,   0}, // UNKNOWN31
  {32, 32,  32}, // SPACE = 32
  {33, 49,  33}, // EXCLAM = 33
  {34, 222,  34}, // DOUBLE_QUOTE = 34
  {35, 51,  35}, // HASH = 35
  {36, 52,  36}, // DOLLAR = 36
  {37, 53,  37}, // PERCENT = 37
  {38, 55,  38}, // AMPERSAND = 38 
  {39, 222,  39}, // SINGLE_QUOTE  = 39
  {40, 57,  40}, // PAREN_LEFT = 40
  {41, 48,  41}, // PAREN_RIGHT = 41
  {42, 56,  42}, // ASTERISK = 42
  {43, 187,  43}, // PLUS = 43
  {44, 188,  44}, // COMMA = 44
  {45, 189,  45}, // MINUS = 45
  {46, 190,  46}, // PERIOD = 46
  {47, 191,  47}, // SLASH = 47
  {48, 48,  48}, // 0 = 48
  {49, 49,  49}, // 1
  {50, 50,  50}, // 2
  {51, 51,  51}, // 3
  {52, 52,  52}, // 4
  {53, 53,  53}, // 5
  {54, 54,  54}, // 6
  {55, 55,  55}, // 7
  {56, 56,  56}, // 8
  {57, 57,  57}, // 9 = 57
  {58, 186,  58}, // COLON = 58
  {59, 186,  59}, // SEMICOLON = 59
  {60, 188,  60}, // LESS = 60
  {61, 187,  61}, // EQUALS = 61
  {62, 190,  62}, // MORE = 62
  {63, 191,  63}, // QUESTION = 63
  {64, 50,  64}, // AT = 64
  {65, 65,  65}, // A = 65
  {66, 66,  66}, // B
  {67, 67,  67}, // C
  {68, 68,  68}, // D
  {69, 69,  69}, // E
  {70, 70,  70}, // F
  {71, 71,  71}, // G
  {72, 72,  72}, // H
  {73, 73,  73}, // I
  {74, 74,  74}, // J
  {75, 75,  75}, // K
  {76, 76,  76}, // L
  {77, 77,  77}, // M
  {78, 78,  78}, // N
  {79, 79,  79}, // O
  {80, 80,  80}, // P
  {81, 81,  81}, // Q
  {82, 82,  82}, // R
  {83, 83,  83}, // S
  {84, 84,  84}, // T
  {85, 85,  85}, // U
  {86, 86,  86}, // V
  {87, 87,  87}, // W
  {88, 88,  88}, // X
  {89, 89,  89}, // Y
  {90, 90,  90}, // Z = 90
  {91, 219,  91}, // LEFT_BRACKET = 91
  {92, 220,  92}, // BACKSLASH = 92
  {93, 221,  93}, // RIGHT_BRACKET = 93
  {94, 54,  94}, // CARET = 94
  {95, 189,  95}, // UNDERSCORE = 95
  {96, 192,  96}, // BACKQUOTE = 96
  {97, 65,  97}, // a = 97
  {98, 66,  98}, // b
  {99, 67,  99}, // c
  {100,68, 100}, // d
  {101,69, 101}, // e
  {102,70, 102}, // f
  {103,71, 103}, // g
  {104,72, 104}, // h
  {105,73, 105}, // i
  {106,74, 106}, // j
  {107,75, 107}, // k
  {108,76, 108}, // l
  {109,77, 109}, // m
  {110,78, 110}, // n
  {111,79, 111}, // o
  {112,80, 112}, // p
  {113,81, 113}, // q
  {114,82, 114}, // r
  {115,83, 115}, // s
  {116,84, 116}, // t
  {117,85, 117}, // u
  {118,86, 118}, // v
  {119,87, 119}, // w
  {120,88, 120}, // x
  {121,89, 121}, // y
  {122,90, 122}, // z = 122
  {123,219,123}, // LEFT_BRACE = 123
  {124,220,124}, // PIPE = 124
  {125,221,125}, // RIGHT_BRACE = 125
  {126,192,126}, // ASCIITILDE = 126
  {6,  46, 127}, // DELETE = 127
  {48, 96,  48}, // KP_0 = 128
  {49, 97,  49}, // KP_1
  {50, 98,  50}, // KP_2
  {51, 99,  51}, // KP_3
  {52, 100, 52}, // KP_4
  {53, 101, 53}, // KP_5
  {54, 102, 54}, // KP_6
  {55, 103, 55}, // KP_7
  {56, 104, 56}, // KP_8
  {57, 105, 57}, // KP_9
  {0,  112,  0}, // F1 = 138
  {0,  113,  0}, // F2
  {0,  114,  0}, // F3
  {0,  115,  0}, // F4
  {0,  116,  0}, // F5
  {0,  117,  0}, // F6
  {0,  118,  0}, // F7
  {0,  119,  0}, // F8
  {0,  120,  0}, // F9
  {0,  121,  0}, // F10
  {0,  122,  0}, // F11
  {0,  123,  0}, // F12
  {0,  124,  0}, // F13
  {0,  125,  0}, // F14
  {0,  126,  0}, // F15 = 152
  {14, 38,   0}, // UP = 153
  {15, 40,   0}, // DOWN = 154
  {2,  39,   0}, // RIGHT = 155
  {1,  37,   0}, // LEFT = 156
  {5,  45,   0}, // INSERT = 157
  {3,  36,   0}, // HOME = 158
  {4,  35,   0}, // END = 159
  {16, 33,   0}, // PAGEUP = 160
  {17, 34,   0}, // PAGEDOWN = 161
  {0, 107,  43}, // KP_ADD = 162
  {0, 109,  45}, // KP_SUBTRACT = 163
  {0, 106,  42}, // KP_MULITPLY = 164
  {0, 111,  47}, // KP_DIVIDE = 165
  {0, 110,  46}, // KP_DECIMAL = 166
  {13, 13,  13}, // KP_ENTER = 167
  {0, 144,   0}, // NUMLOCK = 168
  {0, 160,   160}, // NOBREAKSPACE = 169
  {0, 161,   161}, //   EXCLAM_DOWN = 170
  {0, 162,   162}, //   CENT = 171
  {0, 163,   163}, //   STERLING = 172
  {0, 164,   164}, //   CURRENCY = 173
  {0, 165,   165}, //   YEN = 174
  {0, 166,   166}, //   BROKENBAR = 175
  {0, 167,   167}, //   SECTION = 176
  {0, 168,   168}, //   DIAERESIS = 177
  {0, 169,   169}, //   COPYRIGHT = 178
  {0, 170,   170}, //   ORDFEMININE = 179
  {0, 171,   171}, //   GUILLEMOTLEFT = 180
  {0, 172,   172}, //   NOTSIGN = 181
  {0, 173,   173}, //   HYPHEN = 182
  {0, 174,   174}, //   REGISTERED = 183
  {0, 175,   175}, //   MACRON = 184
  {0, 176,   176}, //   DEGREE = 185
  {0, 177,   177}, //   PLUSMINUS = 186
  {0, 178,   178}, //   TWOSUPERIOR = 187
  {0, 179,   179}, //   THREESUPERIOR = 188
  {0, 180,   180}, //   ACUTE = 189
  {0, 181,   181}, //   MU = 190
  {0, 182,   182}, //   PARAGRAPH = 191
  {0, 183,   183}, //   PERIODCENTRED = 192
  {0, 184,   184}, //   CEDILLA = 193
  {0, 185,   185}, //   ONESUPERIOR = 194
  {0, 186,   186}, //   MASCULINE = 195
  {0, 187,   187}, //   GUILLEMOTRIGHT = 196
  {0, 188,   188}, //   ONEQUARTER = 197
  {0, 189,   189}, //   ONEHALF = 198
  {0, 190,   190}, //   THREEQUARTERS = 199
  {0, 191,   191}, //   QUESTIONDOWN = 200
  {0, 192,   192}, //   AGRAVE = 201
  {0, 193,   193}, //   AACUTE = 202
  {0, 194,   194}, //   ACIRCUMFLEX = 203
  {0, 195,   195}, //   ATILDE = 204
  {0, 196,   196}, //   ADIAERISIS = 205
  {0, 197,   197}, //   ARING = 206
  {0, 198,   198}, //   AE = 207
  {0, 199,   199}, //   CCEDILLA = 208
  {0, 200,   200}, //   EGRAVE = 209
  {0, 201,   201}, //   EACUTE = 210
  {0, 202,   202}, //   ECIRCUMFLEX = 211
  {0, 203,   203}, //   EDIAERESIS = 212
  {0, 204,   204}, //   IGRAVE = 213
  {0, 205,   205}, //   IACUTE = 214
  {0, 206,   206}, //   ICIRCUMFLEX = 215
  {0, 207,   207}, //   IDIAERESIS = 216
  {0, 208,   208}, //   ETH = 217
  {0, 209,   209}, //   NTILDE = 218
  {0, 210,   210}, //   OGRAVE = 219
  {0, 211,   211}, //   OACUTE = 220
  {0, 212,   212}, //   OCIRCUMFLEX = 221
  {0, 213,   213}, //   OTILDE = 222
  {0, 214,   214}, //   ODIAERESIS = 223
  {0, 215,   215}, //   MULTIPLY = 224
  {0, 216,   116}, //   OSLASH = 225
  {0, 217,   217}, //   UGRAVE = 226
  {0, 218,   218}, //   UACTUE = 227
  {0, 219,   219}, //   UCIRCUMFLEX = 228
  {0, 220,   220}, //   UDIAERESIS = 229
  {0, 221,   221}, //   ZACUTE = 230
  {0, 222,   222}, //   THORN = 231
  {0, 223,   223}, //   sSHARP = 232
  {0, 224,   224}, //   aTILDE = 233
  {0, 225,   225}, //   aACUTE = 234
  {0, 226,   226}, //   aCIRCUMFLEX = 235
  {0, 227,   227}, //   aTILDE = 236
  {0, 228,   228}, //   aDIAERESIS = 237
  {0, 229,   229}, //   aRING = 238
  {0, 230,   230}, //   ae = 239
  {0, 231,   231}, //   cCEDILLA = 240
  {0, 232,   232}, //   eGRAVE = 241
  {0, 233,   233}, //   eACUTE = 242
  {0, 234,   234}, //   eCIRCUMFLEX = 243
  {0, 235,   235}, //   eDIAERESIS = 244
  {0, 236,   236}, //   iGRAVE = 245
  {0, 237,   237}, //   iACUTE = 246
  {0, 238,   238}, //   iCIRCUMFLEX = 247
  {0, 239,   239}, //   iDIAERESIS = 248
  {0, 240,   240}, //   eth = 249
  {0, 241,   241}, //   nTILDE = 250
  {0, 242,   242}, //   oTILDE = 251
  {0, 243,   243}, //   oACUTE = 252
  {0, 244,   244}, //   oCIRCUMFLEX = 253
  {0, 245,   245}, //   oTILDE = 254
  {0, 246,   246}, //   oDIAERESIS = 255
  {0, 247,   247}, //   DIVISION = 256
  {0, 248,   248}, //   oSLASH = 257
  {0, 249,   249}, //   uGRAVE = 258
  {0, 250,   250}, //   uACUTE = 259
  {0, 251,   251}, //   uCIRCUMFLEX = 260 
  {0, 252,   252}, //   uDIAERESIS = 261
  {0, 253,   253}, //   yACUTE = 262
  {0, 254,   254}, //   thorn = 263
  {0, 255,   255}, //   yDIAERESIS = 264
  {0, 0,     0},   //   HELP (untested)
};

}   // end namespace key

} // end namespace gnash

#endif // GNASHKEY_H
