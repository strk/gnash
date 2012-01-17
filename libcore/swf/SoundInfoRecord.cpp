// SoundInfo.cpp: parse and store a SoundInfo record.
//
//   Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012
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


#include "SoundInfoRecord.h"
#include "sound_handler.h" // sound::SoundEnvelopes
#include "SWFStream.h"
#include "log.h"

namespace gnash {
namespace SWF {


void
SoundInfoRecord::read(SWFStream& in)
{
	in.ensureBytes(1);
    
    // highest 2 bits are reserved(unused).
    int flags = in.read_u8();
	stopPlayback = flags & (1 << 5); 
	noMultiple   = flags & (1 << 4); 
	hasEnvelope  = flags & (1 << 3); 
	hasLoops     = flags & (1 << 2);  
	hasOutPoint = flags & (1 << 1); 
	hasInPoint  = flags & (1 << 0);  
    
    in.ensureBytes(hasInPoint * 4 + hasOutPoint * 4 + hasLoops * 2);

	if (hasInPoint) {
		inPoint = in.read_u32();
	}

	if (hasOutPoint) {
		outPoint = in.read_u32();
	}

	if (hasLoops) {
		loopCount = in.read_u16();
	}

	if (hasEnvelope) {
		in.ensureBytes(1);
		int nPoints = in.read_u8();
		envelopes.resize(nPoints);
		in.ensureBytes(8 * nPoints);
		for (int i=0; i < nPoints; i++)
		{
			envelopes[i].m_mark44 = in.read_u32();
			envelopes[i].m_level0 = in.read_u16();
			envelopes[i].m_level1 = in.read_u16();
		}
	}
	else {
		envelopes.clear();
	}

	IF_VERBOSE_PARSE(
        log_parse("	hasEnvelope = %d", hasEnvelope);
        log_parse("	hasLoops = %d", hasLoops);
        log_parse("	hasOutPoint = %d", hasOutPoint);
        log_parse("	hasInPoint = %d", hasInPoint);
        log_parse("	inPoint = %d", inPoint);
        log_parse("	outPoint = %d", outPoint);
        log_parse("	loopCount = %d", loopCount);
        log_parse("	envelope size = %d", envelopes.size());
	);
}

}
}

