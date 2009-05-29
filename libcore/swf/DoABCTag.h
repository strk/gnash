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

#ifndef GNASH_SWF_DOABCTAG_H
#define GNASH_SWF_DOABCTAG_H

#include "ControlTag.h" // for inheritance
#include "swf.h" // for tag_type definition
#include "action_buffer.h" // for composition
#include "MovieClip.h" // for inlines
#include "SWFStream.h" // for inlines
#include "abc_block.h"
#include "Machine.h"
#include "VM.h"

// Forward declarations
namespace gnash {
	class movie_definition;
}

namespace gnash {
namespace SWF {

/// SWF Tag DoABC (72)
//
/// Execute an ABC tag. This is AVM2 bytecode.
class DoABCTag : public ControlTag
{
public:

    virtual void execute(MovieClip* m, DisplayList& /* dlist */) const
	{
		VM& vm = m->getVM();
        
        log_debug("getting machine.");
		Machine *mach = vm.getMachine();
		as_object* global = vm.getGlobal();
		
        mABC->prepare(mach);

		log_debug("Begin execute abc_block.");
		mach->initMachine(mABC, global);
		log_debug("Executing machine...");
		mach->execute();
	}

	// Tell the caller that we are an action tag.
	virtual bool is_action_tag() const
	{
	    return true;
	}

    void read(SWFStream* /*in*/)
    {
    }
	
	static void loader(SWFStream& in, TagType tag, movie_definition& m,
            const gnash::RunInfo&)
	{

        if (!m.isAS3()) {
            IF_VERBOSE_MALFORMED_SWF(
                log_swferror("SWF contains ABC tag, but is not an "
                    "AS3 SWF!");
            );
            throw ParserException("ABC tag found in non-AS3 SWF!");
        }

		if (tag == SWF::DOABCDEFINE) {
			in.ensureBytes(4);
			static_cast<void> (in.read_u32());
			std::string name;
			in.read_string(name);
		}

		abc_block* block = new abc_block();
		block->read(in);
        // mABC = block;
		DoABCTag* ABCtag = new DoABCTag(block);
		
		IF_VERBOSE_PARSE (
            log_parse(_("tag %d: DoABCDefine"), tag);
            log_parse(_("-- actions in frame %d"), m.get_loading_frame());
		);

		m.addControlTag(ABCtag); // ownership transferred
	}

private:

	DoABCTag(abc_block *block) : mABC(block) {}

	abc_block *mABC;
	
};

} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_DOABCTAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
