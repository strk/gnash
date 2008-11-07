// SoundInfo.cpp: parse and store a SoundInfo record.
//
//   Copyright (C) 2006, 2007, 2008 Free Software Foundation, Inc.
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
SoundInfo::read(SWFStream& in)
{
	in.ensureBytes(1);
	m_in_point = m_out_point = m_loop_count = 0;
    
    // highest 2 bits are reserved(unused).
    int flags = in.read_u8();
	m_stop_playback = flags & (1 << 5); 
	m_no_multiple   = flags & (1 << 4); 
	m_has_envelope  = flags & (1 << 3); 
	m_has_loops     = flags & (1 << 2);  
	m_has_out_point = flags & (1 << 1); 
	m_has_in_point  = flags & (1 << 0);  
    
	if (m_has_in_point)
	{
		in.ensureBytes(4);
		m_in_point = in.read_u32();
	}
	if (m_has_out_point)
	{
		in.ensureBytes(4);
		m_out_point = in.read_u32();
	}
	if (m_has_loops)
	{
		in.ensureBytes(2);
		m_loop_count = in.read_u16();
	}
	if (m_has_envelope)
	{
		in.ensureBytes(1);
		int nPoints = in.read_u8();
		m_envelopes.resize(nPoints);
		in.ensureBytes(8 * nPoints);
		for (int i=0; i < nPoints; i++)
		{
			m_envelopes[i].m_mark44 = in.read_u32();
			m_envelopes[i].m_level0 = in.read_u16();
			m_envelopes[i].m_level1 = in.read_u16();
		}
	}
	else
	{
		m_envelopes.resize(0);
	}


	IF_VERBOSE_PARSE(
	log_parse("	has_envelope = %d", m_has_envelope);
	log_parse("	has_loops = %d", m_has_loops);
	log_parse("	has_out_point = %d", m_has_out_point);
	log_parse("	has_in_point = %d", m_has_in_point);
	log_parse("	in_point = %d", m_in_point);
	log_parse("	out_point = %d", m_out_point);
	log_parse("	loop_count = %d", m_loop_count);
	log_parse("	envelope size = %d", m_envelopes.size());
	);
}

}
}

