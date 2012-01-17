// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "CodeStream.h"
#include "log.h"

#include <iostream>
#include <sstream>
#include <cassert>
#include <cmath>
#include <string>

#include "check.h"

#include "utility.h"

using namespace gnash;
using std::cout;
using std::endl;

int
main(int /*argc*/, char** /*argv*/)
{
	char data[10] = {0x4,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9};
	
	CodeStream* stream = new CodeStream(std::string(data,10));
	
	//Test read_as30p()
	boost::uint8_t opcode;
	int i = 0;
	while(opcode = stream->read_as3op()){
		check_equals(opcode,data[i]);
		i++;
	}
	
	//Make sure we stopped at the right spot.
	check_equals(i,10);

	//Reset stream.
	stream->seekTo(0);
	stream->clear();

	//Test seekTo
	stream->seekTo(5);
	
	opcode = stream->read_as3op();
	check_equals(opcode,data[5]);

	//Reset stream.
	stream->seekTo(0);
	stream->clear();	

	//Test read_u8.
	i=0;
	while(i<10){
		opcode = stream->read_u8();
		check_equals(opcode,data[i]);
		i++;
	}
	
	char newData[6] = {0x5,0xC5,0x0,0x0,0x1,0x2}; 
	CodeStream* streamA = new CodeStream(std::string(newData,6));
	
	boost::uint8_t byteA = streamA->read_u8(); 
	check_equals(byteA,newData[0]);
	
	//Test read_S24.
	boost::int32_t byteB = streamA->read_S24();
	check_equals(byteB,197);

	
	

}
