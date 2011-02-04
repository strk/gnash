// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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

#include <string>
#include "ControlTag.h" // for inheritance
#include "SWF.h" // for tag_type definition
#include "MovieClip.h" // for inlines
#include "SWFStream.h" // for inlines
#include "Machine.h"
#include "VM.h"
#include "sprite_definition.h"
#include "Global_as.h"

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

	virtual void executeActions(MovieClip* m, DisplayList& /* dlist */) const
	{
		VM& vm = getVM(*getObject(m));
        abc::Machine* mach = vm.getMachine();
		log_debug("SymbolClassTag: Creating class %s.", _rootClass);
		mach->instantiateClass(_rootClass, vm.getGlobal());
	}

	static void loader(SWFStream& in, TagType tag, movie_definition& m,
            const RunResources& /*r*/)
	{
		assert(tag == SYMBOLCLASS); 
        
        if (!m.isAS3()) {
            IF_VERBOSE_MALFORMED_SWF(
                log_swferror("SWF contains SymbolClass tag, but is not an "
                    "AS3 SWF!");
            );
            throw ParserException("SymbolClass tag found in non-AS3 SWF!");
        }
		
        in.ensureBytes(2);
		boost::uint16_t num_symbols = in.read_u16();
		log_debug("There are %u symbols.", num_symbols);
		for (unsigned int i = 0; i < num_symbols; ++i) {
			in.ensureBytes(2);
			boost::uint16_t id = in.read_u16();
			std::string name;
			in.read_string(name);
			IF_VERBOSE_PARSE(
			log_parse("Symbol %u name %s, character %u", i, name, id);
			);
            
            boost::intrusive_ptr<SymbolClassTag> st(new SymbolClassTag(name));
			
            if (id == 0) m.addControlTag(st);
            else {
                sprite_definition* s =
                    dynamic_cast<sprite_definition*>(m.getDefinitionTag(id));
                
                // TODO: it seems that the pp will add the control tag to
                // the main timeline also if the id is not 0 but the
                // sprite_definition does not exist.
                //
                // Manual tests show that:
                // 1. Only the first SymbolClass tag is executed for the
                //    root Sprite. TODO: check also for other Sprites. This
                //    applies to multiple symbols in the same tag or to
                //    subsequent SymbolClass tags.
                // 2. If the id is not a definition, the tag is added to
                //    the root Sprite (main timeline). TODO: check what
                //    happens if the sprite is defined later.
                if (s) s->addControlTag(st);

            }
		}
	}

private:
	
    SymbolClassTag(std::string name) 
        :
        _rootClass(name)
	{}

    const std::string _rootClass;
};

} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_SYMBOLCLASSTAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
