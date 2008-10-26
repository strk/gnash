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

#ifndef GNASH_EXECUTE_TAG_H
#define GNASH_EXECUTE_TAG_H

#include <cassert>

namespace gnash {

// Forward declarations
class MovieClip;
class DisplayList;

/// Control tags are swf tags that control the operation of the movie. 
//
/// Essentially, these are the events associated with a frame.
///
class ControlTag
{
public:

	virtual ~ControlTag()
	{
	}

	/// Execute this tag, whatever it is.
	virtual void execute(MovieClip* /*m*/, DisplayList& /*dlist*/) const
	{
	}

	/// Execute this tag but only if it's a "state" tag.
	//
	/// State tags include all tags except action tags.
	///
	virtual void execute_state(MovieClip* /*m*/,  DisplayList& /*dlist*/) const
	{
	}

	/// Execute this tag but only if it is an action tag
	void execute_action(MovieClip* m, DisplayList& dlist) const
	{
		if ( is_action_tag() ) execute(m, dlist);
	}

	/// Return true if this is an action tag.
	virtual bool	is_action_tag() const { return false; }

};


} // namespace gnash


#endif // GNASH_EXECUTE_TAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
