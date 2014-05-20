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

#ifndef GNASH_SWF_EXPORTASSETSTAG_H
#define GNASH_SWF_EXPORTASSETSTAG_H

#include <vector>
#include <utility>
#include <string>
#include <memory>

#include "ControlTag.h"
#include "Movie.h"
#include "MovieClip.h"
#include "SWFStream.h"
#include "log.h"

namespace gnash {
namespace SWF {

class ExportAssetsTag : public ControlTag
{
public:

    typedef std::vector<std::string> Exports;

    // Load an export tag (for exposing internal resources of m)
    static void loader(SWFStream& in, TagType tag, movie_definition& m,
            const RunResources& /*r*/)
    {
        assert(tag == SWF::EXPORTASSETS); // 56

        boost::intrusive_ptr<ControlTag> t(new ExportAssetsTag(in, m));
        m.addControlTag(t);
    }


    // TODO: use Movie to store the actual exports.
    virtual void executeState(MovieClip* m, DisplayList& /*l*/) const {
        Movie* mov = m->get_root();
        for (Exports::const_iterator it = _exports.begin(), e = _exports.end();
                it != e; ++it) {
            const std::uint16_t id = mov->definition()->exportID(*it);

            // We exported it, so we assume it is known.
            assert(id);
            mov->addCharacter(id);
        }
    }

private:

    ExportAssetsTag(SWFStream& in, movie_definition& m)
    {
        read(in, m);
    }

    void read(SWFStream& in, movie_definition& m) {
        
        in.ensureBytes(2);
        const std::uint16_t count = in.read_u16();

        IF_VERBOSE_PARSE(
            log_parse(_("  export: count = %d"), count);
        );

        // Read the exports.
        for (size_t i = 0; i < count; ++i) {
            in.ensureBytes(2);
            const std::uint16_t id = in.read_u16();

            if (!id) continue;

            std::string symbolName;
            in.read_string(symbolName);

            IF_VERBOSE_PARSE (
                log_parse(_("  export: id = %d, name = %s"), id, symbolName);
            );

            // Register export with global map
            m.registerExport(symbolName, id);

            // Store export for later execution.
            _exports.push_back(symbolName);
        }

    }

private:

    Exports _exports;

};

} // namespace SWF
} // namespace gnash

#endif
