/* 
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
 *   Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 */ 

#include "FuzzyPixel.h"
#include "RGBA.h" // for rgba class

#include <algorithm>

namespace gnash {

bool
FuzzyPixel::operator==(const FuzzyPixel& o) const
{
	// Intolerant FuzzyPixels never succeed in comparison
	if ( _tol < 0 || o._tol < 0 ) return false;

	int tol=std::max(_tol, o._tol);
	if ( ! fuzzyEqual(_col.m_r, o._col.m_r, tol) ) return false;
	if ( ! fuzzyEqual(_col.m_g, o._col.m_g, tol) ) return false;
	if ( ! fuzzyEqual(_col.m_b, o._col.m_b, tol) ) return false;
	if ( ! fuzzyEqual(_col.m_a, o._col.m_a, tol) ) return false;
	return true;
}

std::ostream&
operator<< (std::ostream& o, const FuzzyPixel& p)
{
	return o << "FuzzyPixel(" << p._col << ") [tol:" << p._tol << "]";
}

} // namespace gnash
