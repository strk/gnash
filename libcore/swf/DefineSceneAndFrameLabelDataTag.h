// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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

#ifndef GNASH_SWF_SCENEANDLABELTAG_H
#define GNASH_SWF_SCENEANDLABELTAG_H

#include "ControlTag.h"
#include "SWF.h" 
#include "MovieClip.h" 
#include "SWFStream.h"
#include "log.h"

#include <string>
#include <map>

// Forward declarations
namespace gnash {
    class movie_definition;
}

namespace gnash {
namespace SWF {

class DefineSceneAndFrameLabelDataTag : public ControlTag
{
public:

    /// TODO: implement this.
	virtual void executeState(MovieClip* /*m*/, DisplayList& /* dlist */) const
	{
        log_unimpl("DefineSceneAndFrameLabelDataTag");
	}

	static void loader(SWFStream& in, TagType tag, movie_definition& m,
            const RunResources& /*r*/)
	{
		assert(tag == DEFINESCENEANDFRAMELABELDATA); 
        
        if (!m.isAS3()) {
            IF_VERBOSE_MALFORMED_SWF(
                log_swferror("SWF contains DefineSceneAndFrameLabelData tag, "
                    "but is not an AS3 SWF!");
            );
            throw ParserException("DefineSceneAndFrameLabelData tag found in "
                    "non-AS3 SWF!");
        }

        boost::intrusive_ptr<ControlTag> t(
            new DefineSceneAndFrameLabelDataTag(in));

        /// This tag is only added to the main timeline (SWFMovieDefinition).
        m.addControlTag(t);
        
    }

private:
    
    DefineSceneAndFrameLabelDataTag(SWFStream& in)
    {
        read(in);
    }

    void read(SWFStream& in) {
        
        boost::uint32_t scenes = in.read_V32();

        IF_VERBOSE_PARSE(log_parse("Scene count: %d", scenes));

        for (size_t i = 0; i < scenes; ++i) {
            boost::uint32_t offset = in.read_V32();
            std::string name;
            in.read_string(name);
            IF_VERBOSE_PARSE(log_parse("Offset %d name: %s", offset, name));
            _scenes[offset] = name;
        }

        boost::uint32_t labels = in.read_V32();

        for (size_t i = 0; i < labels; ++i) {
            boost::uint32_t num = in.read_V32();
            std::string label;
            in.read_string(label);
            IF_VERBOSE_PARSE(log_parse("Frame %d label: %s", num, label));
            _frames[num] = label;
        }

    }

    std::map<boost::uint32_t, std::string> _scenes;
    std::map<boost::uint32_t, std::string> _frames;

};

} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_SYMBOLCLASSTAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
