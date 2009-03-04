// 
//   Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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




#include "DynamicShape.h"

#include <algorithm>


namespace gnash {

DynamicShape::DynamicShape()
	:
	shape_character_def(),
	_currpath(0),
	_currfill(0),
	_currline(0),
	_x(0),
	_y(0),
	_changed(0)
{}

void
DynamicShape::clear()
{
	//clear_meshes();
	m_paths.clear();
	m_fill_styles.clear();
	m_line_styles.clear();
	m_bound.set_null();
	_currpath=0; // or would point to invalid memory
	_currfill = _currline = 0; // or would point to cleared m_fill_style and m_line_styles respectively

	// TODO: worth setting _changed=true ? 
}

void
DynamicShape::add_path(const path& pth)
{
	m_paths.push_back(pth);
	_currpath = &(m_paths.back());
	//compute_bound(&m_bound);
}

void
DynamicShape::endFill()
{
	// Close the path
	if ( _currpath ) _currpath->close();

	// Remove reference to the "current" path, as
	// next drawing will happen on a different one
	_currpath = NULL;

	// Remove fill information
	_currfill = 0;

	// TODO: should I also clear _currline ?
}

void
DynamicShape::beginFill(const rgba& color)
{
	// Add the new fill style and set as current
	fill_style style; style.setSolid(color);

	endFill();

	_currfill = add_fill_style(style);
	// TODO: how to know wheter the fill should be set
	//       as *left* or *right* fill ?
	//       A quick test shows that *left* always work fine !
	path newPath(_x, _y, _currfill, 0, _currline, true); // new fill start new subshapes
	add_path(newPath);
}

void
DynamicShape::beginLinearGradientFill(const std::vector<gradient_record>& grad, const SWFMatrix& mat)
{
	// Add the new fill style and set as current
	fill_style style; style.setLinearGradient(grad, mat);

	endFill();

	_currfill = add_fill_style(style);
	// TODO: how to know wheter the fill should be set
	//       as *left* or *right* fill ?
	//       A quick test shows that *left* always work fine !
	path newPath(_x, _y, _currfill, 0, _currline, true); // new fill start new subshapes
	add_path(newPath);
}

void
DynamicShape::beginRadialGradientFill(const std::vector<gradient_record>& grad, const SWFMatrix& mat)
{
	// Add the new fill style and set as current
	fill_style style; style.setRadialGradient(grad, mat);

	endFill();

	_currfill = add_fill_style(style);
	// TODO: how to know wheter the fill should be set
	//       as *left* or *right* fill ?
	//       A quick test shows that *left* always work fine !
	path newPath(_x, _y, _currfill, 0, _currline, true); // new fill start new subshapes
	add_path(newPath);
}

void
DynamicShape::startNewPath(bool newShape)
{
	// Close any pending filled path
	if ( _currpath && _currfill) _currpath->close();

	// The DrawingApiTest.swf file shows we should NOT
	// necessarely end the current fill when starting a new one.
	//endFill();

	// A quick test shows that *left* always work fine !
	// More than that, using a *right* fill seems to break the tests !
	path newPath(_x, _y, _currfill, 0, _currline, newShape);
	add_path(newPath);
}

void
DynamicShape::finalize()
{
	// Nothing to do if not changed
	if ( ! _changed ) return;

	// Close any pending filled path (_currpath should be last path)
	if ( _currpath && _currfill)
	{
		assert( ! m_paths.empty() );
		assert( _currpath == &(m_paths.back()) );
		_currpath->close();
	}

	// TODO: check consistency of fills and such !

	_changed = false;
}

void
DynamicShape::lineStyle(boost::uint16_t thickness, const rgba& color,
	bool vScale, bool hScale, bool pixelHinting, bool noClose,
	cap_style_e startCapStyle, cap_style_e endCapStyle,
	join_style_e joinStyle, float miterLimitFactor)
{
	line_style style(thickness, color, vScale, hScale, pixelHinting,
		noClose, startCapStyle, endCapStyle, joinStyle,
		miterLimitFactor);

	_currline = add_line_style(style);
	startNewPath(false); // don't make this the start of a new subshape (to verify)
}

void
DynamicShape::resetLineStyle()
{
	_currline = 0;
	startNewPath(false); // don't make this the start of a new subshape (to verify)
}

void
DynamicShape::moveTo(boost::int32_t x, boost::int32_t y)
{
	if ( x != _x || y != _y )
	{
		_x = x;
		_y = y;

		// TODO: close previous path if any and filled ?
		startNewPath(false); // don't make this the start of a new subshape (to verify)
	}
}

void
DynamicShape::lineTo(boost::int32_t x, boost::int32_t y, int swfVersion)
{
	if ( ! _currpath ) startNewPath(true); // first shape is always new (I hope this doesn't break anything)
	assert(_currpath);

	_currpath->drawLineTo(x, y);

	// Update bounds 
	unsigned thickness = _currline ? m_line_styles[_currline-1].getThickness() : 0;
	if ( _currpath->size() == 1 ) {
		_currpath->expandBounds(m_bound, thickness, swfVersion);
	} else {
		m_bound.expand_to_circle(x, y, swfVersion < 8 ? thickness : thickness/2.0);
	}
    
	// Update current pen position
	_x = x;
	_y = y;

	// Mark as changed
	changed();
}

void
DynamicShape::curveTo(boost::int32_t cx, boost::int32_t cy, 
                      boost::int32_t ax, boost::int32_t ay, int swfVersion)
{
	if ( ! _currpath ) startNewPath(true); // first shape is always new (I hope this doesn't break anything)
	assert(_currpath);

	_currpath->drawCurveTo(cx, cy, ax, ay);

	// Update bounds 
	unsigned thickness = _currline ? m_line_styles[_currline-1].getThickness() : 0;
	if ( _currpath->size() == 1 ) {
		_currpath->expandBounds(m_bound, thickness, swfVersion);
	} else {
		m_bound.expand_to_circle(ax, ay, swfVersion < 8 ? thickness : thickness/2.0);
		m_bound.expand_to_circle(cx, cy, swfVersion < 8 ? thickness : thickness/2.0);
	}

	// Update current pen position
	_x = ax;
	_y = ay;

	// Mark as changed
	changed();
}

size_t
DynamicShape::add_fill_style(const fill_style& stl)
{
	typedef FillStyleVect V;
	V& v=m_fill_styles;

	// TODO: check if the style is already in our list
	//       (needs operator== defined for fill_style)
	v.push_back(stl);
	return v.size(); // 1-based !
}

size_t
DynamicShape::add_line_style(const line_style& stl)
{
	typedef LineStyleVect V;
	V& v=m_line_styles;

	// TODO: check if the style is already in our list
	//       (needs operator== defined for line_style)
	v.push_back(stl);
	return v.size(); // 1-based !
}
	
}	// end namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
