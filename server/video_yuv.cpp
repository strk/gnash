//
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

//
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gnash.h"

namespace gnash {

enum {Y, U, V, T, NB_TEXS};

YUV_video::YUV_video(int w, int h):
	m_width(w),
	m_height(h)
{
	planes[Y].w = m_width;
	planes[Y].h = m_height;
	planes[Y].size = m_width * m_height;
	planes[Y].offset = 0;

	planes[U] = planes[Y];
	planes[U].w >>= 1;
	planes[U].h >>= 1;
	planes[U].size >>= 2;
	planes[U].offset = planes[Y].size;

	planes[V] = planes[U];
	planes[V].offset += planes[U].size;

	m_size = planes[Y].size + (planes[U].size << 1);

	for (int i = 0; i < 3; ++i)
	{
		planes[i].id = 0;	//texids[i];

		unsigned int ww = planes[i].w;
		unsigned int hh = planes[i].h;
		planes[i].unit = 0; // i[units];
		planes[i].p2w = (ww & (ww - 1)) ? video_nlpo2(ww) : ww;
		planes[i].p2h = (hh & (hh - 1)) ? video_nlpo2(hh) : hh;
		float tw = (double) ww / planes[i].p2w;
		float th = (double) hh / planes[i].p2h;

		planes[i].coords[0][0] = 0.0;
		planes[i].coords[0][1] = 0.0;
		planes[i].coords[1][0] = tw;
		planes[i].coords[1][1] = 0.0;
		planes[i].coords[2][0] = tw; 
		planes[i].coords[2][1] = th;
		planes[i].coords[3][0] = 0.0;
		planes[i].coords[3][1] = th;
	}

	m_data = new uint8_t[m_size];

//		m_bounds->m_x_min = 0.0f;
//		m_bounds->m_x_max = 1.0f;
//		m_bounds->m_y_min = 0.0f;
//		m_bounds->m_y_max = 1.0f;
}	

YUV_video::~YUV_video()
{
	if (m_data) delete [] m_data;
}

unsigned int YUV_video::video_nlpo2(unsigned int x) const
{
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	return x + 1;
}

void YUV_video::update(uint8_t* data)
{
	memcpy(m_data, data, m_size);
}

int YUV_video::size() const
{
	return m_size;
}

void YUV_video::display(const matrix* m, const rect* bounds)
{
}

}  // namespace gnash
