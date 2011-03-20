// CharacterProxy.cpp - rebindable DisplayObject reference, for Gnash
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
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

#include "CharacterProxy.h"
#include "utility.h"
#include "DisplayObject.h" 
#include "VM.h" 
#include "movie_root.h" 
#include "smart_ptr.h"

#include <string>

namespace gnash {

void
CharacterProxy::checkDangling() const
{
	if (_ptr && _ptr->isDestroyed()) {
		_tgt = _ptr->getOrigTarget();
#ifdef GNASH_DEBUG_SOFT_REFERENCES
		log_debug("char %s (%s) was destroyed, stored its orig target "
                "(%s) for later rebinding", _ptr->getTarget(), typeName(*_ptr),
                _tgt);
#endif
		_ptr = 0;
	}
}

std::string
CharacterProxy::getTarget() const
{
    // set _ptr to NULL and _tgt to original target if destroyed
	checkDangling(); 
	if ( _ptr ) return _ptr->getTarget();
	return _tgt;
}

void
CharacterProxy::setReachable() const
{
	checkDangling();
	if ( _ptr ) _ptr->setReachable();
}

DisplayObject*
findDisplayObjectByTarget(const std::string& tgtstr, movie_root& mr)
{
	if (tgtstr.empty()) return 0;
	return mr.findCharacterByTarget(tgtstr);
}

} // namespace gnash

