// visual.h	-- Thatcher Ulrich <tu@tulrich.com>

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// "visual", a base class interface for renderable objects.  Plus a
// few basic visual nodes.


#ifndef VISUAL_H
#define VISUAL_H


#include "view_state.h"


class visual
// Interface for things that can be rendered.
{
public:
	virtual void	render( const view_state& v ) = 0;
	float	get_radius() const { return m_radius; }

protected:
	float	m_radius;
};


class visual_transform : virtual public visual
// Node which contains a transform and another visual node, and
// transforms the render query before passing it down.
{
public:
	void	render( const view_state& v );	// render transformed child visual.

	const matrix&	get_transform() const { return m_transform; }
	void	set_transform( const matrix& m ) { m_transform = m; }

	visual*	get_visual() { return m_visual; }
	void	set_visual( visual* v ) { m_visual = v; }

private:
	visual*	m_visual;
	matrix	m_transform;
};


class visual_list : virtual public visual
// Node which contains a collection of visuals.
{
public:

private:

};


//class visual_label;	// label mix-in.


#endif // VISUAL_H
