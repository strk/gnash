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

#ifndef GNASH_SWF_DOINITACTIONTAG_H
#define GNASH_SWF_DOINITACTIONTAG_H

#include "ControlTag.h" // for inheritance
#include "swf.h" // for tag_type definition
#include "action_buffer.h" // for composition
#include "sprite_instance.h" // for inlines
#include "SWFStream.h" // for inlines

// Forward declarations
namespace gnash {
    class movie_definition;
}

namespace gnash {
namespace SWF {

/// SWF Tag DoInitAction (59)
//
/// Thin wrapper around action_buffer.
///
class DoInitActionTag : public ControlTag
{
public:

    DoInitActionTag(SWFStream& in, movie_definition& md, int cid)
	:
	_buf(md),
	_cid(cid)
    {
        read(in);
    }

    virtual void execute_state(sprite_instance* m, DisplayList& /*dlist*/) const
    {
        m->execute_init_action_buffer(_buf, _cid);
    }

    virtual void execute(sprite_instance* m, DisplayList& /*dlist*/) const
    {
        m->execute_init_action_buffer(_buf, _cid);
    }

    // Tell the caller that we are an action tag.
    virtual bool is_action_tag() const
    {
        return true;
    }

    static void doInitActionLoader(SWFStream& in, tag_type tag, movie_definition& m)
    {
        in.ensureBytes(2);
        int cid = in.read_u16();
        DoInitActionTag* da = new DoInitActionTag(in, m, cid);

        IF_VERBOSE_PARSE (
        log_parse(_("  tag %d: do_init_action_loader"), tag);
        log_parse(_("  -- init actions for sprite %d"), cid);
        );

        //m->add_init_action(da, cid); // ownership transferred
        m.addControlTag(da); // ownership transferred
    }

private:

    /// Read a DoInitAction block from the SWFStream
    //
    void read(SWFStream& in)
    {
        _buf.read(in, in.get_tag_end_position());
    }


    action_buffer _buf;

    // id of referenced character definition
    int _cid;
};

} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_DOINITACTIONTAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

