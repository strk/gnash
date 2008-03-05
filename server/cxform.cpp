// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "cxform.h"
#include "types.h" // for rgba type :(
#include "stream.h" // for reading from SWF
#include "log.h"
#include "utility.h" // for fclamp

using namespace std;

namespace gnash {

cxform	cxform::identity;

cxform::cxform()
// Initialize to identity transform.
{
	m_[0][0] = 1;
	m_[1][0] = 1;
	m_[2][0] = 1;
	m_[3][0] = 1;
	m_[0][1] = 0;
	m_[1][1] = 0;
	m_[2][1] = 0;
	m_[3][1] = 0;
}

void	cxform::concatenate(const cxform& c)
// Concatenate c's transform onto ours.  When
// transforming colors, c's transform is applied
// first, then ours.
{
	m_[0][1] += m_[0][0] * c.m_[0][1];
	m_[1][1] += m_[1][0] * c.m_[1][1];
	m_[2][1] += m_[2][0] * c.m_[2][1];
	m_[3][1] += m_[3][0] * c.m_[3][1];

	m_[0][0] *= c.m_[0][0];
	m_[1][0] *= c.m_[1][0];
	m_[2][0] *= c.m_[2][0];
	m_[3][0] *= c.m_[3][0];
}


rgba	cxform::transform(const rgba& in) const
// Apply our transform to the given color; return the result.
{
	rgba	result(in.m_r, in.m_g, in.m_b, in.m_a);
	
	transform(result.m_r, result.m_g, result.m_b, result.m_a);

	return result;
}

void	cxform::transform(boost::uint8_t& r, boost::uint8_t& g, boost::uint8_t& b, boost::uint8_t& a) const
// Faster transform() method for loops (avoids creation of rgba object)
{
	r = (boost::uint8_t) fclamp(r * m_[0][0] + m_[0][1], 0, 255);
	g = (boost::uint8_t) fclamp(g * m_[1][0] + m_[1][1], 0, 255);
	b = (boost::uint8_t) fclamp(b * m_[2][0] + m_[2][1], 0, 255);
	a = (boost::uint8_t) fclamp(a * m_[3][0] + m_[3][1], 0, 255);
}

void	cxform::read_rgb(stream& in)
{
	in.align();

	in.ensureBits(6);
	bool	has_add = in.read_bit();
	bool	has_mult = in.read_bit();
	int	nbits = in.read_uint(4);

	int reads = has_mult + has_add; // 0, 1 or 2
	if ( reads ) in.ensureBits(nbits*reads*3);

	if (has_mult) {
		m_[0][0] = in.read_sint(nbits) / 255.0f;
		m_[1][0] = in.read_sint(nbits) / 255.0f;
		m_[2][0] = in.read_sint(nbits) / 255.0f;
		m_[3][0] = 1;
	}
	else {
		for (int i = 0; i < 4; i++) { m_[i][0] = 1; }
	}
	if (has_add) {
		m_[0][1] = (float) in.read_sint(nbits);
		m_[1][1] = (float) in.read_sint(nbits);
		m_[2][1] = (float) in.read_sint(nbits);
		m_[3][1] = 1;
	}
	else {
		for (int i = 0; i < 4; i++) { m_[i][1] = 0; }
	}
}

void	cxform::read_rgba(stream& in)
{
	in.align();

	in.ensureBits(6);
	bool	has_add = in.read_bit();
	bool	has_mult = in.read_bit();
	int	nbits = in.read_uint(4);

	int reads = has_mult + has_add; // 0, 1 or 2
	if ( reads ) in.ensureBits(nbits*reads*4);

	if (has_mult) {
		m_[0][0] = in.read_sint(nbits) / 256.0f;
		m_[1][0] = in.read_sint(nbits) / 256.0f;
		m_[2][0] = in.read_sint(nbits) / 256.0f;
		m_[3][0] = in.read_sint(nbits) / 256.0f;
	}
	else {
		for (int i = 0; i < 4; i++) { m_[i][0] = 1; }
	}
	if (has_add) {
		m_[0][1] = (float) in.read_sint(nbits);
		m_[1][1] = (float) in.read_sint(nbits);
		m_[2][1] = (float) in.read_sint(nbits);
		m_[3][1] = (float) in.read_sint(nbits);
	}
	else {
		for (int i = 0; i < 4; i++) { m_[i][1] = 0; }
	}
}

/// Force component values to be in legal range.
void cxform::clamp()
{
	m_[0][0] = fclamp(m_[0][0], 0, 1);
	m_[1][0] = fclamp(m_[1][0], 0, 1);
	m_[2][0] = fclamp(m_[2][0], 0, 1);
	m_[3][0] = fclamp(m_[3][0], 0, 1);
	
	m_[0][1] = fclamp(m_[0][1], -255.0f, 255.0f);
	m_[1][1] = fclamp(m_[1][1], -255.0f, 255.0f);
	m_[2][1] = fclamp(m_[2][1], -255.0f, 255.0f);
	m_[3][1] = fclamp(m_[3][1], -255.0f, 255.0f);
}

void	cxform::print() const
// Debug log.
{
	log_parse("    *         +");
	log_parse("| %4.4f %4.4f|", m_[0][0], m_[0][1]);
	log_parse("| %4.4f %4.4f|", m_[1][0], m_[1][1]);
	log_parse("| %4.4f %4.4f|", m_[2][0], m_[2][1]);
	log_parse("| %4.4f %4.4f|", m_[3][0], m_[3][1]);
}

std::string
cxform::toString() const
{
	std::stringstream ss;
	ss << *this;
	return ss.str();
}

std::ostream&
operator<< (std::ostream& os, const cxform& cx) 
{
	os << "r: *" << cx.m_[0][0] << " +" << cx.m_[0][1] << ", ";
	os << "|g: *" << cx.m_[1][0] << " +" << cx.m_[1][1] << ", ";
	os << "|b: *" << cx.m_[2][0] << " +" << cx.m_[2][1] << ", ";
	os << "|a: *" << cx.m_[3][0] << " +" << cx.m_[3][1];
	return os;
}

bool	cxform::is_identity() const
// Returns true when the cxform equals identity (no transform)
{	   
  for (int a=0; a<4; a++)
   for (int b=0; b<2; b++)
    if (m_[a][b] != identity.m_[a][b])
     return false;
  
  return true;
}

bool	cxform::is_invisible() const
// Returns true when the cxform leads to alpha == 0
{
	return (255.0 * m_[3][0] + m_[3][1]) <= 0.0;	
}


}	// end namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
