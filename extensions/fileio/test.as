// 
//   Copyright (C) 2005, 2006, 2007, 2009, 2010 Free Software
//   Foundation, Inc
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

#include "dejagnu.as"

file = new FileIO();

// test stdio first

buf = "Hello World!\n";

if (file.fopen("/tmp/x", "w+")) {
    pass("FileIO::open()");
} else {
    fail("FileIO::open()");
}

if (file.fputs(buf)) {
    pass("FileIO::fputs()");
} else {
    fail("FileIO::fputs()");
}

file.fwrite(buf, 12);

xxx = file.ftell();
if (file.fseek(0) == 0) {
    pass("FileIO::fseek()");
} else {
    fail("FileIO::fseek()");
}
    
if (file.fflush() == 0) {
    pass("FileIO::fflush()");
} else {
    fail("FileIO::fflush()");
}
    
if ((xxx > 0) && (file.ftell() == 0)) {
    pass("FileIO::ftell()");
} else {
    fail("FileIO::ftell()");
}

x = file.fgetc();
if (x == "H") {
    pass("FileIO::fgetc()");
} else {
    fail("FileIO::fgetc()");
}
//trace(x);

y = file.fgets();
if (y == "ello World!\n") {
    pass("FileIO::fgets()");
} else {
    fail("FileIO::fgets()");
}

file.close();
//trace(y);

//file.read(buf, count);

// These tests use standard I/O
if (file.putchar('X')) {
    pass("FileIO::putchar()");
} else {
    fail("FileIO::putchar()");
}

if (file.puts(buf)) {
    pass("FileIO::puts()");
} else {
    fail("FileIO::puts()");
}


// We can't test thigns automatically if we need user input, so thes
// are commented out.

// a = file.getchar();
// if (a) {
//     pass("FileIO::getchar()");
// } else {
//     fail("FileIO::getchar()");
// }
// trace(a);

// b = file.gets();
// if (b) {
//     pass("FileIO::gets()");
// } else {
//     fail("FileIO::gets()");
// }
//trace(b);


totals();
