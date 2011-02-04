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

#ifndef GNASH_SWF_SCRIPTLIMITSTAG_H
#define GNASH_SWF_SCRIPTLIMITSTAG_H

#include "SWF.h" // for TagType definition
#include "SWFStream.h" // for inlines
#include "movie_root.h"
#include "movie_definition.h"
#include "ControlTag.h"

namespace gnash {
namespace SWF {

/// \brief Sets the desired limits for recursion and timeout for AS scripts
//
/// A loaded movie containing a ScriptLimits tag should change the *global*
/// scriptlimits setting, so this is kept in movie_root rather than the
/// immutable movie_definition. 
class ScriptLimitsTag : public ControlTag
{
public:

    virtual ~ScriptLimitsTag() {}

    virtual void executeState(MovieClip* m, DisplayList& /*dl*/) const {

        log_debug("Setting script limits: recursion %s, timeout %s",
                _recursionLimit, _timeoutLimit);
        getRoot(*getObject(m)).setScriptLimits(_recursionLimit, _timeoutLimit);
    }

    static void loader(SWFStream& in, TagType tag, movie_definition& m,
            const RunResources& /*r*/)
    {
        assert(tag = SWF::SCRIPTLIMITS);
        boost::intrusive_ptr<ControlTag> s(new ScriptLimitsTag(in));
        m.addControlTag(s);
    }

private:

    ScriptLimitsTag(SWFStream& in)
        :
        _recursionLimit(0),
        _timeoutLimit(0)
    {
        in.ensureBytes(4); 
        _recursionLimit = in.read_u16();
        _timeoutLimit = in.read_u16();      

        IF_VERBOSE_PARSE (
            log_parse(_("  ScriptLimits tag: recursion: %d, timeout: %d"),
                    _recursionLimit, _timeoutLimit);
	    );

    }
    
    boost::uint16_t _recursionLimit;
    boost::uint16_t _timeoutLimit;
};

} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_SCRIPTLIMITSTAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
