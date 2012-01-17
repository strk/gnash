// fontlib.h - Internal interfaces to fontlib, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
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


#ifndef GNASH_FONTLIB_H
#define GNASH_FONTLIB_H

#include <boost/intrusive_ptr.hpp>

// Forward declarations
namespace gnash {
	class Font;
}

namespace gnash {

/// Library management
//
/// Font library control.  gnash is able to substitute fonts
/// from the font library, in case a movie lacks glyphs for a
/// declared font.  This would come into play since in recent
/// versions of SWF, the movie is allowed to use "system
/// fonts".  E.g. it can declare a font named "Arial", but not
/// provide glyphs for it, and then the OS is expected to
/// provide the font or a suitable replacement.
///
///
namespace fontlib {

	// For adding fonts.
	void add_font(Font* f);

	/// Clean up the font library
	void clear();

	Font* get_font(const std::string& name, bool bold, bool italic);

	/// Return a default device font.
	boost::intrusive_ptr<Font> get_default_font();
	
}	// end namespace fontlib
}	// end namespace gnash



#endif // GNASH_FONTLIB_H

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
