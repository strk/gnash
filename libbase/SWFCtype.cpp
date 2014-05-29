// SWFCtype.cpp: Case conversion code for SWF strings.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#include "SWFCtype.h"

#include <locale>
#include <map>
#include <cstdint>

#include "log.h"

namespace gnash {

namespace {
    typedef std::map<std::uint16_t, std::uint16_t> CharMap;
    const CharMap& lowerMap();
    const CharMap& upperMap();
}

SWFCtype::char_type
SWFCtype::do_toupper(char_type c) const
{
    if (c >= 97 && c <= 122) return c - 32;
    if (c >= 224 && c <= 246) return c - 32;
    if (c >= 248 && c <= 254) return c - 32;
    if (c >= 941 && c <= 943) return c - 37;
    if (c >= 945 && c <= 961) return c - 32;
    if (c >= 963 && c <= 971) return c - 32;
    if (c >= 1072 && c <= 1103) return c - 32;
    if (c >= 1104 && c <= 1119) return c - 80;
    if (c >= 1377 && c <= 1414) return c - 48;
    if (c >= 7936 && c <= 7943) return c + 8;
    if (c >= 7952 && c <= 7957) return c + 8;
    if (c >= 7968 && c <= 7975) return c + 8;
    if (c >= 7984 && c <= 7991) return c + 8;
    if (c >= 8000 && c <= 8005) return c + 8;
    if (c >= 8032 && c <= 8039) return c + 8;
    if (c >= 8050 && c <= 8053) return c + 86;
    if (c >= 8064 && c <= 8071) return c + 8;
    if (c >= 8080 && c <= 8087) return c + 8;
    if (c >= 8096 && c <= 8103) return c + 8;
    if (c >= 8560 && c <= 8575) return c - 16;
    if (c >= 9424 && c <= 9449) return c - 26;
    if (c >= 65345 && c <= 65370) return c - 32;
    if (!(c & 1)) {
        if (c >= 314 && c <= 328) return c - 1;
        if (c >= 378 && c <= 382) return c - 1;
        if (c >= 436 && c <= 438) return c - 1;
        if (c >= 462 && c <= 476) return c - 1;
        if (c >= 1218 && c <= 1220) return c - 1;
    }
    else if (c & 1) {
        if (c >= 257 && c <= 303) return c - 1;
        if (c >= 307 && c <= 311) return c - 1;
        if (c >= 331 && c <= 375) return c - 1;
        if (c >= 387 && c <= 389) return c - 1;
        if (c >= 417 && c <= 421) return c - 1;
        if (c >= 479 && c <= 495) return c - 1;
        if (c >= 505 && c <= 543) return c - 1;
        if (c >= 547 && c <= 563) return c - 1;
        if (c >= 987 && c <= 1007) return c - 1;
        if (c >= 1121 && c <= 1153) return c - 1;
        if (c >= 1165 && c <= 1215) return c - 1;
        if (c >= 1233 && c <= 1269) return c - 1;
        if (c >= 7681 && c <= 7829) return c - 1;
        if (c >= 7841 && c <= 7929) return c - 1;
        if (c >= 8017 && c <= 8023) return c + 8;
    }

    // Search the remaining map.
    const CharMap& m = upperMap();
    const CharMap::const_iterator it = m.find(c);
    if (it == m.end()) return c;
    return it->second;
}

const SWFCtype::char_type*
SWFCtype::do_toupper(char_type* low, const char_type* high) const
{
    char_type* pos = low;
    while (pos != high) {
        *pos = do_toupper(*pos);
        ++pos;
    }
    return low;
}

SWFCtype::char_type
SWFCtype::do_tolower(char_type c) const
{
    if (c >= 65 && c <= 90) return c + 32;
    if (c >= 192 && c <= 214) return c + 32;
    if (c >= 216 && c <= 222) return c + 32;
    if (c >= 904 && c <= 906) return c + 37;
    if (c >= 913 && c <= 929) return c + 32;
    if (c >= 931 && c <= 939) return c + 32;
    if (c >= 1024 && c <= 1039) return c + 80;
    if (c >= 1040 && c <= 1071) return c + 32;
    if (c >= 1329 && c <= 1366) return c + 48;
    if (c >= 4256 && c <= 4293) return c + 48;
    if (c >= 7944 && c <= 7951) return c - 8;
    if (c >= 7960 && c <= 7965) return c - 8;
    if (c >= 7976 && c <= 7983) return c - 8;
    if (c >= 7992 && c <= 7999) return c - 8;
    if (c >= 8008 && c <= 8013) return c - 8;
    if (c >= 8040 && c <= 8047) return c - 8;
    if (c >= 8072 && c <= 8079) return c - 8;
    if (c >= 8088 && c <= 8095) return c - 8;
    if (c >= 8104 && c <= 8111) return c - 8;
    if (c >= 8136 && c <= 8139) return c - 86;
    if (c >= 8544 && c <= 8559) return c + 16;
    if (c >= 9398 && c <= 9423) return c + 26;
    if (c >= 65313 && c <= 65338) return c + 32;
    if (!(c & 1)) {
        if (c >= 256 && c <= 302) return c + 1;
        if (c >= 306 && c <= 310) return c + 1;
        if (c >= 330 && c <= 374) return c + 1;
        if (c >= 386 && c <= 388) return c + 1;
        if (c >= 416 && c <= 420) return c + 1;
        if (c >= 478 && c <= 494) return c + 1;
        if (c >= 498 && c <= 500) return c + 1;
        if (c >= 504 && c <= 542) return c + 1;
        if (c >= 546 && c <= 562) return c + 1;
        if (c >= 986 && c <= 1006) return c + 1;
        if (c >= 1120 && c <= 1152) return c + 1;
        if (c >= 1164 && c <= 1214) return c + 1;
        if (c >= 1232 && c <= 1268) return c + 1;
        if (c >= 7680 && c <= 7828) return c + 1;
        if (c >= 7840 && c <= 7928) return c + 1;
    }
    else if (c & 1) {
        if (c >= 313 && c <= 327) return c + 1;
        if (c >= 377 && c <= 381) return c + 1;
        if (c >= 435 && c <= 437) return c + 1;
        if (c >= 459 && c <= 475) return c + 1;
        if (c >= 1217 && c <= 1219) return c + 1;
        if (c >= 8025 && c <= 8031) return c - 8;
    }

    const CharMap& m = lowerMap();
    const CharMap::const_iterator it = m.find(c);
    if (it == m.end()) return c;
    return it->second;
}

const SWFCtype::char_type*
SWFCtype::do_tolower(char_type* low, const char_type* high) const
{
    char_type* pos = low;
    while (pos != high) {
        *pos = do_tolower(*pos);
        ++pos;
    }
    return low;
}

namespace {

const CharMap&
upperMap()
{
    static const CharMap upper = {
        {181, 924},
        {255, 376},
        {305, 73},
        {383, 83},
        {392, 391},
        {396, 395},
        {402, 401},
        {405, 502},
        {409, 408},
        {424, 423},
        {429, 428},
        {432, 431},
        {441, 440},
        {445, 444},
        {447, 503},
        {453, 452},
        {454, 452},
        {456, 455},
        {457, 455},
        {459, 458},
        {460, 458},
        {477, 398},
        {498, 497},
        {499, 497},
        {501, 500},
        {595, 385},
        {596, 390},
        {598, 393},
        {599, 394},
        {601, 399},
        {603, 400},
        {608, 403},
        {611, 404},
        {616, 407},
        {617, 406},
        {623, 412},
        {626, 413},
        {629, 415},
        {640, 422},
        {643, 425},
        {648, 430},
        {650, 433},
        {651, 434},
        {658, 439},
        {837, 921},
        {940, 902},
        {962, 931},
        {972, 908},
        {973, 910},
        {974, 911},
        {976, 914},
        {977, 920},
        {981, 934},
        {982, 928},
        {1008, 922},
        {1009, 929},
        {1010, 931},
        {1013, 917},
        {1224, 1223},
        {1228, 1227},
        {1273, 1272},
        {7835, 7776},
        {8048, 8122},
        {8049, 8123},
        {8054, 8154},
        {8055, 8155},
        {8056, 8184},
        {8057, 8185},
        {8058, 8170},
        {8059, 8171},
        {8060, 8186},
        {8061, 8187},
        {8112, 8120},
        {8113, 8121},
        {8115, 8124},
        {8126, 921},
        {8131, 8140},
        {8144, 8152},
        {8145, 8153},
        {8160, 8168},
        {8161, 8169},
        {8165, 8172},
        {8179, 8188}
    };

    return upper;
}

const CharMap&
lowerMap()
{
    static const CharMap lower = {
        {304, 105},
        {376, 255},
        {385, 595},
        {390, 596},
        {391, 392},
        {393, 598},
        {394, 599},
        {395, 396},
        {398, 477},
        {399, 601},
        {400, 603},
        {401, 402},
        {403, 608},
        {404, 611},
        {406, 617},
        {407, 616},
        {408, 409},
        {412, 623},
        {413, 626},
        {415, 629},
        {422, 640},
        {423, 424},
        {425, 643},
        {428, 429},
        {430, 648},
        {431, 432},
        {433, 650},
        {434, 651},
        {439, 658},
        {440, 441},
        {444, 445},
        {452, 454},
        {453, 454},
        {455, 457},
        {456, 457},
        {458, 460},
        {497, 499},
        {502, 405},
        {503, 447},
        {902, 940},
        {908, 972},
        {910, 973},
        {911, 974},
        {1012, 952},
        {1223, 1224},
        {1227, 1228},
        {1272, 1273},
        {8120, 8112},
        {8121, 8113},
        {8122, 8048},
        {8123, 8049},
        {8124, 8115},
        {8140, 8131},
        {8152, 8144},
        {8153, 8145},
        {8154, 8054},
        {8155, 8055},
        {8168, 8160},
        {8169, 8161},
        {8170, 8058},
        {8171, 8059},
        {8172, 8165},
        {8184, 8056},
        {8185, 8057},
        {8186, 8060},
        {8187, 8061},
        {8188, 8179},
        {8486, 969},
        {8490, 107},
        {8491, 229}
    };

    return lower;
}

} // unnamed namespace
} // namespace gnash
