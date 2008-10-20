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
#include "sprite_instance.h" // for inlines
#include "SWFStream.h" // for inlines
#include "abc_block.h"
#include "Machine.h"

// Forward declarations
namespace gnash {
	class movie_definition;
}

namespace gnash {
namespace SWF {

/// SWF Tag DoAction (12) 
//
/// Thin wrapper around action_buffer.
///
class DoABCTag : public ControlTag
{
public:

	DoABCTag(abc_block *block) : mABC(block)

	{}

	/// Read a DoAction block from the stream
	//
// 	void read(SWFStream* in)
// 	{
//             m_buf.read(*in, in->get_tag_end_position());
// 	}

	virtual void execute(sprite_instance* m, DisplayList& /* dlist */) const
	{
		VM& vm = VM::get();
		log_debug("getting machine.");
		Machine *mach = vm.getMachine();
		as_object* global = vm.getGlobal();
		
		std::vector<asClass*>::iterator ci = mABC->mClasses.begin();
		for(; ci != mABC->mClasses.end(); ++ci){
			(*ci)->initPrototype();
		}

		std::vector<asMethod*>::iterator mi = mABC->mMethods.begin();
		for(; mi != mABC->mMethods.end(); ++mi){
			(*mi)->initPrototype(mach);
		}

		std::vector<abc_Trait*>::iterator i = mABC->mTraits.begin();
		
		for ( ; i != mABC->mTraits.end(); ++i)
		{
			(*i)->finalize(mABC);
		}
		mABC->mTraits.clear();
		log_debug("Begin execute abc_block.");
//		log_debug("Getting entry script.");
//		asClass* start_script = a.mScripts.back();
//		log_debug("Getting constructor.");
//		asMethod* method = start_script->getConstructor();
//		log_debug("Loding code stream.");
		mach->initMachine(mABC,global);
		log_debug("Executing machine...");
		mach->execute();
	}

	// Tell the caller that we are an action tag.
	virtual bool is_action_tag() const
	{
	    return true;
	}

	void read(SWFStream* in){


	}
	
	static void doABCLoader(SWFStream& in,tag_type tag, movie_definition& m)
	{
		if(tag == SWF::DOABCDEFINE){
			in.ensureBytes(4);
			static_cast<void> (in.read_u32());
			std::string name;
			in.read_string(name);
		}

		abc_block* block = new abc_block();
		block->read(in);
//		mABC = block;
		DoABCTag *ABCtag = new DoABCTag(block);
		
/*
		IF_VERBOSE_PARSE (
		log_parse(_("tag %d: do_action_loader"), tag);
		log_parse(_("-- actions in frame %d"), m->get_loading_frame());
		);*/

		m.addControlTag(ABCtag); // ownership transferred
	}

private:

	abc_block *mABC;
	
};

} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_DOABCTAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
