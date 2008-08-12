// 
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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

#ifndef GNASH_SWF_SYMBOLCLASSTAG_H
#define GNASH_SWF_SYMBOLCLASSTAG_H

#include "ControlTag.h" // for inheritance
#include "swf.h" // for tag_type definition
#include "action_buffer.h" // for composition
#include "sprite_instance.h" // for inlines
#include "stream.h" // for inlines

// Forward declarations
namespace gnash {
	class movie_definition;
}

namespace gnash {
namespace SWF {

/// SWF Tag SymbolClass (76) 
//
class SymbolClassTag : public ControlTag
{
public:

	SymbolClassTag()

	{}

	static void loader(SWFStream& in,tag_type tag, movie_definition* m)
	{
		assert(tag == SYMBOLCLASS); //76

		log_unimpl(_("%s tag parsed but not yet used"), "SYMBOLCLASS");
		boost::uint16_t num_symbols = in.read_u16();
		log_debug("There are %u symbols.",num_symbols);
		for(unsigned int i = 0;i<num_symbols;i++){
			boost::uint16_t character = in.read_u16();
			std::string name;
			in.read_string(name);
			log_debug("Symbol %u name=%s tag=%u",i,name,character);
		}
	}
	
};

} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_SYMBOLCLASSTAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
